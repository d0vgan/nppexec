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

#ifndef _any_rich_edit_h_
#define _any_rich_edit_h_
//---------------------------------------------------------------------------

// "base.h" must be the first header file
#include "base.h"
#include "CAnyWindow.h"
#include <TCHAR.h>
#include <richedit.h>

// __MINGW32__
#ifndef IMF_AUTOFONT
  #define IMF_AUTOFONT 0x0002
#endif

class CAnyRichEdit : public CAnyWindow {

private:

public:
  
  CAnyRichEdit();
  ~CAnyRichEdit();
  void  AddLine(const TCHAR* cszLine, BOOL bScrollText = FALSE, 
          COLORREF color = RGB(0,0,0), 
          DWORD dwMask = CFM_EFFECTS, DWORD dwEffects = 0);
  void  AddStr(const TCHAR* cszStr, BOOL bScrollText = FALSE, 
          COLORREF color = RGB(0,0,0), 
          DWORD dwMask = CFM_EFFECTS, DWORD dwEffects = 0);
  INT   ExGetSelPos(INT* pnStartPos = NULL, INT* pnEndPos = NULL) const;
  void  ExLimitText(INT nMaxLength);
  INT   ExLineFromChar(INT nCharacterIndex) const;
  void  ExSetSel(INT nStartPos, INT nEndPos);
  TCHAR GetCharAt(INT nPos) const;
  DWORD GetEventMask() const;
  INT   GetLine(INT nLine, TCHAR* lpTextBuf, WORD wTextBufSize) const;
  INT   GetLineCount() const;
  BOOL  GetModify() const;
  INT   GetSelPos(INT* pnStartPos = NULL, INT* pnEndPos = NULL) const;
  INT   GetTextAt(INT nPos, INT nLen, TCHAR* lpText) const;
  INT   GetTextLengthEx() const;
  INT   LineFromChar(INT nCharacterIndex) const;
  INT   LineIndex(INT nLineNumber) const;
  INT   LineLength(INT nCharacterIndex) const;
  BOOL  LineScroll(INT nVerticalLines);
  void  ReplaceSelText(const TCHAR* cszText, BOOL bCanUndo = FALSE);
  void  ScrollCaret();
  BOOL  SetCharFormat(DWORD dwMask, DWORD dwEffects,
          COLORREF color = RGB(0,0,0), DWORD dwOptions = SCF_SELECTION);
  DWORD SetEventMask(DWORD nEventMask);
  BOOL  SetFont(const LOGFONT* plf);
  void  SetModify(BOOL bModified);
  void  SetSel(INT nStartPos, INT nEndPos);

#ifdef any_ctrl_enable_w_members
  INT   ExGetSelPosW(INT* pnStartPos = NULL, INT* pnEndPos = NULL) const;
  void  ExLimitTextW(INT nMaxLength);
  void  ExSetSelW(INT nStartPos, INT nEndPos);
  WCHAR GetCharAtW(INT nPos) const;
  DWORD GetEventMaskW() const;
  INT   GetLineW(INT nLine, WCHAR* lpText) const;
  INT   GetLineCountW() const;
  INT   GetTextAtW(INT nPos, INT nLen, WCHAR* lpText) const;
  void  ReplaceSelTextW(const WCHAR* cszText, BOOL bCanUndo = FALSE);  
  BOOL  SetCharFormatW(DWORD dwMask, DWORD dwEffects,
          COLORREF color = RGB(0,0,0), DWORD dwOptions = SCF_SELECTION);
  DWORD SetEventMaskW(DWORD nEventMask);
#endif

};

//---------------------------------------------------------------------------
#endif
