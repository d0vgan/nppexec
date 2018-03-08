#ifndef _npp_exec_plugin_msg_tester_h_
#define _npp_exec_plugin_msg_tester_h_
//----------------------------------------------------------------------------
#include "core/NppPlugin.h"
#include "MsgTesterMenu.h"


class CMsgTester: public CNppPlugin
{
    public:
        static const TCHAR* const PLUGIN_NAME;

    protected:
        CMsgTesterMenu m_nppPluginMenu;

    public:
        // standard n++ plugin functions
        virtual void         nppBeNotified(SCNotification* pscn);
        virtual FuncItem*    nppGetFuncsArray(int* pnbFuncItems);
        virtual const TCHAR* nppGetName();
        virtual LRESULT      nppMessageProc(UINT uMessage, WPARAM wParam, LPARAM lParam);

        // common n++ notification
        virtual void OnNppSetInfo(const NppData& nppd);

        // custom n++ notifications
        void OnNppReady();
        void OnNppShutdown();

};

CMsgTester& GetMsgTester();

//----------------------------------------------------------------------------
#endif
