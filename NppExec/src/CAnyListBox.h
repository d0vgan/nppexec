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

#ifndef _Any_ListBox_h_
#define _Any_ListBox_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "CAnyWindow.h"


class CAnyListBox : public CAnyWindow
{
private:

public:
  CAnyListBox();
  ~CAnyListBox();

  int     AddString(LPCTSTR cszText);
  int     DeleteString(int nItemIndex);
  int     FindString(LPCTSTR cszText, int nAfterIndex = -1) const;
            // MSDN: not case sensitive
  int     FindStringExact(LPCTSTR cszText, int nAfterIndex = -1) const;
  int     GetCount() const;
  int     GetCurSel() const;
  LRESULT GetItemData(int nItemIndex) const;
  int     GetString(int nItemIndex, TCHAR* lpText);
  int     GetStringLength(int nItemIndex) const;
  int     InsertString(int nItemIndex, LPCTSTR cszText);
  void    ResetContent();
  int     SetCurSel(int nItemIndex);
  int     SetItemData(int nItemIndex, LPARAM lData);
};


//---------------------------------------------------------------------------
#endif
