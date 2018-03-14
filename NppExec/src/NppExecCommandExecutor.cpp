/*
This file is part of NppExec
Copyright (C) 2016 DV <dvv81 (at) ukr (dot) net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "NppExecCommandExecutor.h"
#include "NppExec.h"
#include "NppExecEngine.h"
#include "resource.h"
#include "DlgDoExec.h"
#include "DlgConsole.h"
#include "c_base/int2str.h"
#include <algorithm>


static DWORD WINAPI dwRunCollateralScriptThread(LPVOID lpScrptEngnRnr)
{
    Runtime::GetLogger().OnThreadStarted();

    CScriptEngineRunner* pScriptEngineRunner = static_cast<CScriptEngineRunner*>(lpScrptEngnRnr);
    std::shared_ptr<CScriptEngine> pScriptEngine(pScriptEngineRunner->GetScriptEngine());
    unsigned int nRunFlags = pScriptEngineRunner->GetRunFlags();
    delete pScriptEngineRunner;

    //::Sleep(100);

    CNppExec* pNppExec = pScriptEngine->GetNppExec();
    pNppExec->GetCommandExecutor().RunScriptEngine( pScriptEngine, nRunFlags );

    if ( pNppExec->GetCommandExecutor().GetRunningScriptEngine() )
    {
        DWORD scrptEngnId = 0;
        std::shared_ptr<CScriptEngine> pParentScriptEngine = pScriptEngine->GetParentScriptEngine();
        while ( pParentScriptEngine && pParentScriptEngine->IsDone() )
            pParentScriptEngine = pParentScriptEngine->GetParentScriptEngine();
        if ( pParentScriptEngine )
            scrptEngnId = pParentScriptEngine->GetThreadId();
        if ( pNppExec->GetConsole().GetOutputEnabledDirectly(scrptEngnId) != 0 )
        {
            pNppExec->GetCommandExecutor().WriteChildProcessInput( pNppExec->GetOptions().GetStr(OPTS_KEY_ENTER) );
              // sends "\n" to show child process'es prompt

            pNppExec->GetConsole().LockConsoleEndPosAfterEnterPressed();
        }
    }
    else if ( !pScriptEngine->IsExternal() )
    {
        pNppExec->GetConsole().PrintMessage( _T("================ READY ================"), false );
        pNppExec->GetConsole().RestoreDefaultTextStyle(true);
    }

    pNppExec->GetPluginInterfaceImpl().NotifyExternalPluginResult( pScriptEngine->GetId(), pScriptEngine->IsAborted() ? NPE_RESULT_ABORTED : NPE_RESULT_OK );

    Runtime::GetLogger().OnThreadFinished();

    return 0;
}


CCriticalSection CNppExecCommandExecutor::ExpirableCommand::s_csExpirableCommands;
std::map<UINT_PTR, CNppExecCommandExecutor::ExpirableCommand*> CNppExecCommandExecutor::ExpirableCommand::s_ExpirableCommands;

// ... CNppExecCommandExecutor ...
CNppExecCommandExecutor::CNppExecCommandExecutor()
  : m_pNppExec(NULL)
  , m_hBackgroundExecuteThread(NULL)
  , m_hBackgroundExpiredThread(NULL)
  , m_RunningScriptEngine()
  , m_nRunningScripts(0)
  //, m_nRunningChildProcesses(0)
{
}

CNppExecCommandExecutor::~CNppExecCommandExecutor()
{
    if ( m_hBackgroundExecuteThread != NULL )
        ::CloseHandle(m_hBackgroundExecuteThread);

    if ( m_hBackgroundExpiredThread != NULL )
        ::CloseHandle(m_hBackgroundExpiredThread);

    m_ExpiredQueue.clear();
    m_ExecuteQueue.clear();
}

DWORD WINAPI CNppExecCommandExecutor::BackgroundExecuteThreadFunc(LPVOID lpParam)
{
    CNppExecCommandExecutor* pCommandExecutor = (CNppExecCommandExecutor *) lpParam;
    HANDLE waitEvents[2] = { pCommandExecutor->m_StopEvent.GetHandle(), pCommandExecutor->m_ExecuteCmdEvent.GetHandle() };

    for ( ; ; )
    {
        DWORD dwWaitEvent = ::WaitForMultipleObjects(2, waitEvents, FALSE, INFINITE);
        if ( dwWaitEvent == WAIT_OBJECT_0 ) // Stop Event
            break;

        if ( dwWaitEvent == WAIT_OBJECT_0 + 1 ) // ExecuteCmd Event
        {
            std::shared_ptr<Command> pCommand;
            bool isExecuteQueueEmpty;

            {
                CCriticalSectionLockGuard lock(pCommandExecutor->m_csExecuteQueue);
                auto& ExecuteQueue = pCommandExecutor->m_ExecuteQueue;
                isExecuteQueueEmpty = ExecuteQueue.empty();
                if ( !isExecuteQueueEmpty )
                    pCommand = ExecuteQueue.front(); // this shared_ptr might be empty
            }

            if ( !isExecuteQueueEmpty )
            {
                if ( pCommand )
                {
                    if ( !pCommand->IsExpired() )
                        pCommand->Execute();
                }

                {
                    CCriticalSectionLockGuard lock(pCommandExecutor->m_csExecuteQueue);
                    auto& ExecuteQueue = pCommandExecutor->m_ExecuteQueue;
                    if ( !ExecuteQueue.empty() ) // other thread may call m_ExecuteQueue.clear()
                        ExecuteQueue.pop_front();
                    isExecuteQueueEmpty = ExecuteQueue.empty();
                }

                // process the rest in the queue, if any
                if ( !isExecuteQueueEmpty )
                    pCommandExecutor->m_ExecuteCmdEvent.Set();
            }
        }
    }

    pCommandExecutor->m_ExecuteThreadDoneEvent.Set();
    return 0;
}

DWORD WINAPI CNppExecCommandExecutor::BackgroundExpiredThreadFunc(LPVOID lpParam)
{
    CNppExecCommandExecutor* pCommandExecutor = (CNppExecCommandExecutor *) lpParam;
    HANDLE waitEvents[2] = { pCommandExecutor->m_StopEvent.GetHandle(), pCommandExecutor->m_ExpiredCmdEvent.GetHandle() };

    for ( ; ; )
    {
        DWORD dwWaitEvent = ::WaitForMultipleObjects(2, waitEvents, FALSE, INFINITE);
        if ( dwWaitEvent == WAIT_OBJECT_0 ) // Stop Event
            break;

        if ( dwWaitEvent == WAIT_OBJECT_0 + 1 ) // ExpiredCmd Event
        {
            std::shared_ptr<ExpirableCommand> pCommand;
            bool isExpiredQueueEmpty;

            {
                CCriticalSectionLockGuard lock(pCommandExecutor->m_csExpiredQueue);
                auto& ExpiredQueue = pCommandExecutor->m_ExpiredQueue;
                isExpiredQueueEmpty = ExpiredQueue.empty();
                if ( !isExpiredQueueEmpty )
                    pCommand = ExpiredQueue.front(); // this shared_ptr might be empty
            }

            if ( !isExpiredQueueEmpty )
            {
                if ( pCommand )
                {
                    pCommand->Expire();
                }

                {
                    CCriticalSectionLockGuard lock(pCommandExecutor->m_csExpiredQueue);
                    auto& ExpiredQueue = pCommandExecutor->m_ExpiredQueue;
                    if ( !ExpiredQueue.empty() ) // other thread might call m_ExpiredQueue.clear()
                        ExpiredQueue.pop_front();
                    isExpiredQueueEmpty = ExpiredQueue.empty();
                }

                // process the rest in the queue, if any
                if ( !isExpiredQueueEmpty )
                    pCommandExecutor->m_ExpiredCmdEvent.Set();
            }
        }
    }

    pCommandExecutor->m_ExpiredThreadDoneEvent.Set();
    return 0;
}

void CNppExecCommandExecutor::Start()
{
    if ( m_StopEvent.IsNull() )
        m_StopEvent.Create(NULL, TRUE, FALSE, NULL);

    Runtime::GetLogger().Add_WithoutOutput( _T("; CNppExecCommandExecutor - started") );
}

void CNppExecCommandExecutor::Stop()
{
    if ( !m_StopEvent.IsNull() )
    {
        m_StopEvent.Set();

        // I'd prefer to wait for the thread handles here,
        // but apparently that did not work...

        if ( !m_ExecuteThreadDoneEvent.IsNull() )
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; CNppExecCommandExecutor: waiting for the ExecuteThreadDoneEvent...") );

            m_ExecuteThreadDoneEvent.Wait(INFINITE);
        }

        if ( !m_ExpiredThreadDoneEvent.IsNull() )
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; CNppExecCommandExecutor: waiting for the ExpiredThreadDoneEvent...") );

            m_ExpiredThreadDoneEvent.Wait(INFINITE);
        }
    }

    Runtime::GetLogger().Add_WithoutOutput( _T("; CNppExecCommandExecutor - stopped") );
}

void CNppExecCommandExecutor::ExecuteCommand(Command* cmd)
{
    {
        CCriticalSectionLockGuard lock(m_csExecuteQueue);

        if ( m_ExecuteCmdEvent.IsNull() )
            m_ExecuteCmdEvent.Create(NULL, FALSE, FALSE, NULL);

        if ( m_ExecuteThreadDoneEvent.IsNull() )
            m_ExecuteThreadDoneEvent.Create(NULL, TRUE, FALSE, NULL);

        if ( m_hBackgroundExecuteThread == NULL )
            NppExecHelpers::CreateNewThread(BackgroundExecuteThreadFunc, this, &m_hBackgroundExecuteThread);

        m_ExecuteQueue.push_back( std::shared_ptr<Command>(cmd) );
    }

    m_ExecuteCmdEvent.Set();
}

bool CNppExecCommandExecutor::ExecuteCollateralScript(const CListT<tstr>& scriptCmdList, const tstr& id, unsigned int nRunFlags)
{
    Runtime::GetLogger().Add_WithoutOutput( _T("; Executing a collateral script...") );

    bool bScriptThreadRunning = false;
    CScriptEngine* pScriptEngine = new CScriptEngine(m_pNppExec, scriptCmdList, id);
    if ( pScriptEngine )
    {
        CScriptEngineRunner* pScriptEngineRunner = new CScriptEngineRunner(pScriptEngine, nRunFlags);
        if ( pScriptEngineRunner )
        {
            bScriptThreadRunning = NppExecHelpers::CreateNewThread(dwRunCollateralScriptThread, pScriptEngineRunner);
            /*
            unsigned int nSyncTimeout = GetOptions().GetUint(OPTU_CHILDS_SYNCTIMEOUT_MS);
            HANDLE hScriptThread = NULL;
            if ( CreateNewThread(dwRunCollateralScriptThread, pScriptEngineRunner, &hScriptThread) )
            {
                // this imitates synchronous behavior for "quick" scripts

                // !!! This WaitForSingleObject will not work until it's done within the Console's message !!!
                // because of this, an alternative approach should be used instead...

                if ( ::WaitForSingleObject(hScriptThread, nSyncTimeout) != WAIT_OBJECT_0 )
                {
                    bScriptThreadRunning = true;
                }
                ::CloseHandle(hScriptThread); // we are not going to do anything else with this handle
            }
            */
        }
    }
    return bScriptThreadRunning;
}

