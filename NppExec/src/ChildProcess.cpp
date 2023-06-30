#include "ChildProcess.h"
#include "NppExecHelpers.h"
#include "NppExec.h"
#include "NppExecEngine.h"
#include "DlgConsoleEncoding.h"
#include "cpp/StrSplitT.h"
#include "c_base/MatchMask.h"
#include "c_base/int2str.h"
#include "c_base/str2int.h"

PseudoConsoleHelper g_pseudoCon;

const int nPseudoConWidth = 192;
const int nPseudoConHeight = 1000;

enum eNewLine {
    nlNone = 0,
    nlLF = 1, // \n
    nlCR = 3, // \r
    nlBS = 7  // \b
};

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
bool CChildProcess::Create(HWND /*hParentWnd*/, LPCTSTR cszCommandLine)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFOEX       si;

    reset();

    if ( IsWindowsNT() )
    {
        // security stuff for NT
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

    // Create the Pipe and get r/w handles
    if ( !::CreatePipe(&m_hStdOutReadPipe, &m_hStdOutWritePipe, &sa, DEFAULT_PIPE_SIZE) )
    {
        m_pNppExec->GetConsole().PrintError( _T("CreatePipe(<StdOut>) failed") );
        return false;
    }
    if ( m_hStdOutWritePipe == NULL )
    {
        if ( m_hStdOutReadPipe != NULL )
            ::CloseHandle(m_hStdOutReadPipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdOutWritePipe = NULL") );
        return false;
    }
    if ( m_hStdOutReadPipe == NULL )
    {
        ::CloseHandle(m_hStdOutWritePipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdOutReadPipe = NULL") );
        return false;
    }

    if ( !::CreatePipe(&m_hStdInReadPipe, &m_hStdInWritePipe, &sa, DEFAULT_PIPE_SIZE) )
    {
        m_pNppExec->GetConsole().PrintError( _T("CreatePipe(<StdIn>) failed") );
        return false;
    }
    if ( m_hStdInWritePipe == NULL )
    {
        if ( m_hStdInReadPipe != NULL )
            ::CloseHandle(m_hStdInReadPipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdInWritePipe = NULL") );
        return false;
    }
    if ( m_hStdInReadPipe == NULL )
    {
        ::CloseHandle(m_hStdInWritePipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdInReadPipe = NULL") );
        return false;
    }

    ::SetHandleInformation(m_hStdInWritePipe, HANDLE_FLAG_INHERIT, 0);
    ::SetHandleInformation(m_hStdOutReadPipe, HANDLE_FLAG_INHERIT, 0);

    m_hPseudoCon = NULL;
    m_pAttributeList = NULL;

    if ( m_pNppExec->GetOptions().GetBool(OPTB_CHILDP_PSEUDOCONSOLE) && (g_pseudoCon.pfnCreatePseudoConsole != nullptr) )
    {
        COORD conSize = { nPseudoConWidth, nPseudoConHeight };
        HRESULT hr = g_pseudoCon.pfnCreatePseudoConsole(conSize, m_hStdInReadPipe, m_hStdOutWritePipe, 0, &m_hPseudoCon);
        if ( FAILED(hr) )
        {
            m_hPseudoCon = NULL;
        }
    }

    // Job object
    HANDLE hJob = NULL;
    if ( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_KILLPROCTREE) )
    {
        hJob = ::CreateJobObject(NULL, NULL);
        if ( hJob != NULL )
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli;

            ::ZeroMemory(&jeli, sizeof(jeli));
            jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            // Causes all processes associated with the job to terminate when the last handle to the job is closed.
            if ( !::SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli)) )
            {
                ::CloseHandle(hJob);
                hJob = NULL;
            }
        }
    }

    /*
    DWORD dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    SetNamedPipeHandleState(m_hStdOutWritePipe, &dwMode, NULL, NULL);
    dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    SetNamedPipeHandleState(m_hStdOutReadPipe, &dwMode, NULL, NULL);
    */

    DWORD dwCreationFlags = 0;

    // initialize STARTUPINFOEX struct
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
                    HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
                    closePseudoConsole();
                }
            }
            else
            {
                HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
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
        si.StartupInfo.hStdError = m_hStdOutWritePipe;
    }

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

    if ( hJob != NULL )
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
            TRUE,                        // inherits handles
            dwCreationFlags,             // creation flags
            NULL,                        // environment
            NULL,                        // current directory
            &si.StartupInfo,             // startup info
            &m_ProcessInfo               // process info
       ) )
    {
        if ( hJob != NULL )
        {
            if ( !::AssignProcessToJobObject(hJob, m_ProcessInfo.hProcess) )
            {
                ::CloseHandle(hJob);
                hJob = NULL;
            }
        }

        ::CloseHandle(m_ProcessInfo.hThread); m_ProcessInfo.hThread = NULL;
        if ( m_hPseudoCon )
        {
            ::CloseHandle(m_hStdOutWritePipe); m_hStdOutWritePipe = NULL;
            ::CloseHandle(m_hStdInReadPipe); m_hStdInReadPipe = NULL;

            m_PseudoConScreen.InitScreen(nPseudoConWidth, nPseudoConHeight);
        }

        bool isConsoleProcessRunning = true;
    
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
        
        CStrT<char>  bufLine;
        bool         bPrevLineEmpty = false;
        bool         bDoOutputNext = true;
        int          nPrevState = nlNone;
        DWORD        dwRead = 0;
        unsigned int nEmptyCount = 0;
        const DWORD  dwCycleTimeOut = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_CYCLETIMEOUT_MS);
        const DWORD  dwExitTimeOut = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_EXITTIMEOUT_MS);
    
        bufLine.Clear(); // just in case :-)
    
        do 
        {
            // inside this cycle: the bOutputAll parameter must be controlled within readPipesAndOutput
            dwRead = readPipesAndOutput(bufLine, bPrevLineEmpty, nPrevState, false, bDoOutputNext);

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
                    nEmptyCount = 0;
            }

        }
        while ( (isConsoleProcessRunning = (::WaitForSingleObject(m_ProcessInfo.hProcess, dwCycleTimeOut) == WAIT_TIMEOUT))
             && m_pScriptEngine->ContinueExecution() && !isBreaking() );
        // NOTE: time-out inside WaitForSingleObject() prevents from 100% CPU usage!

        if ( m_pScriptEngine->ContinueExecution() && (!isBreaking()) && !m_pScriptEngine->GetTriedExitCmd() )
        {
            // maybe the child process is exited but not all its data is read
            readPipesAndOutput(bufLine, bPrevLineEmpty, nPrevState, true, bDoOutputNext);
        }

        if ( (!m_pScriptEngine->ContinueExecution()) || isBreaking() )
        {
            if ( isConsoleProcessRunning )
            {
                int nKillMethods = 0;
                CProcessKiller::eKillMethod arrKillMethods[4];

                tstr sAppName;
                CListT<tstr> ArgsList; 
                if (StrSplitToArgs(cszCommandLine, ArgsList, 2) > 0)
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
                        if (nPrevState != nlLF /* new line */ )
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
                            if (nPrevState != nlLF /* new line */ )
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

        DWORD dwExitCode = (DWORD)(-1);
        ::GetExitCodeProcess(m_ProcessInfo.hProcess, &dwExitCode);
        m_nExitCode = (int) dwExitCode;

        // Process cleanup
        ::CloseHandle(m_ProcessInfo.hProcess); m_ProcessInfo.hProcess = NULL;
        closePseudoConsole();
        closePipes();
        if ( hJob != NULL )
        {
            ::CloseHandle(hJob);
            hJob = NULL;
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
                if (nPrevState != nlLF /* new line */ )
                {
                    const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                    m_pNppExec->GetConsole().PrintMessage( _T(""), nPrintMsgFlags );
                }
            }
        }

        return true;
    }
    else
    {
        DWORD dwErrorCode = ::GetLastError();

        closePseudoConsole();
        closePipes();
        if ( hJob != NULL )
        {
            ::CloseHandle(hJob);
            hJob = NULL;
        }

        if ( m_pScriptEngine )
        {
            m_pNppExec->GetConsole().PrintError( m_pScriptEngine->GetLastLoggedCmd().c_str() );
        }
        m_pNppExec->GetConsole().PrintSysError( _T("CreateProcess()"), dwErrorCode );

        return false;
    }
}

