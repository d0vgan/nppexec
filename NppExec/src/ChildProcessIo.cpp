#include "ChildProcessIo.h"
#include "ChildProcess.h"
#include "NppExec.h"
#include "NppExecScriptEngine.h"

extern PseudoConsoleHelper g_pseudoCon;

// P3.2/P3.4: defaults and caps when RichEdit metrics are unavailable or out of range.
static const SHORT nPseudoConDefaultCols = 192;
static const SHORT nPseudoConDefaultRows = 50;
static const SHORT nPseudoConMinCols     = 40;
static const SHORT nPseudoConMinRows     = 10;
static const SHORT nPseudoConMaxCols     = 500;
static const SHORT nPseudoConMaxRows     = 1000;

static SHORT clampPseudoConDim(int value, int minVal, int maxVal)
{
    if ( value < minVal )
        return (SHORT) minVal;
    if ( value > maxVal )
        return (SHORT) maxVal;
    return (SHORT) value;
}

static COORD getPseudoConsoleSizeFromConsole(CNppExec* pNppExec)
{
    COORD size = { nPseudoConDefaultCols, nPseudoConDefaultRows };

    if ( !pNppExec )
        return size;

    const HWND hConsoleWnd = pNppExec->GetConsole().GetConsoleWnd();
    if ( !hConsoleWnd )
        return size;

    RECT rcClient;
    if ( !::GetClientRect(hConsoleWnd, &rcClient) )
        return size;

    const int clientW = rcClient.right - rcClient.left;
    const int clientH = rcClient.bottom - rcClient.top;
    if ( clientW <= 0 || clientH <= 0 )
        return size;

    HDC hDc = ::GetDC(hConsoleWnd);
    if ( !hDc )
        return size;

    HFONT hFont = (HFONT) ::SendMessage(hConsoleWnd, WM_GETFONT, 0, 0);
    if ( !hFont )
        hFont = (HFONT) ::GetStockObject(ANSI_FIXED_FONT);

    HFONT hOldFont = (HFONT) ::SelectObject(hDc, hFont);

    TEXTMETRIC tm;
    ::ZeroMemory(&tm, sizeof(tm));
    if ( ::GetTextMetrics(hDc, &tm) )
    {
        int charW = tm.tmAveCharWidth;
        int charH = tm.tmHeight;
        if ( charW <= 0 )
            charW = 8;
        if ( charH <= 0 )
            charH = 16;

        const int cols = clientW / charW;
        const int rows = clientH / charH;

        size.X = clampPseudoConDim(cols, nPseudoConMinCols, nPseudoConMaxCols);
        size.Y = clampPseudoConDim(rows, nPseudoConMinRows, nPseudoConMaxRows);
    }

    ::SelectObject(hDc, hOldFont);
    ::ReleaseDC(hConsoleWnd, hDc);

    return size;
}

CChildProcessIo::CChildProcessIo()
    : m_hStdInReadPipe(NULL)
    , m_hStdInWritePipe(NULL)
    , m_hStdOutReadPipe(NULL)
    , m_hStdOutWritePipe(NULL)
    , m_hStdErrReadPipe(NULL)
    , m_hStdErrWritePipe(NULL)
    , m_hPseudoCon(NULL)
    , m_pAttributeList(NULL)
    , m_hStdoutReaderThread(NULL)
    , m_bStdoutReaderStop(0)
    , m_hStdinWriterThread(NULL)
    , m_bStdinWriterStop(0)
    , m_hStdinDataEvent(NULL)
    , m_pOwner(NULL)
{
}

CChildProcessIo::~CChildProcessIo()
{
    stopStdoutReader();
    stopStdinWriter();
    closeAll();
}

void CChildProcessIo::reset()
{
    stopStdoutReader();
    stopStdinWriter();
    closeAll();
    m_pOwner = NULL;

    CCriticalSectionLockGuard lock(m_csOutputQueue);
    m_OutputQueue.DeleteAll();

    CCriticalSectionLockGuard lockIn(m_csStdinQueue);
    m_StdinQueue.DeleteAll();
}

