#include "ChildProcess.h"
#include "NppExecHelpers.h"
#include "NppExec.h"
#include "NppExecScriptEngine.h"
#include "DlgConsoleEncoding.h"
#include "cpp/StrSplitT.h"
#include "c_base/MatchMask.h"
#include "c_base/int2str.h"
#include "c_base/str2int.h"

PseudoConsoleHelper g_pseudoCon;

enum eNewLine {
    nlNone = 0,
    nlLF = 1, // \n
    nlCR = 3, // \r
    nlBS = 7  // \b
};

static bool IsProcessStillActive(HANDLE hProcess)
{
    if ( !hProcess )
        return false;

    DWORD dwExitCode = STILL_ACTIVE;
    if ( !::GetExitCodeProcess(hProcess, &dwExitCode) )
        return false;

    return (dwExitCode == STILL_ACTIVE);
}

static bool EncodeInputLine(const TCHAR* szLine, unsigned int enc, CStrT<char>& outBuf)
{
    if ( !szLine )
        return false;

    char* pStr = NULL;
    int   len = 0;

  #ifdef UNICODE

    switch ( enc )
    {
        case CConsoleEncodingDlg::ENC_OEM :
            pStr = SysUniConv::newUnicodeToMultiByte( szLine, -1, CP_OEMCP, &len );
            break;

        case CConsoleEncodingDlg::ENC_UTF8 :
            pStr = SysUniConv::newUnicodeToUTF8( szLine, -1, &len );
            break;

        default:
            pStr = SysUniConv::newUnicodeToMultiByte( szLine, -1, CP_ACP, &len );
            break;
    }

    if ( !pStr )
        return false;

    outBuf.Assign(pStr, len);
    delete [] pStr;

  #else

    bool bNewMemory = false;

    switch ( enc )
    {
        case CConsoleEncodingDlg::ENC_OEM :
            len = lstrlen(szLine);
            pStr = new char[len + 1];
            if ( pStr )
            {
                ::CharToOem(szLine, pStr);
                bNewMemory = true;
            }
            break;

        case CConsoleEncodingDlg::ENC_UTF8 :
            pStr = SysUniConv::newMultiByteToUTF8(szLine, -1, CP_ACP, &len);
            if ( pStr )
                bNewMemory = true;
            break;

        default:
            len = lstrlen(szLine);
            pStr = (char*) szLine;
            bNewMemory = false;
            break;
    }

    if ( !pStr )
        return false;

    outBuf.Assign(pStr, len);
    if ( bNewMemory )
        delete [] pStr;

  #endif

    return true;
}

CChildProcess::CChildProcess(CScriptEngine* pScriptEngine)
{
    m_strInstance = NppExecHelpers::GetInstanceAsString(this);

    m_pNppExec = pScriptEngine->GetNppExec();
    m_pScriptEngine = pScriptEngine;

    reset();

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess - create (instance = %s)"), GetInstanceStr() );
}

CChildProcess::~CChildProcess()
{
    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess - destroy (instance = %s)"), GetInstanceStr() );
}

const TCHAR* CChildProcess::GetInstanceStr() const
{
    return m_strInstance.c_str();
}

void CChildProcess::applyCommandLinePolicy(tstr& sCmdLine, eCommandLinePolicy mode)
{
    if ( mode == clpNone )
        return; // nothing to do

    CStrSplitT<TCHAR> args;
    if ( args.SplitToArgs(sCmdLine, 2) == 0 )
        return; // empty sCmdLine, nothing to do

    const tstr sFileName = args.Arg(0);

    if ( mode == clpComSpec )
    {
        tstr sComSpec = NppExecHelpers::GetEnvironmentVar( _T("COMSPEC") );
        if ( sComSpec.IsEmpty() )
            sComSpec = _T("cmd");

        const tstr sComSpecNamePart = NppExecHelpers::GetFileNamePart(sComSpec, NppExecHelpers::fnpName);
        const tstr sFileNamePart = NppExecHelpers::GetFileNamePart(sFileName, NppExecHelpers::fnpName);
        if ( NppExecHelpers::StrCmpNoCase(sComSpecNamePart, sFileNamePart) == 0 )
            return; // sCmdLine already starts with "cmd"

        if ( sComSpec.Find(_T(' ')) >= 0 )
            NppExecHelpers::StrQuote(sComSpec);
        
        sComSpec += _T(' '); // e.g. "cmd " (notice the trailing space!)
        
        tstr sComSpecSwitches = Runtime::GetNppExec().GetOptions().GetStr(OPTS_CHILDP_COMSPECSWITCHES);
        if ( !sComSpecSwitches.IsEmpty() )
        {
            sComSpec += sComSpecSwitches;
            sComSpec += _T(' '); // e.g. "cmd /C " (notice the trailing space!)
        }

        sCmdLine.Insert(0, sComSpec);
        return;
    }

    // mode == clpPathExt
    tstr sPathExtensions = NppExecHelpers::GetEnvironmentVar( _T("PATHEXT") );
    if ( !sPathExtensions.IsEmpty() )
        NppExecHelpers::StrLower(sPathExtensions); // in lower case!
    else
        sPathExtensions = _T(".com;.exe;.bat;.cmd"); // in lower case!

    CListT<tstr> pathExts;
    if ( StrSplitAsArgs(sPathExtensions.c_str(), pathExts, _T(';')) == 0 )
        return; // no extensions to check, nothing to do

    const tstr existingExt = NppExecHelpers::GetFileNamePart(sFileName, NppExecHelpers::fnpExt);
    if ( !existingExt.IsEmpty() )
    {
        const CListItemT<tstr>* pExt = pathExts.Find( [&existingExt](const tstr& ext) { return (NppExecHelpers::StrCmpNoCase(ext, existingExt) == 0); } );
        if ( pExt )
            return; // the extension from %PATHEXT% is specified explicitly

        // As the existingExt is not empty, we may return here.
        // However, a file name itself may have a form of "file.name", so
        // we still need to inspect all the extensions from %PATHEXT% to
        // check for all possible "file.name.ext" in paths from %PATH%.
    }

    auto findMatchingExtension = [](const tstr& fileName, const CListT<tstr>& exts, const auto& Predicate)
                            {
                                for ( const CListItemT<tstr>* pExt = exts.GetFirst(); pExt != NULL; pExt = pExt->GetNext() )
                                {
                                    const tstr& ext = pExt->GetItem();
                                    if ( Predicate(fileName, ext) )
                                        return ext;
                                }
                                return tstr();
                            };

    auto fexistsWithExt = [](const tstr& fileName, const tstr& ext)
                            {
                                tstr fileNameExt = fileName;
                                fileNameExt += ext;
                                return NppExecHelpers::CheckFileExists(fileNameExt);
                            };

    tstr ext;
    if ( NppExecHelpers::IsFullPath(sFileName) )
    {
        // full path specified...
        if ( NppExecHelpers::CheckFileExists(sFileName) )
            return; // cmd does not try %PATHEXT% for an _existing_ file name

        // check the file extensions in %PATHEXT%...
        ext = findMatchingExtension(sFileName, pathExts, fexistsWithExt);
    }
    else
    {
        // no path specified - check the paths in %PATH% and file extensions in %PATHEXT%...
        tstr sPaths = NppExecHelpers::GetEnvironmentVar( _T("PATH") );
        CListT<tstr> paths;
        if ( !sPaths.IsEmpty() )
        {
            StrSplitAsArgs(sPaths.c_str(), paths, _T(';'));
        }

        const tstr sCurDir = NppExecHelpers::GetCurrentDir();
        if ( !sCurDir.IsEmpty() )
        {
            CListItemT<tstr>* pItem = paths.Find( [&sCurDir](const tstr& path) { return (NppExecHelpers::StrCmpNoCase(path, sCurDir) == 0); } );
            if ( pItem )
            {
                paths.Delete(pItem);
            }
            paths.InsertFirst(sCurDir);
        }

        for ( const CListItemT<tstr>* pPath = paths.GetFirst(); pPath != NULL; pPath = pPath->GetNext() )
        {
            tstr sPathName = pPath->GetItem();
            if ( !sPathName.EndsWith(_T('\\')) && !sPathName.EndsWith(_T('/')) )
                sPathName += _T('\\');
            sPathName += sFileName;
            if ( NppExecHelpers::CheckFileExists(sPathName) )
                return; // cmd does not try %PATHEXT% for an _existing_ file name

            // check the file extensions in %PATHEXT%...
            ext = findMatchingExtension(sPathName, pathExts, fexistsWithExt);
            if ( !ext.IsEmpty() )
                break;
        }
    }

    if ( ext.IsEmpty() )
        return; // no existing full path found - nothing to do

    tstr sFileNameExt = sFileName;
    sFileNameExt += ext;

    int nPos = sCmdLine.Find(sFileName);
    sCmdLine.Replace(nPos, sFileName.length(), sFileNameExt);
}