void CChildProcess::reset()
{
    m_strOutput.Clear();
    m_nExitCode = -1;
    m_nBreakMethod = CProcessKiller::killNone;
    m_hStdInReadPipe = NULL;
    m_hStdInWritePipe = NULL; 
    m_hStdOutReadPipe = NULL;
    m_hStdOutWritePipe = NULL;
    m_hPseudoCon = NULL;
    m_pAttributeList = NULL;
    ::ZeroMemory(&m_ProcessInfo, sizeof(PROCESS_INFORMATION));
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
                                        bool& bDoOutputNext)
{
    DWORD       dwBytesRead = 0;
    char        Buf[CONSOLEPIPE_BUFSIZE];
    CStrT<char> outLine;
  
    bool bSomethingHasBeenReadFromThePipe = false; // great name for a local variable :-)

    const bool bConFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE);
    const bool bConFltrExclAllEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_EXCLALLEMPTY);
    const bool bConFltrExclDupEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_EXCLDUPEMPTY);
    const bool bOutputVar = m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SETOUTPUTVAR);
    const unsigned int nAnsiEscSeq = GetAnsiEscSeq();

    const int nBufLineLength = bufLine.length();

    do
    { 
        ::Sleep(10);  // it prevents from 100% CPU usage while reading!
        dwBytesRead = 0;
        if ( !::PeekNamedPipe(m_hStdOutReadPipe, NULL, 0, NULL, &dwBytesRead, NULL) )
        {
            dwBytesRead = 0;
        }
        if ( !dwBytesRead )
        {
            // no data in the pipe
            if ( !bSomethingHasBeenReadFromThePipe )
            {
                // did we read something from the pipe already?
                // if no, then let's output the data from bufLine (if any)
                bOutputAll = true;
            }
        }
        if ( (dwBytesRead > 0) || bOutputAll )
        {
            // some data is in the Pipe or bOutputAll==true

            bool bContainsData = (dwBytesRead > 0) ? true : false;
            // without bContainsData==true the ReadFile operation will never return

            if ( bContainsData )
                ::ZeroMemory(Buf, CONSOLEPIPE_BUFSIZE);
            dwBytesRead = 0;
            if ( (bContainsData 
                  && ::ReadFile(m_hStdOutReadPipe, Buf, (CONSOLEPIPE_BUFSIZE-1)*sizeof(char), &dwBytesRead, NULL)
                  && (dwBytesRead > 0)) || bOutputAll )
            {
                // some data has been read from the Pipe or bOutputAll==true
        
                int copy_len;

                Buf[dwBytesRead/sizeof(char)] = 0;

                bufLine.Append( Buf, dwBytesRead/sizeof(char) );

                if ( dwBytesRead > 0 )
                {
                    bSomethingHasBeenReadFromThePipe = true;
                }

                if ( m_hPseudoCon != nullptr && nAnsiEscSeq == escProcess )
                {
                    if ( dwBytesRead > 0 )
                    {
                        tstr ws = NppExecHelpers::CStrToWStr(bufLine, CP_UTF8);

                        /*
                        FILE* f = fopen("C:\\temp\\ws.txt", "w");
                        if (f)
                        {
                            fwrite(ws.c_str(), sizeof(TCHAR), ws.length(), f);
                            fclose(f);
                        }
                        */

                        m_PseudoConScreen.ProcessAnsiEscSequences(ws);

                        tstr ws2 = m_PseudoConScreen.ToString();
                        tstr printLine;
                        
                        CStrSplitT<TCHAR> args;

                        int n = args.Split(ws2, _T("\n"));
                        for ( int i = 0; i < n; ++i )
                        {
                            tstr& ln = args.Arg(i);

                            printLine = ln;
                            NppExecHelpers::StrLower(ln);

                            // >>> console output filters
                            bool bOutput = applyOutputFilters(ln, true);
                            // <<< console output filters

                            // >>> console replace filters
                            bOutput = applyReplaceFilters(ln, printLine, bOutput);
                            // <<< console replace filters

                            if ( bOutput )
                            {
                                UINT nPrintOutFlags = CNppExecConsole::pfLogThisMsg;
                                if ( i != n - 1 )
                                    nPrintOutFlags |= CNppExecConsole::pfNewLine;
                                m_pNppExec->GetConsole().PrintOutput( printLine.c_str(), nPrintOutFlags );
                            }
                        }
                    }
                }
                else

                // The following lines are needed for filtered output only.
                // I.e. you can replace all these lines by this one:
                //     GetConsole().PrintOutput(Buf);
                // if you don't need filtered output. (*)
                // (*) But don't forget about Unicode version:
                //     OEM -> WideChar or UTF-8 -> WideChar
        
                /**/
                do {
         
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
                            if ( bufLine[pos+1] != '\n' )
                            {
                                // not "\r\n" pair
                                if ( (bufLine[pos+1] != '\r') || (bufLine.GetAt(pos+2) != '\n') )
                                {
                                    // not "\r\r\n" (stupid M$'s line ending)
                                    // just "\r"
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
                            copy_len = pos;
                            if ( !nIsNewLine )
                            {
                                // i.e. bOutputAll is true
                                copy_len++;
                            }
                            else if ( (pos > 0) && (bufLine[pos-1] == '\r') )
                            {
                                copy_len--;
                                if ( (pos > 1) && (bufLine[pos-2] == '\r') )
                                    copy_len--;
                            }

                            outLine.Assign(bufLine.c_str(), copy_len);

                            if ( nIsNewLine == nlBS ) // '\b'
                            {
                                // counting "\b\b..." and skip them
                                while ( bufLine[pos+1] == '\b' )
                                {
                                    ++nIsNewLine;
                                    ++pos;
                                }
                            }

                            bufLine.Delete(0, pos+1);
                            if ( (copy_len > 0) ||
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
                                                _line.Replace( wchar_t(0x0000), wchNulChar ); // e.g. to 0x25E6 - the "White Bullet" symbol
                                            }
                                        }

                                      #else

                                        {
                                            char chNulChar = CNppConsoleRichEdit::GetNulChar();
                                            if ( chNulChar != 0 )
                                            {
                                                outLine.Replace( char(0x00), chNulChar ); // e.g. to 0x17 - the "End of Text Block" symbol
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

                                    // >>> console output filters
                                    bOutput = applyOutputFilters(_line, bOutput);
                                    // <<< console output filters

                                    // >>> console replace filters
                                    bOutput = applyReplaceFilters(_line, printLine, bOutput);
                                    // <<< console replace filters
                                }

                                if ( bOutput )
                                {
                                    bool bPrintThisLine = true;

                                    if ( nAnsiEscSeq == escRemove )
                                    {
                                        bPrintThisLine = RemoveAnsiEscSequencesFromLine(printLine);
                                    }

                                    if ( nPrevState == nlCR ) // '\r'
                                    {
                                        if ( !(nIsNewLine == nlLF && pos <= 1 && copy_len == 0) )
                                            m_pNppExec->GetConsole().ProcessSlashR();
                                    }
                                    else if ( nPrevState >= nlBS ) // '\b'...
                                    {
                                        m_pNppExec->GetConsole().ProcessSlashB( (nPrevState - nlBS) + 1 );
                                    }

                                    if ( bPrintThisLine )
                                    {
                                        if ( bOutputVar )
                                        {
                                            m_strOutput += printLine;
                                            if ( nIsNewLine == nlLF )
                                            {
                                                m_strOutput += _T("\n");
                                            }
                                        }

                                        UINT nPrintOutFlags = CNppExecConsole::pfLogThisMsg;
                                        if ( nIsNewLine == nlLF )
                                            nPrintOutFlags |= CNppExecConsole::pfNewLine;
                                        m_pNppExec->GetConsole().PrintOutput( printLine.c_str(), nPrintOutFlags );
                                    }
                                }

                                // if the current line is not over, then the current filter 
                                // must be applied to the rest of this line
                                bDoOutputNext = bOutput;
                            }
                            bPrevLineEmpty = (copy_len > 0) ? false : true;
                            nPrevState = nIsNewLine;
                            if ( nIsNewLine == nlLF )
                            {
                                // current line is over - abort current filter
                                bDoOutputNext = true;
                            }
                            break;
                        }
                    
                    }
                } while ( copy_len >= 0 );
                /**/

            }
        }

    } 
    while ( (dwBytesRead > 0) && m_pScriptEngine->ContinueExecution() && !isBreaking() );

    if ( bOutputAll && !dwBytesRead )  dwBytesRead = nBufLineLength;
    return dwBytesRead;
}