void CChildProcessIo::closePipes()
{
    auto closePipe = [](HANDLE& hPipe)
    {
        if ( hPipe )
        {
            ::CloseHandle(hPipe);
            hPipe = NULL;
        }
    };

    closePipe(m_hStdOutReadPipe);
    closePipe(m_hStdOutWritePipe);
    closePipe(m_hStdErrReadPipe);
    closePipe(m_hStdErrWritePipe);
    closePipe(m_hStdInReadPipe);
    closePipe(m_hStdInWritePipe);
}

void CChildProcessIo::closePseudoConsole()
{
    if ( g_pseudoCon.pfnClosePseudoConsole && m_hPseudoCon )
    {
        g_pseudoCon.pfnClosePseudoConsole(m_hPseudoCon);
        m_hPseudoCon = NULL;
    }
    if ( m_pAttributeList )
    {
        g_pseudoCon.pfnDeleteProcThreadAttributeList(m_pAttributeList);
        m_pAttributeList = NULL;
    }
}

void CChildProcessIo::closeAll()
{
    closePipes();
    closePseudoConsole();
}

bool CChildProcessIo::createPipes(CNppExec* pNppExec)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;

    if ( CChildProcess::IsWindowsNT() )
    {
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        sa.lpSecurityDescriptor = &sd;
    }
    else
    {
        sa.lpSecurityDescriptor = NULL;
    }
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    const int DEFAULT_PIPE_SIZE = 0;

    if ( !::CreatePipe(&m_hStdOutReadPipe, &m_hStdOutWritePipe, &sa, DEFAULT_PIPE_SIZE) )
    {
        pNppExec->GetConsole().PrintError( _T("CreatePipe(<StdOut>) failed") );
        return false;
    }
    if ( m_hStdOutWritePipe == NULL )
    {
        if ( m_hStdOutReadPipe != NULL )
            ::CloseHandle(m_hStdOutReadPipe);
        pNppExec->GetConsole().PrintError( _T("hStdOutWritePipe = NULL") );
        return false;
    }
    if ( m_hStdOutReadPipe == NULL )
    {
        ::CloseHandle(m_hStdOutWritePipe);
        pNppExec->GetConsole().PrintError( _T("hStdOutReadPipe = NULL") );
        return false;
    }

    if ( !::CreatePipe(&m_hStdInReadPipe, &m_hStdInWritePipe, &sa, DEFAULT_PIPE_SIZE) )
    {
        pNppExec->GetConsole().PrintError( _T("CreatePipe(<StdIn>) failed") );
        return false;
    }
    if ( m_hStdInWritePipe == NULL )
    {
        if ( m_hStdInReadPipe != NULL )
            ::CloseHandle(m_hStdInReadPipe);
        pNppExec->GetConsole().PrintError( _T("hStdInWritePipe = NULL") );
        return false;
    }
    if ( m_hStdInReadPipe == NULL )
    {
        ::CloseHandle(m_hStdInWritePipe);
        pNppExec->GetConsole().PrintError( _T("hStdInReadPipe = NULL") );
        return false;
    }

    ::SetHandleInformation(m_hStdInWritePipe, HANDLE_FLAG_INHERIT, 0);
    ::SetHandleInformation(m_hStdOutReadPipe, HANDLE_FLAG_INHERIT, 0);

    return true;
}

bool CChildProcessIo::createPseudoConsole(CNppExec* pNppExec)
{
    m_hPseudoCon = NULL;
    m_pAttributeList = NULL;

    if ( !pNppExec->GetOptions().GetBool(OPTB_CHILDP_PSEUDOCONSOLE) || (g_pseudoCon.pfnCreatePseudoConsole == nullptr) )
        return true;

    const COORD conSize = getPseudoConsoleSizeFromConsole(pNppExec);
    const HRESULT hr = g_pseudoCon.pfnCreatePseudoConsole(conSize, m_hStdInReadPipe, m_hStdOutWritePipe, 0,
        &m_hPseudoCon);
    if ( FAILED(hr) )
        m_hPseudoCon = NULL;

    return true;
}

bool CChildProcessIo::resizePseudoConsole(CNppExec* pNppExec)
{
    if ( !m_hPseudoCon || !g_pseudoCon.pfnResizePseudoConsole )
        return false;

    const COORD conSize = getPseudoConsoleSizeFromConsole(pNppExec);
    const HRESULT hr = g_pseudoCon.pfnResizePseudoConsole(m_hPseudoCon, conSize);
    return SUCCEEDED(hr);
}

