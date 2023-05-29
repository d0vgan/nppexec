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

#ifndef _advanced_options_dlg_h_
#define _advanced_options_dlg_h_
//-------------------------------------------------------------------------
#include "base.h"
#include "resource.h"
#include "CAnyWindow.h"
#include "CAnyComboBox.h"
#include "CAnyCheckBox.h"
#include "CAnyListBox.h"
#include "cpp/CBufT.h"


extern const TCHAR* const cszUserMenuItemSep;
extern const TCHAR* const cszUserMenuSeparator;

INT_PTR CALLBACK AdvancedOptionsDlgProc(HWND, UINT, WPARAM, LPARAM);

class CAdvOptDlg : public CAnyWindow
{
    public:
        enum eConst {
            CON_AUTO = 0,
            CON_YES  = 1,
            CON_NO   = 2
        };

    protected:
        CAnyCheckBox m_chMacrosSubmenu;
        CAnyListBox  m_lbMenuItems;
        CAnyComboBox m_cbItemScript;
        CAnyWindow   m_edItemName;
        CAnyComboBox m_cbScriptNppStart;
        CAnyComboBox m_cbScriptNppExit;
        CAnyComboBox m_cbHotKey;
        CAnyComboBox m_cbToolbarBtn;
        CAnyComboBox m_cbConVisible;
        CAnyComboBox m_cbConShowHelp;
        CAnyComboBox m_cbSaveCmdHst;
        CAnyWindow   m_edCommentDelim;
        CAnyWindow   m_edTextColorNorm;
        CAnyWindow   m_edTextColorErr;
        CAnyWindow   m_edTextColorMsg;
        CAnyWindow   m_edBkColor;
        CAnyWindow   m_btMoveUp;
        CAnyWindow   m_btMoveDown;
        CAnyWindow   m_btModify;
        CAnyWindow   m_btDelete;
        CAnyCheckBox m_chUseEditorColorsInConsole;
        CAnyCheckBox m_chUseEditorColorsInExecDlg;

        HWND         m_hToolTip;

        COLORREF     m_OrgColorTextNorm;
        COLORREF     m_OrgColorTextErr;
        COLORREF     m_OrgColorTextMsg;
        COLORREF     m_OrgColorBkgnd;
        int          m_nREMaxLen;
        bool         m_bUseEditorColorsInConsole;
        bool         m_bNppRestartRequired;

    protected:
        void colorValuesInit();
        BOOL colorValuesChanged();

    public:
        CAdvOptDlg();
        ~CAdvOptDlg();

        void OnInitDlg(HWND hDlg);
        void OnEndDlg();
        BOOL OnBtOK();
        void OnBtCancel();
        void OnBtItemNew();
        void OnBtItemDelete();
        void OnBtMoveUp();
        void OnBtMoveDown();
        void OnCbItemScriptSelChange();
        void OnCbOptHotkeySelChange();
        void OnCbOptToolbarBtnSelChange();
        void OnChMacrosSubmenu();
        void OnChEditorColorsConsole();
        void OnEdItemNameChange();
        void OnLbMenuItemsSelChange();

        void ShowError(LPCTSTR szMessage);
        void ShowWarning(LPCTSTR szMessage);
};

//-------------------------------------------------------------------------
#endif
