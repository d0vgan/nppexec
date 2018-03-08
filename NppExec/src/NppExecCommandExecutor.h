/*
This file is part of NppExec
Copyright (C) 2016 DV <dvv81 (at) ukr (dot) net>

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

#ifndef _npp_exec_command_executor_h_
#define _npp_exec_command_executor_h_
//--------------------------------------------------------------------

#include "base.h"
#include "cpp/CStrT.h"
#include "cpp/CListT.h"
#include "NppExecHelpers.h"
#include "CAnyRichEdit.h"
#include "DlgConsole.h"
#include <memory>
#include <map>
#include <list>

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

class CChildProcess
{
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

    protected:
        static bool IsWindowsNT();

        void  reset();
        bool  isBreaking() const;
        void  closePipes();
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
        PROCESS_INFORMATION m_ProcessInfo;
};

class IScriptEngine
{
public:
    enum eErrorType {
        ET_REPORT,
        ET_ABORT,
        ET_UNPREDICTABLE
    };

    enum eRunFlags : unsigned int
    {
        rfShareLocalVars    = 0x0001, // can't be mixed with rfConsoleLocalVars
        rfConsoleLocalVars  = 0x0002, // can't be mixed with rfShareLocalVars
        rfShareConsoleState = 0x0004, // share the OutputEnabled state
        rfCollateralScript  = 0x0010, // is running as a collateral script
        rfStartScript       = 0x0020, // is running on start (NPPN_READY)
        rfExitScript        = 0x0040, // is running on exit (NPPN_SHUTDOWN)
        rfExternal          = 0x0100  // initiated by external plugin
    };

public:
    virtual ~IScriptEngine() = 0; // { }
    virtual bool IsDone() const = 0;
    virtual bool IsAborted() const = 0;
    virtual bool IsChildProcessRunning() const = 0;
    virtual std::shared_ptr<CChildProcess> GetRunningChildProcess() = 0;
    virtual bool GetTriedExitCmd() const = 0;
    virtual void SetTriedExitCmd(bool bTriedExitCmd) = 0;
    virtual void ScriptError(eErrorType type, const TCHAR* cszErrorMessage) = 0;
};

inline IScriptEngine::~IScriptEngine() { }

class CNppExecCommandExecutor
{
public:
    class RunningScriptEnginesStopper
    {
    public:
        RunningScriptEnginesStopper(CNppExecCommandExecutor* pExecutor, unsigned int nCloseConsoleFlags);

        void SendAbortNotificationToRunningScripts(const TCHAR* cszMessage, bool includingCurrentScript = true);
        bool WaitUntilRunningScriptsDone(DWORD dwTimeoutMs);

        void SendAbortNotificationToCurrentRunningScript(const TCHAR* cszMessage);
        bool WaitUntilCurrentRunningScriptDone(DWORD dwTimeoutMs);

    private:
        CNppExecCommandExecutor* m_pExecutor;
        unsigned int m_nCloseConsoleFlags;
        std::shared_ptr<CScriptEngine> m_CurrentRunningScriptEngine;
        std::list< std::shared_ptr<CScriptEngine> > m_RunningScriptEngines;
    };

    class Command
    {
    public:
        enum eCommandType
        {
            cmdUnknown = 0,
            cmdExpirable,
            cmdScriptable,
            cmdCloseConsole,
            cmdExecDlg,
            cmdDirectExec,
            cmdExecScript,
            cmdRunScript,
            cmdNppExit,
            cmdTryExitChildProc
        };

    public:
        Command(const tstr& id, eCommandType type = cmdUnknown);
        virtual ~Command();

        Command(const Command&) = delete;
        Command& operator=(const Command&) = delete;

        virtual void Execute() { }
        virtual bool IsExpired() const { return false; }
        virtual bool IsRunning() const { return false; }
        virtual bool IsDone() const { return false; }
        virtual LONG GetState() const { return 0; }
        
        eCommandType GetType() const { return m_type; }
        const TCHAR* GetTypeStr() const;
        const TCHAR* GetInstanceStr() const;
        CNppExecCommandExecutor* GetExecutor() const { return m_Executor; }
        const tstr& GetId() const { return m_id; }

    private:
        CNppExecCommandExecutor* m_Executor;
        tstr m_strInstance;
        const tstr m_id;
        const eCommandType m_type;
    };

    class ExpirableCommand : public Command
    {
    public:
        enum eExpirableType
        {
            Expirable,
            NonExpirable
        };

    protected:
        friend class CNppExecCommandExecutor;

        enum eCommandState
        {
            CmdIsReady = 0,
            CmdIsCheckingIfExpired,
            CmdIsExpired,
            CmdIsRunning,
            CmdIsDone
        };

    public:
        ExpirableCommand(const tstr& id, eCommandType type = cmdExpirable, eExpirableType expirableType = Expirable);
        virtual ~ExpirableCommand();
        virtual void Execute();
        virtual bool IsExpired() const { return (::InterlockedCompareExchange(&m_nState, CmdIsExpired, CmdIsExpired) == CmdIsExpired); }
        virtual bool IsRunning() const { return (::InterlockedCompareExchange(&m_nState, CmdIsRunning, CmdIsRunning) == CmdIsRunning); }
        virtual bool IsDone() const { return (::InterlockedCompareExchange(&m_nState, CmdIsDone, CmdIsDone) == CmdIsDone); }
        virtual LONG GetState() const { return m_nState; }

    protected:
        const eExpirableType m_expirableType;
        mutable volatile LONG m_nState;
        CEvent m_CheckIfExpiredEvent;

        virtual void subExecute() = 0;
        virtual void Expire();

    private:
        UINT_PTR m_idExpirationTimer;

        static CCriticalSection s_csExpirableCommands;
        static std::map<UINT_PTR, ExpirableCommand*> s_ExpirableCommands;

        void startExpirationTimer(unsigned int uMaxTimeout);
        void stopExpirationTimer();
        static void CALLBACK ExpirationTimerProc(HWND, UINT, UINT_PTR, DWORD);
    };

    class ScriptableCommand : public ExpirableCommand
    {
    public:
        ScriptableCommand(const tstr& id, eCommandType type = cmdScriptable, eExpirableType expirableType = Expirable);
        virtual ~ScriptableCommand();

    protected:
        static DWORD RunConsoleScript(Command* pCommand, const CListT<tstr>& CmdList, unsigned int nRunFlags);
        void OnDoExecDlg();
        void OnDirectExec(bool bCanSaveAll, unsigned int nRunFlags);
        void DoExecScript(LPCTSTR szScriptName, LPCTSTR szScriptArguments, bool bCanSaveAll, unsigned int nRunFlags);
        void DoRunScript(const CListT<tstr>& CmdList, unsigned int nRunFlags);
        void DoCloseConsole(unsigned int nCloseConsoleFlags);
        void DoNppExit();
        void TryExitChildProcess(bool bCloseConsoleOnExit = false, bool bNppIsClosing = false);
        
        bool CanStartScriptOrCommand();

        virtual void Expire();
        
    };

    class DoCloseConsoleCommand : public ScriptableCommand
    {
    public:
        DoCloseConsoleCommand(const tstr& id, unsigned int nCloseConsoleFlags, eExpirableType expirableType = Expirable);

    protected:
        virtual void subExecute();
        virtual void Expire();
        
        bool CanStartScriptOrCommand();

    protected:
        unsigned int m_nCloseConsoleFlags;
        RunningScriptEnginesStopper m_ScriptsStopper;
    };

    class OnDoExecDlgCommand : public ScriptableCommand
    {
    public:
        OnDoExecDlgCommand(const tstr& id, eExpirableType expirableType = Expirable);
    
    protected:
        virtual void subExecute();
    };

    class OnDirectExecCommand : public ScriptableCommand
    {
    public:
        OnDirectExecCommand(const tstr& id, bool bCanSaveAll, unsigned int nRunFlags = 0, eExpirableType expirableType = Expirable);

    protected:
        virtual void subExecute();

    protected:
        bool m_bCanSaveAll;
        unsigned int m_nRunFlags;
    };

    class DoExecScriptCommand : public ScriptableCommand
    {
    public:
        DoExecScriptCommand(const tstr& id, LPCTSTR szScriptName, LPCTSTR szScriptArguments, bool bCanSaveAll, unsigned int nRunFlags = 0, eExpirableType expirableType = Expirable);

    protected:
        virtual void subExecute();

    protected:
        tstr m_sScriptName;
        tstr m_sScriptArguments;
        bool m_bCanSaveAll;
        unsigned int m_nRunFlags;
    };

    class DoRunScriptCommand : public ScriptableCommand
    {
    public:
        DoRunScriptCommand(const tstr& id, const CListT<tstr>& CmdList, unsigned int nRunFlags = 0, eExpirableType expirableType = Expirable);

    protected:
        virtual void subExecute();

    protected:
        unsigned int m_nRunFlags;
        CListT<tstr> m_CmdList;
    };

    class NppExitCommand : public ScriptableCommand
    {
    public:
        NppExitCommand(const tstr& id, eExpirableType expirableType = Expirable);

    protected:
        virtual void subExecute();
        virtual void Expire();

    protected:
        RunningScriptEnginesStopper m_ScriptsStopper;
    };

    class TryExitChildProcessCommand : public ScriptableCommand
    {
    public:
        enum eFlags {
            fCloseConsoleOnExit = 0x01,
            fNppIsClosing       = 0x02,
            fCmdNppConsole      = ConsoleDlg::ccfCmdNppConsole
        };

    public:
        TryExitChildProcessCommand(const tstr& id, unsigned int nFlags = 0, eExpirableType expirableType = Expirable);

    protected:
        virtual void subExecute();
        virtual void Expire();

    protected:
        unsigned int m_nFlags;
        RunningScriptEnginesStopper m_ScriptsStopper;
    };

public:
    enum eScriptEnginesFlags : unsigned int
    {
        sefExcludingCurrent     = 0x01,
        sefExcludingCollateral  = 0x02, // can't be mixed with sefOnlyCollateral
        sefOnlyCollateral       = 0x10, // can't be mixed with sefExcludingCollateral
        sefOnlyRunningChildProc = 0x20
    };

public:
    CNppExecCommandExecutor();
    ~CNppExecCommandExecutor();

    void Start();
    void Stop();
    void ExecuteCommand(Command* cmd); // adds a command to the queue
    bool ExecuteCollateralScript(const CListT<tstr>& scriptCmdList, const tstr& id, unsigned int nRunFlags = IScriptEngine::rfCollateralScript); // executes the script collaterally to the currently executed command (if any)
    void CommandExpired(ExpirableCommand* cmd);
    void ClearCommandQueue();

    CNppExec* GetNppExec() const { return m_pNppExec; }
    void SetNppExec(CNppExec* pNppExec) { m_pNppExec = pNppExec; }

    LONG GetRunningScriptsCount() const { return m_nRunningScripts; } // currently it does not count collateral scripts
    //LONG GetRunningChildProcessesCount() const { return m_nRunningChildProcesses; }

    bool IsScriptRunningOrQueued() const;
    bool IsChildProcessRunning() const;
    bool IsCommandQueueEmpty() const;

    std::shared_ptr<CScriptEngine> GetRunningScriptEngine();
    std::list< std::shared_ptr<CScriptEngine> > GetRunningScriptEngines(unsigned int nScriptEngineFlags = 0);
    void RunScriptEngine(std::shared_ptr<CScriptEngine> pScriptEngine, unsigned int nRunFlags);

    std::shared_ptr<CChildProcess> GetRunningChildProcess();

    bool GetTriedExitCmd() const;
    void SetTriedExitCmd(bool bTriedExitCmd);

    void ExecuteChildProcessCommand(tstr& cmd);
    bool WriteChildProcessInput(const TCHAR* szLine, bool bFFlush = false);

    void ChildProcessMustBreak(unsigned int nBreakMethod); // about to break the current child process
    void ChildProcessMustBreakAll(unsigned int nBreakMethod); // about to break all child processes

    bool WaitUntilAllScriptEnginesDone(DWORD dwTimeoutMs);

private:
    static DWORD WINAPI BackgroundExecuteThreadFunc(LPVOID lpParam);
    static DWORD WINAPI BackgroundExpiredThreadFunc(LPVOID lpParam);
    // critical sections are created first and destroyed last...
    mutable CCriticalSection m_csExecuteQueue;
    mutable CCriticalSection m_csExpiredQueue;
    // data...
    CNppExec* m_pNppExec;
    HANDLE m_hBackgroundExecuteThread;
    HANDLE m_hBackgroundExpiredThread;
    CEvent m_StopEvent;
    CEvent m_ExecuteCmdEvent;
    CEvent m_ExpiredCmdEvent;
    CEvent m_ExecuteThreadDoneEvent;
    CEvent m_ExpiredThreadDoneEvent;
    std::list< std::shared_ptr<Command> > m_ExecuteQueue;
    std::list< std::shared_ptr<ExpirableCommand> > m_ExpiredQueue;

protected:
    friend class ScriptableCommand;

    mutable CCriticalSection m_csRunningScriptEngine;
    std::shared_ptr<IScriptEngine> m_RunningScriptEngine;
    volatile LONG m_nRunningScripts;
    //volatile LONG m_nRunningChildProcesses;
};

//--------------------------------------------------------------------
#endif
