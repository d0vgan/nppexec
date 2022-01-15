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

#include "DlgAdvancedOptions.h"
#include "CAnyWindow.h"
#include "NppExec.h"
#include "DlgDoExec.h"
#include "DlgConsole.h"
#include "c_base/HexStr.h"
#include "c_base/int2str.h"
#include "c_base/str2int.h"
#include "c_base/str_func.h"

const TCHAR* cszUserMenuItemSep = _T(" :: ");
const TCHAR* cszUserMenuSeparator = _T("---- <menu separator> ----");

extern COLORREF g_colorTextNorm;
extern COLORREF g_colorTextErr;
extern COLORREF g_colorTextMsg;
extern COLORREF g_colorBkgnd;

typedef const c_base::byte_t * lpcbyte_t;

CAdvOptDlg advOptDlg;

INT_PTR CALLBACK AdvancedOptionsDlgProc(
  HWND   hDlg, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM /*lParam*/)
{
    if ( uMessage == WM_COMMAND )
    {
        switch ( LOWORD(wParam) ) 
        {
            case IDC_LB_MENUITEMS: 
            {
                if ( HIWORD(wParam) == LBN_SELCHANGE )
                {
                    advOptDlg.OnLbMenuItemsSelChange();
                }
                break;
            }
            case IDC_ED_ITEMNAME:
            {
                if ( HIWORD(wParam) == EN_CHANGE )
                {
                    advOptDlg.OnEdItemNameChange();
                }
                break;
            }
            case IDC_CB_ITEMSCRIPT:
            {
                if ( HIWORD(wParam) == CBN_SELCHANGE )
                {
                    advOptDlg.OnCbItemScriptSelChange();
                }
                break;
            }
            case IDC_CB_OPT_HOTKEY:
            {
                if ( HIWORD(wParam) == CBN_SELCHANGE )
                {
                    advOptDlg.OnCbOptHotkeySelChange();
                }
                break;
            }
            case IDC_CB_OPT_TOOLBARBTN:
            {
                if ( HIWORD(wParam) == CBN_SELCHANGE )
                {
                    advOptDlg.OnCbOptToolbarBtnSelChange();
                }
                break;
            }
            case IDC_BT_ITEMNEW:
            {
                advOptDlg.OnBtItemNew();
                break;
            }
            case IDC_BT_ITEMDELETE:
            {
                advOptDlg.OnBtItemDelete();
                break;
            }
            case IDC_BT_MOVEUP:
            {
                advOptDlg.OnBtMoveUp();
                break;
            }
            case IDC_BT_MOVEDOWN:
            {
                advOptDlg.OnBtMoveDown();
                break;
            }
            case IDC_CH_MACROS_SUBMENU:
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    advOptDlg.OnChMacrosSubmenu();
                }
                break;
            }
            case IDOK:
            {
                if ( advOptDlg.OnBtOK() )
                {
                    EndDialog(hDlg, 1);
                }
                return 1;
            }
            case IDCANCEL:
            {
                advOptDlg.OnBtCancel();
                EndDialog(hDlg, 0);
                return 1;
            }
            default:
                break;
        }
    }

    else if ( uMessage == WM_SYSCOMMAND )
    {
        if ( wParam == SC_CLOSE )
        {
            advOptDlg.OnBtCancel();
            EndDialog(hDlg, 0);
            return 1;
        }
    }

    else if ( uMessage == WM_INITDIALOG )
    {
        advOptDlg.OnInitDlg(hDlg);
    }

    return 0;
}

CAdvOptDlg::CAdvOptDlg() : CAnyWindow()
{
}

CAdvOptDlg::~CAdvOptDlg()
{
}