void CChildProcessIo::configureStartupInfo(STARTUPINFOEX& si, DWORD& dwCreationFlags)
{
    // Match HEAD CChildProcess::Create() startup layout (works with PseudoConsole on Win10+).
    ::ZeroMemory(&si, sizeof(STARTUPINFOEX));
    si.StartupInfo.cb = sizeof(STARTUPINFOEX);
    si.StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.StartupInfo.wShowWindow = SW_HIDE;

    if ( m_hPseudoCon )
    {
        SIZE_T bytesRequired = 0;
        g_pseudoCon.pfnInitializeProcThreadAttributeList(NULL, 1, 0, &bytesRequired);
        si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST) ::HeapAlloc(::GetProcessHeap(), 0, bytesRequired);
        if ( si.lpAttributeList )
        {
            if ( g_pseudoCon.pfnInitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &bytesRequired) )
            {
                if ( g_pseudoCon.pfnUpdateProcThreadAttribute(
                         si.lpAttributeList,
                         0,
                         PseudoConsoleHelper::constPROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
                         m_hPseudoCon,
                         sizeof(m_hPseudoCon),
                         NULL,
                         NULL) )
                {
                    dwCreationFlags |= PseudoConsoleHelper::constEXTENDED_STARTUPINFO_PRESENT;
                    m_pAttributeList = si.lpAttributeList;
                }
                else
                {
                    ::HeapFree(::GetProcessHeap(), 0, si.lpAttributeList);
                    closePseudoConsole();
                }
            }
            else
            {
                ::HeapFree(::GetProcessHeap(), 0, si.lpAttributeList);
                closePseudoConsole();
            }
        }
        else
        {
            closePseudoConsole();
        }
    }

    if ( !m_hPseudoCon )
    {
        si.StartupInfo.hStdInput = m_hStdInReadPipe;
        si.StartupInfo.hStdOutput = m_hStdOutWritePipe;
        si.StartupInfo.hStdError = m_hStdOutWritePipe; // legacy: stderr merged into stdout pipe
    }
}

void CChildProcessIo::onChildProcessStarted()
{
    // HEAD: after CreateProcess, release ConPTY pipe ends held by this process.
    if ( m_hPseudoCon )
    {
        if ( m_hStdOutWritePipe )
        {
            ::CloseHandle(m_hStdOutWritePipe);
            m_hStdOutWritePipe = NULL;
        }
        if ( m_hStdInReadPipe )
        {
            ::CloseHandle(m_hStdInReadPipe);
            m_hStdInReadPipe = NULL;
        }
        return;
    }

    // Pipe mode (P1.4): close child-side ends in parent so EOF is detected on exit.
    if ( m_hStdOutWritePipe )
    {
        ::CloseHandle(m_hStdOutWritePipe);
        m_hStdOutWritePipe = NULL;
    }
    if ( m_hStdInReadPipe )
    {
        ::CloseHandle(m_hStdInReadPipe);
        m_hStdInReadPipe = NULL;
    }
}

HANDLE CChildProcessIo::getStdInWritePipe() const
{
    return m_hStdInWritePipe;
}

HANDLE CChildProcessIo::getStdOutReadPipe() const
{
    return m_hStdOutReadPipe;
}

HANDLE CChildProcessIo::getStdErrReadPipe() const
{
    return m_hStdErrReadPipe;
}

bool CChildProcessIo::isPseudoCon() const
{
    return (m_hPseudoCon != NULL);
}

bool CChildProcessIo::hasSeparateStderrPipe() const
{
    return (m_hStdErrReadPipe != NULL) && !isPseudoCon();
}

void CChildProcessIo::enqueueOutputChunk(const char* pData, int nLen, bool bStderr)
{
    if ( (!pData) || (nLen <= 0) )
        return;

    CChildProcessOutputChunk item;
    item.m_Data.Assign(pData, nLen);
    item.m_bStderr = bStderr;

    CCriticalSectionLockGuard lock(m_csOutputQueue);
    m_OutputQueue.Add(item);
}

void CChildProcessIo::enqueueStdoutChunk(const char* pData, int nLen)
{
    enqueueOutputChunk(pData, nLen, false);
}

