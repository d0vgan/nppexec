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
                    output.Append(currCh);
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

    return (!output.IsEmpty() || input.IsEmpty());
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

        case _T('K'): // EL
            m_Actions.bClearLine = true;
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
