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

#include "CAnyRichEdit.h"

#define aligned_size(i) ((((i) >> 6) + 1) << 6)

#ifndef mod
  #define mod(a)  ((((int)(a)) < 0) ? (0 - ((int)(a))) : (a))
#endif

static int PixelsToTwips(HWND hWnd, int nPixels)
{
  HDC hDC;
  int nTwips = 0;

  if ((hDC = GetDC(hWnd)) != NULL)
  {
    nTwips = MulDiv(nPixels, 1440, GetDeviceCaps(hDC, LOGPIXELSY));
    ReleaseDC(hWnd, hDC);
  }
  return nTwips;
}

CAnyRichEdit::CAnyRichEdit() : CAnyWindow()
{
}

CAnyRichEdit::~CAnyRichEdit()
{
}

void CAnyRichEdit::AddLine(const TCHAR* cszLine, BOOL bScrollText /* = FALSE */, 
  COLORREF color /* = RGB(0,0,0) */, 
  DWORD dwMask /* = CFM_EFFECTS */, DWORD dwEffects /* = 0 */)
{
  if (cszLine != NULL)
  {
    INT nLen = lstrlen(cszLine);
    TCHAR* lpLine = new TCHAR[aligned_size(nLen+3)];
    if (lpLine != NULL)
    {
      lstrcpy(lpLine, cszLine);
      lstrcat(lpLine, _T_RE_EOL);
      AddStr(lpLine, bScrollText, color, dwMask, dwEffects);
      delete [] lpLine;
    }
    else
    {
      AddStr(cszLine, FALSE, color, dwMask, dwEffects);
      AddStr(_T_RE_EOL, bScrollText, color, dwMask, dwEffects);
    }
  }
}

void CAnyRichEdit::AddStr(const TCHAR* cszStr, BOOL bScrollText /* = FALSE */, 
  COLORREF color /* = RGB(0,0,0) */, 
  DWORD dwMask /* = CFM_EFFECTS */, DWORD dwEffects /* = 0 */)
{
  INT       nOldLineCount = 0; 
  CHARRANGE cr = {0, 0};

  if (!bScrollText) 
    SendMsg(EM_EXGETSEL, 0, (LPARAM) &cr);
  else
    nOldLineCount = GetLineCount();

  // no redraw while adding a string
  if (!bScrollText) 
    SetRedraw(FALSE);
    
  ExSetSel(-1, -1);
  SetCharFormat(dwMask, dwEffects, color);
  ReplaceSelText(cszStr);

  if (!bScrollText)
  {
    SendMsg(EM_EXSETSEL, 0, (LPARAM) &cr);
    ScrollCaret();
    SetRedraw(TRUE);
    Redraw();
  }
  else
  {
    //INT nPos = GetTextLengthEx();
    //SetRedraw(TRUE);
    //ExSetSel(nPos, nPos);
    LineScroll(GetLineCount() - nOldLineCount);
  }
  
  //Redraw();
}

INT CAnyRichEdit::ExGetSelPos(INT* pnStartPos, INT* pnEndPos) const
{
  CHARRANGE cr;
  SendMsg(EM_EXGETSEL, 0, (LPARAM) &cr);
  if (pnStartPos != NULL)
    *pnStartPos = (INT) cr.cpMin;
  if (pnEndPos != NULL)
    *pnEndPos = (INT) cr.cpMax;

  return ((INT) cr.cpMin);
}

void CAnyRichEdit::ExLimitText(INT nMaxLength)
{
  SendMsg(EM_EXLIMITTEXT, 0, (LPARAM) nMaxLength);
}

INT CAnyRichEdit::ExLineFromChar(INT nCharacterIndex) const
{
  return ((INT) SendMsg(EM_EXLINEFROMCHAR, 0, (LPARAM) nCharacterIndex));
}

void CAnyRichEdit::ExSetSel(INT nStartPos, INT nEndPos)
{
  CHARRANGE cr;
  cr.cpMin = (LONG) nStartPos;
  cr.cpMax = (LONG) nEndPos;
  SendMsg(EM_EXSETSEL, 0, (LPARAM) &cr);
}

TCHAR CAnyRichEdit::GetCharAt(INT nPos) const
{
  TCHAR     ch = 0;
  TCHAR     str[4];

  if (GetTextAt(nPos, 1, str) > 0)
  {
    ch = str[0];
  }

  return ch;
}

DWORD CAnyRichEdit::GetEventMask() const
{
  return ((DWORD) SendMsg(EM_GETEVENTMASK, 0, 0));
}

