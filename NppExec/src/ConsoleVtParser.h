#ifndef _console_vt_parser_h_
#define _console_vt_parser_h_
//--------------------------------------------------------------------

#include "base.h"
#include "cpp/CStrT.h"

class CConsoleVtParser
{
    public:
        struct VtActions
        {
            bool bClearLine = false;
            bool bClearScreen = false;
            int nBackspaceCount = 0;
            // ConPTY often advances to the next output row with an absolute cursor-
            // position (CUP "ESC[r;cH") or cursor-down/next-line (CUD "ESC[nB",
            // CNL "ESC[nE") sequence instead of a "\r\n" — e.g. at a frame/flush
            // boundary such as when the child sets the window title. A CR/LF-only
            // splitter would then merge the two logical rows. This flags that the
            // parsed line ended with such a downward cursor move so the caller can
            // treat the line as newline-terminated.
            bool bCursorNextLine = false;
        };

        CConsoleVtParser();

        void Reset();

        // Phase 4.1: byte-stream parser foundation.
        // For now, VT sequences are consumed and omitted from output.
        bool ProcessChunk(const tstr& input, tstr& output);
        void ConsumeActions(VtActions& actions);

        bool HasActiveTextColor() const;
        COLORREF GetActiveTextColor() const;

    private:
        enum eEscState
        {
            esNone = 0,
            esEsc,    // ESC symbol found
            esCsi,    // CSI sequence
            esOsc,    // OSC sequence
            esWaitSt, // wait for ST (ESC \)
            esWait1,  // wait for 1 symbol
            esWait2   // wait for 2 symbols
        };

    private:
        eEscState m_State;
        TCHAR     m_Wait1Ch;
        TCHAR     m_Wait2Ch;
        CStrT<TCHAR> m_CsiParams;

        bool     m_bHasActiveTextColor;
        COLORREF m_ActiveTextColor;
        VtActions m_Actions;

    private:
        void finalizeCsiSequence(TCHAR finalCh, tstr& output);
        void applySgrParams(const CStrT<TCHAR>& params);
        static int getFirstParamOrDefault(const CStrT<TCHAR>& params, int nDefaultValue);
};

//--------------------------------------------------------------------
#endif
