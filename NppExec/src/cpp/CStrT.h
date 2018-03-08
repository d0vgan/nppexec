/***********************************************
 *  
 *  CStrT ver. 1.3.0
 *  --------------------------------  
 *  (C) DV, Nov 2006 - Oct 2017
 *  --------------------------------
 *
 *  Template:
 *  >>  CStrT<type T>
 *
 *  Pre-defined types:
 *  >>  CStr  (Str of char)
 *  >>  CWStr (Str of whar_t)
 *
 ***********************************************/

#ifndef _tiny_str_t_h_
#define _tiny_str_t_h_
//----------------------------------------------------------------------------

// va_list
#ifdef __MINGW32__
  #include <stdarg.h>
#endif

// wvsprintf
#include <windows.h>

// _T
#include <TCHAR.h>

#include <utility>

// CONDITION: str != 0
template <class T> int GetStrUnsafeLength(const T* str)
{
    const T* str0 = str;
    while ( *str )  ++str;
    return ( (int) (str - str0) );
}

template <class T> int GetStrSafeLength(const T* str)
{
    return ( str ? GetStrUnsafeLength(str) : 0 );
}

// CONDITION: str1 != 0 && str2 != 0
template <class T> int StrUnsafeCmp(const T* str1, const T* str2)
{
    while ( (*str1) && (*str1 == *str2) )
    {
        ++str1;
        ++str2;
    }
    return (int) ( ((unsigned int) (*str1)) - ((unsigned int) (*str2)) );
}

// CONDITION: str1 != 0 && str2 != 0 && n > 0
template <class T> int StrUnsafeCmpN(const T* str1, const T* str2, unsigned int n)
{
    while ( (*str1) && (*str1 == *str2) && (n != 0) )
    {
        ++str1;
        ++str2;
        --n;
    }
    return (int) ( ((unsigned int) (*str1)) - ((unsigned int) (*str2)) );
}

// CONDITION: str != 0 && substr != 0
template <class T> int StrUnsafeSubCmp(const T* str, const T* substr)
{
    while ( (*str) && (*str == *substr) )
    {
        ++str;
        ++substr;
    }
    if ( !(*substr) )
    {
        return 0;
    }
    return (int) ( ((unsigned int) (*str)) - ((unsigned int) (*substr)) );
}

// CONDITION: str != 0 && substr != 0 && n > 0
template <class T> int StrUnsafeSubCmpN(const T* str, const T* substr, unsigned int n)
{
    while ( (*str) && (*str == *substr) && (n != 0) )
    {
        ++str;
        ++substr;
        --n;
    }
    if ( (!n) || !(*substr) )
    {
        return 0;
    }
    return (int) ( ((unsigned int) (*str)) - ((unsigned int) (*substr)) );
}

// CONDITION: dst != 0 && src != 0 && len > 0
template <class T> void StrUnsafeCopyN(T* dst, const T* src, 
        unsigned int len, bool bFinalNull )
{
    if ( (dst > src) && (dst < src + len) )
    {
        src += len;
        dst += len;
        if ( bFinalNull )  *dst = 0;
        while ( len-- )
        {
            *(--dst) = *(--src);
        }
    }
    else
    {
        while ( len-- )
        {
            *(dst++) = *(src++);
        }
        if ( bFinalNull )  *dst = 0;
    }
}