void CAdvOptDlg::OnInitDlg(HWND hDlg)
{
    CNppExec& NppExec = Runtime::GetNppExec();

    m_hWnd = hDlg;

    m_chMacrosSubmenu.m_hWnd = ::GetDlgItem(hDlg, IDC_CH_MACROS_SUBMENU);
    m_lbMenuItems.m_hWnd = ::GetDlgItem(hDlg, IDC_LB_MENUITEMS);
    m_cbItemScript.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_ITEMSCRIPT);
    m_edItemName.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_ITEMNAME);
    m_cbScriptNppStart.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_SCRIPTNPPSTART);
    m_cbScriptNppExit.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_SCRIPTNPPEXIT);
    m_cbHotKey.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_HOTKEY);
    m_cbToolbarBtn.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_TOOLBARBTN);
    m_cbConVisible.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_CONVISIBLE);
    m_cbConShowHelp.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_SHOWHELP);
    m_cbSaveCmdHst.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_OPT_SAVECMDHST);
    m_edCommentDelim.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_OPT_COMMENTDELIM);
    m_edTextColorNorm.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_OPT_TEXTCOLORNORM);
    m_edTextColorErr.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_OPT_TEXTCOLORERR);
    m_edTextColorMsg.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_OPT_TEXTCOLORMSG);
    m_edBkColor.m_hWnd = ::GetDlgItem(hDlg, IDC_ED_OPT_BKCOLOR);
    m_btMoveUp.m_hWnd = ::GetDlgItem(hDlg, IDC_BT_MOVEUP);
    m_btMoveDown.m_hWnd = ::GetDlgItem(hDlg, IDC_BT_MOVEDOWN);
    m_btModify.m_hWnd = ::GetDlgItem(hDlg, IDC_BT_ITEMNEW);
    m_btDelete.m_hWnd = ::GetDlgItem(hDlg, IDC_BT_ITEMDELETE);

    m_edItemName.SendMsg(EM_LIMITTEXT, MAX_SCRIPTNAME/2, 0);
    m_edCommentDelim.SendMsg(EM_LIMITTEXT, 9, 0);
    m_edTextColorNorm.SendMsg(EM_LIMITTEXT, 9, 0);
    m_edTextColorErr.SendMsg(EM_LIMITTEXT, 9, 0);
    m_edTextColorMsg.SendMsg(EM_LIMITTEXT, 9, 0);
    m_edBkColor.SendMsg(EM_LIMITTEXT, 9, 0);
    
    // notepad++ macros submenu...
    
    m_chMacrosSubmenu.SetCheck( NppExec.GetOptions().GetBool(OPTB_USERMENU_NPPMACROS) ? TRUE : FALSE );
    
    int     i;
    LPCTSTR psz;
    
    // fill menu items list-box...

    for ( i = 0; i < MAX_USERMENU_ITEMS; i++ )
    {
        psz = NppExec.GetOptions().GetStr(OPTS_USERMENU_ITEM01 + i);
        if ( psz && psz[0] )
        {
            m_lbMenuItems.AddString(psz);
        }
    }
    
    // fill script names combo-boxes...

    m_cbItemScript.AddString( _T("") );
    m_cbItemScript.AddString( cszUserMenuSeparator );
    m_cbScriptNppStart.AddString( _T("") );
    m_cbScriptNppExit.AddString( _T("") );
    
    tstr              S;
    CListT<tstr>      ScriptNamesList = NppExec.m_ScriptsList.GetScriptNames();
    CListItemT<tstr>* p = ScriptNamesList.GetFirst();
    while (p)
    {
        S = p->GetItem();
        m_cbItemScript.AddString( S.c_str() );
        m_cbScriptNppStart.AddString( S.c_str() );
        m_cbScriptNppExit.AddString( S.c_str() );
        p = p->GetNext();
    }

    i = 0;
    psz = NppExec.GetOptions().GetStr(OPTS_SCRIPT_NPPSTART);
    if ( psz )
    {
        i = m_cbScriptNppStart.FindStringExact( psz );
        if ( i < 0 )  i = 0;
    }
    m_cbScriptNppStart.SetCurSel(i);

    i = 0;
    psz = NppExec.GetOptions().GetStr(OPTS_SCRIPT_NPPEXIT);
    if ( psz )
    {
        i = m_cbScriptNppExit.FindStringExact( psz );
        if ( i < 0 )  i = 0;
    }
    m_cbScriptNppExit.SetCurSel(i);


    // fill hotkey combo-box...

    m_cbHotKey.AddString( _T("F1") );
    m_cbHotKey.AddString( _T("F2") );
    m_cbHotKey.AddString( _T("F3") );
    m_cbHotKey.AddString( _T("F4") );
    m_cbHotKey.AddString( _T("F5") );
    m_cbHotKey.AddString( _T("F6") );
    m_cbHotKey.AddString( _T("F7") );
    m_cbHotKey.AddString( _T("F8") );
    m_cbHotKey.AddString( _T("F9") );
    m_cbHotKey.AddString( _T("F10") );
    m_cbHotKey.AddString( _T("F11") );
    m_cbHotKey.AddString( _T("F12") );
    unsigned int uHotKey = NppExec.GetOptions().GetUint(OPTU_PLUGIN_HOTKEY);
    for ( i = 0; i < 12; i++ )
    {
        if ( uHotKey == static_cast<unsigned int>(VK_F1 + i) )
        {
            m_cbHotKey.SetCurSel(i);
            break;
        }
    }

    // fill toolbar btn combo-box...

    m_cbToolbarBtn.AddString( _T("none") );
    m_cbToolbarBtn.AddString( _T("Console") );
    m_cbToolbarBtn.AddString( _T("Execute") );
    m_cbToolbarBtn.AddString( _T("ExecPrev") );
    m_cbToolbarBtn.AddString( _T("ExecSel") );
    i = NppExec.GetOptions().GetInt(OPTI_TOOLBARBTN);
    if ( i < 0 || i > 4 )  i = 0;
    m_cbToolbarBtn.SetCurSel(i);
    
    // fill console visible combo-box...
    
    m_cbConVisible.AddString( _T("Auto") );
    m_cbConVisible.AddString( _T("Yes") );
    m_cbConVisible.AddString( _T("No") );
    i = NppExec.GetOptions().GetInt(OPTI_CONSOLE_VISIBLE);
    if ( i < CON_AUTO || i > CON_NO )  i = CON_AUTO;
    m_cbConVisible.SetCurSel(i);

    // fill console show help combo-box...
    
    m_cbConShowHelp.AddString( _T("No") );
    m_cbConShowHelp.AddString( _T("Yes") );
    i = NppExec.GetOptions().GetBool(OPTB_CONSOLE_SHOWHELP) ? 1 : 0;
    m_cbConShowHelp.SetCurSel(i);

    // fill save cmd history combo-box...
    
    m_cbSaveCmdHst.AddString( _T("No") );
    m_cbSaveCmdHst.AddString( _T("Yes") );
    i = NppExec.GetOptions().GetBool(OPTB_CONSOLE_SAVECMDHISTORY) ? 1 : 0;
    m_cbSaveCmdHst.SetCurSel(i);
    
    // fill edit-controls...

    TCHAR szText[64];

    S = NppExec.GetOptions().GetStr(OPTS_COMMENTDELIMITER);
    m_edCommentDelim.SetWindowText(S.c_str());
    
    szText[0] = 0;
    c_base::_tbuf2hexstr( (lpcbyte_t) &g_colorTextNorm, 3, szText, 64 - 1, _T(" ") );
    m_edTextColorNorm.SetWindowText(szText);
    
    szText[0] = 0;
    c_base::_tbuf2hexstr( (lpcbyte_t) &g_colorTextErr, 3, szText, 64 - 1, _T(" ") );
    m_edTextColorErr.SetWindowText(szText);
    
    szText[0] = 0;
    c_base::_tbuf2hexstr( (lpcbyte_t) &g_colorTextMsg, 3, szText, 64 - 1, _T(" ") );
    m_edTextColorMsg.SetWindowText(szText);
    
    if ( g_colorBkgnd == 0xFFFFFFFF )
    {
        m_edBkColor.SetWindowText( _T("0") );
    }
    else
    {
        szText[0] = 0;
        c_base::_tbuf2hexstr( (lpcbyte_t) &g_colorBkgnd, 3, szText, 64 - 1, _T(" ") );
        m_edBkColor.SetWindowText(szText);
    }
    
    // buttons...

    if ( m_lbMenuItems.GetCount() > 1 )
    {
        m_btMoveUp.EnableWindow(TRUE);
        m_btMoveDown.EnableWindow(TRUE);
    }
    else
    {
        m_btMoveUp.EnableWindow(FALSE);
        m_btMoveDown.EnableWindow(FALSE);
    }
    m_btModify.EnableWindow(FALSE);
    m_btDelete.EnableWindow(FALSE);
    
    // finally...

    m_nREMaxLen = NppExec.GetOptions().GetInt(OPTI_RICHEDIT_MAXTEXTLEN);
    colorValuesInit();

    // some options do not require n++ to be restarted
    m_bNppRestartRequired = false;
    
    advOptDlg.CenterWindow(NppExec.m_nppData._nppHandle);
}