// cszCommandLine must be transformed by ModifyCommandLine(...) already
bool CChildProcess::Create(HWND hParentWnd, LPCTSTR cszCommandLine)
{
    // Legacy blocking entrypoint — spawn then wait until exit (see CScriptEngine::Do).
    if ( !Start(hParentWnd, cszCommandLine) )
        return false;
    return WaitForExit(INFINITE);
}

bool CChildProcess::Start(HWND /*hParentWnd*/, LPCTSTR cszCommandLine)
{
    STARTUPINFOEX si;
    DWORD         dwCreationFlags = 0;

    reset();
    m_sCmdLine = cszCommandLine;

    if ( !m_Io.createPipes(m_pNppExec) )
        return false;

    if ( !m_Io.createPseudoConsole(m_pNppExec) )
        return false;

    // Job object (HEAD order: before STARTUPINFOEX); kept alive for the whole child lifetime via m_hJob.
    m_hJob = NULL;
    if ( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_KILLPROCTREE) )
    {
        m_hJob = ::CreateJobObject(NULL, NULL);
        if ( m_hJob != NULL )
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli;
            ::ZeroMemory(&jeli, sizeof(jeli));
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

            // Causes all processes associated with the job to terminate when the last handle to the job is closed.
            if ( !::SetInformationJobObject(m_hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) )
            {
                ::CloseHandle(m_hJob);
                m_hJob = NULL;
            }
        }
    }

    m_Io.configureStartupInfo(si, dwCreationFlags);

    eCommandLinePolicy mode = clpNone;
    switch ( m_pNppExec->GetOptions().GetInt(OPTU_CHILDP_RUNPOLICY) )
    {
        case clpPathExt:
            mode = clpPathExt;
            break;
        case clpComSpec:
            mode = clpComSpec;
            break;
    };

    tstr sCmdLine = cszCommandLine;
    applyCommandLinePolicy(sCmdLine, mode);

    if ( m_hJob != NULL )
    {
        BOOL bIsProcessInJob = FALSE;
        if ( ::IsProcessInJob(GetCurrentProcess(), NULL, &bIsProcessInJob) )
        {
            if ( bIsProcessInJob )
                dwCreationFlags |= CREATE_BREAKAWAY_FROM_JOB;
        }
    }

    if ( ::CreateProcess(
            NULL,
            sCmdLine.data(),
            NULL,                        // security
            NULL,                        // security
            m_Io.shouldChildInheritHandles(), // ConPTY: FALSE (MS samples); pipe mode: TRUE (HEAD)
            dwCreationFlags,            // creation flags
            NULL,                        // environment
            NULL,                        // current directory
            &si.StartupInfo,            // startup info
            &m_ProcessInfo              // process info
        ) )
    {
        if ( m_hJob != NULL )
        {
            if ( !::AssignProcessToJobObject(m_hJob, m_ProcessInfo.hProcess) )
            {
                ::CloseHandle(m_hJob);
                m_hJob = NULL;
            }
        }

        ::CloseHandle(m_ProcessInfo.hThread); m_ProcessInfo.hThread = NULL;
        m_Io.onChildProcessStarted();

        // Pipe mode: readPipesDirect on the script thread (legacy HEAD timing).
        // ConPTY: dedicated reader thread — PeekNamedPipe is unreliable on ConPTY pipes
        // and would delay interactive output (e.g. gawk print before getline).
        if ( m_Io.isPseudoCon() )
            m_Io.startStdoutReader(this);

        m_Io.startStdinWriter();

        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
        m_pNppExec->GetConsole().PrintMessage( tstr().Format(80, _T("Process started (PID=%u) >>>"), m_ProcessInfo.dwProcessId), nMsgFlags );

        {
            TCHAR szProcessId[50];
            c_base::_tint2str(GetProcessId(), szProcessId);
            tstr varName = MACRO_PID;
            m_pNppExec->GetMacroVars().SetUserMacroVar( m_pScriptEngine, varName, szProcessId, CNppExecMacroVars::svLocalVar ); // local var
        }

        // this pause is necessary for child processes that return immediatelly
        ::WaitForSingleObject(m_ProcessInfo.hProcess, m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_STARTUPTIMEOUT_MS));

        // Initialize read-loop state (kept across multiple WaitForExit(timeout) calls).
        m_bufLine.Clear(); // just in case :-)
        m_bufLineStderr.Clear();

        m_bPrevLineEmpty = false;
        m_bDoOutputNext = true;
        m_nPrevState = nlNone;

        m_bPrevLineEmptyStderr = false;
        m_bDoOutputNextStderr = true;
        m_nPrevStateStderr = nlNone;

        return true;
    }

    DWORD dwErrorCode = ::GetLastError();

    m_Io.closeAll();
    if ( m_hJob != NULL )
    {
        ::CloseHandle(m_hJob);
        m_hJob = NULL;
    }

    if ( m_pScriptEngine )
    {
        m_pNppExec->GetConsole().PrintError( m_pScriptEngine->GetLastLoggedCmd().c_str() );
    }
    m_pNppExec->GetConsole().PrintSysError( _T("CreateProcess()"), dwErrorCode );

    return false;
}

