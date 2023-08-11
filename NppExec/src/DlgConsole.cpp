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

#include "DlgConsole.h"
#include "NppExec.h"
#include "NppExecEngine.h"
#include "ChildProcess.h"
#include "npp_files/PluginInterface.h"  // docking feature
#include "npp_files/Docking.h"          // docking feature
#include "WarningAnalyzer.h"
#include "CSimpleLogger.h"
#include "DirFileLister.h"
#include "DlgAdvancedOptions.h"
#include "c_base/str_func.h"
#include <commctrl.h>

#ifndef IMF_AUTOFONT
  #define IMF_AUTOFONT 0x0002
#endif

extern int                   g_nUserMenuItems;
extern FuncItem              g_funcItem[nbFunc + MAX_USERMENU_ITEMS + 1];
extern ShortcutKey           g_funcShortcut[nbFunc + MAX_USERMENU_ITEMS + 1];

extern TCHAR CONSOLE_DLG_TITLE[];
extern TCHAR PLUGIN_NAME_DLL[];
extern TCHAR CMDHISTORY_FILENAME[];

extern COLORREF g_colorBkgnd;
extern COLORREF g_colorTextNorm;

INT  nConsoleFirstUnlockedPos = 0;
bool  bFuncItemEntered = false;

const TCHAR CONSOLE_CMD_HELP[]   = _T("HELP");   // show console commands
const TCHAR CONSOLE_CMD_VER[]    = _T("VER");    // show plugin's version
const TCHAR CONSOLE_CMD_MANUAL[] = _T("MANUAL"); // show plugin's manual
const TCHAR CONSOLE_CMD_ABOUT[]  = _T("ABOUT");  // show plugin's about

#ifdef UNICODE
const TCHAR PLUGIN_CURRENT_VER[] = NPPEXEC_VER_STR _T(" Unicode"); // see also: NppExecPluginInterface.cpp
#else
const TCHAR PLUGIN_CURRENT_VER[] = NPPEXEC_VER_STR _T(" ANSI"); // see also: NppExecPluginInterface.cpp
#endif


const TCHAR CONSOLE_COMMANDS_INFO[] = _T_RE_EOL \
  _T("-------- Console keys --------") _T_RE_EOL \
  _T("Enter  -  executes entered command") _T_RE_EOL \
  _T("Shift+Enter  -  new line") _T_RE_EOL \
  _T("Tab  -  auto-completes current command/looks through the commands history") _T_RE_EOL \
  _T("Shift+Tab  -  the same as Tab but backwards") _T_RE_EOL \
  _T("Arrow Up  -  previous command (when Console Commands History is enabled)") _T_RE_EOL \
  _T("Arrow Down  -  next command (when Console Commands History is enabled)") _T_RE_EOL \
  _T("Ctrl+C  -  copy selected text to the clipboard") _T_RE_EOL \
  _T("Ctrl+V  -  paste from the clipboard") _T_RE_EOL \
  _T("Ctrl+W  -  word wrap on/off") _T_RE_EOL \
  _T("Ctrl+~  -  toggle Console/editing window") _T_RE_EOL \
  _T("Ctrl+T  -  hide toggled Console on/off") _T_RE_EOL \
  _T("Ctrl+F, F3, F7  -  shows Console\'s Find dialog") _T_RE_EOL \
  _T("Esc  -  hides Console\'s Find dialog") _T_RE_EOL \
  _T("Enter, F3 (in Console\'s Find dialog)  -  find next (down)") _T_RE_EOL \
  _T("Shift+Enter, Shift+F3 (in Console\'s Find dialog)  -  find previous (up)") _T_RE_EOL \
  _T("-------- Console commands --------") _T_RE_EOL \
  _T("help  -  show available commands") _T_RE_EOL \
  _T("help <command>  -  show detailed information about specified command") _T_RE_EOL \
  _T("help all  -  show all available help (all the commands in detail)") _T_RE_EOL \
  _T("ver  -  show plugin\'s version") _T_RE_EOL \
  _T("manual  -  show NppExec User Guide") _T_RE_EOL \
  _T("about  - show NppExec Help/About") _T_RE_EOL \
  DEFAULT_NPPEXEC_CMD_PREFIX _T("  -  prefix for NppExec's commands (e.g. \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("npp_console off\")") _T_RE_EOL \
  DEFAULT_NPPEXEC_CMD_PREFIX_COLLATERAL_FORCED _T("  -  always executes a command in a collateral (parallel) script") _T_RE_EOL \
  _T("CTRL+C  -  kill (via Ctrl+C) or terminate current child process") _T_RE_EOL \
  _T("CTRL+BREAK  -  kill (via Ctrl+Break) or terminate current child process") _T_RE_EOL \
  _T("CTRL+BREAK  -  abort current NppExec\'s script (without closing the Console)") _T_RE_EOL \
  _T("CTRL+Z  -  send ^Z to current child process") _T_RE_EOL \
  _T("-------- General commands --------") _T_RE_EOL \
  _T("cls  -  clear Console screen") _T_RE_EOL \
  _T("cd  -  shows current path") _T_RE_EOL \
  _T("cd <absolute_path or relative_path>  -  changes current path") _T_RE_EOL \
  _T("dir  -  lists files and subdirs") _T_RE_EOL \
  _T("dir <mask or path\\mask>  -  lists files/subdirs matched the mask") _T_RE_EOL \
  _T("echo <text>  -  prints a text in the Console") _T_RE_EOL \
  _T("if <condition> goto <label>  -  jumps to the label if the condition is true") _T_RE_EOL \
  _T("if~ <condition> goto <label>  -  calculates and checks the condition") _T_RE_EOL \
  _T("if ... else if ... else ... endif  -  conditional execution") _T_RE_EOL \
  _T("if~ ... else if~ ... else ... endif  -  conditional execution") _T_RE_EOL \
  _T("goto <label>  -  jumps to the label") _T_RE_EOL \
  _T("exit  -  exits the current NppExec's script") _T_RE_EOL \
  _T("exit <type>  -  exits the NppExec's script") _T_RE_EOL \
  _T("set  -  shows all user\'s variables") _T_RE_EOL \
  _T("set <var>  -  shows the value of user\'s variable <var>") _T_RE_EOL \
  _T("set <var> = <value>  -  sets the value of user\'s variable <var>") _T_RE_EOL \
  _T("set <var> ~ <math expression>  -  calculates the math expression") _T_RE_EOL \
  _T("set <var> ~ strlen <string>  -  calculates the string length") _T_RE_EOL \
  _T("set <var> ~ strlenutf8 <string>  -  calculates the UTF-8 string length") _T_RE_EOL \
  _T("set <var> ~ strlenu <string>  -  the same as strlenutf8") _T_RE_EOL \
  _T("set <var> ~ strlensci <string>  -  string length, using Scintilla's encoding") _T_RE_EOL \
  _T("set <var> ~ strlens <string>  -  the same as strlensci") _T_RE_EOL \
  _T("set <var> ~ strupper <string>  -  returns the string in upper case") _T_RE_EOL \
  _T("set <var> ~ strlower <string>  -  returns the string in lower case") _T_RE_EOL \
  _T("set <var> ~ substr <pos> <len> <string>  -  returns the substring") _T_RE_EOL \
  _T("set <var> ~ strfind <s> <t>  -  returns the first position of <t> in <s>") _T_RE_EOL \
  _T("set <var> ~ strrfind <s> <t>  -  returns the last position of <t> in <s>") _T_RE_EOL \
  _T("set <var> ~ strreplace <s> <t0> <t1>  -  replaces all <t0> with <t1>") _T_RE_EOL \
  _T("set <var> ~ strquote <s>  -  surrounds <s> with \"\" quotes") _T_RE_EOL \
  _T("set <var> ~ strunquote <s>  -  removes the surrounding \"\" quotes") _T_RE_EOL \
  _T("set <var> ~ strescape <s>  -  simple character escaping (e.g. <TAB> to '\\t')") _T_RE_EOL \
  _T("set <var> ~ strunescape <s>  -  simple character unescaping (e.g. '\\n' to <LF>)") _T_RE_EOL \
  _T("set <var> ~ strexpand <s>  -  expands all $(sub) values within <s>") _T_RE_EOL \
  _T("set <var> ~ strfromhex <hs>  -  returns a string from the hex-string") _T_RE_EOL \
  _T("set <var> ~ strtohex <s>  -  returns a hex-string from the string") _T_RE_EOL \
  _T("set <var> ~ chr <n>  -  returns a character from a character code <n>") _T_RE_EOL \
  _T("set <var> ~ ord <c>  -  returns a decimal character code of a character <c>") _T_RE_EOL \
  _T("set <var> ~ ordx <c>  -  returns a hexadecimal character code of a character <c>") _T_RE_EOL \
  _T("set <var> ~ normpath <path>  -  returns a normalized path") _T_RE_EOL \
  _T("set <var> ~ fileexists <path>  -  checks if a given file exists") _T_RE_EOL \
  _T("set <var> ~ direxists <path>  -  checks if a given directory exists") _T_RE_EOL \
  _T("set local  -  shows all user\'s local variables") _T_RE_EOL \
  _T("set local <var>  -  shows the value of user\'s local variable <var>") _T_RE_EOL \
  _T("set local <var> = ...  -  sets the value of user\'s local variable <var>") _T_RE_EOL \
  _T("set local <var> ~ ...  -  calculates the value of user\'s local variable") _T_RE_EOL \
  _T("set +v <var> = ...  -  sets the value of <var> using delayed vars substitution") _T_RE_EOL \
  _T("set +v local <var> = ...  -  sets the local <var> using delayed vars substitution") _T_RE_EOL \
  _T("unset <var>  -  removes user\'s variable <var>") _T_RE_EOL \
  _T("unset <var> = <value>  -  removes user\'s variable <var>") _T_RE_EOL \
  _T("unset local <var>  -  removes user\'s local variable <var>") _T_RE_EOL \
  _T("env_set <var>  -  shows the value of environment variable <var>") _T_RE_EOL \
  _T("env_set <var> = <value>  -  sets the value of environment variable <var>") _T_RE_EOL \
  _T("env_set local ...  -  sets an environment variable locally (within the current script)") _T_RE_EOL \
  _T("env_unset <var>  -  removes/restores the environment variable <var>") _T_RE_EOL \
  _T("set_env <var> = <value>  -  see \"env_set\"") _T_RE_EOL \
  _T("unset_env <var>  -  see \"env_unset\"") _T_RE_EOL \
  _T("inputbox \"message\"  -  shows InputBox, sets $(INPUT)") _T_RE_EOL \
  _T("inputbox \"message\" : initial_value  -  shows InputBox, sets $(INPUT)") _T_RE_EOL \
  _T("inputbox \"message\" : \"value_name\" : initial_value  -  InputBox customization") _T_RE_EOL \
  _T("inputbox \"message\" : \"value_name\" : \"initial_value\" : time_ms  -  expirable") _T_RE_EOL \
  _T("messagebox \"text\"  -  shows a simple MessageBox") _T_RE_EOL \
  _T("messagebox \"text\" : \"title\"  -  shows a MessageBox with a custom title") _T_RE_EOL \
  _T("messagebox \"text\" : \"title\" : type  -  shows a MessageBox of a given type") _T_RE_EOL \
  _T("messagebox \"text\" : \"title\" : type : timeout  -  expirable MessageBox") _T_RE_EOL \
  _T("con_colour <colours>  -  sets the Console\'s colours") _T_RE_EOL \
  _T("con_colour local ...  -  sets the colours locally (within the current script)") _T_RE_EOL \
  _T("con_filter <filters>  -  enables/disables the Console\'s output filters") _T_RE_EOL \
  _T("con_filter local ...  -  sets the filters locally (within the current script)") _T_RE_EOL \
  _T("con_loadfrom <file>  -  loads a file\'s content to the Console") _T_RE_EOL \
  _T("con_load <file>  -  see \"con_loadfrom\"") _T_RE_EOL \
  _T("con_saveto <file>  -  saves the Console\'s content to a file") _T_RE_EOL \
  _T("con_save <file>  -  see \"con_saveto\"") _T_RE_EOL \
  _T("sel_loadfrom <file>  -  replace current selection with a file\'s content") _T_RE_EOL \
  _T("sel_load <file>  -  see \"sel_loadfrom\"") _T_RE_EOL \
  _T("sel_saveto <file>  -  save the selected text to a file") _T_RE_EOL \
  _T("sel_saveto <file> : <encoding>  -  save the selected text to a file") _T_RE_EOL \
  _T("sel_save <file> : <encoding>  -  see \"sel_saveto\"") _T_RE_EOL \
  _T("sel_settext <text>  -  replace current selection with the text specified") _T_RE_EOL \
  _T("sel_settext+ <text>  -  replace current selection with the text specified") _T_RE_EOL \
  _T("text_loadfrom <file>  -  replace the entire text with a file\'s content") _T_RE_EOL \
  _T("text_load <file>  -  see \"text_loadfrom\"") _T_RE_EOL \
  _T("text_saveto <file>  -  save the entire text to a file") _T_RE_EOL \
  _T("text_saveto <file> : <encoding>  -  save the entire text to a file") _T_RE_EOL \
  _T("text_save <file> : <encoding>  -  see \"text_saveto\"") _T_RE_EOL \
  _T("clip_settext <text>  -  set the clipboard text") _T_RE_EOL \
  _T("npp_exec <script>  -  execute commands from specified NppExec\'s script") _T_RE_EOL \
  _T("npp_exec <file>  -  execute commands from specified NppExec\'s file") _T_RE_EOL \
  _T("npp_exec <script/file> <args>  -  passes additional arguments <args>") _T_RE_EOL \
  _T("npp_exectext <mode> <text>  -  execute the given text") _T_RE_EOL \
  _T("npp_close  -  close current file in Notepad++") _T_RE_EOL \
  _T("npp_close <file>  -  close specified file opened in Notepad++") _T_RE_EOL \
  _T("npp_console <on/off/keep>  -  show/hide the Console window") _T_RE_EOL \
  _T("npp_console <enable/disable>  -  enables/disables output to the Console") _T_RE_EOL \
  _T("npp_console <1/0/?>  -  show/hide the Console window") _T_RE_EOL \
  _T("npp_console <+/->  -  enables/disables output to the Console") _T_RE_EOL \
  _T("npp_console local ...  -  Console on/off locally (within the current script)") _T_RE_EOL \
  _T("npp_menucommand <menu\\item\\name>  -  executes (invokes) a menu item") _T_RE_EOL \
  _T("npp_open <file>  -  (re)open specified file in Notepad++") _T_RE_EOL \
  _T("npp_open <mask or path\\mask>  -  opens file(s) matched the mask") _T_RE_EOL \
  _T("npp_run <command>  -  run external process/command") _T_RE_EOL \
  _T("npp_save  -  save current file in Notepad++") _T_RE_EOL \
  _T("npp_save <file>  -  save specified file opened in Notepad++") _T_RE_EOL \
  _T("npp_saveas <file>  -  save current file with a new (path)name") _T_RE_EOL \
  _T("npp_saveall  -  save all modified files") _T_RE_EOL \
  _T("npp_switch <file>  -  switch to specified opened file") _T_RE_EOL \
  _T("npp_setfocus  -  set the keyboard focus") _T_RE_EOL \
  _T("npp_sendmsg <msg>  -  send a message (msg) to Notepad++") _T_RE_EOL \
  _T("npp_sendmsg <msg> <wparam>  -  message with parameter (wparam)") _T_RE_EOL \
  _T("npp_sendmsg <msg> <wparam> <lparam>  -  msg to Notepad++") _T_RE_EOL \
  _T("npp_sendmsgex <hwnd> <msg> <wparam> <lparam>  -  msg to hwnd") _T_RE_EOL \
  _T("sci_sendmsg <msg>  -  send a message (msg) to current Scintilla") _T_RE_EOL \
  _T("sci_sendmsg <msg> <wparam>  -  message with parameter (wparam)") _T_RE_EOL \
  _T("sci_sendmsg <msg> <wparam> <lparam>  -  msg to Scintilla") _T_RE_EOL \
  _T("sci_find <flags> <find_what>  -  find a string") _T_RE_EOL \
  _T("sci_replace <flags> <find_what> <replace_with>  -  replace a string") _T_RE_EOL \
  _T("proc_input <string>  -  send a string to a child process") _T_RE_EOL \
  _T("proc_signal <signal>  -  signal to a child process") _T_RE_EOL \
  _T("sleep <ms>  -  sleep for ms milliseconds") _T_RE_EOL \
  _T("sleep <ms> <text>  -  print the text and sleep for ms milliseconds") _T_RE_EOL \
  _T("npe_cmdalias  -  show all command aliases") _T_RE_EOL \
  _T("npe_cmdalias <alias>  -  shows the value of command alias") _T_RE_EOL \
  _T("npe_cmdalias <alias> =  -  removes the command alias") _T_RE_EOL \
  _T("npe_cmdalias <alias> = <command>  -  sets the command alias") _T_RE_EOL \
  _T("npe_cmdalias local ...  -  local command alias (within the current script)") _T_RE_EOL \
  _T("npe_console <options>  -  set/modify Console options/mode") _T_RE_EOL \
  _T("npe_console local ...  -  sets Console's mode locally (within the current script)") _T_RE_EOL \
  _T("npe_debuglog <on/off>  -  enable/disable Debug Log") _T_RE_EOL \
  _T("npe_debuglog local ...  -  enable/disable Debug Log locally (within the current script)") _T_RE_EOL \
  _T("npe_debug <1/0>  -  see \"npe_debuglog\"") _T_RE_EOL \
  _T("npe_noemptyvars <1/0>  -  enable/disable replacement of empty vars") _T_RE_EOL \
  _T("npe_noemptyvars local ...  -  sets empty vars on/off locally (within the current script)") _T_RE_EOL \
  _T("npe_queue <command>  -  queue NppExec's command to be executed") _T_RE_EOL \
  _T("npe_sendmsgbuflen <max_len>  -  set npp/sci_sendmsg's buffer length") _T_RE_EOL \
  _T("npe_sendmsgbuflen local ...  -  sets the buffer length locally (within the current script)") _T_RE_EOL \
  DEFAULT_ALIAS_CMD_NPPEXEC _T("<script/file>  -  the same as  npp_exec <script/file>") _T_RE_EOL \
  DEFAULT_ALIAS_CMD_NPPEXEC _T("<script/file> <args>  -  the same as  npp_exec <script/file> <args>") _T_RE_EOL \
  DEFAULT_NPPEXEC_CMD_PREFIX _T("  -  prefix for NppExec's commands (e.g. \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("npp_console off\")") _T_RE_EOL \
  DEFAULT_NPPEXEC_CMD_PREFIX_COLLATERAL_FORCED _T("  -  always executes a command in a collateral (parallel) script") _T_RE_EOL \
  _T("-------- Environment variables --------") _T_RE_EOL \
  _T("$(FULL_CURRENT_PATH)  :  E:\\my Web\\main\\welcome.html") _T_RE_EOL \
  _T("$(CURRENT_DIRECTORY)  :  E:\\my Web\\main") _T_RE_EOL \
  _T("$(FILE_NAME)  :  welcome.html") _T_RE_EOL \
  _T("$(NAME_PART)  :  welcome") _T_RE_EOL \
  _T("$(EXT_PART)  :  .html") _T_RE_EOL \
  _T("$(NPP_DIRECTORY)  :  full path of directory with notepad++.exe") _T_RE_EOL \
  _T("$(NPP_FULL_FILE_PATH)  :  full path to notepad++.exe") _T_RE_EOL \
  _T("$(CURRENT_WORD)  :  word(s) you selected in Notepad++") _T_RE_EOL \
  _T("$(CURRENT_LINE)  :  current line number") _T_RE_EOL \
  _T("$(CURRENT_LINESTR)  :  text of the current line") _T_RE_EOL \
  _T("$(CURRENT_COLUMN)  :  current column number") _T_RE_EOL \
  _T("$(SELECTED_TEXT)  :  the text you selected in Notepad++") _T_RE_EOL \
  _T("$(FILE_NAME_AT_CURSOR)  :  file name selected in the editor") _T_RE_EOL \
  _T("$(WORKSPACE_ITEM_PATH)  :  full path to the current item in the workspace pane") _T_RE_EOL \
  _T("$(WORKSPACE_ITEM_DIR)  :  directory containing the current item in the workspace pane") _T_RE_EOL \
  _T("$(WORKSPACE_ITEM_NAME)  :  file name of the current item in the workspace pane") _T_RE_EOL \
  _T("$(WORKSPACE_ITEM_ROOT)  :  root path of the current item in the workspace pane") _T_RE_EOL \
  _T("$(CLOUD_LOCATION_PATH)  :  cloud location path (in Notepad++\'s settings)") _T_RE_EOL \
  _T("$(CLIPBOARD_TEXT)  :  text from the clipboard") _T_RE_EOL \
  _T("$(#N)  :  full path of the Nth opened document (N=1,2,3...)") _T_RE_EOL \
  _T("$(#0)  :  full path to notepad++.exe") _T_RE_EOL \
  _T("$(LEFT_VIEW_FILE)  :  current file path-name in primary (left) view") _T_RE_EOL \
  _T("$(RIGHT_VIEW_FILE)  :  current file path-name in secondary (right) view") _T_RE_EOL \
  _T("$(PLUGINS_CONFIG_DIR)  :  full path of the plugins configuration directory") _T_RE_EOL \
  _T("$(CWD)  :  current working directory of NppExec (use \"cd\" to change it)") _T_RE_EOL \
  _T("$(ARGC)  :  number of arguments passed to the NPP_EXEC command") _T_RE_EOL \
  _T("$(ARGV)  :  all arguments passed to the NPP_EXEC command after the script name") _T_RE_EOL \
  _T("$(ARGV[0])  :  script name - first parameter of the NPP_EXEC command") _T_RE_EOL \
  _T("$(ARGV[N])  :  Nth argument (N=1,2,3...)") _T_RE_EOL \
  _T("$(RARGV)  :  all arguments in reverse order (except the script name)") _T_RE_EOL \
  _T("$(RARGV[N])  :  Nth argument in reverse order (N=1,2,3...)") _T_RE_EOL \
  _T("$(INPUT)  :  this value is set by the \'inputbox\' command") _T_RE_EOL \
  _T("$(INPUT[N])  :  Nth field of the $(INPUT) value (N=1,2,3...)") _T_RE_EOL \
  _T("$(OUTPUT)  :  this value can be set by the child process, see npe_console v+") _T_RE_EOL \
  _T("$(OUTPUT1)  :  first line in $(OUTPUT)") _T_RE_EOL \
  _T("$(OUTPUTL)  :  last line in $(OUTPUT)") _T_RE_EOL \
  _T("$(EXITCODE)  :  exit code of the last executed child process") _T_RE_EOL \
  _T("$(PID)  :  process id of the current (or the last) child process") _T_RE_EOL \
  _T("$(IS_PROCESS)  :  is child process running (1 - yes, 0 - no)") _T_RE_EOL \
  _T("$(LAST_CMD_RESULT)  :  result of the last NppExec's command") _T_RE_EOL \
  _T("                         (1 - succeeded, 0 - failed, -1 - invalid arg)") _T_RE_EOL \
  _T("$(MSG_RESULT)  :  result of \'npp_sendmsg[ex]\' or \'sci_sendmsg\'") _T_RE_EOL \
  _T("$(MSG_WPARAM)  :  wParam (output) of \'npp_sendmsg[ex]\' or \'sci_sendmsg\'") _T_RE_EOL \
  _T("$(MSG_LPARAM)  :  lParam (output) of \'npp_sendmsg[ex]\' or \'sci_sendmsg\'") _T_RE_EOL \
  _T("$(NPP_HWND)  :  Notepad++'s main window handle") _T_RE_EOL \
  _T("$(NPP_PID)  :  Notepad++'s process id") _T_RE_EOL \
  _T("$(SCI_HWND)  :  current Scintilla's window handle") _T_RE_EOL \
  _T("$(SCI_HWND1)  :  primary Scintilla's window handle (main view)") _T_RE_EOL \
  _T("$(SCI_HWND2)  :  secondary Scintilla's window handle (second view)") _T_RE_EOL \
  _T("$(CON_HWND)  :  NppExec's Console window handle (RichEdit control)") _T_RE_EOL \
  _T("$(FOCUSED_HWND)  :  focused window handle") _T_RE_EOL \
  _T("$(SYS.<var>)  :  system\'s environment variable, e.g. $(SYS.PATH)") _T_RE_EOL \
  _T("$(@EXIT_CMD)  :  a callback exit command for a child process") _T_RE_EOL \
  _T("$(@EXIT_CMD_SILENT)  :  a silent (non-printed) callback exit command") _T_RE_EOL \
  _T_RE_EOL;

typedef struct sCmdItemInfo {
    const TCHAR* const name;
    const TCHAR* const info;
} tCmdItemInfo;

#ifdef UNICODE
  #define _T_MENUCOMMAND_CYRILLIC  _T("\x0424\x0430\x0439\x043B|\x041D\x043E\x0432\x044B\x0439")
  #define _T_STRLEN_CYRILLIC       _T("\x041F\x0440\x0438\x0432\x0435\x0442")
  #define _T_STRHEX_CYRILLIC       _T("1F 04 40 04 38 04 32 04 35 04 42 04")
#else
  #define _T_MENUCOMMAND_CYRILLIC  _T("\xD4\xE0\xE9\xEB|\xCD\xEE\xE2\xFB\xE9")
  #define _T_STRLEN_CYRILLIC       _T("\xCF\xF0\xE8\xE2\xE5\xF2")
  #define _T_STRHEX_CYRILLIC       _T("CF F0 E8 E2 E5 F2")
#endif

#define _T_LOCAL_CMD_HINT \
  _T("  When \"local\" is specified, the changes are applied locally to the current") _T_RE_EOL \
  _T("  NppExec's script and are reverted back when the current script ends.") _T_RE_EOL

#ifdef UNICODE
  #define _T_HELP_STRTOHEX_STRFROMHEX \
  _T("  // strfromhex (unicode version)") _T_RE_EOL \
  _T("  set s ~ strfromhex 0D 00 0A 00           // <CR><LF> = \\r\\n in C/C++") _T_RE_EOL \
  _T("  set s ~ strfromhex \"ab\" 09 00 \"c\" 0A 00  // ab<TAB>c<LF> = ab\\tc\\n in C/C++") _T_RE_EOL \
  _T("  // strtohex (unicode version)") _T_RE_EOL \
  _T("  set s ~ strtohex abc123   // 61 00 62 00 63 00 31 00 32 00 33 00") _T_RE_EOL \
  _T("  set s ~ strtohex \"ab 12\"  // 61 00 62 00 20 00 31 00 32 00") _T_RE_EOL \
  _T("  set s ~ strtohex ") _T_STRLEN_CYRILLIC _T("   // ") _T_STRHEX_CYRILLIC _T_RE_EOL
#else
  #define _T_HELP_STRTOHEX_STRFROMHEX \
  _T("  // strfromhex (non-unicode version)") _T_RE_EOL \
  _T("  set s ~ strfromhex 0D 0A           // <CR><LF> = \\r\\n in C/C++") _T_RE_EOL \
  _T("  set s ~ strfromhex \"ab\" 09 \"c\" 0A  // ab<TAB>c<LF> = ab\\tc\\n in C/C++") _T_RE_EOL \
  _T("  // strtohex (non-unicode version)") _T_RE_EOL \
  _T("  set s ~ strtohex abc123   // 61 62 63 31 32 33") _T_RE_EOL \
  _T("  set s ~ strtohex \"ab 12\"  // 61 62 20 31 32") _T_RE_EOL \
  _T("  set s ~ strtohex ") _T_STRLEN_CYRILLIC _T("   // ") _T_STRHEX_CYRILLIC _T_RE_EOL
#endif

#ifdef UNICODE
  #define _T_HELP_CHR_ORD_ORDX \
  _T("  // chr")  _T_RE_EOL \
  _T("  set c ~ chr 0x71    // q")  _T_RE_EOL \
  _T("  set c ~ chr 113     // q")  _T_RE_EOL \
  _T("  set c ~ chr 0x0416  // \x0416")  _T_RE_EOL \
  _T("  // ord")  _T_RE_EOL \
  _T("  set n ~ ord /       // 47")  _T_RE_EOL \
  _T("  set n ~ ord q       // 113")  _T_RE_EOL \
  _T("  set n ~ ord \x0439       // 1081")  _T_RE_EOL \
  _T("  set n ~ ord \" \"     // 32")  _T_RE_EOL \
  _T("  set n ~ ord \"z\"     // 122")  _T_RE_EOL \
  _T("  // ordx")  _T_RE_EOL \
  _T("  set n ~ ordx /      // 0x2F")  _T_RE_EOL \
  _T("  set n ~ ordx q      // 0x71")  _T_RE_EOL \
  _T("  set n ~ ordx \x0416      // 0x0416")  _T_RE_EOL \
  _T("  set n ~ ordx \" \"    // 0x20")  _T_RE_EOL \
  _T("  set n ~ ordx \"z\"    // 0x7A")  _T_RE_EOL
#else
  #define _T_HELP_CHR_ORD_ORDX \
  _T("  // chr")  _T_RE_EOL \
  _T("  set c ~ chr 0x71    // q")  _T_RE_EOL \
  _T("  set c ~ chr 113     // q")  _T_RE_EOL \
  _T("  // ord")  _T_RE_EOL \
  _T("  set n ~ ord /       // 47")  _T_RE_EOL \
  _T("  set n ~ ord q     // 113")  _T_RE_EOL \
  _T("  set n ~ ord \" \"     // 32")  _T_RE_EOL \
  _T("  set n ~ ord \"z\"     // 122")  _T_RE_EOL \
  _T("  // ordx")  _T_RE_EOL \
  _T("  set n ~ ordx /      // 0x2F")  _T_RE_EOL \
  _T("  set n ~ ordx q    // 0x71")  _T_RE_EOL \
  _T("  set n ~ ordx \" \"    // 0x20")  _T_RE_EOL \
  _T("  set n ~ ordx \"z\"    // 0x7A")  _T_RE_EOL
#endif

#define _T_HELP_NPE_SEARCHFLAGS \
  _T("  The flags can be a combination of the following:") _T_RE_EOL \
  _T("    NPE_SF_MATCHCASE   = 0x00000001") _T_RE_EOL \
  _T("      - \"text\" finds only \"text\", not \"Text\" or \"TEXT\"") _T_RE_EOL \
  _T("    NPE_SF_WHOLEWORD   = 0x00000010") _T_RE_EOL \
  _T("      - \"word\" finds only \"word\", not \"sword\" or \"words\"") _T_RE_EOL \
  _T("    NPE_SF_WORDSTART   = 0x00000020") _T_RE_EOL \
  _T("      - \"word\" finds \"word\" and \"words\", not \"sword\"") _T_RE_EOL \
  _T("    NPE_SF_REGEXP      = 0x00000100") _T_RE_EOL \
  _T("      - search using a regular expression") _T_RE_EOL \
  _T("    NPE_SF_POSIX       = 0x00000200") _T_RE_EOL \
  _T("      - search using a POSIX-compatible regular expression") _T_RE_EOL \
  _T("    NPE_SF_CXX11REGEX  = 0x00000400") _T_RE_EOL \
  _T("      - search using a C++11 regular expression") _T_RE_EOL \
  _T("    NPE_SF_REGEXP_EMPTYMATCH_NOTAFTERMATCH = 0x00002000") _T_RE_EOL \
  _T("      - allows an empty match, not after another match") _T_RE_EOL \
  _T("    NPE_SF_REGEXP_EMPTYMATCH_ALL = 0x00004000") _T_RE_EOL \
  _T("      - allows an empty match") _T_RE_EOL \
  _T("    NPE_SF_REGEXP_EMPTYMATCH_ALLOWATSTART = 0x00008000") _T_RE_EOL \
  _T("      - allows an empty match, at start") _T_RE_EOL \
  _T("    NPE_SF_BACKWARD    = 0x00010000") _T_RE_EOL \
  _T("      - search backward (from the bottom to the top)") _T_RE_EOL \
  _T("    NPE_SF_NEXT        = 0x00020000") _T_RE_EOL \
  _T("      - search from current_position + 1") _T_RE_EOL \
  _T("    NPE_SF_INSELECTION = 0x00100000") _T_RE_EOL \
  _T("      - search only in the selected text") _T_RE_EOL \
  _T("    NPE_SF_INENTIRETEXT = 0x00200000") _T_RE_EOL \
  _T("      - search in the entire text, not only from the current position") _T_RE_EOL \
  _T("    NPE_SF_SETPOS      = 0x01000000") _T_RE_EOL \
  _T("      - move the caret to the position of the occurrence found") _T_RE_EOL \
  _T("    NPE_SF_SETSEL      = 0x02000000") _T_RE_EOL \
  _T("      - move the caret + select the occurrence found") _T_RE_EOL \
  _T("    NPE_SF_REPLACEALL  = 0x10000000  // only for sci_replace") _T_RE_EOL \
  _T("      - replace all the occurrences from the current pos to the end") _T_RE_EOL \
  _T("    NPE_SF_PRINTALL    = 0x20000000") _T_RE_EOL \
  _T("      - print all the occurrences from the current pos to the end")

#ifdef _DISABLE_CMD_ALIASES
  #define _T_DISABLE_CMD_ALIASES_MENU_ITEM _T("Corresponding menu item (inverse): Disable command aliases.")
#else
  #define _T_DISABLE_CMD_ALIASES_MENU_ITEM _T("There is no corresponding menu item.")
#endif

