#include "ConsoleVtParser.h"
#include "cpp/StrSplitT.h"
#include "c_base/str2int.h"

CConsoleVtParser::CConsoleVtParser()
{
    Reset();
}

void CConsoleVtParser::Reset()
{
    m_State = esNone;
    m_Wait1Ch = 0;
    m_Wait2Ch = 0;
    m_CsiParams.Clear();
    m_bHasActiveTextColor = false;
    m_ActiveTextColor = RGB(0, 0, 0);
    m_Actions = VtActions();
    m_bInPlaceCursorMode = false;
    m_Rows.clear();
    m_CurRow = VtOutputRow();
    m_bPendingRowBreak = false;
    m_nPendingIndent = 0;
}

const std::vector<CConsoleVtParser::VtOutputRow>& CConsoleVtParser::GetOutputRows() const
{
    return m_Rows;
}

std::vector<CConsoleVtParser::VtOutputRow>& CConsoleVtParser::GetOutputRows()
{
    return m_Rows;
}

bool CConsoleVtParser::ProcessChunk(const tstr& input, tstr& output)
{
    output.Clear();
    output.Reserve(input.length());

    const TCHAR* p = input.c_str();
    TCHAR currCh = 0;

    while ( (currCh = *p) != 0 )
    {
        switch ( m_State )
        {
            case esNone:
                if ( currCh == 0x1B )  // ESC
                {
                    m_State = esEsc;
                }
                else
                {
                    emitVisibleChar(currCh);
                }
                break;

            case esEsc:
                switch ( currCh )
                {
                    case _T('['):  // CSI
                        m_State = esCsi;
                        m_CsiParams.Clear();
                        break;
                    case _T(']'):  // OSC
                        m_State = esOsc;
                        break;
                    case _T('P'):  // DCS
                    case _T('X'):  // SOS
                    case _T('^'):  // PM
                    case _T('_'):  // APC
                        m_State = esWaitSt;
                        break;
                    case _T('$'):  // G?DM?
                        m_State = esWait2;
                        m_Wait2Ch = currCh;
                        break;
                    case _T('!'):  // C0-designate
                    case _T('"'):  // C1-designate
                    case _T('#'):  // single control function
                    case _T('%'):  // DOCS
                    case _T('&'):  // IRR
                    case _T('('):  // G0 character set
                    case _T(')'):  // G1 character set
                    case _T('*'):  // G2 character set
                    case _T('+'):  // G3 character set
                    case _T('-'):  // G1 character set, VT300
                    case _T('.'):  // G2 character set, VT300
                    case _T('/'):  // G3 character set, VT300
                    case _T(' '):  // ACS
                        m_State = esWait1;
                        m_Wait1Ch = currCh;
                        break;
                    default:       // RIS, IND, NEL, HTS, RI, ...
                        m_State = esNone;
                        break;
                }
                break;

            case esCsi:
                if ( (currCh >= _T('0') && currCh <= _T('9')) ||
                     (currCh == _T(';')) ||
                     (currCh == _T('?')) )
                {
                    m_CsiParams.Append(currCh);
                }
                else if ( (currCh >= _T('A') && currCh <= _T('Z')) ||
                          (currCh >= _T('a') && currCh <= _T('z')) ||
                          (currCh == _T('@'))  ||
                          (currCh == _T('['))  ||
                          (currCh == _T('\\')) ||
                          (currCh == _T(']'))  ||
                          (currCh == _T('^'))  ||
                          (currCh == _T('_'))  ||
                          (currCh == _T('`'))  ||
                          (currCh == _T('{'))  ||
                          (currCh == _T('|'))  ||
                          (currCh == _T('}'))  ||
                          (currCh == _T('~')) )
                {
                    finalizeCsiSequence(currCh);
                    m_State = esNone;
                }
                break;

            case esOsc:
            case esWaitSt:
                if ( currCh == 0x1B )  // ESC
                {
                    const TCHAR nextCh = *(p + 1);
                    if ( nextCh == _T('\\') )  // ST
                    {
                        ++p; // skip '\'
                        m_State = esNone;
                    }
                    else
                    {
                        m_State = esEsc;
                    }
                }
                break;

            case esWait1:
                m_State = esNone;
                if ( m_Wait1Ch == _T('%') )
                {
                    switch ( currCh )
                    {
                        case _T('/'):  // ESC % / F
                            m_State = esWait1;
                            break;
                    }
                }
                m_Wait1Ch = 0;
                break;

            case esWait2:
                m_State = esWait1;
                if ( m_Wait2Ch == _T('$') )
                {
                    switch ( currCh )
                    {
                        case _T('@'):  // ESC $ @
                        case _T('A'):  // ESC $ A
                        case _T('B'):  // ESC $ B
                            m_State = esNone;
                            break;
                    }
                }
                m_Wait2Ch = 0;
                break;
        }

        ++p;
    }

    // Drop an incomplete sequence at chunk end (line-oriented processing resets per line).
    if ( m_State != esNone )
    {
        m_State = esNone;
        m_Wait1Ch = 0;
        m_Wait2Ch = 0;
        m_CsiParams.Clear();
    }

    // A cursor move that had no text following it (e.g. the title-set frame boundary)
    // just terminates the line; signal it so the caller newline-terminates instead of
    // emitting a spurious extra row.
    if ( m_bPendingRowBreak )
    {
        m_Actions.bCursorNextLine = true;
        m_bPendingRowBreak = false;
        m_nPendingIndent = 0;
    }

    // Flush the row under construction (always, so an empty line still yields one row).
    pushCurrentRow(true);

    // Rebuild 'output' as the row texts joined by '\n' (used by the caller only for
    // length checks and the output-var; printing iterates GetOutputRows() directly).
    output.Clear();
    for ( size_t i = 0; i < m_Rows.size(); ++i )
    {
        if ( i != 0 )
            output.Append( _T('\n') );
        output.Append( m_Rows[i].text );
    }

    return (!m_Rows.empty() || input.IsEmpty());
}