template <class T> class CStrT
{
private:
  T*    m_pData;
  int   m_nLength;
  int   m_nMemSize;
  
  bool  replaceStr(int pos, int count, const T* str_new, int str_len);
    
public:
  typedef T value_type;

  CStrT();
  CStrT(const T* pStr);
  CStrT(const T* pStr, int nLength);
  CStrT(const CStrT& Str);
  CStrT(CStrT&& Str);
  ~CStrT();
  T*    Append(const T* pStr, int nLength = -1); // -1 means all characters
  T*    Append(const CStrT& Str);
  T*    Append(const T ch);
  T*    c_str() const  { return ( m_pData ? m_pData : (T*) "\x00\x00" ); }
  int   CalculateLength();
  void  Clear();
  int   Compare(const T* pStr) const; 
          // returns -1 if c_str() < pStr
  int   Compare(const CStrT& Str) const;     
          // returns 1 if c_str() > pStr;  0 if identical; 
          // 2 if nLength characters are identical and nLength < m_nLength
  T*    Copy(const T* pStr, int nLength = -1); // -1 means all characters
  T*    Copy(const CStrT& Str);
  T*    Copy(CStrT&& Str);
  bool  Delete(int nPos, int nCharacters = -1); 
          // -1 means all characters from nPos
  bool  DeleteFirstChar()  { return Delete(0, 1); }
  bool  DeleteLastChar()  { return Delete(m_nLength - 1, 1); }
  bool  EndsWith(const T ch) const;
  bool  EndsWith(const T* pStr) const;
  bool  EndsWith(const CStrT& Str) const;
  int   Find(const T ch, int nStartPos = 0) const;
  int   Find(const T* pStr, int nStartPos = 0) const;
  int   Find(const CStrT& Str, int nStartPos = 0) const;
  int   FindOneOf(const T* pChars, int nStartPos = 0) const;
  int   FindOneOf(const CStrT& Chars, int nStartPos = 0) const;
  T*    Format(int nMaxLength, const T* pFmt, ...);
  void  FreeMemory();
  T     GetAt(int nPos) const;
  T     GetFirstChar() const;
  T     GetLastChar() const;
  int   GetLength() const  { return m_nLength; }
  T*    GetData() const  { return ( m_pData ? m_pData : (T*) "\x00\x00" ); }  
  int   GetMemSize() const  { return m_nMemSize; }
  T*    Insert(int nPos, const T* pStr, int nLength = -1); 
          // -1 means all characters
  T*    Insert(int nPos, const CStrT& Str);
  T*    Insert(int nPos, const T ch);
  bool  IsEmpty() const  { return (m_nLength == 0); }
  bool  IsEqual(const T* pStr) const  { return ( Compare(pStr) == 0 ); }
  bool  IsEqual(const CStrT& Str) const  { return ( Compare(Str.c_str()) == 0 ); }
  int   length() const  { return m_nLength; }
  int   Replace(const T chOld, const T chNew);
  int   Replace(const T* pSubStrOld, const T* pSubStrNew);
  int   Replace(int nPos, int nCharacters, const T* pSubStrNew, int nLength = -1);
          // nCharacters = -1 means all characters till '\0'
  int   RFind(const T ch, int nStartPos = -1) const;
  int   RFind(const T* pStr, int nStartPos = -1) const;
  int   RFind(const CStrT& Str, int nStartPos = -1) const;
  bool  Reserve(int nLength)  { 
            return ( (nLength > m_nLength) ? SetSize(nLength) : true ); 
        } // like in STL
  bool  SetAt(int nPos, const T ch);
  bool  SetLengthValue(int nLength);
  bool  SetSize(int nLength); // previous string content is kept
  bool  StartsWith(const T ch) const;
  bool  StartsWith(const T* pStr) const;
  bool  StartsWith(const CStrT& Str) const;
  void  Swap(CStrT& Str); // swap str's data
  T     operator[](int nPos) const  { return m_pData[nPos]; }
  T&    operator[](int nPos)  { return m_pData[nPos]; }
  T*    operator=(const T* pStr)  { return Copy(pStr); }
  T*    operator=(const CStrT& Str)  { return Copy(Str); }
  T*    operator=(CStrT&& Str)  { return Copy(std::forward<CStrT>(Str)); }
  T*    operator+=(const T* pStr)  { return Append(pStr); }
  T*    operator+=(const CStrT& Str)  { return Append(Str); }
  T*    operator+=(const T ch)  { return Append(ch); }
  bool  operator==(const T* pStr) const  { 
            return ( Compare(pStr) == 0 ); 
        }
  bool  operator==(const CStrT& Str) const  { 
            return ( Compare(Str.c_str()) == 0 ); 
        }
  bool  operator!=(const T* pStr) const  {
            return ( Compare(pStr) != 0 ); 
        }
  bool  operator!=(const CStrT& Str) const  {
            return ( Compare(Str.c_str()) != 0 );
        }
  bool  operator>(const T* pStr) const  {
            return ( Compare(pStr) > 0 );
        }
  bool  operator>(const CStrT& Str) const  {
            return ( Compare(Str.c_str()) > 0 );
        }
  bool  operator>=(const T* pStr) const  {
            return ( Compare(pStr) >= 0 ); 
        }
  bool  operator>=(const CStrT& Str) const  {
            return ( Compare(Str.c_str()) >= 0 ); 
        }
  bool  operator<(const T* pStr) const  {
            return ( Compare(pStr) < 0 ); 
        }
  bool  operator<(const CStrT& Str) const  {
            return ( Compare(Str.c_str()) < 0 ); 
        }
  bool  operator<=(const T* pStr) const  {
            return ( Compare(pStr) <= 0 ); 
        }
  bool  operator<=(const CStrT& Str) const  {
            return ( Compare(Str.c_str()) <= 0 ); 
        }

};