const tCmdItemInfo CONSOLE_CMD_INFO[] = {
  // HELP
  {
    CONSOLE_CMD_HELP,
    _T("COMMAND:  help") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  help") _T_RE_EOL \
    _T("  help <command>") _T_RE_EOL \
    _T("  help all") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Prints help ;-)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  help           // prints general help information") _T_RE_EOL \
    _T("  help npp_open  // prints detailed information about the NPP_OPEN command") _T_RE_EOL \
    _T("  help all       // prints detailed information about all available commands") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  manual, about, ver") _T_RE_EOL
  },

  // VER
  {
    CONSOLE_CMD_VER,
    _T("COMMAND:  ver") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  ver             (*)") _T_RE_EOL \
    _T("  * available in the Console dialog only") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Prints plugin\'s version") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  about, help, manual") _T_RE_EOL
  },

  // MANUAL
  {
    CONSOLE_CMD_MANUAL,
    _T("COMMAND:  manual") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  manual          (*)") _T_RE_EOL \
    _T("  * available in the Console dialog only") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Shows NppExec User Guide") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  manual  // shows NppExec User Guide") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  help, about, ver") _T_RE_EOL
  },

  // ABOUT
  {
    CONSOLE_CMD_ABOUT,
    _T("COMMAND:  about") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  about           (*)") _T_RE_EOL \
    _T("  * available in the Console dialog only") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Shows NppExec Help/About") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  about  // shows NppExec Help/About") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  ver, help, manual") _T_RE_EOL
  },

  // CLS
  {
    CScriptEngine::DoClsCommand::Name(),
    _T("COMMAND:  cls") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  cls") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Clears the Console screen") _T_RE_EOL
  },

  // CD
  {
    CScriptEngine::DoCdCommand::Name(),
    _T("COMMAND:  cd") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  cd") _T_RE_EOL \
    _T("  cd <relative_path>") _T_RE_EOL \
    _T("  cd <absolute_path>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Changes current directory (\"cd\" with parameters)") _T_RE_EOL \
    _T("  2. Shows current directory path (\"cd\" without parameters)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  cd       // shows current directory path") _T_RE_EOL \
    _T("  cd ..    // go to up-directory") _T_RE_EOL \
    _T("  cd \\     // go to root-directory") _T_RE_EOL \
    _T("  cd $(NPP_DIRECTORY)    // go to notepad++\'s directory") _T_RE_EOL \
    _T("  cd e:\\temp            // change drive and directory") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  dir") _T_RE_EOL
  },

  // DIR
  {
    CScriptEngine::DoDirCommand::Name(),
    _T("COMMAND:  dir") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  dir") _T_RE_EOL \
    _T("  dir <path>") _T_RE_EOL \
    _T("  dir <mask>") _T_RE_EOL \
    _T("  dir <path\\mask>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Lists all subdirs and files in current/specified directory  (*)") _T_RE_EOL \
    _T("  2. Lists subdirs and files matched the mask                   (**)") _T_RE_EOL \
    _T("   * current directory can be set by the \"cd\" command") _T_RE_EOL \
    _T("  ** type \"help mask\" for more information about file masks") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  dir                 // list all subdirs and files in current directory") _T_RE_EOL \
    _T("  dir *               // the same") _T_RE_EOL \
    _T("  dir d:\\docs\\*.txt   // list all *.txt files in the folder d:\\docs") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  cd, mask, npp_open") _T_RE_EOL
  },

  // ECHO
  {
    CScriptEngine::DoEchoCommand::Name(),
    _T("COMMAND:  echo") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  echo <text>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Prints a text in the Console (the environment variables are supported)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  echo Notepad++\'s directory is: $(NPP_DIRECTORY)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sleep, set") _T_RE_EOL
  },

  // CON_COLOUR
  {
    CScriptEngine::DoConColourCommand::Name(),
    _T("COMMAND:  con_colour") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  con_colour") _T_RE_EOL \
    _T("  con_colour fg = <RR GG BB>") _T_RE_EOL \
    _T("  con_colour bg = <RR GG BB>") _T_RE_EOL \
    _T("  con_colour fg = <RR GG BB> bg = <RR GG BB>") _T_RE_EOL \
    _T("  con_colour fg = 0") _T_RE_EOL \
    _T("  con_colour bg = 0") _T_RE_EOL \
    _T("  con_colour fg = 0 bg = 0") _T_RE_EOL \
    _T("  con_colour local fg = <RR GG BB> bg = <RR GG BB>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  fg - sets foreground (text) colour of the Console;") _T_RE_EOL \
    _T("  bg - sets background colour of the Console;") _T_RE_EOL \
    _T("  without parameters - shows current values of the colours;") _T_RE_EOL \
    _T("  the value of 0 restores the original colour(s).") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  con_colour bg=303030 fg=d0d0d0     // white text on dark-grey") _T_RE_EOL \
    _T("  con_colour fg = 303030 bg = D0D0D0 // dark text on light-grey") _T_RE_EOL \
    _T("  con_colour FG = 20 20 90           // dark-blue text") _T_RE_EOL \
    _T("  con_colour fg = 0                  // restore original text colour") _T_RE_EOL \
    _T("  con_colour local fg = 303030       // dark-grey text locally") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  The foreground (text) colour is applied to new text only.") _T_RE_EOL \
    _T("  The background colour is applied to the whole Console\'s background. And") _T_RE_EOL \
    _T("  when it is being applied, all the text is being re-coloured to the current") _T_RE_EOL \
    _T("  foreground colour.") _T_RE_EOL \
    _T("  <RR GG BB> are hex values of Right, Green and Blue components of a colour") _T_RE_EOL \
    _T("  to be used. Each colour component can have a value from 00 to FF. The value") _T_RE_EOL \
    _T("  of 00 means absence of the colour component, FF means maximum. Thus,") _T_RE_EOL \
    _T("  00 00 00 means \"dark\" (absence of all colours), 00 FF 00 means \"maximum of") _T_RE_EOL \
    _T("  green\", FF FF FF means \"maximum white\".") _T_RE_EOL \
    _T("  These colours are run-time only, they are not saved when Notepad++ exits.") _T_RE_EOL \
    _T("  (Use the Advanced Options to set and save the colours.)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  con_filter, npe_console") _T_RE_EOL
  },

  // CON_FILTER
  {
    CScriptEngine::DoConFilterCommand::Name(),
    _T("COMMAND:  con_filter") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  con_filter +x5/-x5 +i1/-i1 +fr4/-fr4 +frc1/-frc1 +h10/-h10") _T_RE_EOL \
    _T("  con_filter local ...") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Enables or disables the eXclude/Include/Replace/Highlight Filters") _T_RE_EOL \
    _T("  +x<N>/-x<N>     - enables/disables the Nth eXclude Mask (N = 1..5)") _T_RE_EOL \
    _T("  +i<N>/-i<N>     - enables/disables the Nth Include Mask (N = 1..5)") _T_RE_EOL \
    _T("  +fr<N>/-fr<N>   - enables/disables the Nth Replacing Filter and") _T_RE_EOL \
    _T("                    deactivates \"Match case\" (N = 1..4)") _T_RE_EOL \
    _T("  +frc<N>/-frc<N> - enables/disables the Nth Replacing Filter and") _T_RE_EOL \
    _T("                    activates \"Match case\" (N = 1..4)") _T_RE_EOL \
    _T("  +h<N>/-h<N>     - enables/disables the Nth Highlight Mask (N = 1..10)") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  con_filter +frc3 +fr2 -i5 +x2 +h1") _T_RE_EOL \
    _T("  con_filter -x4 +h2 +i1 -fr3 +i2 -h7 +x1 +fr1 +frc2") _T_RE_EOL \
    _T("  con_filter local -x4 +h2 +i1 -fr3  // has local effect") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  This command allows to enable and disable the Console Output Filters from") _T_RE_EOL \
    _T("  your script.") _T_RE_EOL \
    _T("  When any eXclude or Include mask is enabled, the \"Enable Console Output Filter\"") _T_RE_EOL \
    _T("  check-box becomes checked.") _T_RE_EOL \
    _T("  When any Replacing Filter is enabled, the \"Enable Replacing Filters\" check-box") _T_RE_EOL \
    _T("  becomes checked.") _T_RE_EOL \
    _T("  While using CON_FILTER to enable or disable the Console Output Filters and having") _T_RE_EOL \
    _T("  macro-variables as the values of these Filters, you are able to apply different") _T_RE_EOL \
    _T("  filters during execution of your script - thus having the full control over the") _T_RE_EOL \
    _T("  filters at runtime.") _T_RE_EOL \
    _T("  To illustrate the last sentence, let\'s assume you have specified the following") _T_RE_EOL \
    _T("  variables as filters in the Console Output Filters:") _T_RE_EOL \
    _T("    $(Exclude5) as the 5th Exclude mask;") _T_RE_EOL \
    _T("    $(Include5) as the 5th Include mask;") _T_RE_EOL \
    _T("    $(Find4) and $(Replace4) as the 4th Replacing Filter;") _T_RE_EOL \
    _T("    $(Highlight10) as the 10th Highlight mask.") _T_RE_EOL \
    _T("  Now you can write the following script:") _T_RE_EOL \
    _T("    // set the filters dynamically") _T_RE_EOL \
    _T("    set Exclude5 = *(C)*     // exclude any line that contains \"(C)\"") _T_RE_EOL \
    _T("    set Include5 = *         // include any line") _T_RE_EOL \
    _T("    set Find4 = /            // find all \'/\'...") _T_RE_EOL \
    _T("    set Replace4 = \\         // ...and replace with \'\\\'") _T_RE_EOL \
    _T("    set Highlight10 = error* // highlight any line that starts with \"error\"") _T_RE_EOL \
    _T("    con_filter +x5 +i5 +fr4 +h10  // enable the above filters") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  con_colour, npe_console, npp_console") _T_RE_EOL
  },

  // CON_LOADFROM
  {
    CScriptEngine::DoConLoadFromCommand::Name(),
    _T("COMMAND:  con_loadfrom") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  con_loadfrom <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Loads a file\'s content to the Console") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  con_loadfrom c:\\temp\\output.txt") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Unicode version of NppExec can read ANSI, UTF-8, UCS-2 LE and") _T_RE_EOL \
    _T("  UCS-2 BE text files") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  con_saveto") _T_RE_EOL
  }, 

  // CON_SAVETO
  {
    CScriptEngine::DoConSaveToCommand::Name(),
    _T("COMMAND:  con_saveto") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  con_saveto <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Saves the Console\'s content to a file") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  con_saveto c:\\temp\\output.txt") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Unicode version of NppExec saves the Console\'s content as") _T_RE_EOL \
    _T("  an Unicode text file (UCS-2 LE)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  con_loadfrom") _T_RE_EOL
  },
                   
  // NPP_CLOSE
  {
    CScriptEngine::DoNppCloseCommand::Name(),
    _T("COMMAND:  npp_close") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_close") _T_RE_EOL \
    _T("  npp_close <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Closes current file (\"npp_close\" without parameters)") _T_RE_EOL \
    _T("  2. Closes specified opened file") _T_RE_EOL \
    _T("     (supports a partial file path/name)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_close $(#3)  // closes 3rd file opened in Notepad++") _T_RE_EOL \
    _T("  npp_close awk    // closes a first file which contains \"awk\" in its name") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_open, npp_switch, npp_save") _T_RE_EOL
  },

  // NPP_CONSOLE
  {
    CScriptEngine::DoNppConsoleCommand::Name(),
    _T("COMMAND:  npp_console") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_console <on/off/keep>") _T_RE_EOL \
    _T("  npp_console <enable/disable>") _T_RE_EOL \
    _T("  npp_console <1/0/?>") _T_RE_EOL \
    _T("  npp_console <+/->") _T_RE_EOL \
    _T("  npp_console local ...") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Shows/hides the Console window.") _T_RE_EOL \
    _T("  Enables/disables output to the Console.") _T_RE_EOL \
    _T("  Usually the Console window is automatically opened each time you execute") _T_RE_EOL \
    _T("  some command or script, so the purpose of this command is to change the") _T_RE_EOL \
    _T("  default behaviour.") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_console ?  // keep the Console\'s state: hidden Console is not shown") _T_RE_EOL \
    _T("  npp_console 1  // open (show) the Console") _T_RE_EOL \
    _T("  npp_console 0  // close (hide) the Console") _T_RE_EOL \
    _T("  npp_console -  // disable output to the Console") _T_RE_EOL \
    _T("  npp_console +  // enable output to the Console") _T_RE_EOL \
    _T("  npp_console local 0  // close the Console locally") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  You can use \'NPP_CONSOLE ?\' as a first command of your script in order to") _T_RE_EOL \
    _T("  keep the Console\'s state: don\'t open hidden Console or don\'t hide opened one.") _T_RE_EOL \
    _T("  If you want to apply \'NPP_CONSOLE ?\' to every script, you don\'t have to add") _T_RE_EOL \
    _T("  this command to the beginning of each script. An auxiliary script can be created") _T_RE_EOL \
    _T("  in order to start every script using this auxiliary script:") _T_RE_EOL \
    _T("  ") _T_RE_EOL \
    _T("    // auxiliary script") _T_RE_EOL \
    _T("    NPP_CONSOLE ?  // keep the Console\'s state") _T_RE_EOL \
    _T("    NPP_EXEC $(ARGV)  // execute NppExec\'s script passed as the argument") _T_RE_EOL \
    _T("  ") _T_RE_EOL \
    _T("  Now you can start any script using") _T_RE_EOL \
    _T("    NPP_EXEC \"auxiliary script\" \"script to be executed\"") _T_RE_EOL \
    _T("  ") _T_RE_EOL \
    _T("  If you disable output to the Console via \'NPP_CONSOLE -\', you will not see any") _T_RE_EOL \
    _T("  output neither from internal nor from external commands until you enable it") _T_RE_EOL \
    _T("  via \'NPP_CONSOLE +\'.") _T_RE_EOL \
    _T("  If you call \'NPP_CONSOLE -\' before running a child process, you will not see") _T_RE_EOL \
    _T("  any prompt even if the proccess waits for user input. And \'NPP_CONSOLE +\'") _T_RE_EOL \
    _T("  will not work until the child process finishes.") _T_RE_EOL \
    _T("  Starting from NppExec v0.6, the command \'NPP_CONSOLE -\' has a \"local\" effect:") _T_RE_EOL \
    _T("  when this command is executed within a script, the Console output is enabled") _T_RE_EOL \
    _T("  automatically when the script ends. If, however, this script is executed via") _T_RE_EOL \
    _T("  NPP_EXEC, the Console remains disabled.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npe_console, proc_signal, @exit_cmd (\"help @exit_cmd\")") _T_RE_EOL
  },
  
  // NPP_EXEC
  {
    CScriptEngine::DoNppExecCommand::Name(),
    _T("COMMAND:  npp_exec") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_exec <script>") _T_RE_EOL \
    _T("  npp_exec <file>") _T_RE_EOL \
    _T("  npp_exec <script/file> <args>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Executes NppExec's commands from a specified script") _T_RE_EOL \
    _T("  2. Executes NppExec's commands from a specified file") _T_RE_EOL \
    _T("     (supports a partial file path/name)") _T_RE_EOL \
    _T("  3. Executes NppExec's commands from a specified script/file") _T_RE_EOL \
    _T("     and passes additional arguments to this script.") _T_RE_EOL \
    _T("     These arguments can be accessed in the script through") _T_RE_EOL \
    _T("     the following macro-variables: $(ARGC), $(ARGV),") _T_RE_EOL \
    _T("     $(ARGV[n]), $(RARGV), $(RARGV[n])") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_exec \"awk-run\"         // executes NppExec\'s script named \"awk-run\"") _T_RE_EOL \
    _T("  npp_exec \"..\\script.txt\"   // executes NppExec\'s file \"..\\script.txt\"") _T_RE_EOL \
    _T("  npp_exec \"..\\script\"       // the same (if there is no other files with similar name)") _T_RE_EOL \
    _T("  npp_exec \"script\" p1 \"p 2\" // in the script: $(ARGV[1]) = p1, $(ARGV[2]) = p 2") _T_RE_EOL \
    _T("  // An example with npp_exec <file>:") _T_RE_EOL \
    _T("  // 1. Create a new file and save it as e.g. \"test.nppexec\";") _T_RE_EOL \
    _T("  // 2. Type the following line in this file:") _T_RE_EOL \
    _T("  //      echo $(ARGV[1])") _T_RE_EOL \
    _T("  //    and save the file;") _T_RE_EOL \
    _T("  // 3. Now type the following in NppExec's Console:") _T_RE_EOL \
    _T("  //      npp_exec \"$(FULL_CURRENT_PATH)\" \"Hello, world!\"") _T_RE_EOL \
    _T("  // 4. Going further, a command alias can be assigned:") _T_RE_EOL \
    _T("  //      npe_cmdalias f = npp_exec \"$(FULL_CURRENT_PATH)\"") _T_RE_EOL \
    _T("  //      f \"Hello, world!\"") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Do _not_ use NPP_EXEC to start a batch file or an executable file in NppExec!") _T_RE_EOL \
    _T("  The purpose of NPP_EXEC is to execute NppExec's own script.") _T_RE_EOL \
    _T("  To start a batch file or an executable file in NppExec, type just:") _T_RE_EOL \
    _T("     application.exe // in case of an executable file named \"application.exe\"") _T_RE_EOL \
    _T("     batchfile.bat   // in case of a batch file named \"batchfile.bat\"") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_exectext") _T_RE_EOL
  },

  // NPP_EXECTEXT
  {
    CScriptEngine::DoNppExecTextCommand::Name(),
    _T("COMMAND:  npp_exectext") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_exectext <mode> <text>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Executes the given text as NppExec's script or sends this text to") _T_RE_EOL \
    _T("  the running child process as an input.") _T_RE_EOL \
    _T("  The value of <mode> can be either 0 or a sum of these flags:") _T_RE_EOL \
    _T("  0:  use the given text as is;") _T_RE_EOL \
    _T("      if there is a running child process then send the text to") _T_RE_EOL \
    _T("      that process as an input;") _T_RE_EOL \
    _T("  4:  (cs) if the first line of the text is \"!collateral\" and there is") _T_RE_EOL \
    _T("      no running child process then start a collateral script;") _T_RE_EOL \
    _T("  8:  (cp) if the first line of the text is \"!collateral\" and there is") _T_RE_EOL \
    _T("      a running child process then start a collateral script;") _T_RE_EOL \
    _T("  16: (ne) lines that start with the \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("\" prefix will be executed by") _T_RE_EOL \
    _T("      NppExec (as NppExec's script commands) and will not be sent to a") _T_RE_EOL \
    _T("      running child process;") _T_RE_EOL \
    _T("  64: (sv) share local variables: npp_exectext uses and updates the existing") _T_RE_EOL \
    _T("      local variables instead of its own local variables.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_exectext 0 $(CLIPBOARD_TEXT)") _T_RE_EOL \
    _T("  npp_exectext 28 $(SELECTED_TEXT) // process \"!collateral\" and \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("\"") _T_RE_EOL \
    _T("  set local A = 123") _T_RE_EOL \
    _T("  npp_exectext 0 echo A is $(A)") _T_RE_EOL \
    _T("  set local cmds ~ strunescape !collateral\\nmessagebox Hello!!!") _T_RE_EOL \
    _T("  npp_exectext 12 $(cmds) // process \"!collateral\"") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_exec") _T_RE_EOL
  },

  // NPP_MENUCOMMAND
  {
    CScriptEngine::DoNppMenuCommandCommand::Name(),
    _T("COMMAND:  npp_menucommand") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_menucommand <full name of menu item>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Executes (invokes) a menu item by its full name.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_menucommand File\\New") _T_RE_EOL \
    _T("  npp_menucommand Edit|Blank Operations|Trim Leading and Trailing Space") _T_RE_EOL \
    _T("  npp_menucommand Plugins/Plugin Manager/Show Plugin Manager") _T_RE_EOL \
    _T("  npp_menucommand Plugins\\\\NppExec\\\\Help/Manual") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  In general case, names of menu items are language-specific. If you have a") _T_RE_EOL \
    _T("  translated menu in Notepad++, you should use the menu item names you see.") _T_RE_EOL \
    _T("  (For example, \"npp_menucommand ") _T_MENUCOMMAND_CYRILLIC _T("\".)") _T_RE_EOL \
    _T("  The menu item names are processed case-sensitively, so \"file/new\" or") _T_RE_EOL \
    _T("  \"File|new\" does not correspond to the menu item \"File/New\".") _T_RE_EOL \
    _T("  You can use \'\\\', \'/\' and \'|\' as a separator. Moreover, you can double or") _T_RE_EOL \
    _T("  even triple the separator if a menu item already contains \'\\\', \'/\' or \'|\'.") _T_RE_EOL \
    _T("  However, be sure to use the very same separator within a full name of menu") _T_RE_EOL \
    _T("  item. For example: \"Plugins||NppExec||Help/Manual\" is correct since it uses") _T_RE_EOL \
    _T("  the same separator \"||\". And \"Plugins\\\\NppExec||Help/Manual\" is incorrect") _T_RE_EOL \
    _T("  because the first \"\\\\\" has been detected as a separator, and the remaining") _T_RE_EOL \
    _T("  part \"NppExec||Help/Manual\" would be treated as a name of one sub-item") _T_RE_EOL \
    _T("  since it did not contain the separator \"\\\\\".") _T_RE_EOL \
    _T("  Note: do not forget that \"//\" by default is a start of a comment in NppExec.") _T_RE_EOL \
    _T("  So do not use doubled \"//\" or tripled \"///\" as a separator of menu item name") _T_RE_EOL \
    _T("  because the remaining part of the item name (after \"//\" or \"///\") will be") _T_RE_EOL \
    _T("  treated as a comment and ignored.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_sendmsg") _T_RE_EOL
  },

  // NPP_OPEN
  {
    CScriptEngine::DoNppOpenCommand::Name(),
    _T("COMMAND:  npp_open") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_open <file>") _T_RE_EOL \
    _T("  npp_open <mask>") _T_RE_EOL \
    _T("  npp_open <path\\mask>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Opens or reopens specified file in Notepad++") _T_RE_EOL \
    _T("  2. Opens file(s) matched the mask                (*)") _T_RE_EOL \
    _T("  * type \"help mask\" for more information about file masks") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_open *.txt             // opens all *.txt files in current directory") _T_RE_EOL \
    _T("  npp_open e:\\temp\\file.txt  // opens the file \"e:\\temp\\file.txt\"") _T_RE_EOL \
    _T("  npp_open .\\text\\*.*        // opens all files in the folder \".\\text\"") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  To reopen a file, specify the full file path.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_close, npp_switch, npp_save, mask, dir") _T_RE_EOL
  },

  // NPP_RUN
  {
    CScriptEngine::DoNppRunCommand::Name(),
    _T("COMMAND:  npp_run") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_run <command>") _T_RE_EOL \
    _T("  npp_run <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. The same as Notepad++\'s Run command - runs an external process") _T_RE_EOL \
    _T("  2. Opens specified file in its associated program (requires NppExec v0.2 RC3.1 or higher)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_run calc.exe    (*)") _T_RE_EOL \
    _T("  * if you type just \"calc.exe\" without \"npp_run\", then NppExec runs") _T_RE_EOL \
    _T("    \"calc.exe\" as a child process and waits until it returns (i.e. until") _T_RE_EOL \
    _T("    you close the calc\'s window)") _T_RE_EOL \
    _T("  npp_run \"index.html\" // opens \"index.html\" in your default web-browser") _T_RE_EOL \
    _T("  npp_run \"C:\\Program Files\\Mozilla Firefox\\firefox.exe\" \"index.html\" // in Firefox") _T_RE_EOL \
    _T("  npp_run \"$(FULL_CURRENT_PATH)\" // opening using the associated program") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  In terms of an application (a program), NPP_RUN allows to run it in its own") _T_RE_EOL \
    _T("  window without waiting for its execution:") _T_RE_EOL \
    _T("    npp_run application.exe") _T_RE_EOL \
    _T("    npp_run application.exe <arguments>") _T_RE_EOL \
    _T("  If, however, there is a need to wait until the application has been executed,") _T_RE_EOL \
    _T("  while still running it in its own window, it can be achieved without NPP_RUN:") _T_RE_EOL \
    _T("    cmd /c start /wait application.exe") _T_RE_EOL \
    _T("    cmd /c start /wait application.exe <arguments>") _T_RE_EOL \
    _T("  In this last case, NppExec waits for cmd to be executed, while cmd itself") _T_RE_EOL \
    _T("  starts the application in its own separate window and waits until it has") _T_RE_EOL \
    _T("  been executed.") _T_RE_EOL
  },

  // NPP_SAVE
  {
    CScriptEngine::DoNppSaveCommand::Name(),
    _T("COMMAND:  npp_save") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_save") _T_RE_EOL \
    _T("  npp_save <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Saves current file (\"npp_save\" without parameters)") _T_RE_EOL \
    _T("  2. Saves specified file if it\'s opened in Notepad++") _T_RE_EOL \
    _T("     (supports a partial file path/name)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_save         // saves current file") _T_RE_EOL \
    _T("  npp_save $(#2)   // saves 2nd file opened in Notepad++") _T_RE_EOL \
    _T("  npp_save txt     // saves a first file which contains \"txt\" in its name") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_saveas, npp_saveall, npp_open") _T_RE_EOL
  },

  // NPP_SAVEALL
  {
    CScriptEngine::DoNppSaveAllCommand::Name(),
    _T("COMMAND:  npp_saveall") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_saveall") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Saves all modified files") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_saveall") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_save, npp_saveas, npp_open") _T_RE_EOL
  },

  // NPP_SAVEAS
  {
    CScriptEngine::DoNppSaveAsCommand::Name(),
    _T("COMMAND:  npp_saveas") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_saveas <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Saves current file with a new (path)name") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_saveas $(SYS.TEMP)\\newname.txt") _T_RE_EOL \
    _T("  npp_saveas anothername.txt") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_save, npp_saveall, npp_open") _T_RE_EOL
  },

  // NPP_SWITCH
  {
    CScriptEngine::DoNppSwitchCommand::Name(),
    _T("COMMAND:  npp_switch") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_switch <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Switches to specified opened file") _T_RE_EOL \
    _T("  (supports a partial file path/name)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_switch $(#3)  // activates 3rd file opened in Notepad++") _T_RE_EOL \
    _T("  npp_switch awk    // activates a first file which contains \"awk\" in its name") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_open, npp_close, npp_save") _T_RE_EOL
  },

  // NPP_SETFOCUS
  {
    CScriptEngine::DoNppSetFocusCommand::Name(),
    _T("COMMAND:  npp_setfocus") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_setfocus con") _T_RE_EOL \
    _T("  npp_setfocus sci") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sets the keyboard focus (to the Console or to Scintilla's editing window)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_setfocus con  // sets the focus to NppExec's Console") _T_RE_EOL \
    _T("  npp_setfocus sci  // sets the focus to the current Scintilla") _T_RE_EOL
  },

  // NPP_SENDMSG
  {
    CScriptEngine::DoNppSendMsgCommand::Name(),
    _T("COMMAND:  npp_sendmsg") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_sendmsg <msg>") _T_RE_EOL \
    _T("  npp_sendmsg <msg> <wparam>") _T_RE_EOL \
    _T("  npp_sendmsg <msg> <wparam> <lparam>") _T_RE_EOL \
    _T("   * Warning: incorrect usage of this command may crash Notepad++ !!!") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sends a message to Notepad++ (to its main window).") _T_RE_EOL \
    _T("  This command invokes Win API function SendMessage:") _T_RE_EOL \
    _T("    LRESULT SendMessage(hNppWnd, Msg, wParam, lParam)") _T_RE_EOL \
    _T("  where lParam or both lParam and wParam can be omitted.") _T_RE_EOL \
    _T("  <wparam> and <lparam> can be the following:") _T_RE_EOL \
    _T("    <int>    - integer value (int or DWORD)") _T_RE_EOL \
    _T("    @<int>   - pointer to integer (LPDWORD -> signed int)") _T_RE_EOL \
    _T("    \"<str>\"  - const string (LPCTSTR)") _T_RE_EOL \
    _T("    @\"<str>\" - pointer to string (LPTSTR)") _T_RE_EOL \
    _T("  Use <int> and \"<str>\" to pass integer or string to Notepad++:") _T_RE_EOL \
    _T("  e.g. 10, 0x1E, \"some text\".") _T_RE_EOL \
    _T("  Use @<int> and @\"str\" to get integer or string from Notepad++:") _T_RE_EOL \
    _T("  e.g. @0, @0x7FF, @\"\", @\"initial string\".") _T_RE_EOL \
    _T("  When using @<int> and @\"<str>\", the values specified are") _T_RE_EOL \
    _T("  initial values of the parameters.") _T_RE_EOL \
    _T("  NppExec reserves a buffer of (4 MB)*sizeof(TCHAR) for @\"<str>\".") _T_RE_EOL \
    _T("  When omitting <wparam> or <lparam>, NppExec treats the omitted") _T_RE_EOL \
    _T("  parameter(s) as 0 (0 as int).") _T_RE_EOL \
    _T("  This command sets the following local variables:") _T_RE_EOL \
    _T("    $(MSG_RESULT) - LRESULT as int") _T_RE_EOL \
    _T("    $(MSG_WPARAM) - int or string for @<int> or @\"<str>\", otherwise empty") _T_RE_EOL \
    _T("    $(MSG_LPARAM) - int or string for @<int> or @\"<str>\", otherwise empty") _T_RE_EOL \
    _T("  See MSDN for more details about SendMessage.") _T_RE_EOL \
    _T("  See Notepad++\'es sources and documentation for more details") _T_RE_EOL \
    _T("  about messages supported by Notepad++.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // !!! All NPPM_* messages must be specified in upper case !!!") _T_RE_EOL \
    _T("  // !!! (These messages are read from NppExec\\Notepad_plus_msgs.h) !!!") _T_RE_EOL \
    _T("  // NPPM_GETNPPVERSION example:") _T_RE_EOL \
    _T("  npp_sendmsg NPPM_GETNPPVERSION  // returns ver as LRESULT") _T_RE_EOL \
    _T("  set local hver ~ trunc($(MSG_RESULT)/65536)  // HIWORD(ver)") _T_RE_EOL \
    _T("  set local lver ~ $(MSG_RESULT) - $(hver)*65536  // LOWORD(ver)") _T_RE_EOL \
    _T("  echo $(hver).$(lver)  // prints version as string") _T_RE_EOL \
    _T("  // NPPM_GETPLUGINSCONFIGDIR example:") _T_RE_EOL \
    _T("  npp_sendmsg NPPM_GETPLUGINSCONFIGDIR 4000000 @\"\"  // send a message") _T_RE_EOL \
    _T("  echo $(MSG_LPARAM)  // prints the string pointed by lParam") _T_RE_EOL \
    _T("  // NPPM_GETFULLPATHFROMBUFFERID _advanced_ example:") _T_RE_EOL \
    _T("  npp_sendmsg NPPM_GETCURRENTBUFFERID  // returns buffer Id as result") _T_RE_EOL \
    _T("  set local bufferId = $(MSG_RESULT)") _T_RE_EOL \
    _T("  npp_sendmsg NPPM_GETFULLPATHFROMBUFFERID $(bufferId) @\"\"") _T_RE_EOL \
    _T("  echo $(MSG_LPARAM)  // file path as string") _T_RE_EOL \
    _T("  npp_sendmsg NPPM_GETFULLPATHFROMBUFFERID $(bufferId) @[]") _T_RE_EOL \
    _T("  echo $(MSG_LPARAM)  // file path as hex string") _T_RE_EOL \
    _T("  // (this last example uses undocumented ability of") _T_RE_EOL \
    _T("  //  handling of hex-string parameters, e.g.:") _T_RE_EOL \
    _T("  //  [00 7F EA 00] - const buffer of 4 bytes") _T_RE_EOL \
    _T("  //  @[FF 02 0C]   - pointer to buffer") _T_RE_EOL \
    _T("  //  Use it with care!)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sci_sendmsg, npp_sendmsgex, npe_sendmsgbuflen, set") _T_RE_EOL
  },

  // NPP_SENDMSGEX
  {
    CScriptEngine::DoNppSendMsgExCommand::Name(),
    _T("COMMAND:  npp_sendmsgex") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npp_sendmsgex <hwnd> <msg>") _T_RE_EOL \
    _T("  npp_sendmsgex <hwnd> <msg> <wparam>") _T_RE_EOL \
    _T("  npp_sendmsgex <hwnd> <msg> <wparam> <lparam>") _T_RE_EOL \
    _T("   * Warning: incorrect usage of this command may crash something !!!") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sends a message to hwnd.") _T_RE_EOL \
    _T("  This command invokes Win API function SendMessage:") _T_RE_EOL \
    _T("    LRESULT SendMessage(hWnd, Msg, wParam, lParam)") _T_RE_EOL \
    _T("  where lParam or both lParam and wParam can be omitted.") _T_RE_EOL \
    _T("  See NPP_SENDMSG and SCI_SENDMSG for more details.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npp_sendmsg NPPM_DMMGETPLUGINHWNDBYNAME \"NppExec Console\" \"NppExec.dll\"") _T_RE_EOL \
    _T("  set local hwnd = $(MSG_RESULT)  // hwnd of NppExec\'s Console") _T_RE_EOL \
    _T("  npp_sendmsgex $(hwnd) WM_COMMAND 1154 0  // Word-Wrap checkbox") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_sendmsg, sci_sendmsg, npe_sendmsgbuflen, set") _T_RE_EOL
  },

  // SCI_SENDMSG
  {
    CScriptEngine::DoSciSendMsgCommand::Name(),
    _T("COMMAND:  sci_sendmsg") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sci_sendmsg <msg>") _T_RE_EOL \
    _T("  sci_sendmsg <msg> <wparam>") _T_RE_EOL \
    _T("  sci_sendmsg <msg> <wparam> <lparam>") _T_RE_EOL \
    _T("   * Warning: incorrect usage of this command may crash Notepad++ !!!") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sends a message to current Scintilla (to its window).") _T_RE_EOL \
    _T("  This command invokes Win API function SendMessage:") _T_RE_EOL \
    _T("    LRESULT SendMessage(hSciWnd, Msg, wParam, lParam)") _T_RE_EOL \
    _T("  where lParam or both lParam and wParam can be omitted.") _T_RE_EOL \
    _T("  <wparam> and <lparam> can be the following:") _T_RE_EOL \
    _T("    <int>    - integer value (int or DWORD)") _T_RE_EOL \
    _T("    @<int>   - pointer to integer (LPDWORD -> signed int)") _T_RE_EOL \
    _T("    \"<str>\"  - const string (LPCTSTR)") _T_RE_EOL \
    _T("    @\"<str>\" - pointer to string (LPTSTR)") _T_RE_EOL \
    _T("  Use <int> and \"<str>\" to pass integer or string to Scintilla:") _T_RE_EOL \
    _T("  e.g. 10, 0x1E, \"some text\".") _T_RE_EOL \
    _T("  Use @<int> and @\"str\" to get integer or string from Scintilla:") _T_RE_EOL \
    _T("  e.g. @0, @0x7FF, @\"\", @\"initial string\".") _T_RE_EOL \
    _T("  When using @<int> and @\"<str>\", the values specified are") _T_RE_EOL \
    _T("  initial values of the parameters.") _T_RE_EOL \
    _T("  NppExec reserves a buffer of (4 MB)*sizeof(char) for @\"<str>\".") _T_RE_EOL \
    _T("  NppExec performs conversion from/to ANSI or UTF-8 automatically.") _T_RE_EOL \
    _T("  When omitting <wparam> or <lparam>, NppExec treats the omitted") _T_RE_EOL \
    _T("  parameter(s) as 0 (0 as int).") _T_RE_EOL \
    _T("  This command sets the following local variables:") _T_RE_EOL \
    _T("    $(MSG_RESULT) - LRESULT as int") _T_RE_EOL \
    _T("    $(MSG_WPARAM) - int or string for @<int> or @\"<str>\", otherwise empty") _T_RE_EOL \
    _T("    $(MSG_LPARAM) - int or string for @<int> or @\"<str>\", otherwise empty") _T_RE_EOL \
    _T("  See MSDN for more details about SendMessage.") _T_RE_EOL \
    _T("  See Scintilla\'s documentation for more details about messages") _T_RE_EOL \
    _T("  supported by Scintilla.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // !!! All SCI_* messages must be specified in upper case !!!") _T_RE_EOL \
    _T("  // !!! (These messages are read from NppExec\\Scintilla.h) !!!") _T_RE_EOL \
    _T("  // get the text of the first visible line:") _T_RE_EOL \
    _T("  sci_sendmsg SCI_GETFIRSTVISIBLELINE  // returns line number as LRESULT") _T_RE_EOL \
    _T("  sci_sendmsg SCI_GETLINE $(MSG_RESULT) @\"\"") _T_RE_EOL \
    _T("  echo $(MSG_LPARAM)  // prints the string pointed by lParam") _T_RE_EOL \
    _T("  // insert some text at the current position:") _T_RE_EOL \
    _T("  sci_sendmsg SCI_INSERTTEXT -1 \"Some text\"") _T_RE_EOL \
    _T("  // insert some multi-line text:") _T_RE_EOL \
    _T("  set local s ~ strunescape 123\\n456\\n789") _T_RE_EOL \
    _T("  sci_sendmsg SCI_INSERTTEXT -1 \"$(s)\"") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_sendmsg, npp_sendmsgex, npe_sendmsgbuflen, set") _T_RE_EOL
  },

  // SCI_FIND
  {
    CScriptEngine::DoSciFindCommand::Name(),
    _T("COMMAND:  sci_find") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sci_find <flags> <find_what>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Searches for the given string according to the specified flags.") _T_RE_EOL \
    _T("  The actual behavior completely depends on the <flags> specified.") _T_RE_EOL \
    _T("  It is recommended to enquote the \"find_what\" string.") _T_RE_EOL \
    _T("  Use the \"Bitwise OR\" operator '|' to combine the flags.") _T_RE_EOL \
    _T_HELP_NPE_SEARCHFLAGS _T_RE_EOL \
    _T("  This command sets the following local variables:") _T_RE_EOL \
    _T("    $(MSG_RESULT) - the position of the occurrence found, or -1") _T_RE_EOL \
    _T("    $(MSG_WPARAM) - <find_what>'s length in Scintilla's chars (*)") _T_RE_EOL \
    _T("    * in case of regular expression, it is the length of the matched string") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // search for \"some text\", starting from the caret position:") _T_RE_EOL \
    _T("  sci_find 0 \"some text\"") _T_RE_EOL \
    _T("  // search backward (from the caret position), select if found:") _T_RE_EOL \
    _T("  sci_find 0x02010000 \"some text\"") _T_RE_EOL \
    _T("  sci_find NPE_SF_BACKWARD|NPE_SF_SETSEL \"some text\"  // the same") _T_RE_EOL \
    _T("  sci_find \"NPE_SF_BACKWARD | NPE_SF_SETSEL\" \"some text\"  // the same") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_BACKWARD | NPE_SF_SETSEL") _T_RE_EOL \
    _T("  sci_find $(flags) \"some text\"  // the same") _T_RE_EOL \
    _T("  // search for a regular expression in the entire text, select if found:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_REGEXP | NPE_SF_INENTIRETEXT | NPE_SF_SETSEL") _T_RE_EOL \
    _T("  sci_find $(flags) \"[0-9]+\"") _T_RE_EOL \
    _T("  // find all empty lines:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INENTIRETEXT | NPE_SF_REGEXP | NPE_SF_REGEXP_EMPTYMATCH_ALL | NPE_SF_PRINTALL") _T_RE_EOL \
    _T("  sci_find $(flags) \"^$\"") _T_RE_EOL \
    _T("  // find all non-empty lines:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INENTIRETEXT | NPE_SF_REGEXP | NPE_SF_PRINTALL") _T_RE_EOL \
    _T("  sci_find $(flags) \"^.+$\"") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  With NPE_SF_PRINTALL, it is possible to achieve a functionality similar to") _T_RE_EOL \
    _T("  (but not equal to) the standard \"Find result\" window.") _T_RE_EOL \
    _T("  Let's assume you want to find all occurrences of \"abc\":") _T_RE_EOL \
    _T("    sci_find NPE_SF_INENTIRETEXT|NPE_SF_PRINTALL \"abc\"") _T_RE_EOL \
    _T("  This will print all the occurrences of \"abc\" in the Console in a form of:") _T_RE_EOL \
    _T("    (<line>,<column>)\t abc") _T_RE_EOL \
    _T("  In case of searching with regular expressions, e.g.") _T_RE_EOL \
    _T("    sci_find NPE_SF_INENTIRETEXT|NPE_SF_REGEXP|NPE_SF_PRINTALL \"WM_[A-Z_]+\"") _T_RE_EOL \
    _T("  the results will be printed in a form of:") _T_RE_EOL \
    _T("    (<line>,<column>)\t <matched_string>") _T_RE_EOL \
    _T("  The \"(<line>,<column>)\" part and the matched string part are always separated") _T_RE_EOL \
    _T("  by two characters: a tabulation character '\\t' plus a space character ' '.") _T_RE_EOL \
    _T("  So the following Highlight Mask can be specified for the search results:") _T_RE_EOL \
    _T("    (%LINE%,%CHAR%)? *") _T_RE_EOL \
    _T("  This will allow to go to the occurrence by simple double-clicking.") _T_RE_EOL \
    _T("  (A Highlight Mask can be specified either directly in the Console Filters") _T_RE_EOL \
    _T("  dialog or using the technique described in \"help con_filter\".)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sci_replace, sci_sendmsg") _T_RE_EOL
  },

  // SCI_REPLACE
  {
    CScriptEngine::DoSciReplaceCommand::Name(),
    _T("COMMAND:  sci_replace") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sci_replace <flags> <find_what> <replace_with>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Searches for the given string according to the specified flags and replaces it.") _T_RE_EOL \
    _T("  The actual behavior completely depends on the <flags> specified.") _T_RE_EOL \
    _T("  It is recommended to enquote the \"find_what\" and \"replace_with\" strings.") _T_RE_EOL \
    _T("  Use the \"Bitwise OR\" operator '|' to combine the flags.") _T_RE_EOL \
    _T_HELP_NPE_SEARCHFLAGS _T_RE_EOL \
    _T("  This command sets the following local variables:") _T_RE_EOL \
    _T("    $(MSG_RESULT) - the position of the occurrence found, or -1") _T_RE_EOL \
    _T("    $(MSG_WPARAM) - <find_what>'s length in Scintilla's chars (*)") _T_RE_EOL \
    _T("    $(MSG_LPARAM) - <replace_with>'s length in Scintilla's chars (*)") _T_RE_EOL \
    _T("    * in case of regular expression, it is the length of the matched string") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // replace the first occurrence of \"some text\" with nothing:") _T_RE_EOL \
    _T("  sci_replace 0 \"some text\"") _T_RE_EOL \
    _T("  // replace all occurrences of \"some text\" with nothing:") _T_RE_EOL \
    _T("  sci_replace NPE_SF_INENTIRETEXT|NPE_SF_REPLACEALL \"some text\"") _T_RE_EOL \
    _T("  // replace all occurrences of \"some text\" with \"other text\":") _T_RE_EOL \
    _T("  sci_replace NPE_SF_INENTIRETEXT|NPE_SF_REPLACEALL \"some text\" \"other text\"") _T_RE_EOL \
    _T("  // replace all occurrences in the selection, searching backward:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INSELECTION | NPE_SF_REPLACEALL | NPE_SF_BACKWARD") _T_RE_EOL \
    _T("  sci_replace $(flags) \"some text\" \"other text\"") _T_RE_EOL \
    _T("  // replace using a regular expression, searching backward:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_BACKWARD | NPE_SF_REGEXP") _T_RE_EOL \
    _T("  sci_replace $(flags) \"([a-z]+)-([a-z]+)\" \"\\2-\\1\"") _T_RE_EOL \
    _T("  // the same:") _T_RE_EOL \
    _T("  sci_replace 0x00010100 \"([a-z]+)-([a-z]+)\" \"\\2-\\1\"") _T_RE_EOL \
    _T("  // the same:") _T_RE_EOL \
    _T("  sci_replace NPE_SF_BACKWARD|NPE_SF_REGEXP \"([a-z]+)-([a-z]+)\" \"\\2-\\1\"") _T_RE_EOL \
    _T("  // replace using a regular expression, select if found:") _T_RE_EOL \
    _T("  sci_replace NPE_SF_REGEXP|NPE_SF_SETSEL \"([a-z]+)-([a-z]+)\" \"\\2-\\1\"") _T_RE_EOL \
    _T("  // insert \"abc\" at the beginning of every line:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INENTIRETEXT | NPE_SF_REGEXP | NPE_SF_REGEXP_EMPTYMATCH_ALL | NPE_SF_REPLACEALL") _T_RE_EOL \
    _T("  sci_replace $(flags) \"^\" \"abc\"") _T_RE_EOL \
    _T("  // append \"xyz\" to every line:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INENTIRETEXT | NPE_SF_REGEXP | NPE_SF_REGEXP_EMPTYMATCH_ALL | NPE_SF_REPLACEALL") _T_RE_EOL \
    _T("  sci_replace $(flags) \"$\" \"xyz\"") _T_RE_EOL \
    _T("  // append \"xyz\" to every non-empty line:") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INENTIRETEXT | NPE_SF_REGEXP | NPE_SF_REPLACEALL") _T_RE_EOL \
    _T("  sci_replace $(flags) \"^(.+)$\" \"\\1xyz\"") _T_RE_EOL \
    _T("  // replace every empty line with \"EMPTY\":") _T_RE_EOL \
    _T("  set local flags ~ NPE_SF_INENTIRETEXT | NPE_SF_REGEXP | NPE_SF_REGEXP_EMPTYMATCH_ALL | NPE_SF_REPLACEALL") _T_RE_EOL \
    _T("  sci_replace $(flags) \"^$\" \"EMPTY\"") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Combining NPE_SF_REPLACEALL with NPE_SF_PRINTALL, it is possible to see all") _T_RE_EOL \
    _T("  the replaced occurrences in the Console (see also: sci_find).") _T_RE_EOL \
    _T("  Using NPE_SF_PRINTALL alone, without NPE_SF_REPLACEALL, it is possible to see") _T_RE_EOL \
    _T("  all the found occurrences in the Console, with only the first occurrence") _T_RE_EOL \
    _T("  replaced.") _T_RE_EOL \
    _T("  The combination of NPE_SF_REPLACEALL|NPE_SF_PRINTALL|NPE_SF_BACKWARD will") _T_RE_EOL \
    _T("  most likely lead to incorrect character positions printed in the Console.") _T_RE_EOL \
    _T("  Why? Because each replacement with a string of different length changes the") _T_RE_EOL \
    _T("  length of the entire text. And while the replacements are being done from") _T_RE_EOL \
    _T("  the bottom to the top, each new replacement changes the offsets of all the") _T_RE_EOL \
    _T("  strings below it, affecting the positions of the occurrences that have") _T_RE_EOL \
    _T("  already been printed in the Console. (And yes, it proves that when you tell") _T_RE_EOL \
    _T("  sci_replace to replace in the backward direction, it really does it in the") _T_RE_EOL \
    _T("  backward direction.)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sci_find, sci_sendmsg") _T_RE_EOL
  },

  // MASK
  {
    _T("MASK"),
    _T("DESCRIPTION:  file mask") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  File masks are used by such commands as DIR and NPP_OPEN") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  The file mask in NppExec is an orthographical construction where") _T_RE_EOL \
    _T("    ? means any single character and") _T_RE_EOL \
    _T("    * means any 0 or more characters") _T_RE_EOL \
    _T("  regardless of such concepts as file name or file extension.") _T_RE_EOL \
    _T("  Thus, *.* in NppExec DOES NOT match any file or directory name -") _T_RE_EOL \
    _T("  it matches any string which contains \'.\'") _T_RE_EOL \
    _T("  Also, *tx*.* does not match \"file.txt\" because \"tx\" must be") _T_RE_EOL \
    _T("  located BEFORE \'.\' and not AFTER it.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  dir *.cpp         // prints all *.cpp files in current directory") _T_RE_EOL \
    _T("  npp_open *dlg*.*  // opens all files which contain \"dlg\" and \'.\' in its name") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  dir, npp_open") _T_RE_EOL
  },

  // SET
  {
    CScriptEngine::DoSetCommand::Name(),
    _T("COMMAND:  set/unset") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  1.  set") _T_RE_EOL \
    _T("  2.  set <var>") _T_RE_EOL \
    _T("  3.  set <var> = <value>") _T_RE_EOL \
    _T("      set $(<var>) = <value>") _T_RE_EOL \
    _T("  4.  set <var> ~ <math expression>") _T_RE_EOL \
    _T("  5a. set <var> ~ strlen <string>") _T_RE_EOL \
    _T("  5b. set <var> ~ strlenutf8 <string>") _T_RE_EOL \
    _T("  5c. set <var> ~ strlensci <string>") _T_RE_EOL \
    _T("  5d. set <var> ~ strupper <string>") _T_RE_EOL \
    _T("  5e. set <var> ~ strlower <string>") _T_RE_EOL \
    _T("  5f. set <var> ~ substr <pos> <len> <string>") _T_RE_EOL \
    _T("  5g. set <var> ~ strfind <string> <sfind>") _T_RE_EOL \
    _T("  5h. set <var> ~ strrfind <string> <sfind>") _T_RE_EOL \
    _T("  5i. set <var> ~ strreplace <string> <sfind> <sreplace>") _T_RE_EOL \
    _T("  5j. set <var> ~ strquote <string>") _T_RE_EOL \
    _T("  5k. set <var> ~ strunquote <string>") _T_RE_EOL \
    _T("  5l. set <var> ~ strescape <string>") _T_RE_EOL \
    _T("  5m. set <var> ~ strunescape <string>") _T_RE_EOL \
    _T("  5n. set <var> ~ strexpand <string>") _T_RE_EOL \
    _T("  5o. set <var> ~ strfromhex <hexstring>") _T_RE_EOL \
    _T("  5p. set <var> ~ strtohex <string>") _T_RE_EOL \
    _T("  5q. set <var> ~ chr <n>") _T_RE_EOL \
    _T("  5r. set <var> ~ ord <c>") _T_RE_EOL \
    _T("  5s. set <var> ~ ordx <c>") _T_RE_EOL \
    _T("  5t. set <var> ~ normpath <path>") _T_RE_EOL \
    _T("  5u. set <var> ~ fileexists <path>") _T_RE_EOL \
    _T("  5v. set <var> ~ direxists <path>") _T_RE_EOL \
    _T("  6.  set local") _T_RE_EOL \
    _T("      set local <var>") _T_RE_EOL \
    _T("      set local <var> = ...") _T_RE_EOL \
    _T("      set local <var> ~ ...") _T_RE_EOL \
    _T("  7.  set +v <var> = ...") _T_RE_EOL \
    _T("      set +v local <var> = ...") _T_RE_EOL \
    _T("  8.  unset <var>") _T_RE_EOL \
    _T("      unset <var> = <value>") _T_RE_EOL \
    _T("  9.  unset local <var>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1.  Shows all user\'s variables (\"set\" without parameters)") _T_RE_EOL \
    _T("  2.  Shows the value of user\'s variable (\"set\" without \"=\")") _T_RE_EOL \
    _T("  3.  Sets the value of user\'s variable (\"set <var> = <value>\")") _T_RE_EOL \
    _T("  4.  Calculates the math expression (\"set <var> ~ <math expr>\")") _T_RE_EOL \
    _T("  5a. Calculates the string length (\"set <var> ~ strlen <string>\")") _T_RE_EOL \
    _T("  5b. Calculates the UTF-8 string length (\"set <var> ~ strlenutf8 <s>\")") _T_RE_EOL \
    _T("  5c. Calculates Scintilla's string length (\"set <var> ~ strlensci <s>\")") _T_RE_EOL \
    _T("      (depending on Scintilla's encoding, equals to strlen or strlenutf8)") _T_RE_EOL \
    _T("  5d. Returns the string in upper case (\"set <var> ~ strupper <s>\")") _T_RE_EOL \
    _T("  5e. Returns the string in lower case (\"set <var> ~ strlower <s>\")") _T_RE_EOL \
    _T("  5f. Returns the substring (\"substr <pos> <len> <s>\")") _T_RE_EOL \
    _T("  5g. Returns the position of first <sfind> in <string>") _T_RE_EOL \
    _T("  5h. Returns the position of last <sfind> in <string>") _T_RE_EOL \
    _T("  5i. Replaces all <sfind> with <sreplace> in <string>") _T_RE_EOL \
    _T("  5j. Returns the string surrounded with \"\" quotes") _T_RE_EOL \
    _T("  5k. Removes the surrounding \"\" quotes") _T_RE_EOL \
    _T("  5l. Simple character escaping: '\\' -> '\\\\', '<TAB>' -> '\\t',") _T_RE_EOL \
    _T("      '<CR>' -> '\\r', '<LF>' -> '\\n', '\"' -> '\\\"'") _T_RE_EOL \
    _T("  5m. Simple character unescaping: '\\\\' -> '\\', '\\t' -> '<TAB>',") _T_RE_EOL \
    _T("      '\\r' -> '<CR>', '\\n' -> '<LF>', '\\?' -> '?'") _T_RE_EOL \
    _T("  5n. Expands all $(sub) values within the <string>") _T_RE_EOL \
    _T("  5o. Returns a string from the <hexstring>") _T_RE_EOL \
    _T("  5p. Returns a hex-string from the <string>") _T_RE_EOL \
    _T("  5q. Returns a character from a character code <n>") _T_RE_EOL \
    _T("  5r. Returns a decimal character code of a character <c>") _T_RE_EOL \
    _T("  5s. Returns a hexadecimal character code of a character <c>") _T_RE_EOL \
    _T("  5t. Returns a normalized path") _T_RE_EOL \
    _T("  5u. Returns 1 if a given file exists, 0 otherwise") _T_RE_EOL \
    _T("  5v. Returns 1 if a given directory exists, 0 otherwise") _T_RE_EOL \
    _T("  6.  Shows/sets the value of local variable (\"set local <var> ...\")") _T_RE_EOL \
    _T("  7.  Sets a value using delayed substitution of variables (\"set +v ...\")") _T_RE_EOL \
    _T("  8.  Removes the variable <var> (\"unset <var>\")") _T_RE_EOL \
    _T("  9.  Removes the local variable <var> (\"unset local <var>\")") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  set p = C:\\Program Files") _T_RE_EOL \
    _T("  dir $(p)\\*  // the same as dir C:\\Program Files\\*") _T_RE_EOL \
    _T("  unset p     // removes the variable p") _T_RE_EOL \
    _T("  // set vs. set local:") _T_RE_EOL \
    _T("  set x = 10       // 10: global x") _T_RE_EOL \
    _T("  set local x = 20 // 20: local x") _T_RE_EOL \
    _T("  echo $(x)        // 20: substitutes the overridden (local) x") _T_RE_EOL \
    _T("  set x            // 10: prints global x") _T_RE_EOL \
    _T("  set local x      // 20: prints local x") _T_RE_EOL \
    _T("  unset local x    // 20: deletes local x") _T_RE_EOL \
    _T("  echo $(x)        // 10: substitutes the global x") _T_RE_EOL \
    _T("  // indirect variable reference:") _T_RE_EOL \
    _T("  set local a = 123") _T_RE_EOL \
    _T("  set local b = a") _T_RE_EOL \
    _T("  echo $($(b))           // prints 123 : $($(b)) = $(a) = 123") _T_RE_EOL \
    _T("  set local c = $($(b))  // $(c) = 123 : $($(b)) = $(a) = 123") _T_RE_EOL \
    _T("  unset local $($(b))    // deletes $(a)") _T_RE_EOL \
    _T("  // delayed substitution of variables, modern way:") _T_RE_EOL \
    _T("  set +v local fname = $(FILE_NAME)  // $(fname) = $(FILE_NAME)") _T_RE_EOL \
    _T("  set local s ~ strexpand $(fname)  // the actual file name") _T_RE_EOL \
    _T("  // delayed substitution of variables, old way:") _T_RE_EOL \
    _T("  set local s = $") _T_RE_EOL \
    _T("  set local cmd = echo $(s)(PID)  // $(cmd) = echo $(PID)") _T_RE_EOL \
    _T("  ") DEFAULT_NPPEXEC_CMD_PREFIX _T("$(cmd)  // will print a running process'es id") _T_RE_EOL \
    _T("  // calculations:") _T_RE_EOL \
    _T("  set ans ~ 1 + 2*(3 + 4) - 0x5  // calculates the expression") _T_RE_EOL \
    _T("  set ans ~ 0x001 | 0x010 | 0x100  // calculates the expression") _T_RE_EOL \
    _T("  set ans ~ hex(0x001 | 0x010 | 0x100)  // calculates as hexadecimal") _T_RE_EOL \
    _T("  // strlen:") _T_RE_EOL \
    _T("  set n ~ strlen   ABC D   // sets n=5 (skipping leading & trailing spaces)") _T_RE_EOL \
    _T("  set n ~ strlen \"  ABC \"  // sets n=8 (including spaces & double quotes)") _T_RE_EOL \
    _T("  // in case of non-Latin characters, strlenutf8 may differ from strlen:") _T_RE_EOL \
    _T("  set n ~ strlenutf8 ") _T_STRLEN_CYRILLIC _T("  // sets n=12 (number of UTF-8 bytes)") _T_RE_EOL \
    _T("  set n ~ strlensci ") _T_STRLEN_CYRILLIC _T("  // sets n=6 for Scintilla with multibyte encoding") _T_RE_EOL \
    _T("  set n ~ strlensci ") _T_STRLEN_CYRILLIC _T("  // sets n=12 for Scintilla with UTF-8 encoding") _T_RE_EOL \
    _T("  // strupper/strlower:") _T_RE_EOL \
    _T("  set s = Text") _T_RE_EOL \
    _T("  set t = $(s)           // t = Text") _T_RE_EOL \
    _T("  set t ~ strupper $(s)  // t = TEXT") _T_RE_EOL \
    _T("  set t ~ strlower $(s)  // t = text") _T_RE_EOL \
    _T("  // substr:") _T_RE_EOL \
    _T("  set s ~ substr 0 3  abcde  // abc (3 characters from position 0)") _T_RE_EOL \
    _T("  set s ~ substr 2 2  abcde  // cd (2 characters from position 2)") _T_RE_EOL \
    _T("  set s ~ substr 3 -  abcde  // de (substring from position 3 to the end)") _T_RE_EOL \
    _T("  set s ~ substr 1 -2 abcde  // bc (substring from position 1 to length-2)") _T_RE_EOL \
    _T("  set s ~ substr -3 - abcde  // cde (substring from position length-3 to the end)") _T_RE_EOL \
    _T("  set s ~ substr -2 1 abcde  // d (1 character from position length-2)") _T_RE_EOL \
    _T("  set s ~ substr -3 -2 abcde // c (substring from position length-3 to length-2)") _T_RE_EOL \
    _T("  set s = abcde") _T_RE_EOL \
    _T("  set t ~ substr 1 3 $(s)    // bcd (3 characters from position 1)") _T_RE_EOL \
    _T("  // strfind/strrfind") _T_RE_EOL \
    _T("  // * quotes are not treated as a part of a string itself") _T_RE_EOL \
    _T("  set n ~ strfind \"Hello world\" Hello  // returns 0") _T_RE_EOL \
    _T("  set n ~ strfind \"Hello world\" \"o\"    // returns 4 - \"o\" in \"Hello\"") _T_RE_EOL \
    _T("  set n ~ strrfind \"Hello world\" o     // returns 7 - \"o\" in \"world\"") _T_RE_EOL \
    _T("  set n ~ strrfind \"Hello world\" x     // returns -1 - \"x\" not found") _T_RE_EOL \
    _T("  set n ~ strfind \"Hello world\" \"o y\"  // returns -1 - \"o y\" not found") _T_RE_EOL \
    _T("  set s = abcd") _T_RE_EOL \
    _T("  set n ~ strfind \"$(s)\" cd            // returns 2") _T_RE_EOL \
    _T("  // strreplace") _T_RE_EOL \
    _T("  // * quotes are not treated as a part of a string itself") _T_RE_EOL \
    _T("  set s ~ strreplace \"Hello world\" \"o\" \"0\" // Hell0 w0rld    (\"o\" -> \"0\")") _T_RE_EOL \
    _T("  set s ~ strreplace \"$(s)\" l 1            // He110 w0r1d    (\"l\" -> \"1\")") _T_RE_EOL \
    _T("  set s ~ strreplace \"$(s)\" 1 \"y \"         // Hey y 0 w0ry d (\"1\" -> \"y \")") _T_RE_EOL \
    _T("  set s ~ strreplace \"queen-bee\" ee \"\"     // qun-b          (\"ee\" -> \"\")") _T_RE_EOL \
    _T("  // strquote") _T_RE_EOL \
    _T("  set s ~ strquote a b c        // \"a b c\"") _T_RE_EOL \
    _T("  set s ~ strquote \"a b c\"      // \"a b c\"") _T_RE_EOL \
    _T("  // strunquote") _T_RE_EOL \
    _T("  set s ~ strunquote \"a b c\"    // a b c") _T_RE_EOL \
    _T("  set s ~ strunquote a b c      // a b c") _T_RE_EOL \
    _T("  // strescape & strunescape") _T_RE_EOL \
    _T("  set local TAB ~ chr 0x09  // <TAB> = '\\t'") _T_RE_EOL \
    _T("  set local LF ~ chr 0x0A  // <LF> = '\\n'") _T_RE_EOL \
    _T("  set local s = C:\\A$(TAB)b\\C$(LF)d\\Ef  // C:\\A<TAB>b\\C<LF>d\\Ef") _T_RE_EOL \
    _T("  set local t ~ strescape \"$(s)\"  // \\\"C:\\\\A\\tb\\\\C\\nd\\\\Ef\\\"") _T_RE_EOL \
    _T("  set local s2 ~ strunescape $(t)\\x  // \"C:\\A<TAB>b\\C<LF>d\\Ef\"x") _T_RE_EOL \
    _T("  set local s3 ~ strunescape $(s2)  // \"C:A<TAB>bC<LF>dEf\"x") _T_RE_EOL \
    _T("  // strexpand") _T_RE_EOL \
    _T("  set local x = 123") _T_RE_EOL \
    _T("  set local y = $") _T_RE_EOL \
    _T("  set local z ~ strexpand $(y)(x) // $(y)(x) -> $(x) -> 123") _T_RE_EOL \
    _T("  clip_settext $(y)(x) // now the clipboard contains \"$(x)\"") _T_RE_EOL \
    _T("  set local c ~ strexpand $(CLIPBOARD_TEXT) // $(x) -> 123") _T_RE_EOL \
    _T_HELP_STRTOHEX_STRFROMHEX \
    _T_HELP_CHR_ORD_ORDX \
    _T("  // normpath") _T_RE_EOL \
    _T("  set s ~ normpath C:\\A\\.\\B\\X\\..\\C  // C:\\A\\B\\C") _T_RE_EOL \
    _T("  set s ~ normpath \"\\\\A\\B\\..\\..\\C\"  // \"\\\\C\"") _T_RE_EOL \
    _T("  // fileexists") _T_RE_EOL \
    _T("  set local f ~ fileexists $(NPP_DIRECTORY)\\notepad++.exe") _T_RE_EOL \
    _T("  set local f ~ fileexists $(PLUGINS_CONFIG_DIR)\\NppExec.ini") _T_RE_EOL \
    _T("  // direxists") _T_RE_EOL \
    _T("  set local d ~ direxists C:\\Program Files") _T_RE_EOL \
    _T("  set local d ~ direxists $(SYS.ProgramFiles)") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  User\'s variables have the lowest priority, so they can\'t override") _T_RE_EOL \
    _T("  other (predefined) variables. Thus, you can set your own variables") _T_RE_EOL \
    _T("  $(FILE_NAME), $(ARGV) and so on, but their values will not be") _T_RE_EOL \
    _T("  available because of predefined variables with the same name.") _T_RE_EOL \
    _T("  These variables are internal variables of NppExec, unlike") _T_RE_EOL \
    _T("  environment variables which can be set using ENV_SET.") _T_RE_EOL \
    _T("  NppExec uses Function Parser (fparser) to calculate the math") _T_RE_EOL \
    _T("  expression given as \"set <var> ~ <math expression>\".") _T_RE_EOL \
    _T("  The math expression can contain hex numbers with leading \"0x\"") _T_RE_EOL \
    _T("  (e.g. set ans ~ 0x0F + 0x10A - 0xB).") _T_RE_EOL \
    _T("  The following constants are hardcoded: WM_USER, NPPMSG.") _T_RE_EOL \
    _T("  For more details about supported functions, see \"fparser.html\".") _T_RE_EOL \
    _T("  You can use NPE_CMDALIAS to define a short alias to the built-in") _T_RE_EOL \
    _T("  calculator:") _T_RE_EOL \
    _T("    npe_cmdalias = = set ans ~  // \"=\" -> \"set ans ~\"") _T_RE_EOL \
    _T("    = 1 + 2/3 + 4/5  // using the alias for calculations") _T_RE_EOL \
    _T("  Local variables (\"set local <var>\") live just within a script") _T_RE_EOL \
    _T("  where they were declared. When the script ends, all its local") _T_RE_EOL \
    _T("  variables are automatically deleted.") _T_RE_EOL \
    _T("  Within a script, a local variable overrides a global variable") _T_RE_EOL \
    _T("  with the same name. Each script may have its own local variables") _T_RE_EOL \
    _T("  that are not inherited by NPP_EXEC-ed scripts.") _T_RE_EOL \
    _T("  The same applies to local variables created directly in NppExec\'s") _T_RE_EOL \
    _T("  Console - these local variables live only in NppExec\'s Console") _T_RE_EOL \
    _T("  and are not visible in NppExec\'s scripts.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  env_set/env_unset, if") _T_RE_EOL
  },
  
  // ENV_SET
  {
    CScriptEngine::DoEnvSetCommand::Name(),
    _T("COMMAND:  env_set/env_unset") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  env_set <var>") _T_RE_EOL \
    _T("  env_set <var> = <value>") _T_RE_EOL \
    _T("  env_set local <var> = <value>") _T_RE_EOL \
    _T("  env_unset <var>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Shows the value of environment variable (\"env_set\" without \"=\")") _T_RE_EOL \
    _T("  2. Sets the value of environment variable (\"env_set <var> = <value>\")") _T_RE_EOL \
    _T("  3. Sets the value of environment variable locally (with \"local\" specified)") _T_RE_EOL \
    _T("  4. Removes/restores the environment variable <var> (\"env_unset <var>\")") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  env_set NPPHOME = $(NPP_DIRECTORY)  // new environment variable: NPPHOME") _T_RE_EOL \
    _T("  env_set PATH = $(SYS.NPPHOME);$(SYS.PATH)  // modifying the PATH variable") _T_RE_EOL \
    _T("  npp_run readme.txt  // opening \"readme.txt\" from Notepad++\'es folder") _T_RE_EOL \
    _T("                      // (NPPHOME is the first path in the PATH variable)") _T_RE_EOL \
    _T("  env_unset NPPHOME  // removing the environment variable NPPHOME") _T_RE_EOL \
    _T("  env_unset PATH     // restoring initial value of PATH") _T_RE_EOL \
    _T("  env_set local PATH = C:\\tools;$(SYS.PATH)  // has local effect") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Unlike set/unset, these commands deal with Notepad++\'es environment vars") _T_RE_EOL \
    _T("  which are inherited by child processes (programs, tools) started from") _T_RE_EOL \
    _T("  NppExec or Notepad++\'es \"Run...\" menu.") _T_RE_EOL \
    _T("  NppExec can use the values of environment variables in a form of $(SYS.<var>).") _T_RE_EOL \
    _T("  For example:") _T_RE_EOL \
    _T("    env_set NPPHOME = $(NPP_DIRECTORY)  // new environment variable: NPPHOME") _T_RE_EOL \
    _T("    echo $(SYS.NPPHOME)  // print the value of NPPHOME") _T_RE_EOL \
    _T("  NppExec does not understand such form of environment variable as %<var>%.") _T_RE_EOL \
    _T("  I.e. you can type  \"env_set PATH = C:\\tools;$(SYS.PATH)\",") _T_RE_EOL \
    _T("  but not  \"env_set PATH = C:\\tools;%PATH%\"") _T_RE_EOL \
    _T("  because NppExec does not support such declaration as %PATH%.") _T_RE_EOL \
    _T("  You can use a value of other variable as an environment variable name:") _T_RE_EOL \
    _T("    set a = PATH  // value of \'a\' is a string \"PATH\"") _T_RE_EOL \
    _T("    env_set $(a) = C:\\tools  // env. variable PATH becomes \"C:\\tools\"") _T_RE_EOL \
    _T("    echo $(SYS.PATH)  // prints the value of env. var PATH i.e. \"C:\\tools\"") _T_RE_EOL \
    _T("    env_unset $(a)  // restore the initial value of PATH") _T_RE_EOL \
    _T("  The ENV_UNSET command removes only those environment variables which") _T_RE_EOL \
    _T("  have been created by the ENV_SET command (i.e. did not exist before).") _T_RE_EOL \
    _T("  Other environment variables (such as PATH, TEMP etc.) are not removed") _T_RE_EOL \
    _T("  by ENV_UNSET because these variables were not created by ENV_SET.") _T_RE_EOL \
    _T("  Instead, ENV_UNSET restores initial values of these variables.") _T_RE_EOL \
    _T("  The usage of \"env_set local <var> = ...\" is similar to the sequence of") _T_RE_EOL \
    _T("  \"env_set <var> = ...\", doing something with the <var> and finally doing") _T_RE_EOL \
    _T("  \"env_unset <var>\". There is a difference, though: ENV_UNSET restores the") _T_RE_EOL \
    _T("  _initial_ value of <var> (i.e. before any ENV_SET), while \"env_set local\"") _T_RE_EOL \
    _T("  restores the _previous_ value of <var> (e.g. after the last ENV_SET) once") _T_RE_EOL \
    _T("  the current NppExec's script ends.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  set/unset") _T_RE_EOL
  },
  
  // INPUTBOX
  {
    CScriptEngine::DoInputBoxCommand::Name(),
    _T("COMMAND:  inputbox") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  inputbox \"message\"") _T_RE_EOL \
    _T("  inputbox \"message\" : initial_value") _T_RE_EOL \
    _T("  inputbox \"message\" :: initial_value") _T_RE_EOL \
    _T("  inputbox \"message\" : \"value_name\" : initial_value") _T_RE_EOL \
    _T("  inputbox \"message\" : \" \" : initial_value") _T_RE_EOL \
    _T("  inputbox \"message\" : \"value_name\" : \"initial_value\" : time_ms") _T_RE_EOL \
    _T("  inputbox \"message\" ::: timeout") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Shows the InputBox with a message \'message\';") _T_RE_EOL \
    _T("     the input value is stored in $(INPUT), $(INPUT[1]) etc.") _T_RE_EOL \
    _T("  2. Shows the InputBox with a message \'message\';") _T_RE_EOL \
    _T("     the initial input value is set to \'initial_value\';") _T_RE_EOL \
    _T("     the input value is stored in $(INPUT), $(INPUT[1]) etc.") _T_RE_EOL \
    _T("  3. Shows the InputBox with a message \'message\';") _T_RE_EOL \
    _T("     the text \"$(INPUT) =\" is replaced with \'value_name\'") _T_RE_EOL \
    _T("     (use \"\" to keep \"$(INPUT) =\"; use \" \" to empty it);") _T_RE_EOL \
    _T("     the initial input value is set to \'initial_value\';") _T_RE_EOL \
    _T("     the input value is stored in $(INPUT), $(INPUT[1]) etc.") _T_RE_EOL \
    _T("  4. Shows the InputBox for 'time_ms' milliseconds. When the") _T_RE_EOL \
    _T("     'time_ms' is over, the InputBox \"expires\" and closes.") _T_RE_EOL \
    _T("  * Note: $(INPUT), $(INPUT[1]) etc. are local variables.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // show the InputBox...") _T_RE_EOL \
    _T("  inputbox \"Input values of A, B and C separated by spaces:\"") _T_RE_EOL \
    _T("  // update the values of our variables...") _T_RE_EOL \
    _T("  set local a = $(input[1])  // e.g. \'A\' in \'A \"B B\" C\'") _T_RE_EOL \
    _T("  set local b = $(input[2])  // e.g. \'B B\' in \'A \"B B\" C\'") _T_RE_EOL \
    _T("  set local c = $(input[3])  // e.g. \'C\' in \'A \"B B\" C\'") _T_RE_EOL \
    _T("  // show the InputBox with specified initial value...") _T_RE_EOL \
    _T("  inputbox \"Please confirm the value of C:\" : $(c)") _T_RE_EOL \
    _T("  // update the value of c...") _T_RE_EOL \
    _T("  set local c = $(input)") _T_RE_EOL \
    _T("  // show the InputBox with the text \"Input a:\", \"a =\"...") _T_RE_EOL \
    _T("  inputbox \"Input a:\" : \"a =\" :") _T_RE_EOL \
    _T("  // show the InputBox with empty value name...") _T_RE_EOL \
    _T("  inputbox \"Input a:\" : \" \" :   // notice the  : \" \" :  part!") _T_RE_EOL \
    _T("  // show the initial value with a colon...") _T_RE_EOL \
    _T("  inputbox \"Input a:\" : : \"1 : 2\"   // notice the  : :  part!") _T_RE_EOL \
    _T("  // the same...") _T_RE_EOL \
    _T("  inputbox \"Input a:\" :: \"1 : 2\"   // notice the  ::  part!") _T_RE_EOL \
    _T("  // expirable InputBox...") _T_RE_EOL \
    _T("  inputbox \"You have 5 seconds to input a:\" : a : 1 : 5000") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  You can use any environment variable inside the input value, e.g.") _T_RE_EOL \
    _T("    INPUTBOX \"Input A:\"") _T_RE_EOL \
    _T("    // ... the InputBox is shown, you can type e.g.: $(ARGV)") _T_RE_EOL \
    _T("    SET local a = $(INPUT)") _T_RE_EOL \
    _T("    // ... $(a) becomes $(INPUT), i.e. $(ARGV)") _T_RE_EOL \
    _T("  To set the keyboard focus to the Console after the InputBox is shown,") _T_RE_EOL \
    _T("  use the following command:") _T_RE_EOL \
    _T("    npp_setfocus con") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  set, unset, if, messagebox") _T_RE_EOL
  },

  // MESSAGEBOX
  {
    CScriptEngine::DoMessageBoxCommand::Name(),
    _T("COMMAND:  messagebox") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  messagebox \"text\"") _T_RE_EOL \
    _T("  messagebox \"text\" : \"title\"") _T_RE_EOL \
    _T("  messagebox \"text\" : \"title\" : type") _T_RE_EOL \
    _T("  messagebox \"text\" : : type") _T_RE_EOL \
    _T("  messagebox \"text\" : \"title\" : type : time_ms") _T_RE_EOL \
    _T("  messagebox \"text\" ::: time_ms") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Shows a MessageBox with a text \'text\' and default title;") _T_RE_EOL \
    _T("  2. Shows a MessageBox with a text \'text\' and a title \'title\';") _T_RE_EOL \
    _T("  3. Shows a MessageBox of the given type with a text \'text\' and a title \'title\';") _T_RE_EOL \
    _T("  4. Shows a MessageBox of the given type with a text \'text\' and default title;") _T_RE_EOL \
    _T("  5. Shows a MessageBox for 'time_ms' milliseconds and then closes it;") _T_RE_EOL \
    _T("  The type can be:") _T_RE_EOL \
    _T("    0 or \"msg\"  - a message") _T_RE_EOL \
    _T("    1 or \"warn\" - a warning") _T_RE_EOL \
    _T("    2 or \"err\"  - an error") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  messagebox \"This is a simple message\"") _T_RE_EOL \
    _T("  messagebox \"This is a simple message\" :: 0 // the same") _T_RE_EOL \
    _T("  messagebox \"This is a simple message\" : : msg // the same") _T_RE_EOL \
    _T("  messagebox \"This is an error!\" :: err") _T_RE_EOL \
    _T("  messagebox \"This is a custom error!\" : \"ERROR\" : 2") _T_RE_EOL \
    _T("  messagebox \"This is a custom warning!\" : \"WARNING\" : warn") _T_RE_EOL \
    _T("  messagebox \"You will see it during 3 seconds!\" ::: 3000") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  To set the keyboard focus to the Console after the MessageBox is shown,") _T_RE_EOL \
    _T("  use the following command:") _T_RE_EOL \
    _T("    npp_setfocus con") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  inputbox") _T_RE_EOL
  },
  
  // NPE_CMDALIAS
  {
    CScriptEngine::DoNpeCmdAliasCommand::Name(),
    _T("COMMAND:  npe_cmdalias") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npe_cmdalias") _T_RE_EOL \
    _T("  npe_cmdalias <alias>") _T_RE_EOL \
    _T("  npe_cmdalias <alias> =") _T_RE_EOL \
    _T("  npe_cmdalias <alias> = <command>") _T_RE_EOL \
    _T("  npe_cmdalias +v <alias> = <command>") _T_RE_EOL \
    _T("  npe_cmdalias -v <alias> = <command>") _T_RE_EOL \
    _T("  npe_cmdalias local ...") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Shows all command aliases (\"npe_cmdalias\" without parameters)") _T_RE_EOL \
    _T("  2. Shows the value of command alias (\"npe_cmdalias\" without \"=\")") _T_RE_EOL \
    _T("  3. Removes the command alias (\"npe_cmdalias\" with empty \"=\")") _T_RE_EOL \
    _T("  4. Sets the value of command alias (\"npe_cmdalias <alias> = <command>\")") _T_RE_EOL \
    _T("  5.1. +v uses delayed vars substitution (default), e.g. $(x) remains $(x)") _T_RE_EOL \
    _T("  5.2. -v uses instant vars substitution, e.g. $(x) is replaced with its value") _T_RE_EOL \
    _T("  6. local within the current NppExec's script") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // basic examples:") _T_RE_EOL \
    _T("  npe_cmdalias = = calc            // type just \"=\" instead of \"calc\"") _T_RE_EOL \
    _T("  npe_cmdalias copy = cmd /c copy  // \"copy\" instead of \"cmd /c copy\"") _T_RE_EOL \
    _T("  npe_cmdalias /c = cmd /c         // \"/c\" instead of \"cmd /c\"") _T_RE_EOL \
    _T("  npe_cmdalias /c                  // show the value of command alias") _T_RE_EOL \
    _T("  npe_cmdalias /c =                // remove the command alias") _T_RE_EOL \
    _T("  // examples with +v and -v:") _T_RE_EOL \
    _T("  npe_cmdalias e1 = echo $(A), $(B) // uses delayed substitution") _T_RE_EOL \
    _T("  npe_cmdalias +v e2 = echo $(A), $(B) // uses delayed substitution") _T_RE_EOL \
    _T("  set A = 123") _T_RE_EOL \
    _T("  set B = 456") _T_RE_EOL \
    _T("  e1 // prints \"123, 456\" because delayed substitution is used") _T_RE_EOL \
    _T("  e2 // prints \"123, 456\" because delayed substitution is used") _T_RE_EOL \
    _T("  npe_cmdalias -v e3 = echo $(A), $(B) // uses instant substitution") _T_RE_EOL \
    _T("  set A = abc") _T_RE_EOL \
    _T("  set B = def") _T_RE_EOL \
    _T("  e1 // prints \"abc, def\" because delayed substitution is used") _T_RE_EOL \
    _T("  e2 // prints \"abc, def\" because delayed substitution is used") _T_RE_EOL \
    _T("  e3 // prints \"123, 456\" because instant substitution was used") _T_RE_EOL \
    _T("  // delayed substitution example:") _T_RE_EOL \
    _T("  npe_cmdalias fp = nircmd clipboard set $(FULL_CURRENT_PATH)") _T_RE_EOL \
    _T("  fp // always uses the actual file path") _T_RE_EOL \
    _T("  // now open or switch to another file in Notepad++") _T_RE_EOL \
    _T("  fp // always uses the actual file path") _T_RE_EOL \
    _T("  // local cmd alias (within the current script):") _T_RE_EOL \
    _T("  npe_cmdalias local copy = cmd /c copy  // set") _T_RE_EOL \
    _T("  npe_cmdalias local copy                // show") _T_RE_EOL \
    _T("  npe_cmdalias local copy =              // remove") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  To use a command alias, type it in the beginning of the input string") _T_RE_EOL \
    _T("  (e.g. \"/c copy /?\" for the command alias \"/c -> cmd /c\"), otherwise") _T_RE_EOL \
    _T("  it will NOT be treated as a command alias. For example, \"/c\" inside") _T_RE_EOL \
    _T("  \"cmd /c move /?\" is not treated as a command alias because it is located") _T_RE_EOL \
    _T("  in the middle of the string, so this string remains unmodified.") _T_RE_EOL \
    _T("  To use -v or +v as the alias name, just specify the desired mode prior") _T_RE_EOL \
    _T("  to the alias name:") _T_RE_EOL \
    _T("    npe_cmdalias -v -v = ... // alias name is \"-v\", instant substitution") _T_RE_EOL \
    _T("    npe_cmdalias +v -v = ... // alias name is \"-v\", delayed substitution") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npe_console q+/q-") _T_RE_EOL
  },
  
  // NPE_CONSOLE
  {
    CScriptEngine::DoNpeConsoleCommand::Name(),
    _T("COMMAND:  npe_console") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npe_console") _T_RE_EOL \
    _T("  npe_console a+/a- d+/d- e0/e1 u+/u- h+/h- m+/m- p+/p- q+/q- v+/v- j+/j- f+/f- r+/r- x+/x- k0..3") _T_RE_EOL \
    _T("  npe_console c<N> s<N>") _T_RE_EOL \
    _T("  npe_console o0/o1/o2 i0/i1/i2") _T_RE_EOL \
    _T("  npe_console <options> --") _T_RE_EOL \
    _T("  npe_console local <options>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Without parameter - shows current Console options/mode") _T_RE_EOL \
    _T("  2. X+/X-  enables/disables the option/mode X:") _T_RE_EOL \
    _T("       a+/a-  append mode (don\'t clear Console) on/off") _T_RE_EOL \
    _T("       d+/d-  follow $(CURRENT_DIRECTORY) on/off") _T_RE_EOL \
    _T("       e0/e1  ansi escape sequences: raw/remove") _T_RE_EOL \
    _T("       u+/u-  pseudoconsole on/off (experimental)") _T_RE_EOL \
    _T("       h+/h-  console commands history on/off") _T_RE_EOL \
    _T("       m+/m-  console internal messages on/off") _T_RE_EOL \
    _T("       p+/p-  print \"==== READY ====\" on/off") _T_RE_EOL \
    _T("       q+/q-  command aliases on/off") _T_RE_EOL \
    _T("       v+/v-  set the $(OUTPUT) local variable on/off") _T_RE_EOL \
    _T("       j+/j-  kill process tree on/off") _T_RE_EOL \
    _T("       f+/f-  console output filter on/off") _T_RE_EOL \
    _T("       r+/r-  console output replace filter on/off") _T_RE_EOL \
    _T("       x+/x-  compiler errors highlight filter on/off") _T_RE_EOL \
    _T("       k0..3  catch NppExec\'s shortcut keys on/off") _T_RE_EOL \
    _T("       c<N>   text processing for Execute Clipboard Text") _T_RE_EOL \
    _T("       s<N>   text processing for Execute Selected Text") _T_RE_EOL \
    _T("  3. Y0/Y1/Y2  sets the value of the option/mode Y:") _T_RE_EOL \
    _T("       o0/o1/o2  console output encoding: ANSI/OEM/UTF8") _T_RE_EOL \
    _T("       i0/i1/i2  console input encoding: ANSI/OEM/UTF8") _T_RE_EOL \
    _T("  4. --  silent (don\'t print Console mode info)") _T_RE_EOL \
    _T("  5. local within the current NppExec's script") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npe_console o1 i1 a+ --") _T_RE_EOL \
    _T("    //// OEM/OEM, append mode, silent") _T_RE_EOL \
    _T("  npe_console h+ m+ a- i0 o0") _T_RE_EOL \
    _T("    //// history on, int. msgs on, don\'t append, ANSI/ANSI") _T_RE_EOL \
    _T("  npe_console d+ k0 q-") _T_RE_EOL \
    _T("    //// follow curr. dir, don\'t catch shortcut keys, cmd aliases off") _T_RE_EOL \
    _T("  npe_console -- v+ m- // silent, enable the $(OUTPUT) var, no int.msgs") _T_RE_EOL \
    _T("  cmd /c time /t       // cmd prints current time; $(OUTPUT) is set") _T_RE_EOL \
    _T("  echo $(OUTPUT)       // NppExec prints $(OUTPUT) i.e. current time") _T_RE_EOL \
    _T("  npe_console -- v- m+ // don\'t forget to disable the $(OUTPUT) var") _T_RE_EOL \
    _T("  npe_console local -- v+ m- // has local effect") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  a+/a-  Console append mode on/off.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is not saved when you close Notepad++.") _T_RE_EOL \
    _T("         Default value: off") _T_RE_EOL \
    _T("         (Console is automatically cleared when new script starts).") _T_RE_EOL \
    _T("  d+/d-  Follow $(CURRENT_DIRECTORY) on/off.") _T_RE_EOL \
    _T("         Corresponding menu item: Follow $(CURRENT_DIRECTORY).") _T_RE_EOL \
    _T("         If On, NppExec follows the directory of current file.") _T_RE_EOL \
    _T("  e0/e1  ANSI escape sequences: raw/remove.") _T_RE_EOL \
    _T("         e0: keep raw esc-sequence characters;") _T_RE_EOL \
    _T("         e1: remove esc-sequence characters.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"AnsiEscapeSequences\".") _T_RE_EOL \
    _T("         Default value: 0.") _T_RE_EOL \
    _T("         Note: if a child process is running, NPE_CONSOLE prints the value") _T_RE_EOL \
    _T("         of this option corresponding to the child process.") _T_RE_EOL \
    _T("  u+/u-  pseudoconsole on/off (experimental).") _T_RE_EOL \
    _T("         When On, the console new line is set to '\\r';") _T_RE_EOL \
    _T("         the input and output encodings are set to UTF-8 (i2, o2);") _T_RE_EOL \
    _T("         the ANSI escape sequences are set to \"remove\" (e1).") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"ChildProcess_PseudoConsole\".") _T_RE_EOL \
    _T("         Note: if a child process is running, NPE_CONSOLE prints the value") _T_RE_EOL \
    _T("         of this option corresponding to the child process.") _T_RE_EOL \
    _T("  h+/h-  Console commands history on/off.") _T_RE_EOL \
    _T("         Corresponding menu item: Console Commands History.") _T_RE_EOL \
    _T("         If On, NppExec\'s Console remembers the commands you typed.") _T_RE_EOL \
    _T("  m+/m-  Console internal messages on/off.") _T_RE_EOL \
    _T("         Corresponding menu item (inverse): No internal messages.") _T_RE_EOL \
    _T("         If On, NppExec\'s Console prints all internal (green) messages.") _T_RE_EOL \
    _T("  p+/p-  Print \"==== READY ====\" on/off.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"PrintMsgReady\".") _T_RE_EOL \
    _T("         Default value: on.") _T_RE_EOL \
    _T("  q+/q-  Command aliases on/off.") _T_RE_EOL \
    _T("         ") _T_DISABLE_CMD_ALIASES_MENU_ITEM _T_RE_EOL \
    _T("         This option is not saved when you close Notepad++.") _T_RE_EOL \
    _T("         Default value: on") _T_RE_EOL \
    _T("         (command aliases created with NPE_CMDALIAS are active).") _T_RE_EOL \
    _T("  v+/v-  Set the $(OUTPUT) local variable on/off.") _T_RE_EOL \
    _T("         (this variable will store the child process\'es output).") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is not saved when you close Notepad++.") _T_RE_EOL \
    _T("         Default value: off.") _T_RE_EOL \
    _T("  j+/j-  Kill process tree on/off.") _T_RE_EOL \
    _T("         When this option is On, killing a running child process (the one") _T_RE_EOL \
    _T("         that runs in NppExec's Console) also kills any processes that were") _T_RE_EOL \
    _T("         started from this running child process.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"KillProcTree\".") _T_RE_EOL \
    _T("         Default value: off.") _T_RE_EOL \
    _T("  f+/f-  Console output filter on/off.") _T_RE_EOL \
    _T("         Corresponding menu item: Console Output Filters...") _T_RE_EOL \
    _T("  r+/r-  Console output replace filter on/off.") _T_RE_EOL \
    _T("         Corresponding menu item: Console Output Filters...") _T_RE_EOL \
    _T("  x+/x-  Compiler errors highlight filter on/off") _T_RE_EOL \
    _T("         Enables or disables a built-in highlight filter that catches and") _T_RE_EOL \
    _T("         highlights most of compilers' errors. This filter has lower priority") _T_RE_EOL \
    _T("         than the user-defined highlight masks.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"CompilerErrors\".") _T_RE_EOL \
    _T("         Default value: off.") _T_RE_EOL \
    _T("  k0..3  Catch NppExec\'s shortcut keys on/off") _T_RE_EOL \
    _T("         Controls if NppExec\'s Console catches shortcut keys related") _T_RE_EOL \
    _T("         to NppExec\'s menu items or scripts. This allows to execute") _T_RE_EOL \
    _T("         the corresponding commands from NppExec\'s Console when the") _T_RE_EOL \
    _T("         associated shortcut keys are pressed.") _T_RE_EOL \
    _T("         k0: Do not catch NppExec\'s shortcut keys;") _T_RE_EOL \
    _T("         k1: Catch standard NppExec\'s shortcut keys (Execute... etc.);") _T_RE_EOL \
    _T("         k2: Catch user\'s NppExec\'s shortcut keys (scripts\' menu items);") _T_RE_EOL \
    _T("         k3: Catch all NppExec\'s shortcut keys (standard+scripts).") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is not saved when you close Notepad++.") _T_RE_EOL \
    _T("         Default value: 3.") _T_RE_EOL \
    _T("  c<N>   Text processing for Execute Clipboard Text.") _T_RE_EOL \
    _T("         The value of <N> can be either 0 or a sum of these flags:") _T_RE_EOL \
    _T("         0:  use the clipboard text as is;") _T_RE_EOL \
    _T("             if there is a running child process then send the clipboard") _T_RE_EOL \
    _T("             text to that process as an input;") _T_RE_EOL \
    _T("         1:  (vs) substitute macro-vars in the clipboard text before the execution") _T_RE_EOL \
    _T("             if there is no running child process;") _T_RE_EOL \
    _T("         2:  (vp) substitute macro-vars in the clipboard text before the execution") _T_RE_EOL \
    _T("             if there is a running child process;") _T_RE_EOL \
    _T("         4:  (cs) if the first line of the clipboard text is \"!collateral\" and") _T_RE_EOL \
    _T("             there is no running child process then start a collateral script;") _T_RE_EOL \
    _T("         8:  (cp) if the first line of the clipboard text is \"!collateral\" and") _T_RE_EOL \
    _T("             there is a running child process then start a collateral script;") _T_RE_EOL \
    _T("         16: (ne) lines that start with the \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("\" prefix will be executed by") _T_RE_EOL \
    _T("             NppExec (as NppExec's script commands) and will not be sent to") _T_RE_EOL \
    _T("             a running child process;") _T_RE_EOL \
    _T("         32: (ls) update the last executed script: Execute Clipboard Text updates") _T_RE_EOL \
    _T("             the commands that will be executed by Execute Previous NppExec Script;") _T_RE_EOL \
    _T("         64: (sv) share local variables: Execute Clipboard Text uses and updates") _T_RE_EOL \
    _T("             the existing local variables instead of its own local variables.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"ExecClipTextMode\".") _T_RE_EOL \
    _T("         Default value: 60 (4+8+16+32).") _T_RE_EOL \
    _T("  s<N>   Text processing for Execute Selected Text.") _T_RE_EOL \
    _T("         The value of <N> can be either 0 or a sum of these flags:") _T_RE_EOL \
    _T("         0:  use the selected text as is;") _T_RE_EOL \
    _T("             if there is a running child process then send the selected") _T_RE_EOL \
    _T("             text to that process as an input;") _T_RE_EOL \
    _T("         1:  (vs) substitute macro-vars in the selected text before the execution") _T_RE_EOL \
    _T("             if there is no running child process;") _T_RE_EOL \
    _T("         2:  (vp) substitute macro-vars in the selected text before the execution") _T_RE_EOL \
    _T("             if there is a running child process;") _T_RE_EOL \
    _T("         4:  (cs) if the first line of the selected text is \"!collateral\" and") _T_RE_EOL \
    _T("             there is no running child process then start a collateral script;") _T_RE_EOL \
    _T("         8:  (cp) if the first line of the selected text is \"!collateral\" and") _T_RE_EOL \
    _T("             there is a running child process then start a collateral script;") _T_RE_EOL \
    _T("         16: (ne) lines that start with the \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("\" prefix will be executed by") _T_RE_EOL \
    _T("             NppExec (as NppExec's script commands) and will not be sent to") _T_RE_EOL \
    _T("             a running child process;") _T_RE_EOL \
    _T("         32: (ls) update the last executed script: Execute Selected Text updates the") _T_RE_EOL \
    _T("             commands that will be executed by Execute Previous NppExec Script;") _T_RE_EOL \
    _T("         64: (sv) share local variables: Execute Selected Text uses and updates") _T_RE_EOL \
    _T("             the existing local variables instead of its own local variables.") _T_RE_EOL \
    _T("         There is no corresponding menu item.") _T_RE_EOL \
    _T("         This option is saved as \"ExecSelTextMode\".") _T_RE_EOL \
    _T("         Default value: 60 (4+8+16+32).") _T_RE_EOL \
    _T("  o0/o1/o2  Console output encoding: ANSI/OEM/UTF8") _T_RE_EOL \
    _T("            Corresponding menu item: Console Output...") _T_RE_EOL \
    _T("            Sets Console output encoding.") _T_RE_EOL \
    _T("            Note: if a child process is running, NPE_CONSOLE prints the value") _T_RE_EOL \
    _T("            of this option corresponding to the child process.") _T_RE_EOL \
    _T("  i0/i1/i2  Console input encoding: ANSI/OEM/UTF8") _T_RE_EOL \
    _T("            Corresponding menu item: Console Output...") _T_RE_EOL \
    _T("            Sets Console input encoding.") _T_RE_EOL \
    _T("            Note: if a child process is running, NPE_CONSOLE prints the value") _T_RE_EOL \
    _T("            of this option corresponding to the child process.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  con_filter, npp_console, npe_noemptyvars") _T_RE_EOL
  },

  // NPE_DEBUGLOG
  {
    CScriptEngine::DoNpeDebugLogCommand::Name(),
    _T("COMMAND:  npe_debuglog") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npe_debuglog") _T_RE_EOL \
    _T("  npe_debuglog on") _T_RE_EOL \
    _T("  npe_debuglog 1") _T_RE_EOL \
    _T("  npe_debuglog off") _T_RE_EOL \
    _T("  npe_debuglog 0") _T_RE_EOL \
    _T("  npe_debuglog local 1/0") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Without parameter - shows current Debug Log state (On/Off)") _T_RE_EOL \
    _T("  2. On or 1 - enables the Debug Log in NppExec's Console") _T_RE_EOL \
    _T("  3. Off or 0 - disables the Debug Log in NppExec's Console") _T_RE_EOL \
    _T("  4. local - enables/disables the Debug Log locally") _T_RE_EOL \
    _T_LOCAL_CMD_HINT
  },

  // NPE_NOEMPTYVARS
  {
    CScriptEngine::DoNpeNoEmptyVarsCommand::Name(),
    _T("COMMAND:  npe_noemptyvars") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npe_noemptyvars") _T_RE_EOL \
    _T("  npe_noemptyvars 1") _T_RE_EOL \
    _T("  npe_noemptyvars 0") _T_RE_EOL \
    _T("  npe_noemptyvars local 1/0") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Without parameter - shows current option\'s state") _T_RE_EOL \
    _T("  2. 1 or On - enables replacement of empty (uninitialized) vars") _T_RE_EOL \
    _T("  3. 0 or Off - disables replacement of empty (uninitialized) vars") _T_RE_EOL \
    _T("  4. local - enables/disables empty vars locally") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  unset var            // to be sure that $(var) is uninitialized") _T_RE_EOL \
    _T("  unset local var      // to be sure that $(var) is uninitialized") _T_RE_EOL \
    _T("  npe_noemptyvars 0    // disabled") _T_RE_EOL \
    _T("  echo var = \"$(var)\"  // prints: var = \"$(var)\"") _T_RE_EOL \
    _T("  npe_noemptyvars 1    // enabled") _T_RE_EOL \
    _T("  echo var = \"$(var)\"  // prints: var = \"\"") _T_RE_EOL \
    _T("  npe_noemptyvars local 1    // enabled locally") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Refer to NppExec.ini, parameter NoEmptyVars in the [Console] section.") _T_RE_EOL \
    _T("  This option is not saved when you close Notepad++.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  set, echo, npe_console") _T_RE_EOL
  },

  // NPE_QUEUE
  {
    CScriptEngine::DoNpeQueueCommand::Name(),
    _T("COMMAND:  npe_queue") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npe_queue <command>") _T_RE_EOL \
    _T("  npe_queue +s/-s +v/-v <command>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Queues a command to be executed after the current NppExec's script is finished.") _T_RE_EOL \
    _T("  +s - execute the queued command as a separate NppExec's script (default)") _T_RE_EOL \
    _T("  -s - execute the queued command in the scope of the current NppExec's script") _T_RE_EOL \
    _T("  +v - use delayed vars substitution (default), e.g. $(x) remains $(x)") _T_RE_EOL \
    _T("  -v - use instant vars substitution, e.g. $(x) is replaced with its value") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // queue each command as a separate script:") _T_RE_EOL \
    _T("  set X = 10") _T_RE_EOL \
    _T("  npe_queue set X = 20 // queued: will set X to 20") _T_RE_EOL \
    _T("  echo $(X) // prints the current value of X, i.e. 10") _T_RE_EOL \
    _T("  npe_queue echo $(X) // queued: will print the latest value of X, i.e. 20") _T_RE_EOL \
    _T("  npe_queue -v echo $(X) // queued: will print the current value of X, i.e. 10") _T_RE_EOL \
    _T("  // queue in the scope of the current script:") _T_RE_EOL \
    _T("  set X = 10") _T_RE_EOL \
    _T("  npe_queue -s set X = 20 // queued: will set X to 20") _T_RE_EOL \
    _T("  echo $(X) // prints the current value of X, i.e. 10") _T_RE_EOL \
    _T("  npe_queue -s +v echo $(X) // queued: will print the latest value of X, i.e. 20") _T_RE_EOL \
    _T("  npe_queue -v -s echo $(X) // queued: will print the current value of X, i.e. 10") _T_RE_EOL \
    _T("  // the order of execution:") _T_RE_EOL \
    _T("  npe_queue echo 5th") _T_RE_EOL \
    _T("  npe_queue echo 6th") _T_RE_EOL \
    _T("  npe_queue -s echo 3rd") _T_RE_EOL \
    _T("  npe_queue -s echo 4th") _T_RE_EOL \
    _T("  echo 1st") _T_RE_EOL \
    _T("  echo 2nd") _T_RE_EOL \
    _T("  // just a nice script that looks elegant:") _T_RE_EOL \
    _T("  npp_console local -      // within this script") _T_RE_EOL \
    _T("  npe_console local -- v+  // within this script") _T_RE_EOL \
    _T("  cmd /c time /t") _T_RE_EOL \
    _T("  npe_queue -v -s echo $(OUTPUT)") _T_RE_EOL \
    _T("  npp_console local +") _T_RE_EOL \
    _T("  // now something really crazy, just because we _can_ do it:") _T_RE_EOL \
    _T("  npe_queue npe_queue npe_queue sleep 2000 Let's sleep for 2 seconds...") _T_RE_EOL \
    _T("  npe_queue -s npe_queue -s npe_queue -s sleep 2000 Let's sleep for 2 seconds...") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  The parameters +v/-v and +s/-s can be specified in any order, but always") _T_RE_EOL \
    _T("  right after \"npe_queue\" and before the command to be executed. Both the") _T_RE_EOL \
    _T("  parameters can be omitted.") _T_RE_EOL \
    _T("  The NPE_QUEUE command can be useful in combination with the \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("\" prefix.") _T_RE_EOL \
    _T("  For example, if cmd.exe is running in NppExec's Console and you realized you") _T_RE_EOL \
    _T("  want some command to be executed right after the cmd.exe has finished, it can") _T_RE_EOL \
    _T("  be queued right from the cmd's prompt:") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("npe_queue npp_console off") _T_RE_EOL \
    _T("  This will close the Console right after you have finished to work with the") _T_RE_EOL \
    _T("  cmd.exe. (Note: as the \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("\" prefix tells NppExec to start a collateral") _T_RE_EOL \
    _T("  script in this case, the usage of \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("npe_queue -s npp_console off\"") _T_RE_EOL \
    _T("  (with \"-s\") would be equivalent to a simple \"") DEFAULT_NPPEXEC_CMD_PREFIX _T("npp_console off\", thus") _T_RE_EOL \
    _T("  trying to execute it immediately rather than queuing.)") _T_RE_EOL \
    _T("  The parameter \"-v\" is necessary when working with local variables. Consider") _T_RE_EOL \
    _T("  the following examples:") _T_RE_EOL \
    _T("    // example 1, inside NppExec's script:") _T_RE_EOL \
    _T("    set local a = 12345") _T_RE_EOL \
    _T("    npe_queue -v echo $(a)") _T_RE_EOL \
    _T("    // example 2, in NppExec's Console while some process is running:") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("npe_queue -v echo $(pid)") _T_RE_EOL \
    _T("  In both situations, the variables $(a) and $(pid) are local variables. If the") _T_RE_EOL \
    _T("  \"-v\" parameter was not specified, it would mean: print the value of $(a) or") _T_RE_EOL \
    _T("  $(pid) at the moment of the previous script or process has finished. But, as") _T_RE_EOL \
    _T("  these variables are local ones, they will not exist at that moment! So, most") _T_RE_EOL \
    _T("  likely, an empty string will be printed - unless there is another, non-local,") _T_RE_EOL \
    _T("  variable with the same name.") _T_RE_EOL \
    _T("  Here is another non-intuitive example: as NPE_QUEUE queues a given command as") _T_RE_EOL \
    _T("  a separate NppExec's script (without \"-s\", of course), this script - as any") _T_RE_EOL \
    _T("  script in NppExec - does not have access to local variables of the invoker.") _T_RE_EOL \
    _T("  For example, let's type the following commands one by one:") _T_RE_EOL \
    _T("    set a = 123             // non-local $(a)") _T_RE_EOL \
    _T("    set local a = abc       // local $(a)") _T_RE_EOL \
    _T("    echo $(a)               // prints \"abc\" - local $(a)") _T_RE_EOL \
    _T("    npe_queue echo $(a)     // separate script: prints \"123\" - non-local $(a)") _T_RE_EOL \
    _T("    npe_queue -s echo $(a)  // current script: prints \"abc\" - local $(a)") _T_RE_EOL \
    _T("    npe_queue -v echo $(a)  // separate script: prints \"abc\" - current $(a)") _T_RE_EOL \
    _T("  Well, I hope you enjoy it :)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npe_cmdalias") _T_RE_EOL
  },

  {
    CScriptEngine::DoNpeSendMsgBufLenCommand::Name(),
    _T("COMMAND:  npe_sendmsgbuflen") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  npe_sendmsgbuflen <max_length>") _T_RE_EOL \
    _T("  npe_sendmsgbuflen local <max_length>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sets npp_sendmsg/sci_sendmsg's maximum buffer length") _T_RE_EOL \
    _T_LOCAL_CMD_HINT \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  npe_sendmsgbuflen  // current buffer length") _T_RE_EOL \
    _T("  npe_sendmsgbuflen 1048576  // 1 MB") _T_RE_EOL \
    _T("  npe_sendmsgbuflen 8M  // 8 MB") _T_RE_EOL \
    _T("  npe_sendmsgbuflen local 4M  // 4 MB locally") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  If the specified value is less than 65536, NppExec sets it to 65536.") _T_RE_EOL \
    _T("  This value is not saved when Notepad++ exits.") _T_RE_EOL \
    _T("  To set this value permanently, refer to \"NppExec_TechInfo.txt\".") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  npp_sendmsg, npp_sendmsgex, sci_sendmsg") _T_RE_EOL
  },

  // SEL_LOADFROM
  {
    CScriptEngine::DoSelLoadFromCommand::Name(),
    _T("COMMAND:  sel_loadfrom") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sel_loadfrom <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Replaces currently selected text with the file\'s content") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  sel_loadfrom c:\\temp\\output.txt") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Unicode version of NppExec can read ANSI, UTF-8, UCS-2 LE and UCS-2 BE") _T_RE_EOL \
    _T("  text files.") _T_RE_EOL \
    _T("  The text loaded from the file is converted to the current Scintialla's") _T_RE_EOL \
    _T("  encoding.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sel_saveto, sel_settext, text_loadfrom") _T_RE_EOL
  },
  
  // SEL_SAVETO
  {
    CScriptEngine::DoSelSaveToCommand::Name(),
    _T("COMMAND:  sel_saveto") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sel_saveto <file>") _T_RE_EOL \
    _T("  sel_saveto <file> : <encoding>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Saves the selected text (in its current encoding) to a file") _T_RE_EOL \
    _T("  2. Saves the selected text (converted to specified encoding) to a file") _T_RE_EOL \
    _T("     <encoding> may be:  a (ANSI), u (UTF-8), w (UCS-2 LE)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  sel_saveto c:\\temp\\output.txt // may be ANSI, UTF-8 or DBCS file") _T_RE_EOL \
    _T("  sel_saveto c:\\temp\\outputANSI.txt :a     // save as ANSI file") _T_RE_EOL \
    _T("  sel_saveto c:\\temp\\outputUTF8.txt :u     // save as UTF-8 (with BOM) file") _T_RE_EOL \
    _T("  sel_saveto c:\\temp\\outputUTF8.txt :u+    // save as UTF-8 (with BOM) file") _T_RE_EOL \
    _T("  sel_saveto c:\\temp\\outputUTF8.txt :u-    // save as UTF-8 (without BOM) file") _T_RE_EOL \
    _T("  sel_saveto c:\\temp\\outputUnicode.txt :w  // save as UCS-2 LE (with BOM) file") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  SEL_SAVETO with the <encoding> parameter omitted saves the text in the") _T_RE_EOL \
    _T("  Scintilla\'s current encoding (it can be ANSI, DBCS or UTF-8).") _T_RE_EOL \
    _T("  So, if you want to save the text as UCS-2 LE (Unicode in Windows), you") _T_RE_EOL \
    _T("  must specify the \":w\" parameter.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sel_loadfrom, sel_settext, text_saveto") _T_RE_EOL
  },

  // SEL_SETTEXT
  {
    CScriptEngine::DoSelSetTextCommand::Name(),
    _T("COMMAND:  sel_settext, sel_settext+") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sel_settext <text>") _T_RE_EOL \
    _T("  sel_settext+ <text>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Replaces currently selected text with the text specified") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  sel_settext This text has replaced the previously selected text") _T_RE_EOL \
    _T("  sel_settext $(INPUT)   // see: inputbox") _T_RE_EOL \
    _T("  sel_settext $(OUTPUT)  // see: npe_console v+") _T_RE_EOL \
    _T("  sel_settext $(SYS.PATH)") _T_RE_EOL \
    _T("  sel_settext+ A text with new line:\\r\\nnew line") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Use sel_settext+ to insert special characters:") _T_RE_EOL \
    _T("  \\n  - line feed (LF) character;") _T_RE_EOL \
    _T("  \\r  - carriage return (CR) character;") _T_RE_EOL \
    _T("  \\t  - tabulation character;") _T_RE_EOL \
    _T("  \\\\  - \\;  \\\\n - \\n;  \\\\t - \\t.") _T_RE_EOL \
    _T("  To replace the entire text rather than the selected text, use") _T_RE_EOL \
    _T("  the following script:") _T_RE_EOL \
    _T("    // 1. disabling redrawing") _T_RE_EOL \
    _T("    sci_sendmsg 0x000B 0 // WM_SETREDRAW FALSE") _T_RE_EOL \
    _T("    // 2. selecting all the text") _T_RE_EOL \
    _T("    sci_sendmsg SCI_SELECTALL") _T_RE_EOL \
    _T("    // 3. enabling redrawing") _T_RE_EOL \
    _T("    sci_sendmsg 0x000B 1 // WM_SETREDRAW TRUE") _T_RE_EOL \
    _T("    // 4. replacing the text") _T_RE_EOL \
    _T("    sel_settext ...") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  sel_loadfrom, sel_saveto") _T_RE_EOL
  },

  // TEXT_LOADFROM
  {
    CScriptEngine::DoTextLoadFromCommand::Name(),
    _T("COMMAND:  text_loadfrom") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  text_loadfrom <file>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Replaces the entire text with the file\'s content") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  text_loadfrom c:\\temp\\output.txt") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  Unicode version of NppExec can read ANSI, UTF-8, UCS-2 LE and UCS-2 BE") _T_RE_EOL \
    _T("  text files.") _T_RE_EOL \
    _T("  The text loaded from the file is converted to the current Scintialla's") _T_RE_EOL \
    _T("  encoding.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  text_saveto, sel_loadfrom") _T_RE_EOL
  },

  // TEXT_SAVETO
  {
    CScriptEngine::DoTextSaveToCommand::Name(),
    _T("COMMAND:  text_saveto") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  text_saveto <file>") _T_RE_EOL \
    _T("  text_saveto <file> : <encoding>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  1. Saves the entire text (in its current encoding) to a file") _T_RE_EOL \
    _T("  2. Saves the entire text (converted to specified encoding) to a file") _T_RE_EOL \
    _T("     <encoding> may be:  a (ANSI), u (UTF-8), w (UCS-2 LE)") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  text_saveto c:\\temp\\output.txt // may be ANSI, UTF-8 or DBCS file") _T_RE_EOL \
    _T("  text_saveto c:\\temp\\outputANSI.txt :a    // save as ANSI file") _T_RE_EOL \
    _T("  text_saveto c:\\temp\\outputUTF8.txt :u    // save as UTF-8 (with BOM) file") _T_RE_EOL \
    _T("  text_saveto c:\\temp\\outputUTF8.txt :u+   // save as UTF-8 (with BOM) file") _T_RE_EOL \
    _T("  text_saveto c:\\temp\\outputUTF8.txt :u-   // save as UTF-8 (without BOM) file") _T_RE_EOL \
    _T("  text_saveto c:\\temp\\outputUnicode.txt :w // save as UCS-2 LE (with BOM) file") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  TEXT_SAVETO with the <encoding> parameter omitted saves the text in the") _T_RE_EOL \
    _T("  Scintilla\'s current encoding (it can be ANSI, DBCS or UTF-8).") _T_RE_EOL \
    _T("  So, if you want to save the text as UCS-2 LE (Unicode in Windows), you") _T_RE_EOL \
    _T("  must specify the \":w\" parameter.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  text_loadfrom, sel_saveto") _T_RE_EOL
  },

  // CLIP_SETTEXT
  {
    CScriptEngine::DoClipSetTextCommand::Name(),
    _T("COMMAND:  clip_settext") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  clip_settext <text>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sets the Clipboard text") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  echo Text in the Clipboard (was): $(CLIPBOARD_TEXT)") _T_RE_EOL
    _T("  clip_settext This is the new Clipboard text") _T_RE_EOL \
    _T("  echo Text in the Clipboard (now): $(CLIPBOARD_TEXT)") _T_RE_EOL
  },

  // IF
  {
    CScriptEngine::DoIfCommand::Name(),
    _T("COMMAND:  if") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  1. if <condition> goto <label>") _T_RE_EOL \
    _T("  2. if <condition> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("  3. if <condition> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     else") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("  4. if <condition1> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     else if <condition2> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     else") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("  5. if <condition1> goto <label1>") _T_RE_EOL \
    _T("     else if <condition2> goto <label2>") _T_RE_EOL \
    _T("     else") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  If the condition is true, jumps to the specified label.") _T_RE_EOL \
    _T("  If the condition is false, proceeds to the next line.") _T_RE_EOL \
    _T("  * If the specified label can not be found within the current script,") _T_RE_EOL \
    _T("  this command reports an error and proceeds to the next line.") _T_RE_EOL \
    _T("  ** You should always place \"if\" and \"goto\" on the same line.") _T_RE_EOL \
    _T("  Available conditions:") _T_RE_EOL \
    _T("  a == b  - equal:             1 == 1, \"Ab\" == \"Ab\"") _T_RE_EOL \
    _T("  a = b   - equal:             2 = 2,  \"Aa\" = \"Aa\"") _T_RE_EOL \
    _T("  a != b  - not equal:         1 != 2, \"Ab\" != \"AB\"") _T_RE_EOL \
    _T("  a <> b  - not equal:         1 <> 2, \"Aa\" <> \"A\"") _T_RE_EOL \
    _T("  a > b   - greater:           2 > 1,  \"Ab\" > \"Aa\"") _T_RE_EOL \
    _T("  a < b   - less:             -2 < 1,  \"a\" < \"ab\"") _T_RE_EOL \
    _T("  a >= b  - greater or equal:  0 >= 0, \"AB\" >= \"AA\"") _T_RE_EOL \
    _T("  a <= b  - less or equal:     1 <= 2, \"A\" <= \"AA\"") _T_RE_EOL \
    _T("  a ~= b  - equal no case:     \"AA\" ~= \"aa\"") _T_RE_EOL \
    _T("  * You can use variables and constants as the operands:") _T_RE_EOL \
    _T("    \"if $(var1) == $(var2) goto EqualVars\"") _T_RE_EOL \
    _T("    \"if $(var) != 10 goto NotEqualTo10\"") _T_RE_EOL \
    _T("    \"if ABC < $(var) goto GreaterThanABC\"") _T_RE_EOL \
    _T("  ** Each condition always expects two operands: the left one and the") _T_RE_EOL \
    _T("  right one. Thus, \"if $(var) goto Label\" is not valid - you should") _T_RE_EOL \
    _T("  specify either \"if $(var) != 0 goto Label\" for a number or") _T_RE_EOL \
    _T("  \"if \"$(var)\" != \"\" goto Label\" for a string.") _T_RE_EOL \
    _T("  *** If both operands are decimal numbers, a numeric comparison is") _T_RE_EOL \
    _T("  used. Otherwise the operands are compared as strings.") _T_RE_EOL \
    _T("  Thus, -2 < -1 (decimal numbers) and \"-2\" > \"-1\" (strings).") _T_RE_EOL \
    _T("  **** No additional calculations are performed during the comparison.") _T_RE_EOL \
    _T("  Thus, \"if 3 == 1 + 2 goto Label\" is not a valid numeric comparison") _T_RE_EOL \
    _T("  because \"1 + 2\" is not calculated and remains as is, so the string") _T_RE_EOL \
    _T("  comparison is used in this case (\"3\" is compared with \"1 + 2\").") _T_RE_EOL \
    _T("  All the calculations should be performed in advance - or use IF~ .") _T_RE_EOL \
    _T("  ***** Notice the usage of double quotes and spaces between the") _T_RE_EOL \
    _T("  operands while comparing strings that contain '>', '<' and so on:") _T_RE_EOL \
    _T("    if \"1 >\" < \"2\" goto Label1  // \"1 >\" vs. \"2\"") _T_RE_EOL \
    _T("    if \"1\" > \"< 2\" goto Label2  // \"1\" vs. \"< 2\"") _T_RE_EOL \
    _T("  ****** Nested conditions are supported.") _T_RE_EOL \
    _T("  ******* The keyword \"then\" can be omitted.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Example 1:") _T_RE_EOL \
    _T("  // emulating \"for (i = 1; i <= 10; i += 1) { cout << i << endl; }\"") _T_RE_EOL \
    _T("  set local i ~ 1          // initialization: \"i = 1\"") _T_RE_EOL \
    _T("  :for_i                   // start of the cycle") _T_RE_EOL \
    _T("  echo $(i)                // the cycle body: \"cout << i << endl;\"") _T_RE_EOL \
    _T("  set local i ~ $(i) + 1   // iteration: \"i += 1\"") _T_RE_EOL \
    _T("  if $(i) <= 10 goto for_i // condition: \"i <= 10\"") _T_RE_EOL \
    _T("  // - equivalent to \"i = 1; do { cout << i << endl; i += 1; } while (i <= 10);\"") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Example 2:") _T_RE_EOL \
    _T("  // compare N leading characters of S1 with N trailing characters of S2") _T_RE_EOL \
    _T("  set local S1 = extension  // input: S1") _T_RE_EOL \
    _T("  set local S2 = some text  // input: S2") _T_RE_EOL \
    _T("  set local N = 3           // initialization: N") _T_RE_EOL \
    _T("  set local sub1 ~ substr 0 $(N) $(S1)   // get N leading characters of S1") _T_RE_EOL \
    _T("  set local sub2 ~ substr -$(N) - $(S2)  // get N trailing characters of S2") _T_RE_EOL \
    _T("  if \"$(sub1)\" == \"$(sub2)\" then") _T_RE_EOL \
    _T("    echo substrings are equal: \"$(sub1)\" == \"$(sub2)\"") _T_RE_EOL \
    _T("  else") _T_RE_EOL \
    _T("    echo substrings are not equal: \"$(sub1)\" != \"$(sub2)\"") _T_RE_EOL \
    _T("  endif") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Example 3:") _T_RE_EOL \
    _T("  // emulating \"IF 1 <= x AND x < 5\"") _T_RE_EOL \
    _T("  if 1 > $(x) goto @If_End  // opposite to 1 <= x") _T_RE_EOL \
    _T("  if $(x) >= 5 goto @If_End // opposite to x < 5") _T_RE_EOL \
    _T("  :@If_True") _T_RE_EOL \
    _T("  // the condition is true, do something...") _T_RE_EOL \
    _T("  echo True: 1 <= x < 5") _T_RE_EOL \
    _T("  :@If_End") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Example 4:") _T_RE_EOL \
    _T("  // emulating \"IF s == \'ABC\' OR s == \'DEF\' THEN ... ELSE ...\"") _T_RE_EOL \
    _T("  if \"$(s)\" == \"ABC\" goto @If_True") _T_RE_EOL \
    _T("  if \"$(s)\" == \"DEF\" goto @If_True") _T_RE_EOL \
    _T("  else") _T_RE_EOL \
    _T("    // the condition is false, do something...") _T_RE_EOL \
    _T("    echo False: s is neither \"ABC\" nor \"DEF\"") _T_RE_EOL \
    _T("  endif") _T_RE_EOL \
    _T("  goto @If_End") _T_RE_EOL \
    _T("  :@If_True") _T_RE_EOL \
    _T("  // the condition is true, do something...") _T_RE_EOL \
    _T("  echo True: s is either \"ABC\" or \"DEF\"") _T_RE_EOL \
    _T("  :@If_End") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Example 5:") _T_RE_EOL \
    _T("  // nested conditions") _T_RE_EOL \
    _T("  set local x = 0") _T_RE_EOL \
    _T("  set local y = 1") _T_RE_EOL \
    _T("  if $(x) == 0 then") _T_RE_EOL \
    _T("    echo x is 0") _T_RE_EOL \
    _T("    if $(y) == 0 then") _T_RE_EOL \
    _T("      echo y is 0") _T_RE_EOL \
    _T("    else") _T_RE_EOL \
    _T("      echo y is not 0") _T_RE_EOL \
    _T("    endif") _T_RE_EOL \
    _T("  else") _T_RE_EOL \
    _T("    echo x is not 0") _T_RE_EOL \
    _T("  endif") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  As you can see, different IF-GOTO constructions in combination with") _T_RE_EOL \
    _T("  another GOTOs and LABELs can serve as IF-ELSE, FOR and WHILE.") _T_RE_EOL \
    _T("  This allows to apply more-or-less complicated conditional logic") _T_RE_EOL \
    _T("  inside NppExec\'s scripts.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  if~, else, endif, goto, label, set") _T_RE_EOL
  },

  // IF~
  {
    CScriptEngine::DoCalcIfCommand::Name(),
    _T("COMMAND:  if~") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  1. if~ <condition> goto <label>") _T_RE_EOL \
    _T("  2. if~ <condition> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("  3. if~ <condition> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     else") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("  4. if~ <condition1> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     else if~ <condition2> then") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     else") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("  5. if~ <condition1> goto <label1>") _T_RE_EOL \
    _T("     else if~ <condition2> goto <label2>") _T_RE_EOL \
    _T("     else") _T_RE_EOL \
    _T("       ...") _T_RE_EOL \
    _T("     endif") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  First calculates the operands, then checks the condition.") _T_RE_EOL \
    _T("  If the condition is true, jumps to the specified label.") _T_RE_EOL \
    _T("  If the condition is false, proceeds to the next line.") _T_RE_EOL \
    _T("  * If the specified label can not be found within the current script,") _T_RE_EOL \
    _T("  this command reports an error and proceeds to the next line.") _T_RE_EOL \
    _T("  ** You should always place \"if\" and \"goto\" on the same line.") _T_RE_EOL \
    _T("  Available conditions:") _T_RE_EOL \
    _T("  a == b  - equal:             1 == 1,  NPPMSG == WM_USER + 1000") _T_RE_EOL \
    _T("  a = b   - equal:             2 = 2,   $(x) + 5 = $(x) + 5") _T_RE_EOL \
    _T("  a != b  - not equal:         1 != 2,  $(x) + 1 != sin($(y))") _T_RE_EOL \
    _T("  a <> b  - not equal:         1 <> 2,  NPPMSG <> pi") _T_RE_EOL \
    _T("  a > b   - greater:           2 > 1,   $(x) + 1 > 1") _T_RE_EOL \
    _T("  a < b   - less:             -2 < 1,   $(x) - 10 < $(x) - 5") _T_RE_EOL \
    _T("  a >= b  - greater or equal:  0 >= 0,  $(x) + 0 >= $(x)") _T_RE_EOL \
    _T("  a <= b  - less or equal:     1 <= 2,  $(x) - 2 <= $(y) - 2") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  IF~ deals only with numbers and numeric calculations.") _T_RE_EOL \
    _T("  A string operand will cause a syntax error.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  if, else, endif, goto, label, set") _T_RE_EOL
  },

  // LABEL
  {
    CScriptEngine::DoLabelCommand::Name(),
    _T("COMMAND:  label, :") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  label <LabelName>") _T_RE_EOL \
    _T("  label :<LabelName>") _T_RE_EOL \
    _T("  :<LabelName>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Declares a label within the current script.") _T_RE_EOL \
    _T("  You can jump to a label using the GOTO command.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  :Label1") _T_RE_EOL \
    _T("  label MyLabelB") _T_RE_EOL \
    _T("  :C") _T_RE_EOL \
    _T("  label 4") _T_RE_EOL \
    _T("  label :4  // equal to label 4") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  You can give any name to your label. Leading and trailing spaces are") _T_RE_EOL \
    _T("  ignored. Leading ':' is ignored (i.e. LABEL X is the same as LABEL :X).") _T_RE_EOL \
    _T("  Each label \"lives\" within a script where it is declared. You can not") _T_RE_EOL \
    _T("  jump to a label located inside another script - either outer or inner") _T_RE_EOL \
    _T("  one. (And this is the difference between labels and variables: each") _T_RE_EOL \
    _T("  script has its own set of labels which are not shared between scripts,") _T_RE_EOL \
    _T("  whereas any declared variable is visible and exists everywhere - unless") _T_RE_EOL \
    _T("  it is a local variable, of course.)") _T_RE_EOL \
    _T("  Thus, you can declare labels with the same names inside different") _T_RE_EOL \
    _T("  scripts (but not within one script). E.g. you may have labels named") _T_RE_EOL \
    _T("  A, B and C inside SomeScript1 and the same label names A, B and C") _T_RE_EOL \
    _T("  inside SomeScript2 - and these labels are different because they") _T_RE_EOL \
    _T("  \"exist\" in different scripts, even if SomeScript1 then uses") _T_RE_EOL \
    _T("  NPP_EXEC to execute SomeScript2 or vice versa.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  goto, if, if~") _T_RE_EOL
  },

  // GOTO
  {
    CScriptEngine::DoGoToCommand::Name(),
    _T("COMMAND:  goto") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  goto <LabelName>") _T_RE_EOL \
    _T("  goto :<LabelName>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Jumps to the label") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Theoretical example:") _T_RE_EOL \
    _T("  // Pure GOTOs with short label names") _T_RE_EOL \
    _T("  goto 1") _T_RE_EOL \
    _T("  :2") _T_RE_EOL \
    _T("  goto 3") _T_RE_EOL \
    _T("  :1") _T_RE_EOL \
    _T("  goto 2") _T_RE_EOL \
    _T("  :3") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // The very same example,") _T_RE_EOL \
    _T("  // using short label names with preceding ':'") _T_RE_EOL \
    _T("  goto :1") _T_RE_EOL \
    _T("  :2") _T_RE_EOL \
    _T("  goto :3") _T_RE_EOL \
    _T("  :1") _T_RE_EOL \
    _T("  goto :2") _T_RE_EOL \
    _T("  :3") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Theoretical example:") _T_RE_EOL \
    _T("  // Endless cycle with neat label name...") _T_RE_EOL \
    _T("  :Neat Label Name") _T_RE_EOL \
    _T("  goto Neat Label Name") _T_RE_EOL \
    _T("  // ...now you can either wait until the number of executed GOTOs") _T_RE_EOL \
    _T("  // exceeds the value of GoTo_MaxCount - or just close the Console.") _T_RE_EOL \
    _T("  // When the Console is closed, the script execution is stopped.") _T_RE_EOL \
    _T("  ////////////////////////") _T_RE_EOL \
    _T("  // Practical example:") _T_RE_EOL \
    _T("  // IF-GOTO") _T_RE_EOL \
    _T("  if \"$(a)\" == \"\" goto @a_is_empty") _T_RE_EOL \
    _T("  echo a is not empty: a=$(a)") _T_RE_EOL \
    _T("  goto done") _T_RE_EOL \
    _T("  :@a_is_empty") _T_RE_EOL \
    _T("  echo a is empty") _T_RE_EOL \
    _T("  :done") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  You can give any name to your label. Leading and trailing spaces are") _T_RE_EOL \
    _T("  ignored. Leading ':' is ignored (i.e. GOTO X is the same as GOTO :X).") _T_RE_EOL \
    _T("  Each label \"lives\" within a script where it is declared. You can not") _T_RE_EOL \
    _T("  jump to a label located inside another script - either outer or inner") _T_RE_EOL \
    _T("  one. (And this is the difference between labels and variables: each") _T_RE_EOL \
    _T("  script has its own set of labels which are not shared between scripts,") _T_RE_EOL \
    _T("  whereas any declared variable is visible and exists everywhere - unless") _T_RE_EOL \
    _T("  it is a local variable, of course.)") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  label, if, if~, exit") _T_RE_EOL
  },

  // ELSE
  {
    CScriptEngine::DoElseCommand::Name(),
    _T("COMMAND:  else") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  if <condition>") _T_RE_EOL \
    _T("    ...") _T_RE_EOL \
    _T("  else if <condition>") _T_RE_EOL \
    _T("    ...") _T_RE_EOL \
    _T("  else") _T_RE_EOL \
    _T("    ...") _T_RE_EOL \
    _T("  endif") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Implements IF ... ELSE ... ENDIF") _T_RE_EOL \
    _T("  or         IF ... ELSE IF ... ENDIF") _T_RE_EOL \
    _T("  or         IF ... ELSE IF ... ELSE ... ENDIF") _T_RE_EOL \
    _T("  or         IF ... ELSE IF ... ELSE IF ... ENDIF") _T_RE_EOL \
    _T("  and similar constructions.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  see: if") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  if, if~, endif") _T_RE_EOL
  },

  // ENDIF
  {
    CScriptEngine::DoEndIfCommand::Name(),
    _T("COMMAND:  endif") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  if <condition>") _T_RE_EOL \
    _T("    ...") _T_RE_EOL \
    _T("  endif") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  The end of the IF ... ENDIF construction.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  see: if, else") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  if, if~, else") _T_RE_EOL
  },

  // EXIT
  {
    CScriptEngine::DoExitCommand::Name(),
    _T("COMMAND:  exit") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  exit") _T_RE_EOL \
    _T("  exit 0") _T_RE_EOL \
    _T("  exit -1") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Exits (aborts) the current NppExec's script.") _T_RE_EOL \
    _T("  The \'exit\' and \'exit 0\' mean \"soft exit\": when such command is located in") _T_RE_EOL \
    _T("  NppExec's script being NPP_EXEC-ed, it aborts only this NPP_EXEC-ed script.") _T_RE_EOL \
    _T("  The \'exit -1\' means \"hard exit\": when this command is located in NppExec's") _T_RE_EOL \
    _T("  script being NPP_EXEC-ed, it aborts the root script.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // To illustrate the difference between \"soft\" and \"hard\" exit,") _T_RE_EOL \
    _T("  // let's look at the following 2 scripts:") _T_RE_EOL \
    _T("  //") _T_RE_EOL \
    _T("  // :: This is Script-1 ::") _T_RE_EOL \
    _T("  echo Script-1 : before Script-2") _T_RE_EOL \
    _T("  npp_exec \"Script-2\"") _T_RE_EOL \
    _T("  echo Script-1 : after Script-2") _T_RE_EOL \
    _T("  //") _T_RE_EOL \
    _T("  // :: This is Script-2 ::") _T_RE_EOL \
    _T("  echo Script-2 : before \'exit\'") _T_RE_EOL \
    _T("  exit  // \"soft\" exit") _T_RE_EOL \
    _T("  echo Script-2 : after \'exit\'") _T_RE_EOL \
    _T("  //") _T_RE_EOL \
    _T("  // Now let's run the Script-1.") _T_RE_EOL \
    _T("  // It prints:") _T_RE_EOL \
    _T("  //   Script-1 : before Script-2") _T_RE_EOL \
    _T("  //   Script-2 : before \'exit\'") _T_RE_EOL \
    _T("  //   Script-1 : after Script-2") _T_RE_EOL \
    _T("  // i.e. only the Script-2 exited, whereas the Script-1 continued.") _T_RE_EOL \
    _T("  // Now let's modify the Script-2 to use the exit type of -1:") _T_RE_EOL \
    _T("  //") _T_RE_EOL \
    _T("  // :: This is Script-2 ::") _T_RE_EOL \
    _T("  echo Script-2 : before \'exit\'") _T_RE_EOL \
    _T("  exit -1  // \"hard\" exit") _T_RE_EOL \
    _T("  echo Script-2 : after \'exit\'") _T_RE_EOL \
    _T("  //") _T_RE_EOL \
    _T("  // Now let's run the Script-1.") _T_RE_EOL \
    _T("  // It prints:") _T_RE_EOL \
    _T("  //   Script-1 : before Script-2") _T_RE_EOL \
    _T("  //   Script-2 : before \'exit\'") _T_RE_EOL \
    _T("  // and that's all.") _T_RE_EOL \
    _T("  // I.e. the \'exit -1\' has aborted the Script-1 as well.") _T_RE_EOL \
    _T("  //") _T_RE_EOL \
    _T("  // Usage example:") _T_RE_EOL \
    _T("  set local error = Some error") _T_RE_EOL \
    _T("  if \"$(error)\" != \"\"") _T_RE_EOL \
    _T("    messagebox \"$(error)\" :: err") _T_RE_EOL \
    _T("    exit") _T_RE_EOL \
    _T("  endif") _T_RE_EOL \
    _T("  echo There was no error, continuing...") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  goto, if, if~") _T_RE_EOL
  },

  // PROC_INPUT
  {
    CScriptEngine::DoProcInputCommand::Name(),
    _T("COMMAND:  proc_input") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  proc_input <string>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sends a given string to the running process.") _T_RE_EOL \
    _T("  The string can be single-line or multi-line.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  proc_input $(SELECTED_TEXT)  // sends selected text to the process") _T_RE_EOL \
    _T("  proc_input $(CLIPBOARD_TEXT) // sends clipboard text to the process") _T_RE_EOL \
    _T("  set local s ~ strunescape echo 123\\necho 456") _T_RE_EOL \
    _T("  proc_input $(s)") _T_RE_EOL \
    _T("  proc_input exit") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  proc_signal, @exit_cmd (\"help @exit_cmd\"), npp_console") _T_RE_EOL
  },

  // PROC_SIGNAL
  {
    CScriptEngine::DoProcSignalCommand::Name(),
    _T("COMMAND:  proc_signal") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  proc_signal <signal>") _T_RE_EOL \
    _T("  proc_signal <signal> <timeout>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  proc_signal ctrl-c        // sends Ctrl-C to the process") _T_RE_EOL \
    _T("  proc_signal ctrl-break    // sends Ctrl-Break to the process") _T_RE_EOL \
    _T("  proc_signal wm_close      // sends WM_CLOSE to the process") _T_RE_EOL \
    _T("  proc_signal wm_close 1000 // wait for reaction during 1000 ms") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  proc_signal CtrlC                 // sends Ctrl-C") _T_RE_EOL \
    _T("  proc_signal Ctrl-C Ctrl-Break     // sends Ctrl-C, then Ctrl-Break") _T_RE_EOL \
    _T("  proc_signal ctrl-break wm_close   // sends Ctrl-Break, then WM_CLOSE") _T_RE_EOL \
    _T("  proc_signal ctrlbreak wmclose 500 // Ctrl-Break, waits 500 ms, WM_CLOSE") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  This command is expected to be sent while a child console process is running") _T_RE_EOL \
    _T("  in NppExec\'s Console. So the usual syntax of this command will be:") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("proc_signal <signal>") _T_RE_EOL \
    _T("  Also this command can be combined with $(@exit_cmd), e.g. to break the child") _T_RE_EOL \
    _T("  process automatically when NppExec's Console is closed (and when the child") _T_RE_EOL \
    _T("  process can not be finished by some regular exit command):") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("set local @exit_cmd = ") DEFAULT_NPPEXEC_CMD_PREFIX _T("proc_signal CtrlC WMCLOSE") _T_RE_EOL \
    _T("    // now close the NppExec's Console - or type:") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("npp_console off") _T_RE_EOL \
    _T("  This latest example actually has a similar effect as the following one:") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("set local @exit_cmd = ") DEFAULT_NPPEXEC_CMD_PREFIX _T("taskkill /pid $(PID)") _T_RE_EOL \
    _T("  Anyway, it's recommended to use an exit command (such as \"exit\" for cmd and") _T_RE_EOL \
    _T("  \"exit()\" for python) whenever possible to let the process exit normally.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  proc_input, @exit_cmd (\"help @exit_cmd\"), npp_console") _T_RE_EOL
  },

  // SLEEP
  {
    CScriptEngine::DoSleepCommand::Name(),
    _T("COMMAND:  sleep") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  sleep <ms>") _T_RE_EOL \
    _T("  sleep <ms> <text>") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  Sleeps during <ms> milliseconds.") _T_RE_EOL \
    _T("  If the <text> parameter is specified, it is printed first.") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  // without the text:") _T_RE_EOL \
    _T("  sleep 1000 // sleeps during 1 second") _T_RE_EOL \
    _T("  // with the text:") _T_RE_EOL \
    _T("  set local t = 2500") _T_RE_EOL \
    _T("  sleep $(t) \"Sleeping during $(t) milliseconds...\"") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  It is not necessary to use quotes for the <text> parameter as this command") _T_RE_EOL \
    _T("  expects either 1 or 2 parameters, not more.") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  echo, set") _T_RE_EOL
  },

  // @EXIT_CMD
  {
    _T("@EXIT_CMD"),
    _T("VARIABLE:  @exit_cmd / @exit_cmd_silent") _T_RE_EOL \
    _T("USAGE:") _T_RE_EOL \
    _T("  set @exit_cmd = exit // sets the child process'es exit command") _T_RE_EOL \
    _T("  set @exit_cmd_silent = exit // sets the child process'es silent exit command") _T_RE_EOL \
    _T("DESCRIPTION:") _T_RE_EOL \
    _T("  When NppExec's Console is being closed or another NppExec's script is about") _T_RE_EOL \
    _T("  to start while a child console process is running in NppExec's Console, the") _T_RE_EOL \
    _T("  specified exit command is sent to the running process automatically.") _T_RE_EOL \
    _T("  The exit command is expected to finish the process normally - otherwise, if") _T_RE_EOL \
    _T("  the process does not finish, the Console won't close.") _T_RE_EOL \
    _T("  In case of @exit_cmd, the exit command is printed in the Console.") _T_RE_EOL \
    _T("  In case of @exit_cmd_silent, the exit command is not printed in the Console.") _T_RE_EOL \
    _T("  Note: the Console can be closed either manually (by clicking the \"x\" button)") _T_RE_EOL \
    _T("  or programmatically (via \"npp_console off\").") _T_RE_EOL \
    _T("EXAMPLES:") _T_RE_EOL \
    _T("  set local @exit_cmd = exit // the exit command is \"exit\"") _T_RE_EOL \
    _T("  cmd // running the cmd.exe") _T_RE_EOL \
    _T("  // now close the Console - \"exit\" will be sent to the cmd.exe automatically") _T_RE_EOL \
    _T("REMARKS:") _T_RE_EOL \
    _T("  An exit command is specific to a running process. For example, it is \"exit\"") _T_RE_EOL \
    _T("  for cmd, and it is \"exit()\" for python. A proper exit command is a recommended") _T_RE_EOL \
    _T("  way to finish a process.") _T_RE_EOL \
    _T("  It is also possible to specify an exit command that will kill the running") _T_RE_EOL \
    _T("  process rather than finish it normally. This is not recommended! Anyway,") _T_RE_EOL \
    _T("  here is an example. It also demonstrates a delayed substitution of $(PID):") _T_RE_EOL \
    _T("    set +v local @exit_cmd = ") DEFAULT_NPPEXEC_CMD_PREFIX _T("taskkill /PID $(PID)") _T_RE_EOL \
    _T("  It is possible to close NppExec's Console by typing \"npp_console off\" in it.") _T_RE_EOL \
    _T("  Even when a child console process (such as \"cmd\" or \"python -u -i\") is running") _T_RE_EOL \
    _T("  and waiting for the input, the Console can be closed by typing:") _T_RE_EOL \
    _T("    ") DEFAULT_NPPEXEC_CMD_PREFIX _T("npp_console off") _T_RE_EOL \
    _T("SEE ALSO:") _T_RE_EOL \
    _T("  set, proc_signal, npp_console") _T_RE_EOL
  }

};

