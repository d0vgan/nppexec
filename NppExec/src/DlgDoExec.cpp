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

#include "DlgDoExec.h"
#include "cpp/CListT.h"
#include "CFileModificationChecker.h"
#include "NppExecHelpers.h"


CDoExecDlg               DoExecDlg;
CScriptNameDlg           ScriptNameDlg;
int                      g_nCurrentWordStart;
int                      g_nCurrentWordEnd;

int  CDoExecDlg::nCurrentDlgWidth = -1;
int  CDoExecDlg::nCurrentDlgHeight = -1;
int  CDoExecDlg::nInitialDlgWidth = -1;
int  CDoExecDlg::nInitialDlgHeight = -1;
RECT CDoExecDlg::rcEdScriptInitial;
RECT CDoExecDlg::rcCbScriptsInitial;
RECT CDoExecDlg::rcBtOkInitial;
RECT CDoExecDlg::rcBtSaveInitial;
RECT CDoExecDlg::rcBtCancelInitial;

const TCHAR TEMP_SCRIPT_NAME[] = _T(" <temporary script>");

#define  MAX_PLUGIN_NAME  60
extern TCHAR EXECUTE_DLG_TITLE[MAX_PLUGIN_NAME + 10];


// this variable stores previous script name from DoExecDlg
// to use it when the script name is changed
static TCHAR  g_szPrevScriptName[MAX_SCRIPTNAME] = _T("\0");

INT_PTR  CALLBACK ScriptNameDlgProc(HWND, UINT, WPARAM, LPARAM);
int      findStrInComboBox(const TCHAR* str, CAnyComboBox& cb);
WNDPROC  OriginalEditProc;
LRESULT  CALLBACK edScriptWindowProc(HWND, UINT, WPARAM, LPARAM);


INT_PTR CALLBACK DoExecDlgProc(
  HWND   hDlg, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM lParam)
{
  if (uMessage == WM_COMMAND)
  {
    switch (LOWORD(wParam)) 
    {
      case IDSAVE: // Save
      {
        DoExecDlg.OnBtSave();
        return 1;
      }
      case IDOK: // Run
      {
        if (!DoExecDlg.m_lbPopupList.IsWindowVisible())
        {
          int ind = DoExecDlg.m_cbScriptNames.GetCurSel();
          if ( ind >= 0 )
          {
            Runtime::GetNppExec().GetOptions().SetInt( OPTI_SELECTED_SCRIPT, ind );
          }

          DoExecDlg.OnBtOK(TRUE);
          DoExecDlg.m_lbPopupList.Destroy();
          EndDialog(hDlg, 1); // OK - returns 1
        }
        else
        {
          int i = DoExecDlg.m_lbPopupList.GetCurSel();
          if (i >= 0)
          {
            TCHAR str[200];
            DoExecDlg.m_lbPopupList.GetString(i, str);
            DoExecDlg.m_lbPopupList.ShowWindow(SW_HIDE);
            DoExecDlg.m_edScript.SetSel(DoExecDlg.m_nCurrentWordStart,
              DoExecDlg.m_nCurrentWordEnd);
            DoExecDlg.m_edScript.ReplaceSelText(str);
            DoExecDlg.m_edScript.Redraw();
            DoExecDlg.m_edScript.SetFocus();
          }
        }
        return 1;
      }
      case IDCANCEL: // Cancel
      {
        if (!DoExecDlg.m_lbPopupList.IsWindowVisible())
        {
          DoExecDlg.m_lbPopupList.Destroy();
          EndDialog(hDlg, 0); // Cancel - returns 0
        }
        else
        {
          DoExecDlg.m_lbPopupList.ShowWindow(SW_HIDE);
          DoExecDlg.SetFocus();
          DoExecDlg.m_edScript.Redraw();
          DoExecDlg.m_edScript.SetFocus();
        }
        return 1;
      }
      default:
        break;
    }
    switch (HIWORD(wParam))
    {
      case CBN_SELCHANGE:
      {
        DoExecDlg.OnCbnSelChange();
        return 1;
      }
      default:
        break;
    }
  }
      
  else if (uMessage == WM_SIZING)
  {
    DoExecDlg.OnSizing( (RECT *) lParam );
  }
  
  else if (uMessage == WM_SIZE)
  {
    DoExecDlg.OnSize();
  }
  
  else if (uMessage == WM_NOTIFY)
  {
    
  }

  else if (uMessage == WM_CTLCOLOREDIT)
  {
    return DoExecDlg.OnCtlColorEdit(wParam, lParam);
  }

  else if (uMessage == WM_CTLCOLORLISTBOX)
  {
    return DoExecDlg.OnCtlColorListBox(wParam, lParam);
  }

  else if (uMessage == WM_INITDIALOG)
  {
    DoExecDlg.OnInitDialog(hDlg);
  }

  /*
  // It works here.
  // But here it is not needed :-)
  else if (uMessage == WM_NCLBUTTONDOWN || uMessage == WM_LBUTTONDOWN)
  {
    MessageBox(NULL, "left mouse btn pressed", "DoExecDlgProc", MB_OK);
  }
  */

  return 0;
}

