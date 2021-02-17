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

/****************************************************************************
 * NppExec plugin ver. 0.6.2 for Notepad++
 * by DV <dvv81 @ ukr.net>, December 2006 - February 2021
 * Powered by Function Parser (C) Juha Nieminen, Joel Yliluoma
 ****************************************************************************
 *
 * Abilities:
 *   1) Run multiple commands from the "Execute..." dialog
 *   2) Run stand-alone command from the Console Dlg
 *   3) Separate console for each NppExec.dll (e.g. NppExec1.dll, NppExec2.dll)
 *   4) Additional commands:
 *        cls - clear Console screen
 *        cd - shows current path
 *        cd <path> - changes current directory (absolute or relative)
 *        cd <drive:\path> - changes current drive and directory
 *        dir - lists subdirs and files
 *        dir <mask> - lists subdirs and files matched the mask
 *        dir <path\mask> - lists subdirs and files matched the mask
 *        echo <text> - prints a text in the Console
 *        if <condition> goto <label> - jumps to the label if the condition is true
 *        if ... else if ... else ... endif - conditional execution
 *        goto <label> - jumps to the label
 *        exit - exits the current NppExec's script
 *        exit <type> - exits the NppExec's script
 *        set - shows all user's variables
 *        set <var> - shows the value of user's variable <var>
 *        set <var> = <value> - sets the value of user's variable <var>
 *        set <var> ~ <math expression> - calculates the math expression
 *        set <var> ~ strlen <string> - calculates the string length
 *        set <var> ~ strlenutf8 <string> - calculates the UTF-8 string length
 *        set <var> ~ strlenu <string> - the same as strlenutf8
 *        set <var> ~ strlensci <string> - string length, using Scintilla's encoding
 *        set <var> ~ strlens <string> - the same as strlensci
 *        set <var> ~ strupper <string> - returns the string in upper case
 *        set <var> ~ strlower <string> - returns the string in lower case
 *        set <var> ~ substr <pos> <len> <string> - returns the substring
 *        set <var> ~ strfind <s> <t> - returns the first position of <t> in <s>
 *        set <var> ~ strrfind <s> <t> - returns the last position of <t> in <s>
 *        set <var> ~ strreplace <s> <t0> <t1> - replaces all <t0> with <t1>
 *        set <var> ~ strquote <s>  -  surrounds <s> with "" quotes
 *        set <var> ~ strunquote <s>  -  removes the surrounding "" quotes
 *        set <var> ~ normpath <path>  -  returns a normalized path
 *        set <var> ~ strfromhex <hs> - returns a string from the hex-string
 *        set <var> ~ strtohex <s> - returns a hex-string from the string
 *        set <var> ~ chr <n> - returns a character from a character code <n>
 *        set <var> ~ ord <c> - returns a decimal character code of a character <c>
 *        set <var> ~ ordx <c> - returns a hexadecimal character code of a character <c>
 *        set local - shows all user's local variables
 *        set local <var> - shows the value of user's local variable <var>
 *        set local <var> = ... - sets the value of user's local variable <var>
 *        set local <var> ~ ... - calculates the value of user's local variable
 *        unset <var> - removes user's variable <var>
 *        unset local <var> - removes user's local variable <var>
 *        env_set <var> - shows the value of environment variable <var>
 *        env_set <var> = <value> - sets the value of environment variable <var>
 *        env_unset <var> - removes/restores the environment variable <var>
 *        inputbox "message" - shows InputBox, sets $(INPUT)
 *        inputbox "message" : initial_value - InputBox, sets $(INPUT)
 *        inputbox "message" : "value_name" : initial_value - InputBox customization
 *        messagebox "text" - shows a simple MessageBox
 *        messagebox "text" : "title" - shows a MessageBox with a custom title
 *        messagebox "text" : "title" : type - shows a MessageBox of a given type
 *        con_colour <colours> - sets the Console's colours
 *        con_filter <filters> - enables/disables the Console's output filters
 *        con_loadfrom <file> - loads a file's content to the Console
 *        con_load <file> - see "con_loadfrom"
 *        con_saveto <file> - saves the Console's content to a file
 *        con_save - see "con_saveto"
 *        sel_loadfrom <file> - replace current selection with a file's content
 *        sel_load <file> - see "sel_loadfrom"
 *        sel_saveto <file> - save the selected text to a file
 *        sel_saveto <file> : <encoding> - save the selected text to a file
 *        sel_save <file> : <encoding> - see "sel_saveto"
 *        sel_settext <text> - replace current selection with the text specified
 *        sel_settext+ <text> - replace current selection with the text specified
 *        text_loadfrom <file> - replace the whole text with a file's content
 *        text_load <file> - see "text_loadfrom"
 *        text_saveto <file> - save the whole text to a file
 *        text_saveto <file> : <encoding> - save the whole text to a file
 *        text_save <file> : <encoding> - see "text_saveto"
 *        clip_settext <text> : set the clipboard text
 *        npp_exec <script> - execute commands from specified NppExec's script
 *        npp_exec <file> - execute commands from specified NppExec's file (*)
 *        npp_close - close current file in Notepad++
 *        npp_close <file> - close specified file opened in Notepad++      (*)
 *        npp_console <on/off/keep> - show/hide the Console window
 *        npp_console <enable/disable> - enable/disable output to the Console
 *        npp_console <1/0/?> - show/hide the Console window
 *        npp_console <+/-> - enable/disable output to the Console
 *        npp_menucommand <menu\item\name> - executes (invokes) a menu item
 *        npp_open <file> - open a file in Notepad++
 *        npp_open <mask> - open files matched the mask
 *        npp_open <path\mask> - open files matched the mask
 *        npp_run <command> - run external process/command
 *        npp_save - save current file in Notepad++
 *        npp_save <file> - save a file in Notepad++ (if it's opened)      (*)
 *        npp_saveas <file> - save current file with a new (path)name
 *        npp_saveall - save all modified files
 *        npp_switch <file> - switch to specified opened file              (*)
 *        npp_setfocus - set the keyboard focus
 *        npp_sendmsg <msg> - send a message (msg) to Notepad++
 *        npp_sendmsg <msg> <wparam> - message with parameter (wparam)
 *        npp_sendmsg <msg> <wparam> <lparam> - msg to Notepad++
 *        npp_sendmsgex <hwnd> <msg> <wparam> <lparam> - msg to hwnd
 *        sci_sendmsg <msg> - send a message (msg) to current Scintilla
 *        sci_sendmsg <msg> <wparam> - message with parameter (wparam)
 *        sci_sendmsg <msg> <wparam> <lparam> - msg to Scintilla
 *        sci_find <flags> <find_what> - find a string
 *        sci_replace <flags> <find_what> <replace_with> - replace a string
 *        proc_signal <signal> - signal to a child process
 *        sleep <ms> - sleep during ms milliseconds
 *        sleep <ms> <text> - print the text and sleep during ms milliseconds
 *        npe_cmdalias - show all command aliases
 *        npe_cmdalias <alias> - shows the value of command alias
 *        npe_cmdalias <alias> = - removes the command alias
 *        npe_cmdalias <alias> = <command> - sets the command alias
 *        npe_console <options> - set/modify Console options/mode
 *        npe_debuglog <on/off> - enable/disable Debug Log
 *        npe_debug <1/0> - see "npe_debuglog"
 *        npe_noemptyvars <1/0> - enable/disable replacement of empty vars
 *        npe_queue <command> - queue NppExec's command to be executed
 *        npe_sendmsgbuflen <max_len> - set npp/sci_sendmsg's buffer length
 *        nppexec: - prefix for NppExec's commands (e.g. "nppexec:npp_console off")
 *        nppexec:: - always executes a command in a collateral (parallel) script
 *        (*) these commands work with a partial file path/name also
 *            i.e.  npp_save c:\dir\f.txt  is the same as  npp_save f.txt
 *   5) Additional console commands (Console Dlg only):
 *        help           - show available commands
 *        help <command> - information on the specific command (e.g. "help cls")
 *        ver            - show plugin's version
 *        manual         - show NppExec User Guide
 *        about          - show NppExec Help/About
 *        CTRL+C         - kill (via Ctrl+C) or terminate current child process
 *        CTRL+BREAK     - kill (via Ctrl+Break) or terminate current child process
 *        CTRL+Z         - send ^Z to current child process
 *   6) All Notepad++ environment variables are supported:
 *        $(FULL_CURRENT_PATH)  : E:\my Web\main\welcome.html
 *        $(CURRENT_DIRECTORY)  : E:\my Web\main
 *        $(FILE_NAME)          : welcome.html
 *        $(NAME_PART)          : welcome
 *        $(EXT_PART)           : .html
 *        $(NPP_DIRECTORY)      : the full path of notepad++'s directory
 *        $(NPP_FULL_FILE_PATH) : the full path to notepad++.exe
 *        $(CURRENT_WORD)       : word(s) you selected in Notepad++
 *        $(CURRENT_LINE)       : current line number
 *        $(CURRENT_COLUMN)     : current column number
 *   7) Additional environment variables:
 *        $(FILE_NAME_AT_CURSOR): file name selected in the editor
 *        $(WORKSPACE_ITEM_PATH): full path to the current item in the workspace pane
 *        $(WORKSPACE_ITEM_DIR) : directory containing the current item in the workspace pane
 *        $(WORKSPACE_ITEM_NAME): file name of the current item in the workspace pane
 *        $(WORKSPACE_ITEM_ROOT): root path of the current item in the workspace pane
 *        $(CLIPBOARD_TEXT)     : text from the clipboard
 *        $(#0)                 : C:\Program Files\Notepad++\notepad++.exe
 *        $(#N), N=1,2,3...     : full path of the Nth opened document
 *        $(LEFT_VIEW_FILE)     : current file path-name in primary (left) view
 *        $(RIGHT_VIEW_FILE)    : current file path-name in second (right) view
 *        $(PLUGINS_CONFIG_DIR) : full path of the plugins configuration directory
 *        $(CWD)                : current working directory of NppExec (use "cd" to change it)
 *        $(ARGC)               : number of arguments passed to the NPP_EXEC command
 *        $(ARGV)               : all arguments passed to the NPP_EXEC command after the script name
 *        $(ARGV[0])            : script name - first parameter of the NPP_EXEC command
 *        $(ARGV[N])            : Nth argument (N=1,2,3...)
 *        $(RARGV)              : all arguments in reverse order (except the script name)
 *        $(RARGV[N])           : Nth argument in reverse order (N=1,2,3...)
 *        $(INPUT)              : this value is set by the 'inputbox' command
 *        $(INPUT[N])           : Nth field of the $(INPUT) value (N=1,2,3...)
 *        $(OUTPUT)             : this value can be set by the child process, see npe_console v+
 *        $(OUTPUT1)            : first line in $(OUTPUT)
 *        $(OUTPUTL)            : last line in $(OUTPUT)
 *        $(EXITCODE)           : exit code of the last executed child process
 *        $(PID)                : process id of the current (or the last) child process
 *        $(LAST_CMD_RESULT)    : result of the last NppExec's command
 *                                  (1 - succeeded, 0 - failed, -1 - invalid arg)
 *        $(MSG_RESULT)         : result of 'npp_sendmsg[ex]' or 'sci_sendmsg'
 *        $(MSG_WPARAM)         : wParam (output) of 'npp_sendmsg[ex]' or 'sci_sendmsg'
 *        $(MSG_LPARAM)         : lParam (output) of 'npp_sendmsg[ex]' or 'sci_sendmsg'
 *        $(NPP_HWND)           : Notepad++'s main window handle
 *        $(SCI_HWND)           : current Scintilla's window handle
 *        $(CON_HWND)           : NppExec's Console window handle (RichEdit control)
 *        $(FOCUSED_HWND)       : focused window handle
 *        $(SYS.<var>)          : system's environment variable, e.g. $(SYS.PATH)
 *        $(@EXIT_CMD)          : a callback exit command for a child process
 *        $(@EXIT_CMD_SILENT)   : a silent (non-printed) callback exit command
 *
 ****************************************************************************/

// actually, the following line does nothing :-)
// #define _TEST_ONLY_

#include "NppExec.h"
#include "NppExecEngine.h"
#include "resource.h"
#include "DlgDoExec.h"
#include "DlgHelpAbout.h"
#include "DlgAdvancedOptions.h"
#include "DlgInputBox.h"
#include "DlgConsoleEncoding.h"
#include "cpp/CFileBufT.h"
#include "cpp/StrSplitT.h"
#include "c_base/MatchMask.h"
#include "c_base/HexStr.h"
#include "CFileModificationChecker.h"
#include "encodings/SysUniConv.h"
#include "c_base/str_func.h"
#include "c_base/int2str.h"

#include "npp_files/PluginInterface.h"  // docking feature
#include "npp_files/Docking.h"          // docking feature
#include "npp_files/menuCmdID.h"

#ifndef __MINGW32__
  #include <HtmlHelp.h>
#endif

#include <shellapi.h>
#include <time.h>
#include <limits.h>
#include <algorithm>

#include <CommCtrl.h>


#define  MAX_PLUGIN_NAME  60
TCHAR PLUGIN_NAME[MAX_PLUGIN_NAME] = _T("NppExec");
TCHAR PLUGIN_NAME_DLL[MAX_PLUGIN_NAME + 6] = _T("NppExec.dll");
TCHAR INI_FILENAME[MAX_PLUGIN_NAME + 6] = _T("NppExec.ini");
TCHAR CONSOLE_DLG_TITLE[MAX_PLUGIN_NAME + 10] = _T(" Console ");
TCHAR SHOW_CONSOLE_MENU_ITEM[MAX_PLUGIN_NAME + 10] = _T("Show Console");
TCHAR TOGGLE_CONSOLE_MENU_ITEM[MAX_PLUGIN_NAME + 10] = _T("Toggle Console");

const TCHAR SZ_CONSOLE_HELP_INFO[] = _T("NppExec Help Info:") _T_RE_EOL \
  _T("- You can execute commands and scripts directly from the Console window.") _T_RE_EOL \
  _T("- Commands are case-insensitive (i.e. NPP_OPEN is the same as Npp_Open).") _T_RE_EOL \
  _T("- Type HELP in the Console window to see all available commands and environment variables.") _T_RE_EOL \
  _T("- Type HELP <COMMAND> to get detailed information about specified command (e.g. HELP NPP_EXEC).") _T_RE_EOL \
  _T("- You can find additional information inside these files: \"NppExec.txt\" and \"NppExec_TechInfo.txt\".") _T_RE_EOL \
  _T("- You can disable this Help Info in NppExec\'s Advanced Options.") _T_RE_EOL \
  _T("================ READY ================");

const TCHAR INI_SECTION_CONSOLE[]          = _T("Console");
const TCHAR INI_SECTION_OPTIONS[]          = _T("Options");
const TCHAR INI_SECTION_CONSOLEFILTER[]    = _T("ConsoleOutputFilter");
const TCHAR INI_SECTION_FILTERSEXCLUDE[]   = _T("FiltersExclude");
const TCHAR INI_SECTION_FILTERSINCLUDE[]   = _T("FiltersInclude");
const TCHAR INI_SECTION_FILTERSREPLACE[]   = _T("FiltersReplace");
const TCHAR INI_SECTION_FILTERSHIGHLIGHT[] = _T("FiltersHighlight");
const TCHAR INI_SECTION_USERMENU[]         = _T("UserMenu");
const TCHAR INI_SECTION_INPUTBOX[]         = _T("InputBox");
const TCHAR INI_SECTION_EXITBOX[]          = _T("ExitBox");
const TCHAR INI_SECTION_RESTORE[]          = _T("Restore");

const TCHAR SCRIPTFILE_TEMP[]              = _T("npes_temp.txt");
const TCHAR SCRIPTFILE_SAVED[]             = _T("npes_saved.txt");
TCHAR       CMDHISTORY_FILENAME[100]       = _T("npec_cmdhistory.txt\0");
const TCHAR PROP_NPPEXEC_DLL[]             = _T("NppExec_dll_exists");

const int   DEFAULT_CHILDP_STARTUPTIMEOUT_MS  = 240;
const int   DEFAULT_CHILDP_CYCLETIMEOUT_MS    = 120;
const int   DEFAULT_CHILDP_EXITTIMEOUT_MS     = 2000;
const int   DEFAULT_CHILDP_KILLTIMEOUT_MS     = 500;
const int   DEFAULT_CHILDP_RUNPOLICY          = 0;
const int   DEFAULT_CHILDS_SYNCTIMEOUT_MS     = 200;
const int   DEFAULT_EXITS_TIMEOUT_MS          = 4000;
const int   DEFAULT_PATH_AUTODBLQUOTES        = 0;
const int   DEFAULT_CMDHISTORY_MAXITEMS       = 256;
const int   DEFAULT_EXEC_MAXCOUNT             = 100;
const int   DEFAULT_GOTO_MAXCOUNT             = 10000;
const int   DEFAULT_RICHEDIT_MAXTEXTLEN       = 4*1024*1024; // 4 MB
const int   DEFAULT_SENDMSG_MAXBUFLEN         = 4*1024*1024; // 4 M symbols
const int   DEFAULT_UTF8_DETECT_LENGTH        = 16384;
const int   DEFAULT_CD_UNNAMED                = 1;
const TCHAR DEFAULT_COMMENTDELIMITER[]        = _T("//");
const TCHAR DEFAULT_HELPFILE[]                = _T("doc\\NppExec\\NppExec_Manual.chm");
const TCHAR DEFAULT_LOGSDIR[]                 = _T("");
const TCHAR DEFAULT_SCRIPTSDIR[]              = _T("");
const TCHAR DEFAULT_CHILDP_COMSPECSWITCHES[]  = _T("/C");
const int   DEFAULT_AUTOSAVE_SECONDS          = 0; // disabled (example: 5*60 = 5 minutes)

const wchar_t DEFAULT_NULCHAR_UNICODE     = 0x25E6; // 0x25E6 - the "White Bullet" symbol
const char    DEFAULT_NULCHAR_ANSI        = 0x17; // 0x17 - the "End of Text Block" symbol

