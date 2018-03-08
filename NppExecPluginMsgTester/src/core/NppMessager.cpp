#include "NppMessager.h"

CNppMessager::CNppMessager()
{
    m_nppData._nppHandle = NULL;
    m_nppData._scintillaMainHandle = NULL;
    m_nppData._scintillaSecondHandle = NULL;
}

CNppMessager::CNppMessager(const NppData& nppd)
{
    m_nppData = nppd;
}

CNppMessager::~CNppMessager()
{
}

LRESULT CNppMessager::SendNppMsg(UINT uMsg, WPARAM wParam , LPARAM lParam )
{
    return ::SendMessage(m_nppData._nppHandle, uMsg, wParam, lParam);
}

LRESULT CNppMessager::SendNppMsg(UINT uMsg, WPARAM wParam , LPARAM lParam ) const
{
    return ::SendMessage(m_nppData._nppHandle, uMsg, wParam, lParam);
}

BOOL CNppMessager::getCurrentFileFullPath(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETFULLCURRENTPATH, (WPARAM) strLen, (LPARAM) str );
}

BOOL CNppMessager::getCurrentFileDirectory(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETCURRENTDIRECTORY, (WPARAM) strLen, (LPARAM) str );
}

BOOL CNppMessager::getCurrentFileNameExt(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETFILENAME, (WPARAM) strLen, (LPARAM) str );
}

BOOL CNppMessager::getCurrentFileNamePart(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETNAMEPART, (WPARAM) strLen, (LPARAM) str );
}

BOOL CNppMessager::getCurrentFileExtPart(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETEXTPART, (WPARAM) strLen, (LPARAM) str );
}

HWND CNppMessager::getCurrentScintillaWnd() const
{
    int currentView = 0;
    SendNppMsg( NPPM_GETCURRENTSCINTILLA, 0, (LPARAM) &currentView );
    return ( (currentView == 0) ? 
      m_nppData._scintillaMainHandle : m_nppData._scintillaSecondHandle );
}

BOOL CNppMessager::getCurrentWord(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETCURRENTWORD, (WPARAM) strLen, (LPARAM) str );
}

BOOL CNppMessager::getNppDirectory(int strLen, TCHAR *str) const
{
    return (BOOL) SendNppMsg( NPPM_GETNPPDIRECTORY, (WPARAM) strLen, (LPARAM) str );
}

HMENU CNppMessager::getNppMainMenu() const
{
    return ::GetMenu(m_nppData._nppHandle);
}

HMENU CNppMessager::getNppPluginMenu() const
{
    return (HMENU) SendNppMsg(NPPM_GETMENUHANDLE, NPPPLUGINMENU, 0);
}

void CNppMessager::getPluginsConfigDir(int strLen, TCHAR *str) const
{
    SendNppMsg(NPPM_GETPLUGINSCONFIGDIR, (WPARAM) strLen, (LPARAM) str);
}

void CNppMessager::makeCurrentBufferDirty()
{
    SendNppMsg(NPPM_MAKECURRENTBUFFERDIRTY);
}

