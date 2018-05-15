/*
This file is part of NppExec
Copyright (C) 2013 DV <dvv81 (at) ukr (dot) net>

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

#include "../NppExec.h"
#include "NppExecPluginInterface.h"


const DWORD  PLUGIN_VER_DWORD = NPPEXEC_VER_DWORD;
#ifdef UNICODE
    const TCHAR* PLUGIN_VER_STR = NPPEXEC_VER_STR _T(" Unicode"); // see also: DlgConsole.cpp
#else
    const TCHAR* PLUGIN_VER_STR = NPPEXEC_VER_STR _T(" ANSI"); // see also: DlgConsole.cpp
#endif


tstr CNppExecPluginInterfaceImpl::CPluginMsg::GenerateUniqueId()
{
    DWORD dwTicks;
    SYSTEMTIME t;
    tstr id;

    dwTicks = ::GetTickCount();
    ::GetLocalTime(&t);
    id.Format(
      20 + 2*sizeof(dwTicks),
      _T("%04u%02u%02u.%02u%02u%02u.%08X"), 
      t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, dwTicks
    );

    return id;
}

CNppExecPluginInterfaceImpl::CNppExecPluginInterfaceImpl()
  : m_pNppExec(NULL)
  , m_hBackgroundExecAsyncCmdThread(NULL)
  , m_hBackgroundSendMsgThread(NULL)
  , m_isEnabled(false)
{
    m_StopEvent.Create(NULL, TRUE, FALSE, NULL);
}

CNppExecPluginInterfaceImpl::~CNppExecPluginInterfaceImpl()
{
    m_StopEvent.Set();
    stopExecAsyncCmdThread();
    stopSendMsgThread();
}

void CNppExecPluginInterfaceImpl::Enable(bool bEnable)
{
    m_isEnabled = bEnable;
}

void CNppExecPluginInterfaceImpl::SetPluginName(const TCHAR* szNppExecDll)
{
    m_sNppExecDll = szNppExecDll;
}

void CNppExecPluginInterfaceImpl::ProcessExternalPluginMsg(long nMsg, const TCHAR* srcModuleName, void* pInfo)
{
    if ( !pInfo )
        return;

    switch ( nMsg )
    {
        case NPEM_GETVERDWORD:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_GETVERDWORD") );
            // Important:
            // If any CNppExecConsole.m_reConsole.SendMsg() call occur under a
            // locked CriticalSection, it can potentially lead to a deadlock
            // because SendMsg() wants the main thread's message loop - while
            // the main thread itself may be trying to obtain the very same
            // CriticalSection at the same time, thus blocking each other.
            // CNppExecConsole has been designed to avoid such deadlocks.

            // pInfo is (DWORD* pdwVersion)
            DWORD* pdwVersion = (DWORD *) pInfo;
            *pdwVersion = PLUGIN_VER_DWORD;
        
            break;
        }
    
        case NPEM_GETVERSTR:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_GETVERSTR") );

            // pInfo is (TCHAR* pszVersion)
            TCHAR* pszVersion = (TCHAR *) pInfo;
            const TCHAR* cszVer = PLUGIN_VER_STR;
            while ( (*(pszVersion++) = *(cszVer++)) != 0 )
            {
                if ( (NPE_MAXVERSTR - 1) == (int) (cszVer - PLUGIN_VER_STR) )
                {
                    *pszVersion = 0;
                    break;
                }
            }
        
            break;
        }
    
        case NPEM_GETSTATE:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_GETSTATE") );

            // pInfo is (DWORD* pdwState)
            *((DWORD *) pInfo) = npemGetState();

            break;
        }

        case NPEM_PRINT:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_PRINT") );

            // pInfo is (const TCHAR* szText)
            const TCHAR* szText = (const TCHAR *) pInfo;
            if ( (npemGetState() == NPE_STATEREADY) && szText )
            {
                npemPrint(szText);
            }

            break;
        }

        case NPEM_EXECUTE:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_EXECUTE") );

            // pInfo is (NpeExecuteParam* nep)
            NpeExecuteParam* nep = (NpeExecuteParam *) pInfo;
            if ( (npemGetState() == NPE_STATEREADY) &&
                 nep->szScriptBody && nep->szScriptBody[0] )
            {
                tstr id;
                if ( nep->dwResult )
                {
                    id = CPluginMsg::GenerateUniqueId();
                    registerPluginMsg( CPluginMsg(srcModuleName, id) );
                }
                nep->dwResult = NPE_EXECUTE_OK;
                npemExecuteScript( id, nep->szScriptBody );
            }
            else
            {
                nep->dwResult = NPE_EXECUTE_FAILED;
            }
        
            break;
        }

        case NPEM_NPPEXEC:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_NPPEXEC") );

            // pInfo is (NpeNppExecParam* npep)
            NpeNppExecParam* npep = (NpeNppExecParam *) pInfo;
            if ( (npemGetState() == NPE_STATEREADY) &&
                 npep->szScriptName && npep->szScriptName[0] )
            {
                tstr id;
                if ( npep->dwResult )
                {
                    id = CPluginMsg::GenerateUniqueId();
                    registerPluginMsg( CPluginMsg(srcModuleName, id) );
                }
                npep->dwResult = NPE_NPPEXEC_OK;
                m_pNppExec->DoExecScript( id, npep->szScriptName, true, npep->szScriptArguments, IScriptEngine::rfExternal );
            }
            else
            {
                npep->dwResult = NPE_NPPEXEC_FAILED;
            }

            break;
        }

        case NPEM_EXECUTE2:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_EXECUTE2") );

            // pInfo is (NpeExecuteParam2* nep2)
            NpeExecuteParam2* nep2 = (NpeExecuteParam2 *) pInfo;
            if ( (npemGetState() == NPE_STATEREADY) &&
                 nep2->szScriptBody && nep2->szScriptBody[0] )
            {
                tstr id = nep2->szID;
                if ( nep2->dwResult )
                {
                    registerPluginMsg( CPluginMsg(srcModuleName, id) );
                }
                nep2->dwResult = NPE_EXECUTE2_OK;
                npemExecuteScript( id, nep2->szScriptBody );
            }
            else
            {
                nep2->dwResult = NPE_EXECUTE2_FAILED;
            }
        
            break;
        }

        case NPEM_NPPEXEC2:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_NPPEXEC2") );

            // pInfo is (NpeNppExecParam2* npep2)
            NpeNppExecParam2* npep2 = (NpeNppExecParam2 *) pInfo;
            if ( (npemGetState() == NPE_STATEREADY) &&
                 npep2->szScriptName && npep2->szScriptName[0] )
            {
                tstr id = npep2->szID;
                if ( npep2->dwResult )
                {
                    registerPluginMsg( CPluginMsg(srcModuleName, id) );
                }
                npep2->dwResult = NPE_NPPEXEC2_OK;
                m_pNppExec->DoExecScript( id, npep2->szScriptName, true, npep2->szScriptArguments, IScriptEngine::rfExternal );
            }
            else
            {
                npep2->dwResult = NPE_NPPEXEC2_FAILED;
            }

            break;
        }

        case NPEM_EXECUTE_COLLATERAL:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_EXECUTE_COLLATERAL") );

            // pInfo is (NpeExecuteCollateralParam* necp)
            NpeExecuteCollateralParam* necp = (NpeExecuteCollateralParam *) pInfo;
            if ( m_isEnabled && necp->szScriptBody && necp->szScriptBody[0] )
            {
                tstr id = necp->szID;
                if ( necp->dwResult )
                {
                    registerPluginMsg( CPluginMsg(srcModuleName, id) );
                }
                necp->dwResult = NPE_EXECUTE_COLLATERAL_OK;
                npemExecuteCollateralScript( id, necp->szScriptBody );
            }
            else
            {
                necp->dwResult = NPE_EXECUTE_COLLATERAL_FAILED;
            }

            break;
        }

        case NPEM_EXECUTE_QUEUED:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_EXECUTE_QUEUED") );

            // pInfo is (NpeExecuteQueuedParam* neqp)
            NpeExecuteQueuedParam* neqp = (NpeExecuteQueuedParam *) pInfo;
            if ( m_isEnabled && neqp->szScriptBody && neqp->szScriptBody[0] )
            {
                tstr id = neqp->szID;
                if ( neqp->dwResult )
                {
                    registerPluginMsg( CPluginMsg(srcModuleName, id) );
                }
                neqp->dwResult = NPE_EXECUTE_QUEUED_OK;
                npemExecuteQueuedScript( id, neqp->szScriptBody );
            }
            else
            {
                neqp->dwResult = NPE_EXECUTE_QUEUED_FAILED;
            }

            break;
        }

        case NPEM_FREEPTR:
        {
            npemFreePtr( pInfo );

            break;
        }

        case NPEM_GETSCRIPTNAMES:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_GETSCRIPTNAMES") );

            // pInfo is (NpeGetScriptNamesParam* nsn)
            NpeGetScriptNamesParam* nsn = (NpeGetScriptNamesParam *) pInfo;
            if ( m_isEnabled )
            {
                nsn->pScriptNames = npemGetScriptNames(); // to be freed by NPEM_FREEPTR
                nsn->dwResult = NPE_GETSCRIPTNAMES_OK;
            }
            else
            {
                nsn->dwResult = NPE_GETSCRIPTNAMES_FAILED;
            }

            break;
        }

        case NPEM_GETSCRIPTBYNAME:
        {
            Runtime::GetLogger().Add_WithoutOutput( _T("; NPEM_GETSCRIPTBYNAME") );

            // pInfo is (NpeGetScriptByNameParam* nsn)
            NpeGetScriptByNameParam* nsn = (NpeGetScriptByNameParam *) pInfo;
            if ( m_isEnabled )
            {
                nsn->pScriptBody = npemGetScriptByName(nsn->szScriptName); // to be freed by NPEM_FREEPTR
                nsn->dwResult = NPE_GETSCRIPTBYNAME_OK;
            }
            else
            {
                nsn->dwResult = NPE_GETSCRIPTBYNAME_FAILED;
            }

            break;
        }

        case NPEM_SUSPENDEDACTION:
        {
            /* obsolete */
            break;
        }

        default:
            break;
    }
}

