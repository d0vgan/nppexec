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
#include "NppExecEngine.h"


const TCHAR* CommandsList[] = {
  CMD_CONCOLOUR,
  CMD_CONFILTER,
  CMD_CONLOADFROM,
  CMD_CONSAVETO,
  CMD_ENVSET,
  CMD_ENVUNSET,
  CMD_INPUTBOX,
  CMD_NPECMDALIAS,
  CMD_NPECONSOLE,
  CMD_NPEDEBUGLOG,
  CMD_NPENOEMPTYVARS,
  CMD_NPEQUEUE,
  CMD_NPPCLOSE,
  CMD_NPPCONSOLE,
  CMD_NPPEXEC,
  CMD_NPPMENUCOMMAND,
  CMD_NPPOPEN,
  CMD_NPPRUN,
  CMD_NPPSAVE,
  CMD_NPPSAVEALL,
  CMD_NPPSAVEAS,
  CMD_NPPSENDMSG,
  CMD_NPPSENDMSGEX,
  CMD_NPPSETFOCUS,
  CMD_NPPSWITCH,
  CMD_PROCSIGNAL,
  CMD_SCIFIND,
  CMD_SCIREPLACE,
  CMD_SCISENDMSG,
  CMD_SELLOADFROM,
  CMD_SELSAVETO,
  CMD_SELSETTEXT,
  CMD_SELSETTEXTEX,
  CMD_SLEEP,
  CMD_TEXTLOADFROM,
  CMD_TEXTSAVETO,

  // search flags:
  _T("NPE_SF_MATCHCASE"),
  _T("NPE_SF_WHOLEWORD"),
  _T("NPE_SF_WORDSTART"),
  _T("NPE_SF_REGEXP"),
  _T("NPE_SF_POSIX"),
  _T("NPE_SF_CXX11REGEX"),
  _T("NPE_SF_BACKWARD"),
  _T("NPE_SF_NEXT"),
  _T("NPE_SF_INSELECTION"),
  _T("NPE_SF_INWHOLETEXT"),
  _T("NPE_SF_SETPOS"),
  _T("NPE_SF_SETSEL"),
  _T("NPE_SF_REPLACEALL"),
  _T("NPE_SF_PRINTALL")
};

const TCHAR* VariablesList[] = {
  _T("$(#1)"),               //  $(#1)
  MACRO_EXIT_CMD,            //  $(@EXIT_CMD)
  MACRO_EXIT_CMD_SILENT,     //  $(@EXIT_CMD_SILENT)
  MACRO_ARGC,                //  $(ARGC)
  _T("$(ARGV)"),             //  $(ARGV)
  _T("$(ARGV[1])"),          //  $(ARGV[1])
  MACRO_CLIPBOARD_TEXT,      //  $(CLIPBOARD_TEXT)
  MACRO_CURRENT_COLUMN,      //  $(CURRENT_COLUMN)
  MACRO_FILE_DIRPATH,        //  $(CURRENT_DIRECTORY)
  MACRO_CURRENT_LINE,        //  $(CURRENT_LINE)
  MACRO_CURRENT_WORD,        //  $(CURRENT_WORD)
  MACRO_CURRENT_WORKING_DIR, //  $(CWD)
  MACRO_EXITCODE,            //  $(EXITCODE)
  MACRO_FILE_EXTONLY,        //  $(EXT_PART)
  MACRO_FILE_FULLNAME,       //  $(FILE_NAME)
  MACRO_FILE_FULLPATH,       //  $(FULL_CURRENT_PATH)
  MACRO_INPUT,               //  $(INPUT)
  _T("$(INPUT[1])"),         //  $(INPUT[1])
  MACRO_LAST_CMD_RESULT,     //  $(LAST_CMD_RESULT)
  MACRO_LEFT_VIEW_FILE,      //  $(LEFT_VIEW_FILE)
  MACRO_MSG_LPARAM,          //  $(MSG_LPARAM)
  MACRO_MSG_RESULT,          //  $(MSG_RESULT)
  MACRO_MSG_WPARAM,          //  $(MSG_WPARAM)
  MACRO_FILE_NAMEONLY,       //  $(NAME_PART)
  MACRO_NPP_DIRECTORY,       //  $(NPP_DIRECTORY)
  MACRO_OUTPUT,              //  $(OUTPUT)
  MACRO_OUTPUT1,             //  $(OUTPUT1)
  MACRO_OUTPUTL,             //  $(OUTPUTL)
  MACRO_PID,                 //  $(PID)
  MACRO_PLUGINS_CONFIG_DIR,  //  $(PLUGINS_CONFIG_DIR)
  _T("$(RARGV)"),            //  $(RARGV)
  _T("$(RARGV[1])"),         //  $(RARGV[1])
  MACRO_RIGHT_VIEW_FILE,     //  $(RIGHT_VIEW_FILE)
  _T("$(SYS.PATH)")          //  $(SYS.PATH)
};

CPopupListBox::CPopupListBox() : CAnyListBox()
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
    WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_HASSTRINGS | LBS_SORT,
    left, top, width, height, 
    hParentWnd, NULL, NULL, NULL);
  
  if (m_hWnd)
  {
    HFONT   hFont;

    hFont = (HFONT) ::SendMessage(hParentWnd, WM_GETFONT, 0, 0);
    ::SendMessage(m_hWnd, WM_SETFONT, (WPARAM) hFont, (LPARAM) FALSE);
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
          (WordUpper.StartsWith(_T("INP")) ||
           WordUpper.StartsWith(_T("PRO")) ||
           WordUpper.StartsWith(_T("SLE")) ))
      || ((nLen >= 3) &&
          (WordUpper.StartsWith(_T("TEX")) ))
       )
    {
      bExactMatch = false;
      for (const TCHAR* const& cmd : CommandsList)
      {
        S = cmd;
        if (S.StartsWith(WordUpper))
        {
          if (S != WordUpper)
          {
            bool bAdd = true;
            const TCHAR ch = szCurrentWord[0];
            if ((ch >= _T('a')) && (ch <= _T('z')))
            {
              if (S.StartsWith(_T("NPE_SF_")))
                bAdd = false; // flags can be in upper case only
              else
                NppExecHelpers::StrLower(S); // preserve lower case
            }
            if (bAdd)
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
      int width = 160;
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

