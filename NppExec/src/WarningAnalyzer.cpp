/*
This file is part of NppExec
Copyright (C) 2013 DV <dvv81 (at) ukr (dot) net>, Nicolas Babled

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

#include "WarningAnalyzer.h"
#include "NppExecEngine.h"
#include "tchar.h"
#include "richedit.h"

static bool match_mask_2( const TCHAR*, const TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR* );
static void preprocessMask( TCHAR* outMask, const TCHAR* inMask, unsigned int& outMaskType );
static TCHAR* skip_tabspaces(TCHAR* s);
static const TCHAR* skip_tabspaces(const TCHAR* s);
static bool is_num_str(const TCHAR* s);


const tregex CWarningAnalyzer::m_rgxFindFilename = tregex( _T( "(?:(([a-zA-Z]:[\\\\/]|\\.)[^.]*\\.[^:\"\\(\\s]{1,8})|(^\\w[^\\s:\\\\/]*\\.\\w{1,8}))(?=[\\s:(\",oline]{1,9}[0-9]+)" ) );
const tregex CWarningAnalyzer::m_rgxFindFileLineNo = tregex( _T( "(?:^[^0-9a-zA-Z_]+|.*line )([0-9]+).*" ) );
const tregex CWarningAnalyzer::m_rgxFindFileLinePos = tregex( _T( ".*[^a - zA - Z]( ? : [0 - 9][, :]\\s* | [0 - 9] char[, :] )([0 - 9] + ).*" ) );
const tregex CWarningAnalyzer::m_rgxFindErrPosIndicator = tregex( _T( "\\s+[\\x5E~]" ) );
const tregex CWarningAnalyzer::m_rgxFindErrPosIndicatorAtStartOfLine = tregex( _T( "^[\\.\\s]+[\\x5E~]" ) );


#define  TCM_FILE1  _T('1')
#define  TCM_FILE2  _T('2')
#define  TCM_LINE   _T('3')
#define  TCM_CHAR   _T('4')

#define  TSM_ABSFILE  _T("*1:\\*2")  // includes TCM_FILE1 and TCM_FILE2
#define  TSM_FILE     _T("*1")       // includes TCM_FILE1

CWarningAnalyzer::CWarningAnalyzer() : m_nLine(0)
                                     , m_nChar(0)
                                     , m_nLastFoundIndex(0)
{
    m_FileName[0] = 0;
}

CWarningAnalyzer::~CWarningAnalyzer()
{
}

void CWarningAnalyzer::SetEffect( int FilterNumber, const TEffect& Effect )
{
    if ( FilterNumber < WARN_MAX_FILTER )
    {
        m_Filter[ FilterNumber ].Effect = Effect;
    }
}

void preprocessMask( TCHAR* outMask, const TCHAR* inMask, unsigned int& outMaskType )
{
    outMaskType = CWarningAnalyzer::MT_NONE;

    while ( *inMask )
    {
        if ( *inMask == _T('%') )
        {
            int len = 0;

            // absolute path: %A% or %ABSFILE%
            if ( ( inMask[ 1 ] == _T('A') )
               &&( inMask[ 2 ] == _T('%') )
               )
            {
                len = 3;
            }
            else if ( ( inMask[ 1 ] == _T('A') )
                    &&( inMask[ 2 ] == _T('B') )
                    &&( inMask[ 3 ] == _T('S') )
                    &&( inMask[ 4 ] == _T('F') )
                    &&( inMask[ 5 ] == _T('I') )
                    &&( inMask[ 6 ] == _T('L') )
                    &&( inMask[ 7 ] == _T('E') )
                    &&( inMask[ 8 ] == _T('%') )
               )
            {
                len = 9;
            }
            if ( len > 0 )
            {
                inMask += len;
                *outMask++ = _T('*');
                *outMask++ = TCM_FILE1;
                *outMask++ = _T(':');
                *outMask++ = _T('\\');
                *outMask++ = _T('*');
                *outMask++ = TCM_FILE2;
                outMaskType |= CWarningAnalyzer::MT_ABSFILE;
            }

            // file name: %F% or %FILE%
            if ( len == 0 )
            {
                if ( ( inMask[ 1 ] == _T('F') )
                   &&( inMask[ 2 ] == _T('%') )
                   )
                {
                    len = 3;
                }
                else if ( ( inMask[ 1 ] == _T('F') )
                        &&( inMask[ 2 ] == _T('I') )
                        &&( inMask[ 3 ] == _T('L') )
                        &&( inMask[ 4 ] == _T('E') )
                        &&( inMask[ 5 ] == _T('%') )
                        )
                {
                    len = 6;
                }
                if ( len > 0 )
                {
                    inMask += len;
                    *outMask++ = _T('*');
                    *outMask++ = TCM_FILE1;
                    outMaskType |= CWarningAnalyzer::MT_FILE;
                }
            }

            // line: %L% or %LINE%
            if ( len == 0 )
            {
                if ( ( inMask[ 1 ] == _T('L') )
                   &&( inMask[ 2 ] == _T('%') )
                   )
                {
                    len = 3;
                }
                else if ( ( inMask[ 1 ] == _T('L') )
                        &&( inMask[ 2 ] == _T('I') )
                        &&( inMask[ 3 ] == _T('N') )
                        &&( inMask[ 4 ] == _T('E') )
                        &&( inMask[ 5 ] == _T('%') )
                        )
                {
                    len = 6;
                }
                if ( len > 0 )
                {
                    inMask += len;
                    *outMask++ = _T('*');
                    *outMask++ = TCM_LINE;
                    outMaskType |= CWarningAnalyzer::MT_LINE;
                }
            }

            // char: %C% or %CHAR%
            if ( len == 0 )
            {
                if ( ( inMask[ 1 ] == _T('C') )
                   &&( inMask[ 2 ] == _T('%') )
                   )
                {
                    len = 3;
                }
                else if ( ( inMask[ 1 ] == _T('C') )
                        &&( inMask[ 2 ] == _T('H') )
                        &&( inMask[ 3 ] == _T('A') )
                        &&( inMask[ 4 ] == _T('R') )
                        &&( inMask[ 5 ] == _T('%') )
                        )
                {
                    len = 6;
                }
                if ( len > 0 )
                {
                    inMask += len;
                    *outMask++ = _T('*');
                    *outMask++ = TCM_CHAR;
                    outMaskType |= CWarningAnalyzer::MT_CHAR;
                }
            }

            if ( len == 0 )
            {
                // no match
                *outMask++ = *inMask++;
            }
        }
        else if ( *inMask == _T('*') )
        {
            *outMask++ = *inMask;
            *outMask++ = *inMask++; // '*' is doubled here
        }
        else
        {
            *outMask++ = *inMask++;
        }
    }
    *outMask = 0;
}

void CWarningAnalyzer::SetMask( int FilterNumber, const TCHAR* Mask )
{
    if ( (FilterNumber < WARN_MAX_FILTER) && (Mask != NULL) )
    {
        TFilter& filter = m_Filter[FilterNumber];
        preprocessMask( filter.Mask, Mask, filter.MaskType );
    }
}


void CWarningAnalyzer::GetEffect( int FilterNumber, TEffect& Effect ) const
{
    if ( FilterNumber < WARN_MAX_FILTER )
    {
        Effect = m_Filter[ FilterNumber ].Effect;
    }
}


const TCHAR* CWarningAnalyzer::GetMask( int FilterNumber, TCHAR* Mask, int /*MaskLength*/ ) const
{
    if ( (FilterNumber < WARN_MAX_FILTER) && (Mask != NULL) )
    {
        TCHAR* pstr1       = Mask;
        const TCHAR* pstr2 = m_Filter[FilterNumber].Mask;

        while ( *pstr2 )
        {
            if ( *pstr2 == _T('*') )
            {
                if ( ( pstr2[ 1 ] == TCM_FILE1 )
                   &&( pstr2[ 2 ] == _T(':')  )
                   &&( pstr2[ 3 ] == _T('\\') )
                   &&( pstr2[ 4 ] == _T('*')  )
                   &&( pstr2[ 5 ] == TCM_FILE2 )
                   )
                {
                    pstr2 += 6;
                    *pstr1++ = _T('%');
                    *pstr1++ = _T('A');
                    *pstr1++ = _T('B');
                    *pstr1++ = _T('S');
                    *pstr1++ = _T('F');
                    *pstr1++ = _T('I');
                    *pstr1++ = _T('L');
                    *pstr1++ = _T('E');
                    *pstr1++ = _T('%');
                }
                else if ( pstr2[ 1 ] == TCM_FILE1 )
                {
                    pstr2 += 2;
                    *pstr1++ = _T('%');
                    *pstr1++ = _T('F');
                    *pstr1++ = _T('I');
                    *pstr1++ = _T('L');
                    *pstr1++ = _T('E');
                    *pstr1++ = _T('%');
                }
                else if ( pstr2[ 1 ] == TCM_LINE )
                {
                    pstr2 += 2;
                    *pstr1++ = _T('%');
                    *pstr1++ = _T('L');
                    *pstr1++ = _T('I');
                    *pstr1++ = _T('N');
                    *pstr1++ = _T('E');
                    *pstr1++ = _T('%');
                }
                else if ( pstr2[ 1 ] == TCM_CHAR )
                {
                    pstr2 += 2;
                    *pstr1++ = _T('%');
                    *pstr1++ = _T('C');
                    *pstr1++ = _T('H');
                    *pstr1++ = _T('A');
                    *pstr1++ = _T('R');
                    *pstr1++ = _T('%');
                }
                else if ( pstr2[ 1 ] == _T('*') )
                {
                    pstr2 += 2;
                    *pstr1++ = _T('*');
                }
                else if ( pstr2[ 1 ] == 0 )
                {
                    pstr2++;
                }
            }
            else
            {
                *pstr1++ = *pstr2++;
            }
        }
        *pstr1 = 0;
    }
    return ( Mask );
}