bool CChildProcessIo::popOutputChunk(CStrT<char>& chunk, bool& bStderr)
{
    CCriticalSectionLockGuard lock(m_csOutputQueue);

    CListItemT< CChildProcessOutputChunk >* pItem = m_OutputQueue.GetFirst();
    if ( !pItem )
        return false;

    const CChildProcessOutputChunk& item = pItem->GetItem();
    chunk = item.m_Data;
    bStderr = item.m_bStderr;
    m_OutputQueue.Delete(pItem);
    return true;
}

bool CChildProcessIo::popStdoutChunk(CStrT<char>& chunk)
{
    bool bStderr = false;
    if ( !popOutputChunk(chunk, bStderr) )
        return false;
    return !bStderr;
}

DWORD CChildProcessIo::readFromPipe(HANDLE hPipe, char* pBuf, DWORD nBufSize)
{
    // P1.3: blocking ReadFile on the reader thread only (no PeekNamedPipe gate).
    if ( (!pBuf) || (nBufSize < 2) || (!hPipe) )
        return 0;

    DWORD dwBytesRead = 0;
    const DWORD dwToRead = (nBufSize - 1);
    if ( ::ReadFile(hPipe, pBuf, dwToRead, &dwBytesRead, NULL) && (dwBytesRead > 0) )
    {
        pBuf[dwBytesRead] = 0;
        return dwBytesRead;
    }

    return 0;
}

void CChildProcessIo::drainPipeToQueue(HANDLE hPipe, bool bStderr)
{
    if ( !hPipe )
        return;

    char Buf[CONSOLEPIPE_BUFSIZE];

    for ( ;; )
    {
        const DWORD dwRead = readFromPipe(hPipe, Buf, CONSOLEPIPE_BUFSIZE);
        if ( dwRead > 0 )
        {
            enqueueOutputChunk(Buf, (int)dwRead, bStderr);
            continue;
        }

        const DWORD dwErr = ::GetLastError();
        if ( (dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_PIPE_NOT_CONNECTED) )
            break;

        break;
    }
}

void CChildProcessIo::drainStdoutToQueue()
{
    drainPipeToQueue(m_hStdOutReadPipe, false);
    if ( hasSeparateStderrPipe() )
        drainPipeToQueue(m_hStdErrReadPipe, true);
}

bool CChildProcessIo::startStdoutReader(CChildProcess* pOwner)
{
    stopStdoutReader();

    if ( !m_hStdOutReadPipe || !pOwner )
        return false;

    m_pOwner = pOwner;
    InterlockedExchange(&m_bStdoutReaderStop, 0);

    m_hStdoutReaderThread = ::CreateThread(NULL, 0, stdoutReaderThreadProc, this, 0, NULL);
    return (m_hStdoutReaderThread != NULL);
}

void CChildProcessIo::stopStdoutReader()
{
    InterlockedExchange(&m_bStdoutReaderStop, 1);

    if ( m_hStdoutReaderThread )
    {
        // Unblock a synchronous ReadFile in the reader thread (P1.3).
        ::CancelSynchronousIo(m_hStdoutReaderThread);
        const DWORD dwWait = ::WaitForSingleObject(m_hStdoutReaderThread, 5000);
        if ( dwWait == WAIT_TIMEOUT )
            ::TerminateThread(m_hStdoutReaderThread, 0);
        ::CloseHandle(m_hStdoutReaderThread);
        m_hStdoutReaderThread = NULL;
    }

    m_pOwner = NULL;
}

bool CChildProcessIo::isStdoutReaderActive() const
{
    return (m_hStdoutReaderThread != NULL);
}

DWORD WINAPI CChildProcessIo::stdoutReaderThreadProc(LPVOID pParam)
{
    CChildProcessIo* pIo = (CChildProcessIo*) pParam;
    if ( pIo )
        pIo->stdoutReaderThreadFunc();
    return 0;
}

bool CChildProcessIo::enqueueStdin(const char* pData, int nLen, bool bFlush)
{
    if ( !m_hStdInWritePipe )
        return false;

    if ( (!pData) || (nLen <= 0) )
    {
        if ( !bFlush )
            return false;
    }

    CChildProcessStdinItem item;
    if ( pData && (nLen > 0) )
        item.m_Data.Assign(pData, nLen);
    item.m_bFlush = bFlush;

    {
        CCriticalSectionLockGuard lock(m_csStdinQueue);
        m_StdinQueue.Add(item);
    }

    if ( m_hStdinDataEvent )
        ::SetEvent(m_hStdinDataEvent);

    return true;
}