typedef BOOL (WINAPI *TRANSPARENCYFUNC)(HWND, COLORREF, BYTE, DWORD);

namespace ConsoleDlg 
{
  void    AddCommandToHistoryList(const tstr& S);
  
  void    OnClose(HWND hDlg, unsigned int nCloseConsoleFlags);
  void    OnDestroy(HWND hDlg);
  void    OnInitDialog(HWND hDlg);
  INT_PTR OnNotify(HWND hDlg, LPARAM lParam);
  INT_PTR OnCtlColorEdit(WPARAM wParam, LPARAM lParam);
  INT_PTR OnPaste(CAnyRichEdit& Edit, MSGFILTER* lpmsgf);
  void    OnShowWindow(HWND hDlg);
  void    OnSize(HWND hDlg);

  void    loadCmdVarsList();
  bool    loadCmdHistory();
  bool    saveCmdHistory();

  bool    isMultilineInput(CAnyRichEdit& Edit, int* pnTotalLines = NULL, int* pnFirstInputLine = NULL);
  bool    isCmdHistoryRequest(CAnyRichEdit& Edit, int* pnCurrLine);
  tstr    getInputText(CAnyRichEdit& Edit, bool* pisMultiline, int* pnTotalTextLen);

  void    enableFindControls(bool bEnable);