const CStaticOptionsManager::OPT_ITEM optArray[OPT_COUNT] = {
    // --- read-write options ---

    // [Options]
    { OPTS_PLUGIN_HOTKEY, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_OPTIONS, _T("HotKey"), 0, _T("F6") },
    { OPTI_TOOLBARBTN,    OPTT_INT | OPTF_READWRITE,
      INI_SECTION_OPTIONS, _T("ToolbarBtn"), 1, NULL },
    { OPTB_WATCHSCRIPTFILE, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_OPTIONS, _T("WatchScriptFile"), 1, NULL },
    { OPTS_SCRIPT_NPPSTART, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_OPTIONS, _T("ScriptNppStart"), 0, NULL },
    { OPTS_SCRIPT_NPPEXIT, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_OPTIONS, _T("ScriptNppExit"), 0, NULL },
    // [Console]
    { OPTI_CONSOLE_VISIBLE, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("Visible"), -1, NULL },
    { OPTB_CONSOLE_SHOWHELP, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("ShowHelp"), -1, NULL },
    { OPTU_CONSOLE_ENCODING, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("Encoding"), -1, NULL },
    { OPTB_CONSOLE_WORDWRAP, OPTT_BOOL | OPTF_READWRITE, 
      INI_SECTION_CONSOLE, _T("WordWrap"), 0, NULL },
    { OPTB_CONSOLE_HIDETOGGLED, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("HideToggled"), 0, NULL },
    { OPTB_CONSOLE_TABASCHAR, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("TabAsChar"), 0, NULL },
    { OPTB_CONSOLE_SAVEONEXECUTE, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("SaveOnExecute"), -1, NULL },
    { OPTB_CONSOLE_CDCURDIR, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("CdCurDir"), 0, NULL },
    { OPTB_CONSOLE_CMDHISTORY, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("CmdHistory"), -1, NULL },
    { OPTB_CONSOLE_SAVECMDHISTORY, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("SaveCmdHistory"), -1, NULL },

    { OPTI_RICHEDIT_MAXTEXTLEN, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("RichEdit_MaxTextLength"),
      DEFAULT_RICHEDIT_MAXTEXTLEN, NULL },
    { OPTI_SENDMSG_MAXBUFLEN, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("SendMsg_MaxBufLength"),
      DEFAULT_SENDMSG_MAXBUFLEN, NULL },
    { OPTS_CALC_PRECISION, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("Calc_Precision"),
      0, NULL }, // see "NppExecEngine.cpp", calc_precision
    { OPTS_COMMENTDELIMITER, OPTT_STR | OPTF_READWRITE, 
      INI_SECTION_CONSOLE, _T("CommentDelimiter"), 
      0, DEFAULT_COMMENTDELIMITER },

    { OPTD_COLOR_TEXTNORM, OPTT_DATA | OPTF_READWRITE, 
      INI_SECTION_CONSOLE, _T("TextColorNormal"), 0, NULL },
    { OPTD_COLOR_TEXTERR, OPTT_DATA | OPTF_READWRITE, 
      INI_SECTION_CONSOLE, _T("TextColorError"), 0, NULL },
    { OPTD_COLOR_TEXTMSG, OPTT_DATA | OPTF_READWRITE, 
      INI_SECTION_CONSOLE, _T("TextColorMessage"), 0, NULL },
    { OPTD_COLOR_BKGND, OPTT_DATA | OPTF_READWRITE, 
      INI_SECTION_CONSOLE, _T("BackgroundColor"), 0, NULL },

  #ifdef _SCROLL_TO_LATEST  
    { OPTB_CONSOLE_SCROLL2LATEST, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("ScrollToLatest"), -1, NULL },
  #endif

    { OPTB_CONSOLE_NOINTMSGS, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("NoInternalMsgs"), -1, NULL },
    { OPTB_CONSOLE_PRINTMSGREADY, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("PrintMsgReady"), -1, NULL },
    { OPTD_CONSOLE_FONT, OPTT_DATA | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("Font"), 0, NULL },
    { OPTI_CONSOLE_ANSIESCSEQ, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLE, _T("AnsiEscapeSequences") },

    //[ConsoleOutputFilter]
    { OPTB_CONFLTR_ENABLE, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("Enable"), 0, NULL },
    { OPTB_CONFLTR_EXCLALLEMPTY, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeAllEmpty"), 0, NULL },
    { OPTB_CONFLTR_EXCLDUPEMPTY, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeDupEmpty"), 0, NULL },
    { OPTI_CONFLTR_INCLMASK, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeMask"), 0, NULL },
    { OPTI_CONFLTR_EXCLMASK, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeMask"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine0"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine1"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine2"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine3"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine4"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine5"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine6"), 0, NULL },
    { OPTS_CONFLTR_INCLLINE8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("IncludeLine7"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine0"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine1"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine2"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine3"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine4"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine5"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine6"), 0, NULL },
    { OPTS_CONFLTR_EXCLLINE8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("ExcludeLine7"), 0, NULL },

    { OPTB_CONFLTR_R_ENABLE, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("REnable"), 0, NULL },
    { OPTB_CONFLTR_R_EXCLEMPTY, OPTT_BOOL | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RExcludeEmpty"), 0, NULL },
    { OPTI_CONFLTR_R_FINDMASK, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFindMask"), 0, NULL },
    { OPTI_CONFLTR_R_CASEMASK, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RCaseMask"), 0, NULL },
    { OPTS_CONFLTR_R_FIND1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind0"), 0, NULL },
    { OPTS_CONFLTR_R_FIND2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind1"), 0, NULL },
    { OPTS_CONFLTR_R_FIND3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind2"), 0, NULL },
    { OPTS_CONFLTR_R_FIND4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind3"), 0, NULL },
    { OPTS_CONFLTR_R_FIND5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind4"), 0, NULL },
    { OPTS_CONFLTR_R_FIND6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind5"), 0, NULL },
    { OPTS_CONFLTR_R_FIND7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind6"), 0, NULL },
    { OPTS_CONFLTR_R_FIND8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RFind7"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc0"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc1"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc2"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc3"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc4"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc5"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc6"), 0, NULL },
    { OPTS_CONFLTR_R_RPLC8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RRplc7"), 0, NULL },

    { OPTS_CONFLTR_RCGNMSK1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask0"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect0"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask1"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect1"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask2"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect2"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask3"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect3"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask4"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect4"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask5"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect5"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask6"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect6"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask7"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect7"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask8"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect8"), 0, NULL },
    { OPTS_CONFLTR_RCGNMSK10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionMask9"), 0, NULL },
    { OPTS_CONFLTR_RCGNEFF10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_CONSOLEFILTER, _T("RecognitionEffect9"), 0, NULL },

    // [Restore]
    { OPTI_SELECTED_SCRIPT, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_RESTORE, _T("LastSelectedScript"), 0, NULL },
    { OPTI_DOEXECDLG_WIDTH, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_RESTORE, _T("DoExecDlg_Width"), -1, NULL },
    { OPTI_DOEXECDLG_HEIGHT, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_RESTORE, _T("DoExecDlg_Height"), -1, NULL },
  
    // [FiltersExclude]
    { OPTS_FILTERS_EXCL1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("0"), 0, NULL },
    { OPTS_FILTERS_EXCL2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("1"), 0, NULL },
    { OPTS_FILTERS_EXCL3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("2"), 0, NULL },
    { OPTS_FILTERS_EXCL4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("3"), 0, NULL },
    { OPTS_FILTERS_EXCL5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("4"), 0, NULL },
    { OPTS_FILTERS_EXCL6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("5"), 0, NULL },
    { OPTS_FILTERS_EXCL7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("6"), 0, NULL },
    { OPTS_FILTERS_EXCL8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("7"), 0, NULL },
    { OPTS_FILTERS_EXCL9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("8"), 0, NULL },
    { OPTS_FILTERS_EXCL10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("9"), 0, NULL },
    { OPTS_FILTERS_EXCL11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("10"), 0, NULL },
    { OPTS_FILTERS_EXCL12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("11"), 0, NULL },
    { OPTS_FILTERS_EXCL13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("12"), 0, NULL },
    { OPTS_FILTERS_EXCL14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("13"), 0, NULL },
    { OPTS_FILTERS_EXCL15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("14"), 0, NULL },
    { OPTS_FILTERS_EXCL16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("15"), 0, NULL },
    { OPTS_FILTERS_EXCL17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("16"), 0, NULL },
    { OPTS_FILTERS_EXCL18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("17"), 0, NULL },
    { OPTS_FILTERS_EXCL19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("18"), 0, NULL },
    { OPTS_FILTERS_EXCL20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSEXCLUDE, _T("19"), 0, NULL },

    // [FiltersInclude]
    { OPTS_FILTERS_INCL1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("0"), 0, NULL },
    { OPTS_FILTERS_INCL2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("1"), 0, NULL },
    { OPTS_FILTERS_INCL3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("2"), 0, NULL },
    { OPTS_FILTERS_INCL4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("3"), 0, NULL },
    { OPTS_FILTERS_INCL5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("4"), 0, NULL },
    { OPTS_FILTERS_INCL6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("5"), 0, NULL },
    { OPTS_FILTERS_INCL7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("6"), 0, NULL },
    { OPTS_FILTERS_INCL8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("7"), 0, NULL },
    { OPTS_FILTERS_INCL9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("8"), 0, NULL },
    { OPTS_FILTERS_INCL10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("9"), 0, NULL },
    { OPTS_FILTERS_INCL11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("10"), 0, NULL },
    { OPTS_FILTERS_INCL12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("11"), 0, NULL },
    { OPTS_FILTERS_INCL13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("12"), 0, NULL },
    { OPTS_FILTERS_INCL14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("13"), 0, NULL },
    { OPTS_FILTERS_INCL15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("14"), 0, NULL },
    { OPTS_FILTERS_INCL16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("15"), 0, NULL },
    { OPTS_FILTERS_INCL17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("16"), 0, NULL },
    { OPTS_FILTERS_INCL18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("17"), 0, NULL },
    { OPTS_FILTERS_INCL19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("18"), 0, NULL },
    { OPTS_FILTERS_INCL20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSINCLUDE, _T("19"), 0, NULL },

    // [FiltersReplace]
    { OPTS_FILTERS_R_FIND1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F0"), 0, NULL },
    { OPTS_FILTERS_R_FIND2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F1"), 0, NULL },
    { OPTS_FILTERS_R_FIND3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F2"), 0, NULL },
    { OPTS_FILTERS_R_FIND4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F3"), 0, NULL },
    { OPTS_FILTERS_R_FIND5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F4"), 0, NULL },
    { OPTS_FILTERS_R_FIND6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F5"), 0, NULL },
    { OPTS_FILTERS_R_FIND7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F6"), 0, NULL },
    { OPTS_FILTERS_R_FIND8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F7"), 0, NULL },
    { OPTS_FILTERS_R_FIND9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F8"), 0, NULL },
    { OPTS_FILTERS_R_FIND10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F9"), 0, NULL },
    { OPTS_FILTERS_R_FIND11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F10"), 0, NULL },
    { OPTS_FILTERS_R_FIND12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F11"), 0, NULL },
    { OPTS_FILTERS_R_FIND13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F12"), 0, NULL },
    { OPTS_FILTERS_R_FIND14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F13"), 0, NULL },
    { OPTS_FILTERS_R_FIND15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F14"), 0, NULL },
    { OPTS_FILTERS_R_FIND16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F15"), 0, NULL },
    { OPTS_FILTERS_R_FIND17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F16"), 0, NULL },
    { OPTS_FILTERS_R_FIND18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F17"), 0, NULL },
    { OPTS_FILTERS_R_FIND19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F18"), 0, NULL },
    { OPTS_FILTERS_R_FIND20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("F19"), 0, NULL },
    { OPTS_FILTERS_R_RPLC1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R0"), 0, NULL },
    { OPTS_FILTERS_R_RPLC2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R1"), 0, NULL },
    { OPTS_FILTERS_R_RPLC3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R2"), 0, NULL },
    { OPTS_FILTERS_R_RPLC4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R3"), 0, NULL },
    { OPTS_FILTERS_R_RPLC5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R4"), 0, NULL },
    { OPTS_FILTERS_R_RPLC6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R5"), 0, NULL },
    { OPTS_FILTERS_R_RPLC7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R6"), 0, NULL },
    { OPTS_FILTERS_R_RPLC8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R7"), 0, NULL },
    { OPTS_FILTERS_R_RPLC9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R8"), 0, NULL },
    { OPTS_FILTERS_R_RPLC10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R9"), 0, NULL },
    { OPTS_FILTERS_R_RPLC11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R10"), 0, NULL },
    { OPTS_FILTERS_R_RPLC12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R11"), 0, NULL },
    { OPTS_FILTERS_R_RPLC13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R12"), 0, NULL },
    { OPTS_FILTERS_R_RPLC14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R13"), 0, NULL },
    { OPTS_FILTERS_R_RPLC15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R14"), 0, NULL },
    { OPTS_FILTERS_R_RPLC16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R15"), 0, NULL },
    { OPTS_FILTERS_R_RPLC17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R16"), 0, NULL },
    { OPTS_FILTERS_R_RPLC18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R17"), 0, NULL },
    { OPTS_FILTERS_R_RPLC19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R18"), 0, NULL },
    { OPTS_FILTERS_R_RPLC20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSREPLACE, _T("R19"), 0, NULL },
      
    // [FiltersHighlight]
    { OPTS_FILTERS_HGLT1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("0"), 0, NULL },
    { OPTS_FILTERS_HGLT2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("1"), 0, NULL },
    { OPTS_FILTERS_HGLT3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("2"), 0, NULL },
    { OPTS_FILTERS_HGLT4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("3"), 0, NULL },
    { OPTS_FILTERS_HGLT5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("4"), 0, NULL },
    { OPTS_FILTERS_HGLT6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("5"), 0, NULL },
    { OPTS_FILTERS_HGLT7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("6"), 0, NULL },
    { OPTS_FILTERS_HGLT8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("7"), 0, NULL },
    { OPTS_FILTERS_HGLT9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("8"), 0, NULL },
    { OPTS_FILTERS_HGLT10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("9"), 0, NULL },
    { OPTS_FILTERS_HGLT11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("10"), 0, NULL },
    { OPTS_FILTERS_HGLT12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("11"), 0, NULL },
    { OPTS_FILTERS_HGLT13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("12"), 0, NULL },
    { OPTS_FILTERS_HGLT14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("13"), 0, NULL },
    { OPTS_FILTERS_HGLT15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("14"), 0, NULL },
    { OPTS_FILTERS_HGLT16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("15"), 0, NULL },
    { OPTS_FILTERS_HGLT17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("16"), 0, NULL },
    { OPTS_FILTERS_HGLT18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("17"), 0, NULL },
    { OPTS_FILTERS_HGLT19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("18"), 0, NULL },
    { OPTS_FILTERS_HGLT20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_FILTERSHIGHLIGHT, _T("19"), 0, NULL },
  
    // [UserMenu]
    { OPTB_USERMENU_NPPMACROS, OPTT_INT | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("NppMacrosSubmenu"), 0, NULL },
    { OPTS_USERMENU_ITEM01, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("0"), 0, NULL },
    { OPTS_USERMENU_ITEM02, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("1"), 0, NULL },
    { OPTS_USERMENU_ITEM03, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("2"), 0, NULL },
    { OPTS_USERMENU_ITEM04, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("3"), 0, NULL },
    { OPTS_USERMENU_ITEM05, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("4"), 0, NULL },
    { OPTS_USERMENU_ITEM06, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("5"), 0, NULL },
    { OPTS_USERMENU_ITEM07, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("6"), 0, NULL },
    { OPTS_USERMENU_ITEM08, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("7"), 0, NULL },
    { OPTS_USERMENU_ITEM09, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("8"), 0, NULL },
    { OPTS_USERMENU_ITEM10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("9"), 0, NULL },
    { OPTS_USERMENU_ITEM11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("10"), 0, NULL },
    { OPTS_USERMENU_ITEM12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("11"), 0, NULL },
    { OPTS_USERMENU_ITEM13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("12"), 0, NULL },
    { OPTS_USERMENU_ITEM14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("13"), 0, NULL },
    { OPTS_USERMENU_ITEM15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("14"), 0, NULL },
    { OPTS_USERMENU_ITEM16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("15"), 0, NULL },
    { OPTS_USERMENU_ITEM17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("16"), 0, NULL },
    { OPTS_USERMENU_ITEM18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("17"), 0, NULL },
    { OPTS_USERMENU_ITEM19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("18"), 0, NULL },
    { OPTS_USERMENU_ITEM20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("19"), 0, NULL },
    { OPTS_USERMENU_ITEM21, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("20"), 0, NULL },
    { OPTS_USERMENU_ITEM22, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("21"), 0, NULL },
    { OPTS_USERMENU_ITEM23, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("22"), 0, NULL },
    { OPTS_USERMENU_ITEM24, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("23"), 0, NULL },
    { OPTS_USERMENU_ITEM25, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("24"), 0, NULL },
    { OPTS_USERMENU_ITEM26, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("25"), 0, NULL },
    { OPTS_USERMENU_ITEM27, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("26"), 0, NULL },
    { OPTS_USERMENU_ITEM28, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("27"), 0, NULL },
    { OPTS_USERMENU_ITEM29, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("28"), 0, NULL },
    { OPTS_USERMENU_ITEM30, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("29"), 0, NULL },
    { OPTS_USERMENU_ITEM31, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("30"), 0, NULL },
    { OPTS_USERMENU_ITEM32, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("31"), 0, NULL },
    { OPTS_USERMENU_ITEM33, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("32"), 0, NULL },
    { OPTS_USERMENU_ITEM34, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("33"), 0, NULL },
    { OPTS_USERMENU_ITEM35, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("34"), 0, NULL },
    { OPTS_USERMENU_ITEM36, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("35"), 0, NULL },
    { OPTS_USERMENU_ITEM37, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("36"), 0, NULL },
    { OPTS_USERMENU_ITEM38, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("37"), 0, NULL },
    { OPTS_USERMENU_ITEM39, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("38"), 0, NULL },
    { OPTS_USERMENU_ITEM40, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("39"), 0, NULL },
    { OPTS_USERMENU_ITEM41, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("40"), 0, NULL },
    { OPTS_USERMENU_ITEM42, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("41"), 0, NULL },
    { OPTS_USERMENU_ITEM43, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("42"), 0, NULL },
    { OPTS_USERMENU_ITEM44, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("43"), 0, NULL },
    { OPTS_USERMENU_ITEM45, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("44"), 0, NULL },
    { OPTS_USERMENU_ITEM46, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("45"), 0, NULL },
    { OPTS_USERMENU_ITEM47, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("46"), 0, NULL },
    { OPTS_USERMENU_ITEM48, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("47"), 0, NULL },
    { OPTS_USERMENU_ITEM49, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("48"), 0, NULL },
    { OPTS_USERMENU_ITEM50, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("49"), 0, NULL },
    { OPTS_USERMENU_ITEM51, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("50"), 0, NULL },
    { OPTS_USERMENU_ITEM52, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("51"), 0, NULL },
    { OPTS_USERMENU_ITEM53, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("52"), 0, NULL },
    { OPTS_USERMENU_ITEM54, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("53"), 0, NULL },
    { OPTS_USERMENU_ITEM55, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("54"), 0, NULL },
    { OPTS_USERMENU_ITEM56, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("55"), 0, NULL },
    { OPTS_USERMENU_ITEM57, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("56"), 0, NULL },
    { OPTS_USERMENU_ITEM58, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("57"), 0, NULL },
    { OPTS_USERMENU_ITEM59, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("58"), 0, NULL },
    { OPTS_USERMENU_ITEM60, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("59"), 0, NULL },
    { OPTS_USERMENU_ITEM61, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("60"), 0, NULL },
    { OPTS_USERMENU_ITEM62, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("61"), 0, NULL },
    { OPTS_USERMENU_ITEM63, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("62"), 0, NULL },
    { OPTS_USERMENU_ITEM64, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("63"), 0, NULL },
    { OPTS_USERMENU_ITEM65, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("64"), 0, NULL },
    { OPTS_USERMENU_ITEM66, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("65"), 0, NULL },
    { OPTS_USERMENU_ITEM67, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("66"), 0, NULL },
    { OPTS_USERMENU_ITEM68, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("67"), 0, NULL },
    { OPTS_USERMENU_ITEM69, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("68"), 0, NULL },
    { OPTS_USERMENU_ITEM70, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("69"), 0, NULL },
    { OPTS_USERMENU_ITEM71, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("70"), 0, NULL },
    { OPTS_USERMENU_ITEM72, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("71"), 0, NULL },
    { OPTS_USERMENU_ITEM73, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("72"), 0, NULL },
    { OPTS_USERMENU_ITEM74, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("73"), 0, NULL },
    { OPTS_USERMENU_ITEM75, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("74"), 0, NULL },
    { OPTS_USERMENU_ITEM76, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("75"), 0, NULL },
    { OPTS_USERMENU_ITEM77, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("76"), 0, NULL },
    { OPTS_USERMENU_ITEM78, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("77"), 0, NULL },
    { OPTS_USERMENU_ITEM79, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("78"), 0, NULL },
    { OPTS_USERMENU_ITEM80, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("79"), 0, NULL },
    { OPTS_USERMENU_ITEM81, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("80"), 0, NULL },
    { OPTS_USERMENU_ITEM82, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("81"), 0, NULL },
    { OPTS_USERMENU_ITEM83, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("82"), 0, NULL },
    { OPTS_USERMENU_ITEM84, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("83"), 0, NULL },
    { OPTS_USERMENU_ITEM85, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("84"), 0, NULL },
    { OPTS_USERMENU_ITEM86, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("85"), 0, NULL },
    { OPTS_USERMENU_ITEM87, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("86"), 0, NULL },
    { OPTS_USERMENU_ITEM88, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("87"), 0, NULL },
    { OPTS_USERMENU_ITEM89, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("88"), 0, NULL },
    { OPTS_USERMENU_ITEM90, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("89"), 0, NULL },
    { OPTS_USERMENU_ITEM91, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("90"), 0, NULL },
    { OPTS_USERMENU_ITEM92, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("91"), 0, NULL },
    { OPTS_USERMENU_ITEM93, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("92"), 0, NULL },
    { OPTS_USERMENU_ITEM94, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("93"), 0, NULL },
    { OPTS_USERMENU_ITEM95, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("94"), 0, NULL },
    { OPTS_USERMENU_ITEM96, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("95"), 0, NULL },
    { OPTS_USERMENU_ITEM97, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("96"), 0, NULL },
    { OPTS_USERMENU_ITEM98, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("97"), 0, NULL },
    { OPTS_USERMENU_ITEM99, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("98"), 0, NULL },
    { OPTS_USERMENU_ITEM100, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_USERMENU, _T("99"), 0, NULL },

    // [InputBox]
    { OPTS_INPUTBOX_VALUE1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I0"), 0, NULL },
    { OPTS_INPUTBOX_VALUE2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I1"), 0, NULL },
    { OPTS_INPUTBOX_VALUE3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I2"), 0, NULL },
    { OPTS_INPUTBOX_VALUE4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I3"), 0, NULL },
    { OPTS_INPUTBOX_VALUE5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I4"), 0, NULL },
    { OPTS_INPUTBOX_VALUE6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I5"), 0, NULL },
    { OPTS_INPUTBOX_VALUE7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I6"), 0, NULL },
    { OPTS_INPUTBOX_VALUE8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I7"), 0, NULL },
    { OPTS_INPUTBOX_VALUE9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I8"), 0, NULL },
    { OPTS_INPUTBOX_VALUE10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I9"), 0, NULL },
    { OPTS_INPUTBOX_VALUE11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I10"), 0, NULL },
    { OPTS_INPUTBOX_VALUE12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I11"), 0, NULL },
    { OPTS_INPUTBOX_VALUE13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I12"), 0, NULL },
    { OPTS_INPUTBOX_VALUE14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I13"), 0, NULL },
    { OPTS_INPUTBOX_VALUE15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I14"), 0, NULL },
    { OPTS_INPUTBOX_VALUE16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I15"), 0, NULL },
    { OPTS_INPUTBOX_VALUE17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I16"), 0, NULL },
    { OPTS_INPUTBOX_VALUE18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I17"), 0, NULL },
    { OPTS_INPUTBOX_VALUE19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I18"), 0, NULL },
    { OPTS_INPUTBOX_VALUE20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_INPUTBOX, _T("I19"), 0, NULL },

    // [ExitBox]
    { OPTS_EXITBOX_VALUE1, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I0"), 0, NULL },
    { OPTS_EXITBOX_VALUE2, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I1"), 0, NULL },
    { OPTS_EXITBOX_VALUE3, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I2"), 0, NULL },
    { OPTS_EXITBOX_VALUE4, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I3"), 0, NULL },
    { OPTS_EXITBOX_VALUE5, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I4"), 0, NULL },
    { OPTS_EXITBOX_VALUE6, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I5"), 0, NULL },
    { OPTS_EXITBOX_VALUE7, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I6"), 0, NULL },
    { OPTS_EXITBOX_VALUE8, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I7"), 0, NULL },
    { OPTS_EXITBOX_VALUE9, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I8"), 0, NULL },
    { OPTS_EXITBOX_VALUE10, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I9"), 0, NULL },
    { OPTS_EXITBOX_VALUE11, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I10"), 0, NULL },
    { OPTS_EXITBOX_VALUE12, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I11"), 0, NULL },
    { OPTS_EXITBOX_VALUE13, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I12"), 0, NULL },
    { OPTS_EXITBOX_VALUE14, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I13"), 0, NULL },
    { OPTS_EXITBOX_VALUE15, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I14"), 0, NULL },
    { OPTS_EXITBOX_VALUE16, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I15"), 0, NULL },
    { OPTS_EXITBOX_VALUE17, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I16"), 0, NULL },
    { OPTS_EXITBOX_VALUE18, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I17"), 0, NULL },
    { OPTS_EXITBOX_VALUE19, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I18"), 0, NULL },
    { OPTS_EXITBOX_VALUE20, OPTT_STR | OPTF_READWRITE,
      INI_SECTION_EXITBOX, _T("I19"), 0, NULL },
      
    // --- read-only options ---

    // [Options]
    { OPTS_PLUGIN_HELPFILE, OPTT_STR | OPTF_READONLY,
      INI_SECTION_OPTIONS, _T("HelpFile"),
      0, DEFAULT_HELPFILE },
    { OPTS_PLUGIN_LOGSDIR, OPTT_STR | OPTF_READONLY,
      INI_SECTION_OPTIONS, _T("LogsDir"),
      0, DEFAULT_LOGSDIR },
    { OPTS_PLUGIN_SCRIPTSDIR, OPTT_STR | OPTF_READONLY,
      INI_SECTION_OPTIONS, _T("ScriptsDir"),
      0, DEFAULT_SCRIPTSDIR },
    { OPTU_PLUGIN_AUTOSAVE_SECONDS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_OPTIONS, _T("AutoSave_Seconds"),
      DEFAULT_AUTOSAVE_SECONDS, NULL },
    // [Console]
    { OPTU_CHILDP_STARTUPTIMEOUT_MS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildProcess_StartupTimeout_ms"), 
      DEFAULT_CHILDP_STARTUPTIMEOUT_MS, NULL },
    { OPTU_CHILDP_CYCLETIMEOUT_MS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildProcess_CycleTimeout_ms"), 
      DEFAULT_CHILDP_CYCLETIMEOUT_MS, NULL },
    { OPTU_CHILDP_EXITTIMEOUT_MS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildProcess_ExitTimeout_ms"),
      DEFAULT_CHILDP_EXITTIMEOUT_MS, NULL },
    { OPTU_CHILDP_KILLTIMEOUT_MS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildProcess_KillTimeout_ms"),
      DEFAULT_CHILDP_KILLTIMEOUT_MS, NULL },
    { OPTU_CHILDP_RUNPOLICY, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildProcess_RunPolicy"),
      DEFAULT_CHILDP_RUNPOLICY, NULL },
    { OPTS_CHILDP_COMSPECSWITCHES, OPTT_STR | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildProcess_ComSpecSwitches"),
      0, DEFAULT_CHILDP_COMSPECSWITCHES },
    { OPTU_CHILDS_SYNCTIMEOUT_MS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ChildScript_SyncTimeout_ms"),
      DEFAULT_CHILDS_SYNCTIMEOUT_MS, NULL },
    { OPTU_EXITS_TIMEOUT_MS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("ExitScript_Timeout_ms"),
      DEFAULT_EXITS_TIMEOUT_MS, NULL },
    { OPTB_PATH_AUTODBLQUOTES, OPTT_BOOL | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("Path_AutoDblQuotes"), 
      DEFAULT_PATH_AUTODBLQUOTES, NULL },
    { OPTI_CMDHISTORY_MAXITEMS, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("CmdHistory_MaxItems"), 
      DEFAULT_CMDHISTORY_MAXITEMS, NULL },
    { OPTI_EXEC_MAXCOUNT, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("Exec_MaxCount"), 
      DEFAULT_EXEC_MAXCOUNT, NULL },
    { OPTI_GOTO_MAXCOUNT, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("GoTo_MaxCount"),
      DEFAULT_GOTO_MAXCOUNT, NULL },
    { OPTB_CONSOLE_NOEMPTYVARS, OPTT_BOOL | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("NoEmptyVars"), 1, NULL },
    { OPTS_ALIAS_CMD_NPPEXEC, OPTT_STR | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("Alias_Cmd_NppExec"), 
      0, DEFAULT_ALIAS_CMD_NPPEXEC },
    { OPTS_KEY_ENTER, OPTT_STR | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("Key_Enter"),
      0, DEFAULT_KEY_ENTER },
    { OPTD_CONSOLE_NULCHAR, OPTT_DATA | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("Console_NulChar"),
      0, NULL }, // <-- this value is adjusted inside ReadOptions()
    { OPTS_NPPEXEC_CMD_PREFIX, OPTT_STR | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("NppExec_Cmd_Prefix"),
      0, DEFAULT_NPPEXEC_CMD_PREFIX },
    { OPTI_UTF8_DETECT_LENGTH, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("UTF8_Detect_Length"),
      DEFAULT_UTF8_DETECT_LENGTH, NULL },
    { OPTI_CONSOLE_CD_UNNAMED, OPTT_INT | OPTF_READONLY,
      INI_SECTION_CONSOLE, _T("Cd_Unnamed"),
      DEFAULT_CD_UNNAMED, NULL },

    // --- internal options ---
    { OPTU_PLUGIN_HOTKEY, OPTT_INT | OPTF_INTERNAL,
      NULL, NULL, VK_F6, NULL },
    { OPTB_NPE_DEBUGLOG, OPTT_BOOL | OPTF_INTERNAL,
      NULL, NULL, 0, NULL },
    { OPTB_CONSOLE_APPENDMODE, OPTT_BOOL | OPTF_INTERNAL,
      NULL, NULL, 0, NULL },
    { OPTU_CONSOLE_CATCHSHORTCUTKEYS, OPTT_INT | OPTF_INTERNAL,
      NULL, NULL, ConsoleDlg::CSK_ALL, NULL },
    { OPTB_CONSOLE_SETOUTPUTVAR, OPTT_BOOL | OPTF_INTERNAL,
      NULL, NULL, 0, NULL },
    { OPTB_CONSOLE_NOCMDALIASES, OPTT_BOOL | OPTF_INTERNAL,
      NULL, NULL, 0, NULL }
};

bool              g_bInitialized = false;
int               g_nUserMenuItems = 0;
FuncItem          g_funcItem[nbFunc + MAX_USERMENU_ITEMS + 1];
ShortcutKey       g_funcShortcut[nbFunc + MAX_USERMENU_ITEMS + 1];
CNppExec          g_nppExec;

namespace Runtime
{
    CNppExec& GetNppExec()
    {
        return g_nppExec;
    }

    CSimpleLogger& GetLogger()
    {
        return g_nppExec.GetLogger();
    }

};

COLORREF g_colorTextNorm = COLOR_CON_TEXTNORM;
COLORREF g_colorTextErr  = COLOR_CON_TEXTERR;
COLORREF g_colorTextMsg  = COLOR_CON_TEXTMSG;
COLORREF g_colorBkgnd    = COLOR_CON_BKGND;


extern BOOL                     g_bIsNppUnicode;
extern WNDPROC                  nppOriginalWndProc;
extern CConsoleOutputFilterDlg  ConsoleOutputFilterDlg;
extern CConsoleEncodingDlg      ConsoleEncodingDlg;
extern CInputBoxDlg             InputBoxDlg;
extern CFileModificationChecker g_scriptFileChecker;


void empty_func()           { /* empty function */ }
void cmdhistory_func()      { Runtime::GetNppExec().OnCmdHistory(); }
void do_exec_dlg_func()     { Runtime::GetNppExec().OnDoExecDlg(); }
void direct_exec_func()     { Runtime::GetNppExec().OnDirectExec(tstr(), true, CScriptEngine::rfConsoleLocalVarsRead); }
void show_console_func()    { Runtime::GetNppExec().OnShowConsoleDlg(); }
void toggle_console_func()  { Runtime::GetNppExec().OnToggleConsoleDlg(); }
void go_to_next_error()     { Runtime::GetNppExec().OnGoToNextError(); }
void go_to_prev_error()     { Runtime::GetNppExec().OnGoToPrevError(); }
void console_enc_func()     { Runtime::GetNppExec().OnConsoleEncoding(); }
void output_f_func()        { Runtime::GetNppExec().OnOutputFilter(); }
void adv_opt_func()         { Runtime::GetNppExec().OnAdvancedOptions(); }
void saveonexecute_func()   { Runtime::GetNppExec().OnSaveOnExecute(); }
void cdcurdir_func()        { Runtime::GetNppExec().OnCdCurDir(); }
void usermenu_item00_func() { Runtime::GetNppExec().OnUserMenuItem(0); }
void usermenu_item01_func() { Runtime::GetNppExec().OnUserMenuItem(1); }
void usermenu_item02_func() { Runtime::GetNppExec().OnUserMenuItem(2); }
void usermenu_item03_func() { Runtime::GetNppExec().OnUserMenuItem(3); }
void usermenu_item04_func() { Runtime::GetNppExec().OnUserMenuItem(4); }
void usermenu_item05_func() { Runtime::GetNppExec().OnUserMenuItem(5); }
void usermenu_item06_func() { Runtime::GetNppExec().OnUserMenuItem(6); }
void usermenu_item07_func() { Runtime::GetNppExec().OnUserMenuItem(7); }
void usermenu_item08_func() { Runtime::GetNppExec().OnUserMenuItem(8); }
void usermenu_item09_func() { Runtime::GetNppExec().OnUserMenuItem(9); }
void usermenu_item10_func() { Runtime::GetNppExec().OnUserMenuItem(10); }
void usermenu_item11_func() { Runtime::GetNppExec().OnUserMenuItem(11); }
void usermenu_item12_func() { Runtime::GetNppExec().OnUserMenuItem(12); }
void usermenu_item13_func() { Runtime::GetNppExec().OnUserMenuItem(13); }
void usermenu_item14_func() { Runtime::GetNppExec().OnUserMenuItem(14); }
void usermenu_item15_func() { Runtime::GetNppExec().OnUserMenuItem(15); }
void usermenu_item16_func() { Runtime::GetNppExec().OnUserMenuItem(16); }
void usermenu_item17_func() { Runtime::GetNppExec().OnUserMenuItem(17); }
void usermenu_item18_func() { Runtime::GetNppExec().OnUserMenuItem(18); }
void usermenu_item19_func() { Runtime::GetNppExec().OnUserMenuItem(19); }
void usermenu_item20_func() { Runtime::GetNppExec().OnUserMenuItem(20); }
void usermenu_item21_func() { Runtime::GetNppExec().OnUserMenuItem(21); }
void usermenu_item22_func() { Runtime::GetNppExec().OnUserMenuItem(22); }
void usermenu_item23_func() { Runtime::GetNppExec().OnUserMenuItem(23); }
void usermenu_item24_func() { Runtime::GetNppExec().OnUserMenuItem(24); }
void usermenu_item25_func() { Runtime::GetNppExec().OnUserMenuItem(25); }
void usermenu_item26_func() { Runtime::GetNppExec().OnUserMenuItem(26); }
void usermenu_item27_func() { Runtime::GetNppExec().OnUserMenuItem(27); }
void usermenu_item28_func() { Runtime::GetNppExec().OnUserMenuItem(28); }
void usermenu_item29_func() { Runtime::GetNppExec().OnUserMenuItem(29); }
void usermenu_item30_func() { Runtime::GetNppExec().OnUserMenuItem(30); }
void usermenu_item31_func() { Runtime::GetNppExec().OnUserMenuItem(31); }
void usermenu_item32_func() { Runtime::GetNppExec().OnUserMenuItem(32); }
void usermenu_item33_func() { Runtime::GetNppExec().OnUserMenuItem(33); }
void usermenu_item34_func() { Runtime::GetNppExec().OnUserMenuItem(34); }
void usermenu_item35_func() { Runtime::GetNppExec().OnUserMenuItem(35); }
void usermenu_item36_func() { Runtime::GetNppExec().OnUserMenuItem(36); }
void usermenu_item37_func() { Runtime::GetNppExec().OnUserMenuItem(37); }
void usermenu_item38_func() { Runtime::GetNppExec().OnUserMenuItem(38); }
void usermenu_item39_func() { Runtime::GetNppExec().OnUserMenuItem(39); }
void usermenu_item40_func() { Runtime::GetNppExec().OnUserMenuItem(40); }
void usermenu_item41_func() { Runtime::GetNppExec().OnUserMenuItem(41); }
void usermenu_item42_func() { Runtime::GetNppExec().OnUserMenuItem(42); }
void usermenu_item43_func() { Runtime::GetNppExec().OnUserMenuItem(43); }
void usermenu_item44_func() { Runtime::GetNppExec().OnUserMenuItem(44); }
void usermenu_item45_func() { Runtime::GetNppExec().OnUserMenuItem(45); }
void usermenu_item46_func() { Runtime::GetNppExec().OnUserMenuItem(46); }
void usermenu_item47_func() { Runtime::GetNppExec().OnUserMenuItem(47); }
void usermenu_item48_func() { Runtime::GetNppExec().OnUserMenuItem(48); }
void usermenu_item49_func() { Runtime::GetNppExec().OnUserMenuItem(49); }
void usermenu_item50_func() { Runtime::GetNppExec().OnUserMenuItem(50); }
void usermenu_item51_func() { Runtime::GetNppExec().OnUserMenuItem(51); }
void usermenu_item52_func() { Runtime::GetNppExec().OnUserMenuItem(52); }
void usermenu_item53_func() { Runtime::GetNppExec().OnUserMenuItem(53); }
void usermenu_item54_func() { Runtime::GetNppExec().OnUserMenuItem(54); }
void usermenu_item55_func() { Runtime::GetNppExec().OnUserMenuItem(55); }
void usermenu_item56_func() { Runtime::GetNppExec().OnUserMenuItem(56); }
void usermenu_item57_func() { Runtime::GetNppExec().OnUserMenuItem(57); }
void usermenu_item58_func() { Runtime::GetNppExec().OnUserMenuItem(58); }
void usermenu_item59_func() { Runtime::GetNppExec().OnUserMenuItem(59); }
void usermenu_item60_func() { Runtime::GetNppExec().OnUserMenuItem(60); }
void usermenu_item61_func() { Runtime::GetNppExec().OnUserMenuItem(61); }
void usermenu_item62_func() { Runtime::GetNppExec().OnUserMenuItem(62); }
void usermenu_item63_func() { Runtime::GetNppExec().OnUserMenuItem(63); }
void usermenu_item64_func() { Runtime::GetNppExec().OnUserMenuItem(64); }
void usermenu_item65_func() { Runtime::GetNppExec().OnUserMenuItem(65); }
void usermenu_item66_func() { Runtime::GetNppExec().OnUserMenuItem(66); }
void usermenu_item67_func() { Runtime::GetNppExec().OnUserMenuItem(67); }
void usermenu_item68_func() { Runtime::GetNppExec().OnUserMenuItem(68); }
void usermenu_item69_func() { Runtime::GetNppExec().OnUserMenuItem(69); }
void usermenu_item70_func() { Runtime::GetNppExec().OnUserMenuItem(70); }
void usermenu_item71_func() { Runtime::GetNppExec().OnUserMenuItem(71); }
void usermenu_item72_func() { Runtime::GetNppExec().OnUserMenuItem(72); }
void usermenu_item73_func() { Runtime::GetNppExec().OnUserMenuItem(73); }
void usermenu_item74_func() { Runtime::GetNppExec().OnUserMenuItem(74); }
void usermenu_item75_func() { Runtime::GetNppExec().OnUserMenuItem(75); }
void usermenu_item76_func() { Runtime::GetNppExec().OnUserMenuItem(76); }
void usermenu_item77_func() { Runtime::GetNppExec().OnUserMenuItem(77); }
void usermenu_item78_func() { Runtime::GetNppExec().OnUserMenuItem(78); }
void usermenu_item79_func() { Runtime::GetNppExec().OnUserMenuItem(79); }
void usermenu_item80_func() { Runtime::GetNppExec().OnUserMenuItem(80); }
void usermenu_item81_func() { Runtime::GetNppExec().OnUserMenuItem(81); }
void usermenu_item82_func() { Runtime::GetNppExec().OnUserMenuItem(82); }
void usermenu_item83_func() { Runtime::GetNppExec().OnUserMenuItem(83); }
void usermenu_item84_func() { Runtime::GetNppExec().OnUserMenuItem(84); }
void usermenu_item85_func() { Runtime::GetNppExec().OnUserMenuItem(85); }
void usermenu_item86_func() { Runtime::GetNppExec().OnUserMenuItem(86); }
void usermenu_item87_func() { Runtime::GetNppExec().OnUserMenuItem(87); }
void usermenu_item88_func() { Runtime::GetNppExec().OnUserMenuItem(88); }
void usermenu_item89_func() { Runtime::GetNppExec().OnUserMenuItem(89); }
void usermenu_item90_func() { Runtime::GetNppExec().OnUserMenuItem(90); }
void usermenu_item91_func() { Runtime::GetNppExec().OnUserMenuItem(91); }
void usermenu_item92_func() { Runtime::GetNppExec().OnUserMenuItem(92); }
void usermenu_item93_func() { Runtime::GetNppExec().OnUserMenuItem(93); }
void usermenu_item94_func() { Runtime::GetNppExec().OnUserMenuItem(94); }
void usermenu_item95_func() { Runtime::GetNppExec().OnUserMenuItem(95); }
void usermenu_item96_func() { Runtime::GetNppExec().OnUserMenuItem(96); }
void usermenu_item97_func() { Runtime::GetNppExec().OnUserMenuItem(97); }
void usermenu_item98_func() { Runtime::GetNppExec().OnUserMenuItem(98); }
void usermenu_item99_func() { Runtime::GetNppExec().OnUserMenuItem(99); }

void (*pUserMenuFuncs[MAX_USERMENU_ITEMS])() = {
    usermenu_item00_func,
    usermenu_item01_func,
    usermenu_item02_func,
    usermenu_item03_func,
    usermenu_item04_func,
    usermenu_item05_func,
    usermenu_item06_func,
    usermenu_item07_func,
    usermenu_item08_func,
    usermenu_item09_func,
    usermenu_item10_func,
    usermenu_item11_func,
    usermenu_item12_func,
    usermenu_item13_func,
    usermenu_item14_func,
    usermenu_item15_func,
    usermenu_item16_func,
    usermenu_item17_func,
    usermenu_item18_func,
    usermenu_item19_func,
    usermenu_item20_func,
    usermenu_item21_func,
    usermenu_item22_func,
    usermenu_item23_func,
    usermenu_item24_func,
    usermenu_item25_func,
    usermenu_item26_func,
    usermenu_item27_func,
    usermenu_item28_func,
    usermenu_item29_func,
    usermenu_item30_func,
    usermenu_item31_func,
    usermenu_item32_func,
    usermenu_item33_func,
    usermenu_item34_func,
    usermenu_item35_func,
    usermenu_item36_func,
    usermenu_item37_func,
    usermenu_item38_func,
    usermenu_item39_func,
    usermenu_item40_func,
    usermenu_item41_func,
    usermenu_item42_func,
    usermenu_item43_func,
    usermenu_item44_func,
    usermenu_item45_func,
    usermenu_item46_func,
    usermenu_item47_func,
    usermenu_item48_func,
    usermenu_item49_func,
    usermenu_item50_func,
    usermenu_item51_func,
    usermenu_item52_func,
    usermenu_item53_func,
    usermenu_item54_func,
    usermenu_item55_func,
    usermenu_item56_func,
    usermenu_item57_func,
    usermenu_item58_func,
    usermenu_item59_func,
    usermenu_item60_func,
    usermenu_item61_func,
    usermenu_item62_func,
    usermenu_item63_func,
    usermenu_item64_func,
    usermenu_item65_func,
    usermenu_item66_func,
    usermenu_item67_func,
    usermenu_item68_func,
    usermenu_item69_func,
    usermenu_item70_func,
    usermenu_item71_func,
    usermenu_item72_func,
    usermenu_item73_func,
    usermenu_item74_func,
    usermenu_item75_func,
    usermenu_item76_func,
    usermenu_item77_func,
    usermenu_item78_func,
    usermenu_item79_func,
    usermenu_item80_func,
    usermenu_item81_func,
    usermenu_item82_func,
    usermenu_item83_func,
    usermenu_item84_func,
    usermenu_item85_func,
    usermenu_item86_func,
    usermenu_item87_func,
    usermenu_item88_func,
    usermenu_item89_func,
    usermenu_item90_func,
    usermenu_item91_func,
    usermenu_item92_func,
    usermenu_item93_func,
    usermenu_item94_func,
    usermenu_item95_func,
    usermenu_item96_func,
    usermenu_item97_func,
    usermenu_item98_func,
    usermenu_item99_func
};

#ifdef _SCROLL_TO_LATEST
void scroll2latest_func() { Runtime::GetNppExec().OnScrollToLatest(); }
#endif

void int_msgs_func()      { Runtime::GetNppExec().OnNoInternalMsgs(); }
void nocmdaliases_func()  { Runtime::GetNppExec().OnNoCmdAliases(); }
void console_font_func()  { Runtime::GetNppExec().OnSelectConsoleFont(); } 
void help_manual_func()   { Runtime::GetNppExec().OnHelpManual(); }
void help_docs_func()     { Runtime::GetNppExec().OnHelpDocs(); }
void help_about_func()    { Runtime::GetNppExec().OnHelpAbout(); }


void InitFuncItem(int            nItem,
                  const TCHAR*   szName, 
                  PFUNCPLUGINCMD pFunc, 
                  //bool           bCheck,
                  ShortcutKey*   pShortcut)
{
  lstrcpy(g_funcItem[nItem]._itemName, szName);
  g_funcItem[nItem]._pFunc = pFunc;
  g_funcItem[nItem]._init2Check = false; //bCheck;
  g_funcItem[nItem]._pShKey = pShortcut;
}

void InitShortcut(int nItem, bool isAlt, bool isCtrl, bool isShift, unsigned int key)
{
  g_funcShortcut[nItem]._isAlt = isAlt;
  g_funcShortcut[nItem]._isCtrl = isCtrl;
  g_funcShortcut[nItem]._isShift = isShift;
  g_funcShortcut[nItem]._key = static_cast<unsigned char>(key);
}

void globalInitialize()
{
  CNppExec& NppExec = Runtime::GetNppExec();
  NppExec.Init();

  // ... Plugin menu ...
  
  // empty shortcuts initialization:
  ::ZeroMemory(g_funcShortcut, (nbFunc + MAX_USERMENU_ITEMS + 1)*sizeof(ShortcutKey));
  
  // init shortcuts:
  InitShortcut(N_DO_EXEC_DLG, false, false, false, NppExec.GetOptions().GetUint(OPTU_PLUGIN_HOTKEY));
  InitShortcut(N_DIRECT_EXEC, false, true, false, NppExec.GetOptions().GetUint(OPTU_PLUGIN_HOTKEY));
  InitShortcut(N_OUTPUT_FILTER, false, false, true,  NppExec.GetOptions().GetUint(OPTU_PLUGIN_HOTKEY));
  InitShortcut(N_TOGGLECONSOLE, false, true,  false, VK_OEM_3); // the '~' key

  // init menu items:
  InitFuncItem(N_DO_EXEC_DLG,     _T("Execute..."),                    do_exec_dlg_func,    &g_funcShortcut[N_DO_EXEC_DLG]);
  InitFuncItem(N_DIRECT_EXEC,     _T("Direct Execute Previous"),       direct_exec_func,    &g_funcShortcut[N_DIRECT_EXEC]);
  InitFuncItem(N_SHOWCONSOLE,     SHOW_CONSOLE_MENU_ITEM,              show_console_func,   &g_funcShortcut[N_SHOWCONSOLE]);
  InitFuncItem(N_TOGGLECONSOLE,   TOGGLE_CONSOLE_MENU_ITEM,            toggle_console_func, &g_funcShortcut[N_TOGGLECONSOLE]);
  InitFuncItem(N_GOTO_NEXT_ERROR, _T("Go to next error"),              go_to_next_error,    NULL);
  InitFuncItem(N_GOTO_PREV_ERROR, _T("Go to previous error"),          go_to_prev_error,    NULL);
  InitFuncItem(N_SEPARATOR_1,     _T(""),                              /*empty_func*/NULL,  NULL);
  InitFuncItem(N_CMDHISTORY,      _T("Console Commands History"),      cmdhistory_func,     NULL);
  InitFuncItem(N_CONSOLE_ENC,     _T("Console Output..."),             console_enc_func,    NULL);

#ifdef _SCROLL_TO_LATEST      
  InitFuncItem(N_SCROLL2LATEST,   _T("Always scroll to latest line"),  scroll2latest_func,  NULL);
#endif

  InitFuncItem(N_NOINTMSGS,       _T("No internal messages"),          int_msgs_func,       NULL);
  InitFuncItem(N_SAVEONEXECUTE,   _T("Save all files on execute"),     saveonexecute_func,  NULL);
  InitFuncItem(N_CDCURDIR,        _T("Follow $(CURRENT_DIRECTORY)"),   cdcurdir_func,       NULL);
  InitFuncItem(N_NOCMDALIASES,    _T("Disable command aliases"),       nocmdaliases_func,   &g_funcShortcut[N_NOCMDALIASES]);
  InitFuncItem(N_SEPARATOR_2,     _T(""),                              /*empty_func*/NULL,  NULL);
  InitFuncItem(N_OUTPUT_FILTER,   _T("Console Output Filters..."),     output_f_func,       &g_funcShortcut[N_OUTPUT_FILTER]);
  InitFuncItem(N_ADV_OPTIONS,     _T("Advanced Options..."),           adv_opt_func,        NULL);
  InitFuncItem(N_CONSOLE_FONT,    _T("Change Console Font..."),        console_font_func,   NULL);
  InitFuncItem(N_SEPARATOR_3,     _T(""),                              /*empty_func*/NULL,  NULL);
  InitFuncItem(N_HELP_MANUAL,     _T("Help/Manual"),                   help_manual_func,    NULL);
  InitFuncItem(N_HELP_DOCS,       _T("Help/Docs..."),                  help_docs_func,      NULL);
  InitFuncItem(N_HELP_ABOUT,      _T("Help/About..."),                 help_about_func,     NULL);

  for ( int i = 0; i < MAX_USERMENU_ITEMS; i++ )
  {
    int len = 0;
    const TCHAR* pStr = NppExec.GetOptions().GetStr(OPTS_USERMENU_ITEM01 + i, &len);
    if ( len > 0 )
    {
      TCHAR szMenuItem[MAX_SCRIPTNAME];
      
      lstrcpy(szMenuItem, pStr);

      len = c_base::_tstr_unsafe_rfind(szMenuItem, len, cszUserMenuItemSep);
      if ( len >= 0 )
        szMenuItem[len] = 0;
      if ( (len < 0) && (lstrcmp(szMenuItem, cszUserMenuSeparator) == 0) )
      {
        InitFuncItem(nbFunc + g_nUserMenuItems + 1, // g_funcItem[nbFunc] is a separator
          _T(""), empty_func, NULL); // separator, use empty_func here!!!
      }
      else
      {
        InitFuncItem(nbFunc + g_nUserMenuItems + 1, // g_funcItem[nbFunc] is a separator
          szMenuItem, pUserMenuFuncs[g_nUserMenuItems], NULL);
      }
      ++g_nUserMenuItems;
    }
  }

  if ( g_nUserMenuItems > 0 )
  {
    // separator
    InitFuncItem(nbFunc, _T(""), empty_func, NULL); // <-- use empty_func here!!!
  }

  g_bInitialized = true;
}

void globalUninitialize()
{
  if ( g_bInitialized )
  {
    CNppExec& NppExec = Runtime::GetNppExec();
    NppExec.StopAutoSaveTimer();
    if ( !NppExec._bOptionsSavedOnNppnShutdown )
    {
      NppExec.SaveOptions();
    }
    NppExec.Uninit();

    g_bInitialized = false;
  }
}

extern "C" BOOL APIENTRY DllMain( 
                       HINSTANCE hInstance, 
                       DWORD     dwReason, 
                       LPVOID    /*lpReserved*/ )
{
  Runtime::GetNppExec().m_hDllModule = (HMODULE) hInstance;
  
  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
    {
      g_bInitialized = false;
      CNppExec::_bIsNppReady = false;

      Runtime::GetNppExec().InitPluginName((HMODULE) hInstance);
        
      //now in the setInfo:
      //globalInitialize();

    }                                                      
      break;

    case DLL_PROCESS_DETACH:
    {
      globalUninitialize();
      CNppExec::_bIsNppReady = false;

    }
      break;

    case DLL_THREAD_ATTACH:
      break;

    case DLL_THREAD_DETACH:
      break;

    default:
      break;
  }

  return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
    CNppExec& NppExec = Runtime::GetNppExec();
    NppExec.m_nppData = notpadPlusData;

    if ( ::GetProp(notpadPlusData._nppHandle, PROP_NPPEXEC_DLL) )
    {
        NppExec.m_bAnotherNppExecDllExists = true;
    }
    ::SetProp(notpadPlusData._nppHandle, PROP_NPPEXEC_DLL, NppExec.m_hDllModule);

    int ver = (int) ::SendMessage(notpadPlusData._nppHandle, NPPM_GETNPPVERSION, 0, 0);
    if ( (HIWORD(ver) < 5) ||
         ((HIWORD(ver) == 5) && (LOWORD(ver) < 1)) )
    {
      ::MessageBox(
          notpadPlusData._nppHandle, 
          _T("This version of NppExec requires Notepad++ ver. 5.1 or higher"),
          _T("NppExec Initialization Error"),
          MB_OK | MB_ICONERROR
      );
      return;
    }

    g_bIsNppUnicode = ::IsWindowUnicode(notpadPlusData._nppHandle);
    
    globalInitialize();
    
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
  return PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
  if (g_nUserMenuItems > 0)
    *nbF = nbFunc + g_nUserMenuItems + 1;
  else
    *nbF = nbFunc;
  return g_funcItem;
}

void UpdateCurrentDirectory()
{
    CNppExec& NppExec = Runtime::GetNppExec();
    if ( NppExec.GetOptions().GetBool(OPTB_CONSOLE_CDCURDIR) )
    {
        TCHAR szFileDir[FILEPATH_BUFSIZE]; 

        szFileDir[0] = 0;
        NppExec.SendNppMsg( NPPM_GETCURRENTDIRECTORY,
          (WPARAM) (FILEPATH_BUFSIZE - 1), (LPARAM) szFileDir);
        if ( szFileDir[0] )
        {
            ::SetCurrentDirectory(szFileDir);
        }
        else
        {
            // dealing with an unnamed file...
            int nCdUnnamed = NppExec.GetOptions().GetInt(OPTI_CONSOLE_CD_UNNAMED);
            if ( nCdUnnamed == 1 )
            {
                szFileDir[0] = 0;
                ::GetTempPath(FILEPATH_BUFSIZE - 1, szFileDir);
                if ( szFileDir[0] )
                {
                    ::SetCurrentDirectory(szFileDir);
                }
            }
        }
    }
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
    static bool bNppReady = false;

    if ( notifyCode->nmhdr.code == NPPN_SHORTCUTREMAPPED )
    {
        const int n = (g_nUserMenuItems > 0) ? (nbFunc + g_nUserMenuItems + 1) : nbFunc;
        for ( int i = 0; i < n; i++ )
        {
            if ( static_cast<unsigned int>(g_funcItem[i]._cmdID) == notifyCode->nmhdr.idFrom )
            {
                if ( g_funcItem[i]._pFunc && (g_funcItem[i]._pFunc != empty_func) )
                {
                    ShortcutKey* pKey = (ShortcutKey *) notifyCode->nmhdr.hwndFrom;
                    ::CopyMemory( &g_funcShortcut[i], pKey, sizeof(ShortcutKey) );
                }
                break;
            }
        }
        return;
    }
  
    if ( notifyCode->nmhdr.hwndFrom == Runtime::GetNppExec().m_nppData._nppHandle )
    {
  
        if ( notifyCode->nmhdr.code == NPPN_BUFFERACTIVATED || 
             notifyCode->nmhdr.code == NPPN_FILESAVED )
        {
            if ( bNppReady )
                UpdateCurrentDirectory();
        }

        else if ( notifyCode->nmhdr.code == NPPN_READY )
        {
            CNppExec& NppExec = Runtime::GetNppExec();

            if ( Runtime::GetLogger().IsLogFileOpen() )
            {
                Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; NPPN_READY - start") );
                Runtime::GetLogger().IncIndentLevel();
            }

            bNppReady = true;
            CNppExec::_bIsNppReady = true;
            NppExec._bStopTheExitScript = false; // just in case

            HANDLE hProp = ::GetProp(NppExec.m_nppData._nppHandle, PROP_NPPEXEC_DLL);
            if ( hProp && (hProp != NppExec.m_hDllModule) )
            {
                NppExec.m_bAnotherNppExecDllExists = true;
            }

// get rid of the "conversion from 'LONG_PTR' to 'LONG'" shit
// get rid of the "conversion from 'LONG' to 'WNDPROC'" shit
// (Microsoft itself does not know what is 'LONG_PTR'?)
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4312)

#ifdef UNICODE
            nppOriginalWndProc = (WNDPROC) SetWindowLongPtrW( 
                NppExec.m_nppData._nppHandle, GWLP_WNDPROC, 
                (LONG_PTR) nppPluginWndProc );
#else
            if ( g_bIsNppUnicode )
            {
                nppOriginalWndProc = (WNDPROC) SetWindowLongPtrW( 
                  NppExec.m_nppData._nppHandle, GWLP_WNDPROC, 
                    (LONG_PTR) nppPluginWndProc );
            }
            else
            {
                nppOriginalWndProc = (WNDPROC) SetWindowLongPtrA( 
                  NppExec.m_nppData._nppHandle, GWLP_WNDPROC, 
                    (LONG_PTR) nppPluginWndProc );
            }
#endif

#pragma warning(pop)

            UpdateCurrentDirectory();
        
            HMENU hMenu = NppExec.GetNppMainMenu();
            if ( hMenu )
            {
                ModifyMenu(hMenu, g_funcItem[N_SEPARATOR_1]._cmdID, 
                  MF_BYCOMMAND | MF_SEPARATOR, g_funcItem[N_SEPARATOR_1]._cmdID, NULL);

                ModifyMenu(hMenu, g_funcItem[N_SEPARATOR_2]._cmdID, 
                  MF_BYCOMMAND | MF_SEPARATOR, g_funcItem[N_SEPARATOR_2]._cmdID, NULL);

                ModifyMenu(hMenu, g_funcItem[N_SEPARATOR_3]._cmdID, 
                  MF_BYCOMMAND | MF_SEPARATOR, g_funcItem[N_SEPARATOR_3]._cmdID, NULL);

                if ( g_nUserMenuItems > 0 )
                {
                    TCHAR        szItemText[128];
                    MENUITEMINFO mii;
                    HMENU        hMacrosMenu = NULL;
            
                    if ( NppExec.GetOptions().GetBool(OPTB_USERMENU_NPPMACROS) )
                    {
                        int iMenuItems = GetMenuItemCount(hMenu);
                        for ( int i = 0; i < iMenuItems; i++ )
                        {
                            HMENU hSubMenu = ::GetSubMenu(hMenu, i);
                            // does IDM_MACRO_STOPRECORDINGMACRO exist here?
                            if ( ::GetMenuState(hSubMenu, IDM_MACRO_STOPRECORDINGMACRO, MF_BYCOMMAND) != -1 )
                            {
                                // this is the Macro sub-menu
                                hMacrosMenu = hSubMenu;
                                break;
                            }
                        }
                    }

                    for ( int i = nbFunc; i < nbFunc + g_nUserMenuItems + 1; i++ )
                    {
                        szItemText[0] = 0;
                        ::ZeroMemory( &mii, sizeof(MENUITEMINFO) );
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
                        mii.dwTypeData = (TCHAR*) szItemText;
                        mii.cch = 128 - 1;

                        if ( ::GetMenuItemInfo(hMenu, g_funcItem[i]._cmdID, FALSE, &mii) )
                        {
                            if ( hMacrosMenu )
                            {
                                ::RemoveMenu( hMenu, g_funcItem[i]._cmdID, MF_BYCOMMAND );
                                if ( (i == nbFunc) || (szItemText[0] == 0) )
                                    ::AppendMenu( hMacrosMenu, MF_SEPARATOR, 0, NULL ); // separator
                                else
                                    ::InsertMenuItem( hMacrosMenu, g_funcItem[i]._cmdID, FALSE, &mii );
                            }
                            else
                            {
                                if ( (i == nbFunc) || (szItemText[0] == 0) )
                                {
                                    // separator
                                    mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING | MIIM_SUBMENU;
                                    mii.fType = MFT_SEPARATOR;
                                    mii.wID = 0;
                                    ::SetMenuItemInfo(hMenu, g_funcItem[i]._cmdID, FALSE, &mii);
                                }
                            }
                        }
                    }

                } // <--- g_nUserMenuItems > 0

                CheckMenuItem(hMenu, g_funcItem[N_CMDHISTORY]._cmdID,
                  MF_BYCOMMAND | (NppExec.GetOptions().GetBool(OPTB_CONSOLE_CMDHISTORY) ? MF_CHECKED : MF_UNCHECKED));

#ifdef _SCROLL_TO_LATEST        
                CheckMenuItem(hMenu, g_funcItem[N_SCROLL2LATEST]._cmdID,
                  MF_BYCOMMAND | (NppExec.GetOptions().GetBool(OPTB_CONSOLE_SCROLL2LATEST) ? MF_CHECKED : MF_UNCHECKED));
#endif

                CheckMenuItem(hMenu, g_funcItem[N_NOINTMSGS]._cmdID,
                  MF_BYCOMMAND | (NppExec.GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) ? MF_CHECKED : MF_UNCHECKED));
      
                NppExec.UpdateConsoleEncoding();

                NppExec.UpdateOutputFilterMenuItem();
                NppExec.UpdateGoToErrorMenuItem();

                CheckMenuItem(hMenu, g_funcItem[N_SAVEONEXECUTE]._cmdID,
                  MF_BYCOMMAND | (NppExec.GetOptions().GetBool(OPTB_CONSOLE_SAVEONEXECUTE) ? MF_CHECKED : MF_UNCHECKED));

                CheckMenuItem(hMenu, g_funcItem[N_CDCURDIR]._cmdID,
                  MF_BYCOMMAND | (NppExec.GetOptions().GetBool(OPTB_CONSOLE_CDCURDIR) ? MF_CHECKED : MF_UNCHECKED));

                EnableMenuItem(hMenu, g_funcItem[N_NOCMDALIASES]._cmdID,
                  MF_BYCOMMAND | MF_GRAYED );

                /*
                EnableMenuItem(hMenu, g_funcItem[N_CONSOLE_FONT]._cmdID,
                  MF_BYCOMMAND | (NppExec._consoleIsVisible ? MF_ENABLED : MF_GRAYED) );
                */
            } // <--- hMenu
    
            const int n = (g_nUserMenuItems > 0) ? (nbFunc + g_nUserMenuItems + 1) : nbFunc;
            for ( int i = 0; i < n; i++ )
            {
                if ( g_funcItem[i]._pFunc && (g_funcItem[i]._pFunc != empty_func) )
                {
                    NppExec.SendNppMsg( NPPM_GETSHORTCUTBYCMDID, 
                        g_funcItem[i]._cmdID, (LPARAM) &g_funcShortcut[i] );
                }
            }

            if ( (NppExec.GetOptions().GetInt(OPTI_CONSOLE_VISIBLE) == CAdvOptDlg::CON_YES) || 
                 NppExec.GetOptions().GetBool(OPTB_CONSOLE_SHOWHELP) )
            {
                NppExec.showConsoleDialog(CNppExec::showIfHidden, 0);
            }

            if ( NppExec.GetOptions().GetBool(OPTB_CONSOLE_SHOWHELP) )
            {
                NppExec.printConsoleHelpInfo();
            }

            NppExec.RunTheStartScript();

            if ( Runtime::GetLogger().IsLogFileOpen() )
            {
                Runtime::GetLogger().DecIndentLevel();
                Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; NPPN_READY - end") );
            }

        } // NPPN_READY

        else if (notifyCode->nmhdr.code == NPPN_SHUTDOWN)
        {
            CNppExec& NppExec = Runtime::GetNppExec();

            if ( Runtime::GetLogger().IsLogFileOpen() )
            {
                Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; NPPN_SHUTDOWN - start") );
                Runtime::GetLogger().IncIndentLevel();
            }

            CNppExec::_bIsNppShutdown = true; // Notepad++ is shutting down; no more Console output
            NppExec._consoleIsVisible = false; // stopping every script (except the exit script, if any) & child process

            NppExec.RunTheExitScript();

            if ( ::GetProp(NppExec.m_nppData._nppHandle, PROP_NPPEXEC_DLL) )
            {
                ::RemoveProp(NppExec.m_nppData._nppHandle, PROP_NPPEXEC_DLL);
            }

            if ( nppOriginalWndProc )
            {
// get rid of the "conversion from 'LONG_PTR' to 'LONG'" shit
#pragma warning(push)
#pragma warning(disable: 4244)
                if ( g_bIsNppUnicode )
                {
                    ::SetWindowLongPtrW(NppExec.m_nppData._nppHandle, 
                        GWLP_WNDPROC, (LONG_PTR) nppOriginalWndProc);
                }
                else
                {
                    ::SetWindowLongPtrA(NppExec.m_nppData._nppHandle, 
                        GWLP_WNDPROC, (LONG_PTR) nppOriginalWndProc);
                }
#pragma warning(pop)
            }

            // to be sure the options are saved even in case of some 
            // issues at Notepad++'s exit (e.g. crash of some plugin)
            NppExec.SaveOptions();
            NppExec._bOptionsSavedOnNppnShutdown = true;

            if ( Runtime::GetLogger().IsLogFileOpen() )
            {
                Runtime::GetLogger().DecIndentLevel();
                Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; NPPN_SHUTDOWN - end") );
            }

        } // NPPN_SHUTDOWN
    
        else if ( notifyCode->nmhdr.code == NPPN_TBMODIFICATION )
        {
            CNppExec& NppExec = Runtime::GetNppExec();

            const int nToolbarBtn = NppExec.GetOptions().GetInt(OPTI_TOOLBARBTN);
            if ( nToolbarBtn )
            {
                NppExec.m_TB_Icon.hToolbarBmp = (HBITMAP) ::LoadImage( 
                  (HINSTANCE) NppExec.m_hDllModule, MAKEINTRESOURCE(IDI_CONSOLEBITMAP), 
                    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADMAP3DCOLORS );

                int cmdID = g_funcItem[N_SHOWCONSOLE]._cmdID;
                if ( nToolbarBtn == 2 )
                    cmdID = g_funcItem[N_DO_EXEC_DLG]._cmdID;
                else if ( nToolbarBtn == 3 )
                    cmdID = g_funcItem[N_DIRECT_EXEC]._cmdID;
                NppExec.SendNppMsg( NPPM_ADDTOOLBARICON, 
                  (WPARAM) cmdID, (LPARAM) &NppExec.m_TB_Icon );
            }
        } // NPPN_TBMODIFICATION
    
    }

}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM /*wParam*/, LPARAM lParam)
{
  if ( Message == NPPM_MSGTOPLUGIN )
  {
    CommunicationInfo* pci = (CommunicationInfo *) lParam;
    if ( pci )
    {
      Runtime::GetNppExec().GetPluginInterfaceImpl().ProcessExternalPluginMsg( pci->internalMsg, pci->srcModuleName, pci->info );
    }
  }
  return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
  return TRUE;
}
#endif

///////////////////////////////////////////////////////////////////////////
// CNppExec class

bool CNppExec::_bIsNppReady = false;
bool CNppExec::_bIsNppShutdown = false;

CNppExec::CNppExec() 
  : m_Options(_T("NppExec"), optArray, OPT_COUNT)
{
    m_CommandExecutor.SetNppExec(this);
    m_PluginInterfaceImpl.SetNppExec(this);
    //m_Console.SetNppExec(this);
    m_MacroVars.SetNppExec(this);

    m_TB_Icon.hToolbarBmp = NULL;
    m_TB_Icon.hToolbarIcon = NULL;

    npp_nbFiles = 0;
    npp_bufFileNames.Clear();

    _bStopTheExitScript = false;
    _bOptionsSavedOnNppnShutdown = false;

    _consoleFont = NULL;
    _consoleIsVisible = false;
    _consoleCommandBreak = false;
    _consoleCommandIsRunning = false;

    m_bAnotherNppExecDllExists = false;
  
    m_TempScriptIsModified = false;
    m_hDllModule = NULL;
    m_nppData._nppHandle = NULL;
    m_nppData._scintillaMainHandle = NULL;
    m_nppData._scintillaSecondHandle = NULL;
    //m_hRichEditDll = LoadLibrary( _T("Riched32.dll") ); // RichEdit 1.0
    m_hRichEditDll = LoadLibrary( _T("Riched20.dll") ); // RichEdit 2.0
  
    m_idTimerAutoSave = 0;
    m_isSavingOptions = false;

    m_hFocusedWindowBeforeScriptStarted = NULL;
}

CNppExec::~CNppExec()
{
    if ( m_hRichEditDll != NULL )
        ::FreeLibrary(m_hRichEditDll);

    if ( _consoleFont != NULL )
        ::DeleteObject(_consoleFont);
}

CListT<tstr> CNppExec::GetCmdList() const
{
    CCriticalSectionLockGuard lock(m_csScriptCmdList);
    return CListT<tstr>( m_ScriptCmdList );
}

void CNppExec::SetCmdList(const CListT<tstr>& CmdList)
{
    CCriticalSectionLockGuard lock(m_csScriptCmdList);
    m_ScriptCmdList.Copy(CmdList);
}

bool CNppExec::IsCmdListEmpty() const
{
    CCriticalSectionLockGuard lock(m_csScriptCmdList);
    return m_ScriptCmdList.IsEmpty();
}

HWND CNppExec::getCurrentScintilla(INT which)
{
  return ((which == 0) ? m_nppData._scintillaMainHandle : 
             m_nppData._scintillaSecondHandle);
}

/*
// seems to be obsolete
const TCHAR* isReservedFileName(LPCTSTR cszFileName)
{
    int len = c_base::_tstr_safe_len(cszFileName);
    if ( len > 0 )
    {
        int i = c_base::_tstr_unsafe_rfindoneof( cszFileName, len, _T("\\/") );
        if ( i < 0 )
            i = 0;
        else
            ++i;

        if ( i < len )
        {
            const int RESERVED_NAMES_COUNT = 4;
            const TCHAR* cszReservedNames[RESERVED_NAMES_COUNT] = {
                _T("CON"),
                _T("PRN"),
                _T("AUX"),
                _T("NUL")
                //_T("COM1"),
                //_T("COM2"),
                //_T("LPT1"),
                //_T("LPT2"),
            };

            TCHAR szName[MAX_PATH + 1];
            
            lstrcpy(szName, cszFileName + i);
            i = c_base::_tstr_unsafe_findch( szName, _T('.') );
            if ( i > 0 )
                szName[i] = 0;

            NppExecHelpers::StrUpper(szName);
    
            for ( i = 0; i < RESERVED_NAMES_COUNT; i++ )
            {
                if ( lstrcmp(szName, cszReservedNames[i]) == 0 )
                    return cszReservedNames[i];
            }
        }
    }

    return NULL;
}
*/

int CNppExec::conLoadFrom(LPCTSTR cszFile)
{
  CFileBufT<TCHAR> fbuf;
  if ( fbuf.LoadFromFile(cszFile, true, GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH)) )
  {
    tstr Line;

    _consoleCommandIsRunning = true;
    _consoleCommandBreak = false;

    while ( _consoleIsVisible && (!_consoleCommandBreak) && (fbuf.GetLine(Line) >= 0) )
    {
      GetConsole().PrintOutput(Line.c_str(), true);
    }

    _consoleCommandIsRunning = false;
    _consoleCommandBreak = false;
    return fbuf.GetBufCount()*sizeof(TCHAR);
  }
  return -1;
}

typedef struct sConSaveStruct {
    CFileBufT<TCHAR>::FilePtr  f;
    LONG  size;
} ConSaveStruct;

DWORD CALLBACK reConsoleStreamSave(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
  ConSaveStruct* pcss = (ConSaveStruct *) dwCookie;
  if ( (cb > 0) && CFileBufT<TCHAR>::writefile(pcss->f, pbBuff, cb) )
  {
    pcss->size += cb;
    *pcb = cb;
  }
  else
  {
    *pcb = 0;
  }
  return 0;
}

int CNppExec::conSaveTo(LPCTSTR cszFile)
{
  /*
  const TCHAR* pReservedName = isReservedFileName(cszFile);
  if ( pReservedName )
  {
      tstr Err = _T("- file name \"");
      Err += pReservedName;
      Err += _T("\" is reserved by OS");
      GetConsole().PrintError( Err.c_str() );
      return -1;
  }
  */
    
  ConSaveStruct css;
  
  css.size = 0;
  css.f = CFileBufT<TCHAR>::openfile(cszFile, true);
  if ( css.f != NULL )
  {
    EDITSTREAM es;

    #ifdef UNICODE
      CFileBufT<TCHAR>::writefile( css.f, "\xFF\xFE", 2 );
      css.size = 2;
    #endif

    es.dwCookie = (DWORD_PTR) &css;
    es.dwError = 0;
    es.pfnCallback = reConsoleStreamSave;
    #ifdef UNICODE
      GetConsole().GetConsoleEdit().SendMsg( EM_STREAMOUT, SF_TEXT | SF_UNICODE, (LPARAM) &es );
    #else
      GetConsole().GetConsoleEdit().SendMsg( EM_STREAMOUT, SF_TEXT, (LPARAM) &es );
    #endif
    
    CFileBufT<TCHAR>::closefile(css.f, true);
    return css.size;
  }
  return -1;
}

int CNppExec::textLoadFrom(LPCTSTR cszFile, bool bSelectionOnly)
{
  CFileBufT<TCHAR> fbuf;
  if ( fbuf.LoadFromFile(cszFile, true, GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH)) )
  {
    const TCHAR tch = 0; // trailing '\0'

    fbuf.GetBufPtr()->Append( tch );

    #ifdef UNICODE
      textSetText( fbuf.GetBufData() + 1, bSelectionOnly ); // skip leading 0xFEFF
    #else
      textSetText( fbuf.GetBufData(), bSelectionOnly );
    #endif

    return (fbuf.GetBufCount() - 1)*sizeof(TCHAR);
  }
  return -1;
}

int CNppExec::textSaveTo(LPTSTR szFileAndEncoding, bool bSelectionOnly)
{
  enum eEnc {
    encAsIs = 0,
    encANSI,
    encUTF8_BOM,
    encUTF8_NoBOM,
    encUCS2LE
  };

  int enc = encAsIs;

  if ( szFileAndEncoding && (szFileAndEncoding[0] == _T('\"')) )
      ++szFileAndEncoding; // skip first \"

  int i = c_base::_tstr_safe_len( szFileAndEncoding );
  i = c_base::_tstr_safe_rfindch( szFileAndEncoding, i, _T(':') );
  if ( i >= 0 ) 
  {
    if ( (szFileAndEncoding[i+1] != _T('\\')) && (szFileAndEncoding[i+1] != _T('/')) )
    {
      TCHAR* p = c_base::_tstr_unsafe_skip_tabspaces(szFileAndEncoding + i + 1);
      if ( *p )
      {
        if ( *p == _T('a') || *p == _T('A') )
        {
          enc = encANSI;
        }
        else if ( *p == _T('u') || *p == _T('U') )
        {
          if ( *(p + 1) == _T('-') )
            enc = encUTF8_NoBOM;
          else
            enc = encUTF8_BOM;
        }
        else if ( *p == _T('w') || *p == _T('W') )
        {
          enc = encUCS2LE;
        }
      }
      p = c_base::_tstr_unsafe_rskip_tabspaces(szFileAndEncoding, i);
      if ( *p == _T('\"') )
        *p = 0; // skip last \" in file name
      else
        *(++p) = 0;
    }
  }

  if ( szFileAndEncoding[0] == 0 || szFileAndEncoding[0] == _T(':') )
  {
    // error in the arguments
    return -1;
  }

  i = c_base::_tstr_safe_len( szFileAndEncoding );
  if ( i > 0 )
  {
    if ( szFileAndEncoding[i - 1] == _T('\"') )
      szFileAndEncoding[i - 1] = 0; // skip last \"
  }

  /*
  const TCHAR* pReservedName = isReservedFileName(szFileAndEncoding);
  if ( pReservedName )
  {
      tstr Err = _T("- file name \"");
      Err += pReservedName;
      Err += _T("\" is reserved by OS");
      GetConsole().PrintError( Err.c_str() );
      return -1;
  }
  */
    
  HWND hSci = GetScintillaHandle();
  int nSciCodePage = (int) ::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0);
  INT_PTR nSciTextLen;
  if ( bSelectionOnly )
  {
    INT_PTR nSciSelStart = (INT_PTR) ::SendMessage(hSci, SCI_GETSELECTIONSTART, 0, 0);
    INT_PTR nSciSelEnd = (INT_PTR) ::SendMessage(hSci, SCI_GETSELECTIONEND, 0, 0);
    nSciTextLen = nSciSelEnd - nSciSelStart;
  }
  else
  {
    nSciTextLen = (INT_PTR) ::SendMessage(hSci, SCI_GETTEXTLENGTH, 0, 0);
  }

  if ( nSciTextLen > INT_MAX - 2 )
  {
    // text length exceeds 2 GB, whereas the character
    // conversion functions below are limited by 'int'
    return -2; 
  }

  char* pSciText = new char[nSciTextLen + 2];
  if ( pSciText )
  {
    char* pOutText = NULL;
    int   nOutSize = 0;
  
    pSciText[0] = 0;
    {
      UINT uMsg = bSelectionOnly ? SCI_GETSELTEXT : SCI_GETTEXT;
      WPARAM wParam = bSelectionOnly ? 0 : (nSciTextLen + 1);
      ::SendMessage( hSci, uMsg, wParam, (LPARAM) pSciText );
    }
    
    if ( nSciTextLen == 0 )
    {
      // no selected text / no text
      pOutText = pSciText; // empty string in this case
    }
    else if ( nSciCodePage == SC_CP_UTF8 )
    {
      switch ( enc )
      {
        case encANSI:  // ANSI
          pOutText = SysUniConv::newUTF8ToMultiByte(pSciText, -1, CP_ACP, &nOutSize);
          delete [] pSciText;
          break;
        case encUCS2LE:  // Unicode
          pOutText = (char *) SysUniConv::newUTF8ToUnicode(pSciText, -1, &nOutSize);
          nOutSize *= 2;
          delete [] pSciText;
          break; 
        default:
          pOutText = pSciText;
          nOutSize = lstrlenA(pOutText);
          break;
      }
    }
    else
    {
      // multi-byte encoding
      if ( nSciCodePage == 0 )  nSciCodePage = CP_ACP;
      switch ( enc )
      {
        case encUTF8_BOM:   // UTF-8
        case encUTF8_NoBOM:
          pOutText = SysUniConv::newMultiByteToUTF8(pSciText, -1, nSciCodePage, &nOutSize);
          delete [] pSciText;
          break;
        case encUCS2LE:  // Unicode
          pOutText = (char *) SysUniConv::newMultiByteToUnicode(pSciText, -1, nSciCodePage, &nOutSize);
          nOutSize *= 2;
          delete [] pSciText;
          break; 
        default:
          pOutText = pSciText;
          nOutSize = lstrlenA(pOutText);
          break;
      }
    }

    if ( pOutText )
    {
      int nWritten = -1; // -1 means "failed to write"

      CFileBufT<TCHAR>::FilePtr f = CFileBufT<TCHAR>::openfile(szFileAndEncoding, true);
      if ( f != NULL )
      {
        nWritten = 0; // 0 bytes written yet
        if ( nSciTextLen > 0 )
        {
          if ( enc == encUCS2LE )
          {
            // UCS-2 LE "BOM"
            if ( CFileBufT<TCHAR>::writefile(f, (void *) "\xFF\xFE", 2) )
              nWritten += 2;
          }
          else if ( (enc == encUTF8_BOM) || (enc == encAsIs && nSciCodePage == SC_CP_UTF8) )
          {
            // UTF-8 "BOM"
            if ( CFileBufT<TCHAR>::writefile(f, (void *) "\xEF\xBB\xBF", 3) )
              nWritten += 3;
          }
        }
        if ( CFileBufT<TCHAR>::writefile(f, pOutText, nOutSize) )
          nWritten += nOutSize;
        CFileBufT<TCHAR>::closefile(f, true);
      }
        
      delete [] pOutText;
      return nWritten;
    }
  }

  return -1;
}

void CNppExec::textSetText(LPCTSTR cszText, bool bSelectionOnly)
{
  HWND  hSci = GetScintillaHandle();
  char* pBufData = SciTextFromLPCTSTR(cszText, hSci);

  unsigned int uMsg = bSelectionOnly ? SCI_REPLACESEL : SCI_SETTEXT;
  ::SendMessage( hSci, uMsg, 0, (LPARAM) (pBufData ? pBufData : "") );

  SciTextDeleteResultPtr(pBufData, cszText);
}

int CNppExec::findFileNameIndexInNppOpenFileNames(const tstr& fileName, bool bGetOpenFileNames, int nView )
{
  if (bGetOpenFileNames)
  {
    if (nView == PRIMARY_VIEW || nView == SECOND_VIEW)
    {
      npp_nbFiles = nppGetOpenFileNamesInView(nView);
    }
    else
      nppGetOpenFileNames();
  }

  tstr S;
  tstr S1 = NppExecHelpers::NormalizePath(fileName);

  int nFileLevel = 0;
  bool bFullPath = NppExecHelpers::IsFullPath(S1);
  if (!bFullPath)
  {
    for (int j = 0; j < S1.length(); ++j)
    {
      if (S1[j] == _T('\\'))
        ++nFileLevel;
    }
  }

  NppExecHelpers::StrUpper(S1);

  int iPartialMatch1 = -1;
  int iFind1 = -1;
  for (int i = 0; i < npp_nbFiles; ++i)
  { 
    S = npp_bufFileNames[i];
    S.Replace(_T('/'), _T('\\'));

    if (!bFullPath)
    {
      int n = 0;
      int j = S.length() - 1;
      while (j >= 0)
      {
        if (S[j] == _T('\\'))
        {
          if (n == nFileLevel)
          {
            S.Delete(0, j + 1);
            break;
          }
          else
            ++n;
        }
        --j;
      }
    }

    NppExecHelpers::StrUpper(S);

    if (S == S1)
      return i; // complete match

    if (!bFullPath)
    {
      if (iPartialMatch1 == -1 || iFind1 != 0)
      { // no partial match yet or not beginning of file name yet
        int ifind = S.Find(S1);
        if (ifind >= 0) // position inside file name
        {
          if (iFind1 == -1 || ifind < iFind1)
          { // no partial match yet or closer to beginning of file name
            iFind1 = ifind;
            iPartialMatch1 = i;
          }
        }
      }
    }
  }

  return iPartialMatch1; // first partial match or -1
}

#define	IDD_FILEBROWSER            3500
#define	IDD_FILEBROWSER_MENU       (IDD_FILEBROWSER + 10)
#define IDM_FILEBROWSER_COPYPATH   (IDD_FILEBROWSER_MENU + 6)

struct tWorkspaceDlg
{
    HWND hDlg;
    HWND hTreeView;

    tWorkspaceDlg() : hDlg(NULL), hTreeView(NULL)
    {
    }
};

static BOOL CALLBACK GetWorkspaceTreeViewEnumFunc(HWND hWnd, LPARAM lParam)
{
    if ( ::IsWindowVisible(hWnd) )
    {
        TCHAR szText[255];

        ::GetWindowText(hWnd, szText, 254);
        if ( ::lstrcmp(szText, _T("File Browser")) == 0 )
        {
            HWND hTreeView = ::FindWindowEx(hWnd, NULL, WC_TREEVIEW, NULL);
            if ( hTreeView )
            {
                tWorkspaceDlg* pWrkspcDlg = (tWorkspaceDlg *) lParam;
                pWrkspcDlg->hDlg = hWnd;
                pWrkspcDlg->hTreeView = hTreeView;

                return FALSE; // stop enumeration
            }
        }
    }

    return TRUE; // continue enumeration
}

/*
bool CNppExec::nppGetWorkspaceRootFolders(CListT<tstr>& listOfRootFolders)
{
    bool bRet = false;
    tWorkspaceDlg wrkspcDlg;

    listOfRootFolders.Clear();

    EnumChildWindows(m_nppData._nppHandle, GetWorkspaceTreeViewEnumFunc, (LPARAM) &wrkspcDlg);

    if ( wrkspcDlg.hTreeView )
    {
        TVITEM tvItem;
        TCHAR szText[FILEPATH_BUFSIZE];

        HTREEITEM hItem = (HTREEITEM) ::SendMessage(wrkspcDlg.hTreeView, TVM_GETNEXTITEM, TVGN_ROOT, 0);
        while ( hItem )
        {
            ::ZeroMemory(&tvItem, sizeof(TVITEM));
            tvItem.mask = TVIF_HANDLE | TVIF_TEXT;
            tvItem.hItem = hItem;
            tvItem.pszText = szText;
            tvItem.cchTextMax = FILEPATH_BUFSIZE - 1;

            if ( ::SendMessage(wrkspcDlg.hTreeView, TVM_GETITEM, 0, (LPARAM) &tvItem) )
            {
                listOfRootFolders.Add( tstr(szText) );
            }

            hItem = (HTREEITEM) ::SendMessage(wrkspcDlg.hTreeView, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM) hItem);
        }

        bRet = true;
    }

    return bRet;
}
*/

bool CNppExec::nppGetWorkspaceItemPath(tstr& itemPath)
{
    bool bRet = false;
    tWorkspaceDlg wrkspcDlg;

    EnumChildWindows(m_nppData._nppHandle, GetWorkspaceTreeViewEnumFunc, (LPARAM) &wrkspcDlg);

    if ( wrkspcDlg.hDlg )
    {
        const tstr sClipboardText0 = NppExecHelpers::GetClipboardText(Runtime::GetNppExec().m_nppData._nppHandle);
        ::SendMessage(wrkspcDlg.hDlg, WM_COMMAND, IDM_FILEBROWSER_COPYPATH, 0);
        itemPath = NppExecHelpers::GetClipboardText(Runtime::GetNppExec().m_nppData._nppHandle);
        NppExecHelpers::SetClipboardText(sClipboardText0, Runtime::GetNppExec().m_nppData._nppHandle);

        bRet = true;
    }

    return bRet;
}

bool CNppExec::nppGetWorkspaceRootItemPath(tstr& rootItemPath)
{
    bool bRet = false;
    tWorkspaceDlg wrkspcDlg;

    EnumChildWindows(m_nppData._nppHandle, GetWorkspaceTreeViewEnumFunc, (LPARAM) &wrkspcDlg);

    if ( wrkspcDlg.hDlg && wrkspcDlg.hTreeView )
    {
        HTREEITEM hCurrItem = (HTREEITEM) ::SendMessage(wrkspcDlg.hTreeView, TVM_GETNEXTITEM, TVGN_CARET, 0);
        if ( hCurrItem )
        {
            HTREEITEM hRootItem = hCurrItem;
            HTREEITEM hItem = hCurrItem;
            while ( hItem )
            {
                hRootItem = hItem;
                hItem = (HTREEITEM) ::SendMessage(wrkspcDlg.hTreeView, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM) hItem);
            }

            if ( hRootItem != hCurrItem )
            {
                ::SendMessage(wrkspcDlg.hTreeView, WM_SETREDRAW, FALSE, 0);
                ::SendMessage(wrkspcDlg.hTreeView, TVM_SELECTITEM, TVGN_CARET, (LPARAM) hRootItem);
            }

            const tstr sClipboardText0 = NppExecHelpers::GetClipboardText(Runtime::GetNppExec().m_nppData._nppHandle);
            ::SendMessage(wrkspcDlg.hDlg, WM_COMMAND, IDM_FILEBROWSER_COPYPATH, 0);
            rootItemPath = NppExecHelpers::GetClipboardText(Runtime::GetNppExec().m_nppData._nppHandle);
            NppExecHelpers::SetClipboardText(sClipboardText0, Runtime::GetNppExec().m_nppData._nppHandle);

            if ( hRootItem != hCurrItem )
            {
                ::SendMessage(wrkspcDlg.hTreeView, TVM_SELECTITEM, TVGN_CARET, (LPARAM) hCurrItem);
                ::SendMessage(wrkspcDlg.hTreeView, WM_SETREDRAW, TRUE, 0);
            }

            bRet = true;
        }
    }

    return bRet;
}

int CNppExec::nppConvertToFullPathName(tstr& fileName, bool bGetOpenFileNames, int nView )
{
  
  Runtime::GetLogger().Add(   _T("nppConvertToFullPathName()") );
  Runtime::GetLogger().Add(   _T("{") );
  Runtime::GetLogger().IncIndentLevel();
      
  if (!fileName.IsEmpty())
  {
    
    Runtime::GetLogger().AddEx( _T("[in]  \"%s\""), fileName.c_str() );  
      
    if (!NppExecHelpers::IsFullPath(fileName))
    {
      int i = findFileNameIndexInNppOpenFileNames(fileName, bGetOpenFileNames, nView);
      if (i != -1)
      {
        fileName = npp_bufFileNames[i];

        Runtime::GetLogger().AddEx( _T("; changed to: \"%s\""), fileName.c_str() ); 
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

        // full path found
        return fileName.length();
      }
    }
    else
    {

      Runtime::GetLogger().Add(   _T("; already full path") );
      Runtime::GetLogger().DecIndentLevel();
      Runtime::GetLogger().Add(   _T("}") );

      // already full path
      return fileName.length();
    }

  }
  
  Runtime::GetLogger().DecIndentLevel();
  Runtime::GetLogger().Add(   _T("}") );
    
  return 0;
}

tstr CNppExec::nppGetSettingsCloudPath()
{
    tstr cloudPath;
    int nLen = (int) SendNppMsg( NPPM_GETSETTINGSONCLOUDPATH, 0, 0 );
    if (nLen != 0)
    {
        cloudPath.Reserve(nLen + 1);
        nLen = (int) SendNppMsg( NPPM_GETSETTINGSONCLOUDPATH, nLen + 1, (LPARAM) cloudPath.c_str() );
        cloudPath.SetLengthValue(nLen);
    }
    return cloudPath;
}

tstr CNppExec::GetSettingsCloudPluginDir()
{
    tstr cloudDir = nppGetSettingsCloudPath();
    if (!cloudDir.IsEmpty())
    {
        if (!cloudDir.EndsWith(_T('\\')))
        {
            cloudDir += _T('\\');
        }
        cloudDir += _T("NppExec\\");
    }
    return cloudDir;
}

static tstr getMenuItemPathSep(const tstr& menuItemPathName)
{
    const int nSeps = 9;
    const TCHAR* szSeps[nSeps] = {
        _T("\\\\\\"),
        _T("///"),
        _T("|||"),
        _T("\\\\"),
        _T("//"),
        _T("||"),
        _T("\\"),
        _T("/"),
        _T("|")
    };
    for ( int i = 0; i < nSeps; i++ )
    {
        if ( menuItemPathName.Find(szSeps[i]) >= 0 )
            return tstr(szSeps[i]);
    }
    return tstr(_T(""));
}

int CNppExec::nppGetMenuItemIdByName(const tstr& menuItemPathName, tstr& parsedPath, tstr& parsedSep)
{
    int menuItemId = -1;
    parsedPath.Clear();
    parsedSep.Clear();

    HMENU hMenu = GetNppMainMenu();
    if ( hMenu )
    {
        tstr sep = getMenuItemPathSep(menuItemPathName);
        parsedSep = sep;

        CStrSplitT<TCHAR> args;
        int level = args.Split(menuItemPathName, sep.c_str());    
        if ( level > 0 )
        {
            typedef std::map<tstr, UINT_PTR> tMenuItems;
            typedef std::list<tMenuItems> tListOfMenuItems;

            static tListOfMenuItems listOfMenuItems;

            tListOfMenuItems::iterator listItr = listOfMenuItems.begin();
            for ( int n = 0; (n < level) && (menuItemId < 0); ++n, ++listItr )
            {
                UINT_PTR uItem = 0;
                const tstr& itemName = args.GetArg(n);
                if ( itemName.IsEmpty() )
                    break;

                if ( listItr != listOfMenuItems.end() )
                {
                    tMenuItems::const_iterator itemItr = listItr->find( itemName );
                    if ( itemItr != listItr->end() )
                    {
                        uItem = itemItr->second;
                        if ( ::IsMenu((HMENU)uItem) )
                            hMenu = (HMENU) uItem;
                        else
                            menuItemId = (int) uItem;
                    }
                }
                if ( uItem == 0 )
                {
                    TCHAR        szItemText[128];
                    MENUITEMINFO mii;
                    tstr         itmText;

                    const int nItems = ::GetMenuItemCount(hMenu);
                    for ( int i = 0; (i < nItems) && (uItem == 0); i++ )
                    {
                        szItemText[0] = 0;
                        ::ZeroMemory( &mii, sizeof(MENUITEMINFO) );
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
                        mii.dwTypeData = (TCHAR*) szItemText;
                        mii.cch = 128 - 1;

                        if ( ::GetMenuItemInfo(hMenu, i, TRUE, &mii) )
                        {
                            itmText = szItemText;
                            if ( itemName.Find(_T('&')) < 0 )
                            {
                                itmText.Replace(_T("&"), _T(""));
                            }
                            if ( itmText.length() > itemName.length() )
                            {
                                itmText.SetSize(itemName.length());
                            }
                            if ( itemName == itmText )
                            {
                                if ( listItr == listOfMenuItems.end() )
                                {
                                    tMenuItems items;
                                    listOfMenuItems.push_back(items);
                                    listItr = listOfMenuItems.end();
                                    --listItr;
                                }
                                if ( mii.hSubMenu )
                                {
                                    uItem = (UINT_PTR) mii.hSubMenu;
                                    hMenu = mii.hSubMenu;
                                }
                                else
                                {
                                    uItem = (UINT_PTR) mii.wID;
                                    menuItemId = mii.wID;
                                }
                                listItr->insert( std::make_pair(itemName, uItem) );
                            }
                        }
                    }
                }
                if ( uItem != 0 )
                {
                    if ( !parsedPath.IsEmpty() )
                        parsedPath += sep;
                    parsedPath += itemName;
                }
                else
                    break;
            }
        }
    }

    return menuItemId;
}

int CNppExec::nppGetOpenFileNames()
{
  TCHAR* p;
  
  npp_nbFiles = (int) SendNppMsg(NPPM_GETNBOPENFILES, 0, 0);
  
  npp_bufFileNames.SetSize(npp_nbFiles);  // reserving memory without 
                                          // modification of its content
  for (int i = npp_bufFileNames.GetCount(); i < npp_nbFiles; i++)
  {
    p = new TCHAR[FILEPATH_BUFSIZE];
    if (p != NULL)
      npp_bufFileNames.Append(p);
  }

  if (npp_nbFiles > npp_bufFileNames.GetCount())
    npp_nbFiles = npp_bufFileNames.GetCount();

  SendNppMsg(NPPM_GETOPENFILENAMES, 
      (WPARAM) npp_bufFileNames.GetData(), (LPARAM) npp_nbFiles);

  return npp_nbFiles;
}

int CNppExec::nppGetOpenFileNamesInView(int nView , int nFiles )
{
  if ( (nView != PRIMARY_VIEW) && 
       (nView != SECOND_VIEW) &&
       (nView != ALL_OPEN_FILES) )
  {
    return -1;
  }
  
  if (nFiles < 0)
  {
    nFiles = (int) SendNppMsg(NPPM_GETNBOPENFILES, (WPARAM) nView, (LPARAM) nView);
  }

  npp_bufFileNames.SetSize(nFiles);  // reserving memory without 
                                     // modification of its content
  for (int i = npp_bufFileNames.GetCount(); i < nFiles; i++)
  {
    TCHAR* p = new TCHAR[FILEPATH_BUFSIZE];
    if (p != NULL)
      npp_bufFileNames.Append(p);
  }

  if (nFiles > npp_bufFileNames.GetCount())
    nFiles = npp_bufFileNames.GetCount();

  UINT uMsg = NPPM_GETOPENFILENAMES;
  if (nView == PRIMARY_VIEW)
    uMsg = NPPM_GETOPENFILENAMESPRIMARY;
  else if (nView == SECOND_VIEW)
    uMsg = NPPM_GETOPENFILENAMESSECOND;
  SendNppMsg(uMsg, (WPARAM) npp_bufFileNames.GetData(), (LPARAM) nFiles);

  return nFiles;
}

bool CNppExec::nppSwitchToDocument(const tstr& fileName, bool bGetOpenFileNames, int nView )
{
  
  Runtime::GetLogger().Add(   _T("nppSwitchToDocument()") );
  Runtime::GetLogger().Add(   _T("{") );
  Runtime::GetLogger().IncIndentLevel();
  Runtime::GetLogger().AddEx( _T("[in]  \"%s\""), fileName.c_str() );
    
  if (!NppExecHelpers::IsFullPath(fileName))
  {
    int i = findFileNameIndexInNppOpenFileNames(fileName, bGetOpenFileNames, nView);
    if (i != -1)
    {
        
      Runtime::GetLogger().AddEx( _T("; changed to: \"%s\""), npp_bufFileNames[i] );  
          
      SendNppMsg(NPPM_SWITCHTOFILE, (WPARAM) 0, (LPARAM) npp_bufFileNames[i]);
    }
    else
    {
      Runtime::GetLogger().Add(   _T("; no similar file name") ); 
    }
    Runtime::GetLogger().DecIndentLevel();
    Runtime::GetLogger().Add(   _T("}") );

    return ( (i != -1) ? true : false );
  }

  Runtime::GetLogger().Add(   _T("; full path specified") );
  Runtime::GetLogger().DecIndentLevel();
  Runtime::GetLogger().Add(   _T("}") );
    
  return ( (SendNppMsg(NPPM_SWITCHTOFILE, 
                (WPARAM) 0, (LPARAM) fileName.c_str()) != 0) ? true : false );
}

bool CNppExec::nppSaveAllFiles()
{
    // TODO: optimize - maybe there are no dirty files i.e. no need to save

    /*
    int ind = (int) SendNppMsg( NPPM_GETCURRENTDOCINDEX
                                , MAIN_VIEW
                                , MAIN_VIEW
                                );
    */

    return ( SendNppMsg(NPPM_SAVEALLFILES, 0, 0) != 0 );

    /*
    SendNppMsg( NPPM_ACTIVATEDOC
                , MAIN_VIEW
                , ind
                );
    */
}

/////////////////////////////////////////////////////////////////////////////

//CChildProcess::CChildProcess()
//{
//    m_pNppExec = NULL;
//    m_pScriptEngine = NULL;
//    
//    reset();
//}

CChildProcess::CChildProcess(CScriptEngine* pScriptEngine)
{
    m_strInstance = NppExecHelpers::GetInstanceAsString(this);

    m_pNppExec = pScriptEngine->GetNppExec();
    m_pScriptEngine = pScriptEngine;

    reset();

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess - create (instance = %s)"), GetInstanceStr() );
}

CChildProcess::~CChildProcess()
{
    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess - destroy (instance = %s)"), GetInstanceStr() );
}

const TCHAR* CChildProcess::GetInstanceStr() const
{
    return m_strInstance.c_str();
}

void CChildProcess::applyCommandLinePolicy(tstr& sCmdLine, eCommandLinePolicy mode)
{
    if ( mode == clpNone )
        return; // nothing to do

    CStrSplitT<TCHAR> args;
    if ( args.SplitToArgs(sCmdLine, 2) == 0 )
        return; // empty sCmdLine, nothing to do

    const tstr sFileName = args.Arg(0);

    if ( mode == clpComSpec )
    {
        tstr sComSpec = NppExecHelpers::GetEnvironmentVariable( _T("COMSPEC") );
        if ( sComSpec.IsEmpty() )
            sComSpec = _T("cmd");

        const tstr sComSpecNamePart = NppExecHelpers::GetFileNamePart(sComSpec, NppExecHelpers::fnpName);
        const tstr sFileNamePart = NppExecHelpers::GetFileNamePart(sFileName, NppExecHelpers::fnpName);
        if ( NppExecHelpers::StrCmpNoCase(sComSpecNamePart, sFileNamePart) == 0 )
            return; // sCmdLine already starts with "cmd"

        if ( sComSpec.Find(_T(' ')) >= 0 )
            NppExecHelpers::StrQuote(sComSpec);
        
        sComSpec += _T(' '); // e.g. "cmd " (notice the trailing space!)
        
        tstr sComSpecSwitches = Runtime::GetNppExec().GetOptions().GetStr(OPTS_CHILDP_COMSPECSWITCHES);
        if ( !sComSpecSwitches.IsEmpty() )
        {
            sComSpec += sComSpecSwitches;
            sComSpec += _T(' '); // e.g. "cmd /C " (notice the trailing space!)
        }

        sCmdLine.Insert(0, sComSpec);
        return;
    }

    // mode == clpPathExt
    tstr sPathExtensions = NppExecHelpers::GetEnvironmentVariable( _T("PATHEXT") );
    if ( !sPathExtensions.IsEmpty() )
        NppExecHelpers::StrLower(sPathExtensions); // in lower case!
    else
        sPathExtensions = _T(".com;.exe;.bat;.cmd"); // in lower case!

    CListT<tstr> pathExts;
    if ( StrSplitAsArgs(sPathExtensions.c_str(), pathExts, _T(';')) == 0 )
        return; // no extensions to check, nothing to do

    const tstr existingExt = NppExecHelpers::GetFileNamePart(sFileName, NppExecHelpers::fnpExt);
    if ( !existingExt.IsEmpty() )
    {
        const CListItemT<tstr>* pExt = pathExts.Find( [&existingExt](const tstr& ext) { return (NppExecHelpers::StrCmpNoCase(ext, existingExt) == 0); } );
        if ( pExt )
            return; // the extension from %PATHEXT% is specified explicitly

        // As the existingExt is not empty, we may return here.
        // However, a file name itself may have a form of "file.name", so
        // we still need to inspect all the extensions from %PATHEXT% to
        // check for all possible "file.name.ext" in paths from %PATH%.
    }

    auto findMatchingExtension = [](const tstr& fileName, const CListT<tstr>& exts, const auto& Predicate)
                            {
                                for ( const CListItemT<tstr>* pExt = exts.GetFirst(); pExt != NULL; pExt = pExt->GetNext() )
                                {
                                    const tstr& ext = pExt->GetItem();
                                    if ( Predicate(fileName, ext) )
                                        return ext;
                                }
                                return tstr();
                            };

    auto fexistsWithExt = [](const tstr& fileName, const tstr& ext)
                            {
                                tstr fileNameExt = fileName;
                                fileNameExt += ext;
                                return NppExecHelpers::CheckFileExists(fileNameExt);
                            };

    tstr ext;
    if ( NppExecHelpers::IsFullPath(sFileName) )
    {
        // full path specified...
        if ( NppExecHelpers::CheckFileExists(sFileName) )
            return; // cmd does not try %PATHEXT% for an _existing_ file name

        // check the file extensions in %PATHEXT%...
        ext = findMatchingExtension(sFileName, pathExts, fexistsWithExt);
    }
    else
    {
        // no path specified - check the paths in %PATH% and file extensions in %PATHEXT%...
        tstr sPaths = NppExecHelpers::GetEnvironmentVariable( _T("PATH") );
        CListT<tstr> paths;
        if ( !sPaths.IsEmpty() )
        {
            StrSplitAsArgs(sPaths.c_str(), paths, _T(';'));
        }

        const tstr sCurDir = NppExecHelpers::GetCurrentDirectory();
        if ( !sCurDir.IsEmpty() )
        {
            CListItemT<tstr>* pItem = paths.Find( [&sCurDir](const tstr& path) { return (NppExecHelpers::StrCmpNoCase(path, sCurDir) == 0); } );
            if ( pItem )
            {
                paths.Delete(pItem);
            }
            paths.InsertFirst(sCurDir);
        }

        for ( const CListItemT<tstr>* pPath = paths.GetFirst(); pPath != NULL; pPath = pPath->GetNext() )
        {
            tstr sPathName = pPath->GetItem();
            if ( !sPathName.EndsWith(_T('\\')) && !sPathName.EndsWith(_T('/')) )
                sPathName += _T('\\');
            sPathName += sFileName;
            if ( NppExecHelpers::CheckFileExists(sPathName) )
                return; // cmd does not try %PATHEXT% for an _existing_ file name

            // check the file extensions in %PATHEXT%...
            ext = findMatchingExtension(sPathName, pathExts, fexistsWithExt);
            if ( !ext.IsEmpty() )
                break;
        }
    }

    if ( ext.IsEmpty() )
        return; // no existing full path found - nothing to do

    tstr sFileNameExt = sFileName;
    sFileNameExt += ext;

    int nPos = sCmdLine.Find(sFileName);
    sCmdLine.Replace(nPos, sFileName.length(), sFileNameExt);
}

// cszCommandLine must be transformed by ModifyCommandLine(...) already
bool CChildProcess::Create(HWND /*hParentWnd*/, LPCTSTR cszCommandLine)
{
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO         si;

    reset();

    if ( IsWindowsNT() )
    {
        // security stuff for NT
        InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
        sa.lpSecurityDescriptor = &sd;
    }
    else
    {
        sa.lpSecurityDescriptor = NULL;
    }
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;

    const int DEFAULT_PIPE_SIZE = 0;

    // Create the Pipe and get r/w handles
    if ( !::CreatePipe(&m_hStdOutReadPipe, &m_hStdOutWritePipe, &sa, DEFAULT_PIPE_SIZE) )
    {
        m_pNppExec->GetConsole().PrintError( _T("CreatePipe(<StdOut>) failed") );
        return false;
    }
    if ( m_hStdOutWritePipe == NULL )
    {
        if ( m_hStdOutReadPipe != NULL )
            ::CloseHandle(m_hStdOutReadPipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdOutWritePipe = NULL") );
        return false;
    }
    if ( m_hStdOutReadPipe == NULL )
    {
        ::CloseHandle(m_hStdOutWritePipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdOutReadPipe = NULL") );
        return false;
    }

    if ( !::CreatePipe(&m_hStdInReadPipe, &m_hStdInWritePipe, &sa, DEFAULT_PIPE_SIZE) )
    {
        m_pNppExec->GetConsole().PrintError( _T("CreatePipe(<StdIn>) failed") );
        return false;
    }
    if ( m_hStdInWritePipe == NULL )
    {
        if ( m_hStdInReadPipe != NULL )
            ::CloseHandle(m_hStdInReadPipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdInWritePipe = NULL") );
        return false;
    }
    if ( m_hStdInReadPipe == NULL )
    {
        ::CloseHandle(m_hStdInWritePipe);
        m_pNppExec->GetConsole().PrintError( _T("hStdInReadPipe = NULL") );
        return false;
    }

    ::SetHandleInformation(m_hStdInWritePipe, HANDLE_FLAG_INHERIT, 0);
    ::SetHandleInformation(m_hStdOutReadPipe, HANDLE_FLAG_INHERIT, 0);

    /*
    DWORD dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    SetNamedPipeHandleState(m_hStdOutWritePipe, &dwMode, NULL, NULL);
    dwMode = PIPE_READMODE_BYTE | PIPE_NOWAIT;
    SetNamedPipeHandleState(m_hStdOutReadPipe, &dwMode, NULL, NULL);
    */
    
    // initialize STARTUPINFO struct
    ::ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = m_hStdInReadPipe;
    si.hStdOutput = m_hStdOutWritePipe;
    si.hStdError = m_hStdOutWritePipe;

    eCommandLinePolicy mode = clpNone;
    switch ( m_pNppExec->GetOptions().GetInt(OPTU_CHILDP_RUNPOLICY) )
    {
        case clpPathExt:
            mode = clpPathExt;
            break;
        case clpComSpec:
            mode = clpComSpec;
            break;
    };
    tstr sCmdLine = cszCommandLine;
    applyCommandLinePolicy(sCmdLine, mode);

    if ( ::CreateProcess(
            NULL,
            sCmdLine.c_str(),
            NULL,                        // security
            NULL,                        // security
            TRUE,                        // inherits handles
            CREATE_NEW_PROCESS_GROUP,    // creation flags
            NULL,                        // environment
            NULL,                        // current directory
            &si,                         // startup info
            &m_ProcessInfo               // process info
       ) )
    {
        ::CloseHandle(m_ProcessInfo.hThread); m_ProcessInfo.hThread = NULL;
        //::CloseHandle(hStdOutWritePipe); hStdOutWritePipe = NULL;
        //::CloseHandle(hStdInReadPipe); hStdInReadPipe = NULL;

        bool isConsoleProcessRunning = true;
    
        m_pNppExec->GetConsole().PrintMessage( tstr().Format(80, _T("Process started (PID=%u) >>>"), m_ProcessInfo.dwProcessId) );

        {
            TCHAR szProcessId[50];
            c_base::_tint2str(GetProcessId(), szProcessId);
            tstr varName = MACRO_PID;
            m_pNppExec->GetMacroVars().SetUserMacroVar( m_pScriptEngine, varName, szProcessId, CNppExecMacroVars::svLocalVar ); // local var
        }
    
        // this pause is necessary for child processes that return immediatelly
        ::WaitForSingleObject(m_ProcessInfo.hProcess, m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_STARTUPTIMEOUT_MS));
        
        CStrT<char>  bufLine;
        bool         bPrevLineEmpty = false;
        bool         bDoOutputNext = true;
        int          nPrevState = 0;
        DWORD        dwRead = 0;
        unsigned int nEmptyCount = 0;
        const DWORD  dwCycleTimeOut = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_CYCLETIMEOUT_MS);
        const DWORD  dwExitTimeOut = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_EXITTIMEOUT_MS);
    
        bufLine.Clear(); // just in case :-)
    
        do 
        {
            // inside this cycle: the bOutputAll parameter must be controlled within readPipesAndOutput
            dwRead = readPipesAndOutput(bufLine, bPrevLineEmpty, nPrevState, false, bDoOutputNext);

            if ( CNppExec::_bIsNppShutdown )
            {
                // Notepad++ is exiting
                if ( dwRead == 0 )
                {
                    // no output from the child process
                    ++nEmptyCount;
                    if ( nEmptyCount > (dwExitTimeOut/dwCycleTimeOut) )
                    {
                        // no output during more than dwExitTimeOut ms, let's kill the process...
                        m_nBreakMethod = CProcessKiller::killCtrlBreak;
                    }
                }
                else
                    nEmptyCount = 0;
            }

        }
        while ( (isConsoleProcessRunning = (::WaitForSingleObject(m_ProcessInfo.hProcess, dwCycleTimeOut) == WAIT_TIMEOUT))
             && m_pScriptEngine->ContinueExecution() && !isBreaking() );
        // NOTE: time-out inside WaitForSingleObject() prevents from 100% CPU usage!

        if ( m_pScriptEngine->ContinueExecution() && (!isBreaking()) && !m_pScriptEngine->GetTriedExitCmd() )
        {
            // maybe the child process is exited but not all its data is read
            readPipesAndOutput(bufLine, bPrevLineEmpty, nPrevState, true, bDoOutputNext);
        }

        if ( (!m_pScriptEngine->ContinueExecution()) || isBreaking() )
        {
            if ( isConsoleProcessRunning )
            {
                int nKillMethods = 0;
                CProcessKiller::eKillMethod arrKillMethods[4];

                tstr sAppName;
                CListT<tstr> ArgsList; 
                if (StrSplitToArgs(cszCommandLine, ArgsList, 2) > 0)
                {
                  sAppName = ArgsList.GetFirst()->GetItem();
                  NppExecHelpers::StrLower(sAppName);
                }

                if ( (sAppName == _T("cmd")) || (sAppName == _T("cmd.exe")) )
                {
                    // cmd can't be closed with Ctrl-Break for unknown reason...
                }
                else
                {
                    if ( m_nBreakMethod == CProcessKiller::killCtrlC )
                        arrKillMethods[nKillMethods++] = CProcessKiller::killCtrlC;
                    else
                        arrKillMethods[nKillMethods++] = CProcessKiller::killCtrlBreak;
                }
                arrKillMethods[nKillMethods++] = CProcessKiller::killWmClose;

                Runtime::GetLogger().AddEx( _T("; trying to kill the child process... (instance = %s)"), GetInstanceStr() );

                unsigned int nWaitTimeout = m_pNppExec->GetOptions().GetUint(OPTU_CHILDP_KILLTIMEOUT_MS);
                CProcessKiller::eKillMethod nSucceededKillMethod = CProcessKiller::killNone;
                if ( Kill(arrKillMethods, nKillMethods, nWaitTimeout, &nSucceededKillMethod) )
                {
                    Runtime::GetLogger().AddEx( _T("; the child process has been killed (instance = %s)"), GetInstanceStr() );

                    if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
                    {
                        tstr Msg;
                        Msg.Format( 80, _T("<<< Process has been killed (PID=%d)"), m_ProcessInfo.dwProcessId );
                        switch ( nSucceededKillMethod )
                        {
                            case CProcessKiller::killCtrlBreak:
                                Msg += _T(" with Ctrl-Break");
                                break;
                            case CProcessKiller::killCtrlC:
                                Msg += _T(" with Ctrl-C");
                                break;
                            case CProcessKiller::killWmClose:
                                Msg += _T(" with WM_CLOSE");
                                break;
                        }
                        Msg += _T('.');
                        m_pNppExec->GetConsole().PrintMessage( Msg.c_str() );
                    }
                    else
                    {
                        if (nPrevState != 1 /* new line */ )  m_pNppExec->GetConsole().PrintMessage( _T(""), false );
                    }
                }
                else
                {
                    Runtime::GetLogger().AddEx( _T("; trying to terminate the child process... (instance = %s)"), GetInstanceStr() );

                    if ( ::TerminateProcess(m_ProcessInfo.hProcess, 0) )
                    {
                        Runtime::GetLogger().AddEx( _T("; the child process has been terminated (instance = %s)"), GetInstanceStr() );

                        if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
                        {
                            m_pNppExec->GetConsole().PrintMessage( tstr().Format(80, _T("<<< Process has been terminated (PID=%d)."), m_ProcessInfo.dwProcessId) );
                        }
                        else
                        {
                            if (nPrevState != 1 /* new line */ )  m_pNppExec->GetConsole().PrintMessage( _T(""), false );
                        }
                    }
                    else
                    {
                        m_pNppExec->GetConsole().PrintError( tstr().Format(80, _T("<<< TerminateProcess() returned FALSE (PID=%d)."), m_ProcessInfo.dwProcessId) );
                    }
                }

            }
        }

        DWORD dwExitCode = (DWORD)(-1);
        ::GetExitCodeProcess(m_ProcessInfo.hProcess, &dwExitCode);
        m_nExitCode = (int) dwExitCode;

        // Process cleanup
        ::CloseHandle(m_ProcessInfo.hProcess); m_ProcessInfo.hProcess = NULL;
        closePipes();

        if ( m_pScriptEngine->ContinueExecution() && !isBreaking() )
        {
            if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
            {
                m_pNppExec->GetConsole().PrintMessage( tstr().Format(100, _T("<<< Process finished (PID=%u). (Exit code %d)"), m_ProcessInfo.dwProcessId, m_nExitCode) ); 
            }
            else
            {
                if (nPrevState != 1 /* new line */ )  m_pNppExec->GetConsole().PrintMessage( _T(""), false );
            }
        }

        return true;
    }
    else
    {
        DWORD dwErrorCode = ::GetLastError();

        closePipes();

        if ( m_pScriptEngine )
        {
            m_pNppExec->GetConsole().PrintError( m_pScriptEngine->GetLastLoggedCmd().c_str() );
        }
        m_pNppExec->GetConsole().PrintSysError( _T("CreateProcess()"), dwErrorCode );

        return false;
    }
}

void CChildProcess::reset()
{
    m_strOutput.Clear();
    m_nExitCode = -1;
    m_nBreakMethod = CProcessKiller::killNone;
    m_hStdInReadPipe = NULL;
    m_hStdInWritePipe = NULL; 
    m_hStdOutReadPipe = NULL;
    m_hStdOutWritePipe = NULL;
    ::ZeroMemory(&m_ProcessInfo, sizeof(PROCESS_INFORMATION));
}

bool CChildProcess::isBreaking() const
{
    return (m_nBreakMethod != CProcessKiller::killNone);
}

bool CChildProcess::applyOutputFilters(const tstr& _line, bool bOutput)
{
    const bool bConFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE);
    const int  nConFltrInclMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_INCLMASK);
    const int  nConFltrExclMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_EXCLMASK);

    tstr _mask;
    tstr sLine;

    // >>> console output filters
    if ( bConFltrEnable && ((nConFltrInclMask > 0) || (nConFltrExclMask > 0)) )
    {
        for ( int i = 0; bOutput && 
               (i < CConsoleOutputFilterDlg::FILTER_ITEMS); i++ )
        {
            int len = 0;
            const TCHAR* cszLine = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_INCLLINE1 + i, &len);
            sLine = cszLine;
            if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sLine, false) )
            {
                cszLine = sLine.c_str();
                len = sLine.length();
            }

            if ( (nConFltrInclMask & (0x01 << i)) && (len > 0) )
            {
                _mask = cszLine;
                NppExecHelpers::StrLower(_mask);
                if ( !c_base::_tmatch_mask(_mask.c_str(), _line.c_str()) )
                {
                    bOutput = false;
                }
            }

            len = 0;
            cszLine = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_EXCLLINE1 + i, &len);
            sLine = cszLine;
            if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sLine, false) )
            {
                cszLine = sLine.c_str();
                len = sLine.length();
            }

            if ( bOutput && (nConFltrExclMask & (0x01 << i)) && (len > 0) )
            {
                _mask = cszLine;
                NppExecHelpers::StrLower(_mask);
                if ( c_base::_tmatch_mask(_mask.c_str(), _line.c_str()) )
                {
                    bOutput = false;
                }
            }
        }
    }
    // <<< console output filters

    return bOutput;
}

bool CChildProcess::applyReplaceFilters(tstr& _line, tstr& printLine, bool bOutput)
{
    const bool bRplcFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_ENABLE);
    const bool bRplcFltrExclEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_EXCLEMPTY);
    const int  nRplcFltrFindMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_R_FINDMASK);
    const int  nRplcFltrCaseMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_R_CASEMASK);

    tstr _mask;
    tstr sFind;
    tstr sRplc;

    // >>> console replace filters
    if ( bOutput && bRplcFltrEnable && (nRplcFltrFindMask > 0) )
    {
        bool bModified = false;
        
        // >>> for ...
        for ( int i = 0; bOutput && 
               (i < CConsoleOutputFilterDlg::REPLACE_ITEMS); i++ )
        {
            int lenFind = 0;
            const TCHAR* cszFind = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_R_FIND1 + i, &lenFind);
            sFind = cszFind;
            if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sFind, false) )
            {
                cszFind = sFind.c_str();
                lenFind = sFind.length();
            }

            if ( (nRplcFltrFindMask & (0x01 << i)) && 
                 ( ((lenFind > 0) && (_line.length() > 0)) || 
                   ((lenFind == 0) && (_line.length() == 0)) )
               )
            {
                int lenRplc = 0;
                const TCHAR* cszRplc = m_pNppExec->GetOptions().GetStr(OPTS_CONFLTR_R_RPLC1 + i, &lenRplc);
                sRplc = cszRplc;
                if ( m_pNppExec->GetMacroVars().CheckAllMacroVars(m_pScriptEngine, sRplc, false) )
                {
                    cszRplc = sRplc.c_str();
                    lenRplc = sRplc.length();
                }

                if ( lenFind > 0 )
                {
                    // original string is not empty
                    
                    int pos = 0;
                    if ( nRplcFltrCaseMask & (0x01 << i) )
                    {
                        // match case
                        while ( (pos = printLine.Find(cszFind, pos)) >= 0 )
                        {
                            bModified = true;

                            // both variables must be changed to stay synchronized
                            _line.Replace(pos, lenFind, cszRplc, lenRplc);
                            printLine.Replace(pos, lenFind, cszRplc, lenRplc);
                            pos += lenRplc;
                        }
                    }
                    else
                    {
                        // case-insensitive
                        _mask = cszFind;
                        NppExecHelpers::StrLower(_mask);
                        while ( (pos = _line.Find(_mask.c_str(), pos)) >= 0 )
                        {
                            bModified = true;

                            // both variables must be changed to stay synchronized
                            _line.Replace(pos, lenFind, cszRplc, lenRplc);
                            printLine.Replace(pos, lenFind, cszRplc, lenRplc);
                            pos += lenRplc;
                        }
                    }
                }
                else
                {
                    // replacing original empty string with cszRplc

                    bModified = true;
                    _line = cszRplc;
                    printLine = cszRplc;
                }

                if ( bRplcFltrExclEmpty && bModified && (printLine.length() == 0) )
                {
                    bOutput = false;
                }
            }
        }
        // <<< for ...
        
    }
    // <<< console replace filters

    return bOutput;
}

DWORD CChildProcess::readPipesAndOutput(CStrT<char>& bufLine, 
                                        bool& bPrevLineEmpty,
                                        int&  nPrevState,
                                        bool  bOutputAll,
                                        bool& bDoOutputNext)
{
    DWORD       dwBytesRead = 0;
    char        Buf[CONSOLEPIPE_BUFSIZE];
    CStrT<char> outLine;
  
    bool bSomethingHasBeenReadFromThePipe = false; // great name for a local variable :-)

    const bool bConFltrEnable = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE);
    const bool bConFltrExclAllEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_EXCLALLEMPTY);
    const bool bConFltrExclDupEmpty = m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_EXCLDUPEMPTY);
    const bool bOutputVar = m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SETOUTPUTVAR);
    const int  nAnsiEscSeq = m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_ANSIESCSEQ);

    const int nBufLineLength = bufLine.length();

    do
    { 
        ::Sleep(10);  // it prevents from 100% CPU usage while reading!
        dwBytesRead = 0;
        if ( !::PeekNamedPipe(m_hStdOutReadPipe, NULL, 0, NULL, &dwBytesRead, NULL) )
        {
            dwBytesRead = 0;
        }
        if ( !dwBytesRead )
        {
            // no data in the pipe
            if ( !bSomethingHasBeenReadFromThePipe )
            {
                // did we read something from the pipe already?
                // if no, then let's output the data from bufLine (if any)
                bOutputAll = true;
            }
        }
        if ( (dwBytesRead > 0) || bOutputAll )
        {
            // some data is in the Pipe or bOutputAll==true

            bool bContainsData = (dwBytesRead > 0) ? true : false;
            // without bContainsData==true the ReadFile operation will never return

            if ( bContainsData )
                ::ZeroMemory(Buf, CONSOLEPIPE_BUFSIZE);
            dwBytesRead = 0;
            if ( (bContainsData 
                  && ::ReadFile(m_hStdOutReadPipe, Buf, (CONSOLEPIPE_BUFSIZE-1)*sizeof(char), &dwBytesRead, NULL)
                  && (dwBytesRead > 0)) || bOutputAll )
            {
                // some data has been read from the Pipe or bOutputAll==true
        
                int copy_len;

                Buf[dwBytesRead/sizeof(char)] = 0;

                bufLine.Append( Buf, dwBytesRead/sizeof(char) );

                if ( dwBytesRead > 0 )
                {
                    bSomethingHasBeenReadFromThePipe = true;
                }

                // The following lines are needed for filtered output only.
                // I.e. you can replace all these lines by this one:
                //     GetConsole().PrintOutput(Buf);
                // if you don't need filtered output. (*)
                // (*) But don't forget about Unicode version:
                //     OEM -> WideChar or UTF-8 -> WideChar
        
                /**/
                do {
         
                    copy_len = -1;

                    for ( int pos = 0; pos < bufLine.length(); pos++ )
                    {
                        int nIsNewLine = 0;
                        if ( bufLine[pos] == '\n' )
                        {
                            nIsNewLine = 1; // BIN: 00000001
                        }
                        else if ( bufLine[pos] == '\r' )
                        {
                            if ( bufLine[pos+1] != '\n' )
                            {
                                // not "\r\n" pair
                                if ( (bufLine[pos+1] != '\r') || (bufLine.GetAt(pos+2) != '\n') )
                                {
                                    // not "\r\r\n" (stupid M$'s line ending)
                                    // just "\r"
                                    nIsNewLine = 3; // BIN: 00000011
                                }
                            }
                        }
                        else if ( bufLine[pos] == '\b' )
                        {
                            nIsNewLine = 7; // BIN: 00000111
                        }
                        
                        if ( nIsNewLine || (bOutputAll && (pos == bufLine.length()-1)) )
                        {
                            copy_len = pos;
                            if ( !nIsNewLine )
                            {
                                // i.e. bOutputAll is true
                                copy_len++;
                            }
                            else if ( (pos > 0) && (bufLine[pos-1] == '\r') )
                            {
                                copy_len--;
                                if ( (pos > 1) && (bufLine[pos-2] == '\r') )
                                    copy_len--;
                            }

                            outLine.Copy(bufLine.c_str(), copy_len);

                            if ( nIsNewLine == 7 ) // '\b'
                            {
                                // counting "\b\b..." and skip them
                                while ( bufLine[pos+1] == '\b' )
                                {
                                    ++nIsNewLine;
                                    ++pos;
                                }
                            }

                            bufLine.Delete(0, pos+1);
                            if ( (copy_len > 0) ||
                                 ( ((!bConFltrExclAllEmpty) || (!bConFltrEnable)) &&
                                   ((!bPrevLineEmpty) || (!bConFltrEnable) || (!bConFltrExclDupEmpty))
                                 ) )
                            {
                                tstr printLine;
                                bool bOutput = bConFltrEnable ? bDoOutputNext : true;

                                if ( bOutput )
                                {
                                    tstr _line;

                                    if ( outLine.length() > 0 )
                                    {
                                        unsigned int enc = m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_ENCODING);
                                        enc = CConsoleEncodingDlg::getOutputEncoding(enc);
                                    
                                      #ifdef UNICODE

                                        switch ( enc )
                                        {
                                            case CConsoleEncodingDlg::ENC_OEM :
                                                _line = NppExecHelpers::CStrToWStr(outLine, CP_OEMCP);
                                                break;
                                            
                                            case CConsoleEncodingDlg::ENC_UTF8 :
                                                _line = NppExecHelpers::CStrToWStr(outLine, CP_UTF8);
                                                break;

                                            default:
                                                _line = NppExecHelpers::CStrToWStr(outLine, CP_ACP);
                                                break;
                                        }

                                        {
                                            wchar_t wchNulChar = CNppConsoleRichEdit::GetNulChar();
                                            if ( wchNulChar != 0 )
                                            {
                                                _line.Replace( wchar_t(0x0000), wchNulChar ); // e.g. to 0x25E6 - the "White Bullet" symbol
                                            }
                                        }

                                      #else

                                        {
                                            char chNulChar = CNppConsoleRichEdit::GetNulChar();
                                            if ( chNulChar != 0 )
                                            {
                                                outLine.Replace( char(0x00), chNulChar ); // e.g. to 0x17 - the "End of Text Block" symbol
                                            }
                                        }

                                        switch ( enc )
                                        {
                                            case CConsoleEncodingDlg::ENC_OEM :
                                                if ( _line.SetSize(outLine.length() + 1) )
                                                {
                                                    ::OemToChar( outLine.c_str(), _line.c_str() );
                                                    _line.CalculateLength();
                                                }
                                                break;
                                            
                                            case CConsoleEncodingDlg::ENC_UTF8 :
                                                {
                                                    char* pStr = SysUniConv::newUTF8ToMultiByte( outLine.c_str() );
                                                    if ( pStr )
                                                    {
                                                        _line = pStr;
                                                        delete [] pStr;
                                                    }
                                                }
                                                break;

                                            default:
                                                _line = outLine;
                                                break;
                                        }

                                      #endif

                                        printLine = _line;
                                        NppExecHelpers::StrLower(_line);
                                    }

                                    // >>> console output filters
                                    bOutput = applyOutputFilters(_line, bOutput);
                                    // <<< console output filters

                                    // >>> console replace filters
                                    bOutput = applyReplaceFilters(_line, printLine, bOutput);
                                    // <<< console replace filters
                                }
                                    
                                if ( bOutput )
                                {
                                    if ( nAnsiEscSeq == escRemove )
                                    {
                                        RemoveAnsiEscSequencesFromLine(printLine);
                                    }

                                    if ( nPrevState == 3 ) // '\r'
                                    {
                                        m_pNppExec->GetConsole().ProcessSlashR();
                                    }
                                    else if ( nPrevState >= 7 ) // '\b'...
                                    {
                                        m_pNppExec->GetConsole().ProcessSlashB( (nPrevState - 7) + 1 );
                                    }
                                    
                                    if ( bOutputVar )
                                    {
                                        m_strOutput += printLine;
                                        if ( nIsNewLine == 1 )
                                        {
                                            m_strOutput += _T("\n");
                                        }
                                    }

                                    m_pNppExec->GetConsole().PrintOutput( printLine.c_str(), (nIsNewLine == 1) ? true : false );
                                }

                                // if the current line is not over, then the current filter 
                                // must be applied to the rest of this line
                                bDoOutputNext = bOutput;
                            }
                            bPrevLineEmpty = (copy_len > 0) ? false : true;
                            nPrevState = nIsNewLine;
                            if ( nIsNewLine == 1 )
                            {
                                // current line is over - abort current filter
                                bDoOutputNext = true;
                            }
                            break;
                        }
                    
                    }
                } while ( copy_len >= 0 );
                /**/

            }
        }

    } 
    while ( (dwBytesRead > 0) && m_pScriptEngine->ContinueExecution() && !isBreaking() );

    if ( bOutputAll && !dwBytesRead )  dwBytesRead = nBufLineLength;
    return dwBytesRead;
}

void CChildProcess::RemoveAnsiEscSequencesFromLine(tstr& Line)
{
    // ANSI escape codes, references:
    //   https://en.wikipedia.org/wiki/ANSI_escape_code
    //   https://en.wikipedia.org/wiki/ISO/IEC_2022
    //   https://man7.org/linux/man-pages/man4/console_codes.4.html
    //   http://ascii-table.com/ansi-escape-sequences.php
    //   http://ascii-table.com/ansi-escape-sequences-vt-100.php
    //   https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
    //   https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences

    enum eEscState {
        esNone = 0,
        esEsc,    // ESC symbol found
        esCsi,    // CSI sequence
        esOsc,    // OSC sequence
        esWaitSt, // wait for ST (ESC \)
        esWait1,  // wait for 1 symbol
        esWait2   // wait for 2 symbols
    };

    
    const TCHAR* p = Line.c_str();
    eEscState state = esNone;
    TCHAR curr_ch = 0;
    TCHAR wait1_ch = 0;
    TCHAR wait2_ch = 0;
    tstr outputLine;

    outputLine.Reserve(Line.length());

    while ( (curr_ch = *p) != 0 )
    {
        switch ( state )
        {
            case esNone:
                if ( curr_ch == 0x1B )  // ESC
                {
                    state = esEsc;
                }
                else
                {
                    outputLine.Append(curr_ch);
                }
                break;

            case esEsc:
                switch ( curr_ch )
                {
                    case _T('['):  // CSI
                        state = esCsi;
                        break;
                    case _T(']'):  // OSC
                        state = esOsc;
                        break;
                    case _T('P'):  // DCS
                    case _T('X'):  // SOS
                    case _T('^'):  // PM
                    case _T('_'):  // APC
                        state = esWaitSt;
                        break;
                    case _T('$'):  // G?DM?
                        state = esWait2;
                        wait2_ch = curr_ch;
                        break;
                    case _T('!'):  // C0-designate
                    case _T('"'):  // C1-designate
                    case _T('#'):  // single control function
                    case _T('%'):  // DOCS
                    case _T('&'):  // IRR
                    case _T('('):  // G0 character set
                    case _T(')'):  // G1 character set
                    case _T('*'):  // G2 character set
                    case _T('+'):  // G3 character set
                    case _T('-'):  // G1 character set, VT300
                    case _T('.'):  // G2 character set, VT300
                    case _T('/'):  // G3 character set, VT300
                    case _T(' '):  // ACS
                        state = esWait1;
                        wait1_ch = curr_ch;
                        break;
                    default:       // RIS, IND, NEL, HTS, RI, ...
                        state = esNone;
                        break;
                }
                break;

            case esCsi:
                if ( (curr_ch >= _T('A') && curr_ch <= _T('Z')) ||
                     (curr_ch >= _T('a') && curr_ch <= _T('z')) ||
                     (curr_ch == _T('@'))  ||
                     (curr_ch == _T('['))  ||
                     (curr_ch == _T('\\')) ||
                     (curr_ch == _T(']'))  ||
                     (curr_ch == _T('^'))  ||
                     (curr_ch == _T('_'))  ||
                     (curr_ch == _T('`'))  ||
                     (curr_ch == _T('{'))  ||
                     (curr_ch == _T('|'))  ||
                     (curr_ch == _T('}'))  ||
                     (curr_ch == _T('~')) )
                {
                    // the "final byte" of the CSI sequence
                    state = esNone;
                }
                // else waiting for the CSI final character...
                break;

            case esOsc:
            case esWaitSt:
                if ( curr_ch == 0x1B )  // ESC
                {
                    const TCHAR next_ch = *(p + 1);
                    if ( next_ch == _T('\\') )  // ST
                    {
                        ++p; // skipping the next character as well
                        state = esNone;
                    }
                    else // ?
                    {
                        state = esEsc; // ?
                    }
                }
                // else waiting for the ST...
                break;

            case esWait1:
                state = esNone;
                if ( wait1_ch == _T('%') )
                {
                    switch ( curr_ch )
                    {
                        case _T('/'):  // ESC % / F
                            state = esWait1;
                            break;
                    }
                }
                wait1_ch = 0;
                break;

            case esWait2:
                state = esWait1;
                if ( wait2_ch == _T('$') )
                {
                    switch ( curr_ch )
                    {
                        case _T('@'):  // ESC $ @
                        case _T('A'):  // ESC $ A
                        case _T('B'):  // ESC $ B
                            state = esNone;
                            break;
                    }
                }
                wait2_ch = 0;
                break;
        }

        ++p;
    }

    Line.Swap(outputLine);
}

void CChildProcess::MustBreak(unsigned int nBreakMethod)
{
    m_nBreakMethod = nBreakMethod;
}

bool CChildProcess::Kill(const CProcessKiller::eKillMethod arrKillMethods[], int nKillMethods,
                         unsigned int nWaitTimeout,
                         CProcessKiller::eKillMethod* pnSucceededKillMethod)
{
    bool isKilled = false;
    CProcessKiller pk(GetProcessInfo());
    for ( int i = 0; i < nKillMethods; i++ )
    {
        CProcessKiller::eKillMethod nKillMethod = arrKillMethods[i];
        isKilled = pk.Kill(nKillMethod, nWaitTimeout);
        if ( isKilled )
        {
            if ( pnSucceededKillMethod )
                *pnSucceededKillMethod = nKillMethod;
            break;
        }
    }
    return isKilled;
}

bool CChildProcess::WriteInput(const TCHAR* szLine, bool bFFlush )
{
    if ( (!szLine) || (!m_hStdInWritePipe) )
        return false;

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CChildProcess::WriteInput(\"%s\") (instance = %s)"), szLine, GetInstanceStr() );
    
    char*        pStr = NULL;
    int          len = 0;
    DWORD        dwBytesWritten = 0;
    unsigned int enc = m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_ENCODING);
        
    enc = CConsoleEncodingDlg::getInputEncoding(enc);

  #ifdef UNICODE
    
    switch ( enc )
    {
        case CConsoleEncodingDlg::ENC_OEM :
            pStr = SysUniConv::newUnicodeToMultiByte( szLine, -1, CP_OEMCP, &len );
            break;
        
        case CConsoleEncodingDlg::ENC_UTF8 :
            pStr = SysUniConv::newUnicodeToUTF8( szLine, -1, &len );
            break;

        default:
            pStr = SysUniConv::newUnicodeToMultiByte( szLine, -1, CP_ACP, &len );
            break;
    }
    
    if ( pStr )
    {
        ::WriteFile(m_hStdInWritePipe, pStr, len*sizeof(char), &dwBytesWritten, NULL);
        if ( bFFlush )
        {
            // beware! this may hang the application (due to MustDie's pipes)
            ::FlushFileBuffers(m_hStdInWritePipe);
        }
        delete [] pStr;
    }
                  
  #else
    
    bool bNewMemory = false;
    
    switch ( enc )
    {
        case CConsoleEncodingDlg::ENC_OEM :
            len = lstrlen(szLine);
            pStr = new char[len + 1];
            if ( pStr )
            {
                ::CharToOem(szLine, pStr);
                bNewMemory = true;
            }
            break;

        case CConsoleEncodingDlg::ENC_UTF8 :
            pStr = SysUniConv::newMultiByteToUTF8(szLine, -1, CP_ACP, &len);
            if ( pStr )
            {
                bNewMemory = true;
            }
            break;

        default:
            len = lstrlen(szLine);
            pStr = (char *) szLine;
            bNewMemory = false;
            break;
    }

    if ( pStr )
    {
        ::WriteFile(m_hStdInWritePipe, pStr, len*sizeof(char), &dwBytesWritten, NULL);
        if ( bFFlush )
        {
            // beware! this may hang the application (due to MustDie's pipes)
            ::FlushFileBuffers(m_hStdInWritePipe);
        }
        if ( bNewMemory )
            delete [] pStr;
    }
  
  #endif

    return true;
}

void CChildProcess::closePipes()
{
    ::CloseHandle(m_hStdOutReadPipe);  m_hStdOutReadPipe = NULL;
    ::CloseHandle(m_hStdOutWritePipe); m_hStdOutWritePipe = NULL;
    ::CloseHandle(m_hStdInReadPipe);   m_hStdInReadPipe = NULL;
    ::CloseHandle(m_hStdInWritePipe);  m_hStdInWritePipe = NULL;
}

tstr& CChildProcess::GetOutput()
{
    return m_strOutput;
}

int CChildProcess::GetExitCode() const
{
    return m_nExitCode;
}

DWORD CChildProcess::GetProcessId() const
{
    return m_ProcessInfo.dwProcessId;
}

const PROCESS_INFORMATION* CChildProcess::GetProcessInfo() const
{
    return &m_ProcessInfo;
}

bool CChildProcess::IsWindowsNT()
{
#ifdef _WIN64
  return true; // 64-bit Windows XP or later? Definitely Windows NT!
#else
  OSVERSIONINFO osv;
  osv.dwOSVersionInfoSize = sizeof(osv);
  GetVersionEx(&osv);
  return (osv.dwPlatformId == VER_PLATFORM_WIN32_NT);
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CNppExec::printScriptLog(const TCHAR* str, int)
{
    // static func
    if ( CNppExec::_bIsNppShutdown ) // if it is called _after_ NPPN_SHUTDOWN, ~CNppExec() may have been called already
        return;

    Runtime::GetNppExec().GetConsole().PrintError( str, false );
}

void CNppExec::printScriptString(const TCHAR* str, int)
{
    // static func
    if ( CNppExec::_bIsNppShutdown ) // if it is called _after_ NPPN_SHUTDOWN, ~CNppExec() may have been called already
        return;

    Runtime::GetNppExec().GetConsole().PrintMessage( str, false, false );
}

bool CNppExec::SendChildProcessExitCommand()
{
    bool bSend = false;

    if ( !GetCommandExecutor().GetTriedExitCmd() )
    {
        CCriticalSectionLockGuard lock(GetMacroVars().GetCsUserMacroVars());
        const int nMacroVarsLists = 2;
        const CNppExecMacroVars::tMacroVars* pMacroVarsLists[nMacroVarsLists] = {
            &GetMacroVars().GetUserLocalMacroVars(nullptr), // try local first
            &GetMacroVars().GetUserMacroVars()
        };

        const int nExitMacroVars = 2;
        const TCHAR* cszExitMacroVars[nExitMacroVars] = {
            MACRO_EXIT_CMD,        // try non-silent first
            MACRO_EXIT_CMD_SILENT
        };

        tstr exit_cmd;
        tstr macro_name;

        for ( int nList = 0; (nList < nMacroVarsLists) && (!bSend); nList++ )
        {
            const CNppExecMacroVars::tMacroVars* pVarsList = pMacroVarsLists[nList];
            for ( int nVar = 0; (nVar < nExitMacroVars) && (!bSend); nVar++ )
            {
                macro_name = cszExitMacroVars[nVar];
                CNppExecMacroVars::tMacroVars::const_iterator itr = pVarsList->find(macro_name);
                if ( itr != pVarsList->end() )
                {
                    exit_cmd = itr->second;
                    bSend = true;
                }
            }
        }

        if ( bSend )
        {
            GetCommandExecutor().SetTriedExitCmd(true);

            Runtime::GetLogger().AddEx( _T("; child process automatic command: %s = %s"), macro_name.c_str(), exit_cmd.c_str() );

            if ( macro_name == MACRO_EXIT_CMD )
            {
                GetConsole().PrintStr( exit_cmd.c_str(), true );
            }
            GetCommandExecutor().ExecuteChildProcessCommand( exit_cmd );
        }
    }

    return bSend;
}

bool CNppExec::ShowChildProcessExitDialog()
{
    const CListItemT<tstr>* p = InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_EXITPROCESS].GetFirst();

    // init the InputBox dialog values
    InputBoxDlg.m_InputMessage = _T("Console process is still running. Send - send the exit command; Cancel [Esc] - continue. Kill is not recommended.");
    if ( p && (p->GetItem().length() > 0) )
    {
        InputBoxDlg.m_InputMessage += _T(" : ");
        InputBoxDlg.m_InputMessage += p->GetItem();
    }
    InputBoxDlg.m_InputVarName = _T("exit command: ");
    InputBoxDlg.setInputBoxType(CInputBoxDlg::IBT_EXITPROCESS);
        
    // show the InputBox dialog
    INT_PTR ret = PluginDialogBox(IDD_INPUTBOX, InputBoxDlgProc);
    switch ( ret )
    {
        case CInputBoxDlg::RET_OK:
            // OK - send the exit command
            {
                tstr exit_cmd = InputBoxDlg.m_OutputValue;

                Runtime::GetLogger().AddEx( _T("; child process manual command: %s"), exit_cmd.c_str() );

                std::shared_ptr<CScriptEngine> pScriptEngine = GetCommandExecutor().GetRunningScriptEngine();
                if ( pScriptEngine && !pScriptEngine->IsCollateral() )
                {
                    GetConsole().PrintStr( exit_cmd.c_str(), true );
                }
                GetCommandExecutor().ExecuteChildProcessCommand( exit_cmd );
            }
            break;
        case CInputBoxDlg::RET_KILL:
            // Kill
            GetCommandExecutor().ChildProcessMustBreak(CProcessKiller::killCtrlBreak);
            break;
    }

    return (ret >= CInputBoxDlg::RET_OK);
}

bool CNppExec::TryExitRunningChildProcess(unsigned int nFlags)
{
    // send the exit command first...
    if ( SendChildProcessExitCommand() )
    {
        unsigned int uMaxTimeout = GetOptions().GetUint(OPTU_CHILDP_EXITTIMEOUT_MS);
        if ( GetCommandExecutor().WaitUntilAllScriptEnginesDone(uMaxTimeout) )
            return true;
    }

    // the exit command did not succeed or it was not sent...
    if ( (nFlags & sfDoNotShowExitDialog) == 0 )
    {
        if ( ShowChildProcessExitDialog() )
        {
            unsigned int uMaxTimeout = GetOptions().GetUint(OPTU_CHILDP_EXITTIMEOUT_MS);
            if ( GetCommandExecutor().WaitUntilAllScriptEnginesDone(uMaxTimeout) )
                return true;
        }
    }

    return false;
}

bool CNppExec::CanStartScriptOrCommand(unsigned int nFlags)
{
    if ( GetCommandExecutor().IsChildProcessRunning() )
    {
        return TryExitRunningChildProcess();
    }
    
    if ( GetCommandExecutor().IsScriptRunningOrQueued() )
    {
        if ( (nFlags & sfDoNotShowWarningOnScript) == 0 )
        {
            ShowWarning( _T("Another script is still running") );
        }
        return false;
    }
    
    return true;
}

void CNppExec::Uninit()
{
    StopAutoSaveTimer();

    _consoleIsVisible = false; // stopping every script (except the exit script, if any) & child process

    // As Ununit() is called from DllMain(), it may be dangerous to wait for
    // NppExec's scripts to be done here, as a script potentially can start
    // a process while DllMain() -> Uninit() is executed, whereas the
    // Dynamic-Link Library Best Practices does not recommend to call
    // CreateProcess() while DllMain() is being executed:
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dn633971(v=vs.85).aspx
    // In practice, there is a very small chance of this to happen in NppExec,
    // because a running NppExec's script (that runs in a background thread)
    // would unlikely start a process at the time of DllMain() & Uninit() is
    // executed, taking into account that _consoleIsVisible was set to false
    // on NPPN_SHUTDOWN (see the comment about the _consoleIsVisible above).

    GetPluginInterfaceImpl().Enable(false); // disabling it before the CommandExecutor is off

    GetCommandExecutor().WaitUntilAllScriptEnginesDone(INFINITE);
    GetCommandExecutor().Stop();

    if ( m_TB_Icon.hToolbarBmp )
        ::DeleteObject(m_TB_Icon.hToolbarBmp);
    if ( m_TB_Icon.hToolbarIcon )
        ::DestroyIcon(m_TB_Icon.hToolbarIcon);

    if ( npp_bufFileNames.GetCount() > 0 )
    {
        for ( int i = 0; i < npp_bufFileNames.GetCount(); i++ )
        {
            if ( npp_bufFileNames[i] != NULL )
            {
                delete [] npp_bufFileNames[i];
                npp_bufFileNames[i] = NULL;  // just in case
            }
        }
    }

}

tstr CNppExec::ExpandToFullConfigPath(const TCHAR* cszFileName, bool bTryCloud )
{
  if (bTryCloud)
  {
    tstr cloudPath = GetSettingsCloudPluginDir();
    if (!cloudPath.IsEmpty())
    {
      cloudPath += cszFileName;
      if (NppExecHelpers::IsValidTextFile(cloudPath))
        return cloudPath;
    }
  }

  tstr localPath = m_szConfigPath;
  localPath += _T("\\");
  localPath += cszFileName;
  return localPath;
}

void CNppExec::InitPluginName(HMODULE hDllModule)
{
  INT   i;
  INT   n;
  TCHAR ch;
  TCHAR szPath[FILEPATH_BUFSIZE];

  szPath[0] = 0;
  GetModuleFileName(hDllModule /*NULL*/ , szPath, FILEPATH_BUFSIZE - 1);
  n = lstrlen(szPath) - 1;
  while (n >= 0 && (ch = szPath[n]) != _T('\\') && ch != _T('/'))  n--;
  i = n; // pos of the last '\' or '/'
  if (i > 0)
  {
    int j = 0;
    while (j < MAX_PLUGIN_NAME)
    {
      ch = szPath[++i];
      if (ch == _T('.'))  ch = 0;
      PLUGIN_NAME[j++] = ch;
      if (!ch)
        break;
    }
    if (j >= MAX_PLUGIN_NAME)
    {
      PLUGIN_NAME[MAX_PLUGIN_NAME - 1] = 0;
    }
  }

  {
    tstr name = PLUGIN_NAME;
    if ( name.length() > 0 )
    {
      // needed in MustDie9x because GetModuleFileName returns 
      // capital letters there (stupid M$...)
      NppExecHelpers::StrUpper(name);
      i = name.Find( _T("NPPEXEC") );
      if ( i >= 0 )
      {
        name.Replace( i, 7, _T("NppExec") );
        if ( i > 1 )
        {
          ::CharLowerBuff( (name.c_str() + 1), i - 1 );
        }
        if ( i + 8 < name.length() )
        {
          ::CharLowerBuff( (name.c_str() + i + 8), name.length() - i - 8 );
        }
        lstrcpy( PLUGIN_NAME, name.c_str() );
      
        // Console dialog title:
        tstr S = name;
        S.Replace( _T("NppExec"), _T("Console") );
        S.Insert( 0, _T(' ') );
        S.Append( _T(' ') );
        lstrcpy( CONSOLE_DLG_TITLE, S.c_str() );

        // Show Console menu item:
        S = name;
        S.Replace( _T("NppExec"), _T("Console") );
        S.Insert( 0, _T("Show ") );
        lstrcpy( SHOW_CONSOLE_MENU_ITEM, S.c_str() );

        // Toggle Console menu item:
        S.Replace( _T("Show "), _T("Toggle ") );
        lstrcpy( TOGGLE_CONSOLE_MENU_ITEM, S.c_str() );

        // Command History file name:
        S = name;
        S.Replace( _T("NppExec"), _T("npec_cmdhistory") );
        S.Append( _T(".txt") );
        lstrcpy( CMDHISTORY_FILENAME, S.c_str() );
      }
    }
  }

  i = n;
  if (i >= 0)  szPath[i] = 0;
  lstrcpy(m_szPluginDllPath, szPath);
  lstrcpy(m_szConfigPath, szPath);
  
  lstrcpy( INI_FILENAME, PLUGIN_NAME );
  lstrcat( INI_FILENAME, _T(".ini") );

  lstrcpy( PLUGIN_NAME_DLL, PLUGIN_NAME );
  lstrcat( PLUGIN_NAME_DLL, _T(".dll") );
}

void CNppExec::Init()
{
  GetPluginInterfaceImpl().SetPluginName( PLUGIN_NAME_DLL );

  if ( m_nppData._nppHandle )
  {
    SendNppMsg( NPPM_GETPLUGINSCONFIGDIR,
      (WPARAM) (FILEPATH_BUFSIZE - 1), (LPARAM) m_szConfigPath );

    if ( !NppExecHelpers::CheckDirectoryExists(m_szConfigPath) )
      NppExecHelpers::CreateDirectoryTree(m_szConfigPath);
  }
  else
  {
    HANDLE          fhandle;
    WIN32_FIND_DATA fdata;
    TCHAR           szPath[FILEPATH_BUFSIZE];

    lstrcpy(szPath, m_szPluginDllPath);
    lstrcat(szPath, _T("\\Config\\*.*"));
    fhandle = FindFirstFile(szPath, &fdata);
    if ((fhandle == INVALID_HANDLE_VALUE) || !fhandle)
    {
          //ShowWarning("FindFirstFile failed");
      lstrcat(m_szConfigPath, _T("\\Config"));
      if (CreateDirectory(m_szConfigPath, NULL))
      {
          //ShowWarning("CreateDirectory OK");
      }
      else
      {
          //ShowWarning("CreateDirectory failed");
        lstrcpy(m_szConfigPath, m_szPluginDllPath);
      }
    }
    else
    {
          //ShowWarning("FindFirstFile OK");
      FindClose(fhandle);
      lstrcat(m_szConfigPath, _T("\\Config"));
    }
  }
  lstrcpy(m_szIniFilePathName, m_szConfigPath);
  lstrcat(m_szIniFilePathName, _T("\\"));
  lstrcat(m_szIniFilePathName, INI_FILENAME);
          //ShowWarning(m_szIniFilePathName);

  ReadOptions();
  GetCommandExecutor().Start();
  GetPluginInterfaceImpl().Enable(true); // enabling it after the CommandExecutor is up
}

void CNppExec::OnCmdHistory()
{
  HMENU hMenu = GetNppMainMenu();
  if (hMenu)
  {
    const bool bCmdHistory = !GetOptions().GetBool(OPTB_CONSOLE_CMDHISTORY);
    GetOptions().SetBool(OPTB_CONSOLE_CMDHISTORY, bCmdHistory);
    CheckMenuItem(hMenu, g_funcItem[N_CMDHISTORY]._cmdID,
      MF_BYCOMMAND | (bCmdHistory ? MF_CHECKED : MF_UNCHECKED));
  }
}

void CNppExec::OnDoExecDlg()
{
    initConsoleDialog();
    CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::OnDoExecDlgCommand(tstr());
    GetCommandExecutor().ExecuteCommand(pCommand);
}

void CNppExec::OnDirectExec(const tstr& id, bool bCanSaveAll, unsigned int nRunFlags /* = 0 */ )
{
    initConsoleDialog();
    CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::OnDirectExecCommand(id, bCanSaveAll, nRunFlags);
    GetCommandExecutor().ExecuteCommand(pCommand);
}

void CNppExec::DoExecScript(const tstr& id, LPCTSTR szScriptName, bool bCanSaveAll, LPCTSTR szScriptArguments /* = NULL */ , unsigned int nRunFlags /* = 0 */ )
{
    initConsoleDialog();
    CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::DoExecScriptCommand(id, szScriptName, szScriptArguments, bCanSaveAll, nRunFlags);
    GetCommandExecutor().ExecuteCommand(pCommand);
}

void CNppExec::DoRunScript(const CListT<tstr>& CmdList, unsigned int nRunFlags /* = 0 */ )
{
    initConsoleDialog();
    CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::DoRunScriptCommand(tstr(), CmdList, nRunFlags);
    GetCommandExecutor().ExecuteCommand(pCommand);
}

void CNppExec::RunTheStartScript()
{
    const TCHAR* pStartScriptName = GetOptions().GetStr(OPTS_SCRIPT_NPPSTART);
    if ( pStartScriptName && pStartScriptName[0] )
    {
        DoExecScript( tstr(), pStartScriptName, false, NULL, CScriptEngine::rfStartScript );
    }
}

void CNppExec::RunTheExitScript()
{
    // Note: the Console output is already disabled (see the NPPN_SHUTDOWN handler).
    // One more important note: NPPN_SHUTDOWN means that NppExec already did what it
    // wanted to allow to shut down. It also means the exit script will be started
    // right here right now because NppExec is ready to run it.

    const TCHAR* pszExitScriptName = GetOptions().GetStr(OPTS_SCRIPT_NPPEXIT);
    if ( pszExitScriptName && pszExitScriptName[0] )
    {
        if ( m_ExitScriptIsDone.IsNull() )
            m_ExitScriptIsDone.Create(NULL, TRUE, FALSE, NULL);

        // instead of calling DoExecScript(), execute it right here, synchronously
        initConsoleDialog();
        CNppExecCommandExecutor::DoExecScriptCommand Cmd(tstr(), pszExitScriptName, NULL, false, CScriptEngine::rfExitScript);

        // the exit script timeout helper
        NppExecHelpers::CreateNewThread(ExitScriptTimeoutThreadProc, this);

        // executing the exit script
        Cmd.Execute();

        // the exit script is done (or has been aborted)
        m_ExitScriptIsDone.Set();
    }
}

void CNppExec::OnNoInternalMsgs()
{
  HMENU hMenu = GetNppMainMenu();
  if (hMenu)
  {
    const bool bNoInternalMsgs = !GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS);
    GetOptions().SetBool(OPTB_CONSOLE_NOINTMSGS, bNoInternalMsgs);
    CheckMenuItem(hMenu, g_funcItem[N_NOINTMSGS]._cmdID,
      MF_BYCOMMAND | (bNoInternalMsgs ? MF_CHECKED : MF_UNCHECKED));
  }
}

void CNppExec::OnNoCmdAliases()
{
    HMENU hMenu = GetNppMainMenu();
    if (hMenu)
    {
        const bool bNoCmdAliases = !GetOptions().GetBool(OPTB_CONSOLE_NOCMDALIASES);
        GetOptions().SetBool(OPTB_CONSOLE_NOCMDALIASES, bNoCmdAliases);
        CheckMenuItem(hMenu, g_funcItem[N_NOCMDALIASES]._cmdID,
            MF_BYCOMMAND | (bNoCmdAliases ? MF_CHECKED : MF_UNCHECKED));
    }
}

#ifdef _SCROLL_TO_LATEST
void CNppExec::OnScrollToLatest()
{
  HMENU hMenu = GetNppMainMenu();
  if (hMenu)
  {
    const bool bScrollToLatest = !GetOptions().GetBool(OPTB_CONSOLE_SCROLL2LATEST);
    GetOptions().SetBool(OPTB_CONSOLE_SCROLL2LATEST, bScrollToLatest);
    CheckMenuItem(hMenu, g_funcItem[N_SCROLL2LATEST]._cmdID,
      MF_BYCOMMAND | (bScrollToLatest ? MF_CHECKED : MF_UNCHECKED));
  }
}
#endif

void CNppExec::OnConsoleEncoding()
{
    ConsoleEncodingDlg.m_enc_opt = GetOptions().GetUint(OPTU_CONSOLE_ENCODING);
    if ( PluginDialogBox(IDD_CONSOLE_ENCODING, ConsoleEncodingDlgProc) == 1 )
    {
        GetOptions().SetUint(OPTU_CONSOLE_ENCODING, ConsoleEncodingDlg.m_enc_opt);
        UpdateConsoleEncoding();
    }
}

void CNppExec::UpdateConsoleEncoding()
{
    HMENU hMenu = GetNppMainMenu();
    if ( hMenu )
    {
        TCHAR        szItemText[128];
        MENUITEMINFO mii;

        ::ZeroMemory( &mii, sizeof(MENUITEMINFO) );
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
        mii.dwTypeData = (TCHAR*) szItemText;
        mii.cch = 128 - 1;

        if ( ::GetMenuItemInfo(hMenu, g_funcItem[N_CONSOLE_ENC]._cmdID, FALSE, &mii) )
        {
            unsigned int enc_opt = GetOptions().GetUint(OPTU_CONSOLE_ENCODING);
            lstrcpy( szItemText, _T("Console Output... {") );
            lstrcat( szItemText, CConsoleEncodingDlg::getOutputEncodingName(enc_opt) );
            lstrcat( szItemText, _T("/") );
            lstrcat( szItemText, CConsoleEncodingDlg::getInputEncodingName(enc_opt) );
            lstrcat( szItemText, _T("}") );
            ::SetMenuItemInfo(hMenu, g_funcItem[N_CONSOLE_ENC]._cmdID, FALSE, &mii);
        }
    }
}

void CNppExec::OnOutputFilter()
{
  if ( PluginDialogBox(IDD_CONSOLE_FILTER, ConsoleOutputFilterDlgProc) == 1 )
  {
    UpdateOutputFilterMenuItem();
    UpdateGoToErrorMenuItem();
  }
}

void CNppExec::UpdateOutputFilterMenuItem()
{
    HMENU hMenu = GetNppMainMenu();
    if ( hMenu )
    {
        bool bFilterEnabled = GetOptions().GetBool(OPTB_CONFLTR_ENABLE);
        if ( !bFilterEnabled )
            bFilterEnabled = GetOptions().GetBool(OPTB_CONFLTR_R_ENABLE);
        CheckMenuItem(hMenu, g_funcItem[N_OUTPUT_FILTER]._cmdID,
          MF_BYCOMMAND | (bFilterEnabled ? MF_CHECKED : MF_UNCHECKED));
    }
}

void CNppExec::UpdateGoToErrorMenuItem()
{
    HMENU hMenu = GetNppMainMenu();
    if ( hMenu )
    {
        UINT uEnable = GetWarningAnalyzer().HasEnabledFilters() ? MF_ENABLED : MF_GRAYED;
        EnableMenuItem(hMenu, g_funcItem[N_GOTO_NEXT_ERROR]._cmdID, MF_BYCOMMAND | uEnable);
        EnableMenuItem(hMenu, g_funcItem[N_GOTO_PREV_ERROR]._cmdID, MF_BYCOMMAND | uEnable);
    }
}

void CNppExec::OnAdvancedOptions()
{
    if ( PluginDialogBox(IDD_ADVANCEDOPTIONS, AdvancedOptionsDlgProc) == 1 )
    {
    }
}

void CNppExec::OnSaveOnExecute()
{
  HMENU hMenu = GetNppMainMenu();
  if (hMenu)
  {
    const bool bSaveOnExecute = !GetOptions().GetBool(OPTB_CONSOLE_SAVEONEXECUTE);
    GetOptions().SetBool(OPTB_CONSOLE_SAVEONEXECUTE, bSaveOnExecute);
    CheckMenuItem(hMenu, g_funcItem[N_SAVEONEXECUTE]._cmdID,
      MF_BYCOMMAND | (bSaveOnExecute ? MF_CHECKED : MF_UNCHECKED));
  }
}

void CNppExec::OnCdCurDir()
{
  HMENU hMenu = GetNppMainMenu();
  if (hMenu)
  {  
    const bool bCDCurDir = !GetOptions().GetBool(OPTB_CONSOLE_CDCURDIR);
    GetOptions().SetBool(OPTB_CONSOLE_CDCURDIR, bCDCurDir);
    CheckMenuItem(hMenu, g_funcItem[N_CDCURDIR]._cmdID,
      MF_BYCOMMAND | (bCDCurDir ? MF_CHECKED : MF_UNCHECKED));
    UpdateCurrentDirectory();
  }
}

void CNppExec::OnSelectConsoleFont()
{
  LOGFONT    lf;
  CHOOSEFONT cf;
  HWND       hEd = GetConsole().GetConsoleWnd();
  HFONT      hEdFont = _consoleFont;
  
  ZeroMemory(&lf, sizeof(LOGFONT));
  GetObject(hEdFont ? hEdFont : GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);

  ZeroMemory(&cf, sizeof(CHOOSEFONT));
  cf.lStructSize = sizeof(CHOOSEFONT);
  cf.hwndOwner = m_nppData._nppHandle;
  cf.lpLogFont = &lf;
  cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
  
  HWND hWndFocus = ::GetFocus();
  
  if (ChooseFont(&cf))
  {
    /*
    if (lf.lfHeight > 0) 
      lf.lfHeight = -lf.lfHeight;
    */
    SetConsoleFont(hEd, &lf);
  }

  ::SetFocus(hWndFocus);

}

void CNppExec::OnShowConsoleDlg()
{
    if ( CNppExec::_bIsNppReady || (GetOptions().GetInt(OPTI_CONSOLE_VISIBLE) == CAdvOptDlg::CON_AUTO) )
        showConsoleDialog(hideIfShown, 0);
}

void CNppExec::OnToggleConsoleDlg()
{
    if ( !isConsoleDialogVisible() )
    {
        showConsoleDialog(showIfHidden, 0);
    }
    else
    {
        HWND hFocused = ::GetFocus();

        if ( (GetConsole().GetDialogWnd() == hFocused) || 
             IsChild(GetConsole().GetDialogWnd(), hFocused) )
        {
            if ( GetOptions().GetBool(OPTB_CONSOLE_HIDETOGGLED) && !GetCommandExecutor().IsChildProcessRunning() )
                showConsoleDialog(hideIfShown, 0);
            else
            {
                ConsoleDlg::EnableTransparency(true);
                ::SetFocus( GetScintillaHandle() );
            }
        }
        else
        {
            ::SetFocus( GetConsole().GetConsoleWnd() );
        }
    }
}

void CNppExec::OnGoToNextError()
{
    if ( GetConsole().GetDialogWnd() )
        showConsoleDialog(showIfHidden, 0);

    ConsoleDlg::GoToError(1);
}

void CNppExec::OnGoToPrevError()
{
    if ( GetConsole().GetDialogWnd() )
        showConsoleDialog(showIfHidden, 0);

    ConsoleDlg::GoToError(-1);
}

void CNppExec::OnHelpManual()
{
    tstr sHelpFile = GetOptions().GetStr(OPTS_PLUGIN_HELPFILE);
    if ( !sHelpFile.IsEmpty() )  // not empty
    {
        GetMacroVars().CheckPluginMacroVars(sHelpFile);
        GetMacroVars().CheckNppMacroVars(sHelpFile);

        sHelpFile.Replace( _T('/'), _T('\\') );
        if ( NppExecHelpers::IsFullPath(sHelpFile) )
        {
            if ( !NppExecHelpers::CheckFileExists(sHelpFile) )
                return; // file does not exist, nothing to do
        }
        else
        {
            // sHelpFile is a relative pathname
            if ( sHelpFile.GetFirstChar() == _T('\\') )
                sHelpFile.DeleteFirstChar();

            // first, trying the folder at the level of NppExec.dll
            tstr sHelpFilePath = getPluginDllPath();
            sHelpFilePath += _T('\\');
            sHelpFilePath += sHelpFile;
            if ( !NppExecHelpers::CheckFileExists(sHelpFilePath) )
            {
                // then, trying the folder one level upper
                sHelpFilePath = NppExecHelpers::GetFileNamePart(getPluginDllPath(), NppExecHelpers::fnpDirPath);
                sHelpFilePath += sHelpFile;
                if ( !NppExecHelpers::CheckFileExists(sHelpFilePath) )
                    return; // file does not exist, nothing to do
            }
            sHelpFile.Swap(sHelpFilePath);
        }
        
#ifndef __MINGW32__
        if ( sHelpFile.Find(_T(".chm")) > 0 )
            ::HtmlHelp( NULL, sHelpFile.c_str(), HH_DISPLAY_TOPIC, NULL );
        else
#endif
            ::ShellExecute( NULL, _T("open"), sHelpFile.c_str(), NULL, NULL, SW_SHOWNORMAL );
    }
}

void CNppExec::OnHelpDocs()
{
    typedef struct sDocFile {
        const TCHAR* cszFileName;
        bool         bShouldExist;
    } tDocFile;

    const tDocFile docFiles[] = {
        { _T("NppExec_HelpAll.txt"),  false },
        { _T("NppExec_TechInfo.txt"), true  },
        { _T("NppExec.txt"),          true  }
    };
    
    tstr doc_file_path;

    // first, trying the folder at the level of NppExec.dll ("doc\NppExec")
    tstr path1 = getPluginDllPath();
    path1 += _T("\\doc\\NppExec\\");

    // then, trying the folder one level upper ("..\doc\Nppexec")
    tstr path2 = NppExecHelpers::GetFileNamePart(getPluginDllPath(), NppExecHelpers::fnpDirPath);
    path2 += _T("doc\\NppExec\\");

    for ( const tDocFile& docFile : docFiles )
    {
        doc_file_path = path1;
        doc_file_path += docFile.cszFileName;
        if ( !NppExecHelpers::CheckFileExists(doc_file_path) )
        {
            doc_file_path = path2;
            doc_file_path += docFile.cszFileName;
            if ( !NppExecHelpers::CheckFileExists(doc_file_path) )
            {
                if ( !docFile.bShouldExist )
                    continue;
            }
        }
        SendNppMsg( NPPM_DOOPEN, 0, (LPARAM) doc_file_path.c_str() );
    }
}

void CNppExec::OnHelpAbout()
{
    PluginDialogBox(IDD_HELP_ABOUT, HelpAboutDlgProc);
}

void CNppExec::OnUserMenuItem(int nItemNumber)
{
    if ( nItemNumber >= 0 && nItemNumber < g_nUserMenuItems )
    {
        int n = 0;
        for ( int i = 0; i < MAX_USERMENU_ITEMS; i++ )
        {
            int len = 0;
            const TCHAR* psz = GetOptions().GetStr(OPTS_USERMENU_ITEM01 + i, &len);
            if ( len > 0 )
            {
                if ( nItemNumber == n++ )
                {
                    len = c_base::_tstr_unsafe_rfind(psz, len, cszUserMenuItemSep);
                    if ( len > 0 )
                    {
                        len += lstrlen(cszUserMenuItemSep);
                        psz += len; // before cszUserMenuItemSep
                        psz = c_base::_tstr_unsafe_skip_tabspaces(psz);
                        if ( *psz )
                        {
                            DoExecScript(tstr(), psz, true);
                        }
                    }
                    break;
                }
            }
        }
    }
}

void CNppExec::ReadOptions()
{
  // Reading options...

  bool bCloudIni = false;
  tstr cloudIniPath = GetSettingsCloudPluginDir();
  if (!cloudIniPath.IsEmpty())
  {
    cloudIniPath += INI_FILENAME;
    if (NppExecHelpers::IsValidTextFile(cloudIniPath))
    {
      GetOptions().ReadOptions(cloudIniPath.c_str());
      bCloudIni = true;
    }
  }

  if (!bCloudIni)
  {
    GetOptions().ReadOptions(m_szIniFilePathName);
  }

  // Log File...
  tstr sLogsDir = GetOptions().GetStr(OPTS_PLUGIN_LOGSDIR);
  if ( !sLogsDir.IsEmpty() )
  {
    TCHAR buf[16];
    time_t rawTime;
    struct tm * timeInfo;
    ::time(&rawTime);
    timeInfo = ::localtime(&rawTime);

    GetMacroVars().CheckPluginMacroVars(sLogsDir);
    GetMacroVars().CheckNppMacroVars(sLogsDir);

    sLogsDir.Replace( _T('/'), _T('\\') );
    if ( !NppExecHelpers::IsFullPath(sLogsDir) )
    {
      tstr sBaseDir = _T("$(SYS.TEMP)");
      GetMacroVars().CheckPluginMacroVars(sBaseDir);
      if ( sBaseDir.IsEmpty() )
      {
        sBaseDir = getConfigPath();
      }
      if ( !sBaseDir.IsEmpty() )
      {
        if ( sBaseDir.GetLastChar() != _T('\\') )
          sBaseDir += _T('\\');
      }
      sLogsDir.Insert( 0, sBaseDir );
    }

    if ( sLogsDir.GetLastChar() != _T('\\') )
      sLogsDir += _T('\\');
    sLogsDir += PLUGIN_NAME;
    sLogsDir += _T('\\');
    buf[0] = 0;
    ::_tcsftime(buf, 15, _T("%Y_%m_%d"), timeInfo); // 2013_09_28
    sLogsDir += buf;

    NppExecHelpers::CreateDirectoryTree(sLogsDir);
    
    tstr sLogFile = sLogsDir;
    if ( sLogFile.GetLastChar() != _T('\\') )
      sLogFile += _T('\\');
    sLogFile += PLUGIN_NAME;
    sLogFile += _T("_log_");
    buf[0] = 0;
    ::_tcsftime(buf, 15, _T("%H%M"), timeInfo); // 2252
    sLogFile += buf;
    sLogFile += _T(".txt");

    Runtime::GetLogger().SetLogFile( sLogFile.c_str() );
    if ( Runtime::GetLogger().IsLogFileOpen() )
    {
        Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; CNppExec::ReadOptions - start") );
    }
  }
  
  // Verifying options' values...
  if (GetOptions().GetInt(OPTU_CHILDP_STARTUPTIMEOUT_MS) < 0)
  {
    GetOptions().SetUint(OPTU_CHILDP_STARTUPTIMEOUT_MS, DEFAULT_CHILDP_STARTUPTIMEOUT_MS);
  }
  if (GetOptions().GetInt(OPTU_CHILDP_CYCLETIMEOUT_MS) < 0)
  {
    GetOptions().SetUint(OPTU_CHILDP_CYCLETIMEOUT_MS, DEFAULT_CHILDP_CYCLETIMEOUT_MS);
  }
  if (GetOptions().GetInt(OPTU_CHILDP_KILLTIMEOUT_MS) < 0)
  {
    GetOptions().SetUint(OPTU_CHILDP_KILLTIMEOUT_MS, DEFAULT_CHILDP_KILLTIMEOUT_MS);
  }
  if (GetOptions().GetInt(OPTU_CHILDP_RUNPOLICY) < 0)
  {
    GetOptions().SetUint(OPTU_CHILDP_RUNPOLICY, DEFAULT_CHILDP_RUNPOLICY);
  }
  if (GetOptions().GetInt(OPTU_CHILDS_SYNCTIMEOUT_MS) < 0)
  {
    GetOptions().SetUint(OPTU_CHILDS_SYNCTIMEOUT_MS, DEFAULT_CHILDS_SYNCTIMEOUT_MS);
  }
  {
    int nChildProcExitTimeout = GetOptions().GetInt(OPTU_CHILDP_EXITTIMEOUT_MS);
    if (nChildProcExitTimeout < 0)
    {
      nChildProcExitTimeout = DEFAULT_CHILDP_EXITTIMEOUT_MS;
      GetOptions().SetUint(OPTU_CHILDP_EXITTIMEOUT_MS, nChildProcExitTimeout);
    }
      
    int nExitScriptTimeout = GetOptions().GetInt(OPTU_EXITS_TIMEOUT_MS);
    if (nExitScriptTimeout < nChildProcExitTimeout)
    {
        nExitScriptTimeout = nChildProcExitTimeout;
        GetOptions().SetUint(OPTU_EXITS_TIMEOUT_MS, nExitScriptTimeout);
    }
  }
  if (GetOptions().GetInt(OPTI_CMDHISTORY_MAXITEMS) < 2)
  {
    GetOptions().SetInt(OPTI_CMDHISTORY_MAXITEMS, DEFAULT_CMDHISTORY_MAXITEMS);
  }
  if (GetOptions().GetInt(OPTI_EXEC_MAXCOUNT) < 2)
  {
    GetOptions().SetInt(OPTI_EXEC_MAXCOUNT, DEFAULT_EXEC_MAXCOUNT);
  }
  if (GetOptions().GetInt(OPTI_GOTO_MAXCOUNT) < 2)
  {
    GetOptions().SetInt(OPTI_GOTO_MAXCOUNT, DEFAULT_GOTO_MAXCOUNT);
  }
  if (GetOptions().GetInt(OPTI_RICHEDIT_MAXTEXTLEN) < 0x10000)
  {
    GetOptions().SetInt(OPTI_RICHEDIT_MAXTEXTLEN, 0x10000);
  }
  if (GetOptions().GetInt(OPTI_SENDMSG_MAXBUFLEN) < 0x10000)
  {
    GetOptions().SetInt(OPTI_SENDMSG_MAXBUFLEN, 0x10000);
  }

  _consoleFont = NULL;
  int dataSize = 0;
  const LOGFONT* pLogFont = (const LOGFONT *) GetOptions().GetData(OPTD_CONSOLE_FONT, &dataSize);
  if ( pLogFont && (dataSize >= sizeof(LOGFONT)) )
  {
    _consoleFont = CreateFontIndirect(pLogFont);
  }

  dataSize = 0;
  c_base::byte_t* pData = (c_base::byte_t *) GetOptions().GetData(OPTD_COLOR_TEXTNORM, &dataSize);
  if ( (dataSize == 3) || (dataSize == 4) )
  {
    g_colorTextNorm = RGB( pData[0], pData[1], pData[2] );
  }

  dataSize = 0;
  pData = (c_base::byte_t *) GetOptions().GetData(OPTD_COLOR_TEXTERR, &dataSize);
  if ( (dataSize == 3) || (dataSize == 4) )
  {
    g_colorTextErr = RGB( pData[0], pData[1], pData[2] );
  }

  dataSize = 0;
  pData = (c_base::byte_t *) GetOptions().GetData(OPTD_COLOR_TEXTMSG, &dataSize);
  if ( (dataSize == 3) || (dataSize == 4) )
  {
    g_colorTextMsg = RGB( pData[0], pData[1], pData[2] );
  }

  dataSize = 0;
  pData = (c_base::byte_t *) GetOptions().GetData(OPTD_COLOR_BKGND, &dataSize);
  if ( (dataSize == 3) || (dataSize == 4) )
  {
    g_colorBkgnd = RGB( pData[0], pData[1], pData[2] );
  }
  
    for ( int i = 0; i < CConsoleOutputFilterDlg::RECOGNITION_ITEMS; i++ )
    {
        const TCHAR* cszMask = GetOptions().GetStr(OPTS_CONFLTR_RCGNMSK1 + i);
        if ( cszMask && cszMask[0] )
        {
            GetWarningAnalyzer().SetMask( i, cszMask );
            const tstr eff = GetOptions().GetStr(OPTS_CONFLTR_RCGNEFF1 + i);
            if ( eff.length() > 0 )
            {
                CWarningAnalyzer::TEffect Effect;
                
                Effect.Enable     = ( eff.GetAt(0) == '+' ? true : false );
                Effect.Italic     = ( eff.GetAt(12) == '+' ? true : false );
                Effect.Bold       = ( eff.GetAt(15) == '+' ? true : false );
                Effect.Underlined = ( eff.GetAt(18) == '+' ? true : false );
                Effect.Red        = CWarningAnalyzer::xtou( eff.GetAt(3), eff.GetAt(4) );
                Effect.Green      = CWarningAnalyzer::xtou( eff.GetAt(6), eff.GetAt(7) );
                Effect.Blue       = CWarningAnalyzer::xtou( eff.GetAt(9), eff.GetAt(10) );
                
                GetWarningAnalyzer().SetEffect( i, Effect );
            }  
        }
    }

  for ( int i = 0; i < CConsoleOutputFilterDlg::HISTORY_ITEMS; i++ )
  {
    tstr s1 = GetOptions().GetStr(OPTS_FILTERS_EXCL1 + i);
    if ( s1.length() > 0 )
    {
      if ( !ConsoleOutputFilterDlg.m_ExcludeHistory.FindExact(s1) )
        ConsoleOutputFilterDlg.m_ExcludeHistory.Add(s1);
    }
    s1 = GetOptions().GetStr(OPTS_FILTERS_INCL1 + i);
    if ( s1.length() > 0 )
    {
      if ( !ConsoleOutputFilterDlg.m_IncludeHistory.FindExact(s1) )
        ConsoleOutputFilterDlg.m_IncludeHistory.Add(s1);
    }
    s1 = GetOptions().GetStr(OPTS_FILTERS_R_FIND1 + i);
    if ( s1.length() > 0 )
    {
      if ( !ConsoleOutputFilterDlg.m_RFindHistory.FindExact(s1) )
        ConsoleOutputFilterDlg.m_RFindHistory.Add(s1);
    }
    s1 = GetOptions().GetStr(OPTS_FILTERS_R_RPLC1 + i);
    if ( s1.length() > 0 )
    {
      if ( !ConsoleOutputFilterDlg.m_RRplcHistory.FindExact(s1) )
        ConsoleOutputFilterDlg.m_RRplcHistory.Add(s1);
    }
    s1 = GetOptions().GetStr(OPTS_FILTERS_HGLT1 + i);
    if ( s1.length() > 0 )
    {
      if ( !ConsoleOutputFilterDlg.m_HighlightHistory.FindExact(s1) )
        ConsoleOutputFilterDlg.m_HighlightHistory.Add(s1);
    }
  }

  for ( int i = 0; i < CInputBoxDlg::HISTORY_ITEMS; i++ )
  {
    tstr s1 = GetOptions().GetStr(OPTS_INPUTBOX_VALUE1 + i);
    if ( s1.length() > 0 )
    {
      if ( !InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_INPUTBOX].FindExact(s1) )
        InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_INPUTBOX].Add(s1);
    }
    s1 = GetOptions().GetStr(OPTS_EXITBOX_VALUE1 + i);
    if ( s1.length() > 0 )
    {
      if ( !InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_EXITPROCESS].FindExact(s1) )
        InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_EXITPROCESS].Add(s1);
    }
  }
  
  const TCHAR* cszHotKey = GetOptions().GetStr(OPTS_PLUGIN_HOTKEY);
  if ( lstrlen(cszHotKey) > 0 )
  {
    unsigned int nHotKey = VK_F6;

    if ( lstrcmpi(cszHotKey, _T("F1")) == 0 )
      nHotKey = VK_F1;
    else if ( lstrcmpi(cszHotKey, _T("F2")) == 0 )
      nHotKey = VK_F2;
    else if ( lstrcmpi(cszHotKey, _T("F3")) == 0 )
      nHotKey = VK_F3;
    else if ( lstrcmpi(cszHotKey, _T("F4")) == 0 )
      nHotKey = VK_F4;
    else if ( lstrcmpi(cszHotKey, _T("F5")) == 0 )
      nHotKey = VK_F5;
    else if ( lstrcmpi(cszHotKey, _T("F6")) == 0 )
      nHotKey = VK_F6;
    else if ( lstrcmpi(cszHotKey, _T("F7")) == 0 )
      nHotKey = VK_F7;
    else if ( lstrcmpi(cszHotKey, _T("F8")) == 0 )
      nHotKey = VK_F8;
    else if ( lstrcmpi(cszHotKey, _T("F9")) == 0 )
      nHotKey = VK_F9;
    else if ( lstrcmpi(cszHotKey, _T("F10")) == 0 )
      nHotKey = VK_F10;
    else if ( lstrcmpi(cszHotKey, _T("F11")) == 0 )
      nHotKey = VK_F11;
    else if ( lstrcmpi(cszHotKey, _T("F12")) == 0 )
      nHotKey = VK_F12;

    GetOptions().SetUint(OPTU_PLUGIN_HOTKEY, nHotKey);
  }

  if (GetOptions().GetInt(OPTI_CONSOLE_VISIBLE) < 0)
  {
    GetOptions().SetInt(OPTI_CONSOLE_VISIBLE, CAdvOptDlg::CON_AUTO);
  }
  if (GetOptions().GetInt(OPTB_CONSOLE_SHOWHELP) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_SHOWHELP, false);
  }
  if (GetOptions().GetInt(OPTU_CONSOLE_ENCODING) < 0)
  {
    GetOptions().SetUint(OPTU_CONSOLE_ENCODING, CConsoleEncodingDlg::defaultEncodingOption());
  }
  if (GetOptions().GetInt(OPTB_CONSOLE_SAVEONEXECUTE) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_SAVEONEXECUTE, false);
  }
  if (GetOptions().GetInt(OPTB_CONSOLE_CMDHISTORY) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_CMDHISTORY, true);
  }
  if (GetOptions().GetInt(OPTB_CONSOLE_SAVECMDHISTORY) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_SAVECMDHISTORY, true);
  }
#ifdef _SCROLL_TO_LATEST  
  if (GetOptions().GetInt(OPTB_CONSOLE_SCROLL2LATEST) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_SCROLL2LATEST, true);
  }
#endif
  if (GetOptions().GetInt(OPTB_CONSOLE_NOINTMSGS) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_NOINTMSGS, false);
  }
  if (GetOptions().GetInt(OPTB_CONSOLE_PRINTMSGREADY) < 0)
  {
    GetOptions().SetBool(OPTB_CONSOLE_PRINTMSGREADY, true);
  }

  int nAnsiEscSeq = GetOptions().GetInt(OPTI_CONSOLE_ANSIESCSEQ);
  if ( (nAnsiEscSeq < 0) || (nAnsiEscSeq >= CChildProcess::escTotalCount) )
  {
    GetOptions().SetInt(OPTI_CONSOLE_ANSIESCSEQ, CChildProcess::escKeepRaw);
  }

  {
    tstr sKeyEnter = GetOptions().GetStr(OPTS_KEY_ENTER);
    if (sKeyEnter.IsEmpty())
      sKeyEnter = DEFAULT_KEY_ENTER;
    sKeyEnter.Replace(_T("\\n"), _T("\n"));
    sKeyEnter.Replace(_T("\\r"), _T("\r"));
    GetOptions().SetStr(OPTS_KEY_ENTER, sKeyEnter.c_str());
  }

  {
    int nNulCharSize = 0;
    const void* pNulCharData = GetOptions().GetData(OPTD_CONSOLE_NULCHAR, &nNulCharSize);

#ifdef UNICODE
    wchar_t wch = 0;
    if ( pNulCharData && (nNulCharSize >= (int) sizeof(wchar_t)) )
    {
      wch = *((const wchar_t *) pNulCharData);
      c_base::str_unsafe_reversen( (char *) &wch, (int) (sizeof(wchar_t)/sizeof(char)) ); // reverse byte order
    }
    else if ( pNulCharData && (nNulCharSize >= (int) sizeof(char)) )
    {
      char ch = *((const char *) pNulCharData);
      wch = ch;
    }
    else
    {
      wch = DEFAULT_NULCHAR_UNICODE;
    }
    GetOptions().SetData(OPTD_CONSOLE_NULCHAR, &wch, (int) sizeof(wchar_t));
#else
    char ch = 0;
    if ( pNulCharData && (nNulCharSize >= (int) sizeof(char)) )
    {
      ch = *((const char *) pNulCharData);
    }
    else
    {
      ch = DEFAULT_NULCHAR_ANSI;
    }
    GetOptions().SetData(OPTD_CONSOLE_NULCHAR, &ch, (int) sizeof(char));
#endif
  }

  const tstr pathToTempScript = ExpandToFullConfigPath(SCRIPTFILE_TEMP, true);
  CFileBufT<TCHAR> fbuf;
  if (fbuf.LoadFromFile(pathToTempScript.c_str(), true, GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH)))
  {
    tstr Line;

    while (fbuf.GetLine(Line) >= 0)
    {
      m_TempScript.GetCmdList().Add(Line);
    } 
  }

  const tstr pathToSavedScripts = ExpandToFullConfigPath(SCRIPTFILE_SAVED, true);
  m_ScriptsList.LoadFromFile(pathToSavedScripts.c_str(), GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH));
  g_scriptFileChecker.AssignFile(pathToSavedScripts.c_str());
  /*
  const tstr pathToCloudSavedScripts = ExpandToFullConfigPath(SCRIPTFILE_SAVED, true);
  const tstr pathToLocalSavedScripts = ExpandToFullConfigPath(SCRIPTFILE_SAVED);
  if (pathToCloudSavedScripts != pathToLocalSavedScripts)
  {
    FILETIME cloudWriteTime;
    FILETIME localWriteTime;

    ::ZeroMemory(&cloudWriteTime, sizeof(FILETIME));
    ::ZeroMemory(&localWriteTime, sizeof(FILETIME));

    NppExecHelpers::GetFileWriteTime(pathToCloudSavedScripts.c_str(), &cloudWriteTime);
    NppExecHelpers::GetFileWriteTime(pathToLocalSavedScripts.c_str(), &localWriteTime);

    if (::CompareFileTime(&cloudWriteTime, &localWriteTime) > 0)
    {
      ::CopyFile(pathToCloudSavedScripts.c_str(), pathToLocalSavedScripts.c_str(), FALSE);
    }
  }
  m_ScriptsList.LoadFromFile(pathToLocalSavedScripts.c_str(), GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH));
  g_scriptFileChecker.AssignFile(pathToLocalSavedScripts.c_str());
  */

  if ( Runtime::GetLogger().IsLogFileOpen() )
  {
    Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; CNppExec::ReadOptions - end") );
  }

  StartAutoSaveTimer();
}

void CNppExec::StartAutoSaveTimer()
{
  if (m_idTimerAutoSave == 0)
  {
    UINT uTimeoutInMilliseconds = GetOptions().GetUint(OPTU_PLUGIN_AUTOSAVE_SECONDS)*1000;
    if (uTimeoutInMilliseconds != 0) // auto-save enabled
    {
      const UINT minTimeout = 10*1000; // 10 seconds

      if (uTimeoutInMilliseconds < minTimeout)
        uTimeoutInMilliseconds = minTimeout;
      m_idTimerAutoSave = ::SetTimer(NULL, 0, uTimeoutInMilliseconds, OnTimer_AutoSaveProc);
    }
  }
}

void CNppExec::StopAutoSaveTimer()
{
  if (m_idTimerAutoSave != 0)
  {
    ::KillTimer(NULL, m_idTimerAutoSave);
    m_idTimerAutoSave = 0;
  }
}

void CALLBACK CNppExec::OnTimer_AutoSaveProc(HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/)
{
  Runtime::GetNppExec().SaveOptions();
}

DWORD WINAPI CNppExec::ExitScriptTimeoutThreadProc(LPVOID lpParam)
{
  // Note: the Console output is already disabled (see the NPPN_SHUTDOWN handler)
    
  CNppExec* pNppExec = (CNppExec*) lpParam;
  unsigned int uExitScriptTimeout = pNppExec->GetOptions().GetUint(OPTU_EXITS_TIMEOUT_MS);
  if ( pNppExec->m_ExitScriptIsDone.Wait(uExitScriptTimeout) != WAIT_OBJECT_0 )
  {
    Runtime::GetLogger().Add_WithoutOutput( _T("; Exit Script - timeout, stopping...") );

    pNppExec->_consoleIsVisible = false; // stopping every script (there should be just the exit one, though)...
    pNppExec->_bStopTheExitScript = true; // ...especially the exit script
    std::shared_ptr<CScriptEngine> pScriptEngine = pNppExec->GetCommandExecutor().GetRunningScriptEngine();
    if ( pScriptEngine )
    {
      pScriptEngine->ScriptError( CScriptEngine::ET_ABORT, _T("; Exiting, stopping the script (from ExitScriptTimeout())") );
    }
  }
  return 0;
}

void CNppExec::CreateCloudDirIfNeeded(const tstr& cloudDir)
{
  if (!NppExecHelpers::CheckDirectoryExists(cloudDir))
  {
    const tstr parentDir = NppExecHelpers::GetFileNamePart(cloudDir, NppExecHelpers::fnpDirPath);
    if (!NppExecHelpers::CheckDirectoryExists(parentDir))
    {
      ::CreateDirectory(parentDir.c_str(), NULL);
    }
    ::CreateDirectory(cloudDir.c_str(), NULL);
  }
}

void CNppExec::SaveConfiguration()
{
  const int nWrittenToIni = GetOptions().SaveOptions(m_szIniFilePathName);

  // Cloud settings...
  const tstr cloudPluginDir = GetSettingsCloudPluginDir();
  if (!cloudPluginDir.IsEmpty())
  {
    CreateCloudDirIfNeeded(cloudPluginDir);

    // Back up the .ini file to the cloud
    tstr cloudIniPath = cloudPluginDir;
    cloudIniPath += INI_FILENAME;
    if ((nWrittenToIni > 0) || !NppExecHelpers::IsValidTextFile(cloudIniPath))
    {
      ::CopyFile(m_szIniFilePathName, cloudIniPath.c_str(), FALSE);
    }
  }
}

void CNppExec::SaveOptions()
{
  if ( m_isSavingOptions )
    return;

  {
    CCriticalSectionLockGuard lock(m_csAutoSave);

    m_isSavingOptions = true;

    if ( Runtime::GetLogger().IsLogFileOpen() )
    {
      Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; CNppExec::SaveOptions - start") );
    }

    for ( int i = 0; i < CConsoleOutputFilterDlg::RECOGNITION_ITEMS; i++ )
    {
        TCHAR strRcgnMask[OUTPUTFILTER_BUFSIZE];

        if ( *GetWarningAnalyzer().GetMask( i, strRcgnMask, OUTPUTFILTER_BUFSIZE - 1 ) != 0 )
        {
            CWarningAnalyzer::TEffect Effect;
            TCHAR strEffect[32];

            GetWarningAnalyzer().GetEffect( i, Effect );
            wsprintf( strEffect
                    , _T("%cE %c%c %c%c %c%c %cI %cB %cU")
                    , ( Effect.Enable ? '+' : '-' )                                                               
                    , ( ( (Effect.Red>>4   )&0xF ) > 9 ? 'A' + ( (Effect.Red>>4   )&0xF ) - 0xA : '0' + ( ( Effect.Red>>4   )&0xF ) )
                    , ( ( (Effect.Red      )&0xF ) > 9 ? 'A' + ( (Effect.Red      )&0xF ) - 0xA : '0' + ( ( Effect.Red      )&0xF ) )
                    , ( ( (Effect.Green>>4 )&0xF ) > 9 ? 'A' + ( (Effect.Green>>4 )&0xF ) - 0xA : '0' + ( ( Effect.Green>>4 )&0xF ) )
                    , ( ( (Effect.Green    )&0xF ) > 9 ? 'A' + ( (Effect.Green    )&0xF ) - 0xA : '0' + ( ( Effect.Green    )&0xF ) )
                    , ( ( (Effect.Blue>>4  )&0xF ) > 9 ? 'A' + ( (Effect.Blue>>4  )&0xF ) - 0xA : '0' + ( ( Effect.Blue>>4  )&0xF ) )
                    , ( ( (Effect.Blue     )&0xF ) > 9 ? 'A' + ( (Effect.Blue     )&0xF ) - 0xA : '0' + ( ( Effect.Blue     )&0xF ) )
                    , ( Effect.Italic ? '+' : '-' )                                                               
                    , ( Effect.Bold ? '+' : '-' )
                    , ( Effect.Underlined ? '+' : '-' )
                    );
            GetOptions().SetStr(OPTS_CONFLTR_RCGNMSK1 + i, strRcgnMask);
            GetOptions().SetStr(OPTS_CONFLTR_RCGNEFF1 + i, strEffect);
        }
        else
        {
            GetOptions().SetStr(OPTS_CONFLTR_RCGNMSK1 + i, _T(""));
        }
    }

    CListItemT<tstr>* p1 = ConsoleOutputFilterDlg.m_ExcludeHistory.GetFirst();
    CListItemT<tstr>* p2 = ConsoleOutputFilterDlg.m_IncludeHistory.GetFirst();
    CListItemT<tstr>* p3 = ConsoleOutputFilterDlg.m_RFindHistory.GetFirst();
    CListItemT<tstr>* p4 = ConsoleOutputFilterDlg.m_RRplcHistory.GetFirst();
    CListItemT<tstr>* p5 = ConsoleOutputFilterDlg.m_HighlightHistory.GetFirst();
    for ( int i = 0; i < CConsoleOutputFilterDlg::HISTORY_ITEMS; i++ )
    {
      if ( p1 )
      {
        if ( p1->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_FILTERS_EXCL1 + i, p1->GetItem().c_str() );
        p1 = p1->GetNext();
      }
      if ( p2 )
      {
        if ( p2->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_FILTERS_INCL1 + i, p2->GetItem().c_str() );
        p2 = p2->GetNext();
      }
      if ( p3 )
      {
        if ( p3->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_FILTERS_R_FIND1 + i, p3->GetItem().c_str() );
        p3 = p3->GetNext();
      }
      if ( p4 )
      {
        if ( p4->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_FILTERS_R_RPLC1 + i, p4->GetItem().c_str() );
        p4 = p4->GetNext();
      }
      if ( p5 )
      {
        if ( p5->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_FILTERS_HGLT1 + i, p5->GetItem().c_str() );
        p5 = p5->GetNext();
      }
    }

    CListItemT<tstr>* p6 = InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_INPUTBOX].GetFirst();
    CListItemT<tstr>* p7 = InputBoxDlg.m_InputHistory[CInputBoxDlg::IBT_EXITPROCESS].GetFirst();
    for ( int i = 0; i < CInputBoxDlg::HISTORY_ITEMS; i++ )
    {
      if ( p6 )
      {
        if ( p6->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_INPUTBOX_VALUE1 + i, p6->GetItem().c_str() );
        p6 = p6->GetNext();
      }
      if ( p7 )
      {
        if ( p7->GetItem().length() > 0 )
          GetOptions().SetStr( OPTS_EXITBOX_VALUE1 + i, p7->GetItem().c_str() );
        p7 = p7->GetNext();
      }
    }

    //if ( GetOptions().GetUint(OPTU_PLUGIN_HOTKEY) != GetOptions().GetUint(OPTU_PLUGIN_HOTKEY, false) )
    // Incorrect comparison because initial value 
    // of OPTU_PLUGIN_HOTKEY is always VK_F6.
    {
      const unsigned int nHotKey = GetOptions().GetUint(OPTU_PLUGIN_HOTKEY);  
      TCHAR szHotKey[8];
        
      if ( nHotKey == VK_F1 )
        lstrcpy(szHotKey, _T("F1"));
      else if ( nHotKey == VK_F2 )
        lstrcpy(szHotKey, _T("F2"));
      else if ( nHotKey == VK_F3 )
        lstrcpy(szHotKey, _T("F3"));
      else if ( nHotKey == VK_F4 )
        lstrcpy(szHotKey, _T("F4"));
      else if ( nHotKey == VK_F5 )
        lstrcpy(szHotKey, _T("F5"));
      else if ( nHotKey == VK_F6 )
        lstrcpy(szHotKey, _T("F6"));
      else if ( nHotKey == VK_F7 )
        lstrcpy(szHotKey, _T("F7"));
      else if ( nHotKey == VK_F8 )
        lstrcpy(szHotKey, _T("F8"));
      else if ( nHotKey == VK_F9 )
        lstrcpy(szHotKey, _T("F9"));
      else if ( nHotKey == VK_F10 )
        lstrcpy(szHotKey, _T("F10"));
      else if ( nHotKey == VK_F11 )
        lstrcpy(szHotKey, _T("F11"));
      else if ( nHotKey == VK_F12 )
        lstrcpy(szHotKey, _T("F12"));
      else
        lstrcpy(szHotKey, _T("F6"));

      GetOptions().SetStr(OPTS_PLUGIN_HOTKEY, szHotKey);
    }

    SaveConfiguration();
    SaveScripts();
    ConsoleDlg::SaveCmdHistory();

    if ( Runtime::GetLogger().IsLogFileOpen() )
    {
      Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; CNppExec::SaveOptions - end") );
    }

    m_isSavingOptions = false;
  }
}

void CNppExec::SaveScripts()
{
  enum eSavedScriptFlags {
    ssfTemp    = 0x01,
    ssfScripts = 0x02
  };

  CListItemT<tstr>* p;
  CFileBufT<TCHAR>  fbuf;
  tstr              Line;
  unsigned int      nSaved = 0;
  tstr              localTempScript = ExpandToFullConfigPath(SCRIPTFILE_TEMP);
  tstr              localSavedScripts = ExpandToFullConfigPath(SCRIPTFILE_SAVED);

  if (m_TempScriptIsModified)
  {
    fbuf.GetBufPtr()->Clear();
    p = m_TempScript.GetCmdList().GetFirst();
    while (p)
    {
      Line = p->GetItem();
      if (p != m_TempScript.GetCmdList().GetLast())
        Line.Append(_T("\r\n"), 2);
      fbuf.GetBufPtr()->Append(Line.c_str(), Line.length());
      p = p->GetNext();
    }
    fbuf.SaveToFile(localTempScript.c_str());
    m_TempScriptIsModified = false;
    nSaved |= ssfTemp;
  }

  if (m_ScriptsList.IsModified())
  {
    m_ScriptsList.SaveToFile(localSavedScripts.c_str());
    nSaved |= ssfScripts;
  }

  // Cloud settings...
  const tstr cloudPluginDir = GetSettingsCloudPluginDir();
  if (!cloudPluginDir.IsEmpty())
  {
    CreateCloudDirIfNeeded(cloudPluginDir);

    // Back up the temp script to the cloud
    tstr cloudTempScript = cloudPluginDir;
    cloudTempScript += SCRIPTFILE_TEMP;
    if ((nSaved & ssfTemp) || !NppExecHelpers::IsValidTextFile(cloudTempScript))
    {
      ::CopyFile(localTempScript.c_str(), cloudTempScript.c_str(), FALSE);
    }

    // Back up the saved scripts to the cloud
    tstr cloudSavedScripts = cloudPluginDir;
    cloudSavedScripts += SCRIPTFILE_SAVED;
    if ((nSaved & ssfScripts) || !NppExecHelpers::IsValidTextFile(cloudSavedScripts))
    {
      ::CopyFile(localSavedScripts.c_str(), cloudSavedScripts.c_str(), FALSE);
      /*
      FILETIME writeTime;
      if (NppExecHelpers::GetFileWriteTime(localSavedScripts.c_str(), &writeTime))
      {
        NppExecHelpers::SetFileWriteTime(cloudSavedScripts.c_str(), &writeTime);
      }
      */
    }
  }

  if (nSaved & ssfScripts)
  {
    g_scriptFileChecker.UpdateFileInfo();
  }
}

HWND CNppExec::GetScintillaHandle()
{
  INT currentEdit;
  SendNppMsg(NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
  return getCurrentScintilla(currentEdit);
}

#define NOTEPADPLUS_USER_INTERNAL  (WM_USER + 0000)
#define NPPM_INTERNAL_GETMENU      (NOTEPADPLUS_USER_INTERNAL + 14)

HMENU CNppExec::GetNppMainMenu()
{
    HMENU hMenu;

    // Notepad++ ver > 6.3
    hMenu = (HMENU) SendNppMsg(NPPM_GETMENUHANDLE, 1, 0);
    if ( hMenu && IsMenu(hMenu) )
        return hMenu;

    // Notepad++ ver <= 6.3
    hMenu = (HMENU) SendNppMsg(NPPM_INTERNAL_GETMENU, 0, 0);
    if ( hMenu && IsMenu(hMenu) )
        return hMenu;

    return ::GetMenu(m_nppData._nppHandle);
}

void CNppExec::SetConsoleFont(HWND hEd, const LOGFONT* plf)
{
  if (_consoleFont)
    ::DeleteObject(_consoleFont);
  GetOptions().SetData(OPTD_CONSOLE_FONT, plf, sizeof(LOGFONT));
  _consoleFont = CreateFontIndirect(plf);
  if (hEd && _consoleFont)
  {
    GetConsole().GetConsoleEdit().SetFont(plf);
    GetConsole().UpdateColours();
  }
}

void CNppExec::setConsoleVisible(bool consoleIsVisible)
{
  _consoleIsVisible = consoleIsVisible;

  SendNppMsg(NPPM_SETMENUITEMCHECK, 
    g_funcItem[N_SHOWCONSOLE]._cmdID, (LPARAM) consoleIsVisible);
  /*
  HMENU hMenu = GetNppMainMenu();
  if (hMenu)
  {
    EnableMenuItem(hMenu, g_funcItem[N_SHOWCONSOLE]._cmdID,
      MF_BYCOMMAND | (_consoleIsVisible ? MF_GRAYED : MF_ENABLED) );
  }
  */
}

CNppExec::eDlgExistResult CNppExec::verifyConsoleDialogExists()
{
    // NOTE:
    // This dialog _must_ be created in the main thread.
    // If it is created in another thread, Notepad++ may
    // hang after trying to dock it.

    eDlgExistResult result = dlgNotExist;
    HWND hConsoleParentWnd = GetConsole().GetDialogWnd();

    if ( !hConsoleParentWnd )
    {
        hConsoleParentWnd = CreateDialog(
          (HINSTANCE) m_hDllModule,
          MAKEINTRESOURCE(IDD_CONSOLE),
          m_nppData._nppHandle,
          ConsoleDlgProc);

        GetConsole().SetDialogWnd(hConsoleParentWnd);
        
        result = (hConsoleParentWnd != NULL) ? dlgJustCreated : dlgNotExist;
    }
    else
    {
        result = dlgExistedAlready;
    }

    if ( hConsoleParentWnd )
    {
        GetConsole().SetConsoleWnd( GetDlgItem(hConsoleParentWnd, IDC_RE_CONSOLE) );

        //::ShowWindow(hConsoleParentWnd, SW_HIDE);
    }

    return result;
}

bool CNppExec::isConsoleDialogVisible()
{
    if ( GetConsole().GetDialogWnd() )
        return ::IsWindowVisible(GetConsole().GetDialogWnd()) ? true : false;
    else
        return _consoleIsVisible;
}

bool CNppExec::initConsoleDialog()
{
    // NOTE:
    // I'd prefer to call ConsoleDlg::DockDialog() here, but the ConsoleDlg
    // has a toolbar icon and it leads to the following call in Notepad++'s
    // code (in DockingCont::viewToolbar):
    //     // show dialog and notify parent to update dialog view
    //     if (isVisible() == false)
    //     {
    //         this->doDialog();
    //         ::SendMessage(_hParent, WM_SIZE, 0, 0);
    //     }
    // Apparently this makes the ConsoleDlg visible while it should not be
    // visible yet.

    /*
    bool bResult = false;
    switch ( verifyConsoleDialogExists() )
    {
        case dlgJustCreated:
            //ConsoleDlg::DockDialog();
            bResult = true;
            break;
        case dlgExistedAlready:
            bResult = true;
            break;
    }
    return bResult;
    */
    return (verifyConsoleDialogExists() != dlgNotExist);
}

bool CNppExec::checkCmdListAndPrepareConsole(const CListT<tstr>& CmdList, bool bCanClearConsole)
{
    tstr S;
    bool bExecute = false;
    const CListItemT<tstr>* p = CmdList.GetFirst();

    while ( p && !bExecute )
    {
        if ( p->GetItem().length() > 0 )
        {
            S = p->GetItem();
            if ( !CScriptEngine::isCommentOrEmpty(this, S) )
            {
                bExecute = true;
            }
        }
        p = p->GetNext();
    }
     
    if ( bExecute )
    {
        const CScriptEngine::eCmdType cmdType = CScriptEngine::getCmdType(this, S, CScriptEngine::ctfUseLogging | CScriptEngine::ctfIgnorePrefix);
        if ( (cmdType == CScriptEngine::CMDTYPE_NPPCONSOLE) ||
             (cmdType == CScriptEngine::CMDTYPE_NPPEXEC) )
        {
            if ( verifyConsoleDialogExists() != CNppExec::dlgNotExist )  _consoleIsVisible = true;
        }
        else
        {
            showConsoleDialog(CNppExec::showIfHidden, 0);
        }

        if ( GetConsole().GetDialogWnd() ) 
        {
            if ( bCanClearConsole && !GetOptions().GetBool(OPTB_CONSOLE_APPENDMODE) )
            {
                if (!GetCommandExecutor().GetRunningScriptEngine()) // a collateral script may be running
                {
                    GetConsole().ClearText();
                }
            }
            return true;
        }
    }

    return false;
}

void CNppExec::showConsoleDialog(eShowConsoleAction showAction, unsigned int nShowFlags)
{
  if ((showAction != hideAlways) && !isConsoleDialogVisible())
  {
    if (initConsoleDialog()) 
    {
      ConsoleDlg::DockDialog();
      SendNppMsg(NPPM_DMMSHOW, 0, (LPARAM) GetConsole().GetDialogWnd());
      setConsoleVisible(true);
    }
  }
  else if ((showAction == hideIfShown) || (showAction == hideAlways))
  {
    if ( GetConsole().GetDialogWnd() )
    {
      ::SetWindowLongPtr(GetConsole().GetDialogWnd(), DWLP_MSGRESULT, 0L);
      if ( (nShowFlags & scfCmdNppConsole) != 0 )
        ::SendMessage(GetConsole().GetDialogWnd(), WM_CONSOLEDLG_CLOSE, 0, 0);
      else
        ::SendMessage(GetConsole().GetDialogWnd(), WM_SYSCOMMAND, SC_CLOSE, 0);
      if (::GetWindowLongPtr(GetConsole().GetDialogWnd(), DWLP_MSGRESULT) != 1L)
      {
        SendNppMsg(NPPM_DMMHIDE, 0, (LPARAM) GetConsole().GetDialogWnd());
        //setConsoleVisible(false);
      }
    }
  }
  else
  {
    _consoleIsVisible = true;
    if ( ConsoleDlg::EnableTransparency(false) )
      ::SetFocus( GetConsole().GetConsoleWnd() );
  }
}

void CNppExec::printConsoleHelpInfo()
{
  GetConsole().PrintMessage(SZ_CONSOLE_HELP_INFO, false);
}

void CNppExec::ShowError(LPCTSTR szMessage)
{
  ::MessageBox(m_nppData._nppHandle, szMessage, 
      _T("Notepad++ (NppExec) Error"), MB_OK | MB_ICONERROR);
}

void CNppExec::ShowWarning(LPCTSTR szMessage)
{
  ::MessageBox(m_nppData._nppHandle, szMessage, 
      _T("Notepad++ (NppExec) Warning"), MB_OK | MB_ICONWARNING);
}

INT_PTR CNppExec::PluginDialogBox(UINT idDlg, DLGPROC lpDlgProc)
{
  HWND hWndFocus = ::GetFocus();
  INT_PTR nRet = ::DialogBox( (HINSTANCE) m_hDllModule, 
      MAKEINTRESOURCE(idDlg), m_nppData._nppHandle, lpDlgProc );
  ::SetFocus(hWndFocus);
  return nRet;
}

LRESULT CNppExec::SendNppMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) // to Notepad++
{
    return ::SendMessage(m_nppData._nppHandle, uMsg, wParam, lParam);
}

//-------------------------------------------------------------------------

CNppConsoleRichEdit::CNppConsoleRichEdit() : CAnyRichEdit()
{
}

//void CNppConsoleRichEdit::AddLine(const TCHAR* cszLine, BOOL bScrollText /* = FALSE */, 
//                                  COLORREF color /* = RGB(0,0,0) */, 
//                                  DWORD dwMask /* = CFM_EFFECTS */, DWORD dwEffects /* = 0 */)
//{
//    tstr S;
//    TCHAR ch = GetNulChar();
//    if ( ch != 0 )
//    {
//        S = cszLine;
//        S.Replace( TCHAR(0), ch );
//        cszLine = S.c_str();
//    }
//
//    CAnyRichEdit::AddLine(cszLine, bScrollText, color, dwMask, dwEffects);
//}

//void CNppConsoleRichEdit::AddStr(const TCHAR* cszStr, BOOL bScrollText /* = FALSE */, 
//                                 COLORREF color /* = RGB(0,0,0) */, 
//                                 DWORD dwMask /* = CFM_EFFECTS */, DWORD dwEffects /* = 0 */)
//{
//    tstr S;
//    TCHAR ch = GetNulChar();
//    if ( ch != 0 )
//    {
//        S = cszStr;
//        S.Replace( TCHAR(0), ch );
//        cszStr = S.c_str();
//    }
//
//    CAnyRichEdit::AddStr(cszStr, bScrollText, color, dwMask, dwEffects);
//}

TCHAR CNppConsoleRichEdit::GetNulChar()
{
    TCHAR ch = 0;
    int nNulCharSize = 0;
    const TCHAR* pNulCharData = (const TCHAR *) Runtime::GetNppExec().GetOptions().GetData(OPTD_CONSOLE_NULCHAR, &nNulCharSize);
    if ( pNulCharData && (nNulCharSize == (int) sizeof(TCHAR)) )
    {
        ch = *pNulCharData;
    }
    return ch;
}

//-------------------------------------------------------------------------

CNppExecConsole::CNppExecConsole() //: m_pNppExec(0)
  : m_colorTextNorm(0)
  , m_colorTextMsg(0)
  , m_colorTextErr(0)
  , m_colorBkgnd(0)
{
    m_hDlg = NULL;
    
    m_StateList.push_back(ConsoleState());
}

CNppExecConsole::~CNppExecConsole()
{
    if ( m_StateList.size() > 1 )
    {
        // this must not happen, but if it does, let's log this at least
        const TCHAR* cszErr = _T("WARNING!!! ~CNppExecConsole(): m_StateList.size > 1");
        Runtime::GetLogger().Add_WithoutOutput( cszErr );
        ::MessageBox(NULL, cszErr, _T("NppExec"), MB_OK | MB_ICONWARNING);
    }

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( _getState(scrptEngnId).PostponedCalls.size() > 0 )
    {
        // this should not happen, but if it does, let's log this at least
        // this is not critical (unlike m_StateList.size()), so no need in MessageBox
        const TCHAR* cszErr = _T("WARNING!!! ~CNppExecConsole(): _getState().PostponedCalls.size() > 0");
        Runtime::GetLogger().Add_WithoutOutput( cszErr );
        //::MessageBox(NULL, cszErr, _T("NppExec"), MB_OK | MB_ICONWARNING);
    }

    // Everything should be fine without the following line
    // as m_StateList is created last and destroyed first...
    // but just in case :)
    m_StateList.clear();
}

//CNppExec* CNppExecConsole::GetNppExec() const
//{
//    return m_pNppExec;
//}

//void CNppExecConsole::SetNppExec(CNppExec* pNppExec)
//{
//    m_pNppExec = pNppExec;
//}

COLORREF CNppExecConsole::GetCurrentColorTextNorm() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorTextNorm;

    return _getCurrentColorTextNorm();
}

COLORREF CNppExecConsole::_getCurrentColorTextNorm() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorTextNorm;

    COLORREF colorTextNorm;
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        colorTextNorm = m_colorTextNorm;
    }

    return (colorTextNorm != 0 ? colorTextNorm : g_colorTextNorm);
}


COLORREF CNppExecConsole::GetCurrentColorTextMsg() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorTextMsg;

    return _getCurrentColorTextMsg();
}

COLORREF CNppExecConsole::_getCurrentColorTextMsg() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorTextMsg;

    COLORREF colorTextMsg;
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        colorTextMsg = m_colorTextMsg;
    }

    return (colorTextMsg != 0 ? colorTextMsg : g_colorTextMsg);
}

COLORREF CNppExecConsole::GetCurrentColorTextErr() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorTextErr;

    return _getCurrentColorTextErr();
}

COLORREF CNppExecConsole::_getCurrentColorTextErr() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorTextErr;

    COLORREF colorTextErr;
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        colorTextErr = m_colorTextErr;
    }

    return (colorTextErr != 0 ? colorTextErr : g_colorTextErr);
}

COLORREF CNppExecConsole::GetCurrentColorBkgnd() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorBkgnd;

    return _getCurrentColorBkgnd();
}

COLORREF CNppExecConsole::_getCurrentColorBkgnd() const
{
    if ( CNppExec::_bIsNppShutdown )
        return g_colorBkgnd;

    COLORREF colorBkgnd;
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        colorBkgnd = m_colorBkgnd;
    }

    return (colorBkgnd != 0 ? colorBkgnd : g_colorBkgnd);
}

bool CNppExecConsole::postponeThisCall(ScriptEngineId scrptEngnId)
{
    ConsoleState* pState;
    bool bPostpone = true;
    bool bStateHasPostponedCalls = false;

    {
        CCriticalSectionLockGuard lock(m_csStateList);
        pState = &m_StateList.back();
        // Honestly, it would be much better to use CScriptEngine's instance (i.e. "this") instead of ThreadId.
        // But it would mean passing CScriptEngine's "this" through the whole call stack... So let it be ThreadId :)
        if ( m_StateList.size() <= 2 || pState->ScrptEngnId == scrptEngnId )
        {
            bPostpone = false;
            bStateHasPostponedCalls = !pState->PostponedCalls.empty();
        }
    }

    if ( bStateHasPostponedCalls && !bPostpone )
    {
        pState->execPostponedCalls(&m_csStateList);
    }

    return bPostpone;
}

void CNppExecConsole::SetCurrentColorTextNorm(COLORREF colorTextNorm)
{
    if ( CNppExec::_bIsNppShutdown )
        return;
    
    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !postponeThisCall(scrptEngnId) )
    {
        _setCurrentColorTextNorm(colorTextNorm);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_setCurrentColorTextNorm, this, colorTextNorm);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_setCurrentColorTextNorm(COLORREF colorTextNorm)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    CCriticalSectionLockGuard lock(m_csStateList);
    m_colorTextNorm = colorTextNorm;
}

void CNppExecConsole::SetCurrentColorTextMsg(COLORREF colorTextMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !postponeThisCall(scrptEngnId) )
    {
        _setCurrentColorTextMsg(colorTextMsg);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_setCurrentColorTextMsg, this, colorTextMsg);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_setCurrentColorTextMsg(COLORREF colorTextMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    CCriticalSectionLockGuard lock(m_csStateList);
    m_colorTextMsg = colorTextMsg;
}

void CNppExecConsole::SetCurrentColorTextErr(COLORREF colorTextErr)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !postponeThisCall(scrptEngnId) )
    {
        _setCurrentColorTextErr(colorTextErr);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_setCurrentColorTextErr, this, colorTextErr);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_setCurrentColorTextErr(COLORREF colorTextErr)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    CCriticalSectionLockGuard lock(m_csStateList);
    m_colorTextErr = colorTextErr;
}

void CNppExecConsole::SetCurrentColorBkgnd(COLORREF colorBkgnd)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !postponeThisCall(scrptEngnId) )
    {
        _setCurrentColorBkgnd(colorBkgnd);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_setCurrentColorBkgnd, this, colorBkgnd);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_setCurrentColorBkgnd(COLORREF colorBkgnd)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    CCriticalSectionLockGuard lock(m_csStateList);
    m_colorBkgnd = colorBkgnd;
}

void CNppExecConsole::PrintError(LPCTSTR cszMessage, bool bLogThisMsg )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _printError(scrptEngnId, cszMessage, bLogThisMsg);
    }
    else
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        ConsoleState& state = _getState(scrptEngnId);
        auto& postponedStrings = state.PostponedStrings;
        postponedStrings.push_back( tstr(cszMessage) );
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_printError, this, scrptEngnId, postponedStrings.back().c_str(), bLogThisMsg);
        state.PostponedCalls.push_back(postponedCall);
    }
}

void CNppExecConsole::_printError(ScriptEngineId scrptEngnId, LPCTSTR cszMessage, bool bLogThisMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    m_reConsole.AddLine( cszMessage, FALSE, _getCurrentColorTextErr() ); 
    m_reConsole.AddStr( _T(""), _isScrollToEnd(), _getCurrentColorTextNorm() );
    _lockConsoleEndPos(scrptEngnId);

    if ( bLogThisMsg && Runtime::GetLogger().IsLogFileOpen() )
    {
        tstr S = _T("<ERR> ");
        S += cszMessage;
        Runtime::GetLogger().Add_WithoutOutput(S.c_str());
    }
}

void CNppExecConsole::PrintMessage(LPCTSTR cszMessage, bool bIsInternalMsg , bool bLogThisMsg )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( bIsInternalMsg && Runtime::GetNppExec().GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _printMessage(scrptEngnId, cszMessage, bIsInternalMsg, bLogThisMsg);
    }
    else
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        ConsoleState& state = _getState(scrptEngnId);
        auto& postponedStrings = state.PostponedStrings;
        postponedStrings.push_back( tstr(cszMessage) );
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_printMessage, this, scrptEngnId, postponedStrings.back().c_str(), bIsInternalMsg, bLogThisMsg);
        state.PostponedCalls.push_back(postponedCall);
    }
}

void CNppExecConsole::_printMessage(ScriptEngineId scrptEngnId, LPCTSTR cszMessage, bool /*bIsInternalMsg*/, bool bLogThisMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    m_reConsole.AddLine( cszMessage, FALSE, _getCurrentColorTextMsg() ); 
    m_reConsole.AddStr( _T(""), _isScrollToEnd(), _getCurrentColorTextNorm() );
    _lockConsoleEndPos(scrptEngnId);

    if ( bLogThisMsg && Runtime::GetLogger().IsLogFileOpen() )
    {
        tstr S = _T("<MSG> ");
        S += cszMessage;
        Runtime::GetLogger().Add_WithoutOutput(S.c_str());
    }
}

void CNppExecConsole::PrintOutput(LPCTSTR cszMessage, bool bNewLine , bool bLogThisMsg )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _printOutput(scrptEngnId, cszMessage, bNewLine, bLogThisMsg);
    }
    else
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        ConsoleState& state = _getState(scrptEngnId);
        auto& postponedStrings = state.PostponedStrings;
        postponedStrings.push_back( tstr(cszMessage) );
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_printOutput, this, scrptEngnId, postponedStrings.back().c_str(), bNewLine, bLogThisMsg);
        state.PostponedCalls.push_back(postponedCall);
    }
}

void CNppExecConsole::_printOutput(ScriptEngineId scrptEngnId, LPCTSTR cszMessage, bool bNewLine, bool bLogThisMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    long color = _getCurrentColorTextNorm();
    int  style = 0;

    CWarningAnalyzer& WarningAnalyzer = Runtime::GetNppExec().GetWarningAnalyzer();
    if ( WarningAnalyzer.match( cszMessage ) )
    {
        color = WarningAnalyzer.GetColor();
        style = WarningAnalyzer.GetStyle();
    }

    if ( bNewLine )
    {
        m_reConsole.AddLine( cszMessage, _isScrollToEnd(), color, CFM_EFFECTS, style );
    }
    else
    {
        m_reConsole.AddStr( cszMessage, _isScrollToEnd(), color, CFM_EFFECTS, style );
    }

    _lockConsoleEndPos(scrptEngnId);

    if ( bLogThisMsg && Runtime::GetLogger().IsLogFileOpen() )
    {
        tstr S = _T("<OUT> ");
        S += cszMessage;
        Runtime::GetLogger().Add_WithoutOutput(S.c_str());
        /* Runtime::GetLogger().AddEx_WithoutOutput(_T("<OUT> [%08X] %s"), GetScriptEngineId(), cszMessage); */
    }
}

void CNppExecConsole::PrintStr(LPCTSTR cszStr, bool bNewLine, bool bLogThisMsg )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _printStr(scrptEngnId, cszStr, bNewLine, bLogThisMsg);
    }
    else
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        ConsoleState& state = _getState(scrptEngnId);
        auto& postponedStrings = state.PostponedStrings;
        postponedStrings.push_back( tstr(cszStr) );
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_printStr, this, scrptEngnId, postponedStrings.back().c_str(), bNewLine, bLogThisMsg);
        state.PostponedCalls.push_back(postponedCall);
    }
}

void CNppExecConsole::_printStr(ScriptEngineId /*scrptEngnId*/, LPCTSTR cszStr, bool bNewLine, bool bLogThisMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    if ( bNewLine )
        m_reConsole.AddLine( cszStr, _isScrollToEnd(), _getCurrentColorTextNorm(), CFM_EFFECTS, 0 );
    else
        m_reConsole.AddStr( cszStr, _isScrollToEnd(), _getCurrentColorTextNorm(), CFM_EFFECTS, 0 );

    if ( bLogThisMsg && Runtime::GetLogger().IsLogFileOpen() )
    {
        tstr S = _T("<STR> ");
        S += cszStr;
        Runtime::GetLogger().Add_WithoutOutput(S.c_str());
    }
}

void CNppExecConsole::PrintSysError(LPCTSTR cszFunctionName, DWORD dwErrorCode, bool bLogThisMsg )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _printSysError(scrptEngnId, cszFunctionName, dwErrorCode, bLogThisMsg);
    }
    else
    {
        CCriticalSectionLockGuard lock(m_csStateList);
        ConsoleState& state = _getState(scrptEngnId);
        auto& postponedStrings = state.PostponedStrings;
        postponedStrings.push_back( tstr(cszFunctionName) );
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_printSysError, this, scrptEngnId, postponedStrings.back().c_str(), dwErrorCode, bLogThisMsg);
        state.PostponedCalls.push_back(postponedCall);
    }
}

void CNppExecConsole::_printSysError(ScriptEngineId scrptEngnId, LPCTSTR cszFunctionName, DWORD dwErrorCode, bool bLogThisMsg)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    if ( cszFunctionName && cszFunctionName[0] )
    {
        LPTSTR lpMsgBuf;
        TCHAR  szText[100];
  
        ::wsprintf( szText, _T("%s failed with error code %lu:"), cszFunctionName, dwErrorCode );
        ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            dwErrorCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, 
            NULL );

        _printError( scrptEngnId, szText, bLogThisMsg );
        _printError( scrptEngnId, lpMsgBuf, bLogThisMsg );

        ::LocalFree(lpMsgBuf);
    }
}

void CNppExecConsole::LockConsolePos(INT nPos, bool bForce )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( (!bForce) && (!_isOutputEnabled(scrptEngnId)) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _lockConsolePos(scrptEngnId, nPos);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_lockConsolePos, this, scrptEngnId, nPos);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_lockConsolePos(ScriptEngineId /*scrptEngnId*/, INT nPos)
{
    extern INT nConsoleFirstUnlockedPos; // this is really ugly, but the whole design of the ConsoleDlg is very bad :(

    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    nConsoleFirstUnlockedPos = nPos;
    if ( _isScrollToEnd() )
    {
        m_reConsole.ExSetSel(nConsoleFirstUnlockedPos, nConsoleFirstUnlockedPos);
    }
}

void CNppExecConsole::LockConsoleEndPos(bool bForce )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( (!bForce) && (!_isOutputEnabled(scrptEngnId)) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _lockConsoleEndPos(scrptEngnId);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_lockConsoleEndPos, this, scrptEngnId);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_lockConsoleEndPos(ScriptEngineId scrptEngnId)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    _lockConsolePos(scrptEngnId, m_reConsole.GetTextLengthEx());
}

void CNppExecConsole::LockConsoleEndPosAfterEnterPressed()
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( _isOutputEnabled(scrptEngnId) ) // <-- this is the only function that works when _isOutputEnabled() is false
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _lockConsoleEndPosAfterEnterPressed(scrptEngnId);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_lockConsoleEndPosAfterEnterPressed, this, scrptEngnId);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_lockConsoleEndPosAfterEnterPressed(ScriptEngineId scrptEngnId)
{
    if ( CNppExec::_bIsNppShutdown ) 
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    int nPos = m_reConsole.GetTextLengthEx();
    nPos += _T_RE_EOL_LEN;
    _lockConsolePos(scrptEngnId, nPos);
}

CAnyRichEdit& CNppExecConsole::GetConsoleEdit()
{
    if ( CNppExec::_bIsNppShutdown )
    {
        static CAnyRichEdit dummy;
        return dummy;
    }

    return m_reConsole;
}

HWND CNppExecConsole::GetConsoleWnd() const
{
    if ( CNppExec::_bIsNppShutdown )
        return NULL;

    return m_reConsole.m_hWnd;
}

void CNppExecConsole::SetConsoleWnd(HWND hWndRichEdit)
{
    m_reConsole.m_hWnd = hWndRichEdit;
}

HWND CNppExecConsole::GetDialogWnd() const
{
    return m_hDlg;
}

void CNppExecConsole::SetDialogWnd(HWND hDlg)
{
    m_hDlg = hDlg;
}

bool CNppExecConsole::IsOutputEnabled() const
{
    if ( CNppExec::_bIsNppShutdown )
        return false;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    return _isOutputEnabled(scrptEngnId);
}

bool CNppExecConsole::_isOutputEnabled(ScriptEngineId scrptEngnId) const
{
    if ( CNppExec::_bIsNppShutdown )
        return false;

    CCriticalSectionLockGuard lock(m_csStateList);
    return (_getState(scrptEngnId).nOutputEnabled != 0);
}

int CNppExecConsole::IsOutputEnabledN() const
{
    if ( CNppExec::_bIsNppShutdown )
        return 0;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    return _isOutputEnabledN(scrptEngnId);
}

int CNppExecConsole::_isOutputEnabledN(ScriptEngineId scrptEngnId) const
{
    if ( CNppExec::_bIsNppShutdown )
        return 0;

    CCriticalSectionLockGuard lock(m_csStateList);
    return _getState(scrptEngnId).nOutputEnabled;
}

void CNppExecConsole::SetOutputEnabled(bool bEnabled)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    _setOutputEnabled(scrptEngnId, bEnabled);
}

void CNppExecConsole::_setOutputEnabled(ScriptEngineId scrptEngnId, bool bEnabled)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    CCriticalSectionLockGuard lock(m_csStateList);
    ConsoleState& state = _getState(scrptEngnId);
    if ( bEnabled )
    {
        if ( state.nOutputEnabled == 0 )
            state.nOutputEnabled = 1; // enabled after was disabled
        else
            state.nOutputEnabled = 2; // enabled after was already enabled
    }
    else
        state.nOutputEnabled = 0;
}

void CNppExecConsole::SetOutputEnabledDirectly(ScriptEngineId scrptEngnId, int nEnabled)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    CCriticalSectionLockGuard lock(m_csStateList);
    ConsoleState& state = _getState(scrptEngnId);
    state.nOutputEnabled = nEnabled;
}

int CNppExecConsole::GetOutputEnabledDirectly(ScriptEngineId scrptEngnId) const
{
    return _isOutputEnabledN(scrptEngnId);
}

BOOL CNppExecConsole::IsScrollToEnd() const
{
    if ( CNppExec::_bIsNppShutdown )
        return TRUE;

    return _isScrollToEnd();
}

BOOL CNppExecConsole::_isScrollToEnd() const
{
    if ( CNppExec::_bIsNppShutdown )
        return TRUE;

#ifdef _SCROLL_TO_LATEST  
    if ( !Runtime::GetNppExec().GetOptions().GetBool(OPTB_CONSOLE_SCROLL2LATEST) )
    {
        return FALSE;
    }
#endif

    // nothing to protect with a lock currently :)
    // CCriticalSectionLockGuard lock(m_csStateList);

    return TRUE;
}

void CNppExecConsole::ClearText(bool bForce )
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( (!bForce) && (!_isOutputEnabled(scrptEngnId)) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _clearText(scrptEngnId);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_clearText, this, scrptEngnId);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_clearText(ScriptEngineId scrptEngnId)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    m_reConsole.SetText( _T("") );
    _restoreDefaultTextStyle( scrptEngnId, true );
}

void CNppExecConsole::RestoreDefaultTextStyle(bool bLockPos)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _restoreDefaultTextStyle(scrptEngnId, bLockPos);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_restoreDefaultTextStyle, this, scrptEngnId, bLockPos);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_restoreDefaultTextStyle(ScriptEngineId scrptEngnId, bool bLockPos)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    m_reConsole.AddStr( _T(" "), FALSE, _getCurrentColorTextNorm(), CFM_EFFECTS, 0 );
    int iPos = m_reConsole.GetTextLengthEx();
    m_reConsole.ExSetSel(iPos - 1, -1);
    m_reConsole.ReplaceSelText( _T("") );
    if ( bLockPos )
        _lockConsolePos( scrptEngnId, iPos - 1 );
}

