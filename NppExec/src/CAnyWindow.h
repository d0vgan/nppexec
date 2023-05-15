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

#ifndef _any_window_h_
#define _any_window_h_
//---------------------------------------------------------------------------

// "base.h" must be the first header file
#include "base.h"

class CAnyWindow {

private:

public:
  HWND m_hWnd;

  CAnyWindow();
  ~CAnyWindow();
  HWND    GetWindowHandle() { return m_hWnd; }
  BOOL    BringWindowToTop();
  BOOL    CenterWindow(HWND hParentWnd, BOOL bRepaint = FALSE);
  BOOL    EnableWindow(BOOL bEnable = TRUE);
  INT     GetText(TCHAR* lpTextBuf, INT nTextBufSize);
  INT     GetTextLength() const;
  INT     GetWindowText(TCHAR* lpTextBuf, INT nTextBufSize);
  INT     GetWindowTextLength() const;
  BOOL    IsWindowVisible() const;
  BOOL    MoveWindow(INT x, INT y, BOOL bRepaint = FALSE);
  BOOL    MoveWindow(INT x, INT y, INT width, INT height, BOOL bRepaint = FALSE);
  BOOL    Redraw();
  BOOL    ResizeWindow(INT width, INT height, BOOL bRepaint = FALSE);
  LRESULT SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam) const;
  HWND    SetFocus();
  HWND    SetParent(HWND hWndNewParent);
  BOOL    SetText(const TCHAR* cszText);
  void    SetRedraw(BOOL bRedraw);
  BOOL    SetWindowText(const TCHAR* cszText);
  BOOL    ShowWindow(INT nCmdShow = SW_SHOWNORMAL);
  BOOL    UpdateWindow();

#ifdef any_ctrl_enable_w_members
  INT     GetTextW(WCHAR* lpTextBuf, INT nTextBufSize);
  INT     GetTextLengthW() const;
  INT     GetWindowTextW(WCHAR* lpTextBuf, INT nTextBufSize);
  INT     GetWindowTextLengthW() const;
  LRESULT SendMsgW(UINT Msg, WPARAM wParam, LPARAM lParam) const;
  BOOL    SetTextW(const WCHAR* cszText);
  BOOL    SetWindowTextW(const WCHAR* cszText);
#endif

};

//---------------------------------------------------------------------------
#endif
