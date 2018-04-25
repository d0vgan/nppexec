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

#ifndef _npp_exec_engine_h_
#define _npp_exec_engine_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "NppExec.h"
#include <memory>
#include <map>

LRESULT CALLBACK nppPluginWndProc(HWND, UINT, WPARAM, LPARAM);

char*  SciTextFromLPCTSTR(LPCTSTR pText, HWND hSci, int* pnLen = NULL); // finally call SciTextDeleteResultPtr
LPTSTR SciTextToLPTSTR(const char* pSciText, HWND hSci); // finally call SciTextDeleteResultPtr

#ifdef UNICODE
#define SciTextDeleteResultPtr(ptrResult, ptrOriginal) if (ptrResult) delete[] ptrResult
#else
#define SciTextDeleteResultPtr(ptrResult, ptrOriginal) if (ptrResult && ptrResult != ptrOriginal) delete[] ptrResult
#endif

class CScriptEngine : public IScriptEngine
{
    public:
        enum eCmdType {
            CMDTYPE_COLLATERAL_FORCED = -2,
            CMDTYPE_COMMENT_OR_EMPTY = -1,
            CMDTYPE_UNKNOWN = 0,

            CMDTYPE_NPPEXEC = 1,
            CMDTYPE_NPPOPEN,
            CMDTYPE_NPPRUN,
            CMDTYPE_NPPSAVE,
            CMDTYPE_NPPSWITCH,
            CMDTYPE_CLS,
            CMDTYPE_CD,
            CMDTYPE_DIR,
            CMDTYPE_NPPSAVEALL,
            CMDTYPE_CONLOADFROM,
            CMDTYPE_CONSAVETO,
            CMDTYPE_ECHO,
            CMDTYPE_NPEDEBUGLOG,
            CMDTYPE_SET,
            CMDTYPE_UNSET,
            CMDTYPE_NPENOEMPTYVARS,
            CMDTYPE_SELSAVETO,
            CMDTYPE_NPPCLOSE,
            CMDTYPE_INPUTBOX,
            CMDTYPE_NPPCONSOLE,
            CMDTYPE_ENVSET,
            CMDTYPE_ENVUNSET,
            CMDTYPE_NPECONSOLE,
            CMDTYPE_SELLOADFROM,
            CMDTYPE_SELSETTEXT,
            CMDTYPE_SELSETTEXTEX,
            CMDTYPE_NPECMDALIAS,
            CMDTYPE_NPPSENDMSG,
            CMDTYPE_SCISENDMSG,
            CMDTYPE_NPPSENDMSGEX,
            CMDTYPE_NPPMENUCOMMAND,
            CMDTYPE_CONCOLOUR,
            CMDTYPE_CONFILTER,
            CMDTYPE_IF,
            CMDTYPE_LABEL,
            CMDTYPE_GOTO,
            CMDTYPE_NPPSAVEAS,
            CMDTYPE_ELSE,
            CMDTYPE_ENDIF,
            CMDTYPE_PROCSIGNAL,
            CMDTYPE_SLEEP,
            CMDTYPE_NPEQUEUE,
            CMDTYPE_SCIFIND,
            CMDTYPE_SCIREPLACE,
            CMDTYPE_TEXTLOADFROM,
            CMDTYPE_TEXTSAVETO,
            CMDTYPE_NPPSETFOCUS,

            CMDTYPE_TOTAL_COUNT
        };

        enum eOnOffParam {
            PARAM_EMPTY   = -1,
            PARAM_OFF     = 0,
            PARAM_ON      = 1,
            PARAM_KEEP    = 2,
            PARAM_ENABLE  = 3,
            PARAM_DISABLE = 4,
            PARAM_UNKNOWN = 10
        };

        enum eCmdResult {
            CMDRESULT_INVALIDPARAM = -1, // error in command's parameter(s)
            CMDRESULT_FAILED = 0,        // error while executing the command
            CMDRESULT_SUCCEEDED = 1,     // OK, success
        };

