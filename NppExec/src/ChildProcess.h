#ifndef _child_process_h_
#define _child_process_h_
//--------------------------------------------------------------------

#include "base.h"
#include "cpp/CStrT.h"
#include "ConsoleVtParser.h"

class CNppExec;
class CScriptEngine;

class CProcessKiller
{
    public:
        enum eKillMethod {
            killNone = 0,      // do nothing
            killCtrlBreak = 1,
            killCtrlC,
            killWmClose
        };

        CProcessKiller(const PROCESS_INFORMATION* pProcInfo);
        ~CProcessKiller();

        bool Kill(eKillMethod nKillMethod, unsigned int nWaitTimeout = 0);

    protected:
        bool KillByCtrlBreak(unsigned int nWaitTimeout);
        bool KillByCtrlC(unsigned int nWaitTimeout);
        bool KillByWmClose(unsigned int nWaitTimeout);

        static BOOL CALLBACK KillAppEnumFunc(HWND hWnd, LPARAM lParam);
        bool KillByConsoleCtrlEvent(unsigned int nCtrlEvent, unsigned int nWaitTimeout);
        bool IsProcessActive() const;

    protected:
        PROCESS_INFORMATION m_ProcInfo;
};

struct PseudoConsoleHelper
{
    static const DWORD constPROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE = 0x00020016;
    static const DWORD constEXTENDED_STARTUPINFO_PRESENT = 0x00080000;

    typedef HANDLE typeHPCON;

    typedef HRESULT (WINAPI *typeCreatePseudoConsole)(
        _In_ COORD size,
        _In_ HANDLE hInput,
        _In_ HANDLE hOutput,
        _In_ DWORD dwFlags,
        _Out_ typeHPCON* phPC
    );

    typedef HRESULT (WINAPI *typeResizePseudoConsole)(
        _In_ typeHPCON hPC,
        _In_ COORD size
    );

    typedef void (WINAPI *typeClosePseudoConsole)(
        _In_ typeHPCON hPC
    );

    typedef BOOL (WINAPI *typeInitializeProcThreadAttributeList)(
        LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
        DWORD dwAttributeCount,
        DWORD dwFlags,
        PSIZE_T lpSize
    );

    typedef BOOL (WINAPI *typeUpdateProcThreadAttribute)(
        LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
        DWORD dwFlags,
        DWORD_PTR Attribute,
        PVOID lpValue,
        SIZE_T cbSize,
        PVOID lpPreviousValue,
        PSIZE_T lpReturnSize
    );

    typedef VOID (WINAPI *typeDeleteProcThreadAttributeList)(
        LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList
    );

    PseudoConsoleHelper()
    {
        HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32"));
        if (hKernel32)
        {
            // Available from Windows 10 October 2018 Update (version 1809)
            pfnCreatePseudoConsole = (typeCreatePseudoConsole) ::GetProcAddress(hKernel32, "CreatePseudoConsole");
            pfnResizePseudoConsole = (typeResizePseudoConsole) ::GetProcAddress(hKernel32, "ResizePseudoConsole");
            pfnClosePseudoConsole = (typeClosePseudoConsole) ::GetProcAddress(hKernel32, "ClosePseudoConsole");

            // Available from Windows Vista
            pfnInitializeProcThreadAttributeList = (typeInitializeProcThreadAttributeList) ::GetProcAddress(hKernel32, "InitializeProcThreadAttributeList");
            pfnUpdateProcThreadAttribute = (typeUpdateProcThreadAttribute) ::GetProcAddress(hKernel32, "UpdateProcThreadAttribute");
            pfnDeleteProcThreadAttributeList = (typeDeleteProcThreadAttributeList) ::GetProcAddress(hKernel32, "DeleteProcThreadAttributeList");
        }
    }

    typeCreatePseudoConsole pfnCreatePseudoConsole = nullptr;
    typeResizePseudoConsole pfnResizePseudoConsole = nullptr;
    typeClosePseudoConsole  pfnClosePseudoConsole = nullptr;
    typeInitializeProcThreadAttributeList pfnInitializeProcThreadAttributeList = nullptr;
    typeUpdateProcThreadAttribute pfnUpdateProcThreadAttribute = nullptr;
    typeDeleteProcThreadAttributeList pfnDeleteProcThreadAttributeList = nullptr;
};

#include "ChildProcessIo.h"

class CChildProcess
{
    friend class CChildProcessIo;

    public:
        enum eAnsiEscSeq {
            escKeepRaw = 0,  // keep the esc-sequence characters without processing
            escRemove,       // remove the esc-sequence characters
            escProcess,      // process via VT parser

            escTotalCount    // the last item here!
        };

