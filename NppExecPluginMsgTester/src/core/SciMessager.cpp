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

unsigned char CSciMessager::getCharAt(int pos) const
{
    return (unsigned char) SendSciMsg(SCI_GETCHARAT, (WPARAM) pos);
}

unsigned int CSciMessager::getCodePage() const
{
    return (unsigned int) SendSciMsg(SCI_GETCODEPAGE);
}

int CSciMessager::getCurrentPos() const
{
    return (int) SendSciMsg(SCI_GETCURRENTPOS);
}

LRESULT CSciMessager::getDocPointer() const
{
    return SendSciMsg(SCI_GETDOCPOINTER);
}

int CSciMessager::getSelectionMode() const
{
    return (int) SendSciMsg(SCI_GETSELECTIONMODE);
}

int CSciMessager::getSelectionEnd() const
{
    return (int) SendSciMsg(SCI_GETSELECTIONEND);
}

int CSciMessager::getSelectionStart() const
{
    return (int) SendSciMsg(SCI_GETSELECTIONSTART);
}

int CSciMessager::getSelText(char* pText) const
{
    return (int) SendSciMsg( SCI_GETSELTEXT, 0, (LPARAM) pText );
}

int CSciMessager::getText(int len, char* pText) const
{
    return (int) SendSciMsg( SCI_GETTEXT, (WPARAM) len, (LPARAM) pText );
}

int CSciMessager::getTextLength() const
{
    return (int) SendSciMsg(SCI_GETTEXTLENGTH);
}

int CSciMessager::getTextRange(int pos1, int pos2, char* pText) const
{
    Sci_TextRange tr;
    tr.chrg.cpMin = pos1;
    tr.chrg.cpMax = pos2;
    tr.lpstrText = pText;
    return (int) SendSciMsg( SCI_GETTEXTRANGE, 0, (LPARAM) &tr );
}

void CSciMessager::goToPos(int pos)
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

void CSciMessager::setSel(int anchorPos, int currentPos)
{
    SendSciMsg( SCI_SETSEL, (WPARAM) anchorPos, (LPARAM) currentPos );
}

void CSciMessager::setSelectionMode(int mode)
{
    SendSciMsg( SCI_SETSELECTIONMODE, (WPARAM) mode );
}

void CSciMessager::setSelectionEnd(int pos)
{
    SendSciMsg( SCI_SETSELECTIONEND, (WPARAM) pos );
}

void CSciMessager::setSelectionStart(int pos)
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