//----------------------------------------------------------------------------

typedef  CStrT<char>     CStr;
typedef  CStrT<wchar_t>  CWStr;

//----------------------------------------------------------------------------

static inline int getAlignedMemSizeStr(int nCount)
{
  int nShift;

  if ( nCount < 256 )
      nShift = 6;
  else if ( nCount < 512 )
      nShift = 7;
  else if ( nCount < 1024 )
      nShift = 8;
  else if ( nCount < 2048 )
      nShift = 9;
  else
      nShift = 10;

  return ( (1 + (nCount >> nShift)) << nShift );
}

//----------------------------------------------------------------------------

template <class T> CStrT<T>::CStrT() : 
  m_pData(NULL), 
  m_nLength(0), 
  m_nMemSize(0)
{
}

template <class T> CStrT<T>::CStrT(const T* pStr) : 
  m_pData(NULL), 
  m_nLength(0), 
  m_nMemSize(0)
{
    if (pStr)  Append(pStr);
}

template <class T> CStrT<T>::CStrT(const T* pStr, int nLength) :
  m_pData(NULL),
  m_nLength(0), 
  m_nMemSize(0)
{
    if (pStr && (nLength != 0))  Append(pStr, nLength);
}

template <class T> CStrT<T>::CStrT(const CStrT& Str) :
  m_pData(NULL), 
  m_nLength(0), 
  m_nMemSize(0)
{
    Append(Str);
}

template <class T> CStrT<T>::CStrT(CStrT&& Str) :
  m_pData(NULL), 
  m_nLength(0), 
  m_nMemSize(0)
{
    Swap(Str);
}

template <class T> CStrT<T>::~CStrT()
{
    FreeMemory();
}

template <class T> T* CStrT<T>::Append(const T* pStr, int nLength )
{
    if ( !pStr )
        nLength = 0;
    else if ( nLength < 0 ) 
        nLength = GetStrUnsafeLength<T>(pStr); // pStr != 0 here

    if ( nLength > 0 )
    {
        int nOffset = -1;
        if ( (pStr >= m_pData) && (pStr < m_pData + m_nLength) )
        {
            nOffset = (int) (pStr - m_pData);
            if (nLength > m_nLength - nOffset)
                nLength = m_nLength - nOffset;
        }
  
        int nOldLength = m_nLength;
        int nNewLength = nOldLength + nLength;
        if ( SetSize(nNewLength) )
        {
            StrUnsafeCopyN<T>(m_pData + nOldLength, 
              (nOffset < 0) ? pStr : (m_pData + nOffset), nLength, true);
            m_nLength = nNewLength;
            // m_pData[m_nLength] = 0 - is set by StrUnsafeCopyN()
        }
    }
    return m_pData; // can be NULL
}

template <class T> T* CStrT<T>::Append(const CStrT& Str)
{
    return Append( Str.c_str(), Str.length() );
}

