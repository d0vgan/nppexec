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

#include "DlgInputBox.h"
#include "cpp/StrSplitT.h"
#include "c_base/str2int.h"


#ifdef UNICODE
  #define _t_sprintf  swprintf
#else
  #define _t_sprintf  sprintf
#endif


#define ID_TIMER_TIMEOUT 1001


CInputBoxDlg InputBoxDlg;

extern const TCHAR MACRO_INPUT[];


INT_PTR CALLBACK InputBoxDlgProc(
  HWND   hDlg, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM /*lParam*/)
{
    if (uMessage == WM_COMMAND)
    {
        switch (LOWORD(wParam)) 
        {
            case IDOK:
            {
                InputBoxDlg.OnBtOK();
                InputBoxDlg.CloseDialog(hDlg, CInputBoxDlg::RET_OK);
                return 1;
            }
            case IDCANCEL:
            {
                InputBoxDlg.CloseDialog(hDlg, CInputBoxDlg::RET_CANCEL);
                return 1;
            }
            case IDC_BT_KILL:
            {
                InputBoxDlg.CloseDialog(hDlg, CInputBoxDlg::RET_KILL);
                return 1;
            }
            default:
                break;
        }
    }

    else if (uMessage == WM_SYSCOMMAND)
    {
        if (wParam == SC_CLOSE)
        {
            //if ( InputBoxDlg.getInputBoxType() != CInputBoxDlg::IBT_INPUTBOX )
            {
                InputBoxDlg.CloseDialog(hDlg, CInputBoxDlg::RET_CANCEL);
            } // else don't close
            return 1;
        }
    }

    else if (uMessage == WM_TIMER)
    {
        InputBoxDlg.OnTimer(hDlg, wParam);
    }

    else if (uMessage == WM_SHOWWINDOW)
    {
        InputBoxDlg.OnShowWindow(hDlg, wParam);
    }

    else if (uMessage == WM_INITDIALOG)
    {
        InputBoxDlg.OnInitDialog(hDlg);
    }

    return 0;
}

CInputBoxDlg::CInputBoxDlg() : CAnyWindow()
{
    m_type = IBT_INPUTBOX;
    m_nInputIndex[IBT_INPUTBOX] = -1;
    m_nInputIndex[IBT_EXITPROCESS] = -1;
    m_nTimeout = 0;
    m_idTimerTimeout = 0;
}

CInputBoxDlg::~CInputBoxDlg()
{
}

void CInputBoxDlg::updateCaptionWithTimeout(HWND hDlg)
{
    (hDlg);

    if ( m_nTimeout != 0 )
    {
        if ( m_type == IBT_INPUTBOX )
        {
            double remaining = m_nTimeout;
            remaining /= 1000;
            int remaining_int = static_cast<int>(remaining);

            TCHAR szCaption[96];
            if ( remaining - remaining_int > 0.0505 )
                _t_sprintf(szCaption, _T("NppExec InputBox (%0.1f seconds remaining)"), remaining);
            else
                _t_sprintf(szCaption, _T("NppExec InputBox (%d seconds remaining)"), remaining_int);

            SetWindowText(szCaption);
        }
    }
}

void CInputBoxDlg::applyInputBoxType()
{ 
    RECT rc;
    
    switch ( m_type )
    {
        case IBT_INPUTBOX:
            SetWindowText( _T("NppExec InputBox") );
            m_btKill.ShowWindow(SW_HIDE);
            m_btCancel.ShowWindow(SW_HIDE);
            ::GetWindowRect(m_btOK.m_hWnd, &rc);
            rc.left = m_nScndBtnLeftPos;
            ::ScreenToClient(m_hWnd, (POINT *) &rc);
            m_btOK.MoveWindow(rc.left, rc.top);
            m_btOK.SetWindowText( _T("OK") );
            m_btOK.ShowWindow(SW_SHOW);
            break;
            
        case IBT_EXITPROCESS:
            SetWindowText( _T("NppExec - WARNING - Console process is still running") );
            ::GetWindowRect(m_btOK.m_hWnd, &rc);
            rc.left = m_nFrstBtnLeftPos;
            ::ScreenToClient(m_hWnd, (POINT *) &rc);
            m_btOK.MoveWindow(rc.left, rc.top);
            ::GetWindowRect(m_btKill.m_hWnd, &rc);
            rc.left = m_nScndBtnLeftPos;
            ::ScreenToClient(m_hWnd, (POINT *) &rc);
            m_btKill.MoveWindow(rc.left, rc.top);
            m_btOK.SetWindowText( _T("Send") );
            m_btOK.ShowWindow(SW_SHOW);
            m_btKill.ShowWindow(SW_SHOW);
            m_btCancel.ShowWindow(SW_SHOW);
            break;
    }
}

void CInputBoxDlg::CloseDialog(HWND hDlg, INT_PTR nResult)
{
    if ( m_idTimerTimeout != 0 )
    {
        ::KillTimer(hDlg, m_idTimerTimeout);
    }
    ::EndDialog(hDlg, nResult);
}

