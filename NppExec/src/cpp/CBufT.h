/***********************************************
 *  
 *  CBufT ver. 1.3.1
 *  --------------------------------  
 *  (C) DV, Nov 2006 - Jun 2021
 *  --------------------------------
 *
 *  Template:
 *  >>  CBufT<type T>
 *
 *  Pre-defined types:
 *  >>  CBuf     (the same as CByteBuf, see below)
 *  >>  CByteBuf (Buf of byte_t: unsigned char)
 *  >>  CIntBuf  (Buf of int)
 *  >>  CPtrBuf  (Buf of PTR: void*)
 *
 ***********************************************/

#ifndef _tiny_buf_t_h_
#define _tiny_buf_t_h_
//----------------------------------------------------------------------------

#include "types.h"
#include <utility>

typedef  void*  PTR;

//----------------------------------------------------------------------------

// CONDITION: buf1 != 0 && buf2 != 0 && count > 0
template <class T> int BufUnsafeCmpn( const T* buf1, const T* buf2, unsigned int count )
{
    while ( (--count) && (*buf1 == *buf2) )
    {
        ++buf1;
        ++buf2;
    }
    return (int) ( ((unsigned int) (*buf1)) - ((unsigned int) (*buf2)) );
}

// CONDITION: dst != 0 && src != 0
template <class T> void BufUnsafeCopyn(T* dst, const T* src, unsigned int count)
{
    if ( (dst > src) && (dst < src + count) )
    {
        dst += count;
        src += count;
        while ( count-- )
        {
            *(--dst) = *(--src);
        }
    }
    else
    {
        while ( count-- )
        {
            *(dst++) = *(src++);
        }
    }
}

template <class T> class CBufT
{
private:
  T*    m_pData;
  int   m_nCount;
  int   m_nMemSize;
    
public:
  typedef T value_type;
  
  CBufT();
  CBufT(const CBufT& Buf);
  CBufT(CBufT&& Buf);
  ~CBufT();
  T*       Append(const T* pData, int nCount);
  T*       Append(const CBufT& Buf);
  T*       Append(const T& item);
  T*       Assign(const T* pData, int nCount);
  T*       Assign(const CBufT& Buf);
  T*       Assign(CBufT&& Buf);
  void     Clear();
  int      Compare(const T* pData, int nCount) const; 
             // -1 if GetData() < pData
  int      Compare(const CBufT& Buf) const;       
             // 1 if GetData() > pData;  0 if identical; 
             // 2 if nCount items are identical and nCount < m_nCount
  bool     Delete(int nPos, int nCount = -1);  
             // -1 means all items from nPos 
  int      Find(const T& item, int nStartPos = 0) const;
  int      Find(const T* pData, int nCount, int nStartPos = 0) const;
  int      Find(const CBufT& Buf, int nStartPos = 0) const;
  void     FreeMemory();
  T        GetAt(int nPos) const;
  int      GetCount() const  { return m_nCount; }
  T*       GetData() const  { return m_pData; }  
  int      GetMemSize() const  { return m_nMemSize; }
  T*       Insert(int nPos, const T* pData, int nCount);
  T*       Insert(int nPos, const CBufT& Buf);
  T*       Insert(int nPos, const T& item);
  bool     IsEmpty() const  { return (m_nCount == 0); }
  bool     IsEqual(const T* pData, int nCount) const  { 
               return ( Compare(pData, nCount) == 0 ); 
           }
  bool     IsEqual(const CBufT& Buf) const  {
               return ( Compare(Buf.GetData(), Buf.GetCount()) == 0 ); 
           }
  bool     Reserve(int nCount)  { 
               return ( (nCount > m_nCount) ? SetSize(nCount) : true );
           } // like in STL
  bool     SetAt(int nPos, const T& item);
  bool     SetCountValue(int nCount);
  bool     SetSize(int nCount); // previous buffer content is kept
  void     Swap(CBufT& Buf); // swap buf's data
  int      capacity() const  { return m_nMemSize; }
  T*       data() const  { return m_pData; }
  int      size() const  { return m_nCount; }
  const T& operator[](int nPos) const  { return m_pData[nPos]; }
  T&       operator[](int nPos)  { return m_pData[nPos]; }
  T*       operator=(const CBufT& Buf)  { return Assign(Buf); }
  T*       operator=(CBufT&& Buf)  { return Assign(std::forward<CBufT>(Buf)); }
  T*       operator+=(const CBufT& Buf)  { return Append(Buf); }
  T*       operator+=(const T& item)  { return Append(item); }
  bool     operator==(const CBufT& Buf) const {
               return ( Compare(Buf.GetData(), Buf.GetCount()) == 0 ); 
           }
  bool     operator!=(const CBufT& Buf) const {
               return ( Compare(Buf.GetData(), Buf.GetCount()) != 0 ); 
           }
  bool     operator>(const CBufT& Buf) const {
               return ( Compare(Buf.GetData(), Buf.GetCount()) > 0 ); 
           }
  bool     operator>=(const CBufT& Buf) const {
               return ( Compare(Buf.GetData(), Buf.GetCount()) >= 0 ); 
           }
  bool     operator<(const CBufT& Buf) const {
               return ( Compare(Buf.GetData(), Buf.GetCount()) < 0 ); 
           }
  bool     operator<=(const CBufT& Buf) const {
               return ( Compare(Buf.GetData(), Buf.GetCount()) <= 0 ); 
           }

};