template <class T> T* CStrT<T>::Append(const T ch)
{
    if ( SetSize(m_nLength + 1) )
    {
        m_pData[m_nLength] = ch;
        ++m_nLength;
        m_pData[m_nLength] = 0;
    }
    return m_pData; // can be NULL
}

template <class T> int CStrT<T>::CalculateLength()
{
    m_nLength = GetStrSafeLength<T>(m_pData);
    return m_nLength;
}

template <class T> void CStrT<T>::Clear()
{
    if ( m_pData )
    {
        SetSize(0);
    }
    m_nLength = 0;  // must be AFTER SetSize(0)!!!
}

template <class T> int CStrT<T>::Compare(const T* pStr) const
{
    if ( m_pData == pStr )
        return 0;
  
    if ( !pStr )
        return ( (m_pData && m_pData[0]) ? 1 : 0 );

    if ( !m_pData )
        return ( pStr[0] ? (-1) : 0 );
    
    return StrUnsafeCmp<T>(m_pData, pStr);
}

template <class T> int CStrT<T>::Compare(const CStrT& Str) const
{
    return Compare( Str.c_str() );
}

template <class T> T* CStrT<T>::Copy(const T* pStr, int nLength )
{ 
    if ( pStr != m_pData )
    { 
        Clear(); 
        // works OK for pStr > m_pData && pStr < m_pData+nLength
        // due to implementation of Append()
        return Append(pStr, nLength); // can be NULL
    }
    else
    {
        if ( nLength < m_nLength )
            SetSize(nLength);
        return m_pData; // can be NULL
    }
}

template <class T> T* CStrT<T>::Copy(const CStrT& Str) 
{ 
    if ( Str.c_str() != m_pData )
    {
        Clear(); 
        return Append( Str.c_str(), Str.length() ); // can be NULL
    }
    else
        return m_pData; // can be NULL
}

template <class T> T* CStrT<T>::Copy(CStrT&& Str) 
{ 
    if ( Str.c_str() != m_pData )
    {
        Clear(); 
        Swap(Str);
    }
    return m_pData; // can be NULL
}

template <class T> bool CStrT<T>::Delete(int nPos, int nCharacters )
{
    if ( (m_nLength == 0) || (nPos < 0) || (nCharacters == 0) )
        return false;

    int nMaxDeleteCharacters = m_nLength - nPos;
    if ( nMaxDeleteCharacters <= 0 )
        return false;

    if ( (nCharacters < 0) || (nCharacters >= nMaxDeleteCharacters) )
        return SetSize(nPos);

    StrUnsafeCopyN<T>(m_pData + nPos, m_pData + nPos + nCharacters, 
      nMaxDeleteCharacters - nCharacters, true);
    m_nLength -= nCharacters;
    // m_pData[m_nLength] = 0 - is set by StrUnsafeCopyN()
    return true;
}

template <class T> bool CStrT<T>::EndsWith(const T ch) const
{
    if ( (ch != 0) && (ch == GetLastChar()) )
        return true;
    else
        return false;
}

template <class T> bool CStrT<T>::EndsWith(const T* pStr) const
{
    if ( pStr && (m_nLength > 0) )
    {
        const int len = GetStrUnsafeLength(pStr);
        if ( len > 0 )
        {
            const int pos = m_nLength - len;
            if ( pos >= 0 )
            {
                if ( StrUnsafeSubCmp(m_pData + pos, pStr) == 0 )
                    return true;
            }
        }
    }
    return false;
}

template <class T> bool CStrT<T>::EndsWith(const CStrT& Str) const
{
    if ( Str.m_nLength > 0 )
    {
        const int pos = m_nLength - Str.m_nLength;
        if ( pos >= 0 )
        {
            if ( StrUnsafeSubCmp(m_pData + pos, Str.m_pData) == 0 )
                return true;
        }
    }
    return false;
}

template <class T> int CStrT<T>::Find(const T ch, int nStartPos ) const
{
    if ( nStartPos >= 0 )
    { 
        while ( nStartPos < m_nLength )
        {
            if ( m_pData[nStartPos] == ch )
            {
                return nStartPos;
            }
            ++nStartPos;
        }
    }
    return -1;
}

