#ifndef _child_process_io_h_
#define _child_process_io_h_
//--------------------------------------------------------------------

#include "base.h"
#include "cpp/CStrT.h"
#include "cpp/CListT.h"
#include "NppExecHelpers.h"

class CNppExec;
class CChildProcess;

struct CChildProcessStdinItem
{
    CStrT<char> m_Data;
    bool        m_bFlush;

    CChildProcessStdinItem() : m_bFlush(false) { }
    CChildProcessStdinItem(const CStrT<char>& data, bool bFlush) : m_Data(data), m_bFlush(bFlush) { }
};

struct CChildProcessOutputChunk
{
    CStrT<char> m_Data;
    bool        m_bStderr;

    CChildProcessOutputChunk() : m_bStderr(false) { }
};

class CChildProcessIo
{
    friend class CChildProcess;

    public:
        CChildProcessIo();
        ~CChildProcessIo();

        void reset();

        bool createPipes(CNppExec* pNppExec);
        bool createPseudoConsole(CNppExec* pNppExec);
        bool resizePseudoConsole(CNppExec* pNppExec);
        void configureStartupInfo(STARTUPINFOEX& si, DWORD& dwCreationFlags);
        void onChildProcessStarted();

        void closePipes();
        void closePseudoConsole();
        void closeAll();

        HANDLE getStdInWritePipe() const;
        HANDLE getStdOutReadPipe() const;
        HANDLE getStdErrReadPipe() const;
        bool   isPseudoCon() const;
        bool   hasSeparateStderrPipe() const;

        bool startStdoutReader(CChildProcess* pOwner);
        void stopStdoutReader();
        bool isStdoutReaderActive() const;

        bool popStdoutChunk(CStrT<char>& chunk);
        bool popOutputChunk(CStrT<char>& chunk, bool& bStderr);

        // P1.4: after child exit / reader stop — ReadFile until pipes dry (no PeekNamedPipe).
        void drainStdoutToQueue();

        // P1.6: encoded bytes queued from WriteInput; writer thread calls WriteFile (and optional flush).
        bool enqueueStdin(const char* pData, int nLen, bool bFlush);
        void writeStdinDirect(const char* pData, int nLen, bool bFlush);
        bool startStdinWriter();
        void stopStdinWriter();
        bool isStdinWriterActive() const;

    protected:
        DWORD readFromPipe(HANDLE hPipe, char* pBuf, DWORD nBufSize);
        static DWORD WINAPI stdoutReaderThreadProc(LPVOID pParam);
        void stdoutReaderThreadFunc();

        void enqueueOutputChunk(const char* pData, int nLen, bool bStderr);
        void enqueueStdoutChunk(const char* pData, int nLen);
        void drainPipeToQueue(HANDLE hPipe, bool bStderr);
        void pumpAvailableStderr(char* pBuf);
        bool drainAvailableFromPipe(HANDLE hPipe, char* pBuf, bool bStderr, bool& bPipeAlive);

        static DWORD WINAPI stdinWriterThreadProc(LPVOID pParam);
        void stdinWriterThreadFunc();
        bool popStdinItem(CChildProcessStdinItem& item);
        void writeStdinItemToPipe(const CChildProcessStdinItem& item);

    private:
        HANDLE m_hStdInReadPipe;
        HANDLE m_hStdInWritePipe;
        HANDLE m_hStdOutReadPipe;
        HANDLE m_hStdOutWritePipe;
        HANDLE m_hStdErrReadPipe;
        HANDLE m_hStdErrWritePipe;
        HANDLE m_hPseudoCon;
        LPPROC_THREAD_ATTRIBUTE_LIST   m_pAttributeList;

        HANDLE m_hStdoutReaderThread;
        volatile LONG m_bStdoutReaderStop;

        CCriticalSection m_csOutputQueue;
        CListT< CChildProcessOutputChunk > m_OutputQueue;

        HANDLE m_hStdinWriterThread;
        volatile LONG m_bStdinWriterStop;
        HANDLE m_hStdinDataEvent;

        CCriticalSection m_csStdinQueue;
        CListT< CChildProcessStdinItem > m_StdinQueue;

        CChildProcess* m_pOwner;
};

//--------------------------------------------------------------------
#endif
