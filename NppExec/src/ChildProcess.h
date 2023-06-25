#ifndef _child_process_h_
#define _child_process_h_
//--------------------------------------------------------------------

#include "base.h"
#include "cpp/CStrT.h"

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

    PseudoConsoleHelper()
    {
        HMODULE hKernel32 = ::GetModuleHandle(_T("kernel32"));
        if (hKernel32)
        {
            // Available from Windows 10 October 2018 Update (version 1809)
            pfnCreatePseudoConsole = (typeCreatePseudoConsole) ::GetProcAddress(hKernel32, "CreatePseudoConsole");
            pfnResizePseudoConsole = (typeResizePseudoConsole) ::GetProcAddress(hKernel32, "ResizePseudoConsole");
            pfnClosePseudoConsole = (typeClosePseudoConsole) ::GetProcAddress(hKernel32, "ClosePseudoConsole");
        }
    }

    typeCreatePseudoConsole pfnCreatePseudoConsole = nullptr;
    typeResizePseudoConsole pfnResizePseudoConsole = nullptr;
    typeClosePseudoConsole  pfnClosePseudoConsole = nullptr;
};

class CChildProcess
{
    public:
        enum eAnsiEscSeq {
            escKeepRaw = 0,  // keep the esc-sequence characters without processing
            escRemove,       // remove the esc-sequence characters

            escTotalCount    // the last item here!
        };

    public:
        //CChildProcess();
        CChildProcess(CScriptEngine* pScriptEngine);
        ~CChildProcess();
        const TCHAR* GetInstanceStr() const;
        bool Create(HWND hParentWnd, LPCTSTR cszCommandLine);
        bool Kill(const CProcessKiller::eKillMethod arrKillMethods[], int nKillMethods,
                  unsigned int nWaitTimeout,
                  CProcessKiller::eKillMethod* pnSucceededKillMethod);
        void MustBreak(unsigned int nBreakMethod);

        bool WriteInput(const TCHAR* szLine, bool bFFlush = false);

        tstr& GetOutput(); // non-const allows to avoid copying at the very end
        int   GetExitCode() const;
        DWORD GetProcessId() const;
        const PROCESS_INFORMATION* GetProcessInfo() const;

        bool IsPseudoCon() const;
        const TCHAR* GetNewLine() const; // usually "\n" or "\r"
        unsigned int GetEncoding() const;
        unsigned int GetAnsiEscSeq() const;

        static void RemoveAnsiEscSequencesFromLine(tstr& Line);

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
        void  closePipes();
        void  closePseudoConsole();
        bool  applyOutputFilters(const tstr& _line, bool bOutput);
        bool  applyReplaceFilters(tstr& _line, tstr& printLine, bool bOutput);
        DWORD readPipesAndOutput(CStrT<char>& bufLine, 
                                 bool& bPrevLineEmpty,
                                 int&  nPrevState,
                                 bool  bOutputAll,
                                 bool& bDoOutputNext);

    private:
        CNppExec*           m_pNppExec;
        CScriptEngine*      m_pScriptEngine;
        tstr                m_strInstance;
        tstr                m_strOutput;
        int                 m_nExitCode;
        unsigned int        m_nBreakMethod;
        HANDLE              m_hStdInReadPipe;
        HANDLE              m_hStdInWritePipe; 
        HANDLE              m_hStdOutReadPipe;
        HANDLE              m_hStdOutWritePipe;
        PseudoConsoleHelper::typeHPCON m_hPseudoCon;
        LPPROC_THREAD_ATTRIBUTE_LIST   m_pAttributeList;
        PROCESS_INFORMATION m_ProcessInfo;
};

//--------------------------------------------------------------------
#endif