bool CChildProcess::WaitForExit(unsigned int nTimeoutMs)
{
    if ( !m_ProcessInfo.hProcess )
        return false;

    bool isConsoleProcessRunning = true;

    DWORD dwRead = 0;
    unsigned int nEmptyCount = 0;

    const DWORD dwCycleTimeOut = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_CYCLETIMEOUT_MS);
    const DWORD dwExitTimeOut  = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_EXITTIMEOUT_MS);

    const bool bUseTimeout = (nTimeoutMs != INFINITE);
    ULONGLONG tEnd = 0;
    if ( bUseTimeout )
        tEnd = ::GetTickCount64() + (ULONGLONG) nTimeoutMs;

    bool bTimedOut = false;

    do
    {
        // inside this cycle: the bOutputAll parameter must be controlled within readPipesAndOutput
        dwRead = readPipesAndOutput(m_bufLine, m_bPrevLineEmpty, m_nPrevState, false, m_bDoOutputNext,
                                    m_bufLineStderr, m_bPrevLineEmptyStderr, m_nPrevStateStderr, m_bDoOutputNextStderr);

        if ( CNppExec::_bIsNppShutdown )
        {
            // Notepad++ is exiting
            if ( dwRead == 0 )
            {
                // no output from the child process
                ++nEmptyCount;
                if ( nEmptyCount > (dwExitTimeOut/dwCycleTimeOut) )
                {
                    // no output during more than dwExitTimeOut ms, let's kill the process...
                    m_nBreakMethod = CProcessKiller::killCtrlBreak;
                }
            }
            else
            {
                nEmptyCount = 0;
            }
        }

        // P1.5: when stdout data was read, poll process exit without cycle sleep;
        // when idle, use full cycle timeout (decouples wait from read chunk size).
        const DWORD dwProcessWait = (dwRead > 0) ? 0 : dwCycleTimeOut;
        DWORD dwWaitTimeout = dwProcessWait;

        if ( bUseTimeout )
        {
            const ULONGLONG now = ::GetTickCount64();
            if ( now >= tEnd )
            {
                bTimedOut = true;
                break;
            }

            const ULONGLONG rem = tEnd - now;
            if ( (ULONGLONG)dwWaitTimeout > rem )
                dwWaitTimeout = (DWORD) rem;
        }

        const DWORD dwWaitResult = [this, dwWaitTimeout]() -> DWORD
        {
            HANDLE hStdoutEvent = m_Io.getStdoutDataEvent();
            if ( hStdoutEvent )
            {
                HANDLE waitHandles[2] = { m_ProcessInfo.hProcess, hStdoutEvent };
                return ::WaitForMultipleObjects(2, waitHandles, FALSE, dwWaitTimeout);
            }
            return ::WaitForSingleObject(m_ProcessInfo.hProcess, dwWaitTimeout);
        }();

        if ( dwWaitResult == WAIT_OBJECT_0 )
            isConsoleProcessRunning = false;
        else
            isConsoleProcessRunning = IsProcessStillActive(m_ProcessInfo.hProcess);

        if ( bUseTimeout && ::GetTickCount64() >= tEnd )
        {
            if ( isConsoleProcessRunning )
                bTimedOut = true;
        }
    }
    while ( isConsoleProcessRunning
             && m_pScriptEngine->ContinueExecution() && !isBreaking() && !bTimedOut );

    if ( bTimedOut && isConsoleProcessRunning )
        return false;

    if ( (!m_pScriptEngine->ContinueExecution()) || isBreaking() )
    {
        if ( isConsoleProcessRunning )
        {
            // Stop I/O threads before kill — do not drain stdout while child is still alive.
            m_Io.stopStdoutReader();
            m_Io.stopStdinWriter();

            int nKillMethods = 0;
            CProcessKiller::eKillMethod arrKillMethods[4];

            tstr sAppName;
            CListT<tstr> ArgsList;
            if ( StrSplitToArgs(m_sCmdLine.c_str(), ArgsList, 2) > 0 )
            {
                sAppName = ArgsList.GetFirst()->GetItem();
                NppExecHelpers::StrLower(sAppName);
            }

            if ( (sAppName == _T("cmd")) || (sAppName == _T("cmd.exe")) )
            {
                // cmd can't be closed with Ctrl-Break for unknown reason...
            }
            else
            {
                if ( m_nBreakMethod == CProcessKiller::killCtrlC )
                    arrKillMethods[nKillMethods++] = CProcessKiller::killCtrlC;
                else
                    arrKillMethods[nKillMethods++] = CProcessKiller::killCtrlBreak;
            }
            arrKillMethods[nKillMethods++] = CProcessKiller::killWmClose;

            Runtime::GetLogger().AddEx( _T("; trying to kill the child process... (instance = %s)"), GetInstanceStr() );

            unsigned int nWaitTimeout = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_KILLTIMEOUT_MS);
            CProcessKiller::eKillMethod nSucceededKillMethod = CProcessKiller::killNone;
            if ( Kill(arrKillMethods, nKillMethods, nWaitTimeout, &nSucceededKillMethod) )
            {
                Runtime::GetLogger().AddEx( _T("; the child process has been killed (instance = %s)"), GetInstanceStr() );

                if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
                {
                    tstr Msg;
                    Msg.Format( 80, _T("<<< Process has been killed (PID=%d)"), m_ProcessInfo.dwProcessId );
                    switch ( nSucceededKillMethod )
                    {
                        case CProcessKiller::killCtrlBreak:
                            Msg += _T(" with Ctrl-Break");
                            break;
                        case CProcessKiller::killCtrlC:
                            Msg += _T(" with Ctrl-C");
                            break;
                        case CProcessKiller::killWmClose:
                            Msg += _T(" with WM_CLOSE");
                            break;
                    }
                    Msg += _T('.');
                    const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                    m_pNppExec->GetConsole().PrintMessage( Msg.c_str(), nPrintMsgFlags );
                }
                else
                {
                    if (m_nPrevState != nlLF /* new line */ )
                    {
                        const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                        m_pNppExec->GetConsole().PrintMessage( _T(""), nPrintMsgFlags );
                    }
                }
            }
            else
            {
                Runtime::GetLogger().AddEx( _T("; trying to terminate the child process... (instance = %s)"), GetInstanceStr() );

                if ( ::TerminateProcess(m_ProcessInfo.hProcess, 0) )
                {
                    Runtime::GetLogger().AddEx( _T("; the child process has been terminated (instance = %s)"), GetInstanceStr() );

                    if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
                    {
                        const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                        m_pNppExec->GetConsole().PrintMessage( tstr().Format(80, _T("<<< Process has been terminated (PID=%d)."), m_ProcessInfo.dwProcessId), nPrintMsgFlags );
                    }
                    else
                    {
                        if ( m_nPrevState != nlLF /* new line */ )
                        {
                            const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                            m_pNppExec->GetConsole().PrintMessage( _T(""), nPrintMsgFlags );
                        }
                    }
                }
                else
                {
                    m_pNppExec->GetConsole().PrintError( tstr().Format(80, _T("<<< TerminateProcess() returned FALSE (PID=%d)."), m_ProcessInfo.dwProcessId) );
                }
            }
        }
    }

    // P1.4: drain stdout only after the child process has terminated (not while it is waiting on stdin).
    if ( m_pScriptEngine->ContinueExecution() && !m_pScriptEngine->GetTriedExitCmd() )
    {
        if ( !IsProcessStillActive(m_ProcessInfo.hProcess) )
        {
            readPipesAndOutput(m_bufLine, m_bPrevLineEmpty, m_nPrevState, true, m_bDoOutputNext,
                               m_bufLineStderr, m_bPrevLineEmptyStderr, m_nPrevStateStderr, m_bDoOutputNextStderr);
        }
    }

    DWORD dwExitCode = (DWORD)(-1);
    ::GetExitCodeProcess(m_ProcessInfo.hProcess, &dwExitCode);
    m_nExitCode = (int) dwExitCode;

    // Process cleanup (reader usually stopped in final readPipesAndOutput or kill path above)
    ::CloseHandle(m_ProcessInfo.hProcess); m_ProcessInfo.hProcess = NULL;
    m_Io.stopStdoutReader();
    m_Io.stopStdinWriter();
    m_Io.closeAll();

    if ( m_hJob != NULL )
    {
        ::CloseHandle(m_hJob);
        m_hJob = NULL;
    }

    if ( m_pScriptEngine->ContinueExecution() && !isBreaking() )
    {
        if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
        {
            const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
            m_pNppExec->GetConsole().PrintMessage( tstr().Format(100, _T("<<< Process finished (PID=%u). (Exit code %d)"), m_ProcessInfo.dwProcessId, m_nExitCode), nPrintMsgFlags );
        }
        else
        {
            if ( m_nPrevState != nlLF /* new line */ )
            {
                const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                m_pNppExec->GetConsole().PrintMessage( _T(""), nPrintMsgFlags );
            }
        }
    }

    return true;
}

void CChildProcess::reset()
{
    m_strOutput.Clear();
    m_sCmdLine.Clear();
    m_nExitCode = -1;
    m_nBreakMethod = CProcessKiller::killNone;
    m_hJob = NULL;
    m_Io.reset();
    ::ZeroMemory(&m_ProcessInfo, sizeof(PROCESS_INFORMATION));

    m_bufLine.Clear();
    m_bPrevLineEmpty = false;
    m_nPrevState = nlNone;
    m_bDoOutputNext = true;

    m_bufLineStderr.Clear();
    m_bPrevLineEmptyStderr = false;
    m_nPrevStateStderr = nlNone;
    m_bDoOutputNextStderr = true;

    m_VtParserStdout.Reset();
    m_VtParserStderr.Reset();
}

bool CChildProcess::isBreaking() const
{
    return (m_nBreakMethod != CProcessKiller::killNone);
}

bool CChildProcess::applyOutputFilters(const tstr& _line, bool bOutput)
{
    const bool bConFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE);
    const int  nConFltrInclMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_INCLMASK);
    const int  nConFltrExclMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_EXCLMASK);

    tstr _mask;
    tstr sLine;

    // >>> console output filters
    if ( bConFltrEnable && ((nConFltrInclMask > 0) || (nConFltrExclMask > 0)) )
    {
        for ( int i = 0; bOutput && 
               (i < CConsoleOutputFilterDlg::FILTER_ITEMS); i++ )
        {
            int len = 0;
            const TCHAR* cszLine = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_INCLLINE1 + i, &len);
            sLine = cszLine;
            if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sLine, false) )
            {
                cszLine = sLine.c_str();
                len = sLine.length();
            }

            if ( (nConFltrInclMask & (0x01 << i)) && (len > 0) )
            {
                _mask = cszLine;
                NppExecHelpers::StrLower(_mask);
                if ( !c_base::_tmatch_mask(_mask.c_str(), _line.c_str()) )
                {
                    bOutput = false;
                }
            }

            len = 0;
            cszLine = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_EXCLLINE1 + i, &len);
            sLine = cszLine;
            if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sLine, false) )
            {
                cszLine = sLine.c_str();
                len = sLine.length();
            }

            if ( bOutput && (nConFltrExclMask & (0x01 << i)) && (len > 0) )
            {
                _mask = cszLine;
                NppExecHelpers::StrLower(_mask);
                if ( c_base::_tmatch_mask(_mask.c_str(), _line.c_str()) )
                {
                    bOutput = false;
                }
            }
        }
    }
    // <<< console output filters

    return bOutput;
}