//----------------------------------------------------------------------------

typedef  CBufT<c_base::byte_t>  CBuf;
typedef  CBufT<c_base::byte_t>  CByteBuf;
typedef  CBufT<int>             CIntBuf;
typedef  CBufT<PTR>             CPtrBuf;

//----------------------------------------------------------------------------

static inline int getAlignedMemSizeBuf(int nCount)
{
    int nShift;

    if ( nCount <= 256 )
        nShift = 6;
    else if ( nCount <= 512 )
        nShift = 7;
    else if ( nCount <= 1024 )
        nShift = 8;
    else if ( nCount <= 2048 )
        nShift = 9;
    else
        nShift = 10;

    return ( (1 + (nCount >> nShift)) << nShift );
}

//----------------------------------------------------------------------------

template <class T> CBufT<T>::CBufT() : 
  m_pData(NULL), 
  m_nCount(0), 
  m_nMemSize(0)
{
}

template <class T> CBufT<T>::CBufT(const CBufT& Buf) :
  m_pData(NULL), 
  m_nCount(0), 
  m_nMemSize(0)
{
    Append(Buf);
}

template <class T> CBufT<T>::CBufT(CBufT&& Buf) :
  m_pData(NULL), 
  m_nCount(0), 
  m_nMemSize(0)
{
    Swap(Buf);
}

template <class T> CBufT<T>::~CBufT()
{
    FreeMemory();
}

template <class T> T* CBufT<T>::Append(const T* pData, int nCount)
{
    if ( (!pData) || (nCount <= 0) )
        return m_pData;

    int nOffset = -1;
    if ( (pData >= m_pData) && (pData < m_pData + m_nCount) )
    {
        nOffset = (int) (pData - m_pData);
        if (nCount > m_nCount - nOffset)
            nCount = m_nCount - nOffset;
    }
  
    int nOldCount = m_nCount;
    int nNewCount = nOldCount + nCount;
    if ( SetSize(nNewCount) )
    {
        BufUnsafeCopyn<T>( m_pData + nOldCount, 
          (nOffset < 0) ? pData : (m_pData + nOffset), nCount );
        m_nCount = nNewCount;
    }
    return m_pData;
}

template <class T> T* CBufT<T>::Append(const CBufT& Buf)
{
    return Append( Buf.GetData(), Buf.GetCount() );
}

template <class T> T* CBufT<T>::Append(const T& item)
{
    if ( SetSize(m_nCount + 1) )
    {
        m_pData[m_nCount] = item;
        ++m_nCount;
    }
    return m_pData;
}

