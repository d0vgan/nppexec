#ifndef _npp_plugin_h_
#define _npp_plugin_h_
//----------------------------------------------------------------------------
#include "base.h"
#include "SciMessager.h"
#include "NppMessager.h"
#include "NppPluginMenu.h"


class CNppPlugin
{
    protected:
        CNppMessager  m_nppMsgr;
        HMODULE       m_hDllModule;
        TCHAR         m_szDllFileName[100];
        TCHAR         m_szIniFileName[100];

    public:
        CNppPlugin();
        virtual ~CNppPlugin();

        // called from DllMain
        void OnDllProcessAttach(HINSTANCE hDLLInstance);
        void OnDllProcessDetach();

        // standard n++ plugin functions
        virtual void         nppBeNotified(SCNotification* pscn)  { }
        virtual FuncItem*    nppGetFuncsArray(int* pnbFuncItems) = 0;
        virtual const TCHAR* nppGetName() = 0;
        virtual LRESULT      nppMessageProc(UINT uMessage, WPARAM wParam, LPARAM lParam)  { return 1; }
        void                 nppSetInfo(const NppData& nppd);

        // common n++ notification
        virtual void OnNppSetInfo(const NppData& nppd)  { }

        const TCHAR* getDllFileName() const  { return m_szDllFileName; }
        HMODULE getDllModule() const  { return m_hDllModule; }
        const TCHAR* getIniFileName() const  { return m_szIniFileName; }
        HWND getNppWnd() const  { return m_nppMsgr.getNppWnd(); }

};

//----------------------------------------------------------------------------
#endif