        enum eIfState {
            IF_NONE = 0,   // not under IF
            IF_EXECUTING,  // IF-condition is true, executing lines under the IF
            IF_WANT_ELSE,  // IF-condition is false, want another ELSE (or ENDIF)
            IF_WANT_ENDIF, // done with IF or ELSE, want ENDIF
            IF_MAYBE_ELSE, // special case: ELSE *may* appear after GOTO
            IF_WANT_SILENT_ENDIF // special case: nested IF in a skipped condition block
        };

        enum eNppExecCmdPrefix {
            CmdPrefixNone = 0,
            CmdPrefixCollateralOrRegular,
            CmdPrefixCollateralForced
        };

        enum eGetCmdTypeFlags {
            ctfUseLogging   = 0x01,
            ctfIgnorePrefix = 0x02
        };

    public:
        typedef eCmdResult (CScriptEngine::*DOFUNC)(const tstr& params);

        eCmdResult Do(const tstr& params);
        eCmdResult DoCd(const tstr& params);
        eCmdResult DoCls(const tstr& params);
        eCmdResult DoConColour(const tstr& params);
        eCmdResult DoConFilter(const tstr& params);
        eCmdResult DoConLoadFrom(const tstr& params);
        eCmdResult DoConSaveTo(const tstr& params);
        eCmdResult DoDir(const tstr& params);
        eCmdResult DoEcho(const tstr& params);
        eCmdResult DoElse(const tstr& params);
        eCmdResult DoEndIf(const tstr& params);
        eCmdResult DoEnvSet(const tstr& params);
        eCmdResult DoEnvUnset(const tstr& params);
        eCmdResult DoGoTo(const tstr& params);
        eCmdResult DoIf(const tstr& params);
        eCmdResult DoInputBox(const tstr& params);
        eCmdResult DoLabel(const tstr& params);
        eCmdResult DoNpeCmdAlias(const tstr& params);
        eCmdResult DoNpeConsole(const tstr& params);
        eCmdResult DoNpeDebugLog(const tstr& params);
        eCmdResult DoNpeNoEmptyVars(const tstr& params);
        eCmdResult DoNpeQueue(const tstr& params);
        eCmdResult DoNppClose(const tstr& params);
        eCmdResult DoNppConsole(const tstr& params);
        eCmdResult DoNppExec(const tstr& params);
        eCmdResult DoNppMenuCommand(const tstr& params);
        eCmdResult DoNppOpen(const tstr& params);
        eCmdResult DoNppRun(const tstr& params);
        eCmdResult DoNppSave(const tstr& params);
        eCmdResult DoNppSaveAs(const tstr& params);
        eCmdResult DoNppSaveAll(const tstr& params);
        eCmdResult DoNppSendMsg(const tstr& params);
        eCmdResult DoNppSendMsgEx(const tstr& params);
        eCmdResult DoNppSetFocus(const tstr& params);
        eCmdResult DoNppSwitch(const tstr& params);
        eCmdResult DoProcSignal(const tstr& params);
        eCmdResult DoSleep(const tstr& params);
        eCmdResult DoSciFind(const tstr& params);
        eCmdResult DoSciReplace(const tstr& params);
        eCmdResult DoSciSendMsg(const tstr& params);
        eCmdResult DoSelLoadFrom(const tstr& params);
        eCmdResult DoSelSaveTo(const tstr& params);
        eCmdResult DoSelSetText(const tstr& params);
        eCmdResult DoSelSetTextEx(const tstr& params);
        eCmdResult DoSet(const tstr& params);
        eCmdResult DoTextLoadFrom(const tstr& params);
        eCmdResult DoTextSaveTo(const tstr& params);
        eCmdResult DoUnset(const tstr& params);

        void Run(unsigned int nRunFlags = 0);

        bool IsSharingLocalVars() const { return ((m_nRunFlags & rfShareLocalVars) != 0); }
        bool IsCollateral() const { return ((m_nRunFlags & rfCollateralScript) != 0); }
        bool IsExternal() const { return ((m_nRunFlags & rfExternal) != 0); }
        bool IsClosingConsole() const { return m_isClosingConsole; } // "npp_console off" is in progress

