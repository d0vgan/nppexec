#include "MsgTester.h"
#include "nppexec_msgs.h"
#include "NppExecPluginMsgSender.h"


const TCHAR* const CMsgTester::PLUGIN_NAME = _T("NppExecPluginMsgTester");

FuncItem* CMsgTester::nppGetFuncsArray(int* pnbFuncItems)
{
    *pnbFuncItems = CMsgTesterMenu::N_NBFUNCITEMS;
    return CMsgTesterMenu::arrFuncItems;
}

const TCHAR* CMsgTester::nppGetName()
{
    return CMsgTester::PLUGIN_NAME;
}

void CMsgTester::nppBeNotified(SCNotification* pscn)
{
    if ( pscn->nmhdr.hwndFrom == m_nppMsgr.getNppWnd() )
    {
        // >>> notifications from Notepad++
        switch ( pscn->nmhdr.code )
        {
            case NPPN_READY:
                OnNppReady();
                break;
            
            case NPPN_SHUTDOWN:
                OnNppShutdown();
                break;

            default:
                break;
        }
        // <<< notifications from Notepad++
    }
}

namespace
{
    const unsigned int nSecondsTillCloseConsole = 5;

    DWORD WINAPI ProcessNotificationFromNpeExecuteCollateralThreadFunc(LPVOID lpParam)
    {
        const TCHAR* pszID = (const TCHAR *) lpParam;
        CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
        NpeExecuteCollateralParam necp;
        TCHAR szScriptBody[128];

        if ( lstrcmp(pszID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_Cmd) == 0 )
        {
            // Done!
            // (This notification is sent right after the "cmd" has exited.)

            necp.szScriptBody = _T("npp_console ?\n") \
                                _T("echo Waiting for 2.5 seconds..."); // "npp_console ?" does not open the closed Console
            necp.szID = NULL;  // no id - no notification
            necp.dwResult = 0; // don't send a notification when executed
            npeMsgr.NpeExecuteCollateral(&necp);
            
            ::Sleep(2500); // give the user time to realize the Console has been closed :)

            necp.szScriptBody = _T("echo Reopening the Console...\n") \
                                _T("npp_console on // show the Console\n") \
                                _T("echo NPEM_EXECUTE_COLLATERAL - done!\n") \
                                _T("echo ================ READY ================"); // collateral scripts do not print "==== READY ====" at the end
            necp.szID = NULL;  // no id - no notification
            necp.dwResult = 0; // don't send a notification when executed
            npeMsgr.NpeExecuteCollateral(&necp);
        }
        else if ( lstrcmp(pszID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoAttention) == 0 )
        {
            ::Sleep(1500); // let's have some pause to not make everything too fast
            
            // Printing a message
            ::wsprintf(szScriptBody, _T("echo NppExec's Console will be closed in %u second(s)..."), nSecondsTillCloseConsole);
            necp.szScriptBody = szScriptBody;
            necp.szID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoInNSecs[nSecondsTillCloseConsole - 1];
            necp.dwResult = 1; // send a notification when executed
            if ( npeMsgr.NpeExecuteCollateral(&necp) != NPE_EXECUTE_COLLATERAL_OK )
            {
                ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), necp.szID, MB_OK | MB_ICONERROR);
                return 1;
            }
        }
        else if ( lstrcmp(pszID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_CloseConsole) == 0 )
        {
            // Nothing to do here - we process cszUniqueIdForExecuteCollateral_Cmd instead
        }
        else
        {
            for ( int i = 0; i < nSecondsTillCloseConsole; ++i )
            {
                if ( lstrcmp(pszID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoInNSecs[i]) == 0 )
                {
                    ::Sleep(1000); // wait for 1 second (repeating nSecondsTillCloseConsole times)
                    
                    if ( i == 0 ) // <-- if it was the last second left, close the Console
                    {
                        // Closing NppExec's Console & exiting "cmd"
                        necp.szScriptBody = _T("set local @exit_cmd_silent = exit // the exit command\n") \
                                            _T("npp_console off // close the Console");
                        // NOTE: a local variable in a collateral script is not the same as in a regular script.
                        // A local variable of a collateral script becomes its parent script's local variable
                        // when the collateral script ends.
                        // A local variable of a regular script dies (disappears) at the end of the regular script.
                        necp.szID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_CloseConsole;
                        necp.dwResult = 1; // send a notification when executed
                        if ( npeMsgr.NpeExecuteCollateral(&necp) != NPE_EXECUTE_COLLATERAL_OK )
                        {
                            ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), necp.szID, MB_OK | MB_ICONERROR);
                            return 1;
                        }
                    }
                    else
                    {
                        // Printing the next message (5... 4... 3... 2... 1 second(s))
                        ::wsprintf(szScriptBody, _T("echo NppExec's Console will be closed in %u second(s)..."), i);
                        necp.szScriptBody = szScriptBody;
                        necp.szID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoInNSecs[i - 1];
                        necp.dwResult = 1; // send a notification when executed
                        if ( npeMsgr.NpeExecuteCollateral(&necp) != NPE_EXECUTE_COLLATERAL_OK )
                        {
                            ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), necp.szID, MB_OK | MB_ICONERROR);
                            return 1;
                        }
                    }

                    break;
                }
            }
        }

        return 0;
    }

    bool processNotificationFromNpeExecuteCollateral(tPluginResultStruct* pResult)
    {
        const TCHAR* pszID = NULL;

        // Checking the pResult->szID ...

        for ( int i = 0; i < nSecondsTillCloseConsole; ++i )
        {
            if ( lstrcmp(pResult->szID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoInNSecs[i]) == 0 )
            {
                pszID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoInNSecs[i];
                break;
            }
        }

        if ( pszID == NULL )
        {
            if ( lstrcmp(pResult->szID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_Cmd) == 0 )
            {
                pszID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_Cmd;
            }
            else if ( lstrcmp(pResult->szID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoAttention) == 0 )
            {
                pszID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoAttention;
            }
            else if ( lstrcmp(pResult->szID, CMsgTesterMenu::cszUniqueIdForExecuteCollateral_CloseConsole) == 0 )
            {
                pszID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_CloseConsole;
            }
        }

        if ( pszID != NULL)
        {
            // The actual processing is in a separate thread to not block Notepad++'s GUI
            if ( !CreateNewThread(ProcessNotificationFromNpeExecuteCollateralThreadFunc, (LPVOID) pszID) )
            {
                ::MessageBox( GetMsgTester().getNppWnd(), _T("Failed to create a thread"), pszID, MB_OK | MB_ICONERROR);
            }
            return true;
        }

        return false;
    }

    bool processNotificationFromNpeExecuteQueued(tPluginResultStruct* pResult)
    {
        TCHAR szScriptBody[128];
        NpeExecuteQueuedParam neqp;

        if ( lstrcmp(pResult->szID, CMsgTesterMenu::cszUniqueIdForExecuteQueued_SetA) == 0 )
        {
            ::wsprintf( szScriptBody, _T("echo NPEM_EXECUTE_QUEUED - Set A - %s"), pResult->dwResult == NPE_RESULT_OK ? _T("OK") : _T("FAILED") );
            neqp.szScriptBody = szScriptBody;
            neqp.szID = NULL;  // no id - no notification
            neqp.dwResult = 0; // don't send a notification when executed

            CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
            npeMsgr.NpeExecuteQueued(&neqp);

            return true;
        }

        if ( lstrcmp(pResult->szID, CMsgTesterMenu::cszUniqueIdForExecuteQueued_SetB) == 0 )
        {
            ::wsprintf( szScriptBody, _T("echo NPEM_EXECUTE_QUEUED - Set B - %s\nnpe_console a- -- // Console: append off, silent"), pResult->dwResult == NPE_RESULT_OK ? _T("OK") : _T("FAILED") );
            neqp.szScriptBody = szScriptBody;
            neqp.szID = NULL;  // no id - no notification
            neqp.dwResult = 0; // don't send a notification when executed

            CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
            npeMsgr.NpeExecuteQueued(&neqp);

            return true;
        }

        return false;
    }
}