void CNppExecCommandExecutor::CommandExpired(ExpirableCommand* cmd)
{
    std::shared_ptr<Command> pCommand;

    {
        CCriticalSectionLockGuard lock(m_csExecuteQueue);
        auto itr = std::find_if( m_ExecuteQueue.begin(), m_ExecuteQueue.end(), 
                                   [&cmd](const std::shared_ptr<Command>& pCmd) { return (pCmd.get() == cmd); } );
        if ( itr != m_ExecuteQueue.end() )
            pCommand = *itr;
    }

    if ( (!pCommand) || (pCommand->GetState() != ExpirableCommand::CmdIsReady) )
        return; // too late to expire
    
    {
        CCriticalSectionLockGuard lock(m_csExpiredQueue);

        if ( m_ExpiredCmdEvent.IsNull() )
            m_ExpiredCmdEvent.Create(NULL, FALSE, FALSE, NULL);

        if ( m_ExpiredThreadDoneEvent.IsNull() )
            m_ExpiredThreadDoneEvent.Create(NULL, TRUE, FALSE, NULL);

        if ( m_hBackgroundExpiredThread == NULL )
            NppExecHelpers::CreateNewThread(BackgroundExpiredThreadFunc, this, &m_hBackgroundExpiredThread);

        m_ExpiredQueue.push_back( std::static_pointer_cast<ExpirableCommand>(pCommand) );
    }

    m_ExpiredCmdEvent.Set();
}

void CNppExecCommandExecutor::ClearCommandQueue()
{
    Runtime::GetLogger().Add( _T("; CNppExecCommandExecutor: m_ExecuteQueue.clear() ...") );

    {
        CCriticalSectionLockGuard lock(m_csExecuteQueue);
        m_ExecuteQueue.clear();
    }

    Runtime::GetLogger().Add( _T("; CNppExecCommandExecutor: m_ExecuteQueue cleared") );
}

bool CNppExecCommandExecutor::IsScriptRunningOrQueued() const
{
    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);
        if ( m_RunningScriptEngine )
            return true;
    }

    {
        CCriticalSectionLockGuard lock(m_csExecuteQueue);
        if ( !m_ExecuteQueue.empty() )
        {
            std::shared_ptr<Command> pCommand = m_ExecuteQueue.front();
            if ( pCommand )
            {
                switch ( pCommand->GetType() )
                {
                    case Command::cmdExecDlg:
                    case Command::cmdDirectExec:
                    case Command::cmdExecScript:
                    case Command::cmdRunScript:
                        return true;
                }
            }
        }
    }

    return false;
}

bool CNppExecCommandExecutor::IsChildProcessRunning() const
{
    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);
        if ( m_RunningScriptEngine )
            return m_RunningScriptEngine->IsChildProcessRunning();
    }
    return false;
}

bool CNppExecCommandExecutor::IsCommandQueueEmpty() const
{
    CCriticalSectionLockGuard lock(m_csExecuteQueue);
    return m_ExecuteQueue.empty();
}