bool CChildProcessIo::popStdinItem(CChildProcessStdinItem& item)
{
    CCriticalSectionLockGuard lock(m_csStdinQueue);

    CListItemT< CChildProcessStdinItem >* pListItem = m_StdinQueue.GetFirst();
    if ( !pListItem )
        return false;

    item = pListItem->GetItem();
    m_StdinQueue.Delete(pListItem);
    return true;
}

void CChildProcessIo::writeStdinDirect(const char* pData, int nLen, bool bFlush)
{
    CChildProcessStdinItem item;
    if ( pData && (nLen > 0) )
        item.m_Data.Assign(pData, nLen);
    item.m_bFlush = bFlush;
    writeStdinItemToPipe(item);
}

void CChildProcessIo::writeStdinItemToPipe(const CChildProcessStdinItem& item)
{
    if ( !m_hStdInWritePipe )
        return;

    const int nLen = item.m_Data.length();
    if ( nLen > 0 )
    {
        DWORD dwBytesWritten = 0;
        ::WriteFile(m_hStdInWritePipe, item.m_Data.c_str(), (DWORD)nLen, &dwBytesWritten, NULL);
    }

    if ( item.m_bFlush )
    {
        // P1.7: only when WriteInput(..., bFFlush=true). Can block until the child reads stdin
        // (pipe full). Never used by default callers; runs on the writer thread only (P1.6).
        ::FlushFileBuffers(m_hStdInWritePipe);
    }
}

bool CChildProcessIo::startStdinWriter()
{
    stopStdinWriter();

    if ( !m_hStdInWritePipe )
        return false;

    InterlockedExchange(&m_bStdinWriterStop, 0);

    if ( !m_hStdinDataEvent )
        m_hStdinDataEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    if ( !m_hStdinDataEvent )
        return false;

    m_hStdinWriterThread = ::CreateThread(NULL, 0, stdinWriterThreadProc, this, 0, NULL);
    return (m_hStdinWriterThread != NULL);
}

void CChildProcessIo::stopStdinWriter()
{
    InterlockedExchange(&m_bStdinWriterStop, 1);

    if ( m_hStdinDataEvent )
        ::SetEvent(m_hStdinDataEvent);

    if ( m_hStdinWriterThread )
    {
        ::CancelSynchronousIo(m_hStdinWriterThread);
        const DWORD dwWait = ::WaitForSingleObject(m_hStdinWriterThread, 5000);
        if ( dwWait == WAIT_TIMEOUT )
            ::TerminateThread(m_hStdinWriterThread, 0);
        ::CloseHandle(m_hStdinWriterThread);
        m_hStdinWriterThread = NULL;
    }

    CChildProcessStdinItem item;
    while ( popStdinItem(item) )
        writeStdinItemToPipe(item);

    if ( m_hStdinDataEvent )
    {
        ::CloseHandle(m_hStdinDataEvent);
        m_hStdinDataEvent = NULL;
    }

    CCriticalSectionLockGuard lock(m_csStdinQueue);
    m_StdinQueue.DeleteAll();
}

bool CChildProcessIo::isStdinWriterActive() const
{
    return (m_hStdinWriterThread != NULL);
}

DWORD WINAPI CChildProcessIo::stdinWriterThreadProc(LPVOID pParam)
{
    CChildProcessIo* pIo = (CChildProcessIo*) pParam;
    if ( pIo )
        pIo->stdinWriterThreadFunc();
    return 0;
}

void CChildProcessIo::stdinWriterThreadFunc()
{
    while ( InterlockedCompareExchange(&m_bStdinWriterStop, 0, 0) == 0 )
    {
        if ( m_hStdinDataEvent )
            ::WaitForSingleObject(m_hStdinDataEvent, 200);

        if ( InterlockedCompareExchange(&m_bStdinWriterStop, 0, 0) != 0 )
            break;

        CChildProcessStdinItem item;
        while ( popStdinItem(item) )
        {
            writeStdinItemToPipe(item);
        }
    }
}