  void    doWordWrap(HWND hDlg);
  void    doHideToggled(HWND hDlg);
  void    doTabAsChar(HWND hDlg);

  INT     GetCompleteLine(const CAnyRichEdit& Edit, INT nLine, TCHAR* lpTextBuf, WORD wTextBufSize, INT* pnLineStartIndex);

  bool    GoToLineIfWarningAnalyzerMatch(CAnyRichEdit& Edit, const int nLine);

  void    printConsoleReady();

//#undef _consoledlg_re_subclass_
#define _consoledlg_re_subclass_

#ifdef _consoledlg_re_subclass_
  
  LRESULT CALLBACK RichEditWndProc(
    HWND   hEd, 
    UINT   uMessage, 
    WPARAM wParam, 
    LPARAM lParam);

  WNDPROC OriginalRichEditProc = NULL;

#endif

#undef _consoledlg_keys_log_
//#define _consoledlg_keys_log_

#ifdef _consoledlg_keys_log_
  
  #include <stdio.h>
  
  FILE* fLog = NULL;

#endif

  LRESULT CALLBACK findEditWndProc(HWND, UINT, WPARAM, LPARAM);
  WNDPROC findEditWndProc0 = NULL;

  HICON   hTabIcon = NULL;
  HFONT   hFont = NULL;
  LOGFONT LogFont = { 15, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
                                   0, 0, 0, 0, _T("Courier New") };
  tTbData dockData;