bool CChildProcess::RemoveAnsiEscSequencesFromLine(tstr& Line)
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

    outputLine.Reserve(Line.length());

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
                if ( (curr_ch >= _T('A') && curr_ch <= _T('Z')) ||
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
                    // the "final byte" of the CSI sequence
                    state = esNone;
                }
                // else waiting for the CSI final character...
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

bool CChildProcess::Kill(const CProcessKiller::eKillMethod arrKillMethods[], int nKillMethods,
                         unsigned int nWaitTimeout,
                         CProcessKiller::eKillMethod* pnSucceededKillMethod)
{
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
    if ( (!szLine) || (!m_hStdInWritePipe) )
        return false;

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess::WriteInput(\"%s\") (instance = %s)"), szLine, GetInstanceStr() );
    
    char*        pStr = NULL;
    int          len = 0;
    DWORD        dwBytesWritten = 0;
    unsigned int enc = CConsoleEncodingDlg::getInputEncoding(GetEncoding());

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
    
    if ( pStr )
    {
        ::WriteFile(m_hStdInWritePipe, pStr, len*sizeof(char), &dwBytesWritten, NULL);
        if ( bFFlush )
        {
            // beware! this may hang the application (due to MustDie's pipes)
            ::FlushFileBuffers(m_hStdInWritePipe);
        }
        delete [] pStr;
    }
                  
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
            {
                bNewMemory = true;
            }
            break;

        default:
            len = lstrlen(szLine);
            pStr = (char *) szLine;
            bNewMemory = false;
            break;
    }

    if ( pStr )
    {
        ::WriteFile(m_hStdInWritePipe, pStr, len*sizeof(char), &dwBytesWritten, NULL);
        if ( bFFlush )
        {
            // beware! this may hang the application (due to MustDie's pipes)
            ::FlushFileBuffers(m_hStdInWritePipe);
        }
        if ( bNewMemory )
            delete [] pStr;
    }
  
  #endif

    return true;
}