BOOL CAdvOptDlg::OnBtOK()
{
    CNppExec& NppExec = Runtime::GetNppExec();
    c_base::byte_t bt[4];
    TCHAR szText[MAX_SCRIPTNAME];
    int   i;
    
    // text/background colours...
    
    bt[0] = 0;
    szText[0] = 0;
    m_edTextColorNorm.GetWindowText(szText, MAX_SCRIPTNAME - 1);
    i = c_base::_thexstr2buf( szText, bt, 4 );
    if ( i >= 3 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_TEXTNORM, bt, 3);
        g_colorTextNorm = RGB( bt[0], bt[1], bt[2] );
    }
    else if ( i <= 1 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_TEXTNORM, bt, 1);
        g_colorTextNorm = COLOR_CON_TEXTNORM;
    }
    else
    {
        ShowError( _T("Incorrect value of TextColorNormal") );
        return FALSE;
    }
    
    bt[0] = 0;
    szText[0] = 0;
    m_edTextColorErr.GetWindowText(szText, MAX_SCRIPTNAME - 1);
    i = c_base::_thexstr2buf( szText, bt, 4 );
    if ( i >= 3 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_TEXTERR, bt, 3);
        g_colorTextErr = RGB( bt[0], bt[1], bt[2] );
    }
    else if ( i <= 1 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_TEXTERR, bt, 1);
        g_colorTextErr = COLOR_CON_TEXTERR;
    }
    else
    {
        ShowError( _T("Incorrect value of TextColorError") );
        return FALSE;
    }
    
    bt[0] = 0;
    szText[0] = 0;
    m_edTextColorMsg.GetWindowText(szText, MAX_SCRIPTNAME - 1);
    i = c_base::_thexstr2buf( szText, bt, 4 );
    if ( i >= 3 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_TEXTMSG, bt, 3);
        g_colorTextMsg = RGB( bt[0], bt[1], bt[2] );
    }
    else if ( i <= 1 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_TEXTMSG, bt, 1);
        g_colorTextMsg = COLOR_CON_TEXTMSG;
    }
    else
    {
        ShowError( _T("Incorrect value of TextColorMessage") );
        return FALSE;
    }
    
    bt[0] = 0;
    szText[0] = 0;
    m_edBkColor.GetWindowText(szText, MAX_SCRIPTNAME - 1);
    i = c_base::_thexstr2buf( szText, bt, 4 );
    if ( i >= 3 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_BKGND, bt, 3);
        g_colorBkgnd = RGB( bt[0], bt[1], bt[2] );
    }
    else if ( i <= 1 )
    {
        NppExec.GetOptions().SetData(OPTD_COLOR_BKGND, bt, 1);
        g_colorBkgnd = COLOR_CON_BKGND;
    }
    else
    {
        ShowError( _T("Incorrect value of BackgroundColor") );
        return FALSE;
    }
    
    // notepad++ macros submenu...
    
    NppExec.GetOptions().SetBool(OPTB_USERMENU_NPPMACROS, m_chMacrosSubmenu.IsChecked() ? true : false );
    
    // user menu items...
    
    i = m_lbMenuItems.GetCount();
    if ( i > MAX_USERMENU_ITEMS )  i = MAX_USERMENU_ITEMS;
    for ( int j = 0; j < i; j++ )
    {
        szText[0] = 0;
        m_lbMenuItems.GetString(j, szText);
        NppExec.GetOptions().SetStr( OPTS_USERMENU_ITEM01 + j, szText );
    }
    while ( i < MAX_USERMENU_ITEMS )
    {
        NppExec.GetOptions().SetStr( OPTS_USERMENU_ITEM01 + i, _T("") );
        ++i;
    }
    
    // npp start/exit scripts...

    i = m_cbScriptNppStart.GetCurSel();
    if ( i >= 0 )
    {
        szText[0] = 0;
        m_cbScriptNppStart.GetLBText(i, szText);
        NppExec.GetOptions().SetStr(OPTS_SCRIPT_NPPSTART, szText);
    }

    i = m_cbScriptNppExit.GetCurSel();
    if ( i >= 0 )
    {
        szText[0] = 0;
        m_cbScriptNppExit.GetLBText(i, szText);
        NppExec.GetOptions().SetStr(OPTS_SCRIPT_NPPEXIT, szText);
    }
    
    // plugin hot-key...
    
    i = m_cbHotKey.GetCurSel();
    if ( i >= 0 )
    {
        NppExec.GetOptions().SetUint(OPTU_PLUGIN_HOTKEY, VK_F1 + i);
    }

    // toolbar btn...

    i = m_cbToolbarBtn.GetCurSel();
    if ( i >= 0 )
    {
        NppExec.GetOptions().SetInt(OPTI_TOOLBARBTN, i);
    }
    
    // console visible...
    
    i = m_cbConVisible.GetCurSel();
    if ( i >= 0 )
    {
        NppExec.GetOptions().SetInt(OPTI_CONSOLE_VISIBLE, i);
    }
    
    // console show help...
    
    i = m_cbConShowHelp.GetCurSel();
    if ( i >= 0 )
    {
        NppExec.GetOptions().SetBool(OPTB_CONSOLE_SHOWHELP, i ? true : false);
    }

    // save cmd history...
    i = m_cbSaveCmdHst.GetCurSel();
    if ( i >= 0 )
    {
        NppExec.GetOptions().SetBool(OPTB_CONSOLE_SAVECMDHISTORY, i ? true : false);
    }
    
    // comment delimiter...
    
    szText[0] = 0;
    m_edCommentDelim.GetWindowText(szText, MAX_SCRIPTNAME - 1);
    NppExec.GetOptions().SetStr(OPTS_COMMENTDELIMITER, szText);
    
    // applying options...
    
    if ( colorValuesChanged() )
    {
        NppExec.GetConsole().UpdateColours();
    }
    
    i = NppExec.GetOptions().GetInt(OPTI_RICHEDIT_MAXTEXTLEN);
    if ( m_nREMaxLen != i )
    {
        NppExec.GetConsole().GetConsoleEdit().ExLimitText(i);
    }
    
    NppExec.SaveOptions();

    if ( m_bNppRestartRequired )
    {
        ShowWarning( _T("Notepad++ must be restarted to apply some of the options") );
    }

    return TRUE;
}