INT CAnyRichEdit::GetLine(INT nLine, TCHAR* lpTextBuf, WORD wTextBufSize) const
{
  INT   nLen;  
  WORD* p;
  
  p = (WORD*) lpTextBuf;
  *p = wTextBufSize;
  nLen = (INT) SendMsg(EM_GETLINE, (WPARAM) nLine, (LPARAM) lpTextBuf);
  lpTextBuf[nLen] = 0;
  return nLen;
}

INT CAnyRichEdit::GetLineCount() const
{
  return ((INT) SendMsg(EM_GETLINECOUNT, 0, 0));
}

BOOL CAnyRichEdit::GetModify() const
{
  return (SendMsg(EM_GETMODIFY, 0, 0) != 0);
}

INT CAnyRichEdit::GetSelPos(INT* pnStartPos, INT* pnEndPos) const
{
  DWORD dwStartPos, dwEndPos;

  SendMsg(EM_GETSEL, (WPARAM) &dwStartPos, (LPARAM) &dwEndPos);
  if (pnStartPos)
    *pnStartPos = (INT) dwStartPos;
  if (pnEndPos)
    *pnEndPos = (INT) dwEndPos;

  return ( (INT) dwStartPos );
}

INT CAnyRichEdit::GetTextAt(INT nPos, INT nLen, TCHAR* lpText) const
{
  INT       nRet;
  TEXTRANGE tr;

  tr.lpstrText = lpText;
  tr.chrg.cpMin = nPos;
  tr.chrg.cpMax = nPos + nLen;
  nRet = (INT) SendMsg(EM_GETTEXTRANGE, 0, (LPARAM) &tr);

  return nRet;
}

INT CAnyRichEdit::GetTextLengthEx() const
{
  GETTEXTLENGTHEX gtle = { 0, 0 };

  gtle.flags = GTL_DEFAULT;
  gtle.codepage = ::IsWindowUnicode(m_hWnd) ? 1200 : CP_ACP;

  return (INT) SendMsg(EM_GETTEXTLENGTHEX, (WPARAM) &gtle, 0);
}

INT CAnyRichEdit::LineFromChar(INT nCharacterIndex) const
{
  return ((INT) SendMsg(EM_LINEFROMCHAR, (WPARAM) nCharacterIndex, 0));
}

INT CAnyRichEdit::LineIndex(INT nLineNumber) const
{
  return ((INT) SendMsg(EM_LINEINDEX, (WPARAM) nLineNumber, 0));
}

INT CAnyRichEdit::LineLength(INT nCharacterIndex) const
{
  return ((INT) SendMsg(EM_LINELENGTH, (WPARAM) nCharacterIndex, 0));
}

BOOL CAnyRichEdit::LineScroll(INT nVerticalLines)
{
  return ((BOOL) SendMsg(EM_LINESCROLL, 0, (LPARAM) nVerticalLines));
}

void CAnyRichEdit::ReplaceSelText(const TCHAR* cszText, BOOL bCanUndo )
{
  SendMsg(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM) cszText);
}

void CAnyRichEdit::ScrollCaret()
{
  SendMsg(EM_SCROLLCARET, 0, 0);
}

BOOL CAnyRichEdit::SetCharFormat(DWORD dwMask,
  DWORD dwEffects, COLORREF color, DWORD dwOptions)
{
  BOOL       bRet;
  CHARFORMAT cf;

  cf.cbSize = sizeof(CHARFORMAT);
  cf.dwMask = dwMask;
  cf.dwEffects = dwEffects;
  cf.crTextColor = color;
  bRet = (BOOL) SendMsg(EM_SETCHARFORMAT, (WPARAM) dwOptions, (LPARAM) &cf);

  return bRet;
}

DWORD CAnyRichEdit::SetEventMask(DWORD nEventMask)
{
  DWORD dwRet;

  dwRet = (DWORD) SendMsg(EM_SETEVENTMASK, 0, (LPARAM) nEventMask);

  return dwRet;
}