CDoExecDlg::CDoExecDlg() : CAnyWindow()
{
  m_nCurrentWordStart = 0;
  m_nCurrentWordEnd = 0;
  m_bFirstSetFocus = true;
  m_editorTextNorm = 0xFFFFFFFF;
  m_editorBkgnd = 0xFFFFFFFF;
  m_hBkgndBrush = NULL;
}

CDoExecDlg::~CDoExecDlg()
{
  if ( m_hBkgndBrush != NULL )
  {
    ::DeleteObject(m_hBkgndBrush);
  }
}

void CDoExecDlg::OnBtOK(BOOL bUpdateCmdList)
{
  BOOL bModified = m_edScript.GetModify();

  if (bUpdateCmdList || bModified)
  {
    CNppExec& NppExec = Runtime::GetNppExec();
    CNppScript   Script;
    CNppScript*  pScript;
    bool         bTempScript;

    tmp_scriptname[0] = 0;
    m_cbScriptNames.GetWindowText(tmp_scriptname, MAX_SCRIPTNAME - 1);
      
    if (lstrcmpi(tmp_scriptname, TEMP_SCRIPT_NAME) == 0)
    {
      pScript = &NppExec.m_TempScript;
      bTempScript = true;
    }
    else
    {
      pScript = &Script;
      bTempScript = false;
    }

    CListT<tstr> CmdList;

    if (bModified) 
      pScript->GetCmdList().DeleteAll();

    TCHAR szScriptLine[CONSOLECOMMAND_BUFSIZE];
    int nLines = m_edScript.GetLineCount();
    for (int i = 0; i < nLines; i++)
    {
      m_edScript.GetLine(i, szScriptLine, CONSOLECOMMAND_BUFSIZE - 1);
      if (bUpdateCmdList)
        CmdList.Add(szScriptLine);
      if (bModified) 
        pScript->GetCmdList().Add(szScriptLine);
    }

    if (bUpdateCmdList)
      NppExec.SetCmdList(CmdList);
    
    if (bModified)
    {
      if (bTempScript) 
      {
        NppExec.m_TempScriptIsModified = true;
      }
      else
      {
        // if this script does not exist it will be created.
        // otherwise it will be modified
        NppExec.m_ScriptsList.AddScript(tmp_scriptname, Script);
      }
    }

    if ( NppExec.m_bAnotherNppExecDllExists )
    {
      NppExec.SaveScripts(0);
    }
  
  } // (bUpdateCmdList || bModified)

}

void CDoExecDlg::OnBtSave()
{
  int i = m_cbScriptNames.GetCurSel();
  if (i >= 0)
  {
    m_cbScriptNames.GetText(m_szScriptNameToSave, MAX_SCRIPTNAME - 1);
    if ( lstrcmpi(m_szScriptNameToSave, TEMP_SCRIPT_NAME) == 0 )
    {
      m_szScriptNameToSave[0] = 0;
    }
  }
  else
  {
    m_szScriptNameToSave[0] = 0;
  }

  m_btOK.EnableWindow(FALSE);
  m_btSave.EnableWindow(FALSE);
  m_btCancel.EnableWindow(FALSE);
      
  if (DialogBox(
        (HINSTANCE) Runtime::GetNppExec().m_hDllModule, 
        MAKEINTRESOURCE(IDD_SCRIPTNAME),
        m_hWnd,
        ScriptNameDlgProc) == 1)
  {
    int nLen = lstrlen(m_szScriptNameToSave);
    if (nLen > 0)
    {
      int j = findStrInComboBox(m_szScriptNameToSave, m_cbScriptNames);
      if (j < 0)
      {
        j = m_cbScriptNames.AddString(m_szScriptNameToSave);
      }
      m_cbScriptNames.SetCurSel(j);
    }
  }
  
  m_btOK.EnableWindow(TRUE);
  m_btSave.EnableWindow(TRUE);
  m_btCancel.EnableWindow(TRUE); 
  m_btSave.SetFocus();
}

void CDoExecDlg::OnCbnSelChange()
{
  m_bFirstSetFocus = true;
  
  // updating g_szPrevScriptName value
  m_cbScriptNames.GetWindowText(g_szPrevScriptName, MAX_SCRIPTNAME - 1);
  //MessageBox(hDlg, g_szPrevScriptName, "script name:", MB_OK);

  ShowScriptText(g_szPrevScriptName);
}

INT_PTR CDoExecDlg::OnCtlColorEdit(WPARAM wParam, LPARAM lParam)
{
  (lParam);

  HBRUSH hBkBrush;
  COLORREF crBkColor;
  COLORREF crTextColor;

  if ( m_hBkgndBrush != NULL )
  {
    hBkBrush = m_hBkgndBrush;
    crBkColor = m_editorBkgnd;
    crTextColor = m_editorTextNorm;
  }
  else
  {
    hBkBrush = GetSysColorBrush(COLOR_WINDOW);
    crBkColor = GetSysColor(COLOR_WINDOW);
    crTextColor = GetSysColor(COLOR_WINDOWTEXT);
  }

  SetTextColor( (HDC) wParam, crTextColor );
  SetBkMode( (HDC) wParam, OPAQUE );
  SetBkColor( (HDC) wParam, crBkColor );
  return (INT_PTR) hBkBrush;
}