void CAdvOptDlg::OnBtCancel()
{
}

void CAdvOptDlg::OnBtItemNew()
{
    TCHAR szItemName[MAX_SCRIPTNAME];
    TCHAR szItemScript[MAX_SCRIPTNAME];
    int   i;

    szItemName[0] = 0;
    if ( ::IsWindowEnabled(m_edItemName.m_hWnd) )
    {
        m_edItemName.GetWindowText(szItemName, MAX_SCRIPTNAME - 1);
        lstrcat(szItemName, cszUserMenuItemSep); // adding trailing cszItemSep
    }

    szItemScript[0] = 0;
    i = m_cbItemScript.GetCurSel();
    if ( i >= 0 )
        m_cbItemScript.GetLBText(i, szItemScript);
    
    if ( szItemName[0] )
        i = m_lbMenuItems.FindString(szItemName);
    else
        i = -1;
    if ( i < 0 )
    {
        // adding new menu item
        if ( m_lbMenuItems.GetCount() >= MAX_USERMENU_ITEMS )
        {
            ShowError( _T("Can not add more menu items") );
            return;
        }

        lstrcat( szItemName, szItemScript );
        i = m_lbMenuItems.AddString(szItemName);
        if ( i != LB_ERR )
        {
            m_lbMenuItems.SetCurSel(i);
            m_btDelete.EnableWindow(TRUE);
            if ( lstrcmp(szItemName, cszUserMenuSeparator) == 0 )
            {
                m_edItemName.SetText(cszUserMenuSeparator);
                m_cbItemScript.SetCurSel(0);
                m_btModify.EnableWindow(FALSE);
                m_btDelete.EnableWindow(TRUE);
            }
        }
    }
    else
    {
        // modifying existing menu item
        lstrcat( szItemName, szItemScript );
        m_lbMenuItems.DeleteString(i);
        m_lbMenuItems.InsertString(i, szItemName);
        m_lbMenuItems.SetCurSel(i);
        m_btDelete.EnableWindow(TRUE);
    }

    if ( m_lbMenuItems.GetCount() > 1 )
    {
        m_btMoveUp.EnableWindow(TRUE);
        m_btMoveDown.EnableWindow(TRUE);
    }
    else
    {
        m_btMoveUp.EnableWindow(FALSE);
        m_btMoveDown.EnableWindow(FALSE);
    }

    // menu item added/modified
    m_bNppRestartRequired = true;
}