bool CChildProcess::applyReplaceFilters(tstr& _line, tstr& printLine, bool bOutput)
{
    const bool bRplcFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_ENABLE);
    const bool bRplcFltrExclEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_EXCLEMPTY);
    const int  nRplcFltrFindMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_R_FINDMASK);
    const int  nRplcFltrCaseMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_R_CASEMASK);

    tstr _mask;
    tstr sFind;
    tstr sRplc;

    // >>> console replace filters
    if ( bOutput && bRplcFltrEnable && (nRplcFltrFindMask > 0) )
    {
        bool bModified = false;
        
        // >>> for ...
        for ( int i = 0; bOutput && 
               (i < CConsoleOutputFilterDlg::REPLACE_ITEMS); i++ )
        {
            int lenFind = 0;
            const TCHAR* cszFind = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_R_FIND1 + i, &lenFind);
            sFind = cszFind;
            if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sFind, false) )
            {
                cszFind = sFind.c_str();
                lenFind = sFind.length();
            }

            if ( (nRplcFltrFindMask & (0x01 << i)) && 
                 ( ((lenFind > 0) && (_line.length() > 0)) || 
                   ((lenFind == 0) && (_line.length() == 0)) )
               )
            {
                int lenRplc = 0;
                const TCHAR* cszRplc = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_R_RPLC1 + i, &lenRplc);
                sRplc = cszRplc;
                if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sRplc, false) )
                {
                    cszRplc = sRplc.c_str();
                    lenRplc = sRplc.length();
                }

                if ( lenFind > 0 )
                {
                    // original string is not empty
                    
                    int pos = 0;
                    if ( nRplcFltrCaseMask & (0x01 << i) )
                    {
                        // match case
                        while ( (pos = printLine.Find(cszFind, pos)) >= 0 )
                        {
                            bModified = true;

                            // both variables must be changed to stay synchronized
                            _line.Replace(pos, lenFind, cszRplc, lenRplc);
                            printLine.Replace(pos, lenFind, cszRplc, lenRplc);
                            pos += lenRplc;
                        }
                    }
                    else
                    {
                        // case-insensitive
                        _mask = cszFind;
                        NppExecHelpers::StrLower(_mask);
                        while ( (pos = _line.Find(_mask.c_str(), pos)) >= 0 )
                        {
                            bModified = true;

                            // both variables must be changed to stay synchronized
                            _line.Replace(pos, lenFind, cszRplc, lenRplc);
                            printLine.Replace(pos, lenFind, cszRplc, lenRplc);
                            pos += lenRplc;
                        }
                    }
                }
                else
                {
                    // replacing original empty string with cszRplc

                    bModified = true;
                    _line = cszRplc;
                    printLine = cszRplc;
                }

                if ( bRplcFltrExclEmpty && bModified && (printLine.length() == 0) )
                {
                    bOutput = false;
                }
            }
        }
        // <<< for ...
        
    }
    // <<< console replace filters

    return bOutput;
}

DWORD CChildProcess::readPipesAndOutput(CStrT<char>& bufLine,
                                        bool& bPrevLineEmpty,
                                        int&  nPrevState,
                                        bool  bOutputAll,
                                        bool& bDoOutputNext,
                                        CStrT<char>& bufLineStderr,
                                        bool& bPrevLineEmptyStderr,
                                        int&  nPrevStateStderr,
                                        bool& bDoOutputNextStderr)
{
    if ( bOutputAll )
        m_Io.stopStdoutReader();
    else if ( !m_Io.isStdoutReaderActive() )
        ::Sleep(10);

    const DWORD dwRead = readPipesDirect(bufLine, bPrevLineEmpty, nPrevState, bOutputAll, bDoOutputNext,
                                         bufLineStderr, bPrevLineEmptyStderr, nPrevStateStderr, bDoOutputNextStderr);

    // Partial-line flush for pipe-mode script-thread reads only (reader thread owns ConPTY).
    if ( !bOutputAll && IsRunning() && !m_Io.isStdoutReaderActive() )
    {
        if ( bufLine.length() > 0 )
        {
            const char c = bufLine[bufLine.length() - 1];
            if ( (c != '\r') && (c != '\n') && (c >= 32) )
            {
                processOutputBuffer(bufLine, bPrevLineEmpty, nPrevState, true, bDoOutputNext,
                                    false, false);
            }
        }

        if ( m_Io.hasSeparateStderrPipe() && (bufLineStderr.length() > 0) )
        {
            const char c = bufLineStderr[bufLineStderr.length() - 1];
            if ( (c != '\r') && (c != '\n') && (c >= 32) )
            {
                processOutputBuffer(bufLineStderr, bPrevLineEmptyStderr, nPrevStateStderr, true, bDoOutputNextStderr,
                                    false, true);
            }
        }
    }

    return dwRead;
}

DWORD CChildProcess::readOnePipeDirect(HANDLE hReadPipe,
                                     bool  bOutputAll,
                                     bool  bFromStderr)
{
    DWORD dwBytesRead = 0;
    char  Buf[CONSOLEPIPE_BUFSIZE];
    bool bSomethingHasBeenReadFromThePipe = false;

    if ( !hReadPipe )
    {
        return 0;
    }

    do
    {
        dwBytesRead = 0;
        if ( !::PeekNamedPipe(hReadPipe, NULL, 0, NULL, &dwBytesRead, NULL) )
        {
            dwBytesRead = 0;
        }

        bool bOutputThisPass = bOutputAll;
        if ( !dwBytesRead )
        {
            if ( !bSomethingHasBeenReadFromThePipe )
                bOutputThisPass = true;
        }

        if ( (dwBytesRead > 0) || bOutputThisPass )
        {
            bool bContainsData = (dwBytesRead > 0) ? true : false;

            if ( bContainsData )
                ::ZeroMemory(Buf, CONSOLEPIPE_BUFSIZE);

            DWORD dwRead = 0;
            if ( (bContainsData
                  && ::ReadFile(hReadPipe, Buf, (CONSOLEPIPE_BUFSIZE-1)*sizeof(char), &dwRead, NULL)
                  && (dwRead > 0)) || bOutputThisPass )
            {
                if ( bContainsData )
                {
                    Buf[dwRead/sizeof(char)] = 0;
                    m_Io.enqueueOutputChunk(Buf, (int)(dwRead/sizeof(char)), bFromStderr);
                    dwBytesRead = dwRead;
                    bSomethingHasBeenReadFromThePipe = true;
                }
                else
                {
                    dwBytesRead = 0;
                }
            }
        }

    }
    while ( (dwBytesRead > 0) && m_pScriptEngine->ContinueExecution() && !isBreaking() );

    return dwBytesRead;
}

DWORD CChildProcess::processOutputQueue(CStrT<char>& bufLine,
                                        bool& bPrevLineEmpty,
                                        int&  nPrevState,
                                        bool& bDoOutputNext,
                                        CStrT<char>& bufLineStderr,
                                        bool& bPrevLineEmptyStderr,
                                        int&  nPrevStateStderr,
                                        bool& bDoOutputNextStderr,
                                        bool  bOutputAll,
                                        bool  bReadStdout,
                                        bool  bReadStderr)
{
    DWORD dwTotal = 0;
    DWORD dwTotalStderr = 0;
    CStrT<char> chunk;
    bool bStderr = false;

    while ( m_Io.popOutputChunk(chunk, bStderr) )
    {
        if ( bStderr )
        {
            bufLineStderr.Append(chunk);
            processOutputBuffer(bufLineStderr, bPrevLineEmptyStderr, nPrevStateStderr, false, bDoOutputNextStderr, true, true);
            dwTotalStderr += (DWORD) chunk.length();
        }
        else
        {
            bufLine.Append(chunk);
            processOutputBuffer(bufLine, bPrevLineEmpty, nPrevState, false, bDoOutputNext, true, false);
            dwTotal += (DWORD) chunk.length();
        }
    }

    // Complete (newline-terminated) lines are already emitted above. Do NOT force-flush
    // an incomplete trailing line while a burst is in progress (dwTotal > 0): the child
    // is still writing, and flushing now would split the line at an arbitrary ConPTY
    // chunk boundary (dropping the pending newline / indentation). Flush the partial
    // tail only on the final drain (bOutputAll), or when the child paused output (idle
    // poll: no data this call) — e.g. an interactive prompt awaiting input.
    if ( bOutputAll || (dwTotal == 0) )
        processOutputBuffer(bufLine, bPrevLineEmpty, nPrevState, true, bDoOutputNext, true, false);

    if ( m_Io.hasSeparateStderrPipe() && (bOutputAll || (dwTotalStderr == 0)) )
        processOutputBuffer(bufLineStderr, bPrevLineEmptyStderr, nPrevStateStderr, true, bDoOutputNextStderr, true, true);

    return dwTotal + dwTotalStderr;
}

