#ifndef _npp_exec_plugin_msg_tester_menu_h_
#define _npp_exec_plugin_msg_tester_menu_h_
//----------------------------------------------------------------------------
#include "core/NppPluginMenu.h"
//#include "resource.h"


class CMsgTesterMenu : public CNppPluginMenu
{
    public:
        enum NMenuItems {
            N_NPE_GETVERDWORD = 0,
            N_NPE_GETVERSTR,
            N_NPE_GETSTATE,
            N_NPE_PRINT,
            N_NPE_EXECUTE,
            N_NPE_NPPEXEC,
            N_NPE_EXECUTE_COLLATERAL,
            N_NPE_EXECUTE_QUEUED,
            
            N_NBFUNCITEMS
        };

        static FuncItem arrFuncItems[N_NBFUNCITEMS];

        static const TCHAR* const cszUniqueIdForExecuteCollateral_Cmd;
        static const TCHAR* const cszUniqueIdForExecuteCollateral_EchoAttention;
        static const TCHAR* const cszUniqueIdForExecuteCollateral_EchoInNSecs[10];
        static const TCHAR* const cszUniqueIdForExecuteCollateral_CloseConsole;

        static const TCHAR* const cszUniqueIdForExecuteQueued_SetA;
        static const TCHAR* const cszUniqueIdForExecuteQueued_SetB;

    protected:
        static void funcNpeGetVerDword();
        static void funcNpeGetVerStr();
        static void funcNpeGetState();
        static void funcNpePrint();
        static void funcNpeExecute();
        static void funcNpeNppExec();
        static void funcNpeExecuteCollateral();
        static void funcNpeExecuteQueued();

};

bool CreateNewThread(LPTHREAD_START_ROUTINE lpFunc, LPVOID lpParam, HANDLE* lphThread = NULL);

//----------------------------------------------------------------------------
#endif