bool CWarningAnalyzer::match( const TCHAR* str )
{
    TCHAR  ostr1[WARN_MAX_FILENAME + 5];
    TCHAR  ostr2[WARN_MAX_FILENAME + 5];
    TCHAR  ostr3[WARN_MAX_FILENAME + 5];
    TCHAR  ostr4[WARN_MAX_FILENAME + 5];

    TCHAR szWarnMask[WARN_MASK_SIZE];
    TCHAR szWarnMaskAbs[WARN_MASK_SIZE + 10];
    const TCHAR* pszMask = 0;
    unsigned int nMaskType = MT_NONE;
    tstr sMask;

    ostr1[0] = 0;
    ostr2[0] = 0;
    ostr3[0] = 0;
    ostr4[0] = 0;

    for ( int i = 0; (i < WARN_MAX_FILTER) && !pszMask; i++ )
    {
        const TFilter& filter = m_Filter[i];
        if ( filter.Effect.Enable )
        {
            nMaskType = filter.MaskType;
            pszMask = filter.Mask;
            sMask = pszMask;
            if ( Runtime::GetNppExec().GetMacroVars().CheckAllMacroVars(nullptr, sMask, false) )
            {
                // The mask contains macro-var, so needs to be preprocessed again
                preprocessMask(szWarnMask, sMask.c_str(), nMaskType);
                pszMask = szWarnMask;
            }

            if ( ((nMaskType & MT_FILE) != 0) && ((nMaskType & MT_ABSFILE) == 0) )
            {
                // This mask contains %FILE% and not %ABSFILE%.
                // Starting from NppExec 0.5.3, %FILE% is treated as both %ABSFILE% and %FILE%.

                // Let's transform %FILE% to %ABSFILE% first...
                TCHAR* pMskAbs = szWarnMaskAbs;
                const TCHAR* pMsk = pszMask;
                TCHAR ch = 1;
                do {
                    ch = *(pMsk++);
                    *(pMskAbs++) = ch;
                    if ( ch == _T('*') )
                    {
                        ch = *(pMsk++);
                        *(pMskAbs++) = ch;
                        if ( ch == TCM_FILE1 )
                        {
                            *(pMskAbs++) = _T(':');
                            *(pMskAbs++) = _T('\\');
                            *(pMskAbs++) = _T('*');
                            *(pMskAbs++) = TCM_FILE2;
                        }
                    }
                } while ( ch != 0 );

                // Let's try to match %ABSFILE%...
                if ( match_mask_2(szWarnMaskAbs, str, ostr1, ostr2, ostr3, ostr4) )
                {
                    // %ABSFILE% matched
                    pszMask = szWarnMaskAbs;
                    m_nLastFoundIndex = i;
                }
            }

            if ( pszMask != szWarnMaskAbs ) // %ABSFILE% did not match
            {
                if ( match_mask_2(pszMask, str, ostr1, ostr2, ostr3, ostr4) )
                {
                    m_nLastFoundIndex = i;
                }
                else
                {
                    pszMask = 0;
                }
            }
        }
    }
    if ( pszMask )
    {
        TCHAR* postr1 = ostr1;
        TCHAR* postr2 = ostr2;
        TCHAR* postr3 = ostr3;
        TCHAR* postr4 = ostr4;

        // %ABSFILE%
        if ( _tcsstr( pszMask, TSM_ABSFILE ) ) 
        {
            while ( *postr1 )  ++postr1;
            *postr1++ = _T(':');
            *postr1++ = _T('\\');
            while ( (*postr1++ = *postr2++) != 0 );
        }
        // %ABSFILE% or %FILE%
        if ( _tcsstr( pszMask, TSM_FILE ) )
        {
            postr2 = m_FileName;
            postr1 = ostr1;
            // skip leading spaces
            postr1 = skip_tabspaces(postr1);
            // copy
            while ( (*postr2++ = *postr1++) != 0 );
            // skip trailing spaces
            --postr2;
            while ( --postr2 >= m_FileName )
            {
                if ( !NppExecHelpers::IsTabSpaceChar(*postr2) )
                    break;
            }
            *(++postr2) = 0;
        }
        else
        {
            // there is no %ABSFILE% or %FILE% in the Mask
            m_FileName[0] = 0;
        }
        
        // skip leading spaces
        postr3 = skip_tabspaces(postr3);
        m_nLine = _ttoi(postr3);

        // skip leading spaces
        postr4 = skip_tabspaces(postr4);
        m_nChar = _ttoi(postr4);
    }
    else // *** START: New regex Warning/Error parser
    {	 // *** All of the new regex Warning/Error parser is within this else block ***
        tsmatch match;
        const tstring HeyStack = str;
        static std::map<int,int> ErrPositionIndicator;
        static tstring PreviousFileName;
        static int PreviousLineNo = 0;
        if ( std::regex_search( HeyStack, match, m_rgxFindFilename ) && match[0].str().size()) // Find file name (find needle in a hey stack)
        {	
            tstring filename = match[0].str().size() > 1 && match[0].str()[0] == L'.' ? match[0].str().substr( 1 ) : match[0].str();
            if ( filename != PreviousFileName )
            {
                PreviousFileName = filename;
                ErrPositionIndicator.clear();
            }
            wcscpy_s( m_FileName, sizeof( m_FileName )/sizeof( m_FileName[0]), filename.c_str() );
            m_nLastFoundIndex = 0;
            *m_Filter = TFilter();
            tstring Suffix = match.suffix();
            tstring strLineNo = std::regex_replace( Suffix, m_rgxFindFileLineNo, _T("$1") ); // Replace to get file number only
            if ( strLineNo.size() && isdigit( strLineNo[0] ) )
            {
                m_Filter->Effect.rgb = COLOR_CON_TEXTERR; //Only color if found file name and line number
                m_nLine = std::stoi( strLineNo.c_str() );
                PreviousLineNo = m_nLine;
                tstring strCharNo = std::regex_replace( HeyStack, m_rgxFindFileLinePos, _T("$1") ); // (if exist) replace to get nChar only
                if ( strCharNo.size() && isdigit( strCharNo[0] ) )
                    m_nChar = std::stoi( strCharNo.c_str() );
                else
                {
                    Suffix = match.suffix();
                    if ( std::regex_search( Suffix, match, m_rgxFindErrPosIndicator ) ) // Find error possition indicator
                        ErrPositionIndicator[m_nLine] = static_cast<int>(match[0].str().size());
                    m_nChar = ErrPositionIndicator[m_nLine];
                }
            } 
            else
            {
                m_nLine = 0;
                m_nChar = 0;
            }
            m_Filter->Effect.Bold = true;
            m_Filter->Effect.Enable = true;
            if ( filename.size() > 4 && lstrcmpi( filename.c_str() + (filename.size() - 4), _T(".exe")) == 0 )
                return false;
            pszMask = m_Filter->Mask;
        }
        else
        {
            if ( std::regex_search( HeyStack, match, m_rgxFindErrPosIndicatorAtStartOfLine ) ) // Find error possition indicator
            {
                tstring ErrorPositionIndicator = match[0].str().size() > 2 && match[0].str().compare( 0, 3, _T("...") ) == 0 ? match[0].str().substr( 3 ) : match[0].str();
                ErrPositionIndicator[PreviousLineNo] = static_cast<int>(ErrorPositionIndicator.size());
            }
        }
    } // *** END: New regex Warning/Error parser

    return ( pszMask ? true : false );
}

