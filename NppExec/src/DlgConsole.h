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

#ifndef _console_dlg_h_
#define _console_dlg_h_
//-------------------------------------------------------------------------
#include "base.h"
#include "CAnyWindow.h"
#include "resource.h"
#include "NppExecHelpers.h"


// this is sent in case of CScriptEngine's NPP_CONSOLE command --
#define WM_CONSOLEDLG_CLOSE (WM_USER + 7701)

// hm, what is this? --
#define WM_NPPCLOSE         (WM_USER + 1011)


extern const TCHAR PLUGIN_CURRENT_VER[];

INT_PTR CALLBACK ConsoleDlgProc(HWND, UINT, WPARAM, LPARAM);

namespace ConsoleDlg 
{
    enum eConsoleShortcutKeySupport {
        CSK_OFF = 0x00,
        CSK_STD = 0x01,
        CSK_USR = 0x02,
        CSK_ALL = 0x03
    };

    enum eCloseConsoleFlags
    {
        ccfNppIsClosing        = 0x01,
        ccfCmdNppConsole       = 0x10  // see also: CNppExec::scfCmdNppConsole
    };

    void DockDialog();
    bool IsDocked();
    const TCHAR* GetTitle();
    bool EnableTransparency(bool bEnable);
    void SaveCmdHistory();
    void GoToError(int direction);
    bool IsConsoleHelpCommand(const tstr& S);
    bool IsConsoleVerCommand(const tstr& S);

    extern int GoToError_nCurrentLine;
}

//-------------------------------------------------------------------------
#endif