    public:
        //CChildProcess();
        CChildProcess(CScriptEngine* pScriptEngine);
        ~CChildProcess();
        const TCHAR* GetInstanceStr() const;
        // Legacy blocking entrypoint (Start + WaitForExit). Prefer Start/WaitForExit at call sites.
        bool Create(HWND hParentWnd, LPCTSTR cszCommandLine);
        // Phase 2: lifecycle API. Start creates the child process and returns.
        // WaitForExit drives the read/kill/cleanup loop until the child exits
        // (or until nTimeoutMs expires, in which case it returns false while keeping the
        // child process running).
        bool Start(HWND hParentWnd, LPCTSTR cszCommandLine);
        bool WaitForExit(unsigned int nTimeoutMs = INFINITE);
        bool IsRunning() const;
        void Stop();
        bool Kill(const CProcessKiller::eKillMethod arrKillMethods[], int nKillMethods,
                  unsigned int nWaitTimeout,
                  CProcessKiller::eKillMethod* pnSucceededKillMethod);
        void MustBreak(unsigned int nBreakMethod);

        // Stdin: encode on caller thread; WriteFile on stdin writer thread (P1.6).
        // bFFlush: if true, FlushFileBuffers after WriteFile — can block until the child reads
        // stdin; default false. No in-tree caller passes true (P1.7 audit).
        bool WriteInput(const TCHAR* szLine, bool bFFlush = false);

        tstr& GetOutput(); // non-const allows to avoid copying at the very end
        int   GetExitCode() const;
        DWORD GetProcessId() const;
        const PROCESS_INFORMATION* GetProcessInfo() const;

        bool IsPseudoCon() const;
        bool ResizePseudoConsoleToConsole();
        const TCHAR* GetNewLine() const; // usually "\n" or "\r"
        unsigned int GetEncoding() const;
        unsigned int GetAnsiEscSeq() const;

        static bool RemoveAnsiEscSequencesFromLine(tstr& Line);

    protected:
        static bool IsWindowsNT();

        enum eCommandLinePolicy {
            clpNone = 0, // do nothing
            clpPathExt,  // try %PATH% and extensions from %PATHEXT%
            clpComSpec   // start with "%COMSPEC% /C" or "cmd /C"
        };
        static void applyCommandLinePolicy(tstr& sCmdLine, eCommandLinePolicy mode);

        void  reset();
        bool  isBreaking() const;
        bool  applyOutputFilters(const tstr& _line, bool bOutput);
        bool  applyReplaceFilters(tstr& _line, tstr& printLine, bool bOutput);
        DWORD readPipesAndOutput(CStrT<char>& bufLine, 
                                 bool& bPrevLineEmpty,
                                 int&  nPrevState,
                                 bool  bOutputAll,
                                 bool& bDoOutputNext,
                                 CStrT<char>& bufLineStderr,
                                 bool& bPrevLineEmptyStderr,
                                 int&  nPrevStateStderr,
                                 bool& bDoOutputNextStderr);
        DWORD processOutputBuffer(CStrT<char>& bufLine,
                                  bool& bPrevLineEmpty,
                                  int&  nPrevState,
                                  bool  bFlushPartialLine,
                                  bool& bDoOutputNext,
                                  bool  bSomethingHasBeenReadFromPipe,
                                  bool  bFromStderr = false);
        DWORD readOnePipeDirect(HANDLE hReadPipe,
                                bool  bOutputAll,
                                bool  bFromStderr);
        DWORD processOutputQueue(CStrT<char>& bufLine,
                                 bool& bPrevLineEmpty,
                                 int&  nPrevState,
                                 bool& bDoOutputNext,
                                 CStrT<char>& bufLineStderr,
                                 bool& bPrevLineEmptyStderr,
                                 int&  nPrevStateStderr,
                                 bool& bDoOutputNextStderr,
                                 bool  bOutputAll,
                                 bool  bReadStdout,
                                 bool  bReadStderr);
        DWORD readPipesDirect(CStrT<char>& bufLine,
                              bool& bPrevLineEmpty,
                              int&  nPrevState,
                              bool  bOutputAll,
                              bool& bDoOutputNext,
                              CStrT<char>& bufLineStderr,
                              bool& bPrevLineEmptyStderr,
                              int&  nPrevStateStderr,
                              bool& bDoOutputNextStderr);

    private:
        CNppExec*           m_pNppExec;
        CScriptEngine*      m_pScriptEngine;
        tstr                m_strInstance;
        tstr                m_strOutput;
        tstr                m_sCmdLine;     // stored original command line for kill heuristics
        HANDLE              m_hJob;         // job object handle (kept alive while the child runs)
        int                 m_nExitCode;
        unsigned int        m_nBreakMethod;
        CChildProcessIo     m_Io;
        PROCESS_INFORMATION m_ProcessInfo;

        // Phase 2 (P2.1): keep read-loop state across multiple WaitForExit(timeout) calls.
        CStrT<char>  m_bufLine;
        bool         m_bPrevLineEmpty;
        int          m_nPrevState;
        bool         m_bDoOutputNext;

        CStrT<char>  m_bufLineStderr;
        bool         m_bPrevLineEmptyStderr;
        int          m_nPrevStateStderr;
        bool         m_bDoOutputNextStderr;

        CConsoleVtParser m_VtParserStdout;
        CConsoleVtParser m_VtParserStderr;
};

//--------------------------------------------------------------------
#endif