bool CWarningAnalyzer::HasEnabledFilters() const
{
    for ( int i = 0; i < WARN_MAX_FILTER; i++ )
    {
        if ( m_Filter[i].Effect.Enable )
            return true;
    }
    return false;
}

int CWarningAnalyzer::GetLastFoundIndex() const
{
    return m_nLastFoundIndex;
}

const TCHAR* CWarningAnalyzer::GetFileName() const
{
    return m_FileName;
}

int CWarningAnalyzer::GetLineNumber() const
{
    return m_nLine;
}

int CWarningAnalyzer::GetCharNumber() const
{
    return m_nChar;
}

long CWarningAnalyzer::GetColor() const
{
    const TEffect& effect = m_Filter[ m_nLastFoundIndex ].Effect;
    return ( RGB( effect.Red
                , effect.Green
                , effect.Blue
                )
           );
}

int CWarningAnalyzer::GetStyle() const
{
    const TEffect& effect = m_Filter[ m_nLastFoundIndex ].Effect;
    return ( ( effect.Italic     ? CFE_ITALIC    : 0 )
           + ( effect.Bold       ? CFE_BOLD      : 0 )
           + ( effect.Underlined ? CFE_UNDERLINE : 0 )
           );
}

bool CWarningAnalyzer::IsEffectEnabled( int FilterNumber ) const
{
    return m_Filter[ FilterNumber ].Effect.Enable;
}