  CListT<tstr>      CmdHistoryList;
  CListT<tstr>      CmdVarsList;
  CListItemT<tstr>* pCmdHistoryItemPtr = NULL;
  bool              bCmdHistoryUpdated = false;

  bool              bWordWrap = false;
  bool              bHideToggled = false;
  bool              bTabAsChar = false;
  bool              bIsDocked = false;

  bool              bFindControls = false;
  CAnyCheckBox      chFindCase;
  CAnyWindow        edFind;
  CAnyWindow        btFindClose;
  CAnyCheckBox      chWordWrap;
  CAnyCheckBox      chHideToggled;
  CAnyCheckBox      chTabAsChar;

  CDirFileLister    tabDirFileLister;

  int               GoToError_nCurrentLine = -1;

}

INT_PTR CALLBACK ConsoleDlgProc(
  HWND   hDlg, 
  UINT   uMessage, 
  WPARAM wParam, 
  LPARAM lParam)
{
  
  /*
  if (uMessage == WM_ACTIVATE)
  {
    MessageBox(NULL, "WM_ACTIVATE", "ConsoleDlgProc", MB_OK);
  }
  else
  */
  
  if (uMessage == WM_COMMAND)
  {
    switch (LOWORD(wParam)) 
    {
      case IDOK: // Save
      {
        if (ConsoleDlg::bFindControls)
        {
          ConsoleDlg::findEditWndProc(ConsoleDlg::edFind.m_hWnd, WM_KEYDOWN, VK_RETURN, 0);
        }
        else
        {
          ConsoleDlg::OnClose(hDlg, 0);
        }
        return 1;
      }
      case IDCANCEL: // Cancel
      case IDC_BT_FINDCLOSE:
      {
        if (ConsoleDlg::bFindControls)
        {
          ConsoleDlg::enableFindControls(false);
          ConsoleDlg::OnSize(hDlg);
        }
        else
        {
          ConsoleDlg::OnClose(hDlg, 0);
        }
        return 1;
      }
      case IDC_CH_CASE:
      {
        if (HIWORD(wParam) == BN_CLICKED)
        {
          if (ConsoleDlg::bFindControls)
          {
            ConsoleDlg::edFind.SetFocus();
          }
          else
          {
            Runtime::GetNppExec().GetConsole().GetConsoleEdit().SetFocus();
          }
          return 1;
        }
        break;
      }
      case IDC_CH_WORDWRAP:
      {
        if (HIWORD(wParam) == BN_CLICKED)
        {
          ConsoleDlg::doWordWrap(hDlg);
          return 1;
        }
        break;
      }
      case IDC_CH_HIDETOGGLED:
      {
        if (HIWORD(wParam) == BN_CLICKED)
        {
          ConsoleDlg::doHideToggled(hDlg);
          return 1;
        }
        break;
      }
      case IDC_CH_TABASCHAR:
      {
        if (HIWORD(wParam) == BN_CLICKED)
        {
          ConsoleDlg::doTabAsChar(hDlg);
          return 1;
        }
        break;
      }
      default:
        break;
    }
  }

  else if (uMessage == WM_NOTIFY)
  {
    return ConsoleDlg::OnNotify(hDlg, lParam);
  }

  else if (uMessage == WM_CTLCOLOREDIT)
  {
    return ConsoleDlg::OnCtlColorEdit(wParam, lParam);
  }

  else if (uMessage == WM_SIZE)
  {
    ConsoleDlg::OnSize(hDlg);
  }

  else if (uMessage == WM_SYSCOMMAND)
  {
    if (wParam == SC_CLOSE)
    {
      ConsoleDlg::OnClose(hDlg, 0);
      return 1;
    }
  }

  else if (uMessage == WM_CONSOLEDLG_CLOSE)
  {
    ConsoleDlg::OnClose(hDlg, ConsoleDlg::ccfCmdNppConsole);
    return 1;
  }

  else if (uMessage == WM_CONSOLEDLG_EXECFUNCITEM)
  {
    int i = (int) wParam;
    if (g_funcItem[i]._pFunc)
    {
      g_funcItem[i]._pFunc();
    }
    bFuncItemEntered = false;
    return 1;
  }

  else if (uMessage == WM_CONSOLEDLG_UPDATECOLOR)
  {
    HWND hEdFindWnd = ConsoleDlg::edFind.m_hWnd;
    if (hEdFindWnd && ::IsWindowVisible(hEdFindWnd))
    {
      ::InvalidateRect(hEdFindWnd, NULL, TRUE);
      ::UpdateWindow(hEdFindWnd);
    }
    return 1;
  }
  /*
  else if (uMessage == WM_ACTIVATE)
  {
      // we don't get here...
      if ( LOWORD(wParam) == WA_INACTIVE )
          ConsoleDlg::EnableTransparency(true);
      else
          ConsoleDlg::EnableTransparency(false);
  }
  */

  else if (uMessage == WM_SHOWWINDOW)
  {
    if ( wParam ) // the window is being shown
      ConsoleDlg::OnShowWindow(hDlg);
  }

  else if (uMessage == WM_INITDIALOG)
  {
    ConsoleDlg::OnInitDialog(hDlg);
  }

  else if (uMessage == WM_DESTROY)
  {
    ConsoleDlg::OnDestroy(hDlg);
  }

  else if (uMessage == WM_NPPCLOSE)
  {
    ConsoleDlg::OnClose(hDlg, ConsoleDlg::ccfNppIsClosing);
    return 1;
  }

  /*
  // This doesn't work.
  // It seems these messages are intercepted
  //   before they can be processed here.
  else if (uMessage == WM_NCLBUTTONDOWN || uMessage == WM_LBUTTONDOWN)
  {
    MessageBox(NULL, "left mouse btn pressed", "ConsoleDlgProc", MB_OK);
  }
  */
  
  return 0;
}


