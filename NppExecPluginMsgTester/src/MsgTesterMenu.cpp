#include "MsgTesterMenu.h"
#include "MsgTester.h"
#include "NppExecPluginMsgSender.h"


const TCHAR* const CMsgTesterMenu::cszUniqueIdForExecuteCollateral_Cmd = _T("NPEM_EXECUTE_COLLATERAL - cmd");
const TCHAR* const CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoAttention = _T("NPEM_EXECUTE_COLLATERAL - echo Attention");
const TCHAR* const CMsgTesterMenu::cszUniqueIdForExecuteCollateral_CloseConsole = _T("NPEM_EXECUTE_COLLATERAL - close Console");
const TCHAR* const CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoInNSecs[10] = {
    _T("NPEM_EXECUTE_COLLATERAL - echo In 1 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 2 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 3 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 4 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 5 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 6 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 7 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 8 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 9 second(s)"),
    _T("NPEM_EXECUTE_COLLATERAL - echo In 10 second(s)")
};

const TCHAR* const CMsgTesterMenu::cszUniqueIdForExecuteQueued_SetA = _T("NPEM_EXECUTE_QUEUED - Set A");
const TCHAR* const CMsgTesterMenu::cszUniqueIdForExecuteQueued_SetB = _T("NPEM_EXECUTE_QUEUED - Set B");

FuncItem CMsgTesterMenu::arrFuncItems[N_NBFUNCITEMS] = {
    { _T("NPEM_GETVERDWORD"),        funcNpeGetVerDword,       0, false, NULL },
    { _T("NPEM_GETVERSTR"),          funcNpeGetVerStr,         0, false, NULL },
    { _T("NPEM_GETSTATE"),           funcNpeGetState,          0, false, NULL },
    { _T("NPEM_PRINT"),              funcNpePrint,             0, false, NULL },
    { _T("NPEM_EXECUTE"),            funcNpeExecute,           0, false, NULL },
    { _T("NPEM_NPPEXEC"),            funcNpeNppExec,           0, false, NULL },
    { _T("NPEM_EXECUTE_COLLATERAL"), funcNpeExecuteCollateral, 0, false, NULL },
    { _T("NPEM_EXECUTE_QUEUED"),     funcNpeExecuteQueued,     0, false, NULL }
};

bool CreateNewThread(LPTHREAD_START_ROUTINE lpFunc, LPVOID lpParam, HANDLE* lphThread /* = NULL */)
{
    DWORD dwThreadId = 0;
    HANDLE hThread = ::CreateThread(
        NULL,
        0,
        lpFunc,
        lpParam,
        0,
        &dwThreadId);

    if ( hThread == NULL )
        return false;
  
    if ( !lphThread )
    {
        ::CloseHandle(hThread);
    }
    else
    {
        *lphThread = hThread;
    }
    return true;
}

namespace
{
    DWORD WINAPI NpeExecuteCollateralThreadFunc(LPVOID lpParam)
    {
        CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
        NpeExecuteCollateralParam necp;
        
        ::Sleep(1000); // let's have some pause to not make everything too fast
        
        // starting "cmd"
        necp.szScriptBody = _T("echo Starting cmd.exe...\n") \
                            _T("cmd");
        necp.szID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_Cmd;
        necp.dwResult = 1; // send a notification when executed
        if ( npeMsgr.NpeExecuteCollateral(&necp) != NPE_EXECUTE_COLLATERAL_OK )
        {
            ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), necp.szID, MB_OK | MB_ICONERROR);
            return 1;
        }

        ::Sleep(2000); // just to ensure cmd.exe has enough time to be started
        
        // Printing a message
        necp.szScriptBody = _T("echo Attention!");
        necp.szID = CMsgTesterMenu::cszUniqueIdForExecuteCollateral_EchoAttention;
        necp.dwResult = 1; // send a notification when executed
        if ( npeMsgr.NpeExecuteCollateral(&necp) != NPE_EXECUTE_COLLATERAL_OK )
        {
            ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), necp.szID, MB_OK | MB_ICONERROR);
            return 1;
        }

        return 0;
    }
}

void CMsgTesterMenu::funcNpeGetVerDword()
{
    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    DWORD dwVersion = npeMsgr.NpeGetVerDword();

    TCHAR str[20];
    ::wsprintf(str, _T("0x%04X"), dwVersion);
    ::MessageBox( GetMsgTester().getNppWnd(), str, _T("NppExec version (as dword)"), MB_OK);
}