void CWarningAnalyzer::EnableEffect( int FilterNumber, bool Enable )
{
    m_Filter[ FilterNumber ].Effect.Enable = Enable;
}

bool match_mask_2( const TCHAR* mask
                 , const TCHAR* str
                 ,       TCHAR* postr1
                 ,       TCHAR* postr2
                 ,       TCHAR* postr3
                 ,       TCHAR* postr4
                 )
{
    if ( mask && str )
    {
        const TCHAR* pMask = mask;
        const TCHAR* pStr  = str;
        TCHAR* postr   = NULL;
        TCHAR* postr0  = NULL;
        bool   matched = false;
        bool   done    = false;

        while (!done)
        {
            if ( *pMask == _T('*') ) // 0 or more characters
            {
                ++pMask;
                if ( *pMask == TCM_FILE1 )
                {
                    postr = postr1;
                }
                else if ( *pMask == TCM_FILE2 )
                {
                    postr = postr2;
                }
                else if ( *pMask == TCM_LINE )
                {
                    postr = postr3; // a number expected
                }
                else if ( *pMask == TCM_CHAR )
                {
                    postr = postr4; // a number expected
                }
                else
                {
                    postr = NULL;
                }
                postr0 = postr;

                if ( ( (postr == postr3) || (postr == postr4) )
                  && ( !is_num_str(pStr) ) 
                   )
                {
                    // pStr is expected to contain a number, but it's not
                    matched = false;
                }
                else
                {
                    if (*pMask) ++pMask;

                    if (*pMask == 0)
                    {
                        matched = true;
                        if ( postr )
                        {
                            while ( (postr < postr0 + WARN_MAX_FILENAME) && (*postr++ = *pStr++) ) ;
                            *postr = 0;
                        }
                    }
                    else
                    {
                        while ( ( !( matched = match_mask_2( pMask, pStr, postr1, postr2, postr3, postr4 ) ) )
                              &&( *pStr != 0 )
                              )
                        {
                            if ( postr )
                            {
                                if ( postr < postr0 + WARN_MAX_FILENAME )
                                {
                                    *postr++ = *pStr;
                                }
                            }
                            pStr++;
                        }
                        if ( postr )
                        {
                            *postr = 0;
                        }
                    }
                }
                done = true;
            }
            else if ( ( pMask[0] == _T(':')   ) // ABS PATH can be drive:\... or drive:/...
                    &&( pMask[1] == _T('\\')  )
                    &&( pMask[2] == _T('*')   )
                    &&( pMask[3] == TCM_FILE2 )
                    &&( pStr[0] == _T(':')    )
                    &&( ( pStr[1] == _T('\\') )
                      ||( pStr[1] == _T('/')  )
                      )
                    )
            {
                    ++pMask;
                    ++pStr;
                    ++pMask;
                    ++pStr;
            }
            else if ( *pMask == 0 ) // mask is over
            {
                matched = (*pStr == 0) ? true : false;
                done = true;
            }
            else
            {
                if ( ( *pMask == *pStr ) // exact match, case-sensitive
                  || ( (*pMask == _T('?')) && (*pStr != 0) ) // any character
                  || ( (*pMask == _T(' ')) && (*pStr == _T('\t')) ) // tab treated as space
                   ) 
                {
                    ++pMask;
                    ++pStr;
                }
                else
                {
                    matched = false;
                    done = true;
                }
            }
        }
        return matched;
    }
    return false;
}

