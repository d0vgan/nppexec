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

#include "DlgConsoleEncoding.h"
#include "NppExec.h"


CConsoleEncodingDlg ConsoleEncodingDlg;



INT_PTR CALLBACK ConsoleEncodingDlgProc(
  HWND   hDlg, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM /*lParam*/)
{
    if (uMessage == WM_COMMAND)
    {
        switch (LOWORD(wParam)) 
        {
            case IDC_CH_INPUT_AS_OUTPUT:
            case IDC_RB_OUTPUT_ANSI:
            case IDC_RB_OUTPUT_OEM: 
            case IDC_RB_OUTPUT_UTF8:
            {
                if ( HIWORD(wParam) == BN_CLICKED )
                {
                    ConsoleEncodingDlg.OnChInputAsOutput();
                    return 1;
                }
                return 0;
            }
            case IDOK:
            {
                ConsoleEncodingDlg.OnBtOK();
                EndDialog(hDlg, 1);
                return 1;
            }
            case IDCANCEL:
            {
                EndDialog(hDlg, 0);
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
            EndDialog(hDlg, 0);
            return 1;
        }
    }

    else if (uMessage == WM_INITDIALOG)
    {
        ConsoleEncodingDlg.OnInitDialog(hDlg);
    }

    return 0;
}

const TCHAR* CConsoleEncodingDlg::szEncNames[ENC_TOTAL] = 
{
    _T("ANSI"),
    _T("OEM"),
    _T("UTF8")
};

CConsoleEncodingDlg::CConsoleEncodingDlg() : CAnyWindow(), m_enc_opt(0)
{
}

CConsoleEncodingDlg::~CConsoleEncodingDlg()
{
}

void CConsoleEncodingDlg::OnBtOK()
{
    unsigned int outEnc = ENC_ANSI;
    unsigned int inEnc = ENC_ANSI;
    bool bInputAsOutput = false;

    if ( ::IsDlgButtonChecked(m_hWnd, IDC_CH_INPUT_AS_OUTPUT) == BST_CHECKED )
        bInputAsOutput = true;

    for ( int i = IDC_RB_OUTPUT_ANSI; i < IDC_RB_OUTPUT_ANSI + ENC_TOTAL; i++ )
    {
        if ( ::IsDlgButtonChecked(m_hWnd, i) == BST_CHECKED )
        {
            outEnc = i - IDC_RB_OUTPUT_ANSI;
            break;
        }
    }

    if ( bInputAsOutput )
    {
        inEnc = outEnc;
    }
    else
    {
        for ( int i = IDC_RB_INPUT_ANSI; i < IDC_RB_INPUT_ANSI + ENC_TOTAL; i++ )
        {
            if ( ::IsDlgButtonChecked(m_hWnd, i) == BST_CHECKED )
            {
                inEnc = i - IDC_RB_INPUT_ANSI;
                break;
            }
        }
    }

    m_enc_opt = 0;
    m_enc_opt |= outEnc;
    m_enc_opt |= (inEnc * 0x10);
    if ( bInputAsOutput )
        m_enc_opt |= 0x0100;

}

void CConsoleEncodingDlg::OnChInputAsOutput()
{
    updateUI();
}

void CConsoleEncodingDlg::OnInitDialog(HWND hDlg)
{
    m_hWnd = hDlg;
    CenterWindow(Runtime::GetNppExec().m_nppData._nppHandle);

    unsigned int outEnc = m_enc_opt & 0x0F;
    unsigned int inEnc = (m_enc_opt & 0xF0)/0x10;

    if ( m_enc_opt & 0x0100 )
        inEnc = outEnc;

    ::CheckDlgButton(hDlg, IDC_CH_INPUT_AS_OUTPUT, 
        (m_enc_opt & 0x0100) ? BST_CHECKED : BST_UNCHECKED);
    ::CheckRadioButton(hDlg, IDC_RB_OUTPUT_ANSI, 
        IDC_RB_OUTPUT_ANSI + ENC_TOTAL - 1, IDC_RB_OUTPUT_ANSI + outEnc);
    ::CheckRadioButton(hDlg, IDC_RB_INPUT_ANSI, 
        IDC_RB_INPUT_ANSI + ENC_TOTAL - 1, IDC_RB_INPUT_ANSI + inEnc);

    updateUI();
}

void CConsoleEncodingDlg::updateUI()
{
    HWND hDlgItm;
    BOOL bInputAsOutput = FALSE;
    
    if ( ::IsDlgButtonChecked(m_hWnd, IDC_CH_INPUT_AS_OUTPUT) == BST_CHECKED )
        bInputAsOutput = TRUE;

    hDlgItm = ::GetDlgItem(m_hWnd, IDC_RB_INPUT_ANSI);
    ::EnableWindow(hDlgItm, !bInputAsOutput);
    hDlgItm = ::GetDlgItem(m_hWnd, IDC_RB_INPUT_OEM);
    ::EnableWindow(hDlgItm, !bInputAsOutput);
    hDlgItm = ::GetDlgItem(m_hWnd, IDC_RB_INPUT_UTF8);
    ::EnableWindow(hDlgItm, !bInputAsOutput);

    if ( bInputAsOutput )
    {
        unsigned int outEnc = ENC_ANSI;
        
        for ( int i = IDC_RB_OUTPUT_ANSI; i < IDC_RB_OUTPUT_ANSI + ENC_TOTAL; i++ )
        {
            if ( ::IsDlgButtonChecked(m_hWnd, i) == BST_CHECKED )
            {
                outEnc = i - IDC_RB_OUTPUT_ANSI;
                break;
            }
        }

        ::CheckRadioButton(m_hWnd, IDC_RB_INPUT_ANSI, 
            IDC_RB_INPUT_ANSI + ENC_TOTAL - 1, IDC_RB_INPUT_ANSI + outEnc);
    }
}

unsigned int CConsoleEncodingDlg::defaultEncodingOption()
{
    return ENC_OEM + (ENC_OEM * 0x10);
}

unsigned int CConsoleEncodingDlg::getEnc(unsigned int enc)
{
    switch ( enc )
    {
        case ENC_ANSI:
            return ENC_ANSI;
        case ENC_OEM:
            return ENC_OEM;
        case ENC_UTF8:
            return ENC_UTF8;
        default:
            return ENC_ANSI;
    }
}

unsigned int CConsoleEncodingDlg::getInputEncoding(unsigned int enc_opt)
{
    return getEnc( (enc_opt & 0xF0)/0x10 );
}

const TCHAR* CConsoleEncodingDlg::getInputEncodingName(unsigned int enc_opt)
{
    return szEncNames[getInputEncoding(enc_opt)];
}

unsigned int CConsoleEncodingDlg::getOutputEncoding(unsigned int enc_opt)
{
    return getEnc( enc_opt & 0x0F );
}

const TCHAR* CConsoleEncodingDlg::getOutputEncodingName(unsigned int enc_opt)
{
    return szEncNames[getOutputEncoding(enc_opt)];
}