void CConsoleVtParser::emitVisibleChar(TCHAR ch)
{
    if ( ch == _T(' ') || ch == _T('\t') )
    {
        // Leading blanks that arrive while a row break is pending become part of the
        // new row's indentation; a bare cursor move followed only by blanks should not
        // resolve into a fresh row on its own.
        if ( m_bPendingRowBreak )
        {
            ++m_nPendingIndent;
            return;
        }
        m_CurRow.text.Append(ch);
        return;
    }

    if ( m_bPendingRowBreak )
    {
        // First non-blank after the cursor move: close the previous row and start a
        // new one indented to the target column.
        pushCurrentRow(false);
        for ( int i = 0; i < m_nPendingIndent; ++i )
            m_CurRow.text.Append( _T(' ') );
        m_bPendingRowBreak = false;
        m_nPendingIndent = 0;
    }

    m_CurRow.text.Append(ch);
    // The row's color tracks the color in effect for its visible (non-blank) text, so
    // a trailing "ESC[31m" left dangling at a row boundary can't recolor this row.
    m_CurRow.bHasColor = m_bHasActiveTextColor;
    m_CurRow.color = m_ActiveTextColor;
}

void CConsoleVtParser::emitForwardSpaces(int n)
{
    if ( n <= 0 )
        return;
    if ( n > 4096 ) // guard against absurd/malformed counts
        n = 4096;
    if ( m_bPendingRowBreak )
    {
        m_nPendingIndent += n;
        return;
    }
    for ( int i = 0; i < n; ++i )
        m_CurRow.text.Append( _T(' ') );
}

void CConsoleVtParser::requestRowBreak(int nIndent)
{
    m_bPendingRowBreak = true;
    m_nPendingIndent = (nIndent > 0) ? nIndent : 0;
}

void CConsoleVtParser::pushCurrentRow(bool bForce)
{
    // Avoid creating a leading empty row (e.g. a line that opens with a cursor move).
    if ( !bForce && m_CurRow.text.IsEmpty() && m_Rows.empty() )
        return;

    m_Rows.push_back(m_CurRow);
    m_CurRow = VtOutputRow();
}

void CConsoleVtParser::SetInPlaceCursorMode(bool bInPlace)
{
    m_bInPlaceCursorMode = bInPlace;
}

void CConsoleVtParser::ConsumeActions(VtActions& actions)
{
    actions = m_Actions;
    m_Actions = VtActions();
}

bool CConsoleVtParser::HasActiveTextColor() const
{
    return m_bHasActiveTextColor;
}

COLORREF CConsoleVtParser::GetActiveTextColor() const
{
    return m_ActiveTextColor;
}