void CAdvOptDlg::OnBtItemDelete()
{
    int i_lb = m_lbMenuItems.GetCurSel();
    if ( i_lb >= 0 )
    {
        m_lbMenuItems.DeleteString(i_lb);
        m_btDelete.EnableWindow(FALSE);

        // menu item deleted
        m_bNppRestartRequired = true;
    }

    if ( m_lbMenuItems.GetCount() > 1 )
    {
        m_btMoveUp.EnableWindow(TRUE);
        m_btMoveDown.EnableWindow(TRUE);
    }
    else
    {
        m_btMoveUp.EnableWindow(FALSE);
        m_btMoveDown.EnableWindow(FALSE);
    }
}

void CAdvOptDlg::OnBtMoveUp()
{
    int i_lb = m_lbMenuItems.GetCurSel();
    if ( i_lb > 0 ) // moving up
    {
        TCHAR szText[MAX_SCRIPTNAME];

        szText[0] = 0;
        m_lbMenuItems.GetString(i_lb, szText);
        m_lbMenuItems.DeleteString(i_lb);
        m_lbMenuItems.InsertString(i_lb - 1, szText);
        m_lbMenuItems.SetCurSel(i_lb - 1);

        // menu item moved up
        m_bNppRestartRequired = true;
    }
}

void CAdvOptDlg::OnBtMoveDown()
{
    int i_lb = m_lbMenuItems.GetCurSel();
    if ( (i_lb >= 0) && (i_lb < m_lbMenuItems.GetCount() - 1) ) // moving down
    {
        TCHAR   szText[MAX_SCRIPTNAME];

        szText[0] = 0;
        m_lbMenuItems.GetString(i_lb, szText);
        m_lbMenuItems.DeleteString(i_lb);
        if ( i_lb < m_lbMenuItems.GetCount() - 1 )
            m_lbMenuItems.InsertString(i_lb + 1, szText);
        else
            m_lbMenuItems.AddString(szText);
        m_lbMenuItems.SetCurSel(i_lb + 1);

        // menu item moved down
        m_bNppRestartRequired = true;
    }
}