DWORD CChildProcess::readPipesDirect(CStrT<char>& bufLine,
                                     bool& bPrevLineEmpty,
                                     int&  nPrevState,
                                     bool  bOutputAll,
                                     bool& bDoOutputNext,
                                     CStrT<char>& bufLineStderr,
                                     bool& bPrevLineEmptyStderr,
                                     int&  nPrevStateStderr,
                                     bool& bDoOutputNextStderr)
{
    bool bReadStdout = false;
    bool bReadStderr = false;

    // When the ConPTY reader thread is active it owns blocking ReadFile on stdout.
    if ( !m_Io.isStdoutReaderActive() )
    {
        bReadStdout = (readOnePipeDirect(m_Io.getStdOutReadPipe(), bOutputAll, false) > 0);

        if ( m_Io.hasSeparateStderrPipe() )
            bReadStderr = (readOnePipeDirect(m_Io.getStdErrReadPipe(), bOutputAll, true) > 0);
    }

    return processOutputQueue(bufLine, bPrevLineEmpty, nPrevState, bDoOutputNext,
                              bufLineStderr, bPrevLineEmptyStderr, nPrevStateStderr, bDoOutputNextStderr,
                              bOutputAll, bReadStdout, bReadStderr);
}

DWORD CChildProcess::processOutputBuffer(CStrT<char>& bufLine,
                                         bool& bPrevLineEmpty,
                                         int&  nPrevState,
                                         bool  bFlushPartialLine,
                                         bool& bDoOutputNext,
                                         bool  bSomethingHasBeenReadFromPipe,
                                         bool  bFromStderr)
{
    CStrT<char> outLine;

    const bool bCondenseEmptyLines = m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_CONDENSEEMPTYLINES);
    const bool bConFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE);
    const bool bConFltrExclAllEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_EXCLALLEMPTY);
    const bool bConFltrExclDupEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_EXCLDUPEMPTY);
    const bool bOutputVar = m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SETOUTPUTVAR);
    const unsigned int nAnsiEscSeq = GetAnsiEscSeq();

    bool bOutputAll = bFlushPartialLine;
    if ( !bSomethingHasBeenReadFromPipe )
        bOutputAll = true;

    // A '\r' as the last byte of the current buffer may be the first half of a
    // "\r\n" that got split across pipe-read chunks (common with ConPTY, whose
    // rows are all "\r\n"-terminated). While the child may still produce output,
    // hold such a trailing '\r' for the next read instead of treating it as a
    // bare CR — otherwise the line's newline is lost and the following chunk's
    // text concatenates onto this line (and ProcessSlashR mangles indentation).
    const bool bDeferSplitCR = IsRunning();

    int copy_len;

    do {

        // t+ h-: backspaces that pull an nppexec: line out of the child's
        // buffer. They must not erase that line on screen.
        if ( m_pNppExec->GetConsole().ConsumeConPtyCancelEcho(bufLine, bOutputAll) )
            return 0;

        copy_len = -1;

        for ( int pos = 0; pos < bufLine.length(); pos++ )
        {
            int nIsNewLine = nlNone;
            if ( bufLine[pos] == '\n' )
            {
                nIsNewLine = nlLF;
            }
            else if ( bufLine[pos] == '\r' )
            {
                // Incomplete when we can't look far enough ahead to tell "\r\n"
                // (or "\r\r\n") from a bare CR: defer while more data may arrive.
                const bool bTrailingCR =
                    ( pos + 1 >= bufLine.length() ) ||
                    ( (bufLine[pos+1] == '\r') && (pos + 2 >= bufLine.length()) );
                if ( bTrailingCR && bDeferSplitCR )
                {
                    break; // keep "...\r" buffered for the next read
                }

                if ( bufLine[pos+1] != '\n' )
                {
                    if ( (bufLine[pos+1] != '\r') || (bufLine.GetAt(pos+2) != '\n') )
                    {
                        nIsNewLine = nlCR;
                    }
                }
            }
            else if ( bufLine[pos] == '\b' )
            {
                nIsNewLine = nlBS;
            }

            if ( nIsNewLine || (bOutputAll && (pos == bufLine.length()-1)) )
            {
                int nConPtyBlotSpaces = 0;
                copy_len = pos;
                if ( !nIsNewLine )
                {
                    copy_len++;
                }
                else if ( (pos > 0) && (bufLine[pos-1] == '\r') )
                {
                    copy_len--;
                    if ( (pos > 1) && (bufLine[pos-2] == '\r') )
                        copy_len--;
                }

                outLine.Assign(bufLine.c_str(), copy_len);

                if ( nIsNewLine == nlBS )
                {
                    while ( bufLine[pos+1] == '\b' )
                    {
                        ++nIsNewLine;
                        ++pos;
                    }
                }

                bufLine.Delete(0, pos+1);
                if ( (copy_len > 0) || (!bCondenseEmptyLines) ||
                     ( ((!bConFltrExclAllEmpty) || (!bConFltrEnable)) &&
                       ((!bPrevLineEmpty) || (!bConFltrEnable) || (!bConFltrExclDupEmpty))
                     ) )
                {
                    tstr printLine;
                    bool bOutput = bConFltrEnable ? bDoOutputNext : true;

                    if ( bOutput )
                    {
                        tstr _line;

                        if ( outLine.length() > 0 )
                        {
                            unsigned int enc = CConsoleEncodingDlg::getOutputEncoding(GetEncoding());

                          #ifdef UNICODE

                            switch ( enc )
                            {
                                case CConsoleEncodingDlg::ENC_OEM :
                                    _line = NppExecHelpers::CStrToWStr(outLine, CP_OEMCP);
                                    break;

                                case CConsoleEncodingDlg::ENC_UTF8 :
                                    _line = NppExecHelpers::CStrToWStr(outLine, CP_UTF8);
                                    break;

                                default:
                                    _line = NppExecHelpers::CStrToWStr(outLine, CP_ACP);
                                    break;
                            }

                            {
                                wchar_t wchNulChar = CNppConsoleRichEdit::GetNulChar();
                                if ( wchNulChar != 0 )
                                {
                                    _line.Replace( wchar_t(0x0000), wchNulChar );
                                }
                            }

                          #else

                            {
                                char chNulChar = CNppConsoleRichEdit::GetNulChar();
                                if ( chNulChar != 0 )
                                {
                                    outLine.Replace( char(0x00), chNulChar );
                                }
                            }

                            switch ( enc )
                            {
                                case CConsoleEncodingDlg::ENC_OEM :
                                    if ( _line.SetSize(outLine.length() + 1) )
                                    {
                                        ::OemToChar( outLine.c_str(), _line.c_str() );
                                        _line.CalculateLength();
                                    }
                                    break;

                                case CConsoleEncodingDlg::ENC_UTF8 :
                                    {
                                        char* pStr = SysUniConv::newUTF8ToMultiByte( outLine.c_str() );
                                        if ( pStr )
                                        {
                                            _line = pStr;
                                            delete [] pStr;
                                        }
                                    }
                                    break;

                                default:
                                    _line = outLine;
                                    break;
                            }

                          #endif

                            printLine = _line;
                            NppExecHelpers::StrLower(_line);
                        }

                        bOutput = applyOutputFilters(_line, bOutput);
                        bOutput = applyReplaceFilters(_line, printLine, bOutput);
                    }

                    if ( bOutput )
                    {
                        bool bPrintThisLine = true;
                        int nCursorColumnAfter = -1;
                        // For escProcess, ConPTY may pack several output rows into a single
                        // CR/LF-delimited chunk: it paints the initial visible screen by
                        // positioning each row with an absolute cursor move (CUP "ESC[r;cH")
                        // rather than "\r\n". The VT parser splits those into rows, each with
                        // its own color; a non-null pointer here means "iterate the rows".
                        std::vector<CConsoleVtParser::VtOutputRow>* pVtRows = nullptr;

                        if ( nAnsiEscSeq == escRemove )
                        {
                            bool bCursorNextLine = false;
                            bPrintThisLine = RemoveAnsiEscSequencesFromLine(printLine, &bCursorNextLine);

                            // Same ConPTY handling as the escProcess path: a CUP/
                            // CUD/CNL sequence used in place of "\r\n" advances to
                            // the next output row, so treat this line as newline-
                            // terminated instead of letting the next row concatenate.
                            if ( bCursorNextLine && (nIsNewLine != nlLF) )
                                nIsNewLine = nlLF;
                        }
                        else if ( nAnsiEscSeq == escProcess )
                        {
                            tstr vtOutput;
                            CConsoleVtParser& vtParser = bFromStderr ? m_VtParserStderr : m_VtParserStdout;
                            // Per-line VT decode (same scope as escRemove); do not carry parser
                            // state across line fragments — ConPTY splits sequences at newlines.
                            vtParser.Reset();
                            CNppExecConsole& console = m_pNppExec->GetConsole();
                            CNppExecCommandExecutor& cmdExec = m_pNppExec->GetCommandExecutor();
                            const bool bInPlaceVtCursor =
                                cmdExec.IsChildProcessPseudoCon() &&
                                cmdExec.IsChildProcessRawImmediateInput();
                            vtParser.SetInPlaceCursorMode(bInPlaceVtCursor);
                            vtParser.ProcessChunk(printLine, vtOutput);
                            printLine.Swap(vtOutput);

                            CConsoleVtParser::VtActions vtActions;
                            vtParser.ConsumeActions(vtActions);
                            pVtRows = &vtParser.GetOutputRows();

                            if ( vtActions.nCursorForward > 0 )
                            {
                                if ( m_pNppExec->GetCommandExecutor().IsChildProcessPseudoCon() )
                                {
                                    // CUF is a cursor move, not "insert N spaces at the append point".
                                    console.AdvanceConPtyColumn(vtActions.nCursorForward);
                                }
                                else
                                {
                                    // Pipe mode: no column grid — approximate indent with spaces.
                                    int n = vtActions.nCursorForward;
                                    if ( n > 4096 ) n = 4096;
                                    tstr spaces;
                                    spaces.Reserve(n);
                                    for ( int i = 0; i < n; ++i )
                                        spaces.Append( _T(' ') );
                                    printLine.Insert(0, spaces);
                                    if ( !pVtRows->empty() )
                                        (*pVtRows)[0].text.Insert(0, spaces);
                                }
                            }

                            const bool bApplyDestructiveVt =
                                cmdExec.IsChildProcessPseudoCon() ||
                                !console.HasUnlockedConsoleInput();

                            if ( vtActions.nSetCursorColumn >= 0 && bInPlaceVtCursor )
                            {
                                console.SetConPtyColumn(vtActions.nSetCursorColumn);
                                if ( printLine.length() > 0 )
                                {
                                    // History or Delete: replacement text follows.
                                    // Drop the previous command before it is written.
                                    console.VtEraseLineInOutputRegion();
                                }
                                else
                                {
                                    // Home is a CUP with no text. Erasing here would
                                    // wipe the command; only the caret moves.
                                    console.PlaceConPtyCaret();
                                }
                                vtActions.bCursorNextLine = false;
                            }
                            if ( bInPlaceVtCursor )
                                nCursorColumnAfter = vtActions.nCursorColumnAfter;

                            if ( vtActions.bClearScreen && bApplyDestructiveVt )
                                console.VtEraseScreenInOutputRegion();
                            if ( vtActions.bClearLine && bApplyDestructiveVt )
                                console.VtEraseLineInOutputRegion();
                            if ( vtActions.nEraseCharCount > 0 && bApplyDestructiveVt )
                                console.VtEraseCharsInOutputRegion(vtActions.nEraseCharCount);
                            if ( vtActions.nBackspaceCount > 0 && bApplyDestructiveVt )
                            {
                                // CSI D is cursor-left, the same move as a lone "\b".
                                if ( bInPlaceVtCursor )
                                    console.ProcessSlashB(vtActions.nBackspaceCount);
                                else
                                    console.VtBackspaceInOutputRegion(vtActions.nBackspaceCount);
                            }

                            // ConPTY advanced to the next row via an absolute cursor-position
                            // (CUP) or cursor-down/next-line sequence instead of "\r\n" with no
                            // text following it (e.g. the frame flush when git sets the window
                            // title). Treat this line as newline-terminated so the following row
                            // starts fresh; and drop the bare-CR erase that would otherwise fire.
                            if ( vtActions.bCursorNextLine && (nIsNewLine != nlLF) )
                                nIsNewLine = nlLF;

                            // cmd post-echo spacer: bare ECH + CRLF with no visible text.
                            if ( (printLine.length() == 0) && (vtActions.nEraseCharCount > 0) &&
                                 (nIsNewLine == nlLF) && cmdExec.IsChildProcessPseudoCon() )
                            {
                                nIsNewLine = nlNone;
                            }

                            // Match escRemove: emit blank lines (e.g. \\n after echoed input),
                            // but skip lines that were only VT sequences.
                            bPrintThisLine = (printLine.length() > 0) || (outLine.length() == 0);
                        }

                        if ( nIsNewLine == nlCR )
                        {
                            m_pNppExec->GetConsole().ProcessSlashR();
                        }
                        else if ( nPrevState >= nlBS )
                        {
                            m_pNppExec->GetConsole().ProcessSlashB( (nPrevState - nlBS) + 1 );
                            nPrevState = nlNone;
                        }

                        if ( bPrintThisLine )
                        {
                            UINT nPrintOutFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfFromChild;
                            if ( nIsNewLine == nlLF )
                                nPrintOutFlags |= CNppExecConsole::pfNewLine;

                            if ( pVtRows && (pVtRows->size() > 1) )
                            {
                                // A ConPTY initial-screen paint collapsed several rows into this
                                // chunk. Emit each on its own console line (all but the last are
                                // unconditionally newline-terminated), each with its own color.
                                CNppExecConsole& console = m_pNppExec->GetConsole();
                                const std::vector<CConsoleVtParser::VtOutputRow>& rows = *pVtRows;
                                const size_t nRows = rows.size();
                                for ( size_t i = 0; i < nRows; ++i )
                                {
                                    const CConsoleVtParser::VtOutputRow& row = rows[i];
                                    const bool bLastRow = (i + 1 == nRows);
                                    UINT nRowFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfFromChild;
                                    if ( !bLastRow || (nIsNewLine == nlLF) )
                                        nRowFlags |= CNppExecConsole::pfNewLine;

                                    if ( bOutputVar )
                                    {
                                        m_strOutput += row.text;
                                        if ( nRowFlags & CNppExecConsole::pfNewLine )
                                            m_strOutput += _T("\n");
                                    }

                                    if ( row.bHasColor )
                                    {
                                        const COLORREF oldColor = console.GetCurrentColorTextNorm();
                                        console.SetCurrentColorTextNorm(row.color);
                                        console.PrintOutput(row.text.c_str(), nRowFlags);
                                        console.SetCurrentColorTextNorm(oldColor);
                                    }
                                    else
                                    {
                                        console.PrintOutput(row.text.c_str(), nRowFlags);
                                    }
                                }
                            }
                            else
                            {
                                if ( bOutputVar )
                                {
                                    m_strOutput += printLine;
                                    if ( nIsNewLine == nlLF )
                                    {
                                        m_strOutput += _T("\n");
                                    }
                                }

                                if ( pVtRows && (pVtRows->size() == 1) && (*pVtRows)[0].bHasColor )
                                {
                                    CNppExecConsole& console = m_pNppExec->GetConsole();
                                    const COLORREF oldColor = console.GetCurrentColorTextNorm();
                                    console.SetCurrentColorTextNorm((*pVtRows)[0].color);
                                    console.PrintOutput(printLine.c_str(), nPrintOutFlags);
                                    console.SetCurrentColorTextNorm(oldColor);
                                }
                                else
                                {
                                    m_pNppExec->GetConsole().PrintOutput(printLine.c_str(), nPrintOutFlags);
                                }
                            }
                        }

                        if ( nCursorColumnAfter >= 0 )
                        {
                            // Shorter history entry parks in the padding it used to
                            // cover the longer command. Mid-line redraw parks before
                            // the text it kept and leaves that text on screen.
                            m_pNppExec->GetConsole().ParkConPtyCursor(nCursorColumnAfter);
                        }

                        if ( bPrintThisLine && (nIsNewLine >= nlBS) &&
                             m_pNppExec->GetCommandExecutor().IsChildProcessPseudoCon() &&
                             m_pNppExec->GetCommandExecutor().IsChildProcessRawImmediateInput() )
                        {
                            const TCHAR* psz = printLine.c_str();
                            int n = (int) printLine.length();
                            while ( (n > 0) && (psz[n - 1] == _T(' ')) )
                                --n;
                            nConPtyBlotSpaces = (int) printLine.length() - n;
                        }
                    }

                    bDoOutputNext = bOutput;
                }
                bPrevLineEmpty = (copy_len > 0) ? false : true;
                // cmd erases a cell as "\b \b" (cursor left, space, cursor left).
                // A lone "\b" is Arrow Left: move the caret, leave the character.
                // The space in "\b \b" (and the space before a history BS) is
                // nConPtyBlotSpaces and is removed after the cursor steps onto it.
                if ( nIsNewLine >= nlBS )
                {
                    CNppExecConsole& console = m_pNppExec->GetConsole();
                    console.ProcessSlashB( (nIsNewLine - nlBS) + 1 );
                    if ( nConPtyBlotSpaces > 0 )
                        console.TrimConPtyBlotSpaces(nConPtyBlotSpaces);
                    nPrevState = nlNone;
                }
                else
                {
                    nPrevState = nIsNewLine;
                }
                if ( nIsNewLine == nlLF )
                {
                    bDoOutputNext = true;
                }
                break;
            }

        }
    } while ( copy_len >= 0 );

    return 0;
}
bool CChildProcess::RemoveAnsiEscSequencesFromLine(tstr& Line, bool* pbCursorNextLine)
{
    // ANSI escape codes, references:
    //   https://en.wikipedia.org/wiki/ANSI_escape_code
    //   https://en.wikipedia.org/wiki/ISO/IEC_2022
    //   https://man7.org/linux/man-pages/man4/console_codes.4.html
    //   http://ascii-table.com/ansi-escape-sequences.php
    //   http://ascii-table.com/ansi-escape-sequences-vt-100.php
    //   https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
    //   https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences

    enum eEscState {
        esNone = 0,
        esEsc,    // ESC symbol found
        esCsi,    // CSI sequence
        esOsc,    // OSC sequence
        esWaitSt, // wait for ST (ESC \)
        esWait1,  // wait for 1 symbol
        esWait2   // wait for 2 symbols
    };

    
    const TCHAR* p = Line.c_str();
    eEscState state = esNone;
    TCHAR curr_ch = 0;
    TCHAR wait1_ch = 0;
    TCHAR wait2_ch = 0;
    tstr outputLine;
    tstr csiParams;

    outputLine.Reserve(Line.length());

    if ( pbCursorNextLine )
        *pbCursorNextLine = false;

    while ( (curr_ch = *p) != 0 )
    {
        switch ( state )
        {
            case esNone:
                if ( curr_ch == 0x1B )  // ESC
                {
                    state = esEsc;
                }
                else
                {
                    outputLine.Append(curr_ch);
                }
                break;

            case esEsc:
                switch ( curr_ch )
                {
                    case _T('['):  // CSI
                        state = esCsi;
                        csiParams.Clear();
                        break;
                    case _T(']'):  // OSC
                        state = esOsc;
                        break;
                    case _T('P'):  // DCS
                    case _T('X'):  // SOS
                    case _T('^'):  // PM
                    case _T('_'):  // APC
                        state = esWaitSt;
                        break;
                    case _T('$'):  // G?DM?
                        state = esWait2;
                        wait2_ch = curr_ch;
                        break;
                    case _T('!'):  // C0-designate
                    case _T('"'):  // C1-designate
                    case _T('#'):  // single control function
                    case _T('%'):  // DOCS
                    case _T('&'):  // IRR
                    case _T('('):  // G0 character set
                    case _T(')'):  // G1 character set
                    case _T('*'):  // G2 character set
                    case _T('+'):  // G3 character set
                    case _T('-'):  // G1 character set, VT300
                    case _T('.'):  // G2 character set, VT300
                    case _T('/'):  // G3 character set, VT300
                    case _T(' '):  // ACS
                        state = esWait1;
                        wait1_ch = curr_ch;
                        break;
                    default:       // RIS, IND, NEL, HTS, RI, ...
                        state = esNone;
                        break;
                }
                break;

            case esCsi:
                if ( (curr_ch >= _T('0') && curr_ch <= _T('9')) ||
                     (curr_ch == _T(';')) ||
                     (curr_ch == _T('?')) )
                {
                    // CSI parameter byte
                    csiParams.Append(curr_ch);
                }
                else if ( (curr_ch >= _T('A') && curr_ch <= _T('Z')) ||
                     (curr_ch >= _T('a') && curr_ch <= _T('z')) ||
                     (curr_ch == _T('@'))  ||
                     (curr_ch == _T('['))  ||
                     (curr_ch == _T('\\')) ||
                     (curr_ch == _T(']'))  ||
                     (curr_ch == _T('^'))  ||
                     (curr_ch == _T('_'))  ||
                     (curr_ch == _T('`'))  ||
                     (curr_ch == _T('{'))  ||
                     (curr_ch == _T('|'))  ||
                     (curr_ch == _T('}'))  ||
                     (curr_ch == _T('~')) )
                {
                    // the "final byte" of the CSI sequence.
                    // ConPTY sometimes uses cursor-movement CSIs in place of
                    // literal spaces / "\r\n"; mirror the escProcess (VT) path so
                    // stripped output keeps its indentation and row breaks.
                    if ( curr_ch == _T('C') ) // CUF (cursor forward) -> N spaces
                    {
                        int n = csiParams.IsEmpty() ? 1 : c_base::_tstr2int(csiParams.c_str());
                        if ( n < 0 )
                            n = 0;
                        else if ( n > 4096 ) // guard against absurd/malformed counts
                            n = 4096;
                        for ( int i = 0; i < n; ++i )
                            outputLine.Append( _T(' ') );
                    }
                    else if ( pbCursorNextLine )
                    {
                        if ( (curr_ch == _T('B')) || (curr_ch == _T('E')) ) // CUD / CNL
                            *pbCursorNextLine = true;
                        else if ( ((curr_ch == _T('H')) || (curr_ch == _T('f'))) &&
                                  (!csiParams.IsEmpty()) ) // CUP / HVP with params
                            *pbCursorNextLine = true;
                    }
                    csiParams.Clear();
                    state = esNone;
                }
                // else (intermediate byte) waiting for the CSI final character...
                break;

            case esOsc:
            case esWaitSt:
                if ( curr_ch == 0x1B )  // ESC
                {
                    const TCHAR next_ch = *(p + 1);
                    if ( next_ch == _T('\\') )  // ST
                    {
                        ++p; // skipping the next character as well
                        state = esNone;
                    }
                    else // ?
                    {
                        state = esEsc; // ?
                    }
                }
                // else waiting for the ST...
                break;

            case esWait1:
                state = esNone;
                if ( wait1_ch == _T('%') )
                {
                    switch ( curr_ch )
                    {
                        case _T('/'):  // ESC % / F
                            state = esWait1;
                            break;
                    }
                }
                wait1_ch = 0;
                break;

            case esWait2:
                state = esWait1;
                if ( wait2_ch == _T('$') )
                {
                    switch ( curr_ch )
                    {
                        case _T('@'):  // ESC $ @
                        case _T('A'):  // ESC $ A
                        case _T('B'):  // ESC $ B
                            state = esNone;
                            break;
                    }
                }
                wait2_ch = 0;
                break;
        }

        ++p;
    }

    Line.Swap(outputLine);
    return (!Line.IsEmpty() || outputLine.IsEmpty());
}