void CNppExecPluginInterfaceImpl::NotifyExternalPluginResult(const tstr& id, DWORD dwResultCode)
{
    if ( !id.IsEmpty() )
    {
        CCriticalSectionLockGuard lock(m_csPluginMsgs);

        PluginMsgsType::iterator itr = m_PluginMsgs.find(id);
        if ( itr != m_PluginMsgs.end() )
        {
            initSendMsgThread();

            CPluginMsg& msg = itr->second;
            msg.Result.dwResult = dwResultCode;
            msg.Result.szID = msg.ID.c_str();
            msg.ci.internalMsg = NPEN_RESULT;
            msg.ci.srcModuleName = m_sNppExecDll.c_str();
            msg.ci.info = &(msg.Result);

            m_SendMsgQueue.Add(msg.ID);
            m_SendMsgEvent.Set();
        }
    }
}

void CNppExecPluginInterfaceImpl::registerAsyncCmd(const tstr& id, const TCHAR* szScriptBody, AsyncCmd::eType type)
{
    {
        CCriticalSectionLockGuard lock(m_csExecAsyncCmds);

        initExecAsyncCmdThread();
        m_ExecAsyncCmdQueue.Add( AsyncCmd(id, szScriptBody, type) );
    }

    m_ExecAsyncCmdEvent.Set();
}