void CNppExecConsole::UpdateColours()
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !postponeThisCall(scrptEngnId) )
    {
        _updateColours(scrptEngnId);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_updateColours, this, scrptEngnId);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_updateColours(ScriptEngineId scrptEngnId)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList

    // background color
    if ( _getCurrentColorBkgnd() != 0xFFFFFFFF )
        m_reConsole.SendMsg(EM_SETBKGNDCOLOR, 0, _getCurrentColorBkgnd());
    else
        m_reConsole.SendMsg(EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_WINDOW));

//#ifdef UNICODE
    DWORD dwLangOpt = (DWORD) m_reConsole.SendMsg( EM_GETLANGOPTIONS, 0, 0 );
    if (dwLangOpt & IMF_AUTOFONT)
    {
        dwLangOpt -= IMF_AUTOFONT;
        m_reConsole.SendMsg( EM_SETLANGOPTIONS, 0, dwLangOpt );
    }
//#endif

    // normal text color
    CHARFORMAT cf;
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_COLOR | CFM_FACE;
    cf.dwEffects = 0;
    // default character format
    m_reConsole.SendMsg( EM_GETCHARFORMAT, SCF_ALL, (LPARAM) &cf );
    if ( cf.dwEffects & CFE_AUTOCOLOR )  
        cf.dwEffects -= CFE_AUTOCOLOR; // removing CFE_AUTOCOLOR
    cf.dwMask = CFM_COLOR | CFM_FACE;
    cf.crTextColor = _getCurrentColorTextNorm();  // set our color
    m_reConsole.SendMsg( EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &cf );
    // set character format explicitly
    _restoreDefaultTextStyle(scrptEngnId, false);
}

