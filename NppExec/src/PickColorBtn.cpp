/*
This file is part of NppExec
Copyright (C) 2023 Anders Kjersem.

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

#include <windows.h>
#include <commdlg.h>
#include <CommCtrl.h>
#include "PickColorBtn.h"


static inline void PickColorBtn_FillRectColor(HDC hDC, const RECT*pR, COLORREF Clr)
{
    UINT orgbc = SetBkColor(hDC, Clr);
    ExtTextOut(hDC, 0, 0, ETO_OPAQUE, pR, NULL, 0, NULL);
    SetBkColor(hDC, orgbc);
}

INT_PTR CALLBACK PickColorBtn_HandleMessage(HWND hDlg, UINT Msg, WPARAM WPar, LPARAM LPar, LRESULT *RetVal)
{
    DRAWITEMSTRUCT*pDIS = (DRAWITEMSTRUCT*) LPar;
    switch(Msg)
    {
    case WM_DRAWITEM:
        if (pDIS->CtlType != ODT_BUTTON)
        {
            break;
        }

        if (pDIS->itemAction != ODA_FOCUS)
        {
            if (pDIS->itemState & ODS_DISABLED)
            {
                HBRUSH hBr = CreateHatchBrush(HS_DIAGCROSS, GetSysColor(COLOR_GRAYTEXT));
                UINT obc = SetBkColor(pDIS->hDC, GetSysColor(COLOR_3DFACE));
                FillRect(pDIS->hDC, &pDIS->rcItem, hBr);
                SetBkColor(pDIS->hDC, obc);
                DeleteObject(hBr);
            }
            else
            {
                PickColorBtn_FillRectColor(pDIS->hDC, &pDIS->rcItem, PickColorBtn_GetColor(pDIS->hwndItem) & 0xffffff);
            }
        }

        if (pDIS->itemAction == ODA_FOCUS || (pDIS->itemState & ODS_FOCUS))
        {
            DrawFocusRect(pDIS->hDC, &pDIS->rcItem);
        }
        return (*RetVal = (SIZE_T) pDIS);

    case WM_COMMAND:
        if (LPar && (GetWindowLongPtr((HWND) LPar, GWL_STYLE) & BS_TYPEMASK) == BS_OWNERDRAW)
        {
            SIZE_T id = GetWindowLongPtr((HWND) LPar, GWLP_ID);
            if (MAKEWPARAM(id, BN_CLICKED) == WPar)
            {
                HWND hCtl = (HWND) LPar;
                COLORREF orgclr = PickColorBtn_GetColor(hCtl) & 0xffffff;
                COLORREF colors[16];
                for (SIZE_T i = 0; i < 16; ++i) colors[i] = RGB(i * 16, i * 16, i * 16);

                CHOOSECOLOR cc;
                cc.lStructSize = sizeof(cc);
                cc.hwndOwner = hCtl;
                cc.Flags = CC_RGBINIT | CC_ANYCOLOR | CC_FULLOPEN;
                cc.rgbResult = colors[0] = orgclr;
                cc.lpCustColors = colors;

                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(id, PCBN_INITCHOOSE), (SIZE_T) &cc); // Set cc.lpCustColors to something better if you wish
                if (ChooseColor(&cc))
                {
                    PickColorBtn_SetColor((HWND) LPar, cc.rgbResult);
                }
                return TRUE;
            }
        }
        break;
    }
    return FALSE;
}

INT_PTR CALLBACK PickColorBtn_HandleMessageForDialog(HWND hDlg, UINT Msg, WPARAM WPar, LPARAM LPar)
{
    LRESULT r;
    LRESULT handled = PickColorBtn_HandleMessage(hDlg, Msg, WPar, LPar, &r);
    if (handled) SetWindowLongPtr(hDlg, DWLP_MSGRESULT, r);
    return handled;
}


void PickColorBtn_InitializeTooltips(HWND hDlg, UINT first, UINT last)
{
    HWND hTT = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW, TOOLTIPS_CLASS, 0, WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX, 0, 0, 0, 0, hDlg, 0, 0, 0);
    TOOLINFO ti;
    ti.cbSize = sizeof(ti);
    ti.hwnd = hDlg;
    ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    ti.lpszText = LPSTR_TEXTCALLBACK;

    for ( UINT id = first; id <= last; ++id )
    {
        HWND hCtl = ::GetDlgItem(hDlg, id);
        TCHAR buf[42];
        if (GetClassName(hCtl, buf, 42) && (buf[0]|32) != 'b') continue; // Only buttons
        ti.uId = (SIZE_T) hCtl;
        if ( hTT && ti.uId ) SendMessage(hTT, TTM_ADDTOOL, 0, (LPARAM) &ti);
    }
}

void PickColorBtn_HandleTooltipsNotify(HWND hDlg, WPARAM, LPARAM lParam)
{
    NMTTDISPINFO*pTTDI = (NMTTDISPINFO*) lParam;
    if ( pTTDI->hdr.code == TTN_GETDISPINFO && (pTTDI->uFlags & TTF_IDISHWND) )
    {
        COLORREF c = PickColorBtn_GetColor((HWND) pTTDI->hdr.idFrom);
        wsprintf(pTTDI->szText, TEXT("#%.2X%.2X%.2X"), GetRValue(c), GetGValue(c), GetBValue(c));
    }
}