void CNppExecPluginInterfaceImpl::registerPluginMsg(const CPluginMsg& msg)
{
    if ( !msg.ID.IsEmpty() )
    {
        CCriticalSectionLockGuard lock(m_csPluginMsgs);

        m_PluginMsgs[msg.ID] = msg;
    }
}

DWORD WINAPI CNppExecPluginInterfaceImpl::BackgroundExecAsyncCmdThreadFunc(LPVOID lpParam)
{
    CNppExecPluginInterfaceImpl* pImpl = (CNppExecPluginInterfaceImpl *) lpParam;
    HANDLE waitEvents[2] = { pImpl->m_StopEvent.GetHandle(), pImpl->m_ExecAsyncCmdEvent.GetHandle() };

    for ( ; ; )
    {
        DWORD dwWaitEvent = ::WaitForMultipleObjects(2, waitEvents, FALSE, INFINITE);
        if ( dwWaitEvent == WAIT_OBJECT_0 ) // Stop Event
            break;

        if ( dwWaitEvent == WAIT_OBJECT_0 + 1 ) // ExecAsyncCmd Event
        {
            CListItemT<AsyncCmd>* pExecAsyncCmdItem;
            
            {
                CCriticalSectionLockGuard lock(pImpl->m_csExecAsyncCmds);
                pExecAsyncCmdItem = pImpl->m_ExecAsyncCmdQueue.GetFirst();
            }

            if ( pExecAsyncCmdItem != NULL )
            {
                AsyncCmd& Cmd = pExecAsyncCmdItem->GetItem();
                CListT<tstr> CmdList;
                getCmdListFromScriptBody(CmdList, Cmd.GetScriptBody().c_str());
                CNppExec* pNppExec = pImpl->GetNppExec();
                pNppExec->initConsoleDialog();
                if ( pNppExec->checkCmdListAndPrepareConsole(CmdList, false) )
                {
                    const tstr& id = Cmd.GetId();
                    const AsyncCmd::eType type = Cmd.GetType();
                    if ( type == AsyncCmd::cmdCollateralScript )
                    {
                        pNppExec->GetCommandExecutor().ExecuteCollateralScript(CmdList, id, IScriptEngine::rfCollateralScript | IScriptEngine::rfExternal);
                    }
                    else if ( type == AsyncCmd::cmdQueuedScript )
                    {
                        CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::DoRunScriptCommand(id, CmdList, IScriptEngine::rfExternal, CNppExecCommandExecutor::ExpirableCommand::NonExpirable);
                        pNppExec->GetCommandExecutor().ExecuteCommand(pCommand);
                    }
                }

                bool isEmpty;
                {
                    CCriticalSectionLockGuard lock(pImpl->m_csExecAsyncCmds);
                    pImpl->m_ExecAsyncCmdQueue.DeleteFirst();
                    isEmpty = pImpl->m_ExecAsyncCmdQueue.IsEmpty();
                }
                if ( !isEmpty )
                    pImpl->m_ExecAsyncCmdEvent.Set();
            }
        }
    }

    pImpl->m_ExecAsyncCmdThreadDoneEvent.Set();
    return 0;
}

