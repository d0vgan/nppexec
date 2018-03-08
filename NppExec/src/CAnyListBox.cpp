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

#include "CAnyListBox.h"

CAnyListBox::CAnyListBox() : CAnyWindow() 
{
}

CAnyListBox::~CAnyListBox()
{                         
}                         

int CAnyListBox::AddString(LPCTSTR cszText)
{
  return ( (int) SendMsg(LB_ADDSTRING, 0, (LPARAM) cszText) );
}

int CAnyListBox::DeleteString(int nItemIndex)
{
  return ( (int) SendMsg(LB_DELETESTRING, (WPARAM) nItemIndex, 0) );
}

int CAnyListBox::FindString(LPCTSTR cszText, int nAfterIndex ) const
{
  return ( (int) SendMsg(LB_FINDSTRING, (WPARAM) nAfterIndex, (LPARAM) cszText) );
}

int CAnyListBox::FindStringExact(LPCTSTR cszText, int nAfterIndex ) const
{
  return ( (int) SendMsg(LB_FINDSTRINGEXACT, (WPARAM) nAfterIndex, (LPARAM) cszText) );
}

int CAnyListBox::GetCount() const
{
  return ( (int) SendMsg(LB_GETCOUNT, 0, 0) );
}

int CAnyListBox::GetCurSel() const
{
  return ( (int) SendMsg(LB_GETCURSEL, 0, 0) );
}

LRESULT CAnyListBox::GetItemData(int nItemIndex) const
{
  return ( (int) SendMsg(LB_GETITEMDATA, (WPARAM) nItemIndex, 0) );
}

int CAnyListBox::GetString(int nItemIndex, TCHAR* lpText)
{
  return ( (int) SendMsg(LB_GETTEXT, (WPARAM) nItemIndex, (LPARAM) lpText) );
}

int CAnyListBox::GetStringLength(int nItemIndex) const
{
  return ( (int) SendMsg(LB_GETTEXTLEN, (WPARAM) nItemIndex, 0) );
}

int CAnyListBox::InsertString(int nItemIndex, LPCTSTR cszText)
{
  return ( (int) SendMsg(LB_INSERTSTRING, (WPARAM) nItemIndex, (LPARAM) cszText) );
}

void CAnyListBox::ResetContent()
{
  SendMsg(LB_RESETCONTENT, 0, 0);
}

int CAnyListBox::SetCurSel(int nItemIndex)
{
  return ( (int) SendMsg(LB_SETCURSEL, (WPARAM) nItemIndex, 0) );
}

int CAnyListBox::SetItemData(int nItemIndex, LPARAM lData)
{
  return ( (int) SendMsg(LB_SETITEMDATA, (WPARAM) nItemIndex, lData) );
}
