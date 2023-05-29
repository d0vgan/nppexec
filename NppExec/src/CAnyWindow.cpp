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

#include "CAnyWindow.h"

CAnyWindow::CAnyWindow() : m_hWnd(NULL)
{
}

CAnyWindow::~CAnyWindow()
{

}

BOOL CAnyWindow::BringWindowToTop()
{
  return ::BringWindowToTop(m_hWnd);
}

BOOL CAnyWindow::CenterWindow(HWND hParentWnd, BOOL bRepaint)
{
  RECT rectParent;
  RECT rect;
  INT  height, width;
  INT  x, y;

  ::GetWindowRect(hParentWnd, &rectParent);
  ::GetWindowRect(m_hWnd, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;
  x = ((rectParent.right - rectParent.left) - width) / 2;
  x += rectParent.left;
  y = ((rectParent.bottom - rectParent.top) - height) / 2;
  y += rectParent.top;
  return ::MoveWindow(m_hWnd, x, y, width, height, bRepaint);
}

BOOL CAnyWindow::EnableWindow(BOOL bEnable)
{
  return ::EnableWindow(m_hWnd, bEnable);
}

INT CAnyWindow::GetText(TCHAR* lpTextBuf, INT nTextBufSize)
{
  return ( (INT) SendMsg(WM_GETTEXT, 
    (WPARAM) nTextBufSize, (LPARAM) lpTextBuf) );
}

INT CAnyWindow::GetTextLength() const
{
  return ( (INT) SendMsg(WM_GETTEXTLENGTH, 0, 0) );
}

INT CAnyWindow::GetWindowText(TCHAR* lpTextBuf, INT nTextBufSize)
{
  return ::GetWindowText(m_hWnd, lpTextBuf, nTextBufSize);
}

INT CAnyWindow::GetWindowTextLength() const
{
  return ::GetWindowTextLength(m_hWnd);
}

BOOL CAnyWindow::IsWindowVisible() const
{
  return ::IsWindowVisible(m_hWnd);
}

BOOL CAnyWindow::MoveWindow(INT x, INT y, BOOL bRepaint)
{
  RECT rect;
  INT  height, width;

  ::GetWindowRect(m_hWnd, &rect);
  width = rect.right - rect.left;
  height = rect.bottom - rect.top;
  return ::MoveWindow(m_hWnd, x, y, width, height, bRepaint);
}

BOOL CAnyWindow::MoveWindow(INT x, INT y, INT width, INT height, BOOL bRepaint)
{
  return ::MoveWindow(m_hWnd, x, y, width, height, bRepaint);
}

BOOL CAnyWindow::Redraw()
{
  if (::InvalidateRect(m_hWnd, NULL, TRUE))
    return ::UpdateWindow(m_hWnd);
  else
    return FALSE;
}

BOOL CAnyWindow::ResizeWindow(INT width, INT height, BOOL bRepaint)
{
  RECT rect;

  ::GetWindowRect(m_hWnd, &rect);
  return ::MoveWindow(m_hWnd, rect.left, rect.top, width, height, bRepaint);
}

LRESULT CAnyWindow::SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam) const
{
  return ::SendMessage(m_hWnd, Msg, wParam, lParam);
}

HWND CAnyWindow::SetFocus()
{
  return ::SetFocus(m_hWnd);
}

HWND CAnyWindow::SetParent(HWND hWndNewParent)
{
  return ::SetParent(m_hWnd, hWndNewParent);
}

void CAnyWindow::SetRedraw(BOOL bRedraw)
{
  SendMsg(WM_SETREDRAW, (WPARAM) bRedraw, 0);
}

BOOL CAnyWindow::SetText(const TCHAR* cszText)
{
  return ( SendMsg(WM_SETTEXT, 0, (LPARAM) cszText) == TRUE );
}

BOOL CAnyWindow::SetWindowText(const TCHAR* cszText)
{
  return ::SetWindowText(m_hWnd, cszText);
}

BOOL CAnyWindow::ShowWindow(INT nCmdShow)
{
  return ::ShowWindow(m_hWnd, nCmdShow);
}

BOOL CAnyWindow::UpdateWindow()
{
  return ::UpdateWindow(m_hWnd);
}

///////////////////////////////////////////////////////////////////////////
// Additional Unicode functions

#ifdef any_ctrl_enable_w_members

  INT CAnyWindow::GetTextW(WCHAR* lpTextBuf, INT nTextBufSize)
  {
    return ( (INT) SendMsgW(WM_GETTEXT,  
      (WPARAM) nTextBufSize, (LPARAM) lpTextBuf) );
  }

  INT CAnyWindow::GetTextLengthW() const
  {
    return ( (INT) SendMsgW(WM_GETTEXTLENGTH, 0, 0) );
  }

  INT CAnyWindow::GetWindowTextW(WCHAR* lpTextBuf, INT nTextBufSize)
  {
    return ::GetWindowTextW(m_hWnd, lpTextBuf, nTextBufSize);
  }

  INT CAnyWindow::GetWindowTextLengthW() const
  {
    return ::GetWindowTextLengthW(m_hWnd);
  }
  
  LRESULT CAnyWindow::SendMsgW(UINT Msg, WPARAM wParam, LPARAM lParam) const
  {
    return ::SendMessageW(m_hWnd, Msg, wParam, lParam);
  }
  
  BOOL CAnyWindow::SetTextW(const WCHAR* cszText)
  {
    return ( SendMsgW(WM_SETTEXT, 0, (LPARAM) cszText) == TRUE );
  }

  BOOL CAnyWindow::SetWindowTextW(const WCHAR* cszText)
  {
    return ::SetWindowTextW(m_hWnd, cszText);
  }

#endif