BOOL CAnyRichEdit::SetFont(const LOGFONT* plf)
{
  BOOL       bRet;
  CHARFORMAT cfmt;

  cfmt.cbSize = sizeof(CHARFORMAT);
  cfmt.dwMask = CFM_SIZE | CFM_CHARSET | CFM_FACE | CFM_BOLD | CFM_ITALIC;
  cfmt.yHeight = mod(PixelsToTwips(m_hWnd, plf->lfHeight));
  cfmt.bCharSet = plf->lfCharSet;
  cfmt.bPitchAndFamily = plf->lfPitchAndFamily;
  lstrcpyn(cfmt.szFaceName, plf->lfFaceName, LF_FACESIZE);
  cfmt.dwEffects = 0;
  if (plf->lfWeight >= FW_BOLD)
    cfmt.dwEffects |= CFE_BOLD;
  if (plf->lfItalic)
    cfmt.dwEffects |= CFE_ITALIC;

  bRet = (BOOL) SendMsg(EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cfmt);

  return bRet;
}

void CAnyRichEdit::SetModify(BOOL bModified)
{
  SendMsg(EM_SETMODIFY, (WPARAM) bModified, 0);
}

void CAnyRichEdit::SetSel(INT nStartPos, INT nEndPos)
{
  SendMsg(EM_SETSEL, (WPARAM) nStartPos, (LPARAM) nEndPos);
}

///////////////////////////////////////////////////////////////////////////
// Additional Unicode functions

#ifdef any_ctrl_enable_w_members

  INT CAnyRichEdit::ExGetSelPosW(INT* pnStartPos, INT* pnEndPos) const
  {
    CHARRANGE cr;
    SendMsgW(EM_EXGETSEL, 0, (LPARAM) &cr);
    if (pnStartPos != NULL)
      *pnStartPos = (INT) cr.cpMin;
    if (pnEndPos != NULL)
      *pnEndPos = (INT) cr.cpMax;

    return cr.cpMin;
  }

  void CAnyRichEdit::ExLimitTextW(INT nMaxLength)
  {
    SendMsgW(EM_EXLIMITTEXT, 0, (LPARAM) nMaxLength);
  }  

  void CAnyRichEdit::ExSetSelW(INT nStartPos, INT nEndPos)
  {
    CHARRANGE cr;
    cr.cpMin = (LONG) nStartPos;
    cr.cpMax = (LONG) nEndPos;
    SendMsgW(EM_EXSETSEL, 0, (LPARAM) &cr);
  }

  WCHAR CAnyRichEdit::GetCharAtW(INT nPos) const
  {
    WCHAR     wch = 0;
    WCHAR     wstr[4];

    if (GetTextAtW(nPos, 1, wstr) > 0)
    {
      wch = wstr[0];
    }

    return wch;
  }  

  DWORD CAnyRichEdit::GetEventMaskW() const
  {
    return ((DWORD) SendMsgW(EM_GETEVENTMASK, 0, 0));
  }

  INT CAnyRichEdit::GetLineW(INT nLine, WCHAR* lpText) const
  {
    INT nLen;  
    nLen = (INT) SendMsgW(EM_GETLINE, (WPARAM) nLine, (LPARAM) lpText);
    lpText[nLen] = 0;
    return nLen;
  }

  INT CAnyRichEdit::GetLineCountW() const
  {
    return ((INT) SendMsgW(EM_GETLINECOUNT, 0, 0));
  }

  INT CAnyRichEdit::GetTextAtW(INT nPos, INT nLen, WCHAR* lpText) const
  {
    INT        nRet;
    TEXTRANGEW trw;

    trw.lpstrText = lpText;
    trw.chrg.cpMin = nPos;
    trw.chrg.cpMax = nPos + nLen;
    nRet = (INT) SendMsgW(EM_GETTEXTRANGE, 0, (LPARAM) &trw);

    return nRet;
  }

  void CAnyRichEdit::ReplaceSelTextW(const WCHAR* cszText, BOOL bCanUndo )
  {
    SendMsgW(EM_REPLACESEL, (WPARAM) bCanUndo, (LPARAM) cszText);
  }

  BOOL CAnyRichEdit::SetCharFormatW(DWORD dwMask,
    DWORD dwEffects, COLORREF color, DWORD dwOptions)
  {
    BOOL       bRet;
    CHARFORMAT cf;

    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = dwMask;
    cf.dwEffects = dwEffects;
    cf.crTextColor = color;
    bRet = (BOOL) SendMsgW(EM_SETCHARFORMAT,
      (WPARAM) dwOptions, (LPARAM) &cf);

    return bRet;
  }  

  DWORD CAnyRichEdit::SetEventMaskW(DWORD nEventMask)
  {
    DWORD dwRet;

    dwRet = (DWORD) SendMsgW(EM_SETEVENTMASK, 0, (LPARAM) nEventMask);

    return dwRet;
  }

#endif

