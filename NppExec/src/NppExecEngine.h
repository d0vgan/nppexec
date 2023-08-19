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
#include <list>
#include <atomic>

#ifdef _DEBUG
  #include <cassert>
#endif

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
            CMDTYPE_CLIPSETTEXT,
            CMDTYPE_MESSAGEBOX,
            CMDTYPE_EXIT,
            CMDTYPE_NPESENDMSGBUFLEN,
            CMDTYPE_CALCIF,
            CMDTYPE_HELP,
            CMDTYPE_PROCINPUT,
            CMDTYPE_NPPEXECTEXT,

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
            IF_NONE = 0,       // not under IF
            IF_EXECUTING,      // IF-condition is true, executing lines under the IF
            IF_EXECUTING_ELSE, // executing lines under plain ELSE (without IF)
            IF_WANT_ELSE,      // IF-condition is false, want another ELSE (or ENDIF)
            IF_WANT_ENDIF,     // done with IF or ELSE, want ENDIF
            IF_MAYBE_ELSE,     // special case: ELSE *may* appear after GOTO
            IF_WANT_SILENT_ENDIF // special case: nested IF in a skipped condition block
        };

        enum eNppExecCmdPrefix {
            CmdPrefixNone = 0,
            CmdPrefixCollateralOrRegular,
            CmdPrefixCollateralForced
        };

        enum eIsNppExecPrefixedFlags {
            npfRemovePrefix        = 0x01,
            npfSubstituteMacroVars = 0x02
        };

        enum eGetCmdTypeFlags {
            ctfUseLogging   = 0x01,
            ctfIgnorePrefix = 0x02,
            ctfReportError  = 0x04
        };

    public:
        typedef eCmdResult (*EXECFUNC)(CScriptEngine* pEngine, const tstr& params);

        // For each of the Do-methods:
        // 1. Specify the corresponding *Command struct
        // 2. Register this *Command struct in the CScriptCommandRegistry::initialize()
        // 3. In case of AltName, see what was done for the existing *Command structs
        // 4. Add help info to the CONSOLE_CMD_INFO array (DlgConsole.cpp)
        eCmdResult Do(const tstr& params);
        eCmdResult DoCd(const tstr& params);
        eCmdResult DoCls(const tstr& params);
        eCmdResult DoClipSetText(const tstr& params);
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
        eCmdResult DoExit(const tstr& params);
        eCmdResult DoGoTo(const tstr& params);
        eCmdResult DoHelp(const tstr& params);
        eCmdResult DoIf(const tstr& params);
        eCmdResult DoCalcIf(const tstr& params);
        eCmdResult DoInputBox(const tstr& params);
        eCmdResult DoLabel(const tstr& params);
        eCmdResult DoMessageBox(const tstr& params);
        eCmdResult DoNpeCmdAlias(const tstr& params);
        eCmdResult DoNpeConsole(const tstr& params);
        eCmdResult DoNpeDebugLog(const tstr& params);
        eCmdResult DoNpeNoEmptyVars(const tstr& params);
        eCmdResult DoNpeQueue(const tstr& params);
        eCmdResult DoNpeSendMsgBufLen(const tstr& params);
        eCmdResult DoNppClose(const tstr& params);
        eCmdResult DoNppConsole(const tstr& params);
        eCmdResult DoNppExec(const tstr& params);
        eCmdResult DoNppExecText(const tstr& params);
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
        eCmdResult DoProcInput(const tstr& params);
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

        struct DoCommand
        {
            static const TCHAR* const Name() { return _T(""); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_UNKNOWN; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->Do(params); }
        };

        struct DoCdCommand
        {
            static const TCHAR* const Name() { return _T("CD"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_CD; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoCd(params); }
        };

        struct DoClsCommand
        {
            static const TCHAR* const Name() { return _T("CLS"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_CLS; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoCls(params); }
        };

        struct DoClipSetTextCommand
        {
            static const TCHAR* const Name() { return _T("CLIP_SETTEXT"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_CLIPSETTEXT; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoClipSetText(params); }
        };

        struct DoConColourCommand
        {
            static const TCHAR* const Name() { return _T("CON_COLOUR"); }
            static const TCHAR* const AltName() { return _T("CON_COLOR"); }
            static eCmdType           Type() { return CMDTYPE_CONCOLOUR; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoConColour(params); }
        };

        struct DoConFilterCommand
        {
            static const TCHAR* const Name() { return _T("CON_FILTER"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_CONFILTER; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoConFilter(params); }
        };

        struct DoConLoadFromCommand
        {
            static const TCHAR* const Name() { return _T("CON_LOADFROM"); }
            static const TCHAR* const AltName() { return _T("CON_LOAD"); }
            static eCmdType           Type() { return CMDTYPE_CONLOADFROM; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoConLoadFrom(params); }
        };

        struct DoConSaveToCommand
        {
            static const TCHAR* const Name() { return _T("CON_SAVETO"); }
            static const TCHAR* const AltName() { return _T("CON_SAVE"); }
            static eCmdType           Type() { return CMDTYPE_CONSAVETO; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoConSaveTo(params); }
        };

        struct DoDirCommand
        {
            static const TCHAR* const Name() { return _T("DIR"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_DIR; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoDir(params); }
        };

        struct DoEchoCommand
        {
            static const TCHAR* const Name() { return _T("ECHO"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_ECHO; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoEcho(params); }
        };

        struct DoElseCommand
        {
            static const TCHAR* const Name() { return _T("ELSE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_ELSE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoElse(params); }
        };

        struct DoEndIfCommand
        {
            static const TCHAR* const Name() { return _T("ENDIF"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_ENDIF; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoEndIf(params); }
        };

        struct DoEnvSetCommand
        {
            static const TCHAR* const Name() { return _T("ENV_SET"); }
            static const TCHAR* const AltName() { return _T("SET_ENV"); }
            static eCmdType           Type() { return CMDTYPE_ENVSET; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoEnvSet(params); }
        };

        struct DoEnvUnsetCommand
        {
            static const TCHAR* const Name() { return _T("ENV_UNSET"); }
            static const TCHAR* const AltName() { return _T("UNSET_ENV"); }
            static eCmdType           Type() { return CMDTYPE_ENVUNSET; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoEnvUnset(params); }
        };

        struct DoExitCommand
        {
            static const TCHAR* const Name() { return _T("EXIT"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_EXIT; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoExit(params); }
        };

        struct DoGoToCommand
        {
            static const TCHAR* const Name() { return _T("GOTO"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_GOTO; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoGoTo(params); }
        };

        struct DoHelpCommand
        {
            static const TCHAR* const Name() { return _T("HELP"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_HELP; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoHelp(params); }
        };

        struct DoIfCommand
        {
            static const TCHAR* const Name() { return _T("IF"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_IF; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoIf(params); }
        };

        struct DoCalcIfCommand
        {
            static const TCHAR* const Name() { return _T("IF~"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_CALCIF; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoCalcIf(params); }
        };

        struct DoInputBoxCommand
        {
            static const TCHAR* const Name() { return _T("INPUTBOX"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_INPUTBOX; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoInputBox(params); }
        };

        struct DoLabelCommand
        {
            static const TCHAR* const Name() { return _T("LABEL"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_LABEL; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoLabel(params); }
        };

        struct DoMessageBoxCommand
        {
            static const TCHAR* const Name() { return _T("MESSAGEBOX"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_MESSAGEBOX; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoMessageBox(params); }
        };

        struct DoNpeCmdAliasCommand
        {
            static const TCHAR* const Name() { return _T("NPE_CMDALIAS"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPECMDALIAS; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNpeCmdAlias(params); }
        };

        struct DoNpeConsoleCommand
        {
            static const TCHAR* const Name() { return _T("NPE_CONSOLE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPECONSOLE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNpeConsole(params); }
        };

        struct DoNpeDebugLogCommand
        {
            static const TCHAR* const Name() { return _T("NPE_DEBUGLOG"); }
            static const TCHAR* const AltName() { return _T("NPE_DEBUG"); }
            static eCmdType           Type() { return CMDTYPE_NPEDEBUGLOG; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNpeDebugLog(params); }
        };

        struct DoNpeNoEmptyVarsCommand
        {
            static const TCHAR* const Name() { return _T("NPE_NOEMPTYVARS"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPENOEMPTYVARS; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNpeNoEmptyVars(params); }
        };

        struct DoNpeQueueCommand
        {
            static const TCHAR* const Name() { return _T("NPE_QUEUE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPEQUEUE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNpeQueue(params); }
        };

        struct DoNpeSendMsgBufLenCommand
        {
            static const TCHAR* const Name() { return _T("NPE_SENDMSGBUFLEN"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPESENDMSGBUFLEN; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNpeSendMsgBufLen(params); }
        };

        struct DoNppCloseCommand
        {
            static const TCHAR* const Name() { return _T("NPP_CLOSE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPCLOSE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppClose(params); }
        };

        struct DoNppConsoleCommand
        {
            static const TCHAR* const Name() { return _T("NPP_CONSOLE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPCONSOLE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppConsole(params); }
        };

        struct DoNppExecCommand
        {
            static const TCHAR* const Name() { return _T("NPP_EXEC"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPEXEC; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppExec(params); }
        };

        struct DoNppExecTextCommand
        {
            static const TCHAR* const Name() { return _T("NPP_EXECTEXT"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPEXECTEXT; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppExecText(params); }
        };

        struct DoNppMenuCommandCommand
        {
            static const TCHAR* const Name() { return _T("NPP_MENUCOMMAND"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPMENUCOMMAND; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppMenuCommand(params); }
        };

        struct DoNppOpenCommand
        {
            static const TCHAR* const Name() { return _T("NPP_OPEN"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPOPEN; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppOpen(params); }
        };

        struct DoNppRunCommand
        {
            static const TCHAR* const Name() { return _T("NPP_RUN"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPRUN; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppRun(params); }
        };

        struct DoNppSaveCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SAVE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSAVE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSave(params); }
        };

        struct DoNppSaveAsCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SAVEAS"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSAVEAS; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSaveAs(params); }
        };

        struct DoNppSaveAllCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SAVEALL"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSAVEALL; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSaveAll(params); }
        };

        struct DoNppSendMsgCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SENDMSG"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSENDMSG; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSendMsg(params); }
        };

        struct DoNppSendMsgExCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SENDMSGEX"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSENDMSGEX; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSendMsgEx(params); }
        };

        struct DoNppSetFocusCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SETFOCUS"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSETFOCUS; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSetFocus(params); }
        };

        struct DoNppSwitchCommand
        {
            static const TCHAR* const Name() { return _T("NPP_SWITCH"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_NPPSWITCH; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoNppSwitch(params); }
        };

        struct DoProcInputCommand
        {
            static const TCHAR* const Name() { return _T("PROC_INPUT"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_PROCINPUT; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoProcInput(params); }
        };

        struct DoProcSignalCommand
        {
            static const TCHAR* const Name() { return _T("PROC_SIGNAL"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_PROCSIGNAL; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoProcSignal(params); }
        };

        struct DoSleepCommand
        {
            static const TCHAR* const Name() { return _T("SLEEP"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SLEEP; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSleep(params); }
        };

        struct DoSciFindCommand
        {
            static const TCHAR* const Name() { return _T("SCI_FIND"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SCIFIND; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSciFind(params); }
        };

        struct DoSciReplaceCommand
        {
            static const TCHAR* const Name() { return _T("SCI_REPLACE"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SCIREPLACE; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSciReplace(params); }
        };

        struct DoSciSendMsgCommand
        {
            static const TCHAR* const Name() { return _T("SCI_SENDMSG"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SCISENDMSG; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSciSendMsg(params); }
        };

        struct DoSelLoadFromCommand
        {
            static const TCHAR* const Name() { return _T("SEL_LOADFROM"); }
            static const TCHAR* const AltName() { return _T("SEL_LOAD"); }
            static eCmdType           Type() { return CMDTYPE_SELLOADFROM; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSelLoadFrom(params); }
        };

        struct DoSelSaveToCommand
        {
            static const TCHAR* const Name() { return _T("SEL_SAVETO"); }
            static const TCHAR* const AltName() { return _T("SEL_SAVE"); }
            static eCmdType           Type() { return CMDTYPE_SELSAVETO; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSelSaveTo(params); }
        };

        struct DoSelSetTextCommand
        {
            static const TCHAR* const Name() { return _T("SEL_SETTEXT"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SELSETTEXT; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSelSetText(params); }
        };

        struct DoSelSetTextExCommand
        {
            static const TCHAR* const Name() { return _T("SEL_SETTEXT+"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SELSETTEXTEX; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSelSetTextEx(params); }
        };

        struct DoSetCommand
        {
            static const TCHAR* const Name() { return _T("SET"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_SET; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoSet(params); }
        };

        struct DoTextLoadFromCommand
        {
            static const TCHAR* const Name() { return _T("TEXT_LOADFROM"); }
            static const TCHAR* const AltName() { return _T("TEXT_LOAD"); }
            static eCmdType           Type() { return CMDTYPE_TEXTLOADFROM; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoTextLoadFrom(params); }
        };

        struct DoTextSaveToCommand
        {
            static const TCHAR* const Name() { return _T("TEXT_SAVETO"); }
            static const TCHAR* const AltName() { return _T("TEXT_SAVE"); }
            static eCmdType           Type() { return CMDTYPE_TEXTSAVETO; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoTextSaveTo(params); }
        };

        struct DoUnsetCommand
        {
            static const TCHAR* const Name() { return _T("UNSET"); }
            static const TCHAR* const AltName() { return nullptr; }
            static eCmdType           Type() { return CMDTYPE_UNSET; }
            static eCmdResult         Exec(CScriptEngine* pEngine, const tstr& params) { return pEngine->DoUnset(params); }
        };

        class CScriptCommandRegistry
        {
            public:
                CScriptCommandRegistry()
                {
                    CCriticalSectionLockGuard lock(m_csConstruct); // just in case

                    if ( m_CommandTypeByName.empty() )
                    {
                        // initialize() was not called

                      #ifdef _DEBUG
                        for ( auto& p : m_CommandExecFunc )
                        {
                            p = nullptr;
                        }
                        for ( auto& n : m_CommandNameByType )
                        {
                            n = nullptr;
                        }
                      #endif

                        initialize();

                      #ifdef _DEBUG
                        // there must be no nullptr items after initialize() !!!
                        for ( const auto& p : m_CommandExecFunc )
                        {
                            assert( p != nullptr );
                        }
                        for ( const auto& n : m_CommandNameByType )
                        {
                            assert( n != nullptr );
                        }
                      #endif
                    }
                }

                const TCHAR* const GetCmdNameByType(eCmdType cmdType) const
                {
                    return m_CommandNameByType[cmdType];
                }

                eCmdType GetCmdTypeByName(const tstr& cmdName) const
                {
                    auto itr = m_CommandTypeByName.find(cmdName);
                    return ( (itr != m_CommandTypeByName.end()) ? itr->second : CMDTYPE_UNKNOWN );
                }

                const std::list<tstr>& GetSortedCmdNames() const
                {
                    return m_SortedCommandNames;
                }

                EXECFUNC GetCmdExecFunc(eCmdType cmdType) const
                {
                    return m_CommandExecFunc[cmdType];
                }

            protected:
                template<class C> void registerCommand()
                {
                    const TCHAR* const cmdName = C::Name();
                    const TCHAR* const cmdAltName = C::AltName();
                    const eCmdType cmdType = C::Type();
                    m_CommandExecFunc[cmdType] = &C::Exec;
                    m_CommandNameByType[cmdType] = cmdName;
                    m_CommandTypeByName[cmdName] = cmdType;
                    if ( cmdAltName != nullptr )
                    {
                        m_CommandTypeByName[cmdAltName] = cmdType;
                    }
                    m_SortedCommandNames.push_back(cmdName); // does _not_ include cmdAltName!
                }

                void initialize()
                {
                    // 1. register commands
                    registerCommand<DoCommand>();
                    registerCommand<DoCdCommand>();
                    registerCommand<DoClsCommand>();
                    registerCommand<DoClipSetTextCommand>();
                    registerCommand<DoConColourCommand>();
                    registerCommand<DoConFilterCommand>();
                    registerCommand<DoConLoadFromCommand>();
                    registerCommand<DoConSaveToCommand>();
                    registerCommand<DoDirCommand>();
                    registerCommand<DoEchoCommand>();
                    registerCommand<DoElseCommand>();
                    registerCommand<DoEndIfCommand>();
                    registerCommand<DoEnvSetCommand>();
                    registerCommand<DoEnvUnsetCommand>();
                    registerCommand<DoExitCommand>();
                    registerCommand<DoGoToCommand>();
                    registerCommand<DoHelpCommand>();
                    registerCommand<DoIfCommand>();
                    registerCommand<DoCalcIfCommand>();
                    registerCommand<DoInputBoxCommand>();
                    registerCommand<DoLabelCommand>();
                    registerCommand<DoMessageBoxCommand>();
                    registerCommand<DoNpeCmdAliasCommand>();
                    registerCommand<DoNpeConsoleCommand>();
                    registerCommand<DoNpeDebugLogCommand>();
                    registerCommand<DoNpeNoEmptyVarsCommand>();
                    registerCommand<DoNpeQueueCommand>();
                    registerCommand<DoNpeSendMsgBufLenCommand>();
                    registerCommand<DoNppCloseCommand>();
                    registerCommand<DoNppConsoleCommand>();
                    registerCommand<DoNppExecCommand>();
                    registerCommand<DoNppExecTextCommand>();
                    registerCommand<DoNppMenuCommandCommand>();
                    registerCommand<DoNppOpenCommand>();
                    registerCommand<DoNppRunCommand>();
                    registerCommand<DoNppSaveCommand>();
                    registerCommand<DoNppSaveAsCommand>();
                    registerCommand<DoNppSaveAllCommand>();
                    registerCommand<DoNppSendMsgCommand>();
                    registerCommand<DoNppSendMsgExCommand>();
                    registerCommand<DoNppSetFocusCommand>();
                    registerCommand<DoNppSwitchCommand>();
                    registerCommand<DoProcInputCommand>();
                    registerCommand<DoProcSignalCommand>();
                    registerCommand<DoSleepCommand>();
                    registerCommand<DoSciFindCommand>();
                    registerCommand<DoSciReplaceCommand>();
                    registerCommand<DoSciSendMsgCommand>();
                    registerCommand<DoSelLoadFromCommand>();
                    registerCommand<DoSelSaveToCommand>();
                    registerCommand<DoSelSetTextCommand>();
                    registerCommand<DoSelSetTextExCommand>();
                    registerCommand<DoSetCommand>();
                    registerCommand<DoTextLoadFromCommand>();
                    registerCommand<DoTextSaveToCommand>();
                    registerCommand<DoUnsetCommand>();

                    // 2. sort names
                    m_SortedCommandNames.sort();
                }

            protected:
                CCriticalSection m_csConstruct;
                EXECFUNC m_CommandExecFunc[CMDTYPE_TOTAL_COUNT];
                const TCHAR* m_CommandNameByType[CMDTYPE_TOTAL_COUNT];
                std::map<tstr, eCmdType> m_CommandTypeByName;
                std::list<tstr> m_SortedCommandNames;
        };

    public:
        static CScriptCommandRegistry& GetCommandRegistry()
        {
            return m_CommandRegistry;
        }

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

        static eNppExecCmdPrefix checkNppExecCmdPrefix(const CNppExec* pNppExec, tstr& Cmd, bool bRemovePrefix = true);
        static eCmdType getCmdType(CNppExec* pNppExec, CScriptEngine* pScriptEngine, tstr& Cmd, unsigned int nFlags = ctfUseLogging|ctfReportError);
        static int      getOnOffParam(const tstr& param);
        static bool     isCmdCommentOrEmpty(const CNppExec* pNppExec, tstr& Cmd);
        static bool     isCmdDirective(const CNppExec* pNppExec, tstr& Cmd);
        static int      isCmdNppExecPrefixed(CNppExec* pNppExec, CScriptEngine* pScriptEngine, tstr& cmd, unsigned int nFlags = npfRemovePrefix);
        static bool     isScriptCollateral(const CNppExec* pNppExec, const CListT<tstr>& CmdList);
        static bool     isSkippingThisCommandDueToIfState(eCmdType cmdType, eIfState ifState);
        static bool     usesDelayedVarSubstitution(eCmdType cmdType);
        static eCmdType modifyCommandLine(CScriptEngine* pScriptEngine, tstr& Cmd, eIfState ifState);
        static bool     isLocalParam(tstr& param);
        static bool     isDelayedSubstVar(tstr& param, bool& bKeywordPresent);

        enum eAddCmdFlags {
            acfKeepLineEndings = 0x01,
            acfAddEmptyLines   = 0x02
        };
        static void addCommandToList(CListT<tstr>& CmdList, tstr& Cmd, unsigned int nFlags);
        static tCmdList getCmdListFromText(const TCHAR* pszText, unsigned int nFlags);
        static void removeLineEndings(CListT<tstr>& CmdList);
        static void removeLineEnding(tstr& Cmd);
        static tCmdList getCollateralCmdListForChildProcess(CNppExec* pNppExec, const tCmdList& CmdList);

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

        bool IsPrintingMsgReady() const;

        virtual void ScriptError(eErrorType type, const TCHAR* cszErrorMessage);
        void UndoAbort(const TCHAR* cszMessage);

        void ChildProcessMustBreakAll();
        bool WaitUntilDone(DWORD dwTimeoutMs) const;

        static std::atomic_int nTotalRunningEnginesCount;
        static std::atomic_int nExecTextEnginesCount;

    public:
        typedef struct sLabel {
            CListItemT<tstr>* pLine;
            int nGoToCounter;
        } tLabel;
        typedef std::map< tstr, tLabel > tLabels;

        typedef struct sCmdRange {
            CListItemT<tstr>* pBegin; // points to first cmd
            CListItemT<tstr>* pEnd;   // points _after_ last cmd
        } tCmdRange;

        class SavedConfiguration {
            public:
                SavedConfiguration()
                {
                }

                void setColorTextNorm(const COLORREF& colorTextNorm)
                {
                    mColorTextNorm = colorTextNorm;
                    mHasValues |= fColorTextNorm;
                }

                void setColorBkgnd(const COLORREF& colorBkgnd)
                {
                    mColorBkgnd = colorBkgnd;
                    mHasValues |= fColorBkgnd;
                }

                void setUseEditorColorsInConsole(bool bUseEditorColorsInConsole)
                {
                    mUseEditorColorsInConsole = bUseEditorColorsInConsole;
                    mHasValues |= fUseEditorColorsInConsole;
                }

                void setConFltrInclMask(int nConFltrInclMask)
                {
                    mConFltrInclMask = nConFltrInclMask;
                    mHasValues |= fConFltrInclMask;
                }

                void setConFltrExclMask(int nConFltrExclMask)
                {
                    mConFltrExclMask = nConFltrExclMask;
                    mHasValues |= fConFltrExclMask;
                }

                void setRplcFltrFindMask(int nRplcFltrFindMask)
                {
                    mRplcFltrFindMask = nRplcFltrFindMask;
                    mHasValues |= fRplcFltrFindMask;
                }

                void setRplcFltrCaseMask(int nRplcFltrCaseMask)
                {
                    mRplcFltrCaseMask = nRplcFltrCaseMask;
                    mHasValues |= fRplcFltrCaseMask;
                }

                void setConFltrEnable(bool bConFltrEnable)
                {
                    mConFltrEnable = bConFltrEnable;
                    mHasValues |= fConFltrEnable;
                }

                void setConFltrRplcEnable(bool bConFltrRplcEnable)
                {
                    mConFltrRplcEnable = bConFltrRplcEnable;
                    mHasValues |= fConFltrRplcEnable;
                }

                void setConFltrCompilerErrors(bool bConFltrCompilerErrors)
                {
                    mConFltrCompilerErrors = bConFltrCompilerErrors;
                    mHasValues |= fConFltrCompilerErrors;
                }

                void setConPseudoConsole(bool bConPseudoConsole)
                {
                    mConPseudoConsole = bConPseudoConsole;
                    mHasValues |= fConPseudoConsole;
                }

                void setWarnEffectEnabled(const bool* pWarnEffectEnabled)
                {
                    for ( int i = 0; i < WARN_MAX_FILTER; ++i )
                    {
                        mWarnEffectEnabled[i] = pWarnEffectEnabled[i];
                    }
                    mHasValues |= fWarnEffectEnabled;
                }

                void setEnvVar(const tstr& varName, const tstr& varValue)
                {
                    mEnvVars[varName] = varValue;
                    if ( varValue.IsEmpty() && varValue.GetMemSize() != 0 )
                        mEnvVars[varName].Reserve(1);
                }

                void setConsoleEncoding(unsigned int nConsoleEncoding)
                {
                    mConsoleEncoding = nConsoleEncoding;
                    mHasValues |= fConsoleEncoding;
                }

                void setConsoleCatchShortcutKeys(unsigned int nConsoleCatchShortcutKeys)
                {
                    mConsoleCatchShortcutKeys = nConsoleCatchShortcutKeys;
                    mHasValues |= fConsoleCatchShortcutKeys;
                }

                void setConsoleAnsiEscSeq(int nConsoleAnsiEscSeq)
                {
                    mConsoleAnsiEscSeq = nConsoleAnsiEscSeq;
                    mHasValues |= fConsoleAnsiEscSeq;
                }

                void setExecClipTextMode(int nExecClipTextMode)
                {
                    mExecClipTextMode = nExecClipTextMode;
                    mHasValues |= fExecClipTextMode;
                }

                void setExecSelTextMode(int nExecSelTextMode)
                {
                    mExecSelTextMode = nExecSelTextMode;
                    mHasValues |= fExecSelTextMode;
                }

                void setConsoleAppendMode(bool bConsoleAppendMode)
                {
                    mConsoleAppendMode = bConsoleAppendMode;
                    mHasValues |= fConsoleAppendMode;
                }

                void setConsoleCdCurDir(bool bConsoleCdCurDir)
                {
                    mConsoleCdCurDir = bConsoleCdCurDir;
                    mHasValues |= fConsoleCdCurDir;
                }

                void setConsoleCmdHistory(bool bConsoleCmdHistory)
                {
                    mConsoleCmdHistory = bConsoleCmdHistory;
                    mHasValues |= fConsoleCmdHistory;
                }

                void setConsoleNoIntMsgs(bool bConsoleNoIntMsgs)
                {
                    mConsoleNoIntMsgs = bConsoleNoIntMsgs;
                    mHasValues |= fConsoleNoIntMsgs;
                }

                void setConsolePrintMsgReady(bool bConsolePrintMsgReady)
                {
                    mConsolePrintMsgReady = bConsolePrintMsgReady;
                    mHasValues |= fConsolePrintMsgReady;
                }

                void setConsoleNoCmdAliases(bool bConsoleNoCmdAliases)
                {
                    mConsoleNoCmdAliases = bConsoleNoCmdAliases;
                    mHasValues |= fConsoleNoCmdAliases;
                }

                void setConsoleSetOutputVar(bool bConsoleSetOutputVar)
                {
                    mConsoleSetOutputVar = bConsoleSetOutputVar;
                    mHasValues |= fConsoleSetOutputVar;
                }

                void setConsoleKillProcTree(bool bConsoleKillProcTree)
                {
                    mConsoleKillProcTree = bConsoleKillProcTree;
                    mHasValues |= fConsoleKillProcTree;
                }

                void setConsoleDebugLog(bool bConsoleDebugLog)
                {
                    mConsoleDebugLog = bConsoleDebugLog;
                    mHasValues |= fConsoleDebugLog;
                }

                void setLoggerOutputMode(bool bLoggerOutputMode)
                {
                    mLoggerOutputMode = bLoggerOutputMode;
                    mHasValues |= fLoggerOutputMode;
                }

                void setConsoleNoEmptyVars(bool bConsoleNoEmptyVars)
                {
                    mConsoleNoEmptyVars = bConsoleNoEmptyVars;
                    mHasValues |= fConsoleNoEmptyVars;
                }

                void setConsoleDialogVisible(bool bConsoleDialogVisible)
                {
                    mConsoleDialogVisible = bConsoleDialogVisible;
                    mHasValues |= fConsoleDialogVisible;
                }

                void setConsoleIsOutputEnabled(bool bConsoleIsOutputEnabled)
                {
                    mConsoleIsOutputEnabled = bConsoleIsOutputEnabled;
                    mHasValues |= fConsoleIsOutputEnabled;
                }

                void setSendMsgBufLen(int nSendMsgBufLen)
                {
                    mSendMsgBufLen = nSendMsgBufLen;
                    mHasValues |= fSendMsgBufLen;
                }

                bool hasColorTextNorm() const { return ((mHasValues & fColorTextNorm) != 0); }
                bool hasColorBkgnd() const { return ((mHasValues & fColorBkgnd) != 0); }
                bool hasUseEditorColorsInConsole() const { return ((mHasValues & fUseEditorColorsInConsole) != 0); }
                bool hasConFltrInclMask() const { return ((mHasValues & fConFltrInclMask) != 0); }
                bool hasConFltrExclMask() const { return ((mHasValues & fConFltrExclMask) != 0); }
                bool hasRplcFltrFindMask() const { return ((mHasValues & fRplcFltrFindMask) != 0); }
                bool hasRplcFltrCaseMask() const { return ((mHasValues & fRplcFltrCaseMask) != 0); }
                bool hasConFltrEnable() const { return ((mHasValues & fConFltrEnable) != 0); }
                bool hasConFltrRplcEnable() const { return ((mHasValues & fConFltrRplcEnable) != 0); }
                bool hasConFltrCompilerErrors() const { return ((mHasValues & fConFltrCompilerErrors) != 0); }
                bool hasConPseudoConsole() const { return ((mHasValues & fConPseudoConsole) != 0); }
                bool hasWarnEffectEnabled() const { return ((mHasValues & fWarnEffectEnabled) != 0); }
                bool hasEnvVar(const tstr& varName) const { return (mEnvVars.find(varName) != mEnvVars.end()); }
                bool hasConsoleEncoding() const { return ((mHasValues & fConsoleEncoding) != 0); }
                bool hasConsoleCatchShortcutKeys() const { return ((mHasValues & fConsoleCatchShortcutKeys) != 0); }
                bool hasConsoleAnsiEscSeq() const { return ((mHasValues & fConsoleAnsiEscSeq) != 0); }
                bool hasExecClipTextMode() const { return ((mHasValues & fExecClipTextMode) != 0); }
                bool hasExecSelTextMode() const { return ((mHasValues & fExecSelTextMode) != 0); }
                bool hasConsoleAppendMode() const { return ((mHasValues & fConsoleAppendMode) != 0); }
                bool hasConsoleCdCurDir() const { return ((mHasValues & fConsoleCdCurDir) != 0); }
                bool hasConsoleCmdHistory() const { return ((mHasValues & fConsoleCmdHistory) != 0); }
                bool hasConsoleNoIntMsgs() const { return ((mHasValues & fConsoleNoIntMsgs) != 0); }
                bool hasConsolePrintMsgReady() const { return ((mHasValues & fConsolePrintMsgReady) != 0); }
                bool hasConsoleNoCmdAliases() const { return ((mHasValues & fConsoleNoCmdAliases) != 0); }
                bool hasConsoleSetOutputVar() const { return ((mHasValues & fConsoleSetOutputVar) != 0); }
                bool hasConsoleKillProcTree() const { return ((mHasValues & fConsoleKillProcTree) != 0); }
                bool hasConsoleDebugLog() const { return ((mHasValues & fConsoleDebugLog) != 0); }
                bool hasLoggerOutputMode() const { return ((mHasValues & fLoggerOutputMode) != 0); }
                bool hasConsoleNoEmptyVars() const { return ((mHasValues & fConsoleNoEmptyVars) != 0); }
                bool hasConsoleDialogVisible() const { return ((mHasValues & fConsoleDialogVisible) != 0); }
                bool hasConsoleIsOutputEnabled() const { return ((mHasValues & fConsoleIsOutputEnabled) != 0); }
                bool hasSendMsgBufLen() const { return ((mHasValues & fSendMsgBufLen) != 0); }

                int getConFltrInclMask() const { return mConFltrInclMask; }
                int getConFltrExclMask() const { return mConFltrExclMask; }
                int getRplcFltrFindMask() const { return mRplcFltrFindMask; }
                int getRplcFltrCaseMask() const { return mRplcFltrCaseMask; }
                const bool* getWarnEffectEnabled() const { return mWarnEffectEnabled; }
                unsigned int getConsoleEncoding() const { return mConsoleEncoding; }

                void removeColorTextNorm() { mHasValues &= ~fColorTextNorm; }
                void removeColorBkgnd() { mHasValues &= ~fColorBkgnd; }
                void removeUseEditorColorsInConsole() { mHasValues &= ~fUseEditorColorsInConsole; }
                void removeConFltrInclMask() { mHasValues &= ~fConFltrInclMask; }
                void removeConFltrExclMask() { mHasValues &= ~fConFltrExclMask; }
                void removeRplcFltrFindMask() { mHasValues &= ~fRplcFltrFindMask; }
                void removeRplcFltrCaseMask() { mHasValues &= ~fRplcFltrCaseMask; }
                void removeConFltrEnable() { mHasValues &= ~fConFltrEnable; }
                void removeConFltrRplcEnable() { mHasValues &= ~fConFltrRplcEnable; }
                void removeConFltrCompilerErrors() { mHasValues &= ~fConFltrCompilerErrors; }
                void removeConPseudoConsole() { mHasValues &= ~fConPseudoConsole; }
                void removeWarnEffectEnabled() { mHasValues &= ~fWarnEffectEnabled; }
                void removeEnvVar(const tstr& varName) { auto itr = mEnvVars.find(varName); if (itr != mEnvVars.end()) mEnvVars.erase(itr); }
                void removeConsoleEncoding() { mHasValues &= ~fConsoleEncoding; }
                void removeConsoleCatchShortcutKeys() { mHasValues &= ~fConsoleCatchShortcutKeys; }
                void removeConsoleAnsiEscSeq() { mHasValues &= ~fConsoleAnsiEscSeq; }
                void removeExecClipTextMode() { mHasValues &= ~fExecClipTextMode; }
                void removeExecSelTextMode() { mHasValues &= ~fExecSelTextMode; }
                void removeConsoleAppendMode() { mHasValues &= ~fConsoleAppendMode; }
                void removeConsoleCdCurDir() { mHasValues &= ~fConsoleCdCurDir; }
                void removeConsoleCmdHistory() { mHasValues &= ~fConsoleCmdHistory; }
                void removeConsoleNoIntMsgs() { mHasValues &= ~fConsoleNoIntMsgs; }
                void removeConsolePrintMsgReady() { mHasValues &= ~fConsolePrintMsgReady; }
                void removeConsoleNoCmdAliases() { mHasValues &= ~fConsoleNoCmdAliases; }
                void removeConsoleSetOutputVar() { mHasValues &= ~fConsoleSetOutputVar; }
                void removeConsoleKillProcTree() { mHasValues &= ~fConsoleKillProcTree; }
                void removeConsoleDebugLog() { mHasValues &= ~fConsoleDebugLog; }
                void removeLoggerOutputMode() { mHasValues &= ~fLoggerOutputMode; }
                void removeConsoleNoEmptyVars() { mHasValues &= ~fConsoleNoEmptyVars; }
                void removeConsoleDialogVisible() { mHasValues &= ~fConsoleDialogVisible; }
                void removeConsoleIsOutputEnabled() { mHasValues &= ~fConsoleIsOutputEnabled; }
                void removeSendMsgBufLen() { mHasValues &= ~fSendMsgBufLen; }

                void Restore(CNppExec* pNppExec)
                {
                    if ( mHasValues != 0 )
                    {
                        restoreNpeDebugLog(pNppExec);
                        restoreConColors(pNppExec);
                        restoreConFilters(pNppExec);
                        restoreNpeConsole(pNppExec);
                        restoreNpeNoEmptyVars(pNppExec);
                        restoreNpeSendMsgBufLen(pNppExec);
                        restoreNppConsole(pNppExec);
                        restoreUI(pNppExec); // the last one here
                    }
                    if ( !mEnvVars.empty() )
                    {
                        restoreEnvVars();
                    }
                }

            protected:
                void restoreConColors(CNppExec* pNppExec)
                {
                    bool bNeedToUpdateColours = false;
                    if ( hasColorTextNorm() )
                    {
                        pNppExec->GetConsole().SetCurrentColorTextNorm(mColorTextNorm);
                        pNppExec->GetConsole().RestoreDefaultTextStyle(true);
                    }
                    if ( hasColorBkgnd() )
                    {
                        pNppExec->GetConsole().SetCurrentColorBkgnd(mColorBkgnd);
                        bNeedToUpdateColours = true;
                    }
                    if ( hasUseEditorColorsInConsole() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_USEEDITORCOLORS, mUseEditorColorsInConsole);
                        pNppExec->GetConsole().ApplyEditorColours(false);
                    }
                    if ( bNeedToUpdateColours )
                    {
                        pNppExec->GetConsole().UpdateColours();
                    }
                }

                void restoreConFilters(CNppExec* pNppExec)
                {
                    if ( hasConFltrInclMask() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONFLTR_INCLMASK, mConFltrInclMask);
                    }
                    if ( hasConFltrExclMask() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONFLTR_EXCLMASK, mConFltrExclMask);
                    }
                    if ( hasRplcFltrFindMask() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONFLTR_R_FINDMASK, mRplcFltrFindMask);
                    }
                    if ( hasRplcFltrCaseMask() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONFLTR_R_CASEMASK, mRplcFltrCaseMask);
                    }
                    if ( hasConFltrEnable() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONFLTR_ENABLE, mConFltrEnable);
                    }
                    if ( hasConFltrRplcEnable() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONFLTR_R_ENABLE, mConFltrRplcEnable);
                    }
                    if ( hasWarnEffectEnabled() )
                    {
                        CWarningAnalyzer& WarnAn = pNppExec->GetWarningAnalyzer();
                        for ( int i = 0; i < WARN_MAX_FILTER; ++i )
                        {
                            WarnAn.EnableEffect(i, mWarnEffectEnabled[i]);
                        }
                    }
                }

                void restoreNpeConsole(CNppExec* pNppExec)
                {
                    if ( hasConsoleEncoding() )
                    {
                        pNppExec->GetOptions().SetUint(OPTU_CONSOLE_ENCODING, mConsoleEncoding);
                    }
                    if ( hasConsoleCatchShortcutKeys() )
                    {
                        pNppExec->GetOptions().SetUint(OPTU_CONSOLE_CATCHSHORTCUTKEYS, mConsoleCatchShortcutKeys);
                    }
                    if ( hasConFltrCompilerErrors() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONFLTR_COMPILER_ERRORS, mConFltrCompilerErrors);
                    }
                    if ( hasConsoleAnsiEscSeq() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONSOLE_ANSIESCSEQ, mConsoleAnsiEscSeq);
                    }
                    if ( hasConPseudoConsole() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CHILDP_PSEUDOCONSOLE, mConPseudoConsole);
                    }
                    if ( hasExecClipTextMode() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONSOLE_EXECCLIPTEXTMODE, mExecClipTextMode);
                    }
                    if ( hasExecSelTextMode() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_CONSOLE_EXECSELTEXTMODE, mExecSelTextMode);
                    }
                    if ( hasConsoleAppendMode() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_APPENDMODE, mConsoleAppendMode);
                    }
                    if ( hasConsoleCdCurDir() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_CDCURDIR, mConsoleCdCurDir);
                    }
                    if ( hasConsoleCmdHistory() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_CMDHISTORY, mConsoleCmdHistory);
                    }
                    if ( hasConsoleNoIntMsgs() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_NOINTMSGS, mConsoleNoIntMsgs);
                    }
                    if ( hasConsolePrintMsgReady() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_PRINTMSGREADY, mConsolePrintMsgReady);
                    }
                    if ( hasConsoleNoCmdAliases() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_NOCMDALIASES, mConsoleNoCmdAliases);
                    }
                    if ( hasConsoleSetOutputVar() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_SETOUTPUTVAR, mConsoleSetOutputVar);
                    }
                    if ( hasConsoleKillProcTree() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_KILLPROCTREE, mConsoleKillProcTree);
                    }
                }

                void restoreNpeDebugLog(CNppExec* pNppExec)
                {
                    if ( hasConsoleDebugLog() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_NPE_DEBUGLOG, mConsoleDebugLog);
                    }
                    if ( hasLoggerOutputMode() )
                    {
                        if ( mLoggerOutputMode )
                            Runtime::GetLogger().SetOutputMode(true, CNppExec::printScriptString);
                        else
                            Runtime::GetLogger().SetOutputMode(false);
                    }
                }

                void restoreNpeNoEmptyVars(CNppExec* pNppExec)
                {
                    if ( hasConsoleNoEmptyVars() )
                    {
                        pNppExec->GetOptions().SetBool(OPTB_CONSOLE_NOEMPTYVARS, mConsoleNoEmptyVars);
                    }
                }

                void restoreNpeSendMsgBufLen(CNppExec* pNppExec)
                {
                    if ( hasSendMsgBufLen() )
                    {
                        pNppExec->GetOptions().SetInt(OPTI_SENDMSG_MAXBUFLEN, mSendMsgBufLen);
                    }
                }

                void restoreNppConsole(CNppExec* pNppExec)
                {
                    if ( hasConsoleIsOutputEnabled() )
                    {
                        if ( mConsoleIsOutputEnabled )
                        {
                            pNppExec->GetConsole().SetOutputEnabled(true);
                            pNppExec->GetConsole().LockConsoleEndPos();
                        }
                        else
                        {
                            pNppExec->GetConsole().LockConsoleEndPos();
                            pNppExec->GetConsole().SetOutputEnabled(false);
                        }
                    }
                    if ( hasConsoleDialogVisible() )
                    {
                        if ( mConsoleDialogVisible )
                        {
                            pNppExec->showConsoleDialog(CNppExec::showIfHidden, CNppExec::scfCmdNppConsole);
                        }
                        else
                        {
                            pNppExec->showConsoleDialog(CNppExec::hideIfShown, CNppExec::scfCmdNppConsole);
                            pNppExec->_consoleIsVisible = true;
                            auto pScriptEngine = pNppExec->GetCommandExecutor().GetRunningScriptEngine();
                            if ( pScriptEngine )
                                pScriptEngine->updateFocus();
                        }
                    }
                }

                void restoreEnvVars()
                {
                    for ( const auto& v : mEnvVars )
                    {
                        SetEnvironmentVariable( v.name.c_str(), (v.value.GetMemSize() != 0) ? v.value.c_str() : NULL );
                    }
                }

                void restoreUI(CNppExec* pNppExec)
                {
                    extern FuncItem  g_funcItem[nbFunc + MAX_USERMENU_ITEMS + 1];

                    if ( hasConFltrEnable() || hasConFltrRplcEnable() )
                    {
                        pNppExec->UpdateOutputFilterMenuItem();
                    }
                    if ( hasWarnEffectEnabled() )
                    {
                        pNppExec->UpdateGoToErrorMenuItem();
                    }
                    if ( hasConsoleEncoding() )
                    {
                        pNppExec->updateConsoleEncodingFlags();
                        pNppExec->UpdateConsoleEncoding();
                    }
                    
                    if ( hasConsoleCdCurDir() || hasConsoleCmdHistory() ||
                         hasConsoleNoIntMsgs() || hasConsoleNoCmdAliases() )
                    {
                        HMENU hMenu = pNppExec->GetNppMainMenu();
                        if ( hMenu )
                        {
                            if ( hasConsoleCdCurDir() )
                            {
                                ::CheckMenuItem(hMenu, g_funcItem[N_CDCURDIR]._cmdID,
                                    MF_BYCOMMAND | (mConsoleCdCurDir ? MF_CHECKED : MF_UNCHECKED));
                            }
                            if ( hasConsoleCmdHistory() )
                            {
                                ::CheckMenuItem(hMenu, g_funcItem[N_CMDHISTORY]._cmdID,
                                    MF_BYCOMMAND | (mConsoleCmdHistory ? MF_CHECKED : MF_UNCHECKED));
                            }
                            if ( hasConsoleNoIntMsgs() )
                            {
                                ::CheckMenuItem(hMenu, g_funcItem[N_NOINTMSGS]._cmdID,
                                    MF_BYCOMMAND | (mConsoleNoIntMsgs ? MF_CHECKED : MF_UNCHECKED));
                            }
                          #ifdef _DISABLE_CMD_ALIASES
                            if ( hasConsoleNoCmdAliases() )
                            {
                                ::CheckMenuItem(hMenu, g_funcItem[N_NOCMDALIASES]._cmdID,
                                    MF_BYCOMMAND | (mConsoleNoCmdAliases ? MF_CHECKED : MF_UNCHECKED));
                            }
                          #endif
                        }
                    }
                }

            protected:
                enum eHasFlags : unsigned int {
                    fColorTextNorm            = 0x00000001,
                    fColorBkgnd               = 0x00000002,
                    fConFltrInclMask          = 0x00000004,
                    fConFltrExclMask          = 0x00000008,
                    fRplcFltrFindMask         = 0x00000010,
                    fRplcFltrCaseMask         = 0x00000020,
                    fConFltrEnable            = 0x00000040,
                    fConFltrRplcEnable        = 0x00000080,
                    fWarnEffectEnabled        = 0x00000100,
                    fConsoleEncoding          = 0x00000200,
                    fConsoleCatchShortcutKeys = 0x00000400,
                    fConsoleAnsiEscSeq        = 0x00000800,
                    fConsoleAppendMode        = 0x00001000,
                    fConsoleCdCurDir          = 0x00002000,
                    fConsoleCmdHistory        = 0x00004000,
                    fConsoleNoIntMsgs         = 0x00008000,
                    fConsolePrintMsgReady     = 0x00010000,
                    fConsoleNoCmdAliases      = 0x00020000,
                    fConsoleSetOutputVar      = 0x00040000,
                    fConsoleNoEmptyVars       = 0x00080000,
                    fConsoleDialogVisible     = 0x00100000,
                    fConsoleIsOutputEnabled   = 0x00200000,
                    fSendMsgBufLen            = 0x00400000,
                    fConsoleDebugLog          = 0x00800000,
                    fLoggerOutputMode         = 0x01000000,
                    fConFltrCompilerErrors    = 0x02000000,
                    fExecClipTextMode         = 0x04000000,
                    fExecSelTextMode          = 0x08000000,
                    fConsoleKillProcTree      = 0x10000000,
                    fUseEditorColorsInConsole = 0x20000000,
                    fConPseudoConsole         = 0x40000000
                };

                // we might use std::optional (C++17) instead, but would it be so fun? :)
                unsigned int mHasValues{0};

                // DoConColour:
                COLORREF mColorTextNorm{};
                COLORREF mColorBkgnd{};
                bool     mUseEditorColorsInConsole{};

                // DoConFilter:
                int mConFltrInclMask{};
                int mConFltrExclMask{};
                int mRplcFltrFindMask{};
                int mRplcFltrCaseMask{};
                bool mConFltrEnable{};
                bool mConFltrRplcEnable{};
                bool mConFltrCompilerErrors{};
                bool mConPseudoConsole{};
                bool mWarnEffectEnabled[WARN_MAX_FILTER]{};

                // DoNpeConsole:
                unsigned int mConsoleEncoding{};
                unsigned int mConsoleCatchShortcutKeys{};
                int  mConsoleAnsiEscSeq{};
                int  mExecClipTextMode{};
                int  mExecSelTextMode{};
                bool mConsoleAppendMode{};
                bool mConsoleCdCurDir{};
                bool mConsoleCmdHistory{};
                bool mConsoleNoIntMsgs{};
                bool mConsolePrintMsgReady{};
                bool mConsoleNoCmdAliases{};
                bool mConsoleSetOutputVar{};
                bool mConsoleKillProcTree{};

                // DoNpeDebugLog
                bool mConsoleDebugLog{};
                bool mLoggerOutputMode{};

                // DoNpeNoEmptyVars:
                bool mConsoleNoEmptyVars{};

                // DoNpeSendMsgBufLen:
                int mSendMsgBufLen{};

                // DoNppConsole:
                bool mConsoleDialogVisible{};
                bool mConsoleIsOutputEnabled{};

                // DoEnvSet:
                CMacroVars mEnvVars;
        };

        class ScriptContext {
            public:
                struct tIfState {
                    eIfState state;
                    CListItemT<tstr>* pStartLine;
                };

            public:
                tstr               ScriptName;
                tCmdRange          CmdRange;
                tLabels            Labels;
                CMacroVars         LocalMacroVars; // use with GetMacroVars().GetCsUserMacroVars()
                CMacroVars         LocalCmdAliases; // use with GetMacroVars().GetCsCmdAliases()
                SavedConfiguration SavedConf;
                CStrSplitT<TCHAR>  Args;
                bool               IsNppExeced;
                bool               IsSharingLocalVars;
                int                IsPrintingMsgReady;

                ScriptContext()
                {
                    CmdRange.pBegin = 0;
                    CmdRange.pEnd = 0;
                    IsNppExeced = false;
                    IsSharingLocalVars = false;
                    IsPrintingMsgReady = -1;
                }

                ~ScriptContext()
                {
                    SavedConf.Restore( &Runtime::GetNppExec() );
                }

            protected:
                CBufT<tIfState> IfState;

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
                        case IF_EXECUTING_ELSE:
                            cszState = _T("IF_EXECUTING_ELSE");
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
                        strAppendIfState(S, IfState[i].state);
                    }

                    Runtime::GetLogger().AddEx_WithoutOutput( _T("; %s() : IfState[%u] = { %s }"), cszFuncName, IfState.size(), S.c_str() );
                }

            public:
                const int GetIfDepth() const
                {
                    return IfState.size();
                }

                const tIfState GetIfState() const
                {
                    const int nSize = IfState.size();
                    return ((nSize != 0) ? IfState[nSize - 1] : tIfState{IF_NONE, NULL});
                }

                void SetIfState(eIfState ifState)
                {
                    IfState[IfState.size() - 1].state = ifState;

                    logIfState( _T("SetIfState") );
                }

                void PushIfState(tIfState ifState)
                {
                    if ( GetIfState().state == IF_MAYBE_ELSE )
                    {
                        PopIfState(); // remove IF_MAYBE_ELSE because of nested IF
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
            // these 6 variables are needed to prevent infinite loops
            int nExecCounter;
            int nExecMaxCount;
            int nGoToMaxCount;
            int nExecTextCounter;
            int nExecTextMaxCount;
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

        enum eIfMode {
            EMPTY_GOTO = -1,
            IF_GOTO = 0,
            IF_THEN = 1,
            IF_ASSUMING_THEN = 2
        };

        static CScriptCommandRegistry m_CommandRegistry; // one and only, for all script engines

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
        unsigned int   m_nRunFlags;
        DWORD          m_dwThreadId;
        int            m_nPrintingMsgReady;
        bool           m_bTriedExitCmd;
        bool           m_isClosingConsole;
        CEvent         m_eventRunIsDone;
        CEvent         m_eventAbortTheScript;

        void errorCmdNotEnoughParams(const TCHAR* cszCmd, const TCHAR* cszErrorMessage);
        void errorCmdNoParam(const TCHAR* cszCmd);
        void messageConsole(const TCHAR* cszCmd, const TCHAR* cszParams);
        bool reportCmdAndParams(const TCHAR* cszCmd, const tstr& params, unsigned int uFlags);
        void updateFocus();

        static void getLabelName(tstr& labelName);
        static eIfMode getIfMode(const tstr& params, int& n);

        eCmdResult doSendMsg(const tstr& params, int cmdType);
        eCmdResult doSciFindReplace(const tstr& params, eCmdType cmdType);
        eCmdResult doIf(const tstr& params, bool isElseIf, bool isCalc);
        eCmdResult doTextLoad(const tstr& params, eCmdType cmdType);
        eCmdResult doTextSave(const tstr& params, eCmdType cmdType);

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
extern const TCHAR MACRO_NPP_FULL_FILE_PATH[];
extern const TCHAR MACRO_CURRENT_WORD[];
extern const TCHAR MACRO_SELECTED_TEXT[];
extern const TCHAR MACRO_FILE_NAME_AT_CURSOR[];
extern const TCHAR MACRO_WORKSPACE_ITEM_PATH[];
extern const TCHAR MACRO_WORKSPACE_ITEM_DIR[];
extern const TCHAR MACRO_WORKSPACE_ITEM_NAME[];
extern const TCHAR MACRO_WORKSPACE_ITEM_ROOT[];
extern const TCHAR MACRO_CLOUD_LOCATION_PATH[];
extern const TCHAR MACRO_CURRENT_LINE[];
extern const TCHAR MACRO_CURRENT_LINESTR[];
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
extern const TCHAR MACRO_IS_PROCESS[];
extern const TCHAR MACRO_MSG_RESULT[];
extern const TCHAR MACRO_MSG_WPARAM[];
extern const TCHAR MACRO_MSG_LPARAM[];
extern const TCHAR MACRO_EXIT_CMD[];
extern const TCHAR MACRO_EXIT_CMD_SILENT[];
extern const TCHAR MACRO_LAST_CMD_RESULT[];
extern const TCHAR MACRO_CLIPBOARD_TEXT[];
extern const TCHAR MACRO_NPP_HWND[];
extern const TCHAR MACRO_NPP_PID[];
extern const TCHAR MACRO_SCI_HWND[];
extern const TCHAR MACRO_SCI_HWND1[];
extern const TCHAR MACRO_SCI_HWND2[];
extern const TCHAR MACRO_CON_HWND[];
extern const TCHAR MACRO_FOCUSED_HWND[];

extern const TCHAR DIRECTIVE_COLLATERAL[];

//---------------------------------------------------------------------------
#endif