template <class T> T* CBufT<T>::Assign(const T* pData, int nCount)
{ 
    if ( pData != m_pData )
    {
        Clear(); 
        // works OK for pData > m_pData && pData < m_pData+nCount
        // due to implementation of Append()
        return Append( pData, nCount ); 
    }
    else
    {
        if ( nCount < m_nCount )
            SetSize(nCount);
        return m_pData;
    }
}

template <class T> T* CBufT<T>::Assign(const CBufT& Buf) 
{ 
    if ( Buf.GetData() != m_pData )
    {
        Clear(); 
        return Append( Buf.GetData(), Buf.GetCount() ); 
    }
    else
        return m_pData;
}

template <class T> T* CBufT<T>::Assign(CBufT&& Buf)
{
    if ( Buf.GetData() != m_pData )
    {
        Clear(); 
        Swap(Buf); 
    }
    return m_pData;
}

template <class T> void CBufT<T>::Clear()
{
    if ( m_pData )
    {
        SetSize(0);
    }
    m_nCount = 0; // must be AFTER SetSize(0)!!!
}

template <class T> int CBufT<T>::Compare(const T* pData, int nCount) const
{
    if ( m_pData == pData )
    {
        if ( m_nCount == m_nCount )
            return 0;
        else if ( m_nCount > nCount )
            return 2;
        else
            return -2;
    }
  
    if ( nCount < 0 )
        return 1;

    if ( (!pData) || (nCount == 0) )
        return ( (m_nCount == 0) ? 0 : 1 );

    if ( !m_pData )
        return -1;
  
    int nResult = BufUnsafeCmpn<T>( m_pData, 
          pData, (m_nCount > nCount) ? nCount : m_nCount );

    if ( (nResult == 0) && (m_nCount != nCount) )
    { 
        nResult = (m_nCount > nCount) ? 2 : (-2);
    }
    return nResult;
}

template <class T> int CBufT<T>::Compare(const CBufT& Buf) const
{
    return Compare( Buf.GetData(), Buf.GetCount() );
}

template <class T> bool CBufT<T>::Delete(int nPos, int nCount )
{
    if ( (m_nCount == 0) || (nPos < 0) || (nCount == 0) )
        return false;

    int nMaxDeleteCount = m_nCount - nPos;
    if ( nMaxDeleteCount <= 0 )
        return false;

    if ( (nCount < 0) || (nCount >= nMaxDeleteCount) )
        return SetSize(nPos);

    BufUnsafeCopyn<T>( m_pData + nPos, 
      m_pData + nPos + nCount, nMaxDeleteCount - nCount );
    m_nCount -= nCount;

    return true;
}

template <class T> int CBufT<T>::Find(const T& item, int nStartPos ) const
{
    if ( nStartPos >= 0 )
    {
        while ( nStartPos < m_nCount )
        {
            if ( m_pData[nStartPos] == item )
            {
                return nStartPos;
            }
            ++nStartPos;
        }
    }
    return -1;
}

template <class T> int CBufT<T>::Find(const T* pData, int nCount, 
                                      int nStartPos ) const
{
    if ( (!pData) || (nCount <= 0) || (nStartPos < 0) )
        return -1;

    while ( nStartPos < m_nCount )
    {
        if ( BufUnsafeCmpn<T>(m_pData + nStartPos, pData, nCount) != 0 )
            ++nStartPos;
        else
            return nStartPos;
    }
    return -1;
}

template <class T> int CBufT<T>::Find(const CBufT& Buf, 
                                      int nStartPos ) const
{
    return Find( Buf.GetData(), Buf.GetCount(), nStartPos );
}

template <class T> void CBufT<T>::FreeMemory()
{
    if ( m_pData )
        delete [] m_pData;
    m_pData = NULL;
    m_nCount = 0;
    m_nMemSize = 0;
}