std::shared_ptr<CScriptEngine> CNppExecCommandExecutor::GetRunningScriptEngine()
{
    CCriticalSectionLockGuard lock(m_csRunningScriptEngine);
    return std::static_pointer_cast<CScriptEngine>(m_RunningScriptEngine);
}

std::list< std::shared_ptr<CScriptEngine> > CNppExecCommandExecutor::GetRunningScriptEngines(unsigned int nScriptEngineFlags)
{
    std::shared_ptr<CScriptEngine> pRunningScriptEngine;
    std::shared_ptr<CScriptEngine> pScriptEngine;
    std::list< std::shared_ptr<CScriptEngine> > ScriptEngines;

    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);

        pRunningScriptEngine = std::static_pointer_cast<CScriptEngine>(m_RunningScriptEngine);
        if ( pRunningScriptEngine )
        {
            auto includeThisScriptEngine = [nScriptEngineFlags](const std::shared_ptr<CScriptEngine>& pScrptEngn)
            {
                return ( ((nScriptEngineFlags & (sefExcludingCollateral | sefOnlyCollateral | sefOnlyRunningChildProc)) == 0) ||
                         (((nScriptEngineFlags & sefExcludingCollateral) != 0) && !pScrptEngn->IsCollateral()) ||
                         (((nScriptEngineFlags & sefOnlyCollateral) != 0) && pScrptEngn->IsCollateral()) ||
                         (((nScriptEngineFlags & sefOnlyRunningChildProc) != 0) && pScrptEngn->IsChildProcessRunning()) );
            };

            if ( (nScriptEngineFlags & sefExcludingCurrent) == 0 )
            {
                // note: when we get here, we already verified that pRunningScriptEngine is not nullptr
                if ( includeThisScriptEngine(pRunningScriptEngine) )
                    ScriptEngines.push_back(pRunningScriptEngine);
            }

            pScriptEngine = pRunningScriptEngine->GetParentScriptEngine();
            while ( pScriptEngine )
            {
                // note: when we get here, we already verified that pScriptEngine is not nullptr
                if ( includeThisScriptEngine(pScriptEngine) )
                    ScriptEngines.push_back(pScriptEngine);
                pScriptEngine = pScriptEngine->GetParentScriptEngine();
            }

            pScriptEngine = pRunningScriptEngine->GetChildScriptEngine();
            while ( pScriptEngine )
            {
                // note: when we get here, we already verified that pScriptEngine is not nullptr
                if ( includeThisScriptEngine(pScriptEngine) )
                    ScriptEngines.push_front(pScriptEngine);
                pScriptEngine = pScriptEngine->GetChildScriptEngine();
            }
        }
    }

    return ScriptEngines;
}

void CNppExecCommandExecutor::RunScriptEngine(std::shared_ptr<CScriptEngine> pScriptEngine, unsigned int nRunFlags)
{
    // before the execution...
    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);

        std::shared_ptr<CScriptEngine> pParentScriptEngine = std::static_pointer_cast<CScriptEngine>(m_RunningScriptEngine);
        if ( pParentScriptEngine && !pParentScriptEngine->IsDone() )
        {
            pParentScriptEngine->SetChildScriptEngine(pScriptEngine);
            pScriptEngine->SetParentScriptEngine(pParentScriptEngine);
        }
        else
        {
            pScriptEngine->SetParentScriptEngine(std::shared_ptr<CScriptEngine>());
        }
        m_RunningScriptEngine = pScriptEngine;
    }

    // the execution...
    pScriptEngine->Run(nRunFlags);

    // after the execution...
    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);

        if ( m_RunningScriptEngine == pScriptEngine )
        {
            std::shared_ptr<CScriptEngine> pParentScriptEngine = pScriptEngine->GetParentScriptEngine();
            std::shared_ptr<CScriptEngine> pChildScriptEngine = pScriptEngine->GetChildScriptEngine();

            while ( pChildScriptEngine && pChildScriptEngine->IsDone() ) // get rid of finished engines
            {
                pChildScriptEngine = pChildScriptEngine->GetChildScriptEngine();
            }

            while ( pParentScriptEngine && pParentScriptEngine->IsDone() ) // get rid of finished engines
            {
                pParentScriptEngine = pParentScriptEngine->GetParentScriptEngine();
            }

            if ( pChildScriptEngine && !pChildScriptEngine->IsDone() )
            {
                pChildScriptEngine->DoNotShareLocalVars(); // they should be shared with pScriptEngine, not with its parent
                pChildScriptEngine->SetParentScriptEngine(pParentScriptEngine);
                if ( pParentScriptEngine && !pParentScriptEngine->IsDone() )
                {
                    pParentScriptEngine->SetChildScriptEngine(pChildScriptEngine);
                }
                m_RunningScriptEngine = pChildScriptEngine;
            }
            else if ( pParentScriptEngine && !pParentScriptEngine->IsDone() )
            {
                pParentScriptEngine->SetChildScriptEngine(std::shared_ptr<CScriptEngine>());
                m_RunningScriptEngine = pParentScriptEngine;
            }
            else
            {
                m_RunningScriptEngine = std::shared_ptr<CScriptEngine>();
            }
        }
        else
        {
            std::shared_ptr<CScriptEngine> pParentScriptEngine = pScriptEngine->GetParentScriptEngine();
            std::shared_ptr<CScriptEngine> pChildScriptEngine = pScriptEngine->GetChildScriptEngine();

            if ( pParentScriptEngine )
            {
                pParentScriptEngine->SetChildScriptEngine(pChildScriptEngine);
            }

            if ( pChildScriptEngine )
            {
                pChildScriptEngine->DoNotShareLocalVars(); // they should be shared with pScriptEngine, not with its parent
                pChildScriptEngine->SetParentScriptEngine(pParentScriptEngine);
            }
        }
    }

    if ( nRunFlags & CScriptEngine::rfShareLocalVars )
    {
        std::shared_ptr<CScriptEngine> pRunningScriptEngine = GetRunningScriptEngine();
        if ( pRunningScriptEngine )
        {
            std::shared_ptr<CChildProcess> pChildProc = pRunningScriptEngine->GetRunningChildProcess();
            if ( pChildProc )
            {
                // if there is a running process, restore its $(PID)
                DWORD dwProcId = pChildProc->GetProcessId();
                TCHAR szProcessId[50];
                c_base::_tint2str(dwProcId, szProcessId);
                tstr varName = MACRO_PID;
                m_pNppExec->GetMacroVars().SetUserMacroVar( pRunningScriptEngine.get(), varName, szProcessId, CNppExecMacroVars::svLocalVar ); // local var
            }
        }
    }
}

std::shared_ptr<CChildProcess> CNppExecCommandExecutor::GetRunningChildProcess()
{
    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);
        if ( m_RunningScriptEngine )
            return m_RunningScriptEngine->GetRunningChildProcess();
    }
    return std::shared_ptr<CChildProcess>();
}

bool CNppExecCommandExecutor::GetTriedExitCmd() const
{
    {
        CCriticalSectionLockGuard lock(m_csRunningScriptEngine);
        if ( m_RunningScriptEngine )
            return m_RunningScriptEngine->GetTriedExitCmd();
    }
    return true;
}