        void DoNotShareLocalVars()
        {
            m_nRunFlags &= ~rfShareLocalVars;
        }

        bool ContinueExecution() const;

        eCmdType GetLastCmdType() const  { return m_nCmdType; }
        //int  GetLastCmdResult() const  { return m_nLastCmdResult; }
        const tstr& GetLastLoggedCmd() const  { return m_sLoggedCmd; }
        const tstr& GetLastCmdParams() const  { return m_sCmdParams; }

        static eNppExecCmdPrefix checkNppExecCmdPrefix(CNppExec* pNppExec, tstr& Cmd, bool bRemovePrefix = true);
        static eCmdType getCmdType(CNppExec* pNppExec, tstr& Cmd, unsigned int nFlags = ctfUseLogging);
        static int      getOnOffParam(const tstr& param);
        static bool     isCommentOrEmpty(CNppExec* pNppExec, tstr& Cmd);
        static bool     isSkippingThisCommandDueToIfState(eCmdType cmdType, eIfState ifState);
        static eCmdType modifyCommandLine(CScriptEngine* pScriptEngine, tstr& Cmd, eIfState ifState);

        CScriptEngine(CNppExec* pNppExec, const CListT<tstr>& CmdList, const tstr& id);
        virtual ~CScriptEngine();

        CNppExec* GetNppExec() const { return m_pNppExec; }

        const TCHAR* GetInstanceStr() const;

        const tstr& GetId() const { return m_id; }

        DWORD GetThreadId() const { return m_dwThreadId; }

        CListT<tstr>& GetCmdList() { return m_CmdList; }
        const std::shared_ptr<CScriptEngine> GetParentScriptEngine() const { return m_pParentScriptEngine; }
        std::shared_ptr<CScriptEngine> GetParentScriptEngine() { return m_pParentScriptEngine; }
        const std::shared_ptr<CScriptEngine> GetChildScriptEngine() const { return m_pChildScriptEngine; }
        std::shared_ptr<CScriptEngine> GetChildScriptEngine() { return m_pChildScriptEngine; }
        bool IsParentOf(const std::shared_ptr<CScriptEngine> pScriptEngine) const;
        bool IsChildOf(const std::shared_ptr<CScriptEngine> pScriptEngine) const;

        void SetParentScriptEngine(std::shared_ptr<CScriptEngine> pParentEngine)
        {
            m_pParentScriptEngine = pParentEngine;
        }

        void SetChildScriptEngine(std::shared_ptr<CScriptEngine> pChildEngine)
        {
            m_pChildScriptEngine = pChildEngine;
        }

        virtual bool IsDone() const { return (m_eventRunIsDone.Wait(0) == WAIT_OBJECT_0); }
        virtual bool IsAborted() const { return (m_eventAbortTheScript.Wait(0) == WAIT_OBJECT_0); }
        virtual bool IsChildProcessRunning() const;
        virtual std::shared_ptr<CChildProcess> GetRunningChildProcess();

        virtual bool GetTriedExitCmd() const { return m_bTriedExitCmd; }
        virtual void SetTriedExitCmd(bool bTriedExitCmd) { m_bTriedExitCmd = bTriedExitCmd; }

        virtual void ScriptError(eErrorType type, const TCHAR* cszErrorMessage);
        void UndoAbort(const TCHAR* cszMessage);

        void ChildProcessMustBreakAll();
        bool WaitUntilDone(DWORD dwTimeoutMs) const;

    public:
        typedef std::map< tstr, CListItemT<tstr>* > tLabels;
        typedef CNppExecMacroVars::tMacroVars tMacroVars;

        typedef struct sCmdRange {
            CListItemT<tstr>* pBegin; // points to first cmd
            CListItemT<tstr>* pEnd;   // points _after_ last cmd
        } tCmdRange;