template <class T> int CStrT<T>::Find(const T* pStr, int nStartPos ) const
{
    if ( (nStartPos >= 0) && pStr && pStr[0] )
    {
        while ( nStartPos < m_nLength )
        {
            if ( StrUnsafeSubCmp<T>(m_pData + nStartPos, pStr) != 0 )
                ++nStartPos;
            else
                return nStartPos;
        }
    }
    return -1;
}

template <class T> int CStrT<T>::Find(const CStrT& Str, int nStartPos ) const
{
    return Find( Str.c_str(), nStartPos );
}

template <class T> int CStrT<T>::FindOneOf(const T* pChars, int nStartPos ) const
{
    if ( (nStartPos >= 0) && pChars && pChars[0] )
    {
        while ( nStartPos < m_nLength )
        {
            const T cch = m_pData[nStartPos];
            const T* pch = pChars;
            while ( *pch )
            {
                if ( *pch == cch )
                {
                    return nStartPos;
                }
                ++pch;
            }
            ++nStartPos;
        }
    }
    return -1;
}

template <class T> int CStrT<T>::FindOneOf(const CStrT& Chars, int nStartPos ) const
{
    return FindOneOf( Chars.c_str(), nStartPos );
}

template <class T> T* CStrT<T>::Format(int nMaxLength, const T* pFmt, ...)
{
    Clear();
    if ( SetSize(nMaxLength) )
    {
        int nLength;
        va_list argList;
        va_start(argList, pFmt);
        nLength = ::wvsprintf(m_pData, pFmt ? pFmt : _T(""), argList);
        va_end(argList);
        SetLengthValue(nLength);
    }
    return m_pData; // can be NULL
}

template <class T> void CStrT<T>::FreeMemory()
{
    if ( m_pData )
        delete [] m_pData;
    m_pData = NULL;
    m_nLength = 0;
    m_nMemSize = 0;
}

template <class T> T CStrT<T>::GetAt(int nPos) const
{
    return ((nPos >= 0) && (nPos < m_nLength)) ? m_pData[nPos] : 0;
}

template <class T> T CStrT<T>::GetFirstChar() const
{
    return (m_nLength > 0) ? m_pData[0] : 0;
}

template <class T> T CStrT<T>::GetLastChar() const
{
    return (m_nLength > 0) ? m_pData[m_nLength - 1] : 0;
}

template <class T> T* CStrT<T>::Insert(int nPos, const T* pStr, int nLength )
{
    if ( (nPos < 0) || (!pStr) )
        return m_pData; // can be NULL

    if ( nLength < 0 ) 
        nLength = GetStrUnsafeLength<T>(pStr);
  
    if ( nLength == 0 )
        return m_pData; // can be NULL

    if ( nPos >= m_nLength )
        return Append(pStr, nLength);

    int nOffset = -1;
    if ( (pStr >= m_pData) && (pStr < m_pData + m_nLength) )
    {
        nOffset = (int) (pStr - m_pData);
        if ( nLength > m_nLength - nOffset )
            nLength = m_nLength - nOffset;
    }

    int nOldLength = m_nLength;
    int nNewLength = nOldLength + nLength;
    if ( SetSize(nNewLength) )
    {
        if (nOffset < 0)
        {
            StrUnsafeCopyN<T>( m_pData + nPos + nLength, 
              m_pData + nPos, nOldLength - nPos, true );
            StrUnsafeCopyN<T>( m_pData + nPos, 
              pStr, nLength, false ); // without final 0
            m_nLength = nNewLength;
            // m_pData[m_nLength] = 0 - is set by StrUnsafeCopyN()
        }
        else
        {
            CStrT<T> tempStr;
            if ( tempStr.Append(m_pData + nOffset, nLength) )
            {
                StrUnsafeCopyN<T>( m_pData + nPos + nLength, 
                  m_pData + nPos, nOldLength - nPos, true );
                StrUnsafeCopyN<T>( m_pData + nPos, 
                  tempStr.c_str(), nLength, false ); // w/o final 0
                m_nLength = nNewLength;
                // m_pData[m_nLength] = 0 - is set by StrUnsafeCopyN()
            }
        }
    
    }
  
    return m_pData; // can be NULL
}