DWORD WINAPI CNppExecPluginInterfaceImpl::BackgroundSendMsgThreadFunc(LPVOID lpParam)
{
    CNppExecPluginInterfaceImpl* pImpl = (CNppExecPluginInterfaceImpl *) lpParam;
    HANDLE waitEvents[2] = { pImpl->m_StopEvent.GetHandle(), pImpl->m_SendMsgEvent.GetHandle() };

    for ( ; ; )
    {
        DWORD dwWaitEvent = ::WaitForMultipleObjects(2, waitEvents, FALSE, INFINITE);
        if ( dwWaitEvent == WAIT_OBJECT_0 ) // Stop Event
            break;
            
        if ( dwWaitEvent == WAIT_OBJECT_0 + 1 ) // SendMsg Event
        {
            CCriticalSectionLockGuard lock(pImpl->m_csPluginMsgs);

            CListItemT<tstr>* pSendMsgItem = pImpl->m_SendMsgQueue.GetFirst();
            if ( pSendMsgItem != NULL )
            {
                const tstr& id = pSendMsgItem->GetItem();
                PluginMsgsType::iterator itr = pImpl->m_PluginMsgs.find(id);
                if ( itr != pImpl->m_PluginMsgs.end() )
                {
                    CPluginMsg& msg = itr->second;
                    ::SendMessage( 
                        pImpl->m_pNppExec->m_nppData._nppHandle, 
                        NPPM_MSGTOPLUGIN,
                        (WPARAM) msg.SrcModuleName.c_str(),
                        (LPARAM) &(msg.ci)
                    );

                    pImpl->m_PluginMsgs.erase(itr);
                }

                pImpl->m_SendMsgQueue.DeleteFirst();
                if ( !pImpl->m_SendMsgQueue.IsEmpty() )
                    pImpl->m_SendMsgEvent.Set();
            }
        }
    }

    pImpl->m_SendMsgThreadDoneEvent.Set();
    return 0;
}