INT_PTR CDoExecDlg::OnCtlColorListBox(WPARAM wParam, LPARAM lParam)
{
  (lParam);

  HBRUSH hBkBrush;
  COLORREF crBkColor;
  COLORREF crTextColor;

  if ( m_hBkgndBrush != NULL )
  {
    hBkBrush = m_hBkgndBrush;
    crBkColor = m_editorBkgnd;
    crTextColor = m_editorTextNorm;
  }
  else
  {
    hBkBrush = GetSysColorBrush(COLOR_WINDOW);
    crBkColor = GetSysColor(COLOR_WINDOW);
    crTextColor = GetSysColor(COLOR_WINDOWTEXT);
  }

  SetTextColor( (HDC) wParam, crTextColor );
  SetBkMode( (HDC) wParam, TRANSPARENT );
  SetBkColor( (HDC) wParam, crBkColor );
  return (INT_PTR) hBkBrush;
}

bool CDoExecDlg::isScriptFileChanged()
{
  CNppExec& NppExec = Runtime::GetNppExec();
  if ( !NppExec.GetOptions().GetBool(OPTB_WATCHSCRIPTFILE) )
    return false;

  CNppScriptList& ScriptsList = NppExec.m_ScriptsList;
  if ( !NppExecHelpers::CheckFileExists(ScriptsList.GetScriptFileName()) )
  {
    ScriptsList.SetModified(true);
    return true;
  }

  int nFileState = ScriptsList.GetFileState();
  if ( nFileState & CNppScriptList::fsfNeedsReload )
    return true;

  if ( nFileState & CNppScriptList::fsfWasReloaded )
  {
    nFileState ^= CNppScriptList::fsfWasReloaded;
    ScriptsList.SetFileState(nFileState);
    return true;
  }

  return false;
}

void CDoExecDlg::OnInitDialog(HWND hDlg)
{
  bool bScriptFileChanged = isScriptFileChanged();
    
  m_bFirstSetFocus = true;
  
  m_hWnd = hDlg;
  m_edScript.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_CMDLINE);
  m_cbScriptNames.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_SCRIPT);
  m_btOK.m_hWnd = ::GetDlgItem(hDlg, IDOK);
  m_btSave.m_hWnd = ::GetDlgItem(hDlg, IDSAVE);
  m_btCancel.m_hWnd = ::GetDlgItem(hDlg, IDCANCEL);
  m_lbPopupList.Create(m_edScript.m_hWnd); 

// get rid of the "conversion from 'LONG_PTR' to 'LONG'" shit
// get rid of the "conversion from 'LONG' to 'WNDPROC'" shit
// (Microsoft itself does not know what is 'LONG_PTR'?)
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4312)

  OriginalEditProc = (WNDPROC) SetWindowLongPtr(m_edScript.m_hWnd, 
    GWLP_WNDPROC, (LONG_PTR) edScriptWindowProc);