template <class T> T* CStrT<T>::Insert(int nPos, const CStrT& Str)
{
    return Insert( nPos, Str.c_str(), Str.length() );
}

template <class T> T* CStrT<T>::Insert(int nPos, const T ch)
{
    if ( nPos < 0 )
        return m_pData; // can be NULL

    if ( nPos >= m_nLength )
        return Append(ch);

    int nOldLength = m_nLength;
    int nNewLength = nOldLength + 1;
    if ( SetSize(nNewLength) )
    {
        StrUnsafeCopyN<T>( m_pData + nPos + 1, 
          m_pData + nPos, nOldLength - nPos, true );
        m_pData[nPos] = ch;
        ++m_nLength;
        // m_pData[m_nLength] = 0 - is set by StrUnsafeCopyN()
    }

    return m_pData; // can be NULL
}

template <class T> bool CStrT<T>::replaceStr(int pos, int count,
  const T* str_new, int str_len)
{
    if ( count == str_len )
    {
        StrUnsafeCopyN<T>( m_pData + pos, str_new, str_len, false );
    }
    else
    {
        if ( count < str_len )
        {
            if ( !SetSize(m_nLength + str_len - count) )
            {
                // can't allocate memory
                return false;
            }
        }
        StrUnsafeCopyN<T>( m_pData + pos + str_len,
          m_pData + pos + count, m_nLength - pos - count, false );
        StrUnsafeCopyN<T>( m_pData + pos, str_new, str_len, false );
        m_nLength += (str_len - count);
        m_pData[m_nLength] = 0;
    }
    return true;
}

template <class T> int CStrT<T>::Replace(const T chOld, const T chNew)
{
    int nReplaces = 0;

    int i = 0;
    while ( i < m_nLength )
    {
        if ( m_pData[i] == chOld )
        {
            m_pData[i] = chNew;
            ++nReplaces;
        }
        ++i;
    }

    return nReplaces;
}

template <class T> int CStrT<T>::Replace(const T* pSubStrOld, const T* pSubStrNew)
{
    int nReplaces = 0;
    
    if ( pSubStrOld && (m_nLength > 0) )
    {
        CStrT<T> tempStrOld;
        int      lenStrOld;
        
        if ( (pSubStrOld >= m_pData) && 
             (pSubStrOld < m_pData + m_nLength) )
        {
            pSubStrOld = tempStrOld.Append(pSubStrOld);
            lenStrOld = tempStrOld.length();
        }
        else
        {
            lenStrOld = GetStrUnsafeLength<T>(pSubStrOld); // pSubStrOld != 0
        }
        
        if ( lenStrOld > 0 )
        {
            CStrT<T> tempStrNew;
            int      lenStrNew;
        
            if ( (pSubStrNew >= m_pData) &&
                 (pSubStrNew < m_pData + m_nLength) )
            {
                pSubStrNew = tempStrNew.Append(pSubStrNew);
                lenStrNew = tempStrNew.length();
            }
            else
            {
                lenStrNew = GetStrSafeLength<T>(pSubStrNew); // pSubStrNew ? 0
            }
            
            int pos = 0;
            while ( (pos = Find(pSubStrOld, pos)) >= 0 )
            {
                if ( !replaceStr(pos, lenStrOld, pSubStrNew, lenStrNew) )
                {
                    break;
                }
                pos += lenStrNew;
                ++nReplaces;
            }
        }
    }
    return nReplaces;
}

template <class T> int CStrT<T>::Replace(int nPos, int nCharacters, 
  const T* pSubStrNew, int nLength )
{
    if ( (nPos >= 0) && (nPos < m_nLength) )
    {
        if ( (nCharacters < 0) || (nCharacters > m_nLength - nPos) )
            nCharacters = m_nLength - nPos;
        if ( nLength < 0 )
            nLength = GetStrSafeLength<T>(pSubStrNew); // pSubStrNew ? 0
        if ( nCharacters || nLength ) {
            if ( replaceStr(nPos, nCharacters, pSubStrNew, nLength) )  
            return 1;
        }
    }
    return 0;
}