void CNppExecConsole::ProcessSlashR() // "\r"
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _processSlashR(scrptEngnId);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_processSlashR, this, scrptEngnId);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_processSlashR(ScriptEngineId /*scrptEngnId*/)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    int nPos = m_reConsole.ExGetSelPos();
    int nLen = m_reConsole.LineLength(nPos);
    m_reConsole.ExSetSel(nPos - nLen, nPos);
    m_reConsole.ReplaceSelText( _T("") );
}

void CNppExecConsole::ProcessSlashB(int nCount) // "\b"
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    const ScriptEngineId scrptEngnId = GetScriptEngineId();
    if ( !_isOutputEnabled(scrptEngnId) )
        return;

    if ( !postponeThisCall(scrptEngnId) )
    {
        _processSlashB(scrptEngnId, nCount);
    }
    else
    {
        std::function<void ()> postponedCall = std::bind(&CNppExecConsole::_processSlashB, this, scrptEngnId, nCount);

        {
            CCriticalSectionLockGuard lock(m_csStateList);
            _getState(scrptEngnId).PostponedCalls.push_back(postponedCall);
        }
    }
}

void CNppExecConsole::_processSlashB(ScriptEngineId /*scrptEngnId*/, int nCount)
{
    if ( CNppExec::_bIsNppShutdown )
        return;

    // Important: SendMsg() calls must _not_ be under m_csStateList
    int nPos = m_reConsole.ExGetSelPos();
    m_reConsole.ExSetSel(nPos - nCount, nPos);
    m_reConsole.ReplaceSelText( _T("") );

}