#pragma warning(pop)

  if ( nInitialDlgWidth < 0 )
  {
    RECT rcDlg;
    RECT rcDlgClient;
      
    ::GetWindowRect(hDlg, &rcDlg);
    ::GetClientRect(hDlg, &rcDlgClient);
    nInitialDlgWidth = rcDlg.right - rcDlg.left;
    nInitialDlgHeight = rcDlg.bottom - rcDlg.top;

    ::GetWindowRect( m_edScript.m_hWnd, &rcEdScriptInitial );
    ::ScreenToClient( hDlg, (POINT *) &rcEdScriptInitial.left );
    ::ScreenToClient( hDlg, (POINT *) &rcEdScriptInitial.right );

    ::GetWindowRect( m_cbScriptNames.m_hWnd, &rcCbScriptsInitial );
    ::ScreenToClient( hDlg, (POINT *) &rcCbScriptsInitial.left );
    ::ScreenToClient( hDlg, (POINT *) &rcCbScriptsInitial.right );

    ::GetWindowRect( m_btOK.m_hWnd, &rcBtOkInitial );
    ::ScreenToClient( hDlg, (POINT *) &rcBtOkInitial.left );
    ::ScreenToClient( hDlg, (POINT *) &rcBtOkInitial.right );

    ::GetWindowRect( m_btSave.m_hWnd, &rcBtSaveInitial );
    ::ScreenToClient( hDlg, (POINT *) &rcBtSaveInitial.left );
    ::ScreenToClient( hDlg, (POINT *) &rcBtSaveInitial.right );

    ::GetWindowRect( m_btCancel.m_hWnd, &rcBtCancelInitial );
    ::ScreenToClient( hDlg, (POINT *) &rcBtCancelInitial.left );
    ::ScreenToClient( hDlg, (POINT *) &rcBtCancelInitial.right );
  }
  if ( nCurrentDlgWidth < 0 || nCurrentDlgHeight < 0 )  
  {
    nCurrentDlgWidth = Runtime::GetNppExec().GetOptions().GetInt(OPTI_DOEXECDLG_WIDTH);
    nCurrentDlgHeight = Runtime::GetNppExec().GetOptions().GetInt(OPTI_DOEXECDLG_HEIGHT);
  }
  if ( nCurrentDlgWidth > 0 && nCurrentDlgHeight > 0 )
  {
    ResizeWindow(nCurrentDlgWidth, nCurrentDlgHeight);
  }
  CenterWindow(Runtime::GetNppExec().m_nppData._nppHandle);

  {
    HFONT hEdFont = Runtime::GetNppExec()._execdlgFont;
    if ( hEdFont )
    {
      m_edScript.SendMsg( WM_SETFONT, (WPARAM) hEdFont, 0 );
    }
  }

  if ( Runtime::GetNppExec().GetOptions().GetBool(OPTB_EXECDLG_USEEDITORCOLORS) )
  {
    COLORREF prevBkgnd = m_editorBkgnd;
    m_editorTextNorm = static_cast<COLORREF>(Runtime::GetNppExec().SendNppMsg(NPPM_GETEDITORDEFAULTFOREGROUNDCOLOR));
    m_editorBkgnd = static_cast<COLORREF>(Runtime::GetNppExec().SendNppMsg(NPPM_GETEDITORDEFAULTBACKGROUNDCOLOR));
    if ( m_editorBkgnd != prevBkgnd || m_hBkgndBrush == NULL )
    {
      if ( m_hBkgndBrush != NULL )
      {
        ::DeleteObject(m_hBkgndBrush);
      }
      m_hBkgndBrush = ::CreateSolidBrush(m_editorBkgnd);
    }
  }
  else
  {
    if ( m_hBkgndBrush != NULL )
    {
      ::DeleteObject(m_hBkgndBrush);
    }
    m_hBkgndBrush = NULL;
    m_editorTextNorm = 0xFFFFFFFF;
    m_editorBkgnd = 0xFFFFFFFF;
  }

  m_cbScriptNames.AddString(TEMP_SCRIPT_NAME);
  
  tstr              S;
  CListT<tstr>      ScriptNamesList = Runtime::GetNppExec().m_ScriptsList.GetScriptNames();
  CListItemT<tstr>* p = ScriptNamesList.GetFirst();
  while (p)
  {
    S = p->GetItem();
    m_cbScriptNames.AddString(S.c_str());
    p = p->GetNext();
  }
  
  int ind = Runtime::GetNppExec().GetOptions().GetInt(OPTI_SELECTED_SCRIPT);
  if ( (ind >= 0) && (ind < m_cbScriptNames.GetCount()) )
  {  
    m_cbScriptNames.SetCurSel( ind );
    m_cbScriptNames.GetLBText( ind, g_szPrevScriptName );
  }
  else
  {
    g_szPrevScriptName[0] = 0;
  }

  if (g_szPrevScriptName[0] == 0)
  {
    // initial g_szPrevScriptName value
    lstrcpy(g_szPrevScriptName, TEMP_SCRIPT_NAME);
    m_cbScriptNames.SetCurSel(0);
  }

  /*
  else
  {
    int i;
    
    g_szPrevScriptName[MAX_SCRIPTNAME - 1] = 0; // just in case
    i = findStrInComboBox(g_szPrevScriptName, m_cbScriptNames);
    if (i > 0)
    {
      tmp_scriptname[0] = 0;
      m_cbScriptNames.GetLBText(i, tmp_scriptname);
      lstrcpy(g_szPrevScriptName, tmp_scriptname);
      m_cbScriptNames.SetCurSel(i);
    }
    else
    {
      lstrcpy(g_szPrevScriptName, TEMP_SCRIPT_NAME);
      m_cbScriptNames.SetCurSel(0);
    }
  }
  */

  ShowScriptText(g_szPrevScriptName);
  
  if (bScriptFileChanged)
  {
    S = EXECUTE_DLG_TITLE;
    S += _T(" *");
    this->SetText( S.c_str() );

    if (Runtime::GetNppExec().m_ScriptsList.GetFileState() == CNppScriptList::fsfWasReloaded)
    {
      Runtime::GetNppExec().m_ScriptsList.SetFileState(0);
    }
  }
  else
  {
    this->SetText( EXECUTE_DLG_TITLE );
  }

  m_cbScriptNames.SetFocus();
}

