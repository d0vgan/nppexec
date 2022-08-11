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

#ifndef _popup_list_box_h_
#define _popup_list_box_h_
//---------------------------------------------------------------------------
#include "CAnyWindow.h"
#include "CAnyListBox.h"


class CPopupListBox : public CAnyListBox
{
private:
  HWND m_hParentWnd;

public:
  CPopupListBox();
  ~CPopupListBox();
  HWND Create(HWND hParentWnd, 
    int left = 0, int top = 0, int width = 60, int height = 120);
  void Destroy();
  bool FillPopupList(const TCHAR* szCurrentWord);
  HWND GetParentWnd() const;
  void SetParentWnd(HWND hParentWnd);
  bool Show(const TCHAR* szCurrentWord);

private:
  int getRequiredWidth();
};

//---------------------------------------------------------------------------
#endif