void CInputBoxDlg::OnBtOK()
{
    TCHAR szValue[1000];

    szValue[0] = 0;
    m_cbVarValue.GetText(szValue, 1000 - 1);
    m_OutputValue = szValue;

    CListItemT<tstr>* p = m_InputHistory[m_type].FindExact(m_OutputValue);
    if ( p )
    {
        if ( p != m_InputHistory[m_type].GetFirst() )
        {
            m_InputHistory[m_type].Delete( p );
            m_InputHistory[m_type].InsertFirst( m_OutputValue );
        }
        m_nInputIndex[m_type] = 0;
    }
    else
    {
        m_InputHistory[m_type].InsertFirst(m_OutputValue);
        m_nInputIndex[m_type] = 0;

        if ( m_InputHistory[m_type].GetCount() > HISTORY_ITEMS + 10 )
        {
            m_InputHistory[m_type].DeleteLast();
        }
    }
   
}

void CInputBoxDlg::OnTimer(HWND hDlg, WPARAM wParam)
{
    if ( wParam == ID_TIMER_TIMEOUT )
    {
        updateCaptionWithTimeout(hDlg);

        UINT uElapse;
        if ( m_nTimeout >= 1000 )
        {
            uElapse = 1000;
            m_nTimeout -= 1000;
        }
        else
        {
            uElapse = m_nTimeout;
            m_nTimeout = 0;
        }

        if ( uElapse != 0 )
        {
            if ( uElapse != 1000 )
            {
                ::KillTimer(hDlg, m_idTimerTimeout);
                m_idTimerTimeout = ::SetTimer(hDlg, ID_TIMER_TIMEOUT, uElapse, NULL);
            }
            // else use the existing timer
        }
        else
        {
            OnBtOK();
            CloseDialog(hDlg, CInputBoxDlg::RET_OK);
        }
    }
}

void CInputBoxDlg::OnShowWindow(HWND hDlg, WPARAM wParam)
{
    if ( wParam ) // showing a window
    {
        if ( m_nTimeout != 0 )
        {
            updateCaptionWithTimeout(hDlg);

            UINT uElapse;
            if ( m_nTimeout >= 1000 )
            {
                uElapse = 1000;
                m_nTimeout -= 1000;
            }
            else
            {
                uElapse = m_nTimeout;
                m_nTimeout = 0;
            }
            m_idTimerTimeout = ::SetTimer(hDlg, ID_TIMER_TIMEOUT, uElapse, NULL);
        }
    }
}

void CInputBoxDlg::OnInitDialog(HWND hDlg)
{
    m_hWnd = hDlg;
    CenterWindow(Runtime::GetNppExec().m_nppData._nppHandle);

    m_btOK.m_hWnd = ::GetDlgItem(hDlg, IDOK);
    m_btKill.m_hWnd = ::GetDlgItem(hDlg, IDC_BT_KILL);
    m_btCancel.m_hWnd = ::GetDlgItem(hDlg, IDCANCEL);
    m_stMessage.m_hWnd = ::GetDlgItem(hDlg, IDC_ST_VARMESSAGE);
    m_stVarName.m_hWnd = ::GetDlgItem(hDlg, IDC_ST_VARNAME);
    m_cbVarValue.m_hWnd = ::GetDlgItem(hDlg, IDC_CB_VARVALUE);

    RECT rc;
    ::GetWindowRect(m_btKill.m_hWnd, &rc);
    m_nFrstBtnLeftPos = rc.left;
    ::GetWindowRect(m_btOK.m_hWnd, &rc);
    m_nScndBtnLeftPos = rc.left;

    m_nTimeout = 0;
    m_idTimerTimeout = 0;

    CStrSplitT<TCHAR> msg;
    if ( msg.SplitAsArgs(m_InputMessage, _T(':'), 2) == 2 )
    {
        m_stMessage.SetWindowText( msg.GetArg(0).c_str() );
        tstr initialValue = msg.GetArg(1);
        if ( m_type == IBT_INPUTBOX )
        {
            CStrSplitT<TCHAR> valueSpecs;
            int n = valueSpecs.SplitAsArgs(initialValue, _T(':'), 3);
            if ( n >= 2 )
            {
                m_InputVarName = valueSpecs.GetArg(0);
                NppExecHelpers::StrUnquoteEx(m_InputVarName);
                if ( m_InputVarName.IsEmpty() )
                {
                    // special case: using the default value name
                    m_InputVarName = MACRO_INPUT;
                    m_InputVarName += _T(" =");
                }
                initialValue = valueSpecs.GetArg(1);
                if ( n == 3 )
                {
                    m_nTimeout = c_base::_tstr2uint(valueSpecs.GetArg(2).c_str());
                }
            }
        }
        m_cbVarValue.SetText( initialValue.c_str() );
        m_nInputIndex[m_type] = m_cbVarValue.FindStringExact( initialValue.c_str() );
    }
    else
    {
        NppExecHelpers::StrUnquoteEx(m_InputMessage);
        m_stMessage.SetWindowText( m_InputMessage.c_str() );
    }
    
    m_stVarName.SetWindowText( m_InputVarName.c_str() );

    // keep previous value instead of m_OutputValue.Clear();

    CListItemT<tstr>* p = m_InputHistory[m_type].GetFirst();
    while ( p )
    {
        m_cbVarValue.AddString( p->GetItem().c_str() );
        p = p->GetNext();
    }

    if ( (m_nInputIndex[m_type] >= 0) && (m_nInputIndex[m_type] < m_cbVarValue.GetCount()) )
    {
        m_cbVarValue.SetCurSel(m_nInputIndex[m_type]);
    }

    applyInputBoxType();
}