void CDoExecDlg::OnSize()
{
  RECT rcDlg;
    
  ::GetWindowRect(m_hWnd, &rcDlg);
  nCurrentDlgWidth = rcDlg.right - rcDlg.left;
  nCurrentDlgHeight = rcDlg.bottom - rcDlg.top;

  Runtime::GetNppExec().GetOptions().SetInt(OPTI_DOEXECDLG_WIDTH, nCurrentDlgWidth);
  Runtime::GetNppExec().GetOptions().SetInt(OPTI_DOEXECDLG_HEIGHT, nCurrentDlgHeight);

  int dx = nCurrentDlgWidth - nInitialDlgWidth;
  int dy = nCurrentDlgHeight - nInitialDlgHeight;

  m_edScript.MoveWindow(
    rcEdScriptInitial.left, 
    rcEdScriptInitial.top, 
    rcEdScriptInitial.right - rcEdScriptInitial.left + dx,
    rcEdScriptInitial.bottom - rcEdScriptInitial.top + dy,
    TRUE
  );

  m_cbScriptNames.MoveWindow(
    rcCbScriptsInitial.left,
    rcCbScriptsInitial.top + dy,
    rcCbScriptsInitial.right - rcCbScriptsInitial.left,
    rcCbScriptsInitial.bottom - rcCbScriptsInitial.top,
    TRUE
  );
  
  m_btOK.MoveWindow(
    rcBtOkInitial.left + dx,
    rcBtOkInitial.top + dy,
    rcBtOkInitial.right - rcBtOkInitial.left,
    rcBtOkInitial.bottom - rcBtOkInitial.top,
    TRUE
  );
  
  m_btSave.MoveWindow(
    rcBtSaveInitial.left + dx,
    rcBtSaveInitial.top + dy,
    rcBtSaveInitial.right - rcBtSaveInitial.left,
    rcBtSaveInitial.bottom - rcBtSaveInitial.top,
    TRUE
  );
  
  m_btCancel.MoveWindow(
    rcBtCancelInitial.left + dx,
    rcBtCancelInitial.top + dy,
    rcBtCancelInitial.right - rcBtCancelInitial.left,
    rcBtCancelInitial.bottom - rcBtCancelInitial.top,
    TRUE
  );

  m_btOK.Redraw();
  m_btSave.Redraw();
  m_btCancel.Redraw();

  // finally
  if ( DoExecDlg.m_lbPopupList.IsWindowVisible() )
  {
    ::SendMessage(m_edScript.m_hWnd, WM_CHAR, 0, 0);
    DoExecDlg.m_lbPopupList.Redraw();
  }
}

void CDoExecDlg::OnSizing(RECT* lpRect)
{
  if ( lpRect->right - lpRect->left < nInitialDlgWidth )
    lpRect->right = lpRect->left + nInitialDlgWidth;
  if ( lpRect->bottom - lpRect->top < nInitialDlgHeight )
    lpRect->bottom = lpRect->top + nInitialDlgHeight;
}

void CDoExecDlg::ShowScriptText(const tstr& ScriptName)
{
  CNppScript        Script;
  CNppScript*       pScript;
  tstr              S, Line;
  CListItemT<tstr>* p;
  
  if (lstrcmpi(ScriptName.c_str(), TEMP_SCRIPT_NAME) == 0)
  {
    pScript = &Runtime::GetNppExec().m_TempScript;
  }
  else
  {
    // getting our script
    Runtime::GetNppExec().m_ScriptsList.GetScript(ScriptName, Script);
    pScript = &Script;
  }

  // clearing previous script text
  m_edScript.SetWindowText( _T("") );
    
  // setting new script text
  S.Clear();
  S.Reserve(pScript->GetCmdList().GetCount() * 64);
  p = pScript->GetCmdList().GetFirst();
  while (p)
  {
    Line = p->GetItem(); 
    if (p != pScript->GetCmdList().GetLast())
      Line.Append( _T("\r\n"), 2);
    S += Line;
    p = p->GetNext();
  }
  m_edScript.SetWindowText(S.c_str());
  m_edScript.SetModify(FALSE); // script text is not modified
}

/////////////////////////////////////////////////////////////////////////////

bool IsDelimiterCharacter(const TCHAR ch)
{
  switch (ch)
  {
    case _T(' '):
    case _T('\t'):
    case _T(','):
    case _T('.'):
    case _T(';'):
    case _T(':'):
    case _T('/'):
    case _T('\\'):
    case _T('|'):
    case _T('\"'):
    case _T('\r'):
    case _T('\n'):
    case _T('\v'):
    case _T('\f'):
    case 0:
      return true;
  }
  return false;
}

int GetCurrentEditWord(HWND hEd, TCHAR* lpWordBuf, int nWordBufSize)
{
  int          i;
  int          nLine;
  CAnyRichEdit ed;

  ed.m_hWnd = hEd;
  i = ed.GetSelPos();
  nLine = ed.LineFromChar(i);
  if (nLine >= 0)
  {
    TCHAR str[CONSOLECOMMAND_BUFSIZE];

    i = ed.GetLine(nLine, str, CONSOLECOMMAND_BUFSIZE - 1);
    if (i > 0)
    {
      int nWordEnd, nWordStart;

      i = ed.LineIndex(nLine);
      nWordEnd = ed.GetSelPos() - i;
      if ((nWordEnd > 0) && (nWordEnd < CONSOLECOMMAND_BUFSIZE))
      {
        i = nWordEnd - 1;
        while ((i >= 0) && !IsDelimiterCharacter(str[i]))
        {
          if ((str[i] == _T('$')) && (str[i+1] == _T('(')))
          {
            i--;
            break;
          }
          else
            i--;
        }
        nWordStart = i + 1;
        i = nWordEnd;
        while ((i < CONSOLECOMMAND_BUFSIZE) && !IsDelimiterCharacter(str[i]))
        {
          i++;
        }
        nWordEnd = i - 1;
        if (nWordStart <= nWordEnd)
        {
          i = ed.LineIndex(nLine);
          g_nCurrentWordStart = nWordStart + i;
          g_nCurrentWordEnd = nWordEnd + i + 1;
          for (i = 0; (i <= nWordEnd-nWordStart) && (i < nWordBufSize-1); i++)
          {
            lpWordBuf[i] = str[nWordStart + i];
          }
          lpWordBuf[i] = 0;
          return i;
        }
      }
    }
  }
  return -1;
}