void CNppExecPluginInterfaceImpl::initExecAsyncCmdThread()
{
    if ( m_ExecAsyncCmdEvent.IsNull() )
        m_ExecAsyncCmdEvent.Create(NULL, FALSE, FALSE, NULL);

    if ( m_ExecAsyncCmdThreadDoneEvent.IsNull() )
        m_ExecAsyncCmdThreadDoneEvent.Create(NULL, TRUE, FALSE, NULL);

    if ( m_hBackgroundExecAsyncCmdThread == NULL )
    {
        NppExecHelpers::CreateNewThread(BackgroundExecAsyncCmdThreadFunc, this, &m_hBackgroundExecAsyncCmdThread);

        Runtime::GetLogger().Add_WithoutOutput( _T("; NppExecPluginInterface - ExecAsyncCmdThread - initialized") );
    }
}

void CNppExecPluginInterfaceImpl::stopExecAsyncCmdThread()
{
    if ( !m_ExecAsyncCmdThreadDoneEvent.IsNull() )
    {
        Runtime::GetLogger().Add_WithoutOutput( _T("; NppExecPluginInterface: waiting for the ExecAsyncCmdThreadDoneEvent...") );

        m_ExecAsyncCmdThreadDoneEvent.Wait(INFINITE);
    }

    if ( m_hBackgroundExecAsyncCmdThread != NULL )
    {
        Runtime::GetLogger().Add_WithoutOutput( _T("; NppExecPluginInterface - ExecAsyncCmdThread - stopped") );

        ::CloseHandle(m_hBackgroundExecAsyncCmdThread);
    }
}

void CNppExecPluginInterfaceImpl::initSendMsgThread()
{
    if ( m_SendMsgEvent.IsNull() )
        m_SendMsgEvent.Create(NULL, FALSE, FALSE, NULL);
            
    if ( m_SendMsgThreadDoneEvent.IsNull() )
        m_SendMsgThreadDoneEvent.Create(NULL, TRUE, FALSE, NULL);
        
    if ( m_hBackgroundSendMsgThread == NULL )
    {
        NppExecHelpers::CreateNewThread(BackgroundSendMsgThreadFunc, this, &m_hBackgroundSendMsgThread);

        Runtime::GetLogger().Add_WithoutOutput( _T("; NppExecPluginInterface - SendMsgThread - initialized") );
    }
}

void CNppExecPluginInterfaceImpl::stopSendMsgThread()
{
    if ( !m_SendMsgThreadDoneEvent.IsNull() )
    {
        Runtime::GetLogger().Add_WithoutOutput( _T("; NppExecPluginInterface: waiting for the SendMsgThreadDoneEvent...") );

        m_SendMsgThreadDoneEvent.Wait(INFINITE);
    }

    if ( m_hBackgroundSendMsgThread != NULL )
    {
        Runtime::GetLogger().Add_WithoutOutput( _T("; NppExecPluginInterface - SendMsgThread - stopped") );

        ::CloseHandle(m_hBackgroundSendMsgThread);
    }
}

DWORD CNppExecPluginInterfaceImpl::npemGetState() const
{
    DWORD dwState = NPE_STATEREADY;

    if ( !m_isEnabled )
        dwState = NPE_STATEDISABLED;
    else if ( m_pNppExec->GetCommandExecutor().IsChildProcessRunning() || m_pNppExec->_consoleCommandIsRunning )
        dwState = NPE_STATECHILDPROC;
    else if ( m_pNppExec->GetCommandExecutor().IsScriptRunningOrQueued() )
        dwState = NPE_STATEBUSY;
    
    return dwState;
}

void CNppExecPluginInterfaceImpl::addCommand(CListT<tstr>& CmdList, tstr& Cmd)
{
    while ( Cmd.GetLastChar() == _T('\r') )
    {
        // remove trailing '\r'
        Cmd.SetSize(Cmd.length() - 1);
    }
    if ( Cmd.length() > 0 )
    {
        CmdList.Add( Cmd );
    }
}