        class ScriptContext {
            public:
                tstr         ScriptName;
                tCmdRange    CmdRange;
                tLabels      Labels;
                tMacroVars   LocalMacroVars; // use with GetMacroVars().GetCsUserMacroVars()
                bool         IsNppExeced;
            
            protected:
                CBufT<eIfState> IfState;

                static void strAppendIfState(tstr& S, eIfState ifState)
                {
                    const TCHAR* cszState = _T("???");
                    switch ( ifState )
                    {
                        case IF_NONE:
                            cszState = _T("IF_NONE");
                            break;
                        case IF_EXECUTING:
                            cszState = _T("IF_EXECUTING");
                            break;
                        case IF_WANT_ELSE:
                            cszState = _T("IF_WANT_ELSE");
                            break;
                        case IF_WANT_ENDIF:
                            cszState = _T("IF_WANT_ENDIF");
                            break;
                        case IF_MAYBE_ELSE:
                            cszState = _T("IF_MAYBE_ELSE");
                            break;
                        case IF_WANT_SILENT_ENDIF:
                            cszState = _T("IF_WANT_SILENT_ENDIF");
                            break;
                    }

                    if ( !S.IsEmpty() )
                        S += _T(", ");

                    S += cszState;
                }
                
                void logIfState(const TCHAR* cszFuncName)
                {
                    tstr S;
                    const int nIfStates = IfState.size();
                    S.Reserve(20 * nIfStates);
                    for ( int i = 0; i < nIfStates; ++i )
                    {
                        strAppendIfState(S, IfState[i]);
                    }

                    Runtime::GetLogger().AddEx_WithoutOutput( _T("; %s() : IfState[%u] = { %s }"), cszFuncName, IfState.size(), S.c_str() );
                }

            public:
                ScriptContext()
                {
                    CmdRange.pBegin = 0;
                    CmdRange.pEnd = 0;
                    IsNppExeced = false;
                }

                const int GetIfDepth() const
                {
                    return IfState.size();
                }

                const eIfState GetIfState() const
                {
                    const int nSize = IfState.size();
                    return ((nSize != 0) ? IfState[nSize - 1] : IF_NONE);
                }

                void SetIfState(eIfState ifState)
                {
                    IfState[IfState.size() - 1] = ifState;

                    logIfState( _T("SetIfState") );
                }

                void PushIfState(eIfState ifState)
                {
                    if ( GetIfState() == IF_MAYBE_ELSE )
                    {
                        SetIfState(IF_NONE); // remove IF_MAYBE_ELSE because of nested IF
                    }
                    IfState.Append(ifState);

                    logIfState( _T("PushIfState") );
                }

                void PopIfState()
                {
                    IfState.SetSize(IfState.size() - 1);

                    logIfState( _T("PopIfState") );
                }
        };

        typedef struct tExecState {
            // these 4 variables are needed to prevent infinite loops
            int nExecCounter;
            int nExecMaxCount;
            int nGoToCounter;
            int nGoToMaxCount;
            // script name, cmd range, labels and so on
            CListT<ScriptContext> ScriptContextList;
            // pointer to current script line in m_CmdList
            CListItemT<tstr>* pScriptLineCurrent;
            // set this value to jump to a specific script line:
            CListItemT<tstr>* pScriptLineNext;
            CListItemT<tstr>* pScriptLineNextBackupCopy;
            // child console process
            std::shared_ptr<CChildProcess> pChildProcess;

            void SetScriptLineNext(CListItemT<tstr>* p)
            {
                pScriptLineNext = p;
                pScriptLineNextBackupCopy = p;
            }
            
            CListItemT<ScriptContext>* GetScriptContextItemPtr(const tstr& ScriptName)
            {
                CListItemT<CScriptEngine::ScriptContext>* p = ScriptContextList.GetFirst();
                while ( p && (p->GetItem().ScriptName != ScriptName) )
                {
                    p = p->GetNext();
                }
                return p;
            }

            ScriptContext& GetCurrentScriptContext()
            {
                return ScriptContextList.GetLast()->GetItem();
            }

            std::shared_ptr<CChildProcess> GetRunningChildProcess()
            {
                return pChildProcess;
            }

            bool IsChildProcessRunning() const
            {
                return (bool)(pChildProcess);
            }
        } ExecState;