template <class T> int CStrT<T>::RFind(const T ch, int nStartPos ) const
{
    if ( nStartPos < m_nLength )
    {
        if ( nStartPos < 0 )
            nStartPos = m_nLength - 1;

        while ( nStartPos >= 0 )
        {
            if ( m_pData[nStartPos] == ch )
            {
                return nStartPos;
            }
            --nStartPos;
        }
    }

    return -1;
}

template <class T> int CStrT<T>::RFind(const T* pStr, int nStartPos ) const
{
    if ( nStartPos < m_nLength )
    {
        const int nStrLen = GetStrSafeLength<T>(pStr);
        if ( nStrLen > 0 )
        {
            if ( (nStartPos < 0) || (nStartPos > m_nLength - nStrLen) )
                nStartPos = m_nLength - nStrLen;

            while ( nStartPos >= 0 )
            {
                if ( StrUnsafeSubCmp<T>(m_pData + nStartPos, pStr) != 0 )
                    --nStartPos;
                else
                    return nStartPos;
            }
        }
    }

    return -1;
}

template <class T> int CStrT<T>::RFind(const CStrT& Str, int nStartPos ) const
{
    return RFind( Str.c_str(), nStartPos );
}

template <class T> bool CStrT<T>::SetAt(int nPos, const T ch)
{
    if ( (nPos >= 0) && (nPos < m_nLength) )
    {
        m_pData[nPos] = ch;
        return true;
    }
    return false;
}

template <class T> bool CStrT<T>::SetLengthValue(int nLength)
{
    if ( (nLength >= 0) && (nLength < m_nMemSize) )
    {
        m_nLength = nLength;
        m_pData[nLength] = 0;
        return true;
    }
    return false;
}

template <class T> bool CStrT<T>::SetSize(int nLength)
{
    if ( nLength < 0 )
        return false;

    if ( (nLength + 1) <= m_nMemSize )
    {
        if ( nLength < m_nLength )
        {
            m_nLength = nLength;
            m_pData[nLength] = 0;
        }
    }
    else
    {
        int nNewMemSize = getAlignedMemSizeStr(nLength + 1);
        T*  pNewData = new T[nNewMemSize];
        if ( !pNewData )
            return false;
    
        if ( m_pData )
        {
            if ( m_nLength > 0 )
                StrUnsafeCopyN<T>( pNewData, m_pData, m_nLength, true );
            delete [] m_pData;
        }
        m_pData = pNewData;
        m_nMemSize = nNewMemSize;
        m_pData[m_nLength] = 0;  // SetSize does not modify the length here
    }
    return true;
}

template <class T> bool CStrT<T>::StartsWith(const T ch) const
{
    if ( (ch != 0) && (ch == GetFirstChar()) )
        return true;
    else
        return false;
}

template <class T> bool CStrT<T>::StartsWith(const T* pStr) const
{
    if ( pStr && pStr[0] && (m_nLength > 0) )
    {
        if ( StrUnsafeSubCmp(m_pData, pStr) == 0 )
            return true;
    }
    return false;
}

template <class T> bool CStrT<T>::StartsWith(const CStrT& Str) const
{
    if ( (Str.m_nLength > 0) && (m_nLength >= Str.m_nLength) )
    {
        if ( StrUnsafeSubCmp(m_pData, Str.m_pData) == 0 )
            return true;
    }
    return false;
}

template <class T> void CStrT<T>::Swap(CStrT& Str)
{
    T*  strData = Str.m_pData;
    int strLength = Str.m_nLength;
    int strMemSize = Str.m_nMemSize;

    Str.m_pData = m_pData;
    Str.m_nLength = m_nLength;
    Str.m_nMemSize = m_nMemSize;

    m_pData = strData;
    m_nLength = strLength;
    m_nMemSize = strMemSize;
}

//----------------------------------------------------------------------------
#endif