LRESULT CALLBACK edScriptWindowProc(
  HWND   hEd, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM lParam)
{
  static DWORD nSelStart = 0;
  static DWORD nSelEnd = 0;
  static TCHAR szLeadingSpaces[CONSOLECOMMAND_BUFSIZE] = { 0 };
  LRESULT lResult;

  if (
      (uMessage == WM_CHAR) ||

      ((uMessage == WM_KEYDOWN) && 
       (wParam == VK_DELETE || wParam == VK_LEFT || wParam == VK_RIGHT || 
        wParam == VK_TAB))
     )
  {
    if (wParam != VK_TAB)
    {
      if (wParam || lParam)
        lResult = CallWindowProc(OriginalEditProc, hEd, uMessage, wParam, lParam);
      else
        lResult = 1;

      if (uMessage == WM_CHAR && wParam == VK_RETURN)
      {
        if (szLeadingSpaces[0])
        {
          SendMessage(hEd, EM_REPLACESEL, (WPARAM) TRUE, (LPARAM) szLeadingSpaces);
          szLeadingSpaces[0] = 0; // final cleanup... just in case :)
        }
      }
      else
      {
        int     i;
        TCHAR   szCurrentWord[200];

        i = GetCurrentEditWord(hEd, szCurrentWord, 200-1);
        if (i > 0)
        {
          DoExecDlg.m_nCurrentWordStart = g_nCurrentWordStart;
          DoExecDlg.m_nCurrentWordEnd = g_nCurrentWordEnd;
          DoExecDlg.m_lbPopupList.Show(szCurrentWord);
        }
        else
        {
          DoExecDlg.m_lbPopupList.ShowWindow(SW_HIDE);
        }
      }
      return lResult;
    }
    else
    {
      DoExecDlg.m_lbPopupList.ShowWindow(SW_HIDE);
    }
  }

  else if (uMessage == WM_KEYDOWN)
  {
    if (wParam == VK_UP || wParam == VK_DOWN)
    {
      if (DoExecDlg.m_lbPopupList.IsWindowVisible())
      {
        DoExecDlg.m_lbPopupList.SetFocus();
        if (DoExecDlg.m_lbPopupList.GetCurSel() < 0)
        {
          if (wParam != VK_UP)
          {
            DoExecDlg.m_lbPopupList.SetCurSel(0);
          }
          else
          {
            int i = DoExecDlg.m_lbPopupList.GetCount() - 1;
            if (i >= 0)
              DoExecDlg.m_lbPopupList.SetCurSel(i);
          }
        }
        return 1;
      }
    }
    else if (wParam != VK_SHIFT)
    {
      DoExecDlg.m_lbPopupList.ShowWindow(SW_HIDE);

      if (wParam == VK_RETURN)
      {
        DWORD dwSelStart = 0, dwSelEnd = 0;
        SendMessage(hEd, EM_GETSEL, (WPARAM) &dwSelStart, (LPARAM) &dwSelEnd);
        INT n = (INT) SendMessage(hEd, EM_LINEFROMCHAR, dwSelStart, 0);
        if (n >= 0) // check if the line number is not negative... well, just in case :)
        {
            *((WORD *) szLeadingSpaces) = CONSOLECOMMAND_BUFSIZE - 1;
            n = (INT) SendMessage(hEd, EM_GETLINE, (WPARAM) n, (LPARAM) szLeadingSpaces);
            // - it is the previous line since this WM_KEYDOWN(VK_RETURN) has not been processed yet
            if (n > 0) // check the line length
            {
                szLeadingSpaces[n] = 0; // 0-terminated string
                n = 0; // now let's check for leading spaces...
                while (NppExecHelpers::IsTabSpaceChar(szLeadingSpaces[n]))  ++n;
            }
        }
        if (n < 0)  n = 0;
        szLeadingSpaces[n] = 0; // empty or contains only leading spaces
        return CallWindowProc(OriginalEditProc, hEd, uMessage, wParam, lParam); // process VK_RETURN
      }
      else if (wParam == 0x41) // 'A'
      {
        if ((GetKeyState(VK_CONTROL) & 0x80) != 0) // Ctrl+A
        {
          SendMessage(hEd, EM_SETSEL, 0, -1); // select all the text
          SendMessage(hEd, EM_GETSEL, (WPARAM) &nSelStart, (LPARAM) &nSelEnd);
          return 1;
        }
      }
    }
  }

  else if (uMessage == WM_LBUTTONDOWN)
  {
  }

  else if (uMessage == WM_SETFOCUS)
  {
    if (DoExecDlg.m_lbPopupList.m_hWnd != (HWND) wParam)
    {
      if (!DoExecDlg.m_bFirstSetFocus)
      {
        //PAINTSTRUCT ps;
        
        lResult = CallWindowProc(OriginalEditProc, 
                    hEd, uMessage, wParam, lParam);
        //BeginPaint(hEd, &ps);
        SendMessage(hEd, EM_SETSEL, 
          (WPARAM) nSelStart, (LPARAM) nSelEnd);
        //EndPaint(hEd, &ps);
        if (DoExecDlg.m_lbPopupList.IsWindowVisible())
        {
          DoExecDlg.m_lbPopupList.ShowWindow(SW_HIDE);
          //DoExecDlg.m_lbPopupList.UpdateWindow();
          //DoExecDlg.m_lbPopupList.SetFocus();
        } 
        return lResult;
      }
      else
      {
        DoExecDlg.m_bFirstSetFocus = false;
      }
    }
  }
  
  else if (uMessage == WM_KILLFOCUS)
  {
    if (DoExecDlg.m_lbPopupList.m_hWnd != (HWND) wParam)
    {
      lResult = CallWindowProc(OriginalEditProc, 
                  hEd, uMessage, wParam, lParam);
      SendMessage(hEd, EM_GETSEL, 
        (WPARAM) &nSelStart, (LPARAM) &nSelEnd);
      return lResult;
    }
  }

  else if (uMessage == WM_CTLCOLORLISTBOX)
  {
    return DoExecDlg.OnCtlColorListBox(wParam, lParam);
  }

  return CallWindowProc(OriginalEditProc, hEd, uMessage, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK ScriptNameDlgProc(
  HWND   hDlg, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM /*lParam*/)
{
  if (uMessage == WM_COMMAND)
  {
    switch (LOWORD(wParam)) 
    {
      case IDOK: // Save
      {
        if (ScriptNameDlg.OnBtSave())
        {
          EndDialog(hDlg, 1); // OK - returns 1
        }
        return 1;
      }
      case IDCANCEL: // Cancel
      {
        EndDialog(hDlg, 0); // Cancel - returns 0
        return 1;
      }
      case IDC_BT_DELETE:
      {
        ScriptNameDlg.OnBtDelete();
        // EndDialog(hDlg, 0);
        return 1;
      }
      default:
        break;
    }
    switch (HIWORD(wParam))
    {
      case CBN_SELCHANGE:
      {
        ScriptNameDlg.OnCbnSelChange();
        return 1;
      }
      case CBN_EDITCHANGE:
      {
        ScriptNameDlg.OnCbnEditChange();
        return 1;
      }
      case CBN_CLOSEUP:
      {
        ScriptNameDlg.OnCbnCloseUp();
        return 1;
      }
      default:
        break;
    }
  }
  else if (uMessage == WM_INITDIALOG)
  {
    ScriptNameDlg.OnInitDialog(hDlg);
  }

  return 0;
}

CScriptNameDlg::CScriptNameDlg() : CAnyWindow()
{
}

CScriptNameDlg::~CScriptNameDlg()
{
}

void CScriptNameDlg::OnBtDelete()
{
  tstr  S;

  tmp_scriptname[0] = 0;
  m_cbScriptName.GetWindowText(tmp_scriptname, MAX_SCRIPTNAME - 1);

  S = _T("Are you sure you want to delete this script:\r\n  \"");
  S += tmp_scriptname;
  S += _T("\" ?");
  if (MessageBox(m_hWnd, S.c_str(), _T("NppExec::DeleteScript Confirmation"), 
        MB_YESNO | MB_ICONQUESTION) == IDYES)
  {
    int i = findStrInComboBox(tmp_scriptname, m_cbScriptName);
    if (i >= 0)
    {
      m_cbScriptName.SetCurSel(-1);
      m_cbScriptName.DeleteString(i);
      m_btDelete.EnableWindow(FALSE);
      
      int k = DoExecDlg.m_cbScriptNames.GetCurSel();
      DoExecDlg.m_cbScriptNames.DeleteString(i + 1); 
        // (i + 1) because 1st item is <temporary script>
      
      if (k > (i + 1))
      {
        //DoExecDlg.m_cbScriptNames.SetCurSel(k - 1);
      }
      else if (k == (i + 1))
      {
        DoExecDlg.m_cbScriptNames.SetCurSel(0); // <temporary script>
        DoExecDlg.ShowScriptText(TEMP_SCRIPT_NAME);
      }

    }
    SetFocus();
    m_cbScriptName.SetFocus();
    Runtime::GetNppExec().m_ScriptsList.DeleteScript(tmp_scriptname);
    DoExecDlg.m_szScriptNameToSave[0] = 0; 
      // we don't need this item in DoExecDlg
  }

}

bool CScriptNameDlg::OnBtSave()
{
  // m_szScriptNameToSave is checked after ScriptNameDlg is closed
  if (m_cbScriptName.GetWindowText(DoExecDlg.m_szScriptNameToSave, MAX_SCRIPTNAME - 1) == 0)
  {
    MessageBox(m_hWnd, _T("Script name is empty"), 
      _T("NppExec::SaveScript Warning"), MB_OK | MB_ICONWARNING);
    SetFocus();
    m_cbScriptName.SetFocus();
    return false;
  }

  if ( NppExecHelpers::IsAnySpaceChar(DoExecDlg.m_szScriptNameToSave[0]) )
  {
    DoExecDlg.m_szScriptNameToSave[0] = 0;
    MessageBox(m_hWnd, _T("Script name can\'t contain leading space characters"), 
      _T("NppExec::SaveScript Warning"), MB_OK | MB_ICONWARNING);
    SetFocus();
    m_cbScriptName.SetFocus();
    return false;
  }

  if (lstrcmpi(DoExecDlg.m_szScriptNameToSave, TEMP_SCRIPT_NAME) == 0)
  {
    tstr S = TEMP_SCRIPT_NAME;
    S += _T(" is a reserved name");
    MessageBox(m_hWnd, S.c_str(), _T("NppExec::SaveScript Warning"), MB_OK | MB_ICONWARNING);
    SetFocus();
    m_cbScriptName.SetFocus();
    return false;
  }
  else
  {
    CNppScript Script;
    TCHAR      szLine[CONSOLECOMMAND_BUFSIZE];

    int lines = DoExecDlg.m_edScript.GetLineCount();
    for (int i = 0; i < lines; i++)
    {
      DoExecDlg.m_edScript.GetLine(i, szLine, CONSOLECOMMAND_BUFSIZE - 1);
      Script.GetCmdList().Add(szLine);
    }
    Runtime::GetNppExec().m_ScriptsList.AddScript(DoExecDlg.m_szScriptNameToSave, Script);

    // no need to modify m_cbScriptName because this dialog will be closed now
    
    // saving modifications in scripts
    Runtime::GetNppExec().SaveScripts(CNppExec::ssfSaveLastScript);
    return true;
  }
}

void CScriptNameDlg::OnCbnCloseUp()
{
  //MessageBox(NULL, _T("CBN_CLOSEUP"), _T("CScriptNameDlg"), MB_OK); // OK
  OnCbnEditChange();
}

void CScriptNameDlg::OnCbnEditChange()
{
  int  i;

  tmp_scriptname[0] = 0;
  m_cbScriptName.GetWindowText(tmp_scriptname, MAX_SCRIPTNAME - 1);
  i = findStrInComboBox(tmp_scriptname, m_cbScriptName);
  //MessageBox(NULL, (i >= 0) ? "TRUE" : "FALSE", "Enable", MB_OK);
  m_btDelete.EnableWindow( (i >= 0) ? TRUE : FALSE );
}

void CScriptNameDlg::OnCbnSelChange()
{
  int i = m_cbScriptName.GetCurSel();
  
  m_btDelete.EnableWindow( (i >= 0) && (i < m_cbScriptName.GetCount()) ? TRUE : FALSE );
}

void CScriptNameDlg::OnInitDialog(HWND hDlg)
{
  RECT     rect, rectParent;

  m_hWnd = hDlg;
  m_cbScriptName.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_SCRIPTNAME);
  m_btSave.m_hWnd = ::GetDlgItem(hDlg, IDOK);
  m_btDelete.m_hWnd = ::GetDlgItem(hDlg, IDC_BT_DELETE);
  m_btCancel.m_hWnd = ::GetDlgItem(hDlg, IDCANCEL);
  
  ::GetWindowRect(hDlg, &rect);
  ::GetClientRect(DoExecDlg.m_hWnd, &rectParent);
  MoveWindow(rect.left + 6, rect.top + 
    (rectParent.bottom - rectParent.top) - (rect.bottom - rect.top) - 5);
      
  int count = DoExecDlg.m_cbScriptNames.GetCount();
  for (int i = 0; i < count; i++)
  {
    tmp_scriptname[0] = 0;
    DoExecDlg.m_cbScriptNames.GetLBText(i, tmp_scriptname);
    if ( lstrcmpi(tmp_scriptname, TEMP_SCRIPT_NAME) != 0 )
    {
      // not TEMP_SCRIPT_NAME
      m_cbScriptName.AddString(tmp_scriptname);
    }
  }
    
  int len = lstrlen(DoExecDlg.m_szScriptNameToSave);
  if (len > 0)
  {
    int i = findStrInComboBox(DoExecDlg.m_szScriptNameToSave, m_cbScriptName);
    if (i < 0)
    {
      i = m_cbScriptName.AddString(DoExecDlg.m_szScriptNameToSave);
    }
    m_cbScriptName.SetCurSel(i);
  }
  else if (m_cbScriptName.GetCount() == 0)
  {
    m_cbScriptName.AddString( _T("") );
  }
   
  m_btDelete.EnableWindow( (len > 0) ? TRUE : FALSE );
  
  //SetWindowText(hEd, g_szScriptName);
  //SendMessage(hEd, EM_SETSEL, (WPARAM) len, (LPARAM) len);
  m_cbScriptName.SetFocus();
  DoExecDlg.m_szScriptNameToSave[0] = 0;
}

/////////////////////////////////////////////////////////////////////////////

int findStrInComboBox(const TCHAR* str, CAnyComboBox& cb)
{
  int len = (str != NULL) ? lstrlen(str) : 0;
  if (len > 0)
  { 
    TCHAR item_text[MAX_SCRIPTNAME];
 
    int nIndex = -1;
    int count = cb.GetCount();
    for (int i = 0; i < count; i++)
    {
      if (cb.GetLBText(i, item_text) > 0)
      {
        if (lstrcmpi(str, item_text) == 0)
        {
          nIndex = i;
          break;
        }
      }
    }

    return nIndex;
  }
  return -1;
}