void CNppExecConsole::OnScriptEngineStarted()
{
    CCriticalSectionLockGuard lock(m_csStateList);
    m_StateList.push_back(ConsoleState());
}

void CNppExecConsole::OnScriptEngineFinished()
{
    std::list<ConsoleState> postponedStateList;
    const ScriptEngineId scrptEngnId = GetScriptEngineId();

    {
        CCriticalSectionLockGuard lock(m_csStateList);

        auto itrState = std::find_if( m_StateList.rbegin(), m_StateList.rend(), 
                                        [scrptEngnId](const ConsoleState& state) { return (state.ScrptEngnId == scrptEngnId); } );
        if ( itrState != m_StateList.rend() )
        {
            itrState->IsFinished = true;
            if ( itrState == m_StateList.rbegin() ) // the last state in the list
            {
                for ( ; ; )
                {
                    if ( !itrState->IsFinished )
                    {
                        --itrState;
                        break;
                    }

                    if ( itrState.base() == m_StateList.begin() )
                        break;

                    ++itrState;
                }
                postponedStateList.splice( postponedStateList.end(), m_StateList, std_helpers::forward_iterator(itrState), m_StateList.end() );
            }
        }
      #if SCRPTENGNID_DEBUG_OUTPUT
        else
        {
            tstr S;
            S.Format(1020, _T("CNppExecConsole::OnScriptEngineFinished - scrptEngnId=%u is not in m_StateList - this is REALLY unexpected!!!\n"), scrptEngnId);
            ::OutputDebugString(S.c_str());
        }
      #endif
    }

    for ( auto itrState = postponedStateList.rbegin(); itrState != postponedStateList.rend(); ++itrState )
    {
      #if SCRPTENGNID_DEBUG_OUTPUT
        {
            tstr S;
            S.Format(1020, _T("CNppExecConsole::OnScriptEngineFinished - scrptEngnId=%u\n"), scrptEngnId);
            ::OutputDebugString(S.c_str());
            S.Format(1020, _T("... itrState->execPostponedCalls - scrptEngnId=%u, PostponedCalls=%u\n"), itrState->ScrptEngnId, itrState->PostponedCalls.size());
            ::OutputDebugString(S.c_str());
        }
      #endif
        itrState->execPostponedCalls(&m_csStateList);
    }
}

