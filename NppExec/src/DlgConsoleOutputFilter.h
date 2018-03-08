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

#ifndef _console_output_filter_dlg_h_
#define _console_output_filter_dlg_h_
//-------------------------------------------------------------------------
#include "base.h"
#include "CAnyWindow.h"
#include "CAnyCheckBox.h"
#include "CAnyComboBox.h"
#include "WarningAnalyzer.h"
#include "NppExec.h"
#include "cpp/CListT.h"

INT_PTR CALLBACK ConsoleOutputFilterDlgProc(HWND, UINT, WPARAM, LPARAM);


class CConsoleOutputFilterDlg : public CAnyWindow 
{
public:
  static const int FILTER_ITEMS          = 5;
  static const int RECOGNITION_ITEMS     = WARN_MAX_FILTER;
  static const int HISTORY_ITEMS         = 16;
  static const int REPLACE_ITEMS         = 4;

  static const int FILTER_REC_RED        = 0;
  static const int FILTER_REC_GREEN      = 1;
  static const int FILTER_REC_BLUE       = 2;
  static const int FILTER_REC_COLOR      = 3;
  static const int FILTER_REC_ITALIC     = 0;
  static const int FILTER_REC_BOLD       = 1;
  static const int FILTER_REC_UNDERLINED = 2;
  static const int FILTER_REC_STYLE      = 3;

  static const DWORD RGB_BK_CHECKBOX = RGB(16, 112, 192);

private:
  enum eDlg {
      DLG_FLTR = 0,
      DLG_RPLC,
      DLG_HGLT,

      DLG_COUNT
  };

  HWND m_hTabDlg[DLG_COUNT];
  HWND m_hTabs;

  int  m_nLastTab;
  HBRUSH m_hBrushBkCheckbox;
  
  CAnyCheckBox m_ch_Include[FILTER_ITEMS];
  CAnyComboBox m_cb_Include[FILTER_ITEMS];
  CAnyCheckBox m_ch_Exclude[FILTER_ITEMS];
  CAnyComboBox m_cb_Exclude[FILTER_ITEMS];
  CAnyCheckBox m_ch_FilterEnable;
  CAnyCheckBox m_ch_ExcludeAllEmpty;
  CAnyCheckBox m_ch_ExcludeDupEmpty;
  
  CAnyCheckBox m_ch_RFind[REPLACE_ITEMS];
  CAnyCheckBox m_ch_RCase[REPLACE_ITEMS];
  CAnyComboBox m_cb_RFind[REPLACE_ITEMS];
  CAnyComboBox m_cb_RRplc[REPLACE_ITEMS];
  CAnyCheckBox m_ch_REnable;
  CAnyCheckBox m_ch_RExcludeEmpty;

  CAnyCheckBox m_ch_Recognition[RECOGNITION_ITEMS];
  CAnyComboBox m_cb_Recognition[RECOGNITION_ITEMS];
  CAnyWindow   m_ed_Recognition_Color[RECOGNITION_ITEMS][FILTER_REC_COLOR];
  CAnyCheckBox m_ch_Recognition_Style[RECOGNITION_ITEMS][FILTER_REC_STYLE];

public:
  CListT<tstr> m_ExcludeHistory;
  CListT<tstr> m_IncludeHistory;
  CListT<tstr> m_RFindHistory;
  CListT<tstr> m_RRplcHistory;
  CListT<tstr> m_HighlightHistory;

public:
  CConsoleOutputFilterDlg();
  ~CConsoleOutputFilterDlg();

  void OnBtOK();
  void OnBtCancel();
  void OnChFilterEnable();
  void OnChRplcFilterEnable();
  void OnEndDialog();
  void OnInitDialog(HWND hDlg);
  void OnInitDlgFltr(HWND hDlgFltr);
  void OnInitDlgHglt(HWND hDlgHglt);
  void OnInitDlgRplc(HWND hDlgRplc);
  void OnNotify(HWND hDlg, LPNMHDR pnmhdr);
  void OnCbnDropDown(unsigned int idComboBox);
  INT_PTR OnCtlColorStatic(HWND hDlg, WPARAM wParam, LPARAM lParam);
};

//-------------------------------------------------------------------------
#endif

