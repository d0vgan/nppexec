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

#include "CPopupListBox.h"
#include "cpp/CStrT.h"
#include "NppExecScriptEngine.h"


const TCHAR* const NpeSearchFlagsList[] = {
  // search flags:
  _T("NPE_SF_MATCHCASE"),
  _T("NPE_SF_WHOLEWORD"),
  _T("NPE_SF_WORDSTART"),
  _T("NPE_SF_REGEXP"),
  _T("NPE_SF_POSIX"),
  _T("NPE_SF_CXX11REGEX"),
  _T("NPE_SF_BACKWARD"),
  _T("NPE_SF_NEXT"),
  _T("NPE_SF_INENTIRETEXT"),
  _T("NPE_SF_INSELECTION"),
  _T("NPE_SF_SETPOS"),
  _T("NPE_SF_SETSEL"),
  _T("NPE_SF_REPLACEALL"),
  _T("NPE_SF_PRINTALL")
};

const TCHAR* const VariablesList[] = {
  _T("$(#1)"),                 //  $(#1)
  MACRO_EXIT_CMD,              //  $(@EXIT_CMD)
  MACRO_EXIT_CMD_SILENT,       //  $(@EXIT_CMD_SILENT)
  MACRO_ARGC,                  //  $(ARGC)
  _T("$(ARGV)"),               //  $(ARGV)
  _T("$(ARGV[1])"),            //  $(ARGV[1])
  MACRO_CLIPBOARD_TEXT,        //  $(CLIPBOARD_TEXT)
  MACRO_CLOUD_LOCATION_PATH,   //  $(CLOUD_LOCATION_PATH)
  MACRO_CON_HWND,              //  $(CON_HWND)
  MACRO_CURRENT_COLUMN,        //  $(CURRENT_COLUMN)
  MACRO_FILE_DIRPATH,          //  $(CURRENT_DIRECTORY)
  MACRO_CURRENT_LINE,          //  $(CURRENT_LINE)
  MACRO_CURRENT_LINESTR,       //  $(CURRENT_LINESTR)
  MACRO_CURRENT_WORD,          //  $(CURRENT_WORD)
  MACRO_CURRENT_WORKING_DIR,   //  $(CWD)
  MACRO_EXITCODE,              //  $(EXITCODE)
  MACRO_FILE_EXTONLY,          //  $(EXT_PART)
  MACRO_FILE_FULLNAME,         //  $(FILE_NAME)
  MACRO_FILE_NAME_AT_CURSOR,   //  $(FILE_NAME_AT_CURSOR)
  MACRO_FOCUSED_HWND,          //  $(FOCUSED_HWND)
  MACRO_FILE_FULLPATH,         //  $(FULL_CURRENT_PATH)
  MACRO_INPUT,                 //  $(INPUT)
  _T("$(INPUT[1])"),           //  $(INPUT[1])
  MACRO_IS_CONSOLE,            //  $(IS_CONSOLE)
  MACRO_IS_CONSOLE0,           //  $(IS_CONSOLE0)
  MACRO_IS_PROCESS,            //  $(IS_PROCESS)
  MACRO_LAST_CMD_RESULT,       //  $(LAST_CMD_RESULT)
  MACRO_LEFT_VIEW_FILE,        //  $(LEFT_VIEW_FILE)
  MACRO_MSG_LPARAM,            //  $(MSG_LPARAM)
  MACRO_MSG_RESULT,            //  $(MSG_RESULT)
  MACRO_MSG_WPARAM,            //  $(MSG_WPARAM)
  MACRO_FILE_NAMEONLY,         //  $(NAME_PART)
  MACRO_NPP_DIRECTORY,         //  $(NPP_DIRECTORY)
  MACRO_NPP_FULL_FILE_PATH,    //  $(NPP_FULL_FILE_PATH)
  MACRO_NPP_HWND,              //  $(NPP_HWND)
  MACRO_NPP_PID,               //  $(NPP_PID)
  MACRO_NPP_SETTINGS_DIR,      //  $(NPP_SETTINGS_DIR)
  MACRO_OUTPUT,                //  $(OUTPUT)
  MACRO_OUTPUT1,               //  $(OUTPUT1)
  MACRO_OUTPUTL,               //  $(OUTPUTL)
  MACRO_PID,                   //  $(PID)
  MACRO_PLUGINS_CONFIG_DIR,    //  $(PLUGINS_CONFIG_DIR)
  _T("$(RARGV)"),              //  $(RARGV)
  _T("$(RARGV[1])"),           //  $(RARGV[1])
  MACRO_RIGHT_VIEW_FILE,       //  $(RIGHT_VIEW_FILE)
  MACRO_SCI_HWND,              //  $(SCI_HWND)
  MACRO_SCI_HWND1,             //  $(SCI_HWND1)
  MACRO_SCI_HWND2,             //  $(SCI_HWND2)
  MACRO_SELECTED_TEXT,         //  $(SELECTED_TEXT)
  _T("$(SYS.PATH)"),           //  $(SYS.PATH)
  MACRO_WORKSPACE_ITEM_DIR,    //  $(WORKSPACE_ITEM_DIR)
  MACRO_WORKSPACE_ITEM_NAME,   //  $(WORKSPACE_ITEM_NAME)
  MACRO_WORKSPACE_ITEM_PATH,   //  $(WORKSPACE_ITEM_PATH)
  MACRO_WORKSPACE_ITEM_ROOT    //  $(WORKSPACE_ITEM_ROOT)
};