void ConsoleDlg::AddCommandToHistoryList(const tstr& S)
{
  if (S.length() > 0)
  {
    CStaticOptionsManager& Options = Runtime::GetNppExec().GetOptions();
    if (Options.GetBool(OPTB_CONSOLE_CMDHISTORY))
    {
      CListItemT<tstr>* p = CmdHistoryList.FindExact(S);
      if (p)
      {
        if (pCmdHistoryItemPtr == p)
        {
          pCmdHistoryItemPtr = NULL;
        }
        CmdHistoryList.Delete(p);
      }
      if (CmdHistoryList.GetCount() >= Options.GetInt(OPTI_CMDHISTORY_MAXITEMS))
      {
        if (pCmdHistoryItemPtr == CmdHistoryList.GetFirst())
        {
          pCmdHistoryItemPtr = NULL;
        }
        CmdHistoryList.DeleteFirst();
      }
      CmdHistoryList.Add(S);
      bCmdHistoryUpdated = true;
      pCmdHistoryItemPtr = NULL;
    }
  }
}


void ConsoleDlg::OnClose(HWND hDlg, unsigned int nCloseConsoleFlags)
{
//  if ( !IsWindowVisible(hDlg) )
//    return;

  Runtime::GetLogger().Add_WithoutOutput( _T("; ConsoleDlg::OnClose()") );

  bool bClose = true;
  CNppExec& NppExec = Runtime::GetNppExec();
  CNppExecCommandExecutor& CommandExecutor = NppExec.GetCommandExecutor();

  std::shared_ptr<CScriptEngine> pScriptEngine = CommandExecutor.GetRunningScriptEngine();
  if (pScriptEngine || CommandExecutor.IsScriptRunningOrQueued())
  {
    if (pScriptEngine && pScriptEngine->IsCollateral() && pScriptEngine->IsChildProcessRunning())
    {
      // We are closing the Console while a collateral script is being executed.
      // As there is no script in the ExecuteQueue, the DoCloseConsoleCommand will not
      // expire and thus CanStartScriptOrCommand() will not be called (this call would
      // send the exit command). This must be sorted out.
      unsigned int nFlags = CNppExecCommandExecutor::TryExitChildProcessCommand::fCloseConsoleOnExit;
      if ((nCloseConsoleFlags & ccfNppIsClosing) != 0)
        nFlags |= CNppExecCommandExecutor::TryExitChildProcessCommand::fNppIsClosing;
      if ((nCloseConsoleFlags & ccfCmdNppConsole) != 0)
        nFlags |= CNppExecCommandExecutor::TryExitChildProcessCommand::fCmdNppConsole;
      CNppExecCommandExecutor::ScriptableCommand * pExitChildProcCommand = new CNppExecCommandExecutor::TryExitChildProcessCommand(tstr(), nFlags);
      CommandExecutor.ExecuteCommand(pExitChildProcCommand);
      bClose = false;
    }
    else
    {
      if ((nCloseConsoleFlags & ccfCmdNppConsole) == 0)
      {
        CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::DoCloseConsoleCommand(tstr(), nCloseConsoleFlags);
        CommandExecutor.ExecuteCommand(pCommand);
        bClose = false;
      }
      else
      {
        CNppExecCommandExecutor::RunningScriptEnginesStopper ScriptsStopper(&CommandExecutor, nCloseConsoleFlags);
        ScriptsStopper.SendAbortNotificationToRunningScripts( _T("; Aborting the script to close the Console (from ConsoleDlg::OnClose)") );
      }
    }
  }
  
  if (bClose)
  {
    NppExec.setConsoleVisible(false);
    // When _consoleIsVisible is set to false, the child process 
    // is killed within CChildProcess::Create.
    // Therefore it's very important to process
    // closing of this dialog here.
    /*
    NppExec.SendNppMsg(WM_MODELESSDIALOG, 
        (WPARAM) MODELESSDIALOGREMOVE, (LPARAM) hDlg); 
    ::DestroyWindow(hDlg);
    */

    /*
    HMENU hMenu = NppExec.GetNppMainMenu();
    if (hMenu)
    {
      EnableMenuItem(hMenu, g_funcItem[N_CONSOLE_FONT]._cmdID,
        MF_BYCOMMAND | (NppExec._consoleIsVisible ? MF_ENABLED : MF_GRAYED) );
    }
    */

    // ... and finally
    ::SetFocus( NppExec.GetScintillaHandle() );
  }
  else
  {
    ::SetWindowLongPtr(hDlg, DWLP_MSGRESULT, 1); // 1 means "don't close"
    //NppExec.SendNppMsg(WM_DMM_SHOW, 0, (LPARAM) hDlg);
    //::ShowWindow(NppExec.GetConsole().GetParentWnd(), SW_SHOW);
  }
}

void ConsoleDlg::OnDestroy(HWND hDlg)
{
  //MessageBox(NULL, "WM_DESTROY", "ConsoleDlg", MB_OK);
   
  /*saveCmdHistory();*/

  if (hTabIcon)
  {
    ::DestroyIcon(hTabIcon);
    hTabIcon = NULL;
  }

  HWND hDlgItem;

// get rid of the "conversion from 'LONG_PTR' to 'LONG'" shit
// (Microsoft itself does not know what is 'LONG_PTR'?)
#pragma warning(push)
#pragma warning(disable: 4244)

#ifdef _consoledlg_re_subclass_
  hDlgItem = GetDlgItem(hDlg, IDC_RE_CONSOLE);
  if (hDlgItem)
    SetWindowLongPtr(hDlgItem, GWLP_WNDPROC, (LONG_PTR) OriginalRichEditProc);
#endif

  hDlgItem = GetDlgItem(hDlg, IDC_ED_FIND);
  if (hDlgItem)
    SetWindowLongPtr(hDlgItem, GWLP_WNDPROC, (LONG_PTR) findEditWndProc0);

#pragma warning(pop)

#ifdef _consoledlg_keys_log_
  if (fLog)
  {
    fclose(fLog);
    fLog = NULL;
  }
#endif

  Runtime::GetNppExec().GetConsole().SetDialogWnd(NULL);
}

void ConsoleDlg::OnInitDialog(HWND hDlg)
{
  CNppExec& NppExec = Runtime::GetNppExec();
  CAnyRichEdit Edit;
  DWORD        dwEventMask;

  edFind.m_hWnd = GetDlgItem(hDlg, IDC_ED_FIND);
  chFindCase.m_hWnd = GetDlgItem(hDlg, IDC_CH_CASE);
  btFindClose.m_hWnd = GetDlgItem(hDlg, IDC_BT_FINDCLOSE);
  chWordWrap.m_hWnd = GetDlgItem(hDlg, IDC_CH_WORDWRAP);
  chHideToggled.m_hWnd = GetDlgItem(hDlg, IDC_CH_HIDETOGGLED);
  chTabAsChar.m_hWnd = GetDlgItem(hDlg, IDC_CH_TABASCHAR);
  
  enableFindControls(false);
  
  Edit.m_hWnd = GetDlgItem(hDlg, IDC_RE_CONSOLE);
  NppExec.GetConsole().SetConsoleWnd( Edit.m_hWnd );
  NppExec.GetConsole().SetDialogWnd( hDlg );

// get rid of the "conversion from 'LONG_PTR' to 'LONG'" shit
// get rid of the "conversion from 'LONG' to 'WNDPROC'" shit
// (Microsoft itself does not know what is 'LONG_PTR'?)
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4312)

#ifdef _consoledlg_re_subclass_
  OriginalRichEditProc = (WNDPROC) SetWindowLongPtr(Edit.m_hWnd, 
    GWLP_WNDPROC, (LONG_PTR) RichEditWndProc);
#endif

  findEditWndProc0 = (WNDPROC) SetWindowLongPtr(edFind.m_hWnd,
    GWLP_WNDPROC, (LONG_PTR) findEditWndProc);

#pragma warning(pop)

#ifdef _consoledlg_keys_log_
  char test_str[128];
  
  fLog = fopen("Keys.log", "w");

  if (fLog)
  {
    lstrcpyA(test_str, "ConsoleDlg::OnInitDialog ...\r\n");
    fwrite(test_str, sizeof(char), lstrlenA(test_str), fLog);
  }
#endif

  dwEventMask = Edit.GetEventMask();

#ifdef _consoledlg_keys_log_
  if (fLog)
  {
    wsprintfA(test_str, "Original EventMask: %08X\r\n", dwEventMask);
    fwrite(test_str, sizeof(char), lstrlenA(test_str), fLog);
  }
#endif

  if (!(dwEventMask & (ENM_KEYEVENTS | ENM_MOUSEEVENTS)))
  {
    dwEventMask |= (ENM_KEYEVENTS | ENM_MOUSEEVENTS);
    Edit.SetEventMask(dwEventMask);

#ifdef _consoledlg_keys_log_
    if (fLog)
    {
      wsprintfA(test_str, "Desired EventMask: %08X\r\n", dwEventMask);
      fwrite(test_str, sizeof(char), lstrlenA(test_str), fLog);
      wsprintfA(test_str, "Set EventMask: %08X\r\n", Edit.GetEventMask());
      fwrite(test_str, sizeof(char), lstrlenA(test_str), fLog);
    }
#endif
  
  }

  if (NppExec._consoleFont)
  {
    int dataSize = 0;
    const LOGFONT* pLogFont = (const LOGFONT *) NppExec.GetOptions().GetData(OPTD_CONSOLE_FONT, &dataSize);
    NppExec.GetConsole().GetConsoleEdit().SetFont(pLogFont);
  }

  NppExec.GetConsole().ApplyEditorColours(false);
  NppExec.GetConsole().UpdateColours();

  Edit.ExLimitText(NppExec.GetOptions().GetInt(OPTI_RICHEDIT_MAXTEXTLEN));

  nConsoleFirstUnlockedPos = 0;

  loadCmdVarsList();
  loadCmdHistory();
    
  bWordWrap = !NppExec.GetOptions().GetBool(OPTB_CONSOLE_WORDWRAP);
  // following doWordWrap() inverts the value of bWordWrap :)
  doWordWrap(hDlg);

  bHideToggled = !NppExec.GetOptions().GetBool(OPTB_CONSOLE_HIDETOGGLED);
  // following doHideToggled() inverts the value of bHideToggled :)
  doHideToggled(hDlg);

  bTabAsChar = !NppExec.GetOptions().GetBool(OPTB_CONSOLE_TABASCHAR);
  // following doTabAsChar() inverts the value of bTabAsChar :)
  doTabAsChar(hDlg);
  
  // docking

  RECT   rect;
  
  rect.left   = 0;
  rect.top    = 0;
  rect.right  = 0;
  rect.bottom = 0;

  hTabIcon = (HICON) ::LoadImage( (HINSTANCE) NppExec.m_hDllModule, 
    MAKEINTRESOURCE(IDI_CONSOLEICON), IMAGE_ICON, 0, 0, 
    LR_LOADMAP3DCOLORS | LR_LOADTRANSPARENT );

  int dlgId = N_SHOWCONSOLE;
  if ( NppExec.GetOptions().GetInt(OPTI_CONSOLE_VISIBLE) != CAdvOptDlg::CON_AUTO )
      dlgId = -1;

  dockData.hClient       = hDlg;
  dockData.pszName       = CONSOLE_DLG_TITLE;
  dockData.dlgID         = dlgId;
  dockData.uMask         = DWS_DF_CONT_BOTTOM | DWS_ICONTAB;
  dockData.hIconTab      = hTabIcon;
  dockData.pszAddInfo    = NULL;
  dockData.rcFloat       = rect;
  dockData.iPrevCont     = -1; // Don said: "this initialization is necessary"
  dockData.pszModuleName = PLUGIN_NAME_DLL;

  
}

void ConsoleDlg::DockDialog()
{
  static bool bIsInitializedDocked = false;

  if ( !bIsInitializedDocked )
  {
    CNppExec& NppExec = Runtime::GetNppExec();

    NppExec.SendNppMsg( NPPM_DMMREGASDCKDLG, 0, (LPARAM) &dockData );
  
    NppExec.SendNppMsg( NPPM_MODELESSDIALOG, 
        (WPARAM) MODELESSDIALOGADD, (LPARAM) NppExec.GetConsole().GetDialogWnd() );
  
    bIsInitializedDocked = true;
  }
}

bool ConsoleDlg::IsDocked()
{
  return bIsDocked;
}

const TCHAR* ConsoleDlg::GetTitle()
{
  return dockData.pszName;
}

bool ConsoleDlg::EnableTransparency(bool bEnable)
{
    static bool bTransparencyFuncInitialized = false;
    static TRANSPARENCYFUNC lpTransparencyFunc = NULL;

    if ( !bTransparencyFuncInitialized )
    {
        HMODULE hUser32 = ::GetModuleHandle(_T("user32"));
        if ( hUser32 )
            lpTransparencyFunc = (TRANSPARENCYFUNC) ::GetProcAddress(hUser32, "SetLayeredWindowAttributes");
        
        bTransparencyFuncInitialized = true;
    }
    
    if ( lpTransparencyFunc && !IsDocked() )
    {
        HWND hDlg = NULL;
        while ( (hDlg = ::FindWindowEx(NULL, hDlg, _T("#32770"), GetTitle())) != NULL )
        {
            if ( ::GetParent(hDlg) == Runtime::GetNppExec().m_nppData._nppHandle )
                break;
        }

        if ( hDlg )
        {
            DWORD dwExStyle = ::GetWindowLong(hDlg, GWL_EXSTYLE);
            if ( bEnable && bHideToggled )
            {
                if ( !(dwExStyle & 0x00080000) )
                {
                    dwExStyle |= 0x00080000; // WS_EX_LAYERED
                    ::SetWindowLong(hDlg, GWL_EXSTYLE, dwExStyle);
                    lpTransparencyFunc(hDlg, 0, 48, 0x00000002); 
                    return true;
                }
            }
            else
            {
                if ( dwExStyle & 0x00080000 )
                {
                    dwExStyle -= 0x00080000; // WS_EX_LAYERED
                    ::SetWindowLong(hDlg, GWL_EXSTYLE, dwExStyle);
                    return true;
                }
            }
        }
    }

    return false;
}

void ConsoleDlg::GoToError(int direction)
{
    CAnyRichEdit& Edit = Runtime::GetNppExec().GetConsole().GetConsoleEdit();
    int nLineCount = Edit.GetLineCount();
    int nCheckLine = GoToError_nCurrentLine;
    nCheckLine += direction;

    if ( nCheckLine >= nLineCount )
    {
        nCheckLine = nLineCount - 1;
    }
    else if ( nCheckLine < 0 )
    {
        nCheckLine = 0;
    }

    while( (nCheckLine >= 0) && (nCheckLine < nLineCount) )
    {
        if ( GoToLineIfWarningAnalyzerMatch(Edit, nCheckLine) )
        {
            GoToError_nCurrentLine = nCheckLine;
            break;
        }
        nCheckLine += direction;
    }
}

bool ConsoleDlg::GoToLineIfWarningAnalyzerMatch(CAnyRichEdit& Edit, const int nLine)
{
    CNppExec& NppExec = Runtime::GetNppExec();
    TCHAR ch[CONSOLECOMMAND_BUFSIZE];
    int   nLineStartIndex = 0;
    int   nLineLength = GetCompleteLine(Edit, nLine, ch, CONSOLECOMMAND_BUFSIZE-1, &nLineStartIndex);
    ch[CONSOLECOMMAND_BUFSIZE-1] = 0; // just in case
        
    // RichEdit returns a string with "\r" or "\r\n" at the end. I wish M$ documented that...
    int len = lstrlen(ch);
    while ( (len > 0) && (ch[len-1] == '\n') )
    {
        --len;
        ch[len] = 0;
    }
    while ( (len > 0) && (ch[len-1] == '\r') )
    {
        --len;
        ch[len] = 0;
    }

    if ( NppExec.GetOptions().GetBool(OPTB_NPE_DEBUGLOG) || Runtime::GetLogger().IsLogFileOpen() )
    {
        Runtime::GetLogger().Clear();
        Runtime::GetLogger().Add(   _T("") );
        Runtime::GetLogger().Add(   _T("; Console\'s line double-clicked") );
        Runtime::GetLogger().Add(   _T("; WarningAnalyzer info") );
        Runtime::GetLogger().Add(   _T("; {") );
        Runtime::GetLogger().AddEx( _T(";    Input line: \"%s\""), ch );
    }

    //NppExecHelpers::StrLower(ch);  -  seems it's useless here...
    CWarningAnalyzer& WarningAnalyzer = NppExec.GetWarningAnalyzer();
    if ( WarningAnalyzer.match( ch ) )
    {
        
        if ( NppExec.GetOptions().GetBool(OPTB_NPE_DEBUGLOG) || Runtime::GetLogger().IsLogFileOpen() )
        {
            TCHAR szRcgnMask[OUTPUTFILTER_BUFSIZE];

            Runtime::GetLogger().Add(   _T(";    Match result: true") );
            WarningAnalyzer.GetMask( WarningAnalyzer.GetLastFoundIndex(), szRcgnMask, OUTPUTFILTER_BUFSIZE - 1 );
            Runtime::GetLogger().AddEx( _T(";    Active filter: \"%s\""), szRcgnMask );
            tstr sMask = szRcgnMask;
            if ( NppExec.GetMacroVars().CheckAllMacroVars(nullptr, sMask, false) )
            {
                Runtime::GetLogger().AddEx( _T(";    Active filter -> \"%s\""), sMask.c_str() );
            }
            if ( WarningAnalyzer.GetFileName()[0] )
                Runtime::GetLogger().AddEx( _T(";    * Parsed File Name: \"%s\""), WarningAnalyzer.GetFileName() );
            else
                Runtime::GetLogger().Add(   _T(";    * Parsed File Name: <none>") );
            if ( WarningAnalyzer.GetLineNumber() )
                Runtime::GetLogger().AddEx( _T(";    * Parsed Line Number: %d"), WarningAnalyzer.GetLineNumber() );
            else
                Runtime::GetLogger().Add(   _T(";    * Parsed Line Number: <none>") );
            if ( WarningAnalyzer.GetCharNumber() )
                Runtime::GetLogger().AddEx( _T(";    * Parsed Char Position: %d"), WarningAnalyzer.GetCharNumber() );
            else
                Runtime::GetLogger().Add(   _T(";    * Parsed Char Position: <none>") );
            Runtime::GetLogger().Add(   _T("; }") );
        }
        
        /*
        tstr S = "\"";
        S += WarningAnalyzer.GetFileName();
        S += "\"";
        NppExec.ShowWarning(S.c_str());
        */
        if ( WarningAnalyzer.GetLineNumber() || WarningAnalyzer.GetFileName()[0] )
        {
            Edit.ExSetSel(nLineStartIndex, nLineStartIndex + nLineLength);
            int nLine1st = (int) Edit.SendMsg(EM_GETFIRSTVISIBLELINE, 0, 0);
            if ( nLine < nLine1st )
            {
                Edit.LineScroll(nLine - nLine1st);
            }
            
            if ( WarningAnalyzer.GetFileName()[0] )
            {
                tstr fileName = WarningAnalyzer.GetFileName();

                int nView = (int) NppExec.SendNppMsg(NPPM_GETCURRENTVIEW);
                nView = (nView == 0) ? PRIMARY_VIEW : SECOND_VIEW;

                // trying the current view first
                int nFile = NppExec.findFileNameIndexInNppOpenFileNames(fileName, true, nView);
                if ( nFile == -1 )
                {
                    // trying the opposite view
                    nView = (nView == PRIMARY_VIEW) ? SECOND_VIEW : PRIMARY_VIEW;
                    nFile = NppExec.findFileNameIndexInNppOpenFileNames(fileName, true, nView);
                }

                if ( nFile != -1 )
                {
                    NppExec.SendNppMsg( NPPM_ACTIVATEDOC, (nView == PRIMARY_VIEW) ? 0 : 1, nFile );
                }
                else
                {
                    NppExec.SendNppMsg( NPPM_DOOPEN, 0, (LPARAM) fileName.c_str() );
                }
            }

            HWND hSciWnd = NppExec.GetScintillaHandle();
            const int nWarnLine = WarningAnalyzer.GetLineNumber() - 1;
            ::SendMessage( hSciWnd, SCI_ENSUREVISIBLE, (WPARAM) nWarnLine, 0 );
            ::SendMessage( hSciWnd, SCI_GOTOLINE,      (WPARAM) nWarnLine, 0 );

            if ( WarningAnalyzer.GetCharNumber() )
            {
                // position of the start of the line
                INT_PTR nWarnPos = (INT_PTR) ::SendMessage( hSciWnd, SCI_POSITIONFROMLINE, (WPARAM) nWarnLine, 0 );

                if ( nWarnPos >= 0 )
                {
                    // document's codepage
                    int nSciCodePage = (int) ::SendMessage( hSciWnd, SCI_GETCODEPAGE, 0, 0 );
                    if ( nSciCodePage == 0 )
                    {
                        // ANSI: one-byte characters
                        nWarnPos += WarningAnalyzer.GetCharNumber() - 1;
                    }
                    else
                    {
                        // Multi-byte
                        int nChars = WarningAnalyzer.GetCharNumber();
                        while ( --nChars > 0 )
                        {
                            nWarnPos = (INT_PTR) ::SendMessage( hSciWnd, SCI_POSITIONAFTER, (WPARAM) nWarnPos, 0 );
                        }
                    }
                    
                    // set the position
                    ::SendMessage( hSciWnd, SCI_GOTOPOS, (WPARAM) nWarnPos, 0 );
                }
            }

            ::SendMessage( hSciWnd, SCI_VERTICALCENTRECARET, 0, 0 );
            ::SetFocus( NppExec.GetScintillaHandle() );
            return true;
        }

    }
    else
    {
        
        if ( NppExec.GetOptions().GetBool(OPTB_NPE_DEBUGLOG) || Runtime::GetLogger().IsLogFileOpen() )
        {
            Runtime::GetLogger().Add(   _T(";    Match result: false (no matching filter found)") );
            Runtime::GetLogger().Add(   _T("; }") );
        }
        
    }

    return false;
}

INT_PTR ConsoleDlg::OnPaste(CAnyRichEdit& Edit, MSGFILTER* lpmsgf)
{
    tstr sText = NppExecHelpers::GetClipboardText();
    Edit.ReplaceSelText(sText.c_str(), TRUE);
    lpmsgf->wParam = 0;
    return 1;
}

