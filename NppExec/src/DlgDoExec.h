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

#ifndef _do_exec_dlg_h_
#define _do_exec_dlg_h_
//-------------------------------------------------------------------------
#include "base.h"
#include "CAnyWindow.h"
#include "CAnyRichEdit.h"
#include "CAnyComboBox.h"
#include "CPopupListBox.h"
#include "NppExec.h"
#include "resource.h"

#define MAX_SCRIPTNAME 256

INT_PTR CALLBACK DoExecDlgProc(HWND, UINT, WPARAM, LPARAM);

class CDoExecDlg : public CAnyWindow
{
private:
  TCHAR        tmp_scriptname[MAX_SCRIPTNAME];
  bool         isScriptFileChanged();

  static int   nCurrentDlgWidth;
  static int   nCurrentDlgHeight;
  static int   nInitialDlgWidth;
  static int   nInitialDlgHeight;
  static RECT  rcEdScriptInitial;
  static RECT  rcCbScriptsInitial;
  static RECT  rcBtOkInitial;
  static RECT  rcBtSaveInitial;
  static RECT  rcBtCancelInitial;

public: 
  CAnyRichEdit  m_edScript;
  CAnyComboBox  m_cbScriptNames;
  CAnyWindow    m_btOK;
  CAnyWindow    m_btSave;
  CAnyWindow    m_btCancel;
  CPopupListBox m_lbPopupList;
  TCHAR         m_szScriptNameToSave[MAX_SCRIPTNAME];
  int           m_nCurrentWordStart;
  int           m_nCurrentWordEnd;
  bool          m_bFirstSetFocus;

  CDoExecDlg();
  ~CDoExecDlg();
  void OnBtOK(BOOL bUpdateCmdList);
  void OnBtSave();
  void OnCbnSelChange();
  void OnInitDialog(HWND hDlg);
  void OnSize();
  void OnSizing(RECT* lpRect);
  void ShowScriptText(const tstr& ScriptName);
};

class CScriptNameDlg : public CAnyWindow
{
private:
  TCHAR        tmp_scriptname[MAX_SCRIPTNAME];

public:
  CAnyComboBox m_cbScriptName;
  CAnyWindow   m_btSave;
  CAnyWindow   m_btDelete;
  CAnyWindow   m_btCancel;

  CScriptNameDlg();
  ~CScriptNameDlg();
  void OnBtDelete();
  bool OnBtSave();
  void OnCbnCloseUp();
  void OnCbnEditChange();
  void OnCbnSelChange();
  void OnInitDialog(HWND hDlg);
};

//-------------------------------------------------------------------------
#endif