void CNppExecPluginInterfaceImpl::getCmdListFromScriptBody(CListT<tstr>& CmdList, const TCHAR* szScriptBody)
{
    tstr Line;
    
    CmdList.Clear();
    while ( *szScriptBody )
    {
        if ( *szScriptBody == _T('\n') )
        {
            addCommand( CmdList, Line );
            Line.Clear();
        }
        else
        {
            Line += *szScriptBody;
        }
        ++szScriptBody;
    }
    addCommand( CmdList, Line );
}

void CNppExecPluginInterfaceImpl::npemExecuteScript(const tstr& id, const TCHAR* szScriptBody)
{
    CListT<tstr> CmdList;
    getCmdListFromScriptBody(CmdList, szScriptBody);
    m_pNppExec->SetCmdList(CmdList);
    m_pNppExec->OnDirectExec(id, true, IScriptEngine::rfExternal);
}

void CNppExecPluginInterfaceImpl::npemExecuteCollateralScript(const tstr& id, const TCHAR* szScriptBody)
{
    // this will be executed asynchronously by its nature;
    // let's process it in a different thread to not block the message loop
    registerAsyncCmd(id, szScriptBody, AsyncCmd::cmdCollateralScript);
}

void CNppExecPluginInterfaceImpl::npemExecuteQueuedScript(const tstr& id, const TCHAR* szScriptBody)
{
    // this will be executed asynchronously by its nature;
    // let's process it in a different thread to not block the message loop
    registerAsyncCmd(id, szScriptBody, AsyncCmd::cmdQueuedScript);
}

void CNppExecPluginInterfaceImpl::conPrintLine(tstr& Line)
{
    while ( Line.GetLastChar() == _T('\r') )
    {
        // remove trailing '\r'
        Line.SetSize(Line.length() - 1);
    }
    m_pNppExec->GetConsole().PrintOutput( Line.c_str(), true );
}

void CNppExecPluginInterfaceImpl::npemPrint(const TCHAR* szText)
{
    tstr Line;
    
    m_pNppExec->_consoleCommandIsRunning = true;
    m_pNppExec->_consoleCommandBreak = false;

    m_pNppExec->showConsoleDialog(CNppExec::showIfHidden, 0);

    while ( *szText )
    {
        if ( *szText == _T('\n') )
        {
            conPrintLine( Line );
            Line.Clear();
            if ( m_pNppExec->_consoleCommandBreak || !m_pNppExec->_consoleIsVisible )
                break;
        }
        else
        {
            Line += *szText;
        }
        ++szText;
    }
    conPrintLine( Line );

    m_pNppExec->_consoleCommandIsRunning = false;
    m_pNppExec->_consoleCommandBreak = false;
}

void CNppExecPluginInterfaceImpl::npemFreePtr(void* p)
{
    if ( p )
    {
        delete [] p;
    }
}

TCHAR* CNppExecPluginInterfaceImpl::npemGetScriptNames()
{
    CListT<tstr> scriptNames = m_pNppExec->m_ScriptsList.GetScriptNames();
    tstr S;
    if ( !scriptNames.IsEmpty() )
    {
        S.Reserve(scriptNames.GetCount() * 20);
        for ( CListItemT<tstr>* p = scriptNames.GetFirst(); p != NULL; p = p->GetNext() )
        {
            if ( !S.IsEmpty() )
                S += _T('\n');
            S += p->GetItem();
        }
    }
    TCHAR* p = new TCHAR[S.length() + 1];
    lstrcpy(p, S.c_str());
    return p; // to be freed by npemFreePtr()
}

TCHAR* CNppExecPluginInterfaceImpl::npemGetScriptByName(const TCHAR* szScriptName)
{
    tstr S;
    CNppScript scriptBody;
    if ( m_pNppExec->m_ScriptsList.GetScript(szScriptName, scriptBody) )
    {
        S.Reserve(scriptBody.GetCount() * 80);
        for ( CListItemT<tstr>* p = scriptBody.GetFirst(); p != NULL; p = p->GetNext() )
        {
            if ( !S.IsEmpty() )
                S += _T('\n');
            S += p->GetItem();
        }
    }
    TCHAR* p = new TCHAR[S.length() + 1];
    lstrcpy(p, S.c_str());
    return p; // to be freed by npemFreePtr()
}