void CMsgTesterMenu::funcNpeGetVerStr()
{
    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    TCHAR str[NPE_MAXVERSTR];
    npeMsgr.NpeGetVerStr(str);

    ::MessageBox( GetMsgTester().getNppWnd(), str, _T("NppExec version (as string)"), MB_OK);
}

void CMsgTesterMenu::funcNpeGetState()
{
    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    DWORD dwState = npeMsgr.NpeGetState();
    if ( dwState == NPE_STATEREADY )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Ready"), _T("NppExec state"), MB_OK);
    else if ( dwState & NPE_STATEBUSY )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Busy"), _T("NppExec state"), MB_OK);
    else
        ::MessageBox( GetMsgTester().getNppWnd(), _T("UNKNOWN"), _T("NppExec state"), MB_OK);
}

void CMsgTesterMenu::funcNpePrint()
{
    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    const TCHAR* cszText = _T("Hello!\nThis is a test message.\r\nAre you happy to see it? ;-)");
    if ( npeMsgr.NpePrint(cszText) != NPE_STATEREADY )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), _T("NPEM_PRINT"), MB_OK | MB_ICONERROR);
}

void CMsgTesterMenu::funcNpeExecute()
{
    NpeExecuteParam nep;
    nep.szScriptBody = _T("cd c:\\ \n dir\r\ncd $(NPP_DIRECTORY)\ndir \ndir *.txt");
    nep.dwResult = 1; // send a notification when executed

    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    if ( npeMsgr.NpeExecute(&nep) != NPE_EXECUTE_OK )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), _T("NPEM_EXECUTE"), MB_OK | MB_ICONERROR);
}

void CMsgTesterMenu::funcNpeNppExec()
{
    NpeNppExecParam npep;
    npep.szScriptName = _T("script");
    // npep.szScriptName = _T("C:\\Program Files\\My NppExec Scripts\\test script.txt");
    npep.szScriptArguments = _T("\"arg 1\" \"arg 2\"");
    npep.dwResult = 1; // send a notification when executed

    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    if ( npeMsgr.NpeNppExec(&npep) != NPE_NPPEXEC_OK )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), _T("NPEM_NPPEXEC"), MB_OK | MB_ICONERROR);
}

void CMsgTesterMenu::funcNpeExecuteCollateral()
{
    // Whatever NppExec is currently doing, the following will be executed collaterally:

    NpeExecuteCollateralParam necp;
    necp.szScriptBody = _T("echo NPEM_EXECUTE_COLLATERAL - starting\n") \
                        _T("echo This text will be printed even when NppExec is \"busy\"");
    necp.szID = NULL;  // no id - no notification
    necp.dwResult = 0; // don't send a notification when executed
    
    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    if ( npeMsgr.NpeExecuteCollateral(&necp) != NPE_EXECUTE_COLLATERAL_OK )
    {
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), _T("NPEM_EXECUTE_COLLATERAL - starting"), MB_OK | MB_ICONERROR);
        return;
    }

    // The rest of the test is in a separate thread to not block Notepad++'s GUI
    if ( !CreateNewThread(NpeExecuteCollateralThreadFunc, NULL) )
    {
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Failed to create a thread"), _T("NPEM_EXECUTE_COLLATERAL - thread"), MB_OK | MB_ICONERROR);
        return;
    }
}

void CMsgTesterMenu::funcNpeExecuteQueued()
{
    // Whatever NppExec is currently doing, the following will be queued:

    NpeExecuteQueuedParam neqp;
    neqp.szScriptBody = _T("npe_console a+ -- // Console: append on, silent\n") \
                        _T("echo Setting A = 10...\n") \
                        _T("set A = 10\n") \
                        _T("echo A = $(A)");
    neqp.szID = cszUniqueIdForExecuteQueued_SetA;
    neqp.dwResult = 1; // send a notification when executed

    CNppExecPluginMsgSender npeMsgr( GetMsgTester().getNppWnd(), GetMsgTester().getDllFileName() );
    if ( npeMsgr.NpeExecuteQueued(&neqp) != NPE_EXECUTE_QUEUED_OK )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), neqp.szID, MB_OK | MB_ICONERROR);

    neqp.szScriptBody = _T("echo Setting B = A + 20...\n") \
                        _T("set B ~ $(A) + 20\n") \
                        _T("echo B = $(B)");
    neqp.szID = cszUniqueIdForExecuteQueued_SetB;
    neqp.dwResult = 1; // send a notification when executed

    if ( npeMsgr.NpeExecuteQueued(&neqp) != NPE_EXECUTE_QUEUED_OK )
        ::MessageBox( GetMsgTester().getNppWnd(), _T("Operation failed"), neqp.szID, MB_OK | MB_ICONERROR);
}
