#include "NppPlugin.h"


CNppPlugin::CNppPlugin()
{
    m_hDllModule = NULL;
    m_szDllFileName[0] = 0;
    m_szIniFileName[0] = 0;
}

CNppPlugin::~CNppPlugin()
{
}

void CNppPlugin::OnDllProcessAttach(HINSTANCE hDLLInstance)
{
    int   nLen;
    TCHAR szPath[2*MAX_PATH + 1];
    
    m_hDllModule = (HMODULE) hDLLInstance;
    nLen = (int) ::GetModuleFileName(m_hDllModule, szPath, 2*MAX_PATH);
    while ( nLen-- > 0 )
    {
        if ( (szPath[nLen] == _T('\\')) || (szPath[nLen] == _T('/')) )
        {
            lstrcpy(m_szDllFileName, szPath + nLen + 1);
            lstrcpy(m_szIniFileName, m_szDllFileName);
            break;
        }
    }

    nLen = lstrlen(m_szIniFileName);
    while ( nLen-- > 0 )
    {
        if ( m_szIniFileName[nLen] == _T('.') )
        {
            lstrcpy(m_szIniFileName + nLen + 1, _T("ini"));
            break;
        }
    }
}

void CNppPlugin::OnDllProcessDetach()
{
}

void CNppPlugin::nppSetInfo(const NppData& nppd)
{
    m_nppMsgr.setNppData(nppd);
    OnNppSetInfo(nppd);
}