void CChildProcess::MustBreak(unsigned int nBreakMethod)
{
    m_nBreakMethod = nBreakMethod;
}

bool CChildProcess::IsRunning() const
{
    if ( !m_ProcessInfo.hProcess )
        return false;

    DWORD dwExitCode = STILL_ACTIVE;
    if ( !::GetExitCodeProcess(m_ProcessInfo.hProcess, &dwExitCode) )
        return false;

    return (dwExitCode == STILL_ACTIVE);
}

void CChildProcess::Stop()
{
    // Request the poll loop to stop and let WaitForExit() take the existing kill path.
    MustBreak(CProcessKiller::killCtrlBreak);
}

bool CChildProcess::Kill(const CProcessKiller::eKillMethod arrKillMethods[], int nKillMethods,
                         unsigned int nWaitTimeout,
                         CProcessKiller::eKillMethod* pnSucceededKillMethod)
{
    if ( m_Io.isPseudoCon() )
    {
        // ConPTY: CProcessKiller::KillByConsoleCtrlEvent attaches this GUI process to the
        // child's pseudo console, then GenerateConsoleCtrlEvent(CTRL_BREAK). Unlike Ctrl+C,
        // Ctrl+Break cannot be ignored via SetConsoleCtrlHandler — the host receives it and
        // Notepad++'s main window disappears while notepad++.exe keeps running.
        // ClosePseudoConsole terminates processes attached to the PTY instead.
        for ( int i = 0; i < nKillMethods; i++ )
        {
            if ( (arrKillMethods[i] == CProcessKiller::killCtrlC)
                    || (arrKillMethods[i] == CProcessKiller::killCtrlBreak) )
            {
                HANDLE hIn = m_Io.getStdInWritePipe();
                if ( hIn )
                {
                    const char ch = '\x03';
                    DWORD nWritten = 0;
                    ::WriteFile(hIn, &ch, 1, &nWritten, NULL);
                }
            }
        }

        const DWORD dwGrace = (nWaitTimeout > 300u) ? 300u : nWaitTimeout;
        if ( dwGrace > 0 )
        {
            if ( ::WaitForSingleObject(m_ProcessInfo.hProcess, dwGrace) == WAIT_OBJECT_0 )
            {
                if ( pnSucceededKillMethod )
                    *pnSucceededKillMethod = CProcessKiller::killCtrlBreak;
                return true;
            }
        }

        m_Io.closePseudoConsole();

        if ( ::WaitForSingleObject(m_ProcessInfo.hProcess, nWaitTimeout) == WAIT_OBJECT_0 )
        {
            if ( pnSucceededKillMethod )
                *pnSucceededKillMethod = CProcessKiller::killCtrlBreak;
            return true;
        }

        if ( ::TerminateProcess(m_ProcessInfo.hProcess, 1) )
        {
            ::WaitForSingleObject(m_ProcessInfo.hProcess, nWaitTimeout);
            return true;
        }
        return false;
    }

    bool isKilled = false;
    CProcessKiller pk(GetProcessInfo());
    for ( int i = 0; i < nKillMethods; i++ )
    {
        CProcessKiller::eKillMethod nKillMethod = arrKillMethods[i];
        isKilled = pk.Kill(nKillMethod, nWaitTimeout);
        if ( isKilled )
        {
            if ( pnSucceededKillMethod )
                *pnSucceededKillMethod = nKillMethod;
            break;
        }
    }
    return isKilled;
}