void CNppExecConsole::OnScriptEngineAborting(DWORD dwThreadId)
{
    const ScriptEngineId scrptEngnId = dwThreadId;

    {
        CCriticalSectionLockGuard lock(m_csStateList);

        auto itrState = std::find_if( m_StateList.rbegin(), m_StateList.rend(), 
            [scrptEngnId](const ConsoleState& state) { return (state.ScrptEngnId == scrptEngnId); } );
        if ( itrState != m_StateList.rend() )
        {
          #if SCRPTENGNID_DEBUG_OUTPUT
            {
                tstr S;
                S.Format(1020, _T("CNppExecConsole::OnScriptEngineAborting - aborting scrptEngnId=%u\n"), scrptEngnId);
                ::OutputDebugString(S.c_str());
            }
          #endif
            itrState->IsAbortRequested = true;
        }
      #if SCRPTENGNID_DEBUG_OUTPUT
        else
        {
            tstr S;
            S.Format(1020, _T("CNppExecConsole::OnScriptEngineAborting - scrptEngnId=%u is not in m_StateList - this is REALLY unexpected!!!\n"), scrptEngnId);
            ::OutputDebugString(S.c_str());
        }
      #endif
    }
}

const CNppExecConsole::ConsoleState& CNppExecConsole::_getState(ScriptEngineId scrptEngnId) const
{
    // hint: m_StateList[0] - _always_ present; [1] - current script engine, if any
    if ( scrptEngnId == 0 )
        return m_StateList.front();

    if ( m_StateList.size() > 2 )
    {
        const auto itrState = std::find_if( m_StateList.rbegin(), m_StateList.rend(), 
                                              [scrptEngnId](const ConsoleState& state) { return (state.ScrptEngnId == scrptEngnId); } );
        if ( itrState != m_StateList.rend() )
            return *itrState;
    }
    return m_StateList.back();
}

