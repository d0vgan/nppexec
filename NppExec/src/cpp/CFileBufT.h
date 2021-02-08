/***********************************************
 *  
 *  CFileBufT ver. 1.0.5
 *  --------------------------------  
 *  (C) DV, Nov 2006 - Oct 2008
 *  --------------------------------
 *
 *  Template:
 *  >>  CFileBufT<type T>
 *
 *  Pre-defined types:
 *  >>  CFileBuf  (Buf of char)
 *  >>  CWFileBuf (Buf of whar_t)
 *
 ***********************************************/

#ifndef _file_buf_t_h_
#define _file_buf_t_h_
//----------------------------------------------------------------------------
#include "CBufT.h"
#include "CStrT.h"
#include <stdio.h>
#include <wchar.h>
#include "../encodings/SysUniConv.h"


template <class T> class CFileBufT
{
private:
  CBufT<T>  m_buf;
  int       m_nLineStartPos;
  int       m_nLineLength;
  int       m_nLineNumber;

  int       getNextLineStartPos(int nLineStartPos, int nLineLength);
  int       getLineLength(int nLineStartPos);
  int       getPrevLineStartPos(int nPrevLineEndPos);
  void      setLineStartPos(int pos);

public:
  typedef T value_type;

#ifdef UNICODE
  typedef HANDLE FilePtr;
#else
  typedef FILE* FilePtr;
#endif

  CFileBufT();
  ~CFileBufT();
  int       GetBufCount() const  { return m_buf.GetCount(); }
  T*        GetBufData() const  { return m_buf.GetData(); }
  CBufT<T>* GetBufPtr() const  { return ( (CBufT<T>*) &m_buf ); }
  int       GetLine(CStrT<T>& strLine, bool bGoToNextLine = true); // \r\n or \n
  int       GetLineLength() const  { return m_nLineLength; }  //  \r\n or \n
  int       GetLineNumber() const  { return m_nLineNumber; }  //  \r\n or \n
  int       GetLineStartPos() const  { return m_nLineStartPos; }  //  \r\n or \n
  bool      GoToFirstLine(); //  \r\n or \n
  bool      GoToLastLine();  //  \r\n or \n
  bool      GoToNextLine();  //  \r\n or \n
  bool      GoToPrevLine();  //  \r\n or \n
  T*        LoadFromFile(const TCHAR* szFile, bool bTextFile = true, int nUtf8DetectLength = 16384);
  //T*        LoadFromFile(const wchar_t* szFile, bool bTextFile = true);
  bool      ReplaceCurrentLine(const T* strNewLine, int nLength = -1);
  bool      ReplaceCurrentLine(const CStrT<T>& strNewLine);
  bool      SaveToFile(const TCHAR* szFile, bool bTextFile = true);
  //bool      SaveToFile(const wchar_t* szFile);

public:

  #ifdef UNICODE

    static FilePtr openfile(const wchar_t* szFilePath, bool bWrite = false)
    {
        DWORD dwDesiredAccess = bWrite ? GENERIC_WRITE : GENERIC_READ;
        DWORD dwShareMode = bWrite ? FILE_SHARE_READ : (FILE_SHARE_READ | FILE_SHARE_WRITE);
        DWORD dwCreationDisposition = bWrite ? CREATE_ALWAYS : OPEN_EXISTING;
        DWORD dwFlagsAndAttributes = bWrite ? FILE_FLAG_WRITE_THROUGH : 0;

        FilePtr hFile = ::CreateFileW(szFilePath, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
        {
            hFile = NULL;
        }

        return hFile;
    }

    static bool closefile(FilePtr hFile, bool bFlush = false)
    {
        if (hFile != NULL)
        {
            if (bFlush)
                ::FlushFileBuffers(hFile);

            if (::CloseHandle(hFile))
                return true;
        }
        return false;
    }

    static long getsize(FilePtr hFile)
    {
        long size = 0;
        if (hFile != NULL) {
            size = static_cast<long>(::GetFileSize(hFile, NULL));
        }
        return size;
    }

    static bool readfile(FilePtr hFile, void* pBuf, size_t nBytes)
    {
        bool bRet = false;
        DWORD dwBytesToRead = static_cast<DWORD>(nBytes);
        DWORD dwBytesActuallyRead = 0;

        if (::ReadFile(hFile, pBuf, dwBytesToRead, &dwBytesActuallyRead, NULL))
        {
            if (dwBytesToRead == dwBytesActuallyRead)
                bRet = true;
        }

        return bRet;
    }

    static bool writefile(FilePtr hFile, const void* pBuf, size_t nBytes)
    {
        bool bRet = false;
        DWORD dwBytesToWrite = static_cast<DWORD>(nBytes);
        DWORD dwBytesActuallyWritten = 0;

        if (::WriteFile(hFile, pBuf, dwBytesToWrite, &dwBytesActuallyWritten, NULL))
        {
            if (dwBytesToWrite == dwBytesActuallyWritten)
                bRet = true;
        }

        return bRet;
    }

  #else

    static FilePtr openfile(const char* szFilePath, bool bWrite = false)
    {
        const char* mode_readonly = "rb";
        const char* mode_readwrite = "w+bc";

        return fopen(szFilePath, bWrite ? mode_readwrite : mode_readonly);
    }

    static bool closefile(FilePtr f, bool bFlush = false)
    {
        bool bClose = false;
        if (f != NULL)
        {
            if (bFlush)
                fflush(f);

            if (fclose(f) == 0)
                bClose = true;
        }
        return bClose;
    }

    static long getsize(FilePtr f)
    {
        long size = 0;
        if (f != NULL) {
            // move to file's end
            if (fseek(f, 0, SEEK_END) == 0) {
                size = ftell(f);
                // move to file's beginning
                fseek(f, 0, SEEK_SET); 
            }
        }
        return size;
    }

    static bool readfile(FilePtr f, void* pBuf, size_t nBytes)
    {
        size_t count = fread(pBuf, 1, nBytes, f);
        return (count == nBytes);
    }

    static bool writefile(FilePtr f, const void* pBuf, size_t nBytes)
    {
        size_t count = fwrite(pBuf, 1, nBytes, f);
        return (count == nBytes);
    }
  #endif

};

//----------------------------------------------------------------------------

typedef  CFileBufT<char>     CFileBuf;
typedef  CFileBufT<wchar_t>  CWFileBuf;
 
/*
  //------------------------------------------//
  //  EXAMPLE:
  //------------------------------------------//
  CFileBuf fbuf;
  if (fbuf.LoadFromFile("file.txt") != NULL)
  {
    CStr S;
    while (fbuf.GetLine(S) >= 0)
    {
      // do something with S
    }
  }
  //------------------------------------------//
*/

//----------------------------------------------------------------------------


template <class T> CFileBufT<T>::CFileBufT() : m_nLineStartPos(0), 
  m_nLineLength(0), m_nLineNumber(0)
{                     
}

template <class T>CFileBufT<T>::~CFileBufT()
{
  m_buf.FreeMemory();
}

template <class T> int CFileBufT<T>::getLineLength(int nLineStartPos)
{
  int nMaxPos;
  int i;

  nMaxPos = m_buf.GetCount()-1;
  i = nLineStartPos;
  if (i > nMaxPos)
    return (-1);

  while (i <= nMaxPos)
  {
    if (m_buf[i] == 0x0A)
    {
      if (i > 0 && m_buf[i-1] == 0x0D)
      {
        i--; // skip 0x0D
      }
      break;
    }
    else
    {
      i++;
    }
  }
  return (i - nLineStartPos); 
}

template <class T> int CFileBufT<T>::getNextLineStartPos(int nLineStartPos, int nLineLength)
{
  int nMaxPos;
  int i;

  nMaxPos = m_buf.GetCount()-1;
  i = nLineStartPos + nLineLength;
  if (i > nMaxPos)
    return (-1);

  if (m_buf[i] == 0x0D)
  {
    i += 1; // after 0x0D
  }
  i += 1; // after 0x0A

  if (i > nMaxPos)
    return (-1);

  return i;
}

template <class T> int CFileBufT<T>::getPrevLineStartPos(int nPrevLineEndPos)
{
  int  i;
  
  for (i = nPrevLineEndPos; i >= 0 && m_buf[i] != 0x0A; i--)
  {
  }

  return (i + 1);
}

template <class T> void CFileBufT<T>::setLineStartPos(int pos)
{
  m_nLineStartPos = pos;
}

template<> void CFileBufT<wchar_t>::setLineStartPos(int pos);

template <class T> int CFileBufT<T>::GetLine(CStrT<T>& strLine, bool bGoToNextLine )
{
  int i;
  int nLen;

  strLine.SetSize(0); // ensure the string is empty
  nLen = GetLineLength();
  if (nLen >= 0)
  {
    strLine.Reserve(nLen);
    for (i = 0; i < nLen; i++)
    {
      strLine += (T) m_buf[m_nLineStartPos+i];
    }
    if (bGoToNextLine)
    {
      GoToNextLine();
    }
    return strLine.length();
  }
  return (-1);
}

template <class T> bool CFileBufT<T>::GoToFirstLine()
{
  m_nLineNumber = 1;
  setLineStartPos( 0 );
  m_nLineLength = getLineLength(m_nLineStartPos);
  return (m_nLineLength >= 0);
}

template <class T> bool CFileBufT<T>::GoToLastLine()
{
  int nBufCount = m_buf.GetCount();
  if (nBufCount == 0)
    return false;

  m_nLineNumber = -1; // 1st line from the file's end
  setLineStartPos( getPrevLineStartPos(nBufCount-1) );
  m_nLineLength = nBufCount - m_nLineStartPos;
  return true;
}

template <class T> bool CFileBufT<T>::GoToNextLine()
{
  m_nLineNumber++;
  setLineStartPos( getNextLineStartPos(m_nLineStartPos, m_nLineLength) );
  if (m_nLineStartPos < 0)
  {
    m_nLineLength = -1;
    return false;
  }
  m_nLineLength = getLineLength(m_nLineStartPos);
  return (m_nLineLength >= 0);
}

template <class T> bool CFileBufT<T>::GoToPrevLine()
{
  int nPrevLineStartPos = -1;
  int nPrevLineEndPos;
  int nPrevLineLength;

  m_nLineNumber--;
  nPrevLineEndPos = m_nLineStartPos;
  if (nPrevLineEndPos <= 0)
  {
    m_nLineLength = -1;
    return false;
  }

  nPrevLineEndPos--;  // 0x0A position
  if (nPrevLineEndPos > 0)
  { 
    nPrevLineEndPos--;  // before 0x0A
    if (nPrevLineEndPos >= 0 && m_buf[nPrevLineEndPos] == 0x0D)
      nPrevLineEndPos--;  // before 0x0D

    nPrevLineStartPos = getPrevLineStartPos(nPrevLineEndPos);
    nPrevLineLength = 1 + nPrevLineEndPos - nPrevLineStartPos;
  }
  else
  {
    nPrevLineStartPos = 0;
    nPrevLineLength = 0;
  }

  setLineStartPos( nPrevLineStartPos );
  m_nLineLength = nPrevLineLength;

  return true;
}

template <class T> T* CFileBufT<T>::LoadFromFile(const TCHAR* szFile, bool bTextFile, int /*nUtf8DetectLength*/)
{
  // initial values when LoadFromFile fails
  m_nLineNumber = 1;
  m_nLineStartPos = 0;
  m_nLineLength = -1;

  m_buf.Clear();

  FilePtr f = openfile(szFile);
  if (f == NULL)
    return NULL;

  long nSize = getsize(f);
  if (!m_buf.Reserve(1 + nSize/sizeof(T)))
  {
    closefile(f);
    return NULL;
  }

  T* p = (T*) m_buf.GetData();
  if (!readfile(f, p, nSize))
  {
    closefile(f);
    return NULL;
  }

  closefile(f);
  
  m_buf.SetCountValue(nSize/sizeof(T));

  if (bTextFile)
  {
    GoToFirstLine();
  }
  return m_buf.GetData();
}

template<> wchar_t* CFileBufT<wchar_t>::LoadFromFile(const TCHAR* szFile, bool bTextFile, int nUtf8DetectLength);

template <class T> bool CFileBufT<T>::ReplaceCurrentLine(const T* strNewLine, int nLength )
{
  if (m_nLineStartPos >= 0 && m_nLineLength >= 0)
  {
    if (strNewLine == NULL || nLength < 0)
      nLength = GetStrSafeLength(strNewLine);

    m_buf.Delete(m_nLineStartPos, m_nLineLength);
    m_buf.Insert(m_nLineStartPos, strNewLine, nLength);
    m_nLineLength = nLength;
    return true;
  }
  return false;
}

template <class T> bool CFileBufT<T>::ReplaceCurrentLine(const CStrT<T>& strNewLine)
{
  return ReplaceCurrentLine(strNewLine.c_str(), strNewLine.length());
}

template <class T> bool CFileBufT<T>::SaveToFile(const TCHAR* szFile, bool bTextFile)
{
  FilePtr f = openfile(szFile, true);
  if (f == NULL)
    return false;

  if ( bTextFile && sizeof(T) == sizeof(wchar_t) )
  {
    // Unicode
    if (!writefile(f, "\xFF\xFE", 2))
    {
      closefile(f);
      return false;
    }
  }
  
  if (!writefile(f, m_buf.GetData(), m_buf.GetCount()*sizeof(T)))
  {
    closefile(f);
    return false;
  }

  closefile(f, true);
  return true;
}

//----------------------------------------------------------------------------
#endif