template <class T> T CBufT<T>::GetAt(int nPos) const
{
    if ( (nPos >= 0) && (nPos < m_nCount) )
        return m_pData[nPos];
    else
        return 0;
}

template <class T> T* CBufT<T>::Insert(int nPos, const T* pData, int nCount)
{
    if ( (nPos < 0) || (!pData) || (nCount <= 0) )
        return m_pData;

    if ( nPos >= m_nCount )
        return Append(pData, nCount);

    int nOffset = -1;
    if ( (pData >= m_pData) && (pData < m_pData + m_nCount) )
    {
        nOffset = (int) (pData - m_pData);
        if ( nCount > m_nCount - nOffset )
            nCount = m_nCount - nOffset;
    }

    int nOldCount = m_nCount;
    int nNewCount = nOldCount + nCount;
    if ( SetSize(nNewCount) )
    {
        if ( nOffset < 0 )
        {
            BufUnsafeCopyn<T>( m_pData + nPos + nCount, 
              m_pData + nPos, nOldCount - nPos );
            BufUnsafeCopyn<T>( m_pData + nPos, pData, nCount );
            m_nCount = nNewCount;
        }
        else
        {
            CBufT<T> tempBuf;
            if ( tempBuf.Append(m_pData + nOffset, nCount) )
            {
                BufUnsafeCopyn<T>( m_pData + nPos + nCount, 
                  m_pData + nPos, nOldCount - nPos );
                BufUnsafeCopyn<T>( m_pData + nPos, tempBuf.GetData(), nCount );
                m_nCount = nNewCount;
            }
        }
    }
  
    return m_pData;
}

template <class T> T* CBufT<T>::Insert(int nPos, const CBufT& Buf)
{
    return Insert( nPos, Buf.GetData(), Buf.GetCount() );
}

template <class T> T* CBufT<T>::Insert(int nPos, const T& item)
{
    if ( nPos < 0 )
        return m_pData;

    if ( nPos >= m_nCount )
        return Append(item);

    int nOldCount = m_nCount;
    int nNewCount = nOldCount + 1;
    if ( SetSize(nNewCount) )
    {
        BufUnsafeCopyn<T>( m_pData + nPos + 1, m_pData + nPos, nOldCount - nPos );
        m_pData[nPos] = item;
        ++m_nCount;
    }

    return m_pData;
}

template <class T> bool CBufT<T>::SetAt(int nPos, const T& item)
{
    if ( (nPos >= 0) && (nPos < m_nCount) )
    {
        m_pData[nPos] = item;
        return true;
    }
    return false;
}

template <class T> bool CBufT<T>::SetCountValue(int nCount)
{
    if ( (nCount >= 0) && (nCount <= m_nMemSize) )
    {
        m_nCount = nCount;
        return true;
    }
    return false;
}

template <class T> bool CBufT<T>::SetSize(int nCount)
{
    if ( nCount < 0 )
        return false;

    if ( nCount <= m_nMemSize )
    {
        if ( nCount < m_nCount )
        {
            m_nCount = nCount;
        }
    }
    else
    {
        int nNewMemSize = getAlignedMemSizeBuf(nCount);
        T*  pNewData = new T[nNewMemSize];
        if ( !pNewData )
            return false;
    
        if ( m_pData )
        {
            if ( m_nCount > 0 )
                BufUnsafeCopyn<T>( pNewData, m_pData, m_nCount );
            delete [] m_pData;
        }
        m_pData = pNewData;
        m_nMemSize = nNewMemSize;
    }
    return true;
}

template <class T> void CBufT<T>::Swap(CBufT& Buf)
{
    T*  bufData = Buf.m_pData;
    int bufCount = Buf.m_nCount;
    int bufMemSize = Buf.m_nMemSize;

    Buf.m_pData = m_pData;
    Buf.m_nCount = m_nCount;
    Buf.m_nMemSize = m_nMemSize;

    m_pData = bufData;
    m_nCount = bufCount;
    m_nMemSize = bufMemSize;
}

//----------------------------------------------------------------------------
#endif