CNppExecConsole::ConsoleState& CNppExecConsole::_getState(ScriptEngineId scrptEngnId)
{
    // hint: m_StateList[0] - _always_ present; [1] - current script engine, if any
    if ( scrptEngnId == 0 )
        return m_StateList.front();

    if ( m_StateList.size() > 2 )
    {
        const auto itrState = std::find_if( m_StateList.rbegin(), m_StateList.rend(), 
                                              [scrptEngnId](const ConsoleState& state) { return (state.ScrptEngnId == scrptEngnId); } );
        if ( itrState != m_StateList.rend() )
            return *itrState;

        static ConsoleState dummy;
        return dummy;
    }
    return m_StateList.back();
}

CNppExecConsole::ConsoleState::ConsoleState()
  : nOutputEnabled(1)
  //, colorTextNorm(0)
  //, colorTextMsg(0)
  //, colorTextErr(0)
  //, colorBkgnd(0)
  , IsFinished(false)
  , IsAbortRequested(false)
{
    ScrptEngnId = GetScriptEngineId();
  #if SCRPTENGNID_DEBUG_OUTPUT
    {
        tstr S;
        S.Format(1020, _T("ConsoleState - create - scrptEngnId=%u\n"), ScrptEngnId);
        ::OutputDebugString(S.c_str());
    }
  #endif
}