void CConsoleVtParser::finalizeCsiSequence(TCHAR finalCh)
{
    switch ( finalCh )
    {
        case _T('m'): // SGR
            applySgrParams(m_CsiParams);
            break;

        case _T('C'): // CUF (cursor forward)
            // At the start of a row (no text yet): accumulate as a column advance so
            // the caller can either keep it as a cursor move (ConPTY bare-\\r history
            // redraw: \\r + CUF(promptLen) + EL + cmd) or turn it into leading spaces
            // (git status indent). After visible text has started, emit spaces (rare).
            {
                int n = getFirstParamOrDefault(m_CsiParams, 1);
                if ( n < 0 )
                    n = 0;
                else if ( n > 4096 )
                    n = 4096;
                if ( m_CurRow.text.IsEmpty() && m_Rows.empty() && !m_bPendingRowBreak )
                    m_Actions.nCursorForward += n;
                else
                    emitForwardSpaces(n);
            }
            break;

        case _T('K'): // EL
            m_Actions.bClearLine = true;
            break;

        case _T('X'): // ECH (erase character)
            {
                int n = getFirstParamOrDefault(m_CsiParams, 1);
                if ( n < 0 )
                    n = 0;
                else if ( n > 4096 )
                    n = 4096;
                m_Actions.nEraseCharCount += n;
            }
            break;

        case _T('J'): // ED
            {
                const int n = getFirstParamOrDefault(m_CsiParams, 0);
                if ( (n == 0) || (n == 2) )
                    m_Actions.bClearScreen = true;
            }
            break;

        case _T('D'): // cursor left
            m_Actions.nBackspaceCount += getFirstParamOrDefault(m_CsiParams, 1);
            break;

        case _T('H'): // CUP (cursor position)
        case _T('f'): // HVP (horizontal/vertical position)
            // "ESC[r;cH" with an explicit position advances to another output row
            // (ConPTY paints the initial visible screen this way instead of "\r\n").
            // Treat it as a row break and use the column as the new row's indent.
            // Bare "ESC[H" (home) has no params and pairs with a screen clear -> ignore.
            if ( !m_CsiParams.IsEmpty() )
            {
                CStrSplitT<TCHAR> args;
                const int nArgs = args.Split(m_CsiParams.c_str(), _T(";"));
                int col = (nArgs >= 2) ? c_base::_tstr2int(args.GetArg(1).c_str()) : 1;
                if ( col < 1 )
                    col = 1;
                const int nCol0 = (col > 1) ? (col - 1) : 0;
                // History redraw is a single CUP before any text in the chunk
                // ("ESC[row;colH" + replacement). Later CUPs in the same chunk
                // are a multi-row screen paint and must still break rows.
                if ( m_bInPlaceCursorMode && m_CurRow.text.IsEmpty() && m_Rows.empty() && !m_bPendingRowBreak )
                    m_Actions.nSetCursorColumn = nCol0;
                else
                    requestRowBreak(nCol0);
            }
            break;

        case _T('B'): // CUD (cursor down)
        case _T('E'): // CNL (cursor next line)
            requestRowBreak(0);
            break;
    }

    m_CsiParams.Clear();
}

void CConsoleVtParser::applySgrParams(const CStrT<TCHAR>& params)
{
    if ( params.IsEmpty() )
    {
        m_bHasActiveTextColor = false;
        return;
    }

    CStrSplitT<TCHAR> args;
    const int nArgs = args.Split(params.c_str(), _T(";"));
    for ( int i = 0; i < nArgs; i++ )
    {
        const int n = c_base::_tstr2int(args.GetArg(i).c_str());
        switch ( n )
        {
            case 0:
            case 39:
                m_bHasActiveTextColor = false;
                break;
            case 30: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(0, 0, 0); break;
            case 31: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(205, 49, 49); break;
            case 32: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(13, 188, 121); break;
            case 33: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(229, 229, 16); break;
            case 34: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(36, 114, 200); break;
            case 35: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(188, 63, 188); break;
            case 36: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(17, 168, 205); break;
            case 37: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(229, 229, 229); break;
            case 90: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(102, 102, 102); break;
            case 91: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(241, 76, 76); break;
            case 92: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(35, 209, 139); break;
            case 93: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(245, 245, 67); break;
            case 94: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(59, 142, 234); break;
            case 95: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(214, 112, 214); break;
            case 96: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(41, 184, 219); break;
            case 97: m_bHasActiveTextColor = true; m_ActiveTextColor = RGB(255, 255, 255); break;
        }
    }
}

int CConsoleVtParser::getFirstParamOrDefault(const CStrT<TCHAR>& params, int nDefaultValue)
{
    if ( params.IsEmpty() )
        return nDefaultValue;

    CStrSplitT<TCHAR> args;
    if ( args.Split(params.c_str(), _T(";")) <= 0 )
        return nDefaultValue;

    const tstr sFirst = args.GetArg(0);
    if ( sFirst.IsEmpty() )
        return nDefaultValue;

    return c_base::_tstr2int(sFirst.c_str());
}