const TCHAR* const DirectivesList[] = {
  DIRECTIVE_COLLATERAL
};

CPopupListBox::CPopupListBox() : CAnyListBox(), m_hParentWnd(NULL)
{
}

CPopupListBox::~CPopupListBox()
{
  // Destroy();
}

HWND CPopupListBox::Create(HWND hParentWnd, 
                           int left, int top, 
                           int width, int height)
{
  m_hWnd = ::CreateWindowEx(WS_EX_TOPMOST | WS_EX_CLIENTEDGE, 
    _T("LISTBOX"), _T(""), 
    WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_HASSTRINGS | LBS_SORT | LBS_NOTIFY,
    left, top, width, height, 
    hParentWnd, NULL, NULL, NULL);

  if (m_hWnd)
  {
    HFONT hFont = (HFONT) ::SendMessage(hParentWnd, WM_GETFONT, 0, 0);
    ::SendMessage(m_hWnd, WM_SETFONT, (WPARAM) hFont, (LPARAM) 0);
    m_hParentWnd = hParentWnd;
  }

  return m_hWnd;
}

void CPopupListBox::Destroy()
{
  if (m_hWnd)
  {
    SetParent(NULL);
    ::DestroyWindow(m_hWnd);
    m_hWnd = NULL;
  }
}

bool CPopupListBox::FillPopupList(const TCHAR* szCurrentWord)
{
  ShowWindow(SW_HIDE);
  ResetContent();

  int nLen = (szCurrentWord == NULL) ? 0 : lstrlen(szCurrentWord);
  if (nLen > 0)
  {
    tstr  WordUpper;
    tstr  S;
    bool  bExactMatch;

    WordUpper = szCurrentWord;
    NppExecHelpers::StrUpper(WordUpper);

    if ( ((nLen >= 3) && 
          (WordUpper.StartsWith(_T("NPP")) || 
           WordUpper.StartsWith(_T("CON")) ||
           WordUpper.StartsWith(_T("SEL")) ||
           WordUpper.StartsWith(_T("ENV")) ||
           WordUpper.StartsWith(_T("NPE")) ||
           WordUpper.StartsWith(_T("SCI")) )) 
      || ((nLen >= 3) &&
          (WordUpper.StartsWith(_T("INP")) ||  // INPUTBOX
           WordUpper.StartsWith(_T("MES")) ||  // MESSAGEBOX
           WordUpper.StartsWith(_T("PRO")) ||  // PROC_
           WordUpper.StartsWith(_T("SLE")) ))  // SLEEP
      || ((nLen >= 3) &&
          (WordUpper.StartsWith(_T("TEX")) ||  // TEXT_
           WordUpper.StartsWith(_T("CLI")) ))  // CLIP_
       )
    {
      bExactMatch = false;
      const auto& CommandsList = CScriptEngine::GetCommandRegistry().GetSortedCmdNames();
      for (const auto& cmd : CommandsList)
      {
        S = cmd;
        if (S.length() < 4)
          continue;

        if (S == CScriptEngine::DoEchoCommand::Name() ||
            S == CScriptEngine::DoCalcEchoCommand::Name() ||
            S == CScriptEngine::DoGoToCommand::Name() ||
            S == CScriptEngine::DoElseCommand::Name() ||
            S == CScriptEngine::DoEndIfCommand::Name() ||
            S == CScriptEngine::DoExitCommand::Name() ||
            S == CScriptEngine::DoUnsetCommand::Name() ||
            S == CScriptEngine::DoLabelCommand::Name())
          continue;

        if (S.StartsWith(WordUpper))
        {
          if (S != WordUpper)
          {
            const TCHAR ch = szCurrentWord[0];
            if ((ch >= _T('a')) && (ch <= _T('z')))
            {
              NppExecHelpers::StrLower(S); // preserve lower case
            }
            AddString(S.c_str());
          }
          else
          {
            bExactMatch = true;
            break;
          }
        }
      }
      if (!bExactMatch)
      {
        for (const TCHAR* const& cmd : NpeSearchFlagsList)
        {
          S = cmd;
          if (S.StartsWith(WordUpper))
          {
            if (S != WordUpper)
            {
              const TCHAR ch = szCurrentWord[0];
              if ((ch >= _T('A')) && (ch <= _T('Z')))
              {
                AddString(S.c_str()); // flags can be in upper case only
              }
            }
            else
            {
              bExactMatch = true;
              break;
            }
          }
        }
      }
      if (bExactMatch)
      {
        ResetContent();
      }
      return ((GetCount() > 0) ? true : false);
    }

    if ((nLen >= 2) && (WordUpper.StartsWith(_T("$("))))
    {
      bExactMatch = false;
      for (const TCHAR* const& v : VariablesList)
      {
        S = v;
        if (S.StartsWith(WordUpper))
        {
          if (S != WordUpper)
          {
            AddString(S.c_str());
          }
          else
          {
            bExactMatch = true;
            break;
          }
        }
      }
      if (bExactMatch)
      {
        ResetContent();
      }
      return ((GetCount() > 0) ? true : false);
    }

    if ((nLen >= 2) && WordUpper.StartsWith(_T("!")))
    {
      bExactMatch = false;
      for (const TCHAR* const& d : DirectivesList)
      {
        S = d;
        if (S.StartsWith(WordUpper))
        {
          if (S != WordUpper)
          {
            const TCHAR ch = szCurrentWord[1];
            if ((ch >= _T('a')) && (ch <= _T('z')))
            {
              NppExecHelpers::StrLower(S); // preserve lower case
            }
            AddString(S.c_str());
          }
          else
          {
            bExactMatch = true;
            break;
          }
        }
      }
      if (bExactMatch)
      {
        ResetContent();
      }
      return ((GetCount() > 0) ? true : false);
    }

  }
  return false;
}