bool CChildProcessIo::drainAvailableFromPipe(HANDLE hPipe, char* pBuf, bool bStderr, bool& bPipeAlive)
{
    if ( !hPipe || !pBuf || !bPipeAlive )
        return false;

    bool bReadAny = false;

    for ( ;; )
    {
        DWORD dwAvail = 0;
        if ( !::PeekNamedPipe(hPipe, NULL, 0, NULL, &dwAvail, NULL) || !dwAvail )
            break;

        const DWORD dwRead = readFromPipe(hPipe, pBuf, CONSOLEPIPE_BUFSIZE);
        if ( dwRead > 0 )
        {
            enqueueOutputChunk(pBuf, (int)dwRead, bStderr);
            bReadAny = true;
            continue;
        }

        const DWORD dwErr = ::GetLastError();
        if ( (dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_PIPE_NOT_CONNECTED) || (dwErr == ERROR_OPERATION_ABORTED) )
            bPipeAlive = false;

        break;
    }

    return bReadAny;
}

void CChildProcessIo::pumpAvailableStderr(char* pBuf)
{
    if ( !hasSeparateStderrPipe() || !m_hStdErrReadPipe || !pBuf )
        return;

    bool bErrAlive = true;
    drainAvailableFromPipe(m_hStdErrReadPipe, pBuf, true, bErrAlive);
}

void CChildProcessIo::stdoutReaderThreadFunc()
{
    char Buf[CONSOLEPIPE_BUFSIZE];
    bool bStdoutAlive = (m_hStdOutReadPipe != NULL);
    bool bStderrAlive = hasSeparateStderrPipe();

    // Match legacy readPipesAndOutput: Peek+Read until the pipe is empty, then block for more.
    // One ReadFile per wake left banner bytes in the kernel buffer until a later cycle (python -i).

    while ( InterlockedCompareExchange(&m_bStdoutReaderStop, 0, 0) == 0 )
    {
        if ( !m_pOwner )
            break;

        if ( !m_pOwner->m_pScriptEngine->ContinueExecution() || m_pOwner->isBreaking() )
            break;

        const PROCESS_INFORMATION* pProcInfo = m_pOwner->GetProcessInfo();
        if ( pProcInfo && pProcInfo->hProcess )
        {
            DWORD dwExitCode = STILL_ACTIVE;
            if ( ::GetExitCodeProcess(pProcInfo->hProcess, &dwExitCode) && (dwExitCode != STILL_ACTIVE) )
                break;
        }

        if ( bStderrAlive )
            pumpAvailableStderr(Buf);

        bool bDrainedAny = false;

        if ( bStdoutAlive )
            bDrainedAny = drainAvailableFromPipe(m_hStdOutReadPipe, Buf, false, bStdoutAlive);

        if ( !bStdoutAlive && bStderrAlive )
            bDrainedAny = drainAvailableFromPipe(m_hStdErrReadPipe, Buf, true, bStderrAlive);

        if ( !bStdoutAlive && !bStderrAlive )
            break;

        if ( bDrainedAny )
            continue;

        if ( bStdoutAlive )
        {
            const DWORD dwRead = readFromPipe(m_hStdOutReadPipe, Buf, CONSOLEPIPE_BUFSIZE);
            if ( dwRead > 0 )
            {
                enqueueOutputChunk(Buf, (int)dwRead, false);
            }
            else
            {
                const DWORD dwErr = ::GetLastError();
                if ( (dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_PIPE_NOT_CONNECTED) )
                    bStdoutAlive = false;
                else if ( dwErr == ERROR_OPERATION_ABORTED )
                    break;
            }
        }
        else if ( bStderrAlive )
        {
            const DWORD dwRead = readFromPipe(m_hStdErrReadPipe, Buf, CONSOLEPIPE_BUFSIZE);
            if ( dwRead > 0 )
            {
                enqueueOutputChunk(Buf, (int)dwRead, true);
            }
            else
            {
                const DWORD dwErr = ::GetLastError();
                if ( (dwErr == ERROR_BROKEN_PIPE) || (dwErr == ERROR_PIPE_NOT_CONNECTED) || (dwErr == ERROR_OPERATION_ABORTED) )
                    bStderrAlive = false;
            }
        }
    }
}