        const ExecState& GetExecState() const { return m_execState; }
        ExecState& GetExecState() { return m_execState; }

    protected:
        enum eReportFlags {
            fMessageToConsole = 0x01,
            fReportEmptyParam = 0x02,
            fFailIfEmptyParam = 0x10
        };

        std::shared_ptr<CScriptEngine> m_pParentScriptEngine;
        std::shared_ptr<CScriptEngine> m_pChildScriptEngine;
        CNppExec*      m_pNppExec;
        CListT<tstr>   m_CmdList; // let the script have its own copy of CmdList
        tstr           m_strInstance;
        tstr           m_id;
        ExecState      m_execState;
        eCmdType       m_nCmdType;       // is not updated for empty command type
        //int          m_nLastCmdResult; // is not updated for empty command type
        tstr           m_sCmdParams;
        tstr           m_sLoggedCmd; // last cmd as it's present in the log
        DOFUNC         m_DoFunc[CMDTYPE_TOTAL_COUNT];
        unsigned int   m_nRunFlags;
        DWORD          m_dwThreadId;
        bool           m_bTriedExitCmd;
        bool           m_isClosingConsole;
        CEvent         m_eventRunIsDone;
        CEvent         m_eventAbortTheScript;

        void errorCmdNotEnoughParams(const TCHAR* cszCmd, const TCHAR* cszErrorMessage);
        void errorCmdNoParam(const TCHAR* cszCmd);
        void messageConsole(const TCHAR* cszCmd, const TCHAR* cszParams);
        bool reportCmdAndParams(const TCHAR* cszCmd, const tstr& params, unsigned int uFlags);
        void updateFocus();

        eCmdResult doSendMsg(const tstr& params, int cmdType);
        eCmdResult doSciFindReplace(const tstr& params, int cmdType);
        eCmdResult doIf(const tstr& params, bool isElseIf);
        eCmdResult doTextLoad(const tstr& params, int cmdType);
        eCmdResult doTextSave(const tstr& params, int cmdType);

};

class CScriptEngineRunner
{
    public:
        CScriptEngineRunner(CScriptEngine* pScriptEngine, unsigned int nRunFlags)
          : m_pScriptEngine(pScriptEngine)
          , m_nRunFlags(nRunFlags)
        {
        }

        CScriptEngine* GetScriptEngine() const { return m_pScriptEngine; }
        unsigned int   GetRunFlags() const { return m_nRunFlags; }

    protected:
        CScriptEngine* m_pScriptEngine;
        unsigned int m_nRunFlags;
};