void CNppExecCommandExecutor::SetTriedExitCmd(bool bTriedExitCmd)
{
    CCriticalSectionLockGuard lock(m_csRunningScriptEngine);
    if ( m_RunningScriptEngine )
        m_RunningScriptEngine->SetTriedExitCmd(bTriedExitCmd);
}

void CNppExecCommandExecutor::ExecuteChildProcessCommand(tstr& cmd)
{
    Runtime::GetLogger().AddEx_WithoutOutput( _T_RE_EOL _T("; @Child Process\'es Input: %s"), cmd.c_str() );
    Runtime::GetLogger().Add( _T("") );

    bool bScriptThreadRunning = false;
    // We don't call StrDelLeadingTabSpaces here as the leading space(s)
    // may be a meaningful part of a command given to the child process.
    CScriptEngine::eNppExecCmdPrefix cmdPrefix = CScriptEngine::checkNppExecCmdPrefix(m_pNppExec, cmd);
    if ( cmdPrefix != CScriptEngine::CmdPrefixNone )
    {
        tstr s = cmd;
        const CScriptEngine::eCmdType cmdType = CScriptEngine::getCmdType(m_pNppExec, s, CScriptEngine::ctfIgnorePrefix);

        if ( (cmdType != CScriptEngine::CMDTYPE_NPPSENDMSG) &&
             (cmdType != CScriptEngine::CMDTYPE_NPPSENDMSGEX) &&
             (cmdType != CScriptEngine::CMDTYPE_SCISENDMSG) &&
             (cmdType != CScriptEngine::CMDTYPE_SCIFIND) &&
             (cmdType != CScriptEngine::CMDTYPE_SCIREPLACE) &&
             (cmdType != CScriptEngine::CMDTYPE_NPECMDALIAS) &&
             (cmdType != CScriptEngine::CMDTYPE_NPEQUEUE) &&
             (cmdType != CScriptEngine::CMDTYPE_CONFILTER) )
        {
            m_pNppExec->GetMacroVars().CheckCmdAliases(cmd, true);
            m_pNppExec->GetMacroVars().CheckAllMacroVars(nullptr, cmd, true);
        }
    }
    else
    {
        m_pNppExec->GetMacroVars().CheckCmdAliases(cmd, true);
        m_pNppExec->GetMacroVars().CheckAllMacroVars(nullptr, cmd, true);

        cmdPrefix = CScriptEngine::checkNppExecCmdPrefix(m_pNppExec, cmd);
    }

    if ( cmdPrefix != CScriptEngine::CmdPrefixNone )
    {
        tstr S1 = cmd;
        NppExecHelpers::StrUpper(S1);

        if ( ConsoleDlg::IsConsoleHelpCommand(S1) || 
             ConsoleDlg::IsConsoleVerCommand(S1) )
        {
        }
        else
        {
            CListT<tstr> scriptCmdList(cmd);
            unsigned int nRunFlags = CScriptEngine::rfCollateralScript;
            if ( cmdPrefix == CScriptEngine::CmdPrefixCollateralOrRegular )
                nRunFlags |= (CScriptEngine::rfShareLocalVars | CScriptEngine::rfShareConsoleState);
            bScriptThreadRunning = ExecuteCollateralScript(scriptCmdList, tstr(), nRunFlags);
        }
    }
    else
    {
        WriteChildProcessInput( cmd.c_str() );
    }

    if ( !bScriptThreadRunning )
    {
        WriteChildProcessInput( m_pNppExec->GetOptions().GetStr(OPTS_KEY_ENTER) );
          // "\n" must be sent separately in some cases - ask M$ why
          // or, in case of NppExecCmdPrefix, it sends "\n" to show child process'es prompt

        m_pNppExec->GetConsole().LockConsoleEndPosAfterEnterPressed();
    }
}

bool CNppExecCommandExecutor::WriteChildProcessInput(const TCHAR* szLine, bool bFFlush )
{
    std::shared_ptr<CChildProcess> pChildProc = GetRunningChildProcess();
    return (pChildProc ? pChildProc->WriteInput(szLine, bFFlush) : false);
}

void CNppExecCommandExecutor::ChildProcessMustBreak(unsigned int nBreakMethod)
{
    std::shared_ptr<CChildProcess> pChildProc = GetRunningChildProcess();
    if ( pChildProc )
        pChildProc->MustBreak(nBreakMethod);
}

void CNppExecCommandExecutor::ChildProcessMustBreakAll(unsigned int /*nBreakMethod*/)
{
    std::shared_ptr<CScriptEngine> pScriptEngine = GetRunningScriptEngine();
    if ( pScriptEngine )
        pScriptEngine->ChildProcessMustBreakAll();
}

bool CNppExecCommandExecutor::WaitUntilAllScriptEnginesDone(DWORD dwTimeoutMs)
{
    std::shared_ptr<CScriptEngine> pScriptEngine;
    while ( (pScriptEngine = GetRunningScriptEngine()) )
    {
        if ( !pScriptEngine->WaitUntilDone(dwTimeoutMs) ) // OK, if we don't want this to be locked by N++ GUI, we _must_ execute everything from a worker thread
            return false;
    }
    return true;
}

//-------------------------------------------------------------------------

CNppExecCommandExecutor::RunningScriptEnginesStopper::RunningScriptEnginesStopper(CNppExecCommandExecutor* pExecutor, unsigned int nCloseConsoleFlags)
  : m_pExecutor(pExecutor)
  , m_nCloseConsoleFlags(nCloseConsoleFlags)
{
}

void CNppExecCommandExecutor::RunningScriptEnginesStopper::SendAbortNotificationToRunningScripts(const TCHAR* cszMessage, bool includingCurrentScript)
{
    Runtime::GetLogger().Add_WithoutOutput( _T("; RunningScriptEnginesStopper - sending abort notification to running scripts...") );

    int nScriptEngineFlags = 0;
    if ( !includingCurrentScript )
    {
        m_CurrentRunningScriptEngine = m_pExecutor->GetRunningScriptEngine(); // to be processed separately
        nScriptEngineFlags = CNppExecCommandExecutor::sefExcludingCurrent;
    }

    m_RunningScriptEngines = m_pExecutor->GetRunningScriptEngines(nScriptEngineFlags);
    if ( !m_RunningScriptEngines.empty() )
    {
        if ( (m_nCloseConsoleFlags & ConsoleDlg::ccfCmdNppConsole) != 0 )
        {
            // excluding those script engines that are closing the Console via "npp_console off"
            m_RunningScriptEngines.remove_if( 
                [](const std::shared_ptr<CScriptEngine>& pScriptEngine) { return (pScriptEngine->IsClosingConsole()); } 
            );
        }
        for ( auto& pScriptEngine : m_RunningScriptEngines )
        {
            pScriptEngine->ScriptError(IScriptEngine::ET_ABORT, cszMessage);
        }
    }
}

