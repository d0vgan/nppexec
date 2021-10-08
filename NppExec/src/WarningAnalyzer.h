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

#ifndef _warning_analyzer_h_
#define _warning_analyzer_h_
//---------------------------------------------------------------------------
#include "base.h"
#include <regex>

#define WARN_MASK_SIZE    ( 150 )
#define WARN_MAX_FILTER   ( 10 )
#define WARN_MAX_FILENAME ( 2000 )

class CWarningAnalyzer 
{
public:
    enum eMaskType {
        MT_NONE    = 0x0000,
        MT_FILE    = 0x0001,
        MT_ABSFILE = 0x0002,
        MT_LINE    = 0x0010,
        MT_CHAR    = 0x0020
    };

    struct TEffect
    {
        bool Enable;
        bool Italic;
        bool Bold;
        bool Underlined;
        unsigned char Red;
        unsigned char Green;
        unsigned char Blue;

        TEffect() : Enable(false)
                  , Italic(false), Bold(false), Underlined(false)
                  , Red(0), Green(0), Blue(0)
        {
        }
    };

    struct TFilter
    {
        TCHAR        Mask[WARN_MASK_SIZE];
        unsigned int MaskType;
        TEffect      Effect;

        TFilter() : MaskType(0)
        {
            Mask[0] = 0;
        }
    };
	typedef std::basic_string<TCHAR> tstring;
	typedef std::basic_regex<TCHAR> tregex;
	typedef std::match_results<tstring::const_iterator> tsmatch;

public:
    CWarningAnalyzer();
    ~CWarningAnalyzer();
    bool         match( const TCHAR* str );
    bool         HasEnabledFilters() const;
    int          GetLastFoundIndex() const;
    const TCHAR* GetFileName() const;
    int          GetLineNumber() const;
    int          GetCharNumber() const;
    long         GetColor() const;
    int          GetStyle() const;
    bool         IsEffectEnabled( int FilterNumber ) const;
    void         EnableEffect( int FilterNumber, bool Enable );
    void         SetEffect( int FilterNumber, const TEffect& Effect );
    void         SetMask( int FilterNumber, const TCHAR* Mask );
    void         GetEffect( int FilterNumber, TEffect& Effect ) const;
    const TCHAR* GetMask( int FilterNumber, TCHAR* Mask, int MaskLength ) const;

    static unsigned char xtou( const TCHAR x1, const TCHAR x0 );
    static TCHAR* utox( unsigned char i, TCHAR *x, int size );

private:
    TFilter m_Filter[WARN_MAX_FILTER];
    TCHAR   m_FileName[WARN_MAX_FILENAME + 5];
    int     m_nLine;
    int     m_nChar;
    int     m_nLastFoundIndex;
	static const tregex m_rgxFindFilename;// Regex to find file name with file number proceeding it
	static const tregex m_rgxFindFileLineNo;// Regex to find file line number with file name preceeding it
	static const tregex m_rgxFindFileLinePos;// Regex to find error possition number
	static const tregex m_rgxFindErrPosIndicator;// Regex to find error possition indicator
	static const tregex m_rgxFindErrPosIndicatorAtStartOfLine;// Regex to find error possition indicator
};

//---------------------------------------------------------------------------
#endif
