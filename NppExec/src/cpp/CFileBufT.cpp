#include "CFileBufT.h"

// Checks utf-8 characters consisting of up to 6 utf-8 bytes.
// FYI, the BOM [0xEF, 0xBB, 0xBF] satisfies these requirements
// Note:
// This function expects the following to be _always_ true:
//  s <= sEndLimit, the range is [s ... sEndLimit)
//  sEndLimit <= sEndTotal, we must not cross the end of the sequence
static bool IsSequenceUtf8(const unsigned char* s, const unsigned char* const sEndLimit, const unsigned char* const sEndTotal)
{
  bool containsUtf8 = false; // contains utf-8 leading byte(s)
  bool errorUtf8 = false;    // error: unexpected byte in utf-8 sequence
  unsigned int n = 0;        // number of continuation bytes

  while ( (!errorUtf8) && (s != sEndLimit) )
  {
    unsigned char uch = *s;
    if ( n == 0 )
    {
      if ( uch >= 0xC0 )
      {
        containsUtf8 = true;
        if ( uch < 0xE0 )
          n = 1;
        else if ( uch < 0xF0 )
          n = 2;
        else if ( uch < 0xF8 )
          n = 3;
        else if ( uch < 0xFC )
          n = 4;
        else if ( uch < 0xFE )
          n = 5;
        else
          n = 6;
      }
      else if ( uch >= 0x80 )
        errorUtf8 = true;
    }
    else
    {
      if ( uch >= 0x80 && uch < 0xC0 )
        --n;
      else
        errorUtf8 = true;
    }
    ++s;
  }

  if ( containsUtf8 && !errorUtf8 )
  {
    if ( (n == 0) || (sEndLimit != sEndTotal) )
      return true;
  }
  return false;
}

template<> void CFileBufT<wchar_t>::setLineStartPos(int pos)
{
    m_nLineStartPos = pos;
    if ( pos == 0 )
        m_nLineStartPos = 1; // skip 0xFEFF
}

template<> wchar_t* CFileBufT<wchar_t>::LoadFromFile(const TCHAR* szFile, bool bTextFile, int nUtf8DetectLength)
{
  FILE* f;
  
  // initial values when LoadFromFile fails
  m_nLineNumber = 1;
  m_nLineStartPos = 0;
  m_nLineLength = -1;

  m_buf.Clear();

  f = openfile(szFile);
  if (f == NULL)
    return NULL;

  long nSize = getsize(f);
  if (!m_buf.Reserve(1 + nSize/sizeof(wchar_t)))
  {
    closefile(f);
    return NULL;
  }

  unsigned char* p = (unsigned char *) m_buf.GetData();
  if (!readfile(f, p, nSize))
  {
    closefile(f);
    return NULL;
  }

  closefile(f);

  m_buf.SetCountValue(nSize/sizeof(wchar_t));

  if ( bTextFile )
  {
    bool bIsWideCharText = false;
    if ( (nSize >= 2) && ((nSize % 2) == 0) )
    {
      if ( m_buf[0] == 0xFEFF )
      {
        // UCS-2 LE
        bIsWideCharText = true;
      }
      else if ( m_buf[0] == 0xFFFE )
      {
        // UCS-2 BE
        bIsWideCharText = true;
        for ( int i = 0; i < m_buf.GetCount(); i++ )
        {
          const wchar_t wch = m_buf[i];
          m_buf[i] = ((wch >> 8) & 0x00FF) + (((wch & 0x00FF) << 8) & 0xFF00);
        }
      }
    }

    if ( !bIsWideCharText )
    {
      CBufT<wchar_t> wbuf;

      if ( !wbuf.Reserve(nSize + 2) )
      {
        return NULL;
      }
      
      wbuf[0] = 0xFEFF;
      wbuf[1] = 0; // just in case :)

      if ( (nUtf8DetectLength > nSize) || (nUtf8DetectLength < 0) )
        nUtf8DetectLength = nSize;

      int count;
      if ( (nSize >= 3) &&
           (p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) )
      {
        // UTF-8 with BOM
        count = SysUniConv::UTF8ToUnicode(
                  (wchar_t *) wbuf.GetData() + 1, nSize + 1,
                  (const char *) p + 3, nSize - 3);
      }
      else if ( IsSequenceUtf8(p, p + nUtf8DetectLength, p + nSize) )
      {
        // UTF-8 without BOM
        count = SysUniConv::UTF8ToUnicode(
                  (wchar_t *) wbuf.GetData() + 1, nSize + 1,
                  (const char *) p, nSize);
      }
      else
      {
        count = SysUniConv::MultiByteToUnicode( 
                  (wchar_t *) wbuf.GetData() + 1, nSize + 1,
                  (const char *) p, nSize);
      }
      wbuf.SetCountValue(count + 1);

      m_buf.Swap(wbuf);
    }
    
    m_nLineStartPos = 1;
  }

  if (bTextFile)
  {
    GoToFirstLine();
  }
  return m_buf.GetData();
}