void CAdvOptDlg::OnCbItemScriptSelChange()
{
    int i_cb = m_cbItemScript.GetCurSel();
    if ( i_cb >= 0 )
    {
        TCHAR szScriptName[MAX_SCRIPTNAME];

        szScriptName[0] = 0;
        m_edItemName.GetWindowText(szScriptName, MAX_SCRIPTNAME - 1);
        bool isNoName = ( (szScriptName[0] == 0) || 
                          (lstrcmp(szScriptName, cszUserMenuSeparator) == 0) );

        szScriptName[0] = 0;
        m_cbItemScript.GetLBText(i_cb, szScriptName);
        bool isSep = (lstrcmp(szScriptName, cszUserMenuSeparator) == 0);

        m_edItemName.EnableWindow( !isSep );
        if ( isNoName )
            m_edItemName.SetWindowText(szScriptName);

        if ( (m_cbItemScript.GetLBTextLength(i_cb) > 0) &&
             (isSep || (m_edItemName.GetTextLength() > 0)) )
            m_btModify.EnableWindow(TRUE);
        else
            m_btModify.EnableWindow(FALSE);
    }
}

void CAdvOptDlg::OnCbOptHotkeySelChange()
{
    m_bNppRestartRequired = true;
}

void CAdvOptDlg::OnCbOptToolbarBtnSelChange()
{
    m_bNppRestartRequired = true;
}

void CAdvOptDlg::OnChMacrosSubmenu()
{
    m_bNppRestartRequired = true;
}

