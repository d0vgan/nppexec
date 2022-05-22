#ifndef _str_split_t_h_
#define _str_split_t_h_
//----------------------------------------------------------------------------
#include "CStrT.h"
#include "CListT.h"

namespace StrSplitT
{
    inline bool is_any_space_char(char ch)
    {
        switch ( ch )
        {
            case ' ':
            case '\t':
            case '\v':
            case '\f':
                return true;
        }
        return false;
    }

    inline bool is_any_space_char(wchar_t ch)
    {
        switch ( ch )
        {
            case L' ':
            case L'\t':
            case L'\v':
            case L'\f':
                return true;
        }
        return false;
    }
}

// split to args, considering quotes
template <class T> int StrSplitToArgs(const T* str,
  CListT< CStrT<T> >& outList, int max_items = 0)
{
    outList.Clear();
    if ( str )
    {
        const int MODE_TABSPACE  = 0;
        const int MODE_CHARACTER = 1;
        const int MODE_DBLQUOTES = 2;
        const int MODE_SGLQUOTE1 = 3;
        const int MODE_SGLQUOTE2 = 4;
        const T   CHAR_DBLQUOTES = '\"';
        const T   CHAR_SGLQUOTE1 = '\'';
        const T   CHAR_SGLQUOTE2 = '`';

        int      mode = MODE_TABSPACE;
        CStrT<T> S;

        while ( *str )
        {
            const T ch = *str;
            if ( mode == MODE_DBLQUOTES )
            {
                if ( ch == CHAR_DBLQUOTES )
                {
                    mode = MODE_CHARACTER;
                }
                else
                {
                    S.Append( ch );
                }
                ++str;
                continue;
            }
            if ( mode == MODE_SGLQUOTE1 )
            {
                if ( ch == CHAR_SGLQUOTE1 )
                {
                    mode = MODE_CHARACTER;
                }
                else
                {
                    S.Append( ch );
                }
                ++str;
                continue;
            }
            if ( mode == MODE_SGLQUOTE2 )
            {
                if ( ch == CHAR_SGLQUOTE2 )
                {
                    mode = MODE_CHARACTER;
                }
                else
                {
                    S.Append( ch );
                }
                ++str;
                continue;
            }
            if ( mode == MODE_TABSPACE )
            {
                if ( StrSplitT::is_any_space_char(ch) )
                {
                    ++str;
                    continue;
                }
                else
                {
                    if ( max_items > 0 )
                    {
                        if ( --max_items == 0 )
                        {
                            outList.Add( str );
                            return outList.GetCount();
                        }
                    }
                    mode = MODE_CHARACTER;
                }
            }
            
            if ( ch == CHAR_DBLQUOTES )
            {
                mode = MODE_DBLQUOTES;
            }
            else if ( ch == CHAR_SGLQUOTE1 )
            {
                mode = MODE_SGLQUOTE1;
            }
            else if ( ch == CHAR_SGLQUOTE2 )
            {
                mode = MODE_SGLQUOTE2;
            }
            else if ( StrSplitT::is_any_space_char(ch) )
            {
                mode = MODE_TABSPACE;
                outList.Add( S );
                S.Clear();
            }
            else
            {
                S.Append( ch );
            }
            ++str;
        }
        if ( mode != MODE_TABSPACE )
        {
            outList.Add( S );
        }
    }
    return outList.GetCount();
}

