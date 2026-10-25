#ifndef _console_vt_parser_h_
#define _console_vt_parser_h_
//--------------------------------------------------------------------

#include "base.h"
#include "cpp/CStrT.h"
#include <vector>

class CConsoleVtParser
{
    public:
        struct VtActions
        {
            bool bClearLine = false;
            bool bClearScreen = false;
            int nBackspaceCount = 0;
            // ECH "CSI n X" — erase n characters forward from the cursor.
            int nEraseCharCount = 0;
            // CUP/HVP absolute column (0-based). Set when in-place cursor mode is active.
            int nSetCursorColumn = -1;
            // A later CUP on the same row, after the replacement text: park the
            // cursor there and drop the padding that covered a longer command.
            int nCursorColumnAfter = -1;
            // ConPTY often advances to the next output row with an absolute cursor-
            // position (CUP "ESC[r;cH") or cursor-down/next-line (CUD "ESC[nB",
            // CNL "ESC[nE") sequence instead of a "\r\n" — e.g. at a frame/flush
            // boundary such as when the child sets the window title. A CR/LF-only
            // splitter would then merge the two logical rows. This flags that the
            // parsed line ended with such a downward cursor move (with no text
            // following it) so the caller can treat the line as newline-terminated.
            bool bCursorNextLine = false;
            // CUF at the start of a row (before any visible text): column advance for
            // ConPTY row-replace (bare \\r + CUF + EL + text), or leading indent spaces
            // when not in row-replace mode.
            int nCursorForward = 0;
        };

        // One visible output row produced from a parsed line. ConPTY renders the
        // initial visible screen by positioning each row with an absolute cursor
        // move (CUP) instead of "\r\n", so a single CR/LF-delimited chunk can carry
        // several rows. Each row also carries its own color, because git wraps every
        // entry as "ESC[31m<name>ESC[m" and ConPTY may leave a dangling color code at
        // a row boundary (which must not bleed onto the neighbouring row).
        struct VtOutputRow
        {
            tstr     text;
            bool     bHasColor = false;
            COLORREF color = RGB(0, 0, 0);
        };

        CConsoleVtParser();

        void Reset();

        // ConPTY t+ h-: CUP/HVP reposition the write column on the current row
        // (cmd history redraw) instead of starting a new output row.
        void SetInPlaceCursorMode(bool bInPlace);

        // Phase 4.1: byte-stream parser foundation.
        // VT sequences are consumed; visible text is split into rows (see VtOutputRow).
        // 'output' receives the row texts joined by '\n' (for length/output-var use).
        bool ProcessChunk(const tstr& input, tstr& output);
        void ConsumeActions(VtActions& actions);

        const std::vector<VtOutputRow>& GetOutputRows() const;
        std::vector<VtOutputRow>& GetOutputRows();

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
        bool m_bInPlaceCursorMode;
        int  m_nInPlaceCupRow;
        // Text was written, then a CUP arrived before any earlier CUP named the
        // row. Hold it: no further text means "park the cursor" (insert redraw);
        // further text means the CUP really advanced to the next row.
        bool m_bDeferInPlacePark;
        int  m_nDeferParkColumn;

        // Row assembly (see VtOutputRow).
        std::vector<VtOutputRow> m_Rows;
        VtOutputRow m_CurRow;
        bool m_bPendingRowBreak; // a CUP/CNL/CUD is waiting for the next visible text
        int  m_nPendingIndent;   // leading spaces for the row that starts after the break

    private:
        void finalizeCsiSequence(TCHAR finalCh);
        void applySgrParams(const CStrT<TCHAR>& params);
        static int getFirstParamOrDefault(const CStrT<TCHAR>& params, int nDefaultValue);

        void emitVisibleChar(TCHAR ch); // a printable char from the input stream
        void emitForwardSpaces(int n);  // CUF "ESC[nC" -> n columns of indentation
        void requestRowBreak(int nIndent); // CUP/CNL/CUD -> break before next text
        void pushCurrentRow(bool bForce);  // move m_CurRow into m_Rows
};

//--------------------------------------------------------------------
#endif