void CChildProcess::closePipes()
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
    closePipe(m_hStdInReadPipe);
    closePipe(m_hStdInWritePipe);
}

void CChildProcess::closePseudoConsole()
{
    if ( g_pseudoCon.pfnClosePseudoConsole && m_hPseudoCon )
    {
        g_pseudoCon.pfnClosePseudoConsole(m_hPseudoCon); m_hPseudoCon = NULL;
    }
    if ( m_pAttributeList )
    {
        g_pseudoCon.pfnDeleteProcThreadAttributeList(m_pAttributeList); m_pAttributeList = NULL;
    }
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
    return (m_hPseudoCon ? true : false);
}

const TCHAR* CChildProcess::GetNewLine() const
{
    return (m_hPseudoCon ? _T("\r") : m_pNppExec->GetOptions().GetStr(OPTS_KEY_ENTER));
}

unsigned int CChildProcess::GetEncoding() const
{
    return (m_hPseudoCon ? CConsoleEncodingDlg::getPseudoConsoleEncoding() : m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_ENCODING));
}

unsigned int CChildProcess::GetAnsiEscSeq() const
{
    int nAnsiEscSeq = m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_ANSIESCSEQ);
    if ( m_hPseudoCon )
    {
        // TODO: NppExec provides very limited support of ANSI Escape Sequences
        // (PsequdoConsole requires _full_ support of them)
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

///////////////////////////////////////////////////////////////////////////////

CPseudoConsoleScreen::CPseudoConsoleScreen()
  : m_width(0), m_height(0), m_currX(0), m_currY(0), m_maxY(0)
{
}

void CPseudoConsoleScreen::InitScreen(int width, int height)
{
    m_width = width;
    m_height = height;

    int len = (width + 1)*height; // using (width + 1) for adding explicit '\n'
    m_screen.Reserve(len);
    ::ZeroMemory(m_screen.c_str(), len*sizeof(TCHAR));
    m_screen.SetLengthValue(len);
}

bool CPseudoConsoleScreen::ProcessAnsiEscSequences(tstr& Line)
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
    int n, x, y;
    TCHAR curr_ch = 0;
    TCHAR wait1_ch = 0;
    TCHAR wait2_ch = 0;
    tstr escSeq;

    m_currX = 0;
    m_currY = 0;
    m_maxY = 0;
    ::ZeroMemory(m_screen.c_str(), (m_width + 1)*m_height*sizeof(TCHAR));

    while ( (curr_ch = *p) != 0 )
    {
        switch ( state )
        {
        case esNone:
            switch ( curr_ch )
            {
            case 0x1B:  // ESC
                state = esEsc;
                break;
            case 0x0D:  // CR
                CarriageReturn();
                break;
            case 0x0A:  // LF
                LineFeed();
                break;
            case _T('\b'):
                BackSpace();
                break;
            default:
                AddCharacter(curr_ch);
                break;
            }
            break;

        case esEsc:
            switch ( curr_ch )
            {
            case _T('['):  // CSI
                state = esCsi;
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
            if ( (curr_ch >= _T('A') && curr_ch <= _T('Z')) ||
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
                // the "final byte" of the CSI sequence
                switch ( curr_ch )
                {
                case _T('@'): // CSI n @        ICH    Insert n Blank Characters
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    InsertBlankCharacters(n);
                    break;
                case _T('A'): // CSI n A        CUU    Cursor Up
                case _T('F'): // CSI n F        CPL    Cursor Previous Line
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    CursorUp(n);
                    break;
                case _T('B'): // CSI n B        CUD    Cursor Down
                case _T('E'): // CSI n E        CNL    Cursor Next Line
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    CursorDown(n);
                    break;
                case _T('C'): // CSI n C        CUF    Cursor Forward
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    CursorForward(n);
                    break;
                case _T('D'): // CSI n D        CUB    Cursor Back
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    CursorBackward(n);
                    break;
                case _T('G'): // CSI n G        CHA    Cursor Horizontal Absolute
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    CursorHorizontalPosition(n);
                    break;
                case _T('d'): // CSI n d        VPA    Vertical Line Position Absolute
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    CursorVerticalPosition(n);
                    break;
                case _T('H'): // CSI y ; x H    CUP    Cursor Position
                case _T('f'): // CSI y ; x f    HVP    Horizontal Vertical Position
                    y = c_base::_tstr2int(escSeq.c_str()); // row
                    if ( y > 0 )
                        --y; // 0-based
                    n = escSeq.Find(_T(';'));
                    if ( n != -1 )
                    {
                        x = c_base::_tstr2int(escSeq.c_str() + n + 1); // column
                        if ( x > 0 )
                            --x; // 0-based
                    }
                    else
                        x = 0;
                    CursorPosition(x, y);
                    break;
                case _T('J'): // CSI n J        ED     Erase in Display
                    if ( escSeq.GetFirstChar() == _T('?') )
                        n = c_base::_tstr2int(escSeq.c_str() + 1);
                    else
                        n = c_base::_tstr2int(escSeq.c_str());
                    EraseInDisplay(n);
                    break;
                case _T('K'): // CSI n K        EL     Erase in Line
                    if ( escSeq.GetFirstChar() == _T('?') )
                        n = c_base::_tstr2int(escSeq.c_str() + 1);
                    else
                        n = c_base::_tstr2int(escSeq.c_str());
                    EraseInLine(n);
                    break;
                case _T('L'): // CSI n L        IL     Insert Line
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    InsertLines(n);
                    break;
                case _T('M'): // CSI n M        DL     Delete Line
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    DeleteLines(n);
                    break;
                case _T('P'): // CSI n P        DCH    Delete Character
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    DeleteCharacters(n);
                    break;
                case _T('X'): // CSI n X        RCH    Erase n Characters
                    n = c_base::_tstr2int(escSeq.c_str());
                    if ( n == 0 )
                        n = 1;
                    EraseCharacters(n);
                    break;
                }

                escSeq.Clear();
                state = esNone;
            }
            else
            {
                // waiting for the CSI final character...
                escSeq += curr_ch;
            }
            break;

        case esOsc:
        case esWaitSt:
            if ( curr_ch == 0x1B )  // ESC
            {
                const TCHAR next_ch = *(p + 1);
                if ( next_ch == _T('\\') )  // ST
                {
                    ++p; // skipping the next character as well

                    escSeq.Clear();
                    state = esNone;
                }
                else // ?
                {
                    escSeq.Clear();
                    state = esEsc; // ?
                }
            }
            else
            {
                // waiting for the ST...
                escSeq += curr_ch;
            }
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

    return true;
}

int CPseudoConsoleScreen::getCurrPos() const
{
    return (m_currY*(m_width + 1) + m_currX);
}

int CPseudoConsoleScreen::getCurrLineBeginPos() const
{
    return m_currY*(m_width + 1);
}

int CPseudoConsoleScreen::getMaxPos() const
{
    return ((m_width + 1)*m_height);
}

void CPseudoConsoleScreen::incCurrPos()
{
    ++m_currX;
    if ( m_currX >= m_width )
    {
        m_currX -= m_width;
        ++m_currY;
        if ( m_currY > m_maxY )
        {
            m_maxY = m_currY;
            resizeScreenIfNeeded(m_currY);
        }
    }
}

void CPseudoConsoleScreen::resizeScreenIfNeeded(int y)
{
    if ( y >= m_height )
    {
        int len = (m_width + 1)*(y + 1);
        m_screen.Reserve(len);
        ::ZeroMemory(m_screen.c_str() + (m_width + 1)*m_height, (m_width + 1)*(y + 1 - m_height)*sizeof(TCHAR));
        m_screen.SetLengthValue(len);
        m_height = y;
    }
}

void CPseudoConsoleScreen::AddCharacter(wchar_t ch)
{
    int pos = getCurrPos();
    if ( pos >= getMaxPos() )
    {
        resizeScreenIfNeeded(m_currY);
    }

    m_screen[pos] = ch;
    incCurrPos();
}

void CPseudoConsoleScreen::BackSpace()
{
    if ( m_currX != 0 )
    {
        --m_currX;

        int pos = getCurrPos();
        m_screen[pos] = 0;
    }
}

void CPseudoConsoleScreen::CarriageReturn()
{
    //m_currX = 0;
}

void CPseudoConsoleScreen::CursorBackward(int count)
{
    if ( count <= 0 )
        return;

    if ( m_currX > count )
        m_currX -= count;
    else
        m_currX = 0;
}

void CPseudoConsoleScreen::CursorDown(int count)
{
    if ( count <= 0 )
        return;

    int maxY = m_height - 1;
    if ( m_currY + count < maxY )
        m_currY = maxY;
    else
        m_currY = maxY;
}

void CPseudoConsoleScreen::CursorForward(int count)
{
    if ( count <= 0 )
        return;

    m_currX += count;
    if ( m_currX > m_width )
        m_currX = m_width;
}

void CPseudoConsoleScreen::CursorHorizontalPosition(int x)
{
    if ( x <= 0 )
        return;

    m_currX = x;
    if ( m_currX > m_width )
        m_currX = m_width;
}

void CPseudoConsoleScreen::CursorPosition(int x, int y)
{
    m_currX = x;
    if ( m_currX > m_width )
        m_currX = m_width;

    m_currY = y;
    if ( m_currY > m_maxY )
    {
        m_maxY = m_currY;
        resizeScreenIfNeeded(m_currY);
    }
}

void CPseudoConsoleScreen::CursorUp(int count)
{
    if ( count <= 0 )
        return;

    if ( m_currY > count )
        m_currY -= count;
    else
        m_currY = 0;
}

void CPseudoConsoleScreen::CursorVerticalPosition(int y)
{
    if ( y <= 0 )
        return;

    m_currY = y;
    if ( m_currY > m_maxY )
    {
        m_maxY = m_currY;
        resizeScreenIfNeeded(m_currY);
    }
}

void CPseudoConsoleScreen::DeleteCharacters(int count)
{
    if ( count <= 0 )
        return;

    int pos = getCurrPos();
    int maxPos = pos + m_width - m_currX;

    if ( m_currX + count < m_width )
    {
        for ( int i = 0; i < m_width - m_currX - count; ++i )
        {
            m_screen[pos] = m_screen[pos + count];
            ++pos;
        }
    }

    for ( ; pos < maxPos; ++pos )
    {
        m_screen[pos] = 0;
    }
}

void CPseudoConsoleScreen::DeleteLines(int count)
{
    if ( count <= 0 )
        return;

    if ( m_currY + count > m_height )
        count = m_height - m_currY;

    int n = count*(m_width + 1);
    int pos = getCurrLineBeginPos();
    m_screen.Delete(pos, n);

    tstr S;
    S.Reserve(n);
    S.SetLengthValue(n);
    ::ZeroMemory(S.c_str(), n*sizeof(TCHAR));

    m_screen.Append(S);
}

void CPseudoConsoleScreen::EraseCharacters(int count)
{
    if ( count <= 0 )
        return;

    if ( m_currX + count > m_width )
        count = m_width - m_currX;

    int pos = getCurrPos();
    for ( ; count != 0; --count )
    {
        m_screen[pos] = 0;
        ++pos;
    }
}

void CPseudoConsoleScreen::EraseInDisplay(int mode)
{
    int pos, maxPos;

    switch ( mode )
    {
    case 0: // 0 - clear from cursor to end of screen
        pos = getCurrPos();
        maxPos = getMaxPos();
        for ( ; pos < maxPos; ++pos )
        {
            m_screen[pos] = 0;
        }
        m_maxY = m_currY;
        break;
    case 1: // 1 - clear from cursor to beginning of the screen
        pos = 0;
        maxPos = getCurrPos();
        for ( ; pos < maxPos; ++pos )
        {
            m_screen[pos] = 0;
        }
        break;
    case 2: // 2 - clear entire screen (and moves cursor to upper left on DOS ANSI.SYS)
    case 3: // 3 - clear entire screen and delete all lines saved in the scrollback buffer
        pos = 0;
        maxPos = getMaxPos();
        for ( ; pos < maxPos; ++pos )
        {
            m_screen[pos] = 0;
        }
        m_currX = 0;
        m_currY = 0;
        m_maxY = 0;
        break;
    }
}

void CPseudoConsoleScreen::EraseInLine(int mode)
{
    int pos;
    int maxPos;

    switch ( mode )
    {
    case 0: // 0 - clear from cursor to the end of the line
        pos = getCurrPos();
        maxPos = getCurrLineBeginPos() + m_width + 1;
        for ( ; pos < maxPos; ++pos )
        {
            m_screen[pos] = 0;
        }
        break;
    case 1: // 1 - clear from cursor to beginning of the line
        pos = getCurrLineBeginPos();
        maxPos = getCurrPos();
        for ( ; pos < maxPos; ++pos )
        {
            m_screen[pos] = 0;
        }
        break;
    case 2: // 2 - clear entire line
        pos = getCurrLineBeginPos();
        maxPos = pos + m_width + 1;
        for ( ; pos < maxPos; ++pos )
        {
            m_screen[pos] = 0;
        }
        break;
    }
}

void CPseudoConsoleScreen::InsertBlankCharacters(int count)
{
    if ( count <= 0 )
        return;

    int pos = getCurrPos();

    if ( m_currX + count < m_width )
    {
        for ( int i = 0; i < m_width - m_currX - count; ++i )
        {
            m_screen[pos + count + i] = m_screen[pos + i];
        }
    }

    for ( int i = m_currX; i < m_width && count != 0; ++i )
    {
        m_screen[pos] = _T(' ');
        ++pos;
        --count;
        incCurrPos();
    }
}

void CPseudoConsoleScreen::InsertLines(int count)
{
    if ( count <= 0 )
        return;

    int n = count*(m_width + 1);
    m_screen.DeleteLast(n);

    tstr S;
    S.Reserve(n);
    S.SetLengthValue(n);
    ::ZeroMemory(S.c_str(), n*sizeof(TCHAR));

    int pos = (m_currY + 1)*(m_width + 1);
    m_screen.Insert(pos, S);
}

void CPseudoConsoleScreen::LineFeed()
{
    int pos = getCurrPos();
    m_screen[pos] = _T('\n');

    m_currX = 0;
    ++m_currY;
    if ( m_currY > m_maxY )
    {
        m_maxY = m_currY;
        resizeScreenIfNeeded(m_currY);
    }
}

tstr CPseudoConsoleScreen::ToString() const
{
    tstr S;
    S.Reserve((m_width + 1)*4);

    int maxPos = getCurrPos();
    for ( int pos = 0; pos < maxPos; ++pos )
    {
        if ( m_screen[pos] != 0 )
            S += m_screen[pos];
    }

    int nCharsToDelete = 0;
    if ( !m_prevString.IsEmpty() )
    {
        for ( ; nCharsToDelete < m_prevString.length(); ++nCharsToDelete )
        {
            if ( S[nCharsToDelete] != m_prevString[nCharsToDelete] )
                break;
        }
    }
    m_prevString = S;
    if ( nCharsToDelete != 0 )
    {
        S.Delete(0, nCharsToDelete);
    }

    return S;
}