HWND CPopupListBox::GetParentWnd() const
{
  return m_hParentWnd;
}

void CPopupListBox::SetParentWnd(HWND hParentWnd)
{
  m_hParentWnd = hParentWnd;
}

int CPopupListBox::getRequiredWidth()
{
  int nWidth = 0;

  HDC hDC = ::GetDC(m_hWnd);
  if (hDC)
  {
    HFONT hFont = (HFONT) ::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
    HGDIOBJ hPrevFont = ::SelectObject(hDC, hFont);
    SIZE sz;
    TCHAR szItemText[64];

    int nItems = GetCount();
    for (int n = 0; n < nItems; ++n)
    {
      sz.cx = 0;
      sz.cy = 0;

      int nItemTextLen = GetString(n, szItemText);
      szItemText[nItemTextLen++] = _T('W');
      szItemText[nItemTextLen] = 0;
      ::GetTextExtentPoint32(hDC, szItemText, nItemTextLen, &sz);

      if (nWidth < sz.cx)
        nWidth = sz.cx;
    }

    ::SelectObject(hDC, hPrevFont);
    ::ReleaseDC(m_hWnd, hDC);
  }

  if (nWidth > 0)
  {
    int nScrollBarWidth = ::GetSystemMetrics(SM_CXVSCROLL);
    nWidth += nScrollBarWidth;
  }
  else
  {
    nWidth = 160;
  }
  return nWidth;
}

bool CPopupListBox::Show(const TCHAR* szCurrentWord)
{
  if (FillPopupList(szCurrentWord))
  {
    POINT pt;
    RECT  rc;
        
    if (::GetCaretPos(&pt) && ::GetClientRect(m_hParentWnd, &rc))
    {
      int itemHeight = 0;
      int itemsCount = GetCount();
      if (itemsCount > 0)
      {
        RECT itemRect = { 0, 0, 0, 0 };
        if (SendMsg(LB_GETITEMRECT, 0, (LPARAM)&itemRect) != LB_ERR)
          itemHeight = itemRect.bottom - itemRect.top;
        if (itemHeight == 0)
          itemHeight = (int) SendMsg(LB_GETITEMHEIGHT, 0, 0);
      }

      int y;
      int height;
      int x = pt.x;
      int width = getRequiredWidth();
      int ih = (itemHeight > 0) ? itemHeight : 12;
      if (x + width > rc.right - rc.left)
      {
        x = rc.right - rc.left - width;
      }
      if (pt.y <= (rc.bottom - rc.top - ih)/2)
      {
        y = pt.y + ih;
        height = rc.bottom - rc.top - y;
      }
      else
      {
        y = rc.top;
        height = pt.y + ih/2;
      }

      // adjust the height
      if (itemHeight > 0)
      {
        int heightEnough = itemHeight*itemsCount + 24;
        if (height > heightEnough)
        {
          if (y == rc.top)
            y += (height - heightEnough);
          height = heightEnough;
        }
      }

      MoveWindow(x, y, width, height);
      ShowWindow();
      return true;
    }
  }
  return false;
}