LRESULT CMsgTester::nppMessageProc(UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    if ( uMessage == NPPM_MSGTOPLUGIN )
    {
        CommunicationInfo* pci = (CommunicationInfo *) lParam;
        if ( pci )
        {
            if ( pci->internalMsg == NPEN_RESULT )
            { 
                // NPEN_RESULT notification...
                if ( lstrcmpi(pci->srcModuleName, _T("NppExec.dll")) == 0 )
                {
                    // ...from NppExec plugin
                    DWORD dwResult = *((DWORD *) pci->info); // <-- the old way (NppExec version >= 0x02F5)
                    tPluginResultStruct* pResult = (tPluginResultStruct *) pci->info; // <-- the new way (NppExec version >= 0x06A0)

                    if ( !processNotificationFromNpeExecuteCollateral(pResult) && 
                         !processNotificationFromNpeExecuteQueued(pResult) )
                    {
                        TCHAR szText[128];

                        if ( dwResult == NPE_RESULT_OK )
                        {
                            // OK, the script has been executed
                            ::wsprintf(szText, _T("OK\n(id = %s)"), pResult->szID);
                            ::MessageBox( getNppWnd(), szText, _T("NPEN_RESULT notification"), MB_OK );
                        }
                        else if ( dwResult == NPE_RESULT_ABORTED )
                        {
                            // aborted, maybe because NppExec's Console was closed
                            ::wsprintf(szText, _T("ABORTED\n(id = %s)"), pResult->szID);
                            ::MessageBox( getNppWnd(), szText, _T("NPEN_RESULT notification"), MB_OK | MB_ICONWARNING );
                        }
                        else
                        {
                            // failed, maybe internal error in NppExec
                            ::wsprintf(szText, _T("FAILED\n(id = %s)"), pResult->szID);
                            ::MessageBox( getNppWnd(), szText, _T("NPEN_RESULT notification"), MB_OK | MB_ICONERROR );
                        }
                    }
                }
            }
        }
    }
    return 1;
}

void CMsgTester::OnNppSetInfo(const NppData& nppd)
{
    m_nppPluginMenu.setNppData(nppd);
}

void CMsgTester::OnNppReady()
{
    // TODO:  add your code here :)
}

void CMsgTester::OnNppShutdown()
{
    // TODO:  add your code here :)
}

CMsgTester& GetMsgTester()
{
    static CMsgTester theMsgTester;

    return theMsgTester;
}

