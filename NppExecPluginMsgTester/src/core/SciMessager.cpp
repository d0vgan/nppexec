#include "SciMessager.h"

CSciMessager::CSciMessager(HWND hSciWnd )
{
    m_hSciWnd = hSciWnd;
}

CSciMessager::~CSciMessager()
{
}

LRESULT CSciMessager::SendSciMsg(UINT uMsg, WPARAM wParam , LPARAM lParam )
{
    return ::SendMessage(m_hSciWnd, uMsg, wParam, lParam);
}

LRESULT CSciMessager::SendSciMsg(UINT uMsg, WPARAM wParam , LPARAM lParam ) const
{
    return ::SendMessage(m_hSciWnd, uMsg, wParam, lParam);
}

void CSciMessager::beginUndoAction()
{
    SendSciMsg(SCI_BEGINUNDOACTION);
}

void CSciMessager::endUndoAction()
{
    SendSciMsg(SCI_ENDUNDOACTION);
}

unsigned char CSciMessager::getCharAt(Sci_Position pos) const
{
    return (unsigned char) SendSciMsg(SCI_GETCHARAT, (WPARAM) pos);
}

unsigned int CSciMessager::getCodePage() const
{
    return (unsigned int) SendSciMsg(SCI_GETCODEPAGE);
}

Sci_Position CSciMessager::getCurrentPos() const
{
    return (Sci_Position) SendSciMsg(SCI_GETCURRENTPOS);
}

LRESULT CSciMessager::getDocPointer() const
{
    return SendSciMsg(SCI_GETDOCPOINTER);
}

int CSciMessager::getSelectionMode() const
{
    return (int) SendSciMsg(SCI_GETSELECTIONMODE);
}

Sci_Position CSciMessager::getSelectionEnd() const
{
    return (Sci_Position) SendSciMsg(SCI_GETSELECTIONEND);
}

Sci_Position CSciMessager::getSelectionStart() const
{
    return (Sci_Position) SendSciMsg(SCI_GETSELECTIONSTART);
}

Sci_Position CSciMessager::getSelText(char* pText) const
{
    return (Sci_Position) SendSciMsg( SCI_GETSELTEXT, 0, (LPARAM) pText );
}

Sci_Position CSciMessager::getText(Sci_Position len, char* pText) const
{
    return (Sci_Position) SendSciMsg( SCI_GETTEXT, (WPARAM) len, (LPARAM) pText );
}

Sci_Position CSciMessager::getTextLength() const
{
    return (Sci_Position) SendSciMsg(SCI_GETTEXTLENGTH);
}

Sci_Position CSciMessager::getTextRange(Sci_Position pos1, Sci_Position pos2, char* pText) const
{
    Sci_TextRangeFull tr;
    tr.chrg.cpMin = pos1;
    tr.chrg.cpMax = pos2;
    tr.lpstrText = pText;
    return (Sci_Position) SendSciMsg( SCI_GETTEXTRANGEFULL, 0, (LPARAM) &tr );
}

void CSciMessager::goToPos(Sci_Position pos)
{
    SendSciMsg( SCI_GOTOPOS, (WPARAM) pos );
}

bool CSciMessager::isModified() const
{
    return SendSciMsg(SCI_GETMODIFY) ? true : false;
}

bool CSciMessager::isSelectionRectangle() const
{
    return SendSciMsg(SCI_SELECTIONISRECTANGLE) ? true : false;
}

void CSciMessager::setCodePage(unsigned int codePage)
{
    SendSciMsg( SCI_SETCODEPAGE, (WPARAM) codePage );
}

void CSciMessager::setSel(Sci_Position anchorPos, Sci_Position currentPos)
{
    SendSciMsg( SCI_SETSEL, (WPARAM) anchorPos, (LPARAM) currentPos );
}

void CSciMessager::setSelectionMode(int mode)
{
    SendSciMsg( SCI_SETSELECTIONMODE, (WPARAM) mode );
}

void CSciMessager::setSelectionEnd(Sci_Position pos)
{
    SendSciMsg( SCI_SETSELECTIONEND, (WPARAM) pos );
}

void CSciMessager::setSelectionStart(Sci_Position pos)
{
    SendSciMsg( SCI_SETSELECTIONSTART, (WPARAM) pos );
}

void CSciMessager::setSelText(const char* pText)
{
    SendSciMsg( SCI_REPLACESEL, 0, (LPARAM) pText );
}

void CSciMessager::setText(const char* pText)
{
    SendSciMsg( SCI_SETTEXT, 0, (LPARAM) pText );
}