CNppExecConsole::ConsoleState::~ConsoleState()
{
    execPostponedCalls(nullptr);
  #if SCRPTENGNID_DEBUG_OUTPUT
    {
        tstr S;
        S.Format(1020, _T("ConsoleState - destroy - scrptEngnId=%u\n"), ScrptEngnId);
        ::OutputDebugString(S.c_str());
    }
  #endif
}

void CNppExecConsole::ConsoleState::execPostponedCalls(CCriticalSection* csState)
{
    std::list< std::function<void ()> > pstpndCalls;
    std::list< tstr > pstpndStrings;

    {
        if ( csState != nullptr )
            csState->Lock();

        if ( !PostponedCalls.empty() )
        {
            // This allows to avoid blocking during the processing.
            // Also it clears the PostponedCalls & PostponedStrings.
            std::swap(pstpndCalls, PostponedCalls);
            std::swap(pstpndStrings, PostponedStrings);
        }

        if ( csState != nullptr )
            csState->Unlock();
    }

    if ( CNppExec::_bIsNppShutdown || pstpndCalls.empty() )
        return;

  #if SCRPTENGNID_DEBUG_OUTPUT
    {
        tstr S;
        S.Format(1020, _T("CNppExecConsole::ConsoleState::execPostponedCalls - scrptEngnId=%u, PostponedCalls=%u\n"), ScrptEngnId, pstpndCalls.size());
        ::OutputDebugString(S.c_str());
    }
  #endif

    // the processing
    for ( const auto& func : pstpndCalls )
    {
        if ( IsAbortRequested )
            break;

        func();
      #if SCRPTENGNID_DEBUG_OUTPUT
        ::Sleep(50);
      #endif
    }
}

//-------------------------------------------------------------------------