INT_PTR ConsoleDlg::OnNotify(HWND hDlg, LPARAM lParam)
{
  NMHDR* pnmh = (NMHDR*) lParam;

  // >>> EN_MSGFILTER
  if (pnmh->code == EN_MSGFILTER)
  {
    static bool  bCommandEntered = false;
    static bool  bDoubleClkEntered = false;

    MSGFILTER* lpmsgf = (MSGFILTER*) lParam;

    switch (lpmsgf->msg)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_SYSCHAR:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
            break; // these messages are processed below

        default:
            return 0; // we are not interested in the rest of the messages
    }

    CAnyRichEdit Edit;
    Edit.m_hWnd = GetDlgItem(hDlg, IDC_RE_CONSOLE);

    // All the following code (for EN_MSGFILTER)
    // is never executed under 'wine' in Linux
    // (tested in Mandriva Linux 2007).
    // I don't know why it doesn't work there -
    // so, if you do, let me know :-)

#ifdef _consoledlg_keys_log_
    if (fLog)
    {
      int  log_str_len;
      char log_str[256];

      static bool bfrst = true;
      if (bfrst)
      {
        lstrcpyA(log_str, "Inside EN_MSGFILTER\r\n\r\n");
        fwrite(log_str, sizeof(char), lstrlenA(log_str), fLog);
        bfrst = false;
      }


      if ((lpmsgf->msg == WM_KEYDOWN) || 
          (lpmsgf->msg == WM_KEYUP))
      {
        const char* fmt = (lpmsgf->msg == WM_KEYDOWN) ? 
          "WM_KEYDOWN,  0x%02X  :  " : "WM_KEYUP,  0x%02X  :  ";
        log_str_len = wsprintfA(log_str, fmt, lpmsgf->wParam);
        fwrite(log_str, sizeof(char), log_str_len, fLog);
      }
      else if (lpmsgf->msg == WM_CHAR)
      {
        const char* fmt = "WM_CHAR,  %c  [0x%02X]  :  ";
        log_str_len = wsprintfA(log_str, fmt, lpmsgf->wParam, lpmsgf->wParam);
        fwrite(log_str, sizeof(char), log_str_len, fLog);
      }
    }
#endif

    // >>> VK_ESCAPE
    if ((lpmsgf->wParam == VK_ESCAPE) &&
        ((lpmsgf->msg == WM_KEYDOWN) || 
         (lpmsgf->msg == WM_KEYUP) || 
         (lpmsgf->msg == WM_CHAR)))
    {

#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "VK_ESCAPE";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif
      
      if (bFindControls)
      {
        enableFindControls(false);
        OnSize(hDlg);
      }
      lpmsgf->wParam = 0;
      return 0;
    }
    // <<< VK_ESCAPE

    /////////////////////////////////////////////////////////////////////////
    // original code by Nicolas Babled, modified by DV
        
    // >>> WM_LBUTTONUP
    if (lpmsgf->msg == WM_LBUTTONUP)
    {
        // >>> bDoubleClkEntered
        if (bDoubleClkEntered)
        {

#ifdef _consoledlg_keys_log_
            if (fLog)
            {
              const char* logstr = "VK_LBUTTONUP && DblClick\r\n";
              fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
            }
#endif            
         
            lpmsgf->msg = 0;
            bDoubleClkEntered = false;
            ::SetFocus( Runtime::GetNppExec().GetScintillaHandle() );
            return 0;
        }
        // <<< bDoubleClkEntered
        else
        {
            int posStart = 0;
            int posEnd = 0;
            Edit.ExGetSelPos(&posStart, &posEnd);
            GoToError_nCurrentLine = Edit.ExLineFromChar(posEnd);
        }
    }
    // <<< WM_LBUTTONUP

    // >>> WM_LBUTTONDBLCLK
    if (lpmsgf->msg == WM_LBUTTONDBLCLK) 
    {
            
#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "WM_LBUTTONDBLCLK\r\n";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif          
          
        int nLine = Edit.ExLineFromChar(Edit.ExGetSelPos());
        bDoubleClkEntered = GoToLineIfWarningAnalyzerMatch(Edit, nLine);
        if ( bDoubleClkEntered )
        {
            GoToError_nCurrentLine = nLine;
            lpmsgf->msg = 0;
        }
        
        return 0;
    }
    // <<< WM_LBUTTONDBLCLK
    
    // >>> WM_LBUTTONDOWN
    if (lpmsgf->msg == WM_LBUTTONDOWN)
    {
        bDoubleClkEntered = false;
        return 0;
    }
    // <<< WM_LBUTTONDOWN

    //
    /////////////////////////////////////////////////////////////////////////

    bool bCtrl  = ((GetKeyState(VK_CONTROL) & 0x80) != 0) ? true : false; // Ctrl
    bool bAlt   = ((GetKeyState(VK_MENU) & 0x80) != 0)    ? true : false; // Alt
    bool bShift = ((GetKeyState(VK_SHIFT) & 0x80) != 0)   ? true : false; // Shift

    // >>> bCtrl
    if (bCtrl) // Ctrl is pressed
    {
      
#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "Ctrl + ";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif          
        
        // >>> bCtrl : Ctrl+V || Ctrl+X
        if ((lpmsgf->wParam == 0x56) || // Ctrl+V
            (lpmsgf->wParam == 0x58))   // Ctrl+X
        {
        
#ifdef _consoledlg_keys_log_
            if (fLog)
            {
              const char* logstr = "V or X";
              fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
            }
#endif                  

            // >>> bCtrl : Ctrl+V || Ctrl+X : WM_KEYDOWN
            if (lpmsgf->msg == WM_KEYDOWN)
            {
                int nCharIndex = Edit.ExGetSelPos();
                if (nCharIndex < nConsoleFirstUnlockedPos)
                {
                    if (lpmsgf->wParam == 0x56)  //Ctrl+V
                    {
                        int len = Edit.GetTextLengthEx();
                        Edit.ExSetSel(len, len); // jump to the last line
                        return OnPaste(Edit, lpmsgf);
                    }
                    else if (lpmsgf->wParam == 0x58)  // Ctrl+X
                    {
                        lpmsgf->wParam = 0x43;  // Ctrl+C
                    }
                    else
                    {
                        lpmsgf->wParam = 0;
                    }
                }
                
                if (lpmsgf->wParam == 0x56) // Ctrl+V
                {
                    return OnPaste(Edit, lpmsgf);
                }
            }
            // <<< bCtrl : Ctrl+V || Ctrl+X : WM_KEYDOWN
        }
        // <<< bCtrl : Ctrl+V || Ctrl+X
        
        // NOTE:  Ctrl+X can be modified to Ctrl+C
        
        // >>> bCtrl : Ctrl+C
        if (lpmsgf->wParam == 0x43) // Ctrl+C
        {
      
#ifdef _consoledlg_keys_log_
            if (fLog)
            {
              const char* logstr = "C";
              fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
            }
#endif                  
        
            // >>> bCtrl : Ctrl+C : WM_KEYDOWN
            if (lpmsgf->msg == WM_KEYDOWN)
            {
                INT nSelStart = 0, nSelEnd = 0;

                Edit.ExGetSelPos(&nSelStart, &nSelEnd);
                if (nSelEnd > nSelStart) 
                {
                    TCHAR ch;
                    INT   nEnd = nSelEnd;

                    while ((nEnd > nSelStart) &&
                           (NppExecHelpers::IsAnySpaceChar(ch = Edit.GetCharAt(nEnd-1)) ||
                            (ch == _T('\r')) || (ch == _T('\n'))))
                    {
                        nEnd--;
                    }
                    if (nSelEnd != nEnd)
                    {
                        Edit.ExSetSel(nSelStart, nEnd);
                        //Runtime::GetNppExec().ShowWarning("Ctrl+C: modified");
                    }
                    return 0;
                }
                else
                {
                    if (Runtime::GetNppExec().GetCommandExecutor().IsChildProcessRunning())
                    {
                        Runtime::GetNppExec().GetCommandExecutor().ChildProcessMustBreak(CProcessKiller::killCtrlC);
                        //return 1;
                    }
                    else if (Runtime::GetNppExec()._consoleCommandIsRunning)
                    {
                        Runtime::GetNppExec()._consoleCommandBreak = true;
                    }
                    else
                    {
                        // std::shared_ptr<CScriptEngine> pScriptEngine = Runtime::GetNppExec().GetCommandExecutor().GetRunningScriptEngine();
                        // if (pScriptEngine)
                        // {
                        //     pScriptEngine->ScriptError(CScriptEngine::ET_ABORT, _T("; Aborted by user"));
                        //     Runtime::GetNppExec().GetConsole().PrintError(_T("- aborted by user"));
                        // }
                    }
                    return 0;
                }
            }
            // <<< bCtrl : Ctrl+C : WM_KEYDOWN
        }
        // <<< bCtrl : Ctrl+C

        // >>> bCtrl : Ctrl+Z
        else if (lpmsgf->wParam == 0x5A) // Ctrl+Z
        {

#ifdef _consoledlg_keys_log_
            if (fLog)
            {
              const char* logstr = "Z";
              fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
            }
#endif

            if (lpmsgf->msg == WM_KEYDOWN)
            {
                if (Runtime::GetNppExec().GetCommandExecutor().IsChildProcessRunning())
                {
                    Runtime::GetNppExec().GetConsole().PrintStr( _T("^Z") );
                    Runtime::GetNppExec().GetConsole().LockConsoleEndPos();
                    Runtime::GetNppExec().GetCommandExecutor().WriteChildProcessInput( _T("\x1A") ); // ^Z
                    Runtime::GetNppExec().GetCommandExecutor().WriteChildProcessInputNewLine();
                    return 0;
                }
            }
        }
        // <<< bCtrl : Ctrl+Z

        /*
        // I don't know the hex code(s) to be sent for Ctrl+D...
        // >>> bCtrl : Ctrl+D
        else if (lpmsgf->wParam == 0x44) // Ctrl+D
        {
            if (lpmsgf->msg == WM_KEYDOWN)
            {
                if (Runtime::GetNppExec().isChildProcessRunning())
                {
                    Runtime::GetNppExec().GetConsole().PrintStr( _T("^D") );
                    Runtime::GetNppExec().GetConsole().LockConsoleEndPos();
                    Runtime::GetNppExec().WriteChildProcessInput( _T("\x04") ); // ^D ???
                    Runtime::GetNppExec().WriteChildProcessInputNewLine();
                    return 0;
                }
            }
        }
        // <<< bCtrl : Ctrl+D
        */

        // >>> bCtrl : WM_KEYDOWN && Ctrl+Break
        else if ((lpmsgf->msg == WM_KEYDOWN) && (lpmsgf->wParam == 0x03)) // Ctrl+Break
        {
        
#ifdef _consoledlg_keys_log_
            if (fLog)
            {
              const char* logstr = "Break";
              fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
            }
#endif          
        
            if (Runtime::GetNppExec().GetCommandExecutor().IsChildProcessRunning())
            {
                Runtime::GetNppExec().GetCommandExecutor().ChildProcessMustBreak(CProcessKiller::killCtrlBreak);
                //return 1;
            }
            else if (Runtime::GetNppExec()._consoleCommandIsRunning)
            {
                Runtime::GetNppExec()._consoleCommandBreak = true;
            }
            else
            {
                std::shared_ptr<CScriptEngine> pScriptEngine = Runtime::GetNppExec().GetCommandExecutor().GetRunningScriptEngine();
                if (pScriptEngine)
                {
                    pScriptEngine->ScriptError(CScriptEngine::ET_ABORT, _T("; Aborted by user"));
                    Runtime::GetNppExec().GetConsole().PrintError(_T("- aborted by user"));
                }
            }
            return 0;
        }
        // <<< bCtrl : WM_KEYDOWN && Ctrl+Break

    }
    // <<< bCtrl

    // >>> bAlt
    else if (bAlt)
    {

#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "Alt";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif          

        SetFocus( GetDlgItem(hDlg, IDC_RE_CONSOLE) );
        if (lpmsgf->msg != WM_SYSKEYDOWN && lpmsgf->msg != WM_SYSKEYUP && lpmsgf->msg != WM_SYSCHAR)
            return 0;
    }
    // <<< bAlt

    // >>> bShift
    else if (bShift)
    {
        // due to "else if", we don't get here when Ctrl or Alt is also pressed

        // >>> bShift : Shift+Insert
        if (lpmsgf->wParam == VK_INSERT)
        {

#ifdef _consoledlg_keys_log_
            if (fLog)
            {
                const char* logstr = "Shift+Insert";
                fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
            }
#endif                  
            // >>> bShift : Shift+Insert : WM_KEYDOWN
            if (lpmsgf->msg == WM_KEYDOWN)
            {
                int nCharIndex = Edit.ExGetSelPos();
                if (nCharIndex < nConsoleFirstUnlockedPos)
                {
                    int len = Edit.GetTextLengthEx();
                    Edit.ExSetSel(len, len); // jump to the last line
                }
                return OnPaste(Edit, lpmsgf);
            }
            // <<< bShift : Shift+Insert : WM_KEYDOWN
        }
        // <<< bShift : Shift+Insert
    }
    // <<< bShift

    // >>> WM_KEYDOWN (Find Controls)
    if ( lpmsgf->msg == WM_KEYDOWN )
    {
        if ( ((lpmsgf->wParam == VK_F3) && (!bCtrl) && (!bAlt)) || // F3
             ((lpmsgf->wParam == VK_F7) && (!bCtrl) && (!bAlt)) || // F7
             ((lpmsgf->wParam == 0x46) && bCtrl && (!bAlt))  // Ctrl+F
           )
        {
            if ( bFindControls && (lpmsgf->wParam == VK_F3) )
            {
                ConsoleDlg::findEditWndProc(ConsoleDlg::edFind.m_hWnd, WM_KEYDOWN, VK_F3, 0);
            }
            else
            {
                bFindControls = true;
                OnSize(hDlg);
                enableFindControls(true);
            }
            return 0;
        }

        if ( (lpmsgf->wParam == 0x57) && bCtrl && (!bAlt) )  // Ctrl+W
        {
            doWordWrap(hDlg);
            return 0;
        }

        if ( (lpmsgf->wParam == 0x54) && bCtrl && (!bAlt) )  // Ctrl+T
        {
            doHideToggled(hDlg);
            return 0;
        }
    }
    // <<< WM_KEYDOWN (Find Controls)

    // >>> WM_KEYDOWN && hot-key
    if ( lpmsgf->msg == WM_KEYDOWN || lpmsgf->msg == WM_SYSKEYDOWN )
    {
        const unsigned int k = Runtime::GetNppExec().GetOptions().GetUint(OPTU_CONSOLE_CATCHSHORTCUTKEYS) & ConsoleDlg::CSK_ALL;
        if ( k != 0 )
        {
            int i = 0;
            int N = 0;
            if ( k == ConsoleDlg::CSK_STD )
            {
                // standard items: [0 .. (nbFunc-1)]
                i = 0;
                N = nbFunc;
            }
            else if ( k == ConsoleDlg::CSK_USR )
            {
                // user items: [nbFunc .. (nbFunc+g_nUserMenuItems)]
                i = nbFunc;
                N = (g_nUserMenuItems > 0) ? (nbFunc + g_nUserMenuItems + 1) : nbFunc;
            }
            else if ( k == ConsoleDlg::CSK_ALL )
            {
                // all items: [0 .. (nbFunc+g_nUserMenuItems)]
                i = 0;
                N = (g_nUserMenuItems > 0) ? (nbFunc + g_nUserMenuItems + 1) : nbFunc;
            }
            for ( ; i < N; i++ )
            {
                const ShortcutKey& shortcut = g_funcShortcut[i];
                if ( (shortcut._key == lpmsgf->wParam) &&
                     (shortcut._isAlt == bAlt) &&
                     (shortcut._isCtrl == bCtrl) &&
                     (shortcut._isShift == bShift) )
                {
                    #ifdef _consoledlg_keys_log_
                        if (fLog)
                        {
                          char logstr[100];
                          sprintf(logstr, "Hot-key: function[%d]", i);
                          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
                        }
                    #endif

                    if ( Runtime::GetLogger().IsOutputMode() )
                    {
                        Runtime::GetNppExec().GetConsole().ClearCurrentInput();
                    }
                    Runtime::GetLogger().AddEx( _T("; Hot-key: executing function [%d], \"%s\""), i, g_funcItem[i]._itemName );

                    lpmsgf->msg = 0;
                    lpmsgf->wParam = 0;
                    bFuncItemEntered = true;
                    ::PostMessage( Runtime::GetNppExec().GetConsole().GetDialogWnd(), WM_CONSOLEDLG_EXECFUNCITEM, i, 0 );
                    return 0;
                }
            }
        }
    }
    // <<< WM_KEYDOWN && hot-key

    if ( bFuncItemEntered )
    {
        if ( lpmsgf->msg == WM_CHAR || lpmsgf->msg == WM_SYSCHAR )
        {
            bFuncItemEntered = false;
            lpmsgf->msg = 0; // disables the "bell" sound when e.g. Alt+Enter is pressed
            lpmsgf->wParam = 0;
            return 0;
        }
        else if ( lpmsgf->msg == WM_KEYUP || lpmsgf->msg == WM_SYSKEYUP )
        {
            bFuncItemEntered = false;
            lpmsgf->msg = 0;
            lpmsgf->wParam = 0;
            return 0;
        }
    }

    // >>> (VK_UP || VK_DOWN) && (WM_KEYDOWN || WM_KEYUP)
    if (Runtime::GetNppExec().GetOptions().GetBool(OPTB_CONSOLE_CMDHISTORY) && 
        ((lpmsgf->wParam == VK_UP) || (lpmsgf->wParam == VK_DOWN)) &&
        ((lpmsgf->msg == WM_KEYDOWN) || (lpmsgf->msg == WM_KEYUP)) &&
        (!bCtrl) && (!bAlt)
       )
    {  

#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "Console Cmd History\r\n";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif          

        Edit.ScrollCaret();

        int nLine = -1;
        if (isCmdHistoryRequest(Edit, &nLine))
        {
            // >>> (VK_UP || VK_DOWN) && (WM_KEYDOWN || WM_KEYUP) : WM_KEYDOWN
            if (lpmsgf->msg == WM_KEYDOWN)
            {
                bool  bPrevItem = (lpmsgf->wParam == VK_UP);
                tstr  S = _T("");

                if (bPrevItem)
                {
                    if (pCmdHistoryItemPtr)
                    {
                        pCmdHistoryItemPtr = pCmdHistoryItemPtr->GetPrev();
                        /*
                        if (!pCmdHistoryItemPtr)
                        {
                            pCmdHistoryItemPtr = CmdHistoryList.GetLast();
                        }
                        */
                    }
                    else
                    {
                        pCmdHistoryItemPtr = CmdHistoryList.GetLast();
                    }
                }
                else
                {
                    if (pCmdHistoryItemPtr)
                    {
                        pCmdHistoryItemPtr = pCmdHistoryItemPtr->GetNext();
                        /*
                        if (!pCmdHistoryItemPtr)
                        {
                            pCmdHistoryItemPtr = CmdHistoryList.GetFirst();
                        }
                        */
                    }
                    else
                    {
                        pCmdHistoryItemPtr = CmdHistoryList.GetFirst();
                    }
                }

                int nFirst = Edit.LineIndex( nLine );
                if (nFirst < nConsoleFirstUnlockedPos)
                    nFirst = nConsoleFirstUnlockedPos;
                int nLength = Edit.LineLength( nFirst );
                if (pCmdHistoryItemPtr)
                {
                    S = pCmdHistoryItemPtr->GetItem();
                }
                Edit.SetRedraw(FALSE);
                Edit.ExSetSel( nFirst, nFirst+nLength );
                Edit.SetRedraw(TRUE);
                Edit.ReplaceSelText( _T("") );
                Edit.ReplaceSelText( S.c_str() );
            }
            // <<< (VK_UP || VK_DOWN) && (WM_KEYDOWN || WM_KEYUP) : WM_KEYDOWN

            lpmsgf->wParam = 0;
            //return 1;
        }
        return 0;
    }
    // <<< (VK_UP || VK_DOWN) && (WM_KEYDOWN || WM_KEYUP)

    // >>> VK_DELETE || VK_BACK
    if ((lpmsgf->wParam == VK_DELETE || lpmsgf->wParam == VK_BACK) &&
        (lpmsgf->msg == WM_KEYDOWN || lpmsgf->msg == WM_KEYUP || lpmsgf->msg == WM_CHAR))
    {

#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "VK_DELETE or VK_BACK";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif          

        int nCharIndex = Edit.ExGetSelPos();
        if (nCharIndex < nConsoleFirstUnlockedPos)
        {
            lpmsgf->wParam = 0;
            //return 1;
        }
        else if (lpmsgf->msg != WM_CHAR && lpmsgf->wParam == VK_BACK &&
                 nConsoleFirstUnlockedPos == nCharIndex)
        {
            lpmsgf->wParam = 0;
            //return 1;
        }
        return 0;
    }
    // <<< VK_DELETE || VK_BACK

    // >>> WM_CHAR (any character)
    if (lpmsgf->msg == WM_CHAR)
    {
      
#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "WM_CHAR or Ctrl+V";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif          
      
        if ((!bCtrl) || (lpmsgf->wParam == 0x56)) // not Ctrl, or Ctrl+V
        {
            int nCharIndex = Edit.ExGetSelPos();
            if (nCharIndex < nConsoleFirstUnlockedPos)
            {
                int len = Edit.GetTextLengthEx();
                Edit.ExSetSel(len, len); // jump to the last line
            }
        }

        if (bCtrl && (lpmsgf->wParam == 0x56)) // Ctrl+V
        {
            lpmsgf->wParam = 0;
            return 1;
        }

        // This is not needed since Shift+Insert does not generate WM_CHAR:
        //if (bShift && (lpmsgf->wParam == VK_INSERT)) // Shift+Insert
        //{
        //    lpmsgf->wParam = 0;
        //    return 1;
        //}

        // DO NOT exit here !!!
    }
    // <<< WM_CHAR (any character)

    // >>> VK_RETURN && WM_KEYDOWN
    if ((lpmsgf->wParam == VK_RETURN) && (lpmsgf->msg == WM_KEYDOWN))
    {
      
#ifdef _consoledlg_keys_log_
        if (fLog)
        {
          const char* logstr = "VK_RETURN";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
#endif

        if (bShift) // Shift+Enter
        {
            int nPos = 0, nEndPos = 0;
            Edit.ExGetSelPos(&nPos, &nEndPos);
            Edit.ReplaceSelText(Runtime::GetNppExec().GetCommandExecutor().GetChildProcessNewLine().c_str());
            nPos += _T_RE_EOL_LEN;
            Edit.ExSetSel(nPos, nPos);
            lpmsgf->wParam = 0;
            return 0;
        }

        int nCharIndex = Edit.ExGetSelPos();

        if (Runtime::GetNppExec().GetCommandExecutor().IsChildProcessRunning() &&
            (nCharIndex >= nConsoleFirstUnlockedPos))
        {
            // input line(s) as a console input
            bool isMultiline = false;
            int nLenPos = 0;
            tstr cmd = getInputText(Edit, &isMultiline, &nLenPos);

            if (cmd.length() > 0)
            {
                Edit.ExSetSel(nLenPos, nLenPos);

                if (!isMultiline)
                {
                    TCHAR ch;
                    while (((ch = cmd.GetLastChar()) == _T('\n')) || (ch == _T('\r')))
                    {
                        cmd.DeleteLastChar();
                    }
                    AddCommandToHistoryList(cmd);
                }

                Runtime::GetNppExec().GetCommandExecutor().ExecuteChildProcessCommand(cmd, true, true);
            }
            else
            {
                Runtime::GetNppExec().GetCommandExecutor().WriteChildProcessInputNewLine();
                Runtime::GetNppExec().GetConsole().LockConsoleEndPosAfterEnterPressed(true);
            }
        }
        else if (Runtime::GetNppExec().GetCommandExecutor().IsChildProcessRunning())
        {
            bCommandEntered = true;
            lpmsgf->wParam = 0;
        }
        else if (nCharIndex >= nConsoleFirstUnlockedPos)
        {
            // input line(s) as stand-alone command(s)
            bool isMultiline = false;
            int nLenPos = 0;
            tstr S = getInputText(Edit, &isMultiline, &nLenPos);

            if (S.length() > 0)
            {
                Edit.ExSetSel(nLenPos, nLenPos);

                if (isMultiline)
                {
                    Runtime::GetLogger().AddEx( _T_RE_EOL _T("; @Input Command: %s"), S.c_str() );
                    Runtime::GetLogger().Add( _T("; DoExecText") );

                    Runtime::GetNppExec().GetConsole().LockConsoleEndPosAfterEnterPressed(true);

                    unsigned int nMode = CNppExec::etfCollateralNoChildProc | CNppExec::etfNppExecPrefix | CNppExec::etfLastScript | \
                                         CNppExec::etfShareLocalVars | CNppExec::etfShareConsoleState;
                    Runtime::GetNppExec().DoExecText(S, nMode);

                    return 0;
                }

                tstr S1 = S;
                CScriptEngine::eNppExecCmdPrefix cmdPrefix = CScriptEngine::checkNppExecCmdPrefix(&Runtime::GetNppExec(), S1);
                bool isScriptRunningOrQueued = Runtime::GetNppExec().GetCommandExecutor().IsScriptRunningOrQueued();

                if (isScriptRunningOrQueued && (cmdPrefix == CScriptEngine::CmdPrefixNone))
                {
                    lpmsgf->wParam = 0;
                }
                else
                {
                    AddCommandToHistoryList(S); // with the prefix, if any

                    Runtime::GetLogger().AddEx( _T_RE_EOL _T("; @Input Command: %s"), S.c_str() );
                
                    S = S1; // without the prefix, if any
                    NppExecHelpers::StrUpper(S1);

                    if (S1 == CScriptEngine::DoClsCommand::Name())
                    {
                        Runtime::GetNppExec().GetConsole().ClearText(true);
                        bCommandEntered = true;
                        lpmsgf->wParam = 0;
                        return 0; // nothing else to do as ClearText() was forced
                    }

                    // Important:
                    // If any CNppExecConsole.m_reConsole.SendMsg() call occur under a
                    // locked CriticalSection, it can potentially lead to a deadlock
                    // because SendMsg() wants the main thread's message loop - while
                    // the main thread itself may be trying to obtain the very same
                    // CriticalSection at the same time, thus blocking each other.
                    // CNppExecConsole has been designed to avoid such deadlocks.
                    Runtime::GetNppExec().GetConsole().LockConsoleEndPosAfterEnterPressed(true);

                    if (IsConsoleVerCommand(S1))
                    {
                        bCommandEntered = true;
                        if ( Runtime::GetNppExec().GetConsole().IsOutputEnabled() )
                            lpmsgf->wParam = 0;
                    }
                    else if (IsConsoleHelpCommand(S1))
                    {
                        bCommandEntered = true;
                        if ( Runtime::GetNppExec().GetConsole().IsOutputEnabled() )
                            lpmsgf->wParam = 0;
                    }
                    else
                    {
                        // as we pressed Enter in the RichEdit's window, 
                        // the Console is definitely visible, isn't it?
                        Runtime::GetNppExec()._consoleIsVisible = true;

                        CListT<tstr> CmdList(S);
                        if (isScriptRunningOrQueued || (cmdPrefix == CScriptEngine::CmdPrefixCollateralForced))
                        {
                            unsigned int nRunFlags = CScriptEngine::rfCollateralScript;
                            if (cmdPrefix == CScriptEngine::CmdPrefixCollateralOrRegular)
                                nRunFlags |= (CScriptEngine::rfShareLocalVars | CScriptEngine::rfShareConsoleState);
                            Runtime::GetNppExec().GetCommandExecutor().ExecuteCollateralScript(CmdList, tstr(), nRunFlags);
                        }
                        else
                        {
                            Runtime::GetNppExec().SetCmdList(CmdList);
                            Runtime::GetNppExec().DoRunScript(CmdList, CScriptEngine::rfConsoleLocalVars | CScriptEngine::rfShareConsoleState);
                        }

                        //lpmsgf->wParam = 0;
                    }
                }
            }
            else
            {
                lpmsgf->wParam = 0;
            }

        }
        else
        {
            lpmsgf->wParam = 0;
        }
        //lpmsgf->wParam = 0;
        return 0;
    }
    // <<< VK_RETURN && WM_KEYDOWN

    else if (bCommandEntered && (lpmsgf->wParam == VK_RETURN) && (lpmsgf->msg == WM_CHAR))
    {
        lpmsgf->wParam = 0;
        return 0;
    }
    else if (bCommandEntered && (lpmsgf->wParam == VK_RETURN) && (lpmsgf->msg == WM_KEYUP))
    {
        lpmsgf->wParam = 0;
        bCommandEntered = false;
        return 0;
    }

    // >>> VK_HOME && WM_KEYDOWN
    if ((lpmsgf->wParam == VK_HOME) && (lpmsgf->msg == WM_KEYDOWN))
    {
        if (!bCtrl)
        {
            int nPos1 = 0, nPos2 = 0;
            Edit.ExGetSelPos(&nPos1, &nPos2);
            int nLine = Edit.ExLineFromChar(nPos2);
            if (nLine == Edit.ExLineFromChar(nConsoleFirstUnlockedPos))
            {
                if (!bShift)
                {
                    Edit.ExSetSel(nConsoleFirstUnlockedPos, nConsoleFirstUnlockedPos);
                }
                else
                {
                    if (nPos2 < nConsoleFirstUnlockedPos)
                    {
                        Edit.ExSetSel(Edit.LineIndex(nLine), nPos2);
                    }
                    else
                    {
                        Edit.ExSetSel(nConsoleFirstUnlockedPos, nPos2);
                    }
                }
                lpmsgf->wParam = 0;
                return 0;
            }
        }
        return 0;
    }
    // <<< VK_HOME && WM_KEYDOWN

    // >>> VK_END && (WM_KEYDOWN || WM_KEYUP)
    if ((lpmsgf->wParam == VK_END) && ((lpmsgf->msg == WM_KEYDOWN) || (lpmsgf->msg == WM_KEYUP)))
    {
        if (!bCtrl)
        {
            int nPos1 = 0, nPos2 = 0;
            Edit.ExGetSelPos(&nPos1, &nPos2);
            if (Edit.LineLength(nPos2) == 0)
            {
                lpmsgf->wParam = 0;
                return 0;
            }
        }
        return 0;
    }
    // <<< VK_END && (WM_KEYDOWN || WM_KEYUP)

    // >>> VK_TAB
    if (lpmsgf->wParam == VK_TAB)
    { 
        if (lpmsgf->msg == WM_KEYDOWN)
        {
            if (Runtime::GetNppExec().GetOptions().GetBool(OPTB_CONSOLE_TABASCHAR))
            {
                if (bCtrl || bAlt)
                    return 0;

                Edit.ReplaceSelText(_T("\t"), TRUE);
            }
            else
            {
                int nPos1 = 0, nPos2 = 0;
                Edit.ExGetSelPos(&nPos1, &nPos2);
                int nLine = Edit.ExLineFromChar(nPos2);
                if (nLine >= Edit.ExLineFromChar(nConsoleFirstUnlockedPos))
                {
                    TCHAR szLine[CONSOLECOMMAND_BUFSIZE];
                    TCHAR szLinePrefix[64];
                    int   pos = 0; // line start pos
                    int   nLen = GetCompleteLine(Edit, nLine, szLine, CONSOLECOMMAND_BUFSIZE - 1, &pos); // line length
                    if (nPos1 != nPos2)
                    {
                        if (nPos1 < pos)  nPos1 = pos;
                        if (nPos1 - pos < nLen)
                        {
                            nLen = nPos1 - pos;
                            szLine[nLen] = 0;
                        }
                    }
                    if ((nLen > 0) && (szLine[nLen-1] == _T('\n')))
                    {
                        szLine[--nLen] = 0;
                    }
                    if ((nLen > 0) && (szLine[nLen-1] == _T('\r')))
                    {
                        szLine[--nLen] = 0;
                    }
                    if (nPos1 == nPos2)
                    {
                        nPos1 = pos + nLen;
                        nPos2 = nPos1;
                        Edit.ExSetSel(nPos1, nPos1);
                    }
                    if (nConsoleFirstUnlockedPos > pos)
                    {
                        lstrcpy(szLine, szLine + nConsoleFirstUnlockedPos - pos);
                        nLen -= (nConsoleFirstUnlockedPos - pos);
                    }
                    if (nLen > 0)
                    {
                        static CListItemT<tstr>* p = NULL; 
                        static tstr prevLine = _T("");
                        static tstr prevFileLine = _T("");
                        static bool bEnvVar = false;
                        static bool bAllowCyclicSearch = false;
                        tstr S;
                        CListItemT<tstr>* p0 = p;
                        const bool bCmdHistory = 
                            ((CmdHistoryList.GetCount() > 0) && Runtime::GetNppExec().GetOptions().GetBool(OPTB_CONSOLE_CMDHISTORY)) ? true : false;

                        c_base::_tstr_unsafe_cpyn(szLinePrefix, szLine, 64 - 1); // keeps original case
                        NppExecHelpers::StrUpper(szLine); // upper case

                        const TCHAR* pszFileName = NULL;
                        bool         bFullPath = false;
                        int          len2 = 0;
                        CScriptEngine::eCmdType nCmdType = CScriptEngine::CMDTYPE_UNKNOWN;
                        
                        if ( /*(!Runtime::GetNppExec().isChildProcessRunning()) &&*/ 
                             ((!p) || (prevLine != szLine)) )
                        {
                            UINT uSearchFlags = 0;
                            tstr cmdLine = szLine;

                            /*
                            // disabled because '\' or '/' as alias to "NPP_EXEC"
                            // must be autocompleted in the same way as "NPP_EXEC"
                            switch ( cmdLine.GetAt(0) )
                            {
                                case _T('\\'):
                                case _T('/'):
                                    nCmdType = 0;
                                    break;
                                default:
                                    nCmdType = CScriptEngine::getCmdType(&Runtime::GetNppExec(), nullptr, cmdLine);
                                    break;
                            }
                            */
                            
                            // disable logging (no output while processing TAB pressed)
                            Runtime::GetLogger().Activate(false);
                            
                            nCmdType = CScriptEngine::getCmdType(&Runtime::GetNppExec(), nullptr, cmdLine, CScriptEngine::ctfIgnorePrefix);

                            switch ( nCmdType )
                            {
                                case CScriptEngine::CMDTYPE_COMMENT_OR_EMPTY:
                                    lpmsgf->wParam = 0;
                                    return 0;
                                case CScriptEngine::CMDTYPE_UNKNOWN:
                                    {
                                        uSearchFlags = CDirFileLister::ESF_DIRS | CDirFileLister::ESF_FILES | CDirFileLister::ESF_PLACEFILESFIRST | CDirFileLister::ESF_SORTED;

                                        CStrSplitT<TCHAR> args;
                                        int nArgs = args.SplitToArgs(cmdLine);
                                        if ( nArgs >= 2 )
                                        {
                                            if ( Runtime::GetNppExec().GetCommandExecutor().IsChildProcessRunning() )
                                            {
                                                // file name completion is allowed after any child process'es command
                                                int nLastArgPos = cmdLine.RFind(args.GetArg(nArgs-1));
                                                cmdLine.Delete(0, nLastArgPos);
                                            }
                                            else
                                            {
                                                // file name completion is allowed only after an executable
                                                const TCHAR* cszFileExtsToTry[] = {
                                                    _T(""), // as is - always try this first
                                                    _T(".exe"),
                                                    _T(".com"),
                                                    _T(".bat"),
                                                    _T(".cmd"),
                                                    NULL // end of the list, _must_ be the last item
                                                };
                                                tstr fileName;
                                                const TCHAR** ppszFileExt = &cszFileExtsToTry[0];
                                                while ( *ppszFileExt != NULL )
                                                {
                                                    fileName = args.GetArg(0);
                                                    fileName += *ppszFileExt;
                                                    if ( SearchPath(NULL, fileName.c_str(), NULL, 0, NULL, NULL) != 0 )
                                                    {
                                                        int nLastArgPos = cmdLine.RFind(args.GetArg(nArgs-1));
                                                        cmdLine.Delete(0, nLastArgPos);
                                                        *ppszFileExt = NULL; // break condition
                                                    }
                                                    else
                                                        ++ppszFileExt;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                case CScriptEngine::CMDTYPE_CD:
                                    uSearchFlags = CDirFileLister::ESF_DIRS | CDirFileLister::ESF_SORTED;
                                    break;
                                case CScriptEngine::CMDTYPE_DIR:
                                    uSearchFlags = CDirFileLister::ESF_DIRS | CDirFileLister::ESF_FILES | CDirFileLister::ESF_PLACEDIRSFIRST | CDirFileLister::ESF_SORTED;
                                    break;
                                default:
                                    uSearchFlags = CDirFileLister::ESF_DIRS | CDirFileLister::ESF_FILES | CDirFileLister::ESF_PLACEFILESFIRST | CDirFileLister::ESF_SORTED;
                                    break;
                            }

                            NppExecHelpers::StrDelLeadingAnySpaces(cmdLine);

                            bool bEndsWithIncompleteMacroVar = false;
                            const int nMacroVarStart = cmdLine.RFind(_T("$("));
                            if ( nMacroVarStart != -1 )
                            {
                                const int nMacroVarEnd = cmdLine.RFind(_T(")"));
                                if ( (nMacroVarEnd == -1) || (nMacroVarEnd < nMacroVarStart) )
                                    bEndsWithIncompleteMacroVar = true;
                            }

                            if ( !bEndsWithIncompleteMacroVar )
                                Runtime::GetNppExec().GetMacroVars().CheckAllMacroVars(nullptr, cmdLine, false);
                            
                            // enable logging (don't forget to enable after disabling!)
                            Runtime::GetLogger().Activate(true);
                            
                            if ( (cmdLine.length() > 0) && !bEndsWithIncompleteMacroVar )
                            {
                                if ( (cmdLine.length() == 2) && (cmdLine[1] == _T(':')) )
                                {
                                    cmdLine += _T('\\');
                                    Edit.ReplaceSelText( _T("\\") );
                                    ++nPos1;
                                    ++nPos2;
                                }
                            
                                bFullPath = NppExecHelpers::IsFullPath(cmdLine);
                                if ( !bFullPath )
                                {
                                    int   nPathLen;
                                    TCHAR szFullPath[FILEPATH_BUFSIZE];
                                    
                                    szFullPath[0] = 0;
                                    nPathLen = (int) ::GetCurrentDirectory(FILEPATH_BUFSIZE - 1, szFullPath);
                                    if ( nPathLen > 0 )
                                    {
                                        if ( ((cmdLine.GetAt(0) == _T('\\')) && (cmdLine.GetAt(1) != _T('\\'))) || 
                                             ((cmdLine.GetAt(0) == _T('/')) && (cmdLine.GetAt(1) != _T('/'))) )
                                        {
                                            szFullPath[2] = 0; // i.e. 'C:' remaining
                                            cmdLine.Insert(0, szFullPath);
                                        }
                                        else
                                        {
                                            szFullPath[nPathLen++] = _T('\\');
                                            szFullPath[nPathLen] = 0;
                                            cmdLine.Insert(0, szFullPath, nPathLen);
                                        }
                                    }
                                }

                                len2 = 0;
                                for ( int h = cmdLine.length() - 1; h >= 0; h-- )
                                {
                                    const TCHAR tch = cmdLine[h];
                                    if ( (tch != _T('\\')) && (tch != _T('/')) && (tch != _T(':')) )
                                        ++len2;
                                    else
                                        break;
                                }

                                if ( prevFileLine != szLine )
                                {
                                    prevFileLine = szLine;
                                    tabDirFileLister.Clear(); // reset the DirFileLister
                                }
                                
                                if ( !bShift )
                                    pszFileName = tabDirFileLister.FindNext(cmdLine.c_str(), uSearchFlags, bAllowCyclicSearch);
                                else
                                    pszFileName = tabDirFileLister.FindPrev(cmdLine.c_str(), uSearchFlags, bAllowCyclicSearch);
                                bAllowCyclicSearch = false;
                            }
                        }
                        
                        if ( pszFileName )
                        {
                            UINT itemLen;
                            bool itemIsDir;
                            
                            tabDirFileLister.GetItem(&itemLen, &itemIsDir);
                            
                            Edit.SetRedraw(FALSE);
                            Edit.ExSetSel(nPos1 - len2, nPos2);
                            Edit.SetRedraw(TRUE);
                            if ( itemIsDir /*&& (nCmdType != CScriptEngine::CMDTYPE_CD)*/ )
                            {
                                tstr itemStr = pszFileName;
                                itemStr += _T('\\');
                                ++itemLen;
                                Edit.ReplaceSelText(itemStr.c_str());
                            }
                            else
                            {
                                Edit.ReplaceSelText(pszFileName);
                            }
                            Edit.ExSetSel(nPos1, nPos1 - len2 + itemLen);
                        }
                        else
                        {
                            enum eIndx {
                                indNone           = 0,
                                indCmdHistoryList = 1, // CmdHistoryList
                                indCmdVarsList    = 2, // CmdVarsList
                                indPrepareForVar  = 3, // prepare to look for $(var)
                                indLookingForVar  = 4  // looking for $(var)
                            };

                            eIndx nListInd = indNone;

                            if ( (!p) || (prevLine != szLine) )
                            {
                                if ( !bShift )
                                {
                                    p = bCmdHistory ? CmdHistoryList.GetLast() : CmdVarsList.GetLast();
                                }
                                else
                                {
                                    p = NULL;
                                    nListInd = indPrepareForVar;
                                }
                                bEnvVar = false;
                            }
                            else
                            {
                                CListItemT<tstr>* p1;
                                
                                if ( !bShift )
                                {
                                    p1 = ( (bCmdHistory && (p->GetOwner() == &CmdHistoryList)) ? 
                                              CmdHistoryList.GetLast() : CmdVarsList.GetLast() );
                                }
                                else
                                {
                                    p1 = ( (bCmdHistory && (p->GetOwner() == &CmdHistoryList)) ? 
                                              CmdHistoryList.GetFirst() : CmdVarsList.GetFirst() );
                                }

                                while (p1 && (p1 != p))
                                {
                                    if ( !bShift )
                                        p1 = p1->GetPrev();
                                    else
                                        p1 = p1->GetNext();
                                }
                                if (p1)
                                {
                                    nListInd = (p1->GetOwner() == &CmdHistoryList) ? indCmdHistoryList : indCmdVarsList;
                                    if ( !bShift )
                                    {
                                        p = p1->GetPrev();
                                    }
                                    else
                                    {
                                        p = p1->GetNext();
                                    }
                                }
                                else
                                {
                                    if ( !bShift )
                                    {
                                        p = bCmdHistory ? CmdHistoryList.GetLast() : CmdVarsList.GetLast();
                                    }
                                    else
                                    {
                                        p = bCmdHistory ? CmdHistoryList.GetFirst() : CmdVarsList.GetFirst();
                                    }
                                    bEnvVar = false;
                                }
                            }

                            prevLine = szLine;
                            
                            if ( nListInd == indNone )
                            {
                                nListInd = ((p && (p->GetOwner() == &CmdVarsList)) || !bCmdHistory) ? indCmdVarsList : indCmdHistoryList;
                            }

                            while ( nListInd != indNone )
                            {
                                int n = 0;
                                if ( bEnvVar && (nListInd < indPrepareForVar) )
                                {
                                    p = NULL;
                                    nListInd = indPrepareForVar;
                                }
                                if ( p && (p->GetOwner() == &CmdVarsList) )
                                {
                                    if ( c_base::_tstr_unsafe_cmpn(szLine, _T("HELP"), 4) == 0 )
                                    {
                                        if ( NppExecHelpers::IsAnySpaceChar(szLine[4]) )
                                        {
                                            // Allow Tab-completion from CmdVarsList after "help ..."
                                            n = 5;
                                        }
                                    }
                                    else
                                    {
                                        int nPrefixLen = 0;
                                        const TCHAR* pszPrefix = Runtime::GetNppExec().GetOptions().GetStr(OPTS_NPPEXEC_CMD_PREFIX, &nPrefixLen);
                                        if ( nPrefixLen != 0 )
                                        {
                                            if ( c_base::_tstr_unsafe_cmpn(szLinePrefix, pszPrefix, nPrefixLen) == 0 )
                                            {
                                                // Allow Tab-completion from CmdVarsList after NppExec_Cmd_Prefix
                                                n = nPrefixLen;
                                                if ( szLinePrefix[n] == pszPrefix[nPrefixLen - 1] ) // is the last symbol doubled?
                                                    ++n;
                                            }
                                        }
                                    }
                                    if ( n != 0 )
                                    {
                                        // skip tabs and spaces
                                        while ( NppExecHelpers::IsAnySpaceChar(szLine[n]) )  ++n;

                                        // don't do anything special for "$(" here
                                        if ( szLine[n] == _T('$') )
                                        {
                                            if ( szLine[n+1] == _T('(') )
                                                n = 0;
                                        }

                                        if ( nLen <= n )
                                        {
                                            // nothing to do: no word to complete
                                            p = NULL;
                                        }
                                    }
                                }
                                while ( p )
                                {
                                    S = p->GetItem();
                                    NppExecHelpers::StrUpper(S);
                                    if ( S.StartsWith(szLine + n) )
                                    {
                                        if ( S.length() > nLen - n )
                                        {
                                            if ( nListInd /*== indLookingForVar*/ /*p->GetOwner() == &CmdVarsList*/ )
                                            {
                                                Edit.SetRedraw(FALSE);
                                                Edit.ExSetSel(nPos1 - nLen + n, nPos2);
                                                Edit.SetRedraw(TRUE);
                                                Edit.ReplaceSelText(p->GetItem().c_str());
                                                //Edit.Redraw();
                                            }
                                            else
                                            {
                                                Edit.ReplaceSelText(p->GetItem().c_str() + nLen);
                                            }
                                            Edit.ExSetSel(nPos1, nPos1 + S.length() - nLen + n);
                                        }
                                        bEnvVar = (nListInd == indLookingForVar) ? true : false;
                                        nListInd = indNone;
                                        break;
                                    }
                                    if ( !bShift )
                                        p = p->GetPrev();
                                    else
                                        p = p->GetNext();
                                }
                                if ( nListInd == indCmdHistoryList )
                                {
                                    if ( !bShift )
                                    {
                                        nListInd = indPrepareForVar;
                                        p = CmdVarsList.GetLast();
                                    }
                                    else
                                    {
                                        nListInd = indNone;
                                        //p = CmdVarsList.GetFirst();
                                    }
                                    bEnvVar = false;
                                }
                                else if ( nListInd == indCmdVarsList )
                                {
                                    if ( !bShift )
                                    {
                                        nListInd = indPrepareForVar;
                                        //p = bCmdHistory ? CmdHistoryList.GetLast() : NULL;
                                    }
                                    else
                                    {
                                        nListInd = indCmdHistoryList;
                                        p = bCmdHistory ? CmdHistoryList.GetFirst() : NULL;
                                    }
                                    bEnvVar = false;
                                }
                                else if ( nListInd == indPrepareForVar )
                                {
                                    nListInd = indLookingForVar;
                                    int i = nLen - 1;
                                    for ( ; i >= 0; i--)
                                    {
                                        if ( (szLine[i] == _T('(')) && (i > 0) && (szLine[i-1] == _T('$')) )
                                        {
                                            // "$(" found
                                            --i;
                                            lstrcpy(szLine, szLine + i);
                                            nLen -= i;
                                            break;
                                        }
                                    }
                                    if ( (nLen > 0) && (i < 0) )  // no "$(" found
                                    {
                                        if ( szLine[0] == _T('\"') )  // skip first '"'
                                        {
                                            lstrcpy(szLine, szLine + 1);
                                            nLen -= 1;
                                        }
                                    }
                                    if ( (nLen > 0) && (szLine[0] == _T('$')) )
                                    {
                                        if ( p0 && (p0->GetOwner() == &CmdVarsList) )
                                        {
                                            if ( !bShift )
                                                p = p0->GetPrev();
                                            else
                                                p = p0->GetNext();
                                        }
                                        else
                                        {
                                            if ( !bShift )
                                                p = CmdVarsList.GetLast();
                                            else
                                                p = CmdVarsList.GetFirst();
                                        }
                                    }
                                    else    
                                    {
                                        if ( !bShift )
                                        {
                                            nListInd = indNone;
                                            //p = NULL;
                                        }
                                        else
                                        {
                                            nListInd = indCmdVarsList;
                                            p = CmdVarsList.GetFirst();
                                            lstrcpy( szLine, prevLine.c_str() );
                                            nLen = prevLine.length();
                                        }
                                        bEnvVar = false;
                                    }
                                }
                                else
                                {
                                    if (nListInd == indLookingForVar)  bEnvVar = false;
                                    if ( !bShift )
                                    {
                                        nListInd = indNone;
                                    }
                                    else if ( nListInd != indNone )
                                    {
                                        if ( bCmdHistory )
                                        {
                                            nListInd = indCmdHistoryList;
                                            p = CmdHistoryList.GetFirst();
                                        }
                                        else
                                        {    
                                            nListInd = indCmdVarsList;
                                            p = CmdVarsList.GetFirst();
                                        }
                                        lstrcpy( szLine, prevLine.c_str() );
                                        nLen = prevLine.length();
                                    }
                                }
                            }
                            if ( !p )
                            {
                                Edit.ReplaceSelText( _T("") );
                                Edit.ExSetSel(nPos1, nPos1);
                                bAllowCyclicSearch = true;
                            }
                        }
                    }
                }
            }
        }
        lpmsgf->wParam = 0;
        return 0;
    }
    // <<< VK_TAB

#ifdef _consoledlg_keys_log_
      if (fLog)
      {
        if ((lpmsgf->msg == WM_KEYDOWN) ||
            (lpmsgf->msg == WM_KEYUP) ||
            (lpmsgf->msg == WM_CHAR))
        {       
          const char* logstr = "\r\n";
          fwrite(logstr, sizeof(char), lstrlenA(logstr), fLog);
        }
      }
#endif          

  }
  // <<< EN_MSGFILTER

  else if (pnmh->hwndFrom == Runtime::GetNppExec().m_nppData._nppHandle)
  {
      if (LOWORD(pnmh->code) == DMN_CLOSE)
      {
          // closing dlg
          //char str[256]; wsprintf(str, "0x%X", pnmh->idFrom); MessageBox(0,str,"",0);
          ConsoleDlg::OnClose(hDlg, 0);
          return 0;
      }
      else if (LOWORD(pnmh->code) == DMN_FLOAT)
      {
          // floating dlg
          bIsDocked = false;
      }
      else if (LOWORD(pnmh->code) == DMN_DOCK)
      {
          // docking dlg
          bIsDocked = true;
      }
  }
  return 0;
}

INT_PTR ConsoleDlg::OnCtlColorEdit(WPARAM wParam, LPARAM lParam)
{
    if ( edFind.m_hWnd == (HWND) lParam )
    {
        COLORREF crTextColor;
        COLORREF crBkgndColor;
        HBRUSH   hBkgndBrush = NULL;

        if ( Runtime::GetNppExec().GetOptions().GetBool(OPTB_CONSOLE_USEEDITORCOLORS) )
        {
            hBkgndBrush = Runtime::GetNppExec().GetConsole().GetCurrentBkgndBrush();
        }

        if ( hBkgndBrush != NULL )
        {
            crTextColor = Runtime::GetNppExec().GetConsole().GetCurrentColorTextNorm();
            crBkgndColor = Runtime::GetNppExec().GetConsole().GetCurrentColorBkgnd();
        }
        else
        {
            crTextColor = GetSysColor(COLOR_WINDOWTEXT);
            crBkgndColor = GetSysColor(COLOR_WINDOW);
            hBkgndBrush = GetSysColorBrush(COLOR_WINDOW);
        }

        SetTextColor( (HDC) wParam, crTextColor );
        SetBkMode( (HDC) wParam, TRANSPARENT );
        SetBkColor( (HDC) wParam, crBkgndColor );

        return (INT_PTR) hBkgndBrush;
    }

    return 0;
}

void ConsoleDlg::OnShowWindow(HWND hDlg)
{
  HWND hEd = GetDlgItem(hDlg, IDC_RE_CONSOLE);
  ::SetFocus(hEd);
}

void ConsoleDlg::OnSize(HWND hDlg)
{
  RECT  rect;
  HWND  hEd;

  //::GetWindowRect(hDlg, &rect);
  ::GetClientRect(hDlg, &rect);
  /*
  rect.left += 1;
  rect.top += 1;
  rect.right -= 2;
  rect.bottom -= 2;
  */
  if (bFindControls)
  {
    rect.bottom -= 24;
  }
  hEd = ::GetDlgItem(hDlg, IDC_RE_CONSOLE);
  ::MoveWindow(hEd, rect.left, rect.top, rect.right, rect.bottom, TRUE);
  ::InvalidateRect(hEd, NULL, TRUE);
  ::UpdateWindow(hEd);
  if (bFindControls)
  {
    int left = 2;
    int top = rect.bottom + 2;
    
    btFindClose.MoveWindow(left, top, TRUE);
    ::GetWindowRect( btFindClose.m_hWnd, &rect );
    left += (6 + rect.right - rect.left);
    edFind.MoveWindow(left, top, TRUE);
    ::GetWindowRect( edFind.m_hWnd, &rect );
    left += (5 + rect.right - rect.left);
    chFindCase.MoveWindow(left, top + 1, TRUE);
    ::GetWindowRect( chFindCase.m_hWnd, &rect );
    left += (4 + rect.right - rect.left);
    chWordWrap.MoveWindow(left, top + 1, TRUE);
    ::GetWindowRect( chWordWrap.m_hWnd, &rect );
    left += (4 + rect.right - rect.left);
    chHideToggled.MoveWindow(left, top + 1, TRUE);
    ::GetWindowRect( chHideToggled.m_hWnd, &rect );
    left += (4 + rect.right - rect.left);
    chTabAsChar.MoveWindow(left, top + 1, TRUE);
  }
}

void ConsoleDlg::printConsoleReady()
{
  CNppExec& NppExec = Runtime::GetNppExec();
  if (NppExec.GetOptions().GetBool(OPTB_CONSOLE_PRINTMSGREADY) &&
      !NppExec.GetCommandExecutor().IsChildProcessRunning())
  {
    tstr sMsgReady = NppExec.GetOptions().GetStr(OPTS_CONSOLE_CUSTOMMSGREADY);
    if (CNppExecMacroVars::ContainsMacroVar(sMsgReady))
    {
      NppExec.GetMacroVars().CheckAllMacroVars(nullptr, sMsgReady, true);
    }
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg;
    NppExec.GetConsole().PrintMessage( sMsgReady.c_str(), nMsgFlags );
  }
}

bool ConsoleDlg::IsConsoleHelpCommand(const tstr& S, bool bCalledFromScriptEngine )
{
  CNppExec& NppExec = Runtime::GetNppExec();

  if (S.StartsWith(CONSOLE_CMD_HELP))
  {
    if (S == CONSOLE_CMD_HELP)
    {
      const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
      NppExec.GetConsole().PrintMessage(CONSOLE_COMMANDS_INFO, nMsgFlags);
      if (!bCalledFromScriptEngine) printConsoleReady();
      return true;
    }
    else
    {
      int i = S.FindOneOf(_T(" \t\v\f"));
      if (i > 0)
      {
        tstr S1;

        ++i;
        while (NppExecHelpers::IsAnySpaceChar(S[i])) ++i;
        S1.Assign( S.c_str() + i, S.length() - i );
        i = 0;
        while ((i < S1.length()) && !NppExecHelpers::IsAnySpaceChar(S1[i])) ++i;
        if (i < S1.length())
        {
          S1.SetSize(i);
        }
        if (S1.length() > 0)
        {
          NppExecHelpers::StrUpper(S1);
          if (S1 == _T("ALL"))
          {
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            NppExec.GetConsole().PrintMessage( _T(""), nMsgFlags );
            NppExec.GetConsole().PrintMessage(CONSOLE_COMMANDS_INFO, nMsgFlags);

            for (const tCmdItemInfo& ci : CONSOLE_CMD_INFO)
            {
              NppExec.GetConsole().PrintMessage( _T("------------------------------------------------------------------------------"), nMsgFlags );
              NppExec.GetConsole().PrintMessage( ci.info, nMsgFlags );
            }
            if (!bCalledFromScriptEngine) printConsoleReady();
            return true;
          }
          else
          {
            if (S1 == CScriptEngine::DoConSaveToCommand::AltName())
              S1 = CScriptEngine::DoConSaveToCommand::Name();
            else if (S1 == CScriptEngine::DoConLoadFromCommand::AltName())
              S1 = CScriptEngine::DoConLoadFromCommand::Name();
            else if (S1 == CScriptEngine::DoUnsetCommand::Name())
              S1 = CScriptEngine::DoSetCommand::Name();
            else if (S1 == CScriptEngine::DoEnvUnsetCommand::Name() || 
                     S1 == CScriptEngine::DoEnvSetCommand::AltName() || 
                     S1 == CScriptEngine::DoEnvUnsetCommand::AltName())
              S1 = CScriptEngine::DoEnvSetCommand::Name();
            else if (S1 == CScriptEngine::DoNpeDebugLogCommand::AltName())
              S1 = CScriptEngine::DoNpeDebugLogCommand::Name();
            else if (S1 == CScriptEngine::DoSelLoadFromCommand::AltName())
              S1 = CScriptEngine::DoSelLoadFromCommand::Name();
            else if (S1 == CScriptEngine::DoSelSaveToCommand::AltName())
              S1 = CScriptEngine::DoSelSaveToCommand::Name();
            else if (S1 == CScriptEngine::DoSelSetTextExCommand::Name())
              S1 = CScriptEngine::DoSelSetTextCommand::Name();
            else if (S1 == CScriptEngine::DoTextLoadFromCommand::AltName())
              S1 = CScriptEngine::DoTextLoadFromCommand::Name();
            else if (S1 == CScriptEngine::DoTextSaveToCommand::AltName())
              S1 = CScriptEngine::DoTextSaveToCommand::Name();
            else if (S1 == CScriptEngine::DoConColourCommand::AltName())
              S1 = CScriptEngine::DoConColourCommand::Name();
            else if (S1 == _T("STRLENUTF8") || S1 == _T("STRLENU")     || 
                     S1 == _T("STRLENSCI")  || S1 == _T("STRLENS")     || 
                     S1 == _T("STRLENA")    || S1 == _T("STRLEN")      || 
                     S1 == _T("STRUPPER")   || S1 == _T("STRLOWER")    || 
                     S1 == _T("SUBSTR")     ||
                     S1 == _T("STRFIND")    || S1 == _T("STRRFIND")    ||
                     S1 == _T("STRREPLACE") || S1 == _T("STRRPLC")     ||
                     S1 == _T("STRQUOTE")   || S1 == _T("STRUNQUOTE")  ||
                     S1 == _T("STRESCAPE")  || S1 == _T("STRUNESCAPE") ||
                     S1 == _T("STREXPAND")  ||
                     S1 == _T("STRFROMHEX") || S1 == _T("STRTOHEX")    ||
                     S1 == _T("CHR")        ||
                     S1 == _T("ORD")        || S1 == _T("ORDX") ||
                     S1 == _T("NORMPATH")   ||
                     S1 == _T("FILEEXISTS") || S1 == _T("DIREXISTS"))
              S1 = CScriptEngine::DoSetCommand::Name();
            else if (S1.GetAt(0) == DEFAULT_ALIAS_CMD_LABEL && S1.GetAt(1) == 0)
              S1 = CScriptEngine::DoLabelCommand::Name();
            else if (S1 == _T("EXIT_CMD")     || S1 == _T("EXIT_CMD_SILENT")   ||
                     S1 == _T("@EXIT_CMD")    || S1 == _T("@EXIT_CMD_SILENT")  ||
                     S1 == _T("$(@EXIT_CMD)") || S1 == _T("$(@EXIT_CMD_SILENT)"))
              S1 = _T("@EXIT_CMD");
            for (const tCmdItemInfo& ci : CONSOLE_CMD_INFO)
            {
              if (S1 == ci.name)
              {
                const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                NppExec.GetConsole().PrintMessage( _T(""), nMsgFlags );
                NppExec.GetConsole().PrintMessage( ci.info, nMsgFlags );
                if (!bCalledFromScriptEngine) printConsoleReady();
                return true;
              }
            }
          }
        }

      }
    }
  }
  else if (S == CONSOLE_CMD_MANUAL || S == CONSOLE_CMD_ABOUT)
  {
    if (S == CONSOLE_CMD_MANUAL)
      NppExec.OnHelpManual();
    else
      NppExec.OnHelpAbout();
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
    NppExec.GetConsole().PrintMessage( _T(""), nMsgFlags );
    printConsoleReady();
    return true;
  }
  return false;
}

bool ConsoleDlg::IsConsoleVerCommand(const tstr& S)
{
    if (S == CONSOLE_CMD_VER)
    {
        CNppExec& NppExec = Runtime::GetNppExec();
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        NppExec.GetConsole().PrintMessage( _T(""), nMsgFlags );
        NppExec.GetConsole().PrintMessage( PLUGIN_CURRENT_VER, nMsgFlags );
        printConsoleReady();
        return true;
    }
    return false;
}

void ConsoleDlg::loadCmdVarsList()
{
  // environment variables in reverse order
  CmdVarsList.Add( MACRO_WORKSPACE_ITEM_ROOT );    //  $(WORKSPACE_ITEM_ROOT)
  CmdVarsList.Add( MACRO_WORKSPACE_ITEM_PATH );    //  $(WORKSPACE_ITEM_PATH)
  CmdVarsList.Add( MACRO_WORKSPACE_ITEM_NAME );    //  $(WORKSPACE_ITEM_NAME)
  CmdVarsList.Add( MACRO_WORKSPACE_ITEM_DIR );     //  $(WORKSPACE_ITEM_DIR)
  CmdVarsList.Add( _T("$(SYS.PATH)") );            //  $(SYS.PATH)
  CmdVarsList.Add( MACRO_SELECTED_TEXT );          //  $(SELECTED_TEXT)
  CmdVarsList.Add( MACRO_SCI_HWND2 );              //  $(SCI_HWND2)
  CmdVarsList.Add( MACRO_SCI_HWND1 );              //  $(SCI_HWND1)
  CmdVarsList.Add( MACRO_SCI_HWND );               //  $(SCI_HWND)
  CmdVarsList.Add( MACRO_RIGHT_VIEW_FILE );        //  $(RIGHT_VIEW_FILE)
  CmdVarsList.Add( _T("$(RARGV[1])") );            //  $(RARGV[1])
  CmdVarsList.Add( _T("$(RARGV)") );               //  $(RARGV)
  CmdVarsList.Add( MACRO_PLUGINS_CONFIG_DIR );     //  $(PLUGINS_CONFIG_DIR)
  CmdVarsList.Add( MACRO_PID );                    //  $(PID)
  CmdVarsList.Add( MACRO_OUTPUTL );                //  $(OUTPUTL)
  CmdVarsList.Add( MACRO_OUTPUT1 );                //  $(OUTPUT1)
  CmdVarsList.Add( MACRO_OUTPUT );                 //  $(OUTPUT)
  CmdVarsList.Add( MACRO_NPP_PID );                //  $(NPP_PID)
  CmdVarsList.Add( MACRO_NPP_HWND );               //  $(NPP_HWND)
  CmdVarsList.Add( MACRO_NPP_FULL_FILE_PATH );     //  $(NPP_FULL_FILE_PATH)
  CmdVarsList.Add( MACRO_NPP_DIRECTORY );          //  $(NPP_DIRECTORY)
  CmdVarsList.Add( MACRO_FILE_NAMEONLY );          //  $(NAME_PART)
  CmdVarsList.Add( MACRO_MSG_WPARAM );             //  $(MSG_WPARAM)
  CmdVarsList.Add( MACRO_MSG_RESULT );             //  $(MSG_RESULT)
  CmdVarsList.Add( MACRO_MSG_LPARAM );             //  $(MSG_LPARAM)
  CmdVarsList.Add( MACRO_LEFT_VIEW_FILE );         //  $(LEFT_VIEW_FILE)
  CmdVarsList.Add( MACRO_LAST_CMD_RESULT );        //  $(LAST_CMD_RESULT)
  CmdVarsList.Add( MACRO_IS_PROCESS );             //  $(IS_PROCESS)
  CmdVarsList.Add( _T("$(INPUT[1])") );            //  $(INPUT[1])
  CmdVarsList.Add( MACRO_INPUT );                  //  $(INPUT)
  CmdVarsList.Add( MACRO_FILE_FULLPATH );          //  $(FULL_CURRENT_PATH)
  CmdVarsList.Add( MACRO_FOCUSED_HWND );           //  $(FOCUSED_HWND)
  CmdVarsList.Add( MACRO_FILE_NAME_AT_CURSOR );    //  $(FILE_NAME_AT_CURSOR)
  CmdVarsList.Add( MACRO_FILE_FULLNAME );          //  $(FILE_NAME)
  CmdVarsList.Add( MACRO_FILE_EXTONLY );           //  $(EXT_PART)
  CmdVarsList.Add( MACRO_EXITCODE );               //  $(EXITCODE)
  CmdVarsList.Add( MACRO_CURRENT_WORKING_DIR );    //  $(CWD)
  CmdVarsList.Add( MACRO_CURRENT_WORD );           //  $(CURRENT_WORD)
  CmdVarsList.Add( MACRO_CURRENT_LINESTR );        //  $(CURRENT_LINESTR)
  CmdVarsList.Add( MACRO_CURRENT_LINE );           //  $(CURRENT_LINE)
  CmdVarsList.Add( MACRO_FILE_DIRPATH );           //  $(CURRENT_DIRECTORY)
  CmdVarsList.Add( MACRO_CURRENT_COLUMN );         //  $(CURRENT_COLUMN)
  CmdVarsList.Add( MACRO_CON_HWND );               //  $(CON_HWND)
  CmdVarsList.Add( MACRO_CLOUD_LOCATION_PATH );    //  $(CLOUD_LOCATION_PATH)
  CmdVarsList.Add( MACRO_CLIPBOARD_TEXT );         //  $(CLIPBOARD_TEXT)
  CmdVarsList.Add( _T("$(ARGV[1])") );             //  $(ARGV[1])
  CmdVarsList.Add( _T("$(ARGV)") );                //  $(ARGV)
  CmdVarsList.Add( MACRO_ARGC );                   //  $(ARGC)
  CmdVarsList.Add( MACRO_EXIT_CMD_SILENT );        //  $(@EXIT_CMD_SILENT)
  CmdVarsList.Add( MACRO_EXIT_CMD );               //  $(@EXIT_CMD)
  CmdVarsList.Add( _T("$(#1)") );                  //  $(#1)
  
  // commands in reverse order
  const auto& CommandsList = CScriptEngine::GetCommandRegistry().GetSortedCmdNames();
  for ( auto itr = CommandsList.rbegin(); itr != CommandsList.rend(); ++itr )
  {
    const tstr& S = *itr;
    if ( S.length() < 3 )
      continue;
    
    if ( S == CScriptEngine::DoGoToCommand::Name() ||
         S == CScriptEngine::DoElseCommand::Name() ||
         S == CScriptEngine::DoEndIfCommand::Name() ||
         S == CScriptEngine::DoLabelCommand::Name() )
      continue;

    CmdVarsList.Add( S );
  }
}

bool ConsoleDlg::loadCmdHistory()
{
  CNppExec& NppExec = Runtime::GetNppExec();
  if ( !NppExec.GetOptions().GetBool(OPTB_CONSOLE_SAVECMDHISTORY) )
    return false;

  CFileBufT<TCHAR> fbuf;
  tstr             S;

  tstr path = NppExec.ExpandToFullConfigPath(CMDHISTORY_FILENAME, true);
  if ( fbuf.LoadFromFile(path.c_str(), true, NppExec.GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH)) )
  {
    while ( fbuf.GetLine(S) >= 0 )
    {
      if ( !S.IsEmpty() )
        CmdHistoryList.Add(S);
    }

    return true;
  }

  return false;
}

bool ConsoleDlg::saveCmdHistory()
{
  CNppExec& NppExec = Runtime::GetNppExec();
  if ( !NppExec.GetOptions().GetBool(OPTB_CONSOLE_SAVECMDHISTORY) )
    return false;

  bool bSaved = false;
  tstr localCmdHistory = NppExec.ExpandToFullConfigPath(CMDHISTORY_FILENAME);

  if ( bCmdHistoryUpdated )
  {
    CFileBufT<TCHAR> fbuf;
    tstr             S;

    fbuf.GetBufPtr()->Reserve( CmdHistoryList.GetCount() * 64 );
    CListItemT<tstr> * p = CmdHistoryList.GetFirst();
    while ( p )
    {
      S = p->GetItem();
      S += _T("\r\n");
      fbuf.GetBufPtr()->Append( S.c_str(), S.length() );
      p = p->GetNext();
    }

    if ( fbuf.SaveToFile(localCmdHistory.c_str()) )
    {
      bCmdHistoryUpdated = false;
      bSaved = true;
    }
  }

  // Cloud settings...
  const tstr cloudPluginDir = NppExec.GetSettingsCloudPluginDir();
  if (!cloudPluginDir.IsEmpty())
  {
    CNppExec::CreateCloudDirIfNeeded(cloudPluginDir);

    // Back up the cmd history to the cloud
    tstr cloudCmdHistory = cloudPluginDir;
    cloudCmdHistory += CMDHISTORY_FILENAME;
    if (bSaved || !NppExecHelpers::IsValidTextFile(cloudCmdHistory))
    {
      ::CopyFile(localCmdHistory.c_str(), cloudCmdHistory.c_str(), FALSE);
    }
  }

  return bSaved;
}

bool ConsoleDlg::SaveCmdHistory()
{
  return saveCmdHistory();
}

bool ConsoleDlg::isMultilineInput(CAnyRichEdit& Edit, int* pnTotalLines, int* pnFirstInputLine)
{
  bool bRet = false;
  int nFirstInputLine = 0;
  int nTotalLines = Edit.GetLineCount();
  if (nTotalLines > 1)
  {
    nFirstInputLine = Edit.ExLineFromChar(nConsoleFirstUnlockedPos);
    if (nFirstInputLine != nTotalLines - 1)
      bRet = true;
  }

  if (pnTotalLines) *pnTotalLines = nTotalLines;
  if (pnFirstInputLine) *pnFirstInputLine = nFirstInputLine;
  return bRet;
}

bool ConsoleDlg::isCmdHistoryRequest(CAnyRichEdit& Edit, int* pnCurrLine)
{
  int nTotalLines = 0, nFirstInputLine = 0;
  if (isMultilineInput(Edit, &nTotalLines, &nFirstInputLine))
    return false;

  int nPos = Edit.ExGetSelPos();
  int nLine = Edit.ExLineFromChar(nPos);
  *pnCurrLine = nLine;
  return ((nLine >= nFirstInputLine) && (nLine < nTotalLines));
}

tstr ConsoleDlg::getInputText(CAnyRichEdit& Edit, bool* pisMultiline, int* pnTotalTextLen)
{
    tstr S;
    int nTotalLines = 0;
    *pisMultiline = isMultilineInput(Edit, &nTotalLines);
    if (nTotalLines != 0)
    {
        *pnTotalTextLen = Edit.GetTextLengthEx();
        int nInputLength = *pnTotalTextLen; 
        nInputLength -= nConsoleFirstUnlockedPos;
        if (nInputLength > 0)
        {
            S.Reserve(nInputLength);

            nInputLength = Edit.GetTextAt(nConsoleFirstUnlockedPos, nInputLength, S.data());
            S.SetLengthValue(nInputLength);
            S.Replace(_T_RE_EOL, Runtime::GetNppExec().GetCommandExecutor().GetChildProcessNewLine().c_str());
        }
    }

    return S;
}

void ConsoleDlg::enableFindControls(bool bEnable)
{
  INT nShow = bEnable ? SW_SHOWNORMAL : SW_HIDE;
  bFindControls = bEnable;
  btFindClose.ShowWindow(nShow);
  chFindCase.ShowWindow(nShow);
  edFind.ShowWindow(nShow);
  chWordWrap.ShowWindow(nShow);
  chHideToggled.ShowWindow(nShow);
  chTabAsChar.ShowWindow(nShow);
  if (bEnable) 
  {
    edFind.SetFocus();
  }
  else
  {
    Runtime::GetNppExec().GetConsole().GetConsoleEdit().SetFocus();
  }
}

void ConsoleDlg::doWordWrap(HWND hDlg)
{
    HWND hEd = ::GetDlgItem(hDlg, IDC_RE_CONSOLE);
    bWordWrap = !bWordWrap;
    SendMessage(hEd, EM_SETTARGETDEVICE, 0, bWordWrap ? 0 : 1);
    // - it does work, but why on earth EM_SETTARGETDEVICE is responsible for
    // word-wrapping and this ability is not mentioned in the documentation?
    chWordWrap.SetCheck(bWordWrap ? TRUE : FALSE);
    Runtime::GetNppExec().GetOptions().SetBool(OPTB_CONSOLE_WORDWRAP, bWordWrap);
}

void ConsoleDlg::doHideToggled(HWND /*hDlg*/)
{
    bHideToggled = !bHideToggled;

    chHideToggled.SetCheck(bHideToggled ? TRUE : FALSE);
    Runtime::GetNppExec().GetOptions().SetBool(OPTB_CONSOLE_HIDETOGGLED, bHideToggled);
}

void ConsoleDlg::doTabAsChar(HWND /*hDlg*/)
{
    bTabAsChar = !bTabAsChar;

    chTabAsChar.SetCheck(bTabAsChar ? TRUE : FALSE);
    Runtime::GetNppExec().GetOptions().SetBool(OPTB_CONSOLE_TABASCHAR, bTabAsChar);
}

INT ConsoleDlg::GetCompleteLine(const CAnyRichEdit& Edit, INT nLine, TCHAR* lpTextBuf, WORD wTextBufSize, INT* pnLineStartIndex)
{
    if ( bWordWrap )
    {
        // It looks like RichEdit with version >= 2.0 returns a string with
        // '\r' at the end when it is a complete line (or the trailing part
        // of a word-wrapped line); and it returns a string without '\r' at
        // the end when it is an incomplete part of a word-wrapped line.
        // Though this is not mentioned in the official documentation, so
        // I'm not sure whether we can rely on this with any combination of
        // RichEdit version and Windows version.
        auto ends_with_RE_EOL = [](const TCHAR* pszLine, INT nLen) -> bool
        {
            const TCHAR ch = pszLine[nLen - 1];
            return (ch == '\r' || ch == '\n');
        };

        CListT<tstr> LineParts;
        INT nLen;
        INT nLineStartIndex = -1;

        // Check the previous (word-wrapped) lines first...
        for ( INT n = nLine - 1; n >= 0; --n )
        {
            nLen = Edit.GetLine(n, lpTextBuf, wTextBufSize);
            if ( nLen > 0 && !ends_with_RE_EOL(lpTextBuf, nLen) )
            {
                nLineStartIndex = Edit.LineIndex(n);
                LineParts.InsertFirst( tstr(lpTextBuf, nLen) ); // leading part(s) of the line
            }
            else
                break;
        }

        if ( nLineStartIndex == -1 )
        {
            nLineStartIndex = Edit.LineIndex(nLine);
        }

        // Current (word-wrapped) line...
        nLen = Edit.GetLine(nLine, lpTextBuf, wTextBufSize);
        if ( nLen > 0 )
        {
            LineParts.Add( tstr(lpTextBuf, nLen) );
            if ( !ends_with_RE_EOL(lpTextBuf, nLen) )
            {
                // Check the next (word-wrapped) lines...
                const int nLines = Edit.GetLineCount();
                for ( INT n = nLine + 1; n < nLines; ++n )
                {
                    nLen = Edit.GetLine(n, lpTextBuf, wTextBufSize);
                    if ( nLen > 0 )
                    {
                        LineParts.Add( tstr(lpTextBuf, nLen) ); // trailing part(s) of the line
                        if ( ends_with_RE_EOL(lpTextBuf, nLen) )
                            break;
                    }
                }
            }
        }

        // Construct the complete line...
        nLen = 0;
        for ( CListItemT<tstr>* pItem = LineParts.GetFirst(); pItem != NULL; pItem = pItem->GetNext() )
        {
            const tstr& S = pItem->GetItem();
            int nLenToCopy = static_cast<int>(wTextBufSize) - nLen - 1; // max len available
            if ( nLenToCopy > S.length() )
                nLenToCopy = S.length();
            lstrcpyn(lpTextBuf + nLen, S.c_str(), nLenToCopy + 1); // not obvious: lstrcpyn additionally counts the trailing '\0' as part of its Length parameter
            nLen += nLenToCopy;
            if ( nLenToCopy != S.length() )
                break; // no more room, the string is too long
        }
        
        *pnLineStartIndex = nLineStartIndex;
        lpTextBuf[nLen] = 0;
        return nLen;
    }

    *pnLineStartIndex = Edit.LineIndex(nLine);
    return Edit.GetLine(nLine, lpTextBuf, wTextBufSize);
}

LRESULT CALLBACK ConsoleDlg::findEditWndProc(
  HWND   hEd, 
  UINT   uMsg, 
  WPARAM wParam, 
  LPARAM lParam)
{
  if (uMsg == WM_KEYDOWN)
  {
    if (wParam == VK_RETURN || wParam == VK_F3)
    {
      TCHAR szText[500];
      int len = edFind.GetText(szText, 500-1);
      if (len > 0)
      {
        szText[len] = 0;

        bool bShift = ((GetKeyState(VK_SHIFT) & 0x80) != 0) ? true : false; // Shift is pressed
        WPARAM wFlags = bShift ? 0 : FR_DOWN;
        if (chFindCase.IsChecked())  wFlags |= FR_MATCHCASE;

        CNppExecConsole& Console = Runtime::GetNppExec().GetConsole();

        int      pos = 0;
        FINDTEXT ft;
        Console.GetConsoleEdit().ExGetSelPos(NULL, &pos);
        if ((wFlags & FR_DOWN) == FR_DOWN)
        {
          ft.chrg.cpMin = pos;
          ft.chrg.cpMax = -1;
        }
        else
        {
          ft.chrg.cpMin = pos - 1;
          ft.chrg.cpMax = 0;
        }
        ft.lpstrText = szText;
        
        pos = (int) ::SendMessage(Console.GetConsoleWnd(), 
                        EM_FINDTEXT, wFlags, (LPARAM) &ft);
        if (pos == -1)
        {
          if ((wFlags & FR_DOWN) == FR_DOWN)
          {
            ft.chrg.cpMin = 0;
            ft.chrg.cpMax = -1;
          }
          else
          {
            ft.chrg.cpMin = Console.GetConsoleEdit().GetTextLengthEx();
            ft.chrg.cpMax = 0;
          }
          ft.lpstrText = szText;
          pos = (int) ::SendMessage(Console.GetConsoleWnd(), 
                          EM_FINDTEXT, wFlags, (LPARAM) &ft);
        }
        if (pos >= 0)
        {
          Console.GetConsoleEdit().ExSetSel(pos, pos + len);
        }
      }
      return 1;
    }
    else if (wParam == VK_ESCAPE)
    {
      if (bFindControls)
      {
        enableFindControls(false);
        OnSize( ::GetParent(hEd) );
      }
      return 1;
    }
  }

  return CallWindowProc(findEditWndProc0, hEd, uMsg, wParam, lParam);
}

#ifdef _consoledlg_re_subclass_
  LRESULT CALLBACK ConsoleDlg::RichEditWndProc(
    HWND   hEd, 
    UINT   uMessage, 
    WPARAM wParam, 
    LPARAM lParam)
  {
    //static bool bTrackingMouse = false;

    /**/
    if (uMessage == WM_SETFOCUS)
    {
      ConsoleDlg::EnableTransparency(false);
      //return 0;
    }
    /**/
    else if (uMessage == WM_KILLFOCUS)
    {
      if ( wParam && (((HWND) wParam) == Runtime::GetNppExec().GetScintillaHandle()) )
      {
        if ( (GetKeyState(VK_LBUTTON) & 0x80) == 0 ) // left mouse button not pressed
          ConsoleDlg::EnableTransparency(true);
        //return 0;
      }
    }
    /*
    else if (uMessage == WM_MOUSEMOVE)
    {
      if ( (!bTrackingMouse) && (hEd != ::GetFocus()) )
      {
        TRACKMOUSEEVENT tme;

        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = hEd;
        tme.dwHoverTime = HOVER_DEFAULT;
        if ( _TrackMouseEvent(&tme) )
        {
          bTrackingMouse = true;
        }
        ConsoleDlg::EnableTransparency(false);
      }
    }    
    else if (uMessage == WM_MOUSELEAVE)
    {
      HWND hFocused = ::GetFocus();
      
      bTrackingMouse = false;
      if ( (hFocused != hEd) &&
           (::GetParent(hFocused) != Runtime::GetNppExec().GetConsole().GetParentWnd()) )
      {     
        ConsoleDlg::EnableTransparency(true);
      }  
    }
    */

    return CallWindowProc(OriginalRichEditProc, hEd, uMessage, wParam, lParam);
  }
#endif