bool CNppExecCommandExecutor::RunningScriptEnginesStopper::WaitUntilRunningScriptsDone(DWORD dwTimeoutMs)
{
    Runtime::GetLogger().Add_WithoutOutput( _T("; RunningScriptEnginesStopper - waiting until running scripts are done...") );

    bool bAllDone = m_RunningScriptEngines.empty();
    if ( !bAllDone )
    {
        DWORD dwTickCountDiff = 0;
        DWORD dwStartTickCount = ::GetTickCount();

        while ( dwTickCountDiff < dwTimeoutMs )
        {
            for ( auto& pScriptEngine : m_RunningScriptEngines )
            {
                bAllDone = pScriptEngine->WaitUntilDone(20);
                if ( !bAllDone )
                    break;
            }
        
            if ( !bAllDone )
            {
                dwTickCountDiff = ::GetTickCount();
                if ( dwTickCountDiff <= dwStartTickCount )
                {
                    // DWORD overflowed
                    dwTickCountDiff += (0xFFFFFFFF - dwStartTickCount);
                    dwTickCountDiff += 1;
                    dwStartTickCount = 0;
                }
                else
                    dwTickCountDiff -= dwStartTickCount;
            }
            else
                break;
        }
    }

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; RunningScriptEnginesStopper - done (result: %s)"), bAllDone ? _T("OK") : _T("timed out") );

    return bAllDone;
}

void CNppExecCommandExecutor::RunningScriptEnginesStopper::SendAbortNotificationToCurrentRunningScript(const TCHAR* cszMessage)
{
    if ( !m_CurrentRunningScriptEngine )
        return;

    Runtime::GetLogger().Add_WithoutOutput( _T("; RunningScriptEnginesStopper - sending abort notification to the current running script...") );

    m_CurrentRunningScriptEngine->ScriptError(IScriptEngine::ET_ABORT, cszMessage);
}