bool CChildProcess::WriteInput(const TCHAR* szLine, bool bFFlush )
{
    if ( (!szLine) || (!m_Io.getStdInWritePipe()) )
        return false;

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess::WriteInput(\"%s\") (instance = %s)"), szLine, GetInstanceStr() );

    CStrT<char> encoded;
    const unsigned int enc = CConsoleEncodingDlg::getInputEncoding(GetEncoding());
    if ( !EncodeInputLine(szLine, enc, encoded) )
        return false;

    const int nLen = encoded.length();
    if ( (nLen <= 0) && !bFFlush )
        return true;

    // P1.6: enqueue on caller thread; blocking WriteFile/FlushFileBuffers on writer thread only.
    if ( m_Io.isStdinWriterActive() )
        return m_Io.enqueueStdin(encoded.c_str(), nLen, bFFlush);

    m_Io.writeStdinDirect(encoded.c_str(), nLen, bFFlush);
    return true;
}

tstr& CChildProcess::GetOutput()
{
    return m_strOutput;
}

int CChildProcess::GetExitCode() const
{
    return m_nExitCode;
}

DWORD CChildProcess::GetProcessId() const
{
    return m_ProcessInfo.dwProcessId;
}

const PROCESS_INFORMATION* CChildProcess::GetProcessInfo() const
{
    return &m_ProcessInfo;
}