extern const TCHAR MACRO_FILE_FULLPATH[];
extern const TCHAR MACRO_FILE_DIRPATH[];
extern const TCHAR MACRO_FILE_FULLNAME[];
extern const TCHAR MACRO_FILE_NAMEONLY[];
extern const TCHAR MACRO_FILE_EXTONLY[];
extern const TCHAR MACRO_NPP_DIRECTORY[];
extern const TCHAR MACRO_CURRENT_WORD[];
extern const TCHAR MACRO_CURRENT_LINE[];
extern const TCHAR MACRO_CURRENT_COLUMN[];
extern const TCHAR MACRO_DOCNUMBER[];
extern const TCHAR MACRO_SYSVAR[];
extern const TCHAR MACRO_LEFT_VIEW_FILE[];
extern const TCHAR MACRO_RIGHT_VIEW_FILE[];
extern const TCHAR MACRO_PLUGINS_CONFIG_DIR[];
extern const TCHAR MACRO_CURRENT_WORKING_DIR[];
extern const TCHAR MACRO_ARGC[];
extern const TCHAR MACRO_ARGV[];
extern const TCHAR MACRO_RARGV[];
extern const TCHAR MACRO_INPUT[];
extern const TCHAR MACRO_OUTPUT[];
extern const TCHAR MACRO_OUTPUT1[];
extern const TCHAR MACRO_OUTPUTL[];
extern const TCHAR MACRO_EXITCODE[];
extern const TCHAR MACRO_PID[];
extern const TCHAR MACRO_MSG_RESULT[];
extern const TCHAR MACRO_MSG_WPARAM[];
extern const TCHAR MACRO_MSG_LPARAM[];
extern const TCHAR MACRO_EXIT_CMD[];
extern const TCHAR MACRO_EXIT_CMD_SILENT[];
extern const TCHAR MACRO_LAST_CMD_RESULT[];
extern const TCHAR CMD_CLS[];
extern const TCHAR CMD_CD[];
extern const TCHAR CMD_DIR[];
extern const TCHAR CMD_ECHO[];
extern const TCHAR CMD_CONCOLOR[];
extern const TCHAR CMD_CONCOLOUR[];
extern const TCHAR CMD_CONFILTER[];
extern const TCHAR CMD_CONLOADFROM[];
extern const TCHAR CMD_CONLOAD[];
extern const TCHAR CMD_CONSAVETO[];
extern const TCHAR CMD_CONSAVE[];
extern const TCHAR CMD_NPPCLOSE[];
extern const TCHAR CMD_NPPCONSOLE[];
extern const TCHAR CMD_NPPEXEC[];
extern const TCHAR CMD_NPPSENDMSG[];
extern const TCHAR CMD_NPPSENDMSGEX[];
extern const TCHAR CMD_NPPMENUCOMMAND[];
extern const TCHAR CMD_NPPOPEN[];
extern const TCHAR CMD_NPPRUN[];
extern const TCHAR CMD_NPPSAVE[];
extern const TCHAR CMD_NPPSAVEAS[];
extern const TCHAR CMD_NPPSAVEALL[];
extern const TCHAR CMD_NPPSWITCH[];
extern const TCHAR CMD_NPPSETFOCUS[];
extern const TCHAR CMD_NPECMDALIAS[];
extern const TCHAR CMD_NPECONSOLE[];
extern const TCHAR CMD_NPEDEBUGLOG[];
extern const TCHAR CMD_NPEDEBUG[];
extern const TCHAR CMD_NPENOEMPTYVARS[];
extern const TCHAR CMD_NPEQUEUE[];
extern const TCHAR CMD_SCISENDMSG[];
extern const TCHAR CMD_SCIFIND[];
extern const TCHAR CMD_SCIREPLACE[];
extern const TCHAR CMD_SELLOADFROM[];
extern const TCHAR CMD_SELLOAD[];
extern const TCHAR CMD_SELSAVETO[];
extern const TCHAR CMD_SELSAVE[];
extern const TCHAR CMD_SELSETTEXT[];
extern const TCHAR CMD_SELSETTEXTEX[];
extern const TCHAR CMD_TEXTLOADFROM[];
extern const TCHAR CMD_TEXTLOAD[];
extern const TCHAR CMD_TEXTSAVETO[];
extern const TCHAR CMD_TEXTSAVE[];
extern const TCHAR CMD_SET[];
extern const TCHAR CMD_UNSET[];
extern const TCHAR CMD_ENVSET[];
extern const TCHAR CMD_ENVUNSET[];
extern const TCHAR CMD_SETENV[];
extern const TCHAR CMD_UNSETENV[];
extern const TCHAR CMD_INPUTBOX[];
extern const TCHAR CMD_IF[];
extern const TCHAR CMD_LABEL[];
extern const TCHAR CMD_GOTO[];
extern const TCHAR CMD_ELSE[];
extern const TCHAR CMD_ENDIF[];
extern const TCHAR CMD_PROCSIGNAL[];
extern const TCHAR CMD_SLEEP[];

//---------------------------------------------------------------------------
#endif
