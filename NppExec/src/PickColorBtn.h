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

#ifndef PICKCOLORBTN_INC
#define PICKCOLORBTN_INC
#include <windows.h>

enum PICKCOLORBTNNOTIFYCMD {
    PCBN_INITCHOOSE = 0x8000+0,
};

#define PickColorBtn_GetColor(h) ( (COLORREF) GetWindowLongPtr((h), GWLP_USERDATA) )
#define PickColorBtn_SetColor(h, c) ( SetWindowLongPtr((h), GWLP_USERDATA, (c)), InvalidateRect((h), NULL, false) )

INT_PTR CALLBACK PickColorBtn_HandleMessage(HWND hDlg, UINT Msg, WPARAM WPar, LPARAM LPar, LRESULT *RetVal);
INT_PTR CALLBACK PickColorBtn_HandleMessageForDialog(HWND hDlg, UINT Msg, WPARAM WPar, LPARAM LPar);

void PickColorBtn_InitializeTooltips(HWND hDlg, UINT first, UINT last);
void PickColorBtn_HandleTooltipsNotify(HWND hDlg, WPARAM, LPARAM lParam);

#endif