void CAdvOptDlg::OnEdItemNameChange()
{
    if ( m_edItemName.GetTextLength() > 0 )
    {
        bool  bReturn = false;
        TCHAR szItemName[MAX_SCRIPTNAME];

        szItemName[0] = 0;
        m_edItemName.GetWindowText(szItemName, MAX_SCRIPTNAME - 1);
        lstrcat(szItemName, cszUserMenuItemSep);
        int i_lb = m_lbMenuItems.GetCurSel();
        if ( (i_lb >= 0) && 
             (i_lb == m_lbMenuItems.FindString(szItemName)) )
        {
            m_btDelete.EnableWindow(TRUE);
            bReturn = true;
        }
        else
        {
            m_btDelete.EnableWindow(FALSE);
        }
        
        int i_cb = m_cbItemScript.GetCurSel();
        if ( i_cb >= 0 )
        {
            if ( m_cbItemScript.GetLBTextLength(i_cb) > 0 )
            {
                m_btModify.EnableWindow(TRUE);
                bReturn = true;
            }
        }

        if ( bReturn )
            return;
    }
    m_btModify.EnableWindow(FALSE);
    m_btDelete.EnableWindow(FALSE);
}

void CAdvOptDlg::OnLbMenuItemsSelChange()
{
    int i_lb = m_lbMenuItems.GetCurSel();
    if ( i_lb >= 0 )
    {
        TCHAR  szItemName[MAX_SCRIPTNAME];
        TCHAR  szItemScript[MAX_SCRIPTNAME];
        TCHAR* p;
        int    i;

        szItemName[0] = 0;
        m_lbMenuItems.GetString(i_lb, szItemName);
        i = lstrlen(szItemName);
        i = c_base::_tstr_unsafe_rfind(szItemName, i, cszUserMenuItemSep);
        if ( i >= 0 )
        {
            p = c_base::_tstr_unsafe_rskip_tabspaces(szItemName, i);
            *(++p) = 0; // cut at cszItemSep position
            i += lstrlen(cszUserMenuItemSep); // postion after cszItemSep
            p = c_base::_tstr_unsafe_skip_tabspaces(szItemName + i);
            lstrcpy(szItemScript, p);
        }
        else
        {
            szItemScript[0] = 0; // no script name
        }

        if ( (i < 0) && (lstrcmp(szItemName, cszUserMenuSeparator) == 0) )
        {
            m_edItemName.EnableWindow(FALSE);
            //m_cbItemScript.EnableWindow(FALSE);
        }
        else
        {
            m_edItemName.EnableWindow(TRUE);
            //m_cbItemScript.EnableWindow(TRUE);
        }
        m_edItemName.SetWindowText(szItemName);
        i = m_cbItemScript.FindStringExact(szItemScript);
        if ( i < 0 )  i = 0;
        m_cbItemScript.SetCurSel(i);

        OnEdItemNameChange();
        m_btDelete.EnableWindow(TRUE);
        return;
    }
    m_btDelete.EnableWindow(FALSE);
}

void CAdvOptDlg::ShowError(LPCTSTR szMessage)
{
  ::MessageBox(m_hWnd, szMessage, 
      _T("NppExec Advanced Options"), MB_OK | MB_ICONERROR);
}

void CAdvOptDlg::ShowWarning(LPCTSTR szMessage)
{
  ::MessageBox(m_hWnd, szMessage, 
      _T("NppExec Advanced Options"), MB_OK | MB_ICONWARNING);
}

void CAdvOptDlg::colorValuesInit()
{
    m_bufColorTextNorm.Assign( (lpcbyte_t) &g_colorTextNorm, sizeof(COLORREF) );
    m_bufColorTextErr.Assign( (lpcbyte_t) &g_colorTextErr, sizeof(COLORREF) );
    m_bufColorTextMsg.Assign( (lpcbyte_t) &g_colorTextMsg, sizeof(COLORREF) );
    m_bufColorBkgnd.Assign( (lpcbyte_t) &g_colorBkgnd, sizeof(COLORREF) );
}

BOOL CAdvOptDlg::colorValuesChanged()
{
    if ( m_bufColorTextNorm.Compare((lpcbyte_t) &g_colorTextNorm, sizeof(COLORREF)) != 0 )
        return TRUE;

    if ( m_bufColorTextErr.Compare((lpcbyte_t) &g_colorTextErr, sizeof(COLORREF)) != 0 )
        return TRUE;

    if ( m_bufColorTextMsg.Compare((lpcbyte_t) &g_colorTextMsg, sizeof(COLORREF)) != 0 )
        return TRUE;

    if ( m_bufColorBkgnd.Compare((lpcbyte_t) &g_colorBkgnd, sizeof(COLORREF)) != 0 )
        return TRUE;
    
    return FALSE;
}
