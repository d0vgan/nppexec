/*
This file is part of NppExec
Copyright (C) 2013 DV <dvv81 (at) ukr (dot) net>

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

#include "CSimpleLogger.h"
#include <stdio.h>


#ifdef UNICODE
class LogFileWriter
{
public:
    LogFileWriter(bool useOwnLock = true) : m_hFile(INVALID_HANDLE_VALUE), m_useOwnLock(useOwnLock)
    {
    }

    ~LogFileWriter()
    {
        Close();
    }

    bool Open(const TCHAR* szFilePathName, bool bAppend = false)
    {
        bool bOpenOK = false;

        Close();

        const DWORD dwCreationDisposition = bAppend ? OPEN_EXISTING : CREATE_ALWAYS;
        const DWORD dwFlagsAndAttributes = FILE_FLAG_WRITE_THROUGH;

        if ( m_useOwnLock )
            m_csFile.Lock();

        m_hFile = ::CreateFileW(szFilePathName, GENERIC_WRITE, FILE_SHARE_READ, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
        if ( (m_hFile == INVALID_HANDLE_VALUE) && bAppend )
        {
            m_hFile = ::CreateFileW(szFilePathName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, dwFlagsAndAttributes, NULL);
        }

        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            m_filePathName = szFilePathName;
            bOpenOK = true;

            if ( bAppend )
            {
                DWORD dwFileSize = ::GetFileSize(m_hFile, NULL);
                ::SetFilePointer(m_hFile, dwFileSize, NULL, FILE_BEGIN);
            }
        }

        if ( m_useOwnLock )
            m_csFile.Unlock();

        return bOpenOK;
    }

    bool Close()
    {
        bool bCloseOK = false;

        if ( m_useOwnLock )
            m_csFile.Lock();

        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            ::FlushFileBuffers(m_hFile);
            if ( ::CloseHandle(m_hFile) != FALSE )
                bCloseOK = true;
            m_hFile = INVALID_HANDLE_VALUE;
            m_filePathName.Clear();
        }

        if ( m_useOwnLock )
            m_csFile.Unlock();

        return bCloseOK;
    }

    tstr GetFilePathName() const
    {
        if ( m_useOwnLock )
        {
            CCriticalSectionLockGuard lock(m_csFile);
            return m_filePathName;
        }

        return m_filePathName;
    }

    bool IsOpen() const
    {
        bool isOpen = false;

        if ( m_useOwnLock )
            m_csFile.Lock();

        if ( m_hFile != INVALID_HANDLE_VALUE )
            isOpen = true;

        if ( m_useOwnLock )
            m_csFile.Unlock();

        return isOpen;
    }

    bool Write(const void* pBuf, size_t nBytes)
    {
        bool bWriteOK = false;

        if ( m_useOwnLock )
            m_csFile.Lock();

        if ( m_hFile != INVALID_HANDLE_VALUE )
        {
            DWORD dwBytesToWrite = static_cast<DWORD>(nBytes);
            DWORD dwBytesWritten = 0;
            if ( ::WriteFile(m_hFile, pBuf, dwBytesToWrite, &dwBytesWritten, NULL) )
            {
                if ( dwBytesToWrite == dwBytesWritten )
                    bWriteOK = true;
            }
        }

        if ( m_useOwnLock )
            m_csFile.Unlock();

        return bWriteOK;
    }

protected:
    mutable CCriticalSection m_csFile;
    HANDLE m_hFile;
    CStrT<TCHAR> m_filePathName;
    bool m_useOwnLock;
};
#else
class LogFileWriter
{
    public:
        LogFileWriter(bool useOwnLock = true) : m_f(0), m_useOwnLock(useOwnLock)
        {
        }

        ~LogFileWriter()
        {
            Close();
        }

        bool Open(const TCHAR* szFilePathName, bool bAppend = false)
        {
            bool bOpenOK = false;

            Close();

            const char* szMode = bAppend ? "a+bc" : "w+bc";
              // c - enable the commit flag for the associated filename
              // so that the contents of the file buffer are written
              // directly to disk if either fflush or _flushall is called.

            if ( m_useOwnLock )
                m_csFile.Lock();

            m_f = ::fopen(szFilePathName, szMode);

            if ( m_f )
            {
                m_filePathName = szFilePathName;
                bOpenOK = true;
            }

            if ( m_useOwnLock )
                m_csFile.Unlock();

            return bOpenOK;
        }

        bool Close()
        {
            bool bCloseOK = false;

            if ( m_useOwnLock )
                m_csFile.Lock();

            if ( m_f )
            {
                if ( ::fclose(m_f) == 0 )
                    bCloseOK = true;
                m_f = 0;
                m_filePathName.Clear();
            }

            if ( m_useOwnLock )
                m_csFile.Unlock();

            return bCloseOK;
        }

        tstr GetFilePathName() const
        {
            if ( m_useOwnLock )
            {
                CCriticalSectionLockGuard lock(m_csFile);
                return m_filePathName;
            }

            return m_filePathName;
        }

        bool IsOpen() const
        {
            bool isOpen = false;
            
            if ( m_useOwnLock )
                m_csFile.Lock();

            if ( m_f )
                isOpen = true;

            if ( m_useOwnLock )
                m_csFile.Unlock();

            return isOpen;
        }

        bool Write(const void* pBuf, size_t nBytes)
        {
            bool bWriteOK = false;

            if ( m_useOwnLock )
                m_csFile.Lock();

            if ( m_f )
            {
                if ( ::fwrite(pBuf, 1, nBytes, m_f) == nBytes )
                {
                    if ( ::fflush(m_f) == 0 )
                        bWriteOK = true;
                }
            }

            if ( m_useOwnLock )
                m_csFile.Unlock();

            return bWriteOK;
        }

    protected:
        mutable CCriticalSection m_csFile;
        FILE* m_f;
        CStrT<TCHAR> m_filePathName;
        bool m_useOwnLock;
};
#endif

CSimpleLogger::CSimpleLogger()
  : m_pFileWriter(NULL)
  , m_pOutputStrFunc(NULL)
  , m_nMode(0)
{
    m_IndentStr = _T("   ");
}

CSimpleLogger::~CSimpleLogger()
{
    if ( m_pFileWriter )
    {
        writeFileLogEnd(m_nMode);
        delete m_pFileWriter;
    }
}

unsigned int CSimpleLogger::getMode() const
{
    CCriticalSectionLockGuard lock(m_csState);
    return m_nMode;
}

CSimpleLogger::sThreadState CSimpleLogger::getThreadState(DWORD dwThreadId) const
{
    {
        CCriticalSectionLockGuard lock(m_csThreadStates);
        auto itr = m_ThreadStates.find(dwThreadId);
        if ( itr != m_ThreadStates.end() ) 
            return itr->second;
    }

    return sThreadState(true, tstr());
}

void CSimpleLogger::writeFileLogStart(unsigned int nMode)
{
  #ifdef UNICODE
    {
        CCriticalSectionLockGuard lock(m_csLogFile);
        m_pFileWriter->Write( L"\xFEFF", 1*sizeof(TCHAR) );
    }
  #endif

    const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    add( sCurrentState(sThreadState(true, tstr()), dwThreadId, nMode & nMask), _T("Log start >>") );
}

void CSimpleLogger::writeFileLogEnd(unsigned int nMode)
{
    const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    add( sCurrentState(sThreadState(true, tstr()), dwThreadId, nMode & nMask), _T("Log exit.") );
}

void CSimpleLogger::Activate(bool bActivate)
{
    const DWORD dwThreadId = ::GetCurrentThreadId();
    {
        CCriticalSectionLockGuard lock(m_csThreadStates);
        auto itr = m_ThreadStates.find(dwThreadId);
        if ( itr != m_ThreadStates.end() )
            itr->second.IsActive = bActivate;
        else
            m_ThreadStates.emplace( dwThreadId, sThreadState(bActivate, tstr()) );
    }
}

bool CSimpleLogger::IsActive() const
{
    bool bActive = true;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    {
        CCriticalSectionLockGuard lock(m_csThreadStates);
        auto itr = m_ThreadStates.find(dwThreadId);
        if ( itr != m_ThreadStates.end() )
            bActive = itr->second.IsActive;
    }
    return bActive;
}

void CSimpleLogger::Add(const TCHAR* str)
{
    const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
    const unsigned int nMode = getMode() & nMask;
    if ( nMode != 0 )
    {
        const DWORD dwThreadId = ::GetCurrentThreadId();
        const sThreadState threadState = getThreadState(dwThreadId);
        if ( threadState.IsActive )
        {
            add( sCurrentState(threadState, dwThreadId, nMode), str );
        }
    }
}

void CSimpleLogger::Add_WithoutOutput(const TCHAR* str)
{
    const unsigned int nMask = lfStrList | lfLogFile;
    const unsigned int nMode = getMode() & nMask;
    if ( nMode != 0 )
    {
        const DWORD dwThreadId = ::GetCurrentThreadId();
        const sThreadState threadState = getThreadState(dwThreadId);
        if ( threadState.IsActive )
        {
            add( sCurrentState(threadState, dwThreadId, nMode), str );
        }
    }
}

void CSimpleLogger::add(const sCurrentState& state, const TCHAR* str)
{
    tstr S;
    S.Reserve(1020); // note: always use "+=" after Reserve(), do not use "=" !
    S += state.ThreadState.LogIndent;
    S += str;

  #if SIMPLELOGGER_USE_STRLIST
    if ( (state.nMode & lfStrList) != 0 )
    {
        m_StrList.Add(S);
    }
  #endif

    if ( (state.nMode & lfOutputFunc) != 0 )
    {
        OUTPUTFUNC pOutputStrFunc;
        {
            CCriticalSectionLockGuard lock(m_csState);
            pOutputStrFunc = m_pOutputStrFunc;
        }

        if ( pOutputStrFunc )
        {
            pOutputStrFunc( S.c_str(), S.length() );
        }
    }

    if ( (state.nMode & lfLogFile) != 0 )
    {
        SYSTEMTIME t;
        TCHAR szTimeAndThreadId[48];

        ::GetLocalTime(&t);
        ::wsprintf(szTimeAndThreadId, _T("%02d:%02d:%02d.%03d  %u  "), t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, state.dwThreadId);
        S.Insert(0, szTimeAndThreadId);
        S += _T("\r\n"); // new line

        {
            CCriticalSectionLockGuard lock(m_csLogFile);
            m_pFileWriter->Write( S.c_str(), S.length()*sizeof(TCHAR) );
        }
    }
}

void CSimpleLogger::AddEx(const TCHAR* fmt, ...)
{
    if ( fmt && fmt[0] )
    {
        const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
        const unsigned int nMode = getMode() & nMask;
        if ( nMode != 0 )
        {
            const DWORD dwThreadId = ::GetCurrentThreadId();
            const sThreadState threadState = getThreadState(dwThreadId);
            if ( threadState.IsActive )
            {
                va_list argList;

                va_start(argList, fmt);
                addex( sCurrentState(threadState, dwThreadId, nMode), fmt, argList );
                va_end(argList);
            }
        }
    }
}

void CSimpleLogger::AddEx_WithoutOutput(const TCHAR* fmt, ...)
{
    if ( fmt && fmt[0] )
    {
        const unsigned int nMask = lfStrList | lfLogFile;
        const unsigned int nMode = getMode() & nMask;
        if ( nMode != 0 )
        {
            const DWORD dwThreadId = ::GetCurrentThreadId();
            const sThreadState threadState = getThreadState(dwThreadId);
            if ( threadState.IsActive )
            {
                va_list argList;

                va_start(argList, fmt);
                addex( sCurrentState(threadState, dwThreadId, nMode), fmt, argList );
                va_end(argList);
            }
        }
    }
}

void CSimpleLogger::addex(const sCurrentState& state, const TCHAR* fmt, va_list argList)
{
    TCHAR   str_fmt[2048];
    TCHAR   str[2048];
    int     i = 0, j = 0;

    while ( fmt[i] )
    {
        if ( (fmt[i] == _T('%')) && (fmt[i+1] == _T('s')) )
        {
            lstrcpy( str_fmt + j, _T("%.960s") );
            j += 6; // len of "%.960s"
            i += 2; // len of "%s"
        }
        else
        {
            str_fmt[j++] = fmt[i++];
        }
    }
    str_fmt[j] = 0;

    ::wvsprintf(str, str_fmt, argList);
    str[2048 - 1] = 0;
    add(state, str);
}

void CSimpleLogger::Clear()
{
    const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
    const unsigned int nMode = getMode() & nMask;
    if ( nMode != 0 )
    {
        const DWORD dwThreadId = ::GetCurrentThreadId();

      #if SIMPLELOGGER_USE_STRLIST
        bool bActive;
        {
            CCriticalSectionLockGuard lock(m_csThreadStates);
            auto itr = m_ThreadStates.find(dwThreadId);
            if ( itr != m_ThreadStates.end() )
            {
                sThreadState& threadState = itr->second;
                bActive = threadState.IsActive;
                threadState.IsActive = false;
                threadState.LogIndent.Clear();
            }
            else
            {
                bActive = true;
                m_ThreadStates.emplace( dwThreadId, sThreadState(false, tstr()) );
            }
        }

        {
            CCriticalSectionLockGuard lock(m_csState);
            m_StrList.Clear();
        }

        if ( bActive )
        {
            CCriticalSectionLockGuard lock(m_csThreadStates);
            auto itr = m_ThreadStates.find(dwThreadId);
            if ( itr != m_ThreadStates.end() )
            {
                sThreadState& threadState = itr->second;
                threadState.IsActive = true;
            }
        }
      #else
        {
            CCriticalSectionLockGuard lock(m_csThreadStates);
            auto itr = m_ThreadStates.find(dwThreadId);
            if ( itr != m_ThreadStates.end() )
            {
                sThreadState& threadState = itr->second;
                threadState.LogIndent.Clear();
            }
        }
      #endif
    }
}

void CSimpleLogger::DecIndentLevel()
{
    const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
    const unsigned int nMode = getMode() & nMask;
    if ( nMode != 0 )
    {
        const DWORD dwThreadId = ::GetCurrentThreadId();

        {
            CCriticalSectionLockGuard lock(m_csThreadStates);
            auto itr = m_ThreadStates.find(dwThreadId);
            if ( itr != m_ThreadStates.end() )
            {
                sThreadState& threadState = itr->second;
                if ( threadState.IsActive && !threadState.LogIndent.IsEmpty() )
                {
                    int n = threadState.LogIndent.length() - m_IndentStr.length();
                    if ( n > 0 )
                        threadState.LogIndent.SetSize(n);
                    else
                        threadState.LogIndent.Clear();
                }
            }
        }
    }
}

void CSimpleLogger::IncIndentLevel()
{
    const unsigned int nMask = lfStrList | lfLogFile | lfOutputFunc;
    const unsigned int nMode = getMode() & nMask;
    if ( nMode != 0 )
    {
        const DWORD dwThreadId = ::GetCurrentThreadId();

        {
            CCriticalSectionLockGuard lock(m_csThreadStates);
            auto itr = m_ThreadStates.find(dwThreadId);
            if ( itr != m_ThreadStates.end() )
            {
                sThreadState& threadState = itr->second;
                if ( threadState.IsActive )
                {
                    threadState.LogIndent += m_IndentStr;
                }
            }
            else
            {
                m_ThreadStates.emplace( dwThreadId, sThreadState(true, m_IndentStr) );
            }
        }
    }
}

tstr CSimpleLogger::GetIndentStr() const
{
    CCriticalSectionLockGuard lock(m_csThreadStates);
    return m_IndentStr;
}

void CSimpleLogger::SetIndentStr(const TCHAR* str)
{
    CCriticalSectionLockGuard lock(m_csThreadStates);
    m_IndentStr = str;
}

#if SIMPLELOGGER_USE_STRLIST
tstr CSimpleLogger::GetLastLine() const
{
    {
        CCriticalSectionLockGuard lock(m_csState);
        CListItemT<tstr>* pLastItem = m_StrList.GetLast();
        if ( pLastItem )
        {
            return pLastItem->GetItem();
        }
    }

    return tstr();
}
#endif

#if SIMPLELOGGER_USE_STRLIST
CListT<tstr> CSimpleLogger::GetStrList() const
{
    CCriticalSectionLockGuard lock(m_csState);
    return m_StrList;
}
#endif

bool CSimpleLogger::IsOutputMode() const
{
    CCriticalSectionLockGuard lock(m_csState);
    return (((m_nMode & lfOutputFunc) != 0) && (m_pOutputStrFunc != NULL));
}

void CSimpleLogger::SetOutputMode(bool bDirectOutput, OUTPUTFUNC pOutputStrFunc )
{
    CCriticalSectionLockGuard lock(m_csState);
    if ( bDirectOutput )
    {
        m_pOutputStrFunc = pOutputStrFunc;
        m_nMode |= lfOutputFunc;
    }
    else
    {
        m_nMode &= ~lfOutputFunc;
        m_pOutputStrFunc = NULL;
    }
}

tstr CSimpleLogger::GetLogFile() const 
{
    {
        CCriticalSectionLockGuard lock(m_csLogFile);
        if ( m_pFileWriter )
            return m_pFileWriter->GetFilePathName();
    }

    return tstr();
}

bool CSimpleLogger::IsLogFileOpen() const
{
    const unsigned int nMode = getMode();
    return ((nMode & lfLogFile) != 0);
}

bool CSimpleLogger::SetLogFile(const TCHAR* logFilePathName)
{
    bool isLogFileOpen;
    {
        CCriticalSectionLockGuard lock(m_csLogFile);
        if ( !m_pFileWriter )
        {
            m_pFileWriter = new LogFileWriter(false);
        }
        isLogFileOpen = (m_pFileWriter && m_pFileWriter->Open(logFilePathName));
    }

    if ( isLogFileOpen )
    {
        unsigned int nMode;
        {
            CCriticalSectionLockGuard lock(m_csState);
            m_nMode |= lfLogFile;
            nMode = m_nMode;
        }
        writeFileLogStart(nMode);
    }
    else
    {
        CCriticalSectionLockGuard lock(m_csState);
        m_nMode &= ~lfLogFile;
    }

    return isLogFileOpen;
}

void CSimpleLogger::OnThreadStarted()
{
    const DWORD dwThreadId = ::GetCurrentThreadId();
    {
        CCriticalSectionLockGuard lock(m_csThreadStates);
        m_ThreadStates[dwThreadId] = sThreadState(true, tstr());
    }
}

void CSimpleLogger::OnThreadFinished()
{
    const DWORD dwThreadId = ::GetCurrentThreadId();
    {
        CCriticalSectionLockGuard lock(m_csThreadStates);
        m_ThreadStates.erase(dwThreadId);
    }
}
