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


#include "CAnyComboBox.h"

CAnyComboBox::CAnyComboBox() : CAnyWindow()
{
}

CAnyComboBox::~CAnyComboBox()
{
}

INT CAnyComboBox::AddString(LPCTSTR cszString)
{
  return ((INT) SendMsg(CB_ADDSTRING, 0, (LPARAM) cszString));
}

INT CAnyComboBox::DeleteString(INT nItemIndex)
{
  return ((INT) SendMsg(CB_DELETESTRING, (WPARAM) nItemIndex, 0));
}

INT CAnyComboBox::GetEditSel(INT* pnStartPos , INT* pnEndPos ) const
{
  DWORD n1 = 0, n2 = 0;
  SendMsg(CB_GETEDITSEL, (WPARAM) &n1, (LPARAM) &n2);
  if (pnStartPos)  *pnStartPos = (INT) n1;
  if (pnEndPos)    *pnEndPos = (INT) n2;
  return ((INT) n1);
}

INT CAnyComboBox::FindString(LPCTSTR cszString, INT nAfterIndex ) const
{
  return ((INT) SendMsg(CB_FINDSTRING, 
    (WPARAM) nAfterIndex, (LPARAM) cszString));
}

INT CAnyComboBox::FindStringExact(LPCTSTR cszString, INT nAfterIndex ) const
{
  return ((INT) SendMsg(CB_FINDSTRINGEXACT, 
    (WPARAM) nAfterIndex, (LPARAM) cszString));
}

INT CAnyComboBox::GetCount() const
{
  return ((INT) SendMsg(CB_GETCOUNT, 0, 0));
}

INT CAnyComboBox::GetCurSel() const
{
  return ((INT) SendMsg(CB_GETCURSEL, 0, 0));
}

INT CAnyComboBox::GetLBText(INT nItemIndex, LPTSTR lpString)
{
  return ((INT) SendMsg(CB_GETLBTEXT, 
    (WPARAM) nItemIndex, (LPARAM) lpString));
}

INT CAnyComboBox::GetLBTextLength(INT nItemIndex) const
{
  return ((INT) SendMsg(CB_GETLBTEXTLEN, (WPARAM) nItemIndex, 0));
}

INT CAnyComboBox::InsertString(INT nItemIndex, LPCTSTR cszString)
{
  return ((INT) SendMsg(CB_INSERTSTRING,
    (WPARAM) nItemIndex, (LPARAM) cszString));
}

void CAnyComboBox::LimitText(INT nMaxCharacters)
{
    SendMsg(CB_LIMITTEXT, (WPARAM) nMaxCharacters, 0);
}

void CAnyComboBox::ResetContent()
{
  SendMsg(CB_RESETCONTENT, 0, 0);
}

INT CAnyComboBox::SetCurSel(INT nItemIndex)
{
  return ((INT) SendMsg(CB_SETCURSEL, (WPARAM) nItemIndex, 0));
}

INT CAnyComboBox::SetEditSel(INT nStartPos, INT nEndPos)
{
  LPARAM lParam = nStartPos;
  lParam |= (nEndPos << 16);
  return ((INT) SendMsg(CB_SETEDITSEL, 0, lParam));
}
