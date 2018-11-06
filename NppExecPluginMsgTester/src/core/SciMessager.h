#ifndef _sci_messager_h_
#define _sci_messager_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "npp_files/Scintilla.h"

class CSciMessager
{
    protected:
        HWND m_hSciWnd;

    public:
        CSciMessager(HWND hSciWnd = NULL);
        virtual ~CSciMessager();

        LRESULT SendSciMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0);
        LRESULT SendSciMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0) const;
        
        void          beginUndoAction();
        void          endUndoAction();
        unsigned char getCharAt(int pos) const;
        unsigned int  getCodePage() const; // 0 (non-Unicode), SC_CP_UTF8, DBCS etc.
        int           getCurrentPos() const;
        LRESULT       getDocPointer() const; // identifies the document
        HWND          getSciWnd() const  { return m_hSciWnd; }
        int           getSelectionMode() const; // SC_SEL_STREAM, SC_SEL_RECTANGLE, SC_SEL_LINES
        int           getSelectionEnd() const;
        int           getSelectionStart() const;
        int           getSelText(char* pText) const;
        int           getText(int len, char* pText) const;
        int           getTextLength() const;
        int           getTextRange(int pos1, int pos2, char* pText) const;
        void          goToPos(int pos);
        bool          isModified() const;
        bool          isSelectionRectangle() const;
        void          setCodePage(unsigned int codePage);
        void          setSciWnd(HWND hSciWnd)  { m_hSciWnd = hSciWnd; }
        void          setSel(int anchorPos, int currentPos);
        void          setSelectionMode(int mode); // SC_SEL_STREAM, SC_SEL_RECTANGLE, SC_SEL_LINES
        void          setSelectionEnd(int pos);
        void          setSelectionStart(int pos);
        void          setSelText(const char* pText);
        void          setText(const char* pText);
};

//---------------------------------------------------------------------------
#endif
