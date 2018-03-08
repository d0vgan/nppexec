#ifndef _npp_messager_h_
#define _npp_messager_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "npp_stuff/PluginInterface.h"
#include "npp_stuff/menuCmdID.h"

class CNppMessager
{
    protected:
        NppData m_nppData;

    public:
        CNppMessager();
        CNppMessager(const NppData& nppd);
        virtual ~CNppMessager();

        LRESULT SendNppMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0);
        LRESULT SendNppMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) const;
        
        BOOL    getCurrentFileFullPath(int strLen, TCHAR *str) const;
        BOOL    getCurrentFileDirectory(int strLen, TCHAR *str) const;
        BOOL    getCurrentFileNameExt(int strLen, TCHAR *str) const;
        BOOL    getCurrentFileNamePart(int strLen, TCHAR *str) const;
        BOOL    getCurrentFileExtPart(int strLen, TCHAR *str) const;
        HWND    getCurrentScintillaWnd() const;
        BOOL    getCurrentWord(int strLen, TCHAR *str) const;
        BOOL    getNppDirectory(int strLen, TCHAR *str) const;
        HWND    getNppWnd() const  { return m_nppData._nppHandle; }
        HMENU   getNppMainMenu() const;
        HMENU   getNppPluginMenu() const;
        void    getPluginsConfigDir(int strLen, TCHAR *str) const;
        void    makeCurrentBufferDirty();
        void    setNppData(const NppData& nppd)  { m_nppData = nppd; }

};

//---------------------------------------------------------------------------
#endif