// split to args by separator, considering quotes
template <class T> int StrSplitAsArgs(const T* str,
  CListT< CStrT<T> >& outList, const T sep, int max_items = 0)
{
    outList.Clear();
    if ( str )
    {
        const int MODE_TABSPACE  = 0;
        const int MODE_CHARACTER = 1;
        const int MODE_DBLQUOTES = 2;
        const int MODE_SGLQUOTE1 = 3;
        const int MODE_SGLQUOTE2 = 4;
        const T   CHAR_DBLQUOTES = '\"';
        const T   CHAR_SGLQUOTE1 = '\'';
        const T   CHAR_SGLQUOTE2 = '`';

        int      n = 0;
        int      mode = MODE_TABSPACE;
        CStrT<T> S;

        while ( *str )
        {
            const T ch = *str;
            if ( mode == MODE_DBLQUOTES )
            {
                if ( ch == CHAR_DBLQUOTES )
                {
                    mode = MODE_CHARACTER;
                }
                else
                {
                    S.Append( ch );
                    n = S.length(); // to keep quoted spaces, if any
                }
                ++str;
                continue;
            }
            if ( mode == MODE_SGLQUOTE1 )
            {
                if ( ch == CHAR_SGLQUOTE1 )
                {
                    mode = MODE_CHARACTER;
                }
                else
                {
                    S.Append( ch );
                    n = S.length(); // to keep quoted spaces, if any
                }
                ++str;
                continue;
            }
            if ( mode == MODE_SGLQUOTE2 )
            {
                if ( ch == CHAR_SGLQUOTE2 )
                {
                    mode = MODE_CHARACTER;
                }
                else
                {
                    S.Append( ch );
                    n = S.length(); // to keep quoted spaces, if any
                }
                ++str;
                continue;
            }
            if ( mode == MODE_TABSPACE )
            {
                if ( StrSplitT::is_any_space_char(ch) )
                {
                    ++str;
                    continue;
                }
                else
                {
                    if ( max_items > 0 )
                    {
                        if ( --max_items == 0 )
                        {
                            outList.Add( str );
                            return outList.GetCount();
                        }
                    }
                    mode = MODE_CHARACTER;
                }
            }
            
            if ( ch == CHAR_DBLQUOTES )
            {
                mode = MODE_DBLQUOTES;
            }
            else if ( ch == CHAR_SGLQUOTE1 )
            {
                mode = MODE_SGLQUOTE1;
            }
            else if ( ch == CHAR_SGLQUOTE2 )
            {
                mode = MODE_SGLQUOTE2;
            }
            else if ( ch == sep )
            {
                mode = MODE_TABSPACE;

                int i = S.length() - 1;
                while ( (i >= n) && StrSplitT::is_any_space_char(S.GetAt(i)) )  --i;
                S.SetSize(i + 1);
                outList.Add( S );
                S.Clear();
                n = 0;
            }
            else
            {
                S.Append( ch );
            }
            ++str;
        }
        //if ( mode != MODE_TABSPACE )
        {
            outList.Add( S );
        }
    }
    return outList.GetCount();
}

template <class T> int StrSplit(const CStrT<T>& str, 
  const T* separator, CListT< CStrT<T> >& outList, int max_items = 0)
{
    outList.Clear();
    if ( str.length() > 0 )
    {
        int sep_len = GetStrSafeLength(separator);
        if ( sep_len > 0 )
        {
            CStrT<T> S;
            int      pos2, pos1 = 0;
            if (max_items > 0)
            {
                --max_items;
                while ( (outList.GetCount() < max_items) &&
                        ((pos2 = str.Find(separator, pos1)) >= 0) )
                {
                    S.Assign( str.c_str() + pos1, pos2 - pos1 );
                    outList.Add(S);
                    pos1 = pos2 + sep_len;
                }
            }
            else
            {
                while ( (pos2 = str.Find(separator, pos1)) >= 0 )
                {
                    S.Assign( str.c_str() + pos1, pos2 - pos1 );
                    outList.Add(S);
                    pos1 = pos2 + sep_len;
                }
            }
            S.Assign( str.c_str() + pos1, str.length() - pos1 );
            outList.Add(S);
        }
    }
    return outList.GetCount();
}

//----------------------------------------------------------------------------

template <class T> class CStrSplitT
{
private:
    CListT< CStrT<T> >     m_argList;
    mutable CStrT<T>       m_tempStr;
    static const CStrT<T>  m_emptyStr;

public:
    const CStrT<T>& Arg(int index) const;
    CStrT<T>&       Arg(int index);
    const CStrT<T>& GetArg(int index) const;
    int             GetArgCount() const  { return m_argList.GetCount(); }
    const CStrT<T>& GetArgs() const;            // string of Args without Arg[0]
    const CStrT<T>& GetRArgs() const;           // Args in reverse order
    const CStrT<T>& GetRArg(int index) const; // GetRArg(0) is not valid!
    const CStrT<T>& RArg(int index) const;    // RArg(0) is not valid!
    CStrT<T>&       RArg(int index);          // RArg(0) is not valid!
    bool            SetArg(int index, const T* S);
    bool            SetArg(int index, const CStrT<T>& S);
    int             SplitAsArgs(const T* str, const T separator, int max_items = 0);
    int             SplitAsArgs(const CStrT<T>& str, const T separator, int max_items = 0);
    int             SplitToArgs(const T* str, int max_items = 0);
    int             SplitToArgs(const CStrT<T>& str, int max_items = 0);
    int             Split(const T* str, const T* separator, int max_items = 0);
    int             Split(const CStrT<T>& str, const T* separator, int max_items = 0);
};

template <class T> const CStrT<T> CStrSplitT<T>::m_emptyStr;

template <class T> const CStrT<T>& CStrSplitT<T>::Arg(int index) const
{
    if ( (index >= 0) && (index < m_argList.GetCount()) )
    {
        CListItemT< CStrT<T> > * p = m_argList.GetFirst();
        while ( p && (index--) )
        {
            p = p->GetNext();
        }
        if ( p )  return p->GetItem();
    }
    return m_emptyStr;
}