TCHAR* skip_tabspaces(TCHAR* s)
{
    while ( NppExecHelpers::IsTabSpaceChar(*s) )  ++s;
    return s;
}

const TCHAR* skip_tabspaces(const TCHAR* s)
{
    while ( NppExecHelpers::IsTabSpaceChar(*s) )  ++s;
    return s;
}

bool is_num_str(const TCHAR* s)
{
    if ( *s )
    {
        s = skip_tabspaces(s);
        if ( *s )
        {
            if ( (*s == _T('-')) || (*s == _T('+')) )
                ++s;

            if ( (*s >= _T('0')) && (*s <= _T('9')) )
                return true;
        }
    }

    return false;
}

unsigned char CWarningAnalyzer::xtou( const TCHAR x1, const TCHAR x0 )
{

    return ((unsigned char) ( 16 * ( ( x0 == 0  )               ? 0
                                   : ( x1 >='A' )&&( x1 <='F' ) ? x1 - 'A' + 10
                                   : ( x1 >='a' )&&( x1 <='f' ) ? x1 - 'a' + 10
                                   : ( x1 >='0' )&&( x1 <='9' ) ? x1 - '0'
                                   : 0
                                   )
                            +      ( ( x0 >='A' )&&( x0 <='F' ) ? x0 - 'A' + 10
                                   : ( x0 >='a' )&&( x0 <='f' ) ? x0 - 'a' + 10
                                   : ( x0 >='0' )&&( x0 <='9' ) ? x0 - '0'
                                   : 0
                                   )
                            ));
}

TCHAR* CWarningAnalyzer::utox( unsigned char i, TCHAR *x, int size )
{
    unsigned char j = i;
    int count = size - 2;

    x[size-1] = 0;
    while ( count >= 0 )
    {
        x[ count-- ] = ( (j&0xF) > 9 ? 'A' + (j&0xF) - 10 : '0' + (j&0xF) );
        j = j>>4;
    }
    return ( x );    
}
