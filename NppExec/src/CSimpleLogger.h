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

#ifndef _simple_logger_h_
#define _simple_logger_h_
//---------------------------------------------------------------------------
#include "base.h"
#include <TCHAR.h>
#include "cpp/CStrT.h"
#include "cpp/CListT.h"
#include "NppExecHelpers.h"
#include <map>

#define SIMPLELOGGER_USE_STRLIST 0

class LogFileWriter;

class CSimpleLogger
{
public:
    typedef CStrT<TCHAR> tstr; 
    typedef void (*OUTPUTFUNC)(const TCHAR* str, int len);

public:
    enum eLogFlags : unsigned int
    {
      #if SIMPLELOGGER_USE_STRLIST
        lfStrList    = 0x10,
      #else
        lfStrList    = 0x00,
      #endif
        lfLogFile    = 0x20,
        lfOutputFunc = 0x40
    };

protected:
    struct sThreadState
    {
        bool IsActive;
        tstr LogIndent;

        sThreadState() : IsActive(true)
        {
        }

        sThreadState(bool isActive, const tstr& logIndent) : IsActive(isActive), LogIndent(logIndent)
        {
        }
    };

    struct sCurrentState
    {
        const sThreadState& ThreadState;
        const DWORD dwThreadId;
        const unsigned int nMode;

        sCurrentState(const sThreadState& threadState, const DWORD threadId, const unsigned int mode)
          : ThreadState(threadState), dwThreadId(threadId), nMode(mode)
        {
        }

        sCurrentState& operator=(const sCurrentState&) = delete;
    };

protected:
    // critical sections are created first and destroyed last...
    mutable CCriticalSection m_csState;
    mutable CCriticalSection m_csThreadStates;
    mutable CCriticalSection m_csLogFile;
    // data...
    LogFileWriter* m_pFileWriter;
    OUTPUTFUNC     m_pOutputStrFunc;
    unsigned int   m_nMode;
    tstr           m_IndentStr;
  #if SIMPLELOGGER_USE_STRLIST
    CListT<tstr>   m_StrList;
  #endif
    std::map<DWORD, sThreadState> m_ThreadStates;

    unsigned int getMode() const;
    sThreadState getThreadState(DWORD dwThreadId) const;

    void writeFileLogStart(unsigned int nMode);
    void writeFileLogEnd(unsigned int nMode);

    void add(const sCurrentState& state, const TCHAR* str);
    void addex(const sCurrentState& state, const TCHAR* fmt, va_list argList);

public:
    CSimpleLogger();
    ~CSimpleLogger();

    void Activate(bool bActivate);
    void Add(const TCHAR* str);
    void Add_WithoutOutput(const TCHAR* str);
    void AddEx(const TCHAR* fmt, ...);
    void AddEx_WithoutOutput(const TCHAR* fmt, ...);
    void Clear();
    void DecIndentLevel();
    void IncIndentLevel();
    tstr GetIndentStr() const;
    void SetIndentStr(const TCHAR* str);
  #if SIMPLELOGGER_USE_STRLIST
    tstr GetLastLine() const; // returns a copy
    CListT<tstr> GetStrList() const; // returns a copy
  #endif
    bool IsOutputMode() const;
    void SetOutputMode(bool bDirectOutput, OUTPUTFUNC pOutputStrFunc = NULL);

    tstr GetLogFile() const;
    bool IsLogFileOpen() const;
    bool SetLogFile(const TCHAR* logFilePathName);

    void OnThreadStarted();
    void OnThreadFinished();
};

//---------------------------------------------------------------------------
#endif