bool CNppExecCommandExecutor::RunningScriptEnginesStopper::WaitUntilCurrentRunningScriptDone(DWORD dwTimeoutMs)
{
    if ( !m_CurrentRunningScriptEngine )
        return true;

    Runtime::GetLogger().Add_WithoutOutput( _T("; RunningScriptEnginesStopper - waiting until the current running script is done...") );

    bool bIsDone = m_CurrentRunningScriptEngine->WaitUntilDone(dwTimeoutMs);

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; RunningScriptEnginesStopper - done (result: %s)"), bIsDone ? _T("OK") : _T("timed out") );

    return bIsDone;
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::Command ...
CNppExecCommandExecutor::Command::Command(const tstr& id, eCommandType type) : m_id(id), m_type(type)
{
    m_strInstance = NppExecHelpers::GetInstanceAsString(this);

    m_Executor = &Runtime::GetNppExec().GetCommandExecutor();

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - create (instance = %s)"), GetTypeStr(), GetInstanceStr() );
}

CNppExecCommandExecutor::Command::~Command()
{
    Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - destroy (instance = %s)"), GetTypeStr(), GetInstanceStr() );
}

const TCHAR* CNppExecCommandExecutor::Command::GetTypeStr() const
{
    const TCHAR* cszTypeStr;
    switch ( m_type )
    {
        case cmdExpirable:
            cszTypeStr = _T("Expirable");
            break;
        case cmdScriptable:
            cszTypeStr = _T("Scriptable");
            break;
        case cmdCloseConsole:
            cszTypeStr = _T("CloseConsole");
            break;
        case cmdExecDlg:
            cszTypeStr = _T("ExecDlg");
            break;
        case cmdDirectExec:
            cszTypeStr = _T("DirectExec");
            break;
        case cmdExecScript:
            cszTypeStr = _T("ExecScript");
            break;
        case cmdRunScript:
            cszTypeStr = _T("RunScript");
            break;
        case cmdNppExit:
            cszTypeStr = _T("NppExit");
            break;
        case cmdTryExitChildProc:
            cszTypeStr = _T("TryExitChildProc");
            break;
        default:
            cszTypeStr = _T("Unknown");
            break;
    }
    return cszTypeStr;
}

const TCHAR* CNppExecCommandExecutor::Command::GetInstanceStr() const
{
    return m_strInstance.c_str();
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::ExpirableCommand ...
CNppExecCommandExecutor::ExpirableCommand::ExpirableCommand(const tstr& id, eCommandType type, eExpirableType expirableType)
  : Command(id, type)
  , m_expirableType(expirableType)
  , m_nState(CmdIsReady)
  , m_idExpirationTimer(0)
{
    if ( m_expirableType == Expirable )
    {
        const unsigned int nSyncTimeout = Runtime::GetNppExec().GetOptions().GetUint(OPTU_CHILDS_SYNCTIMEOUT_MS);
        startExpirationTimer(nSyncTimeout);
    }
}

CNppExecCommandExecutor::ExpirableCommand::~ExpirableCommand()
{
    if ( m_expirableType == Expirable )
    {
        stopExpirationTimer();
    }
}

void CNppExecCommandExecutor::ExpirableCommand::Execute()
{
    LONG nPrevState = ::InterlockedCompareExchange(&m_nState, CmdIsRunning, CmdIsReady);
    if ( nPrevState == CmdIsCheckingIfExpired )
    {
        m_CheckIfExpiredEvent.Wait(INFINITE); // wait for the decision
        nPrevState = ::InterlockedCompareExchange(&m_nState, CmdIsRunning, CmdIsReady);
    }

    if ( nPrevState == CmdIsReady )
    {
        if ( m_expirableType == Expirable )
        {
            stopExpirationTimer();
        }

        Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - executing (instance = %s)"), GetTypeStr(), GetInstanceStr() );

        subExecute();
        ::InterlockedCompareExchange(&m_nState, CmdIsDone, CmdIsRunning);
    }
}

void CNppExecCommandExecutor::ExpirableCommand::Expire()
{
    ::InterlockedCompareExchange(&m_nState, CmdIsExpired, CmdIsReady);
}

void CALLBACK CNppExecCommandExecutor::ExpirableCommand::ExpirationTimerProc(HWND /*hWnd*/, 
                UINT /*uMsg*/, UINT_PTR idEvent, DWORD /*dwTime*/)
{
    ExpirableCommand* pCommand = NULL;

    {
        CCriticalSectionLockGuard lock(s_csExpirableCommands);
        std::map<UINT_PTR, ExpirableCommand*>::iterator itr = s_ExpirableCommands.find(idEvent);
        if ( itr != s_ExpirableCommands.end() )
            pCommand = itr->second;
    }

    if ( pCommand != NULL )
    {
        pCommand->stopExpirationTimer();
        pCommand->GetExecutor()->CommandExpired(pCommand); // the Executor will do it in another thread
    }
    else if ( idEvent != 0 )
    {
        {
            CCriticalSectionLockGuard lock(s_csExpirableCommands);
            s_ExpirableCommands.erase(idEvent);
        }

        ::KillTimer(NULL, idEvent);
    }
}

void CNppExecCommandExecutor::ExpirableCommand::startExpirationTimer(unsigned int uMaxTimeout)
{
    stopExpirationTimer(); // just in case
    m_idExpirationTimer = ::SetTimer(NULL, 0, uMaxTimeout, ExpirationTimerProc);

    {
        CCriticalSectionLockGuard lock(s_csExpirableCommands);
        s_ExpirableCommands.insert(std::make_pair(m_idExpirationTimer, this));
    }
}

void CNppExecCommandExecutor::ExpirableCommand::stopExpirationTimer()
{
    if ( m_idExpirationTimer != 0 )
    {
        {
            CCriticalSectionLockGuard lock(s_csExpirableCommands);
            s_ExpirableCommands.erase(m_idExpirationTimer);
        }

        ::KillTimer(NULL, m_idExpirationTimer);
        m_idExpirationTimer = 0;
    }
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::ScriptableCommand ...
CNppExecCommandExecutor::ScriptableCommand::ScriptableCommand(const tstr& id, eCommandType type, eExpirableType expirableType)
  : ExpirableCommand(id, type, expirableType)
{
}

CNppExecCommandExecutor::ScriptableCommand::~ScriptableCommand()
{
}

DWORD CNppExecCommandExecutor::ScriptableCommand::RunConsoleScript(Command* pCommand, const CListT<tstr>& CmdList, unsigned int nRunFlags)
{
    CNppExec* pNppExec = pCommand->GetExecutor()->GetNppExec();
    
    pNppExec->m_hFocusedWindowBeforeScriptStarted = ::GetFocus();

    std::shared_ptr<CScriptEngine> pScriptEngine(new CScriptEngine(pNppExec, CmdList, pCommand->GetId()));
    pCommand->GetExecutor()->RunScriptEngine(pScriptEngine, nRunFlags);

    if ( pScriptEngine->GetCmdList().GetCount() > 0 )
    {
        const int nLastCmdType = pScriptEngine->GetLastCmdType();
        if ( nLastCmdType != CScriptEngine::CMDTYPE_CLS )
        {
            if ( /*(nLastCmdType != CScriptEngine::CMDTYPE_NPPCONSOLE) ||
                 (CScriptEngine::getOnOffParam(pScriptEngine->GetLastCmdParams()) != CScriptEngine::PARAM_DISABLE) || 
                 (pNppExec->GetConsole().IsOutputEnabledN() > 1)*/ 
                 pNppExec->GetConsole().IsOutputEnabledN() != 0 )
            {
                if ( !pNppExec->GetCommandExecutor().GetRunningScriptEngine() )
                {
                    pNppExec->GetConsole().PrintMessage( _T("================ READY ================"), false );
                }
                ConsoleDlg::GoToError_nCurrentLine = -1;
            }

            if ( !pNppExec->GetCommandExecutor().GetRunningScriptEngine() )
            {
                pNppExec->GetConsole().RestoreDefaultTextStyle(true);
            }
        }
    }

    if ( pCommand->GetExecutor()->IsChildProcessRunning() )
    {
        DWORD scrptEngnId = 0;
        std::shared_ptr<CScriptEngine> pParentScriptEngine = pScriptEngine->GetParentScriptEngine();
        while ( pParentScriptEngine && pParentScriptEngine->IsDone() )
            pParentScriptEngine = pParentScriptEngine->GetParentScriptEngine();
        if ( pParentScriptEngine )
            scrptEngnId = pParentScriptEngine->GetThreadId();
        if ( pNppExec->GetConsole().GetOutputEnabledDirectly(scrptEngnId) != 0 )
        {
            pCommand->GetExecutor()->WriteChildProcessInput( pNppExec->GetOptions().GetStr(OPTS_KEY_ENTER) );
              // sends "\n" to show child process'es prompt

            pNppExec->GetConsole().LockConsoleEndPosAfterEnterPressed();
        }
    }
  
    pNppExec->GetPluginInterfaceImpl().NotifyExternalPluginResult(pCommand->GetId(), pScriptEngine->IsAborted() ? NPE_RESULT_ABORTED : NPE_RESULT_OK);

    return 0;
}

void CNppExecCommandExecutor::ScriptableCommand::OnDoExecDlg()
{
    CNppExec* pNppExec = GetExecutor()->GetNppExec();

    if ( pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SAVEONEXECUTE) )
    {
        pNppExec->nppSaveAllFiles();
    }

    if ( pNppExec->PluginDialogBox(IDD_DOEXEC, DoExecDlgProc) == 1 )
    {
        if ( pNppExec->IsCmdListEmpty() )
        {
            // adding empty command if m_ScriptCmdList is empty 
            pNppExec->SetCmdList( CListT<tstr>( tstr(_T("")) ) );
            // now m_ScriptCmdList is not empty ;-)
        }
        OnDirectExec(false, 0);
    }
}

void CNppExecCommandExecutor::ScriptableCommand::OnDirectExec(bool bCanSaveAll, unsigned int nRunFlags)
{
    CNppExec* pNppExec = GetExecutor()->GetNppExec();

    CListT<tstr> CmdList = pNppExec->GetCmdList();
    CListItemT<tstr>* p = CmdList.GetFirst();
    if ( !p )
    {
        // m_ScriptCmdList is empty (no previous script executed)
        if ( !CNppExec::_bIsNppShutdown )
        {
            OnDoExecDlg();
        }
        return;
    }
    else
    {
        // OK, m_ScriptCmdList is not empty
        if (bCanSaveAll && pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SAVEONEXECUTE))
        {
            pNppExec->nppSaveAllFiles();
        }
    }

    if ( pNppExec->checkCmdListAndPrepareConsole(CmdList) )
    {
        DoRunScript(CmdList, nRunFlags);
    }
}

void CNppExecCommandExecutor::ScriptableCommand::DoExecScript(LPCTSTR szScriptName, LPCTSTR szScriptArguments, bool bCanSaveAll, unsigned int nRunFlags)
{
    CNppExec* pNppExec = GetExecutor()->GetNppExec();

    if ( szScriptName && szScriptName[0] )
    {
        if ( nRunFlags & CScriptEngine::rfExitScript )
        {
            Runtime::GetLogger().Add/*_WithoutOutput*/(   _T("; Exit Script - started") );
        }
        else if ( nRunFlags & CScriptEngine::rfStartScript )
        {
            Runtime::GetLogger().Add/*_WithoutOutput*/(   _T("; Start Script - started") );
        }

        tstr S = _T("NPP_EXEC \"");
        S += szScriptName;
        S += _T("\""); //  \"\" are necessary: script name may contain spaces!!!

        if ( szScriptArguments && szScriptArguments[0] )
        {
            S += _T(" ");
            S += szScriptArguments;
        }

        pNppExec->SetCmdList( CListT<tstr>( S ) );
        OnDirectExec(bCanSaveAll, nRunFlags);

        if ( nRunFlags & CScriptEngine::rfExitScript )
        {
            Runtime::GetLogger().AddEx_WithoutOutput( _T("; Exit Script - %s"), pNppExec->_bStopTheExitScript ? _T("aborted") : _T("finished") );
        }
        else if ( nRunFlags & CScriptEngine::rfStartScript )
        {
            Runtime::GetLogger().Add/*_WithoutOutput*/(   _T("; Start Script - finished") );
        }
    }
}

void CNppExecCommandExecutor::ScriptableCommand::DoRunScript(const CListT<tstr>& CmdList, unsigned int nRunFlags)
{
    ::InterlockedIncrement( &GetExecutor()->m_nRunningScripts );
    RunConsoleScript( this, CmdList, nRunFlags );
    ::InterlockedDecrement( &GetExecutor()->m_nRunningScripts );
}

void CNppExecCommandExecutor::ScriptableCommand::DoCloseConsole(unsigned int nCloseConsoleFlags)
{
    CNppExec* pNppExec = GetExecutor()->GetNppExec();

    unsigned int nShowFlags = 0;
    if ( (nCloseConsoleFlags & ConsoleDlg::ccfCmdNppConsole) != 0 )
        nShowFlags |= CNppExec::scfCmdNppConsole;
    pNppExec->showConsoleDialog(CNppExec::hideAlways, nShowFlags);
    ::SetFocus( pNppExec->GetScintillaHandle() );
}

void CNppExecCommandExecutor::ScriptableCommand::DoNppExit()
{
    CNppExec* pNppExec = GetExecutor()->GetNppExec();

    if ( GetExecutor()->IsChildProcessRunning() )
    {
        // Usually we get here from NppExitCommand::Expire(), when the child
        // process is running in a regular NppExec's script.
        // Also we can get here from NppExitCommand::subExecute() when there
        // is no regular NppExec's script running and the child process is
        // running in a collateral NppExec's script.

        bool bDoClose = true;

        if ( !pNppExec->TryExitRunningChildProcess() ) // still running?
        {
            bool bConsoleOutputEnabled = pNppExec->GetConsole().IsOutputEnabled();

            // disable the Console output to avoid message locking in GUI
            pNppExec->GetConsole().SetOutputEnabled(false);

            Runtime::GetLogger().Add_WithoutOutput( _T("; Console output was disabled") );

            int nClose = ::MessageBox(pNppExec->m_nppData._nppHandle, _T("Child process is still active.\n") \
                _T("It will be killed if you close Notepad++.\nClose the program?"), 
                _T("Notepad++ : NppExec Warning"), MB_YESNO | MB_ICONWARNING);
        
            if ( nClose == IDYES )
            {
                GetExecutor()->ChildProcessMustBreakAll(CProcessKiller::killCtrlBreak);
                
                unsigned int uMaxTimeout = pNppExec->GetOptions().GetUint(OPTU_CHILDP_EXITTIMEOUT_MS);
                GetExecutor()->WaitUntilAllScriptEnginesDone(uMaxTimeout);
            }
            else
            {
                auto ScriptEngines = GetExecutor()->GetRunningScriptEngines(CNppExecCommandExecutor::sefOnlyRunningChildProc);
                for ( auto& pScriptEngine : ScriptEngines )
                {
                    pScriptEngine->UndoAbort( _T("Cancelling NppExit") );
                }

                // restore the Console output state
                pNppExec->GetConsole().SetOutputEnabled(bConsoleOutputEnabled);

                Runtime::GetLogger().Add_WithoutOutput( _T("; Console output was restored") );

                // 'No' pressed
                bDoClose = false;

                // send "\n" to show child process'es prompt
                GetExecutor()->WriteChildProcessInput( pNppExec->GetOptions().GetStr(OPTS_KEY_ENTER) );
            }
        }
    
        if ( !bDoClose )
        {
            if ( !pNppExec->isConsoleDialogVisible() )
                pNppExec->showConsoleDialog(CNppExec::showIfHidden, 0);
            ::SetFocus( pNppExec->GetConsole().GetConsoleWnd() );
            return;
        }
    }
    else if ( GetExecutor()->IsScriptRunningOrQueued() )
    {
        // stop the script
        std::shared_ptr<CScriptEngine> pScriptEngine = GetExecutor()->GetRunningScriptEngine();
        if ( pScriptEngine )
        {
            CNppExec::_bIsNppShutdown = true;
            pScriptEngine->ScriptError( CScriptEngine::ET_ABORT, _T("; Exiting, stopping the script (from DoNppExit())") );
        }
    }

    CNppExec::_bIsNppShutdown = true;
    GetExecutor()->ClearCommandQueue();
    pNppExec->SendNppMsg(WM_CLOSE, 0, 0);
}

void CNppExecCommandExecutor::ScriptableCommand::TryExitChildProcess(bool bCloseConsoleOnExit, bool bNppIsClosing)
{
    CNppExec* pNppExec = GetExecutor()->GetNppExec();

    if ( pNppExec->TryExitRunningChildProcess() )
    {
        if ( bCloseConsoleOnExit )
            DoCloseConsole(bNppIsClosing);
    }
    else
    {
        auto ScriptEngines = GetExecutor()->GetRunningScriptEngines(CNppExecCommandExecutor::sefOnlyRunningChildProc);
        for ( auto& pScriptEngine : ScriptEngines )
        {
            pScriptEngine->UndoAbort( _T("Cancelling TryExitChildProcess") );
        }
    }
}

void CNppExecCommandExecutor::ScriptableCommand::Expire()
{
    m_CheckIfExpiredEvent.Create(NULL, FALSE, FALSE, NULL);

    if ( ::InterlockedCompareExchange(&m_nState, CmdIsCheckingIfExpired, CmdIsReady) == CmdIsReady )
    {
        Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring... (instance = %s)"), GetTypeStr(), GetInstanceStr() );

        if ( CanStartScriptOrCommand() )
        {
            ::InterlockedCompareExchange(&m_nState, CmdIsReady, CmdIsCheckingIfExpired);

            Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring -> ready (instance = %s)"), GetTypeStr(), GetInstanceStr() );
        }
        else
        {
            ::InterlockedCompareExchange(&m_nState, CmdIsExpired, CmdIsCheckingIfExpired);

            Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring -> expired (instance = %s)"), GetTypeStr(), GetInstanceStr() );
        }

        m_CheckIfExpiredEvent.Set();
    }
}

bool CNppExecCommandExecutor::ScriptableCommand::CanStartScriptOrCommand()
{
    if ( CNppExec::_bIsNppShutdown )
        return false;

    CNppExec* pNppExec = GetExecutor()->GetNppExec();
    return pNppExec->CanStartScriptOrCommand();
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::DoCloseConsoleCommand ...
CNppExecCommandExecutor::DoCloseConsoleCommand::DoCloseConsoleCommand(const tstr& id, unsigned int nCloseConsoleFlags, eExpirableType expirableType)
  : ScriptableCommand(id, cmdCloseConsole, expirableType)
  , m_nCloseConsoleFlags(nCloseConsoleFlags)
  , m_ScriptsStopper(GetExecutor(), nCloseConsoleFlags)
{
    m_ScriptsStopper.SendAbortNotificationToRunningScripts( _T("; Aborting the script to close the Console (from DoCloseConsoleCommand())") );
}

void CNppExecCommandExecutor::DoCloseConsoleCommand::subExecute()
{
    const unsigned int nSyncTimeout = Runtime::GetNppExec().GetOptions().GetUint(OPTU_CHILDS_SYNCTIMEOUT_MS);
    m_ScriptsStopper.WaitUntilRunningScriptsDone(nSyncTimeout);

    DoCloseConsole(m_nCloseConsoleFlags);
}

void CNppExecCommandExecutor::DoCloseConsoleCommand::Expire()
{
    m_CheckIfExpiredEvent.Create(NULL, FALSE, FALSE, NULL);

    if ( ::InterlockedCompareExchange(&m_nState, CmdIsCheckingIfExpired, CmdIsReady) == CmdIsReady )
    {
        std::shared_ptr<CScriptEngine> pRunningScriptEngine = GetExecutor()->GetRunningScriptEngine();
        if ( pRunningScriptEngine )
        {
            pRunningScriptEngine->ScriptError(IScriptEngine::ET_ABORT, _T("; Aborting the script (from DoCloseConsoleCommand::Expire())"));
        }

        Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring... (instance = %s)"), GetTypeStr(), GetInstanceStr() );

        if ( CanStartScriptOrCommand() )
        {
            ::InterlockedCompareExchange(&m_nState, CmdIsReady, CmdIsCheckingIfExpired);

            Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring -> ready (instance = %s)"), GetTypeStr(), GetInstanceStr() );
        }
        else
        {
            ::InterlockedCompareExchange(&m_nState, CmdIsExpired, CmdIsCheckingIfExpired);

            Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring -> expired (instance = %s)"), GetTypeStr(), GetInstanceStr() );

            auto ScriptEngines = GetExecutor()->GetRunningScriptEngines(CNppExecCommandExecutor::sefOnlyRunningChildProc);
            for ( auto& pScriptEngine : ScriptEngines )
            {
                pScriptEngine->UndoAbort( _T("Cancelling CloseConsoleCommand") );
            }
        }

        m_CheckIfExpiredEvent.Set();
    }
}

bool CNppExecCommandExecutor::DoCloseConsoleCommand::CanStartScriptOrCommand()
{
    if ( CNppExec::_bIsNppShutdown )
        return true;
    
    CNppExec* pNppExec = GetExecutor()->GetNppExec();
    return pNppExec->CanStartScriptOrCommand(CNppExec::sfDoNotShowWarningOnScript);
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::OnDoExecDlgCommand ...
CNppExecCommandExecutor::OnDoExecDlgCommand::OnDoExecDlgCommand(const tstr& id, eExpirableType expirableType)
  : ScriptableCommand(id, cmdExecDlg, expirableType)
{
}

void CNppExecCommandExecutor::OnDoExecDlgCommand::subExecute()
{
    OnDoExecDlg();
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::OnDirectExecCommand ...
CNppExecCommandExecutor::OnDirectExecCommand::OnDirectExecCommand(const tstr& id, bool bCanSaveAll, unsigned int nRunFlags, eExpirableType expirableType)
  : ScriptableCommand(id, cmdDirectExec, expirableType)
  , m_bCanSaveAll(bCanSaveAll)
  , m_nRunFlags(nRunFlags)
{
}

void CNppExecCommandExecutor::OnDirectExecCommand::subExecute()
{
    OnDirectExec( m_bCanSaveAll, m_nRunFlags );
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::DoExecScriptCommand ...
CNppExecCommandExecutor::DoExecScriptCommand::DoExecScriptCommand(const tstr& id, LPCTSTR szScriptName, LPCTSTR szScriptArguments, bool bCanSaveAll, unsigned int nRunFlags, eExpirableType expirableType)
  : ScriptableCommand(id, cmdExecScript, expirableType)
  , m_sScriptName(szScriptName)
  , m_sScriptArguments(szScriptArguments)
  , m_bCanSaveAll(bCanSaveAll)
  , m_nRunFlags(nRunFlags)
{
}

void CNppExecCommandExecutor::DoExecScriptCommand::subExecute()
{
    DoExecScript( m_sScriptName.c_str(), m_sScriptArguments.c_str(), m_bCanSaveAll, m_nRunFlags );

    if ( (m_nRunFlags & CScriptEngine::rfExitScript) != 0 )
    {
        CEvent& stopEvent = GetExecutor()->m_StopEvent;
        if ( !stopEvent.IsNull() )
        {
            stopEvent.Set(); // stopping the Executor after the exit script
        }
    }
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::DoRunScriptCommand ...
CNppExecCommandExecutor::DoRunScriptCommand::DoRunScriptCommand(const tstr& id, const CListT<tstr>& CmdList, unsigned int nRunFlags, eExpirableType expirableType)
  : ScriptableCommand(id, cmdRunScript, expirableType)
  , m_nRunFlags(nRunFlags)
{
    m_CmdList.AddItems(CmdList);
}

void CNppExecCommandExecutor::DoRunScriptCommand::subExecute()
{
    DoRunScript(m_CmdList, m_nRunFlags);
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::NppExitCommand ...
CNppExecCommandExecutor::NppExitCommand::NppExitCommand(const tstr& id, eExpirableType expirableType)
  : ScriptableCommand(id, cmdNppExit, expirableType)
  , m_ScriptsStopper(GetExecutor(), 0)
{
    GetExecutor()->GetNppExec()->GetPluginInterfaceImpl().Enable(false);

    m_ScriptsStopper.SendAbortNotificationToRunningScripts( _T("; Aborting the script to exit Notepad++ (from NppExitCommand())") );
}

void CNppExecCommandExecutor::NppExitCommand::subExecute()
{
    const unsigned int nSyncTimeout = Runtime::GetNppExec().GetOptions().GetUint(OPTU_CHILDS_SYNCTIMEOUT_MS);
    m_ScriptsStopper.WaitUntilRunningScriptsDone(nSyncTimeout);

    DoNppExit();

    if ( !CNppExec::_bIsNppShutdown )
    {
        GetExecutor()->GetNppExec()->GetPluginInterfaceImpl().Enable(true);
    }
}

void CNppExecCommandExecutor::NppExitCommand::Expire()
{
    ::InterlockedCompareExchange(&m_nState, CmdIsExpired, CmdIsReady);

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring (instance = %s)"), GetTypeStr(), GetInstanceStr() );

    subExecute(); // DoNppExit anyway...
}

//-------------------------------------------------------------------------

// ... CNppExecCommandExecutor::TryExitChildProcessCommand ...
CNppExecCommandExecutor::TryExitChildProcessCommand::TryExitChildProcessCommand(const tstr& id, unsigned int nFlags, eExpirableType expirableType)
  : ScriptableCommand(id, cmdTryExitChildProc, expirableType)
  , m_nFlags(nFlags)
  , m_ScriptsStopper(GetExecutor(), nFlags & fCmdNppConsole)
{
    if ( (nFlags & fCloseConsoleOnExit) != 0 )
    {
        m_ScriptsStopper.SendAbortNotificationToRunningScripts( _T("; Aborting the script to close the Console (from TryExitChildProcessCommand())") );
    }
}

void CNppExecCommandExecutor::TryExitChildProcessCommand::subExecute()
{
    TryExitChildProcess((m_nFlags & fCloseConsoleOnExit) != 0, (m_nFlags & fNppIsClosing) != 0);
}

void CNppExecCommandExecutor::TryExitChildProcessCommand::Expire()
{
    ::InterlockedCompareExchange(&m_nState, CmdIsExpired, CmdIsReady);

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; %sCommand - expiring (instance = %s)"), GetTypeStr(), GetInstanceStr() );

    bool bCloseConsoleOnExit = (((m_nFlags & fCloseConsoleOnExit) != 0) && ((m_nFlags & fCmdNppConsole) == 0));
    TryExitChildProcess(bCloseConsoleOnExit, (m_nFlags & fNppIsClosing) != 0);
}

//-------------------------------------------------------------------------

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
    DWORD dwExitCode = (DWORD)(-1);;
    ::GetExitCodeProcess(m_ProcInfo.hProcess, &dwExitCode);
    return (dwExitCode == STILL_ACTIVE);
}

/////////////////////////////////////////////////////////////////////////////
