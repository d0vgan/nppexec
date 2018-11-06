#ifndef _npp_plugin_msg_sender_h_
#define _npp_plugin_msg_sender_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "npp_files/PluginInterface.h"
#include <string>

class CNppPluginMsgSender
{
    protected:
        HWND m_hNppWnd;
        std::basic_string<TCHAR> m_srcModuleName;

    public:
        CNppPluginMsgSender(HWND hNppWnd, const TCHAR* srcModuleName) : 
          m_hNppWnd(hNppWnd), m_srcModuleName(srcModuleName)
        {
        }

        void setSrcModuleName(const TCHAR* srcModuleName)
        {
            m_srcModuleName = srcModuleName;
        }

        BOOL SendMsg(const TCHAR *destModuleName, long internalMsg, void* info)
        {
            CommunicationInfo ci = { internalMsg, 
                                     m_srcModuleName.c_str(),
                                     info };
            return (BOOL) ::SendMessage( m_hNppWnd, 
                                         NPPM_MSGTOPLUGIN,
                                         (WPARAM) destModuleName,
                                         (LPARAM) &ci 
                                       );
        }

};

//---------------------------------------------------------------------------
#endif
