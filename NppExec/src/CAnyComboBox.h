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

#ifndef _any_combo_box_h_
#define _any_combo_box_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "CAnyWindow.h"

class CAnyComboBox : public CAnyWindow {

private:

public:
  CAnyComboBox();
  ~CAnyComboBox();

  INT  AddString(LPCTSTR cszString);
  INT  DeleteString(INT nItemIndex);
  INT  FindString(LPCTSTR cszString, INT nAfterIndex = -1) const;
  INT  FindStringExact(LPCTSTR cszString, INT nAfterIndex = -1) const;
  INT  GetCount() const;
  INT  GetCurSel() const;
  INT  GetEditSel(INT* pnStartPos = NULL, INT* pnEndPos = NULL) const;
  INT  GetLBText(INT nItemIndex, LPTSTR lpString);
  INT  GetLBTextLength(INT nItemIndex) const;
  INT  InsertString(INT nItemIndex, LPCTSTR cszString);
  void LimitText(INT nMaxCharacters);
  void ResetContent();
  INT  SetCurSel(INT nItemIndex);
  INT  SetEditSel(INT nStartPos, INT nEndPos);

};

//---------------------------------------------------------------------------
#endif