template <class T> CStrT<T>& CStrSplitT<T>::Arg(int index)
{
    if ( (index >= 0) && (index < m_argList.GetCount()) )
    {
        CListItemT< CStrT<T> > * p = m_argList.GetFirst();
        while ( p && (index--) )
        {
            p = p->GetNext();
        }
        if ( p )  return p->GetItem();
    }
    m_tempStr.Clear();
    return m_tempStr;
}

template <class T> const CStrT<T>& CStrSplitT<T>::GetArg(int index) const
{
    return Arg(index);
}

template <class T> const CStrT<T>& CStrSplitT<T>::GetArgs() const
{
    CListItemT< CStrT<T> > * p = m_argList.GetFirst();
    if ( p )
    {
        m_tempStr.Clear();
        p = p->GetNext();
        while ( p )
        {
            if (m_tempStr.length() > 0)
                m_tempStr.Append(' ');
            const CStrT<T>& S = p->GetItem();
            if ( (S.Find(' ') >= 0) || (S.Find('\t') >= 0) )
            {
                m_tempStr.Append( '\"' );
                m_tempStr.Append( S );
                m_tempStr.Append( '\"' );
            }
            else
            {
                m_tempStr.Append( S );
            }
            p = p->GetNext();
        }
        return m_tempStr;
    }
    return m_emptyStr;
}

template <class T> const CStrT<T>& CStrSplitT<T>::GetRArgs() const
{
    CListItemT< CStrT<T> > * p = m_argList.GetLast();
    if ( p )
    {
        m_tempStr.Clear();
        while ( p && (p != m_argList.GetFirst()) )
        {
            if (m_tempStr.length() > 0)
                m_tempStr.Append(' ');
            const CStrT<T>& S = p->GetItem();
            if ( (S.Find(' ') >= 0) || (S.Find('\t') >= 0) )
            {
                m_tempStr.Append( '\"' );
                m_tempStr.Append( S );
                m_tempStr.Append( '\"' );
            }
            else
            {
                m_tempStr.Append( S );
            }
            p = p->GetPrev();
        }
        return m_tempStr;
    }
    return m_emptyStr;
}

template <class T> const CStrT<T>& CStrSplitT<T>::GetRArg(int index) const
{
    return RArg(index);
}

template <class T> const CStrT<T>& CStrSplitT<T>::RArg(int index) const
{
    if ( (index > 0) && (index < m_argList.GetCount()) )
    {
        CListItemT< CStrT<T> > * p = m_argList.GetLast();
        while ( p && (--index) )
        {
            p = p->GetPrev();
        }
        if ( p )  return p->GetItem();
    }
    return m_emptyStr;
}

template <class T> CStrT<T>& CStrSplitT<T>::RArg(int index)
{
    if ( (index > 0) && (index < m_argList.GetCount()) )
    {
        CListItemT< CStrT<T> > * p = m_argList.GetLast();
        while ( p && (--index) )
        {
            p = p->GetPrev();
        }
        if ( p )  return p->GetItem();
    }
    m_tempStr.Clear();
    return m_tempStr;
}

template <class T> bool CStrSplitT<T>::SetArg(int index, const T* S)
{
    return SetArg( index, CStrT<T>(S) );
}

template <class T> bool CStrSplitT<T>::SetArg(int index, const CStrT<T>& S)
{
    if ( (index >= 0) && (index < m_argList.GetCount()) )
    {
        CListItemT< CStrT<T> > * p = m_argList.GetFirst();
        while ( p && index )
        {
            --index;
            p = p->GetNext();
        }
        if ( p ) 
        {
            p->SetItem(S);
            return true;
        }
    }
    return false;
}

template <class T> int CStrSplitT<T>::SplitAsArgs(const T* str, const T separator, int max_items)
{
    return StrSplitAsArgs( str, m_argList, separator, max_items );
}

template <class T> int CStrSplitT<T>::SplitAsArgs(const CStrT<T>& str, const T separator, int max_items)
{
    return StrSplitAsArgs( str.c_str(), m_argList, separator, max_items );
}

template <class T> int CStrSplitT<T>::SplitToArgs(const T* str, int max_items)
{
    return StrSplitToArgs<T>( str, m_argList, max_items );
}

template <class T> int CStrSplitT<T>::SplitToArgs(const CStrT<T>& str, int max_items)
{
    return StrSplitToArgs<T>( str.c_str(), m_argList, max_items );
}

template <class T> int CStrSplitT<T>::Split(const T* str, const T* separator, int max_items)
{
    return StrSplit<T>( CStrT<T>(str), separator, m_argList, max_items );
}

template <class T> int CStrSplitT<T>::Split(const CStrT<T>& str, const T* separator, int max_items)
{
    return StrSplit<T>( str, separator, m_argList, max_items );
}


//----------------------------------------------------------------------------
#endif