bool CChildProcess::IsPseudoCon() const
{
    // See also: CNppExecCommandExecutor::IsChildProcessPseudoCon()
    return m_Io.isPseudoCon();
}

bool CChildProcess::ResizePseudoConsoleToConsole()
{
    return m_Io.resizePseudoConsole(m_pNppExec);
}

const TCHAR* CChildProcess::GetNewLine() const
{
    return (m_Io.isPseudoCon() ? _T("\r") : m_pNppExec->GetOptions().GetStr(OPTS_KEY_ENTER));
}

unsigned int CChildProcess::GetEncoding() const
{
    return (m_Io.isPseudoCon() ? CConsoleEncodingDlg::getPseudoConsoleEncoding() : m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_ENCODING));
}

unsigned int CChildProcess::GetAnsiEscSeq() const
{
    int nAnsiEscSeq = m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_ANSIESCSEQ);
    if ( m_Io.isPseudoCon() )
    {
        // PseudoConsole output should be parsed (or at least stripped), not dumped raw.
        if ( nAnsiEscSeq == escKeepRaw )
            nAnsiEscSeq = escProcess;
    }
    return nAnsiEscSeq;
}

bool CChildProcess::IsWindowsNT()
{
#ifdef _WIN64
    return true; // 64-bit Windows XP or later? Definitely Windows NT!
#elif defined(_MSC_VER) && (_MSC_VER >= 1910) // Visual Studio 2017 or later
    return true;
#else
    OSVERSIONINFO osv;
    osv.dwOSVersionInfoSize = sizeof(osv);
    GetVersionEx(&osv);
    return (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
#endif
}

/////////////////////////////////////////////////////////////////////////////

CProcessKiller::CProcessKiller(const PROCESS_INFORMATION* pProcInfo)
{
    ::CopyMemory(&m_ProcInfo, pProcInfo, sizeof(PROCESS_INFORMATION));
}

CProcessKiller::~CProcessKiller()
{
}

bool CProcessKiller::Kill(eKillMethod nKillMethod, unsigned int nWaitTimeout)
{
    bool isKilled = !IsProcessActive();
    if ( !isKilled )
    {
        switch ( nKillMethod )
        {
            case killCtrlBreak:
                isKilled = KillByCtrlBreak(nWaitTimeout);
                break;
            case killCtrlC:
                isKilled = KillByCtrlC(nWaitTimeout);
                break;
            case killWmClose:
                isKilled = KillByWmClose(nWaitTimeout);
                break;
        }
    }
    return isKilled;
}

bool CProcessKiller::KillByCtrlBreak(unsigned int nWaitTimeout)
{
    return KillByConsoleCtrlEvent(CTRL_BREAK_EVENT, nWaitTimeout);
}

bool CProcessKiller::KillByCtrlC(unsigned int nWaitTimeout)
{
    return KillByConsoleCtrlEvent(CTRL_C_EVENT, nWaitTimeout);
}

bool CProcessKiller::KillByWmClose(unsigned int nWaitTimeout)
{
    bool isKilled = false;
    ::EnumWindows(KillAppEnumFunc, m_ProcInfo.dwProcessId);
    if ( ::WaitForSingleObject(m_ProcInfo.hProcess, nWaitTimeout) == WAIT_OBJECT_0 )
    {
        isKilled = true;
    }
    return isKilled;
}

BOOL CALLBACK CProcessKiller::KillAppEnumFunc(HWND hWnd, LPARAM lParam)
{
    DWORD dwID = 0;
    GetWindowThreadProcessId(hWnd, &dwID);
    if ( dwID == (DWORD) lParam )
    {
        ::PostMessage(hWnd, WM_CLOSE, 0, 0) ;
    }
    return TRUE; // continue enumeration
}

bool CProcessKiller::KillByConsoleCtrlEvent(unsigned int nCtrlEvent, unsigned int nWaitTimeout)
{
    // Do not use for ConPTY children from a GUI host — see CChildProcess::Kill().
    typedef BOOL (WINAPI * PFNATTCON)(DWORD);

    bool isKilled = false;
    HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32"));
    if ( hKernel32 )
    {
        PFNATTCON pfnAttachConsole = (PFNATTCON) ::GetProcAddress(hKernel32, "AttachConsole");
        if ( pfnAttachConsole )
        {
            if ( pfnAttachConsole(m_ProcInfo.dwProcessId) )
            {
                ::SetConsoleCtrlHandler(NULL, TRUE); // Disable Ctrl-C handling for our program
                ::GenerateConsoleCtrlEvent(nCtrlEvent, m_ProcInfo.dwProcessId);
                if ( ::WaitForSingleObject(m_ProcInfo.hProcess, nWaitTimeout) == WAIT_OBJECT_0 )
                {
                    isKilled = true;
                }
                ::FreeConsole();
                ::SetConsoleCtrlHandler(NULL, FALSE); // Re-enable Ctrl-C handling
            }
        }
    }
    return isKilled;
}

bool CProcessKiller::IsProcessActive() const
{
    DWORD dwExitCode = (DWORD)(-1);
    ::GetExitCodeProcess(m_ProcInfo.hProcess, &dwExitCode);
    return (dwExitCode == STILL_ACTIVE);
}
