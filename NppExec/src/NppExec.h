/*
This file is part of NppExec
Copyright (C) 2020 DV <dvv81 (at) ukr (dot) net>

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
 * NppExec History:
 ****************************************************************************

 v0.8.1
 ------
 * 24.02.2022: Russkii voennyi korabl', idi nakhui!!!
 + added: $(CURRENT_LINESTR), $(SCI_HWND1), $(SCI_HWND2)
 + added: dynamic reloading of the saved scripts ("npes_saved.txt")
 * the "About" dialog slightly updated


 v0.8 - February 2022
 --------------------
 + added: built-in highlight filter that catches most of compiler error
   messages, thanks to David Maisonave.
   This filter is disabled by default to avoid an impact on performance.
   It is recommended to enable this filter locally, right before running
   a compiler or an interpreter: "npe_console local -- x+".
   See also: "NppExec_TechInfo.txt", the 'CompilerErrors' setting.
 + added: now WarningAnalyzer caches the previously matched lines.
   It allows the built-in highlight filter (see above) to react to a
   double-click in the Console even when this filter is disabled at the
   moment of double-clicking. Explanation: let's consider a situation when
   the built-in highlight filter had been disabled globally but was locally
   enabled via "npe_console local -- x+" right before running a compiler. So
   the messages produced by compiler are analyzed by the built-in highlight
   filter and the filter is automatically disabled after the compiler exits.
   Now, as WarningAnalyzer has cached the matched lines from the compiler's
   output, it is possible to double-click these lines in NppExec's Console
   to get the cached match result.
 + added: the last executed script is now saved to "npes_last.txt".
 + added: new menu items "Execute Selected Text", "Execute Clipboard Text".
 + NPE_CONSOLE c<N> and s<N> to change the text processing for the
   Execute Clipboard Text and Execute Selected Text.
 + NPE_CONSOLE j+/j- to kill process tree on/off.
 + added: new command "proc_input".
 + added: new command "npp_exectext".
 + added: new variables $(SELECTED_TEXT), $(IS_PROCESS).
 * changed: the menu item "Disable command aliases" has been removed. Use
   the "npe_console q+/q-" instead.
 * changed: now "help" command works in NppExec's scripts.
 * npp_files updated to Notepad++ 8.3.
 * note: 32-bit NppExec is compatible with Notepad++ 7.9.2 under Windows XP.
 - fixed: now "set local" (without an argument) prints only local vars.
 - fixed: "npe_console k3" did not work with Alt+key.
 + NppExec Manual updated


 v0.7.1 - August 2021
 --------------------
 * changed: now IF/IF~/ELSE IF use delayed $(var) substitution.
   It means that IF "$(var)" != "" will work even when the value of $(var)
   contains inner " quote character(s).
 + added: now npe_debuglog supports the keyword "local".
 - fixed: when there was a "local" command followed by a similar "non-local"
   command, the "non-local" one behaved as if it was "local".
 * changed: $(var) substitution has been reworked and improved.
 + added: set <var> ~ strexpand <s>


 v0.7 - July 2021
 ----------------
 + added: now NppExec supports the "Dark Mode" of Notepad++ v8
   (Thanks to Peter Jones for the updated icons!)
   NppExec is still compatible with previous versions of Notepad++.
 * changed: now NppExec supports quoted strings in the form of "abc", 'abc'
   and `abc`.
   This allows to pass quote characters within a quoted string: `"`, '"',
   '"abc" `def`' and so on.
   Now, if you want to pass a text that includes ' or ` character, you need
   to enquote this text in a different pair of quotes. For example:
   `'t was brillig...`, "Can't stop", "Press `Esc`", etc.
 * changed: now NppExec's Console and the Toolbar button explicitly mention
   "NppExec" in their names
 + added: new menu item "Change Execute Script Font..."
 + added: now MESSAGEBOX and INPUTBOX can accept 4th parameter 'time_ms'
 * changed: now the InputBox can be closed by pressing Esc. When it happens,
   the value of $(INPUT) will be empty.
 + added: now con_colour, con_filter, env_set, npe_console, npe_noemptyvars,
   npe_sendmsgbuflen and npp_console support a new keyword "local".
   This new keyword means: the changes are applied locally to the current
   NppExec's script and are reverted back when the current script ends.
   Thus, the "local" keyword does not make sense in a single command executed
   directly in NppExec's Console because the previous state is restored right
   after the execution of this single "local" command, so you will not see any
   effect of it.
 + added: if~ <condition> - first calculates, then checks the condition
 + added: set <var> ~ strescape <s>, set <var> ~ strunescape <s>
 + added: file names "npes_temp.txt" and "npes_saved.txt" can be customized
   (see "NppExec_TechInfo.txt" for details)
 - fixed: indirect variable reference in e.g. "echo #$(i) = $(#$(i))"
 + new advanced option "CustomMsgReady" (see "NppExec_TechInfo.txt")
 + NppExec Manual updated


 v0.6.2 - February 2021
 ----------------------
 * changed: now NppExec uses CreateFile+FILE_FLAG_WRITE_THROUGH while writing
   files to avoid filling with zero bytes on system shutdown.
 * changed: now NppExec changes the current directory to %TEMP% when an unnamed
   file (such as "new 1") is activated and "Follow $(CURRENT_DIRECTORY)" is on.
   To revert to the old behavior (the current directory is not changed when an
   unnamed file is activated), set the manual option "Cd_UnnamedFile" to 0.
 + added: now NppExec supports "cloud location path" in Notepad++'s settings.
     With cloud location path specified in Notepad++'s settings, NppExec does
     the following:
     * On start, NppExec tries to read its configuration files from the cloud
       location path. If these files do not exist or are filled with NULs,
       NppExec reads its configuration from $(PLUGINS_CONFIG_DIR).
     * When NppExec saves its configuration files, first they are saved to
       $(PLUGINS_CONFIG_DIR) and then copied to the cloud location path. Thus,
       NppExec always has copies of its current configuration files within the
       $(PLUGINS_CONFIG_DIR) folder.
     * NppExec's saved scripts - the "npes_saved.txt" file - are monitored in
       the cloud location path. So, if you manually edit the "npes_saved.txt"
       within the cloud location, NppExec detects it. If, however, you manually
       edit the "npes_saved.txt" within the $(PLUGINS_CONFIG_DIR) folder, it is
       ignored.
     When the cloud location path is _not_ specified in Notepad++'s settings,
     NppExec reads and stores its configuration within $(PLUGINS_CONFIG_DIR).
     And the "npes_saved.txt" is monitored in that folder.
 + added: $(NPP_FULL_FILE_PATH), $(CLOUD_LOCATION_PATH)
 + added: indirect variable reference, e.g. $($(name)). Examples:
     set local c = 123        // $(c) = 123
     set local b = c          // $(b) = c
     set local a = $($(b))    // $(a) = $($(b)) = $(c) = 123
     set local $($(b)) = 456  // $(c) = 456
     unset local $($(b))      // deletes $(c)
     set local i = #          // $(i) = #
     set local j = 1          // $(j) = 1
     echo $($(i)$(j))         // echo $(#1)
 * changed: now the variables $(ARGC), $(ARGV), $(ARGV[1]) and so on support
   the indirect variable reference (see above)
 + added: Ctrl+Break in the Console aborts the currently running script


 v0.6.1 - November 2020
 ----------------------
 * improved: now ChildProcess_RunPolicy=1 works better
 * improved: now GoToLineIfWarningAnalyzerMatch calls SCI_ENSUREVISIBLE
 * npp_files updated to Notepad++ 7.9.1


 v0.6 RC4 - September 2020
 -------------------------
 + added: $(FILE_NAME_AT_CURSOR), $(CON_HWND), $(FOCUSED_HWND)
 + added: $(WORKSPACE_ITEM_*)
 + added: the message "==== READY ====" is optional now (npe_console p-)
 + added: ANSI escape sequences can be ignored (npe_console e1)
 + added: set <var> ~ strquote <s>, set <var> ~ strunquote <s>
 + added: set <var> ~ normpath <path>
 + added: set <var> ~ chr <char code>
 + added: set <var> ~ ord <char>, set <var> ~ ordx <char>
 * now $(LAST_CMD_RESULT) will be 0 in case of StrCalc error (set x ~ ...)
 * now WarningAnalyzer tries to find a file in the current view first
 * now GOTO X equals to GOTO :X, as well as LABEL X equals to LABEL :X
 - fixed: variable names can contain brackets - e.g. $(a(b(c)))
 - fixed: end of an embedded npp_exec-ed script may also be the end of its parent script
 * $(var) completion in the "Execute" and "Console" dialogs improved
 + now FParserWrapper::readConstsFromFile supports "enum { ... }" definitions
 * internal improvements
 + NppExec Manual updated


 v0.6 RC3 - April 2019
 ---------------------
 + new advanced option "ChildProcess_RunPolicy" (see "NppExec_TechInfo.txt")
 * better compatibility with Notepad++ 7.6.x (and higher)
 * internal improvements
 + the NppExec Manual has been updated


 v0.6 RC2 - June 2018
 --------------------
 + new environment variables: $(CLIPBOARD_TEXT), $(NPP_HWND), $(SCI_HWND)
 + new commands: clip_settext, messagebox, exit, npe_sendmsgbuflen
 + set <var> ~ strlensci <string>
 * internal improvements
 + the NppExec Manual has been updated


 v0.6 RC1 - May 2018
 -------------------
 ! NppExec's help files were moved from "doc\*.*" to "doc\NppExec\*.*"
 + new command: npp_setfocus
 + Plugin Interface: new messages NPEM_GETSCRIPTNAMES, NPEM_GETSCRIPTBYNAME
 * now the actual name of NppExec.dll affects the "NppExec" submenu and its "Show Console" item name
 * various small improvements


 v0.6 beta 1 - February 2018
 ---------------------------
 * changed: now the Execute dialog's auto-completion pop-up uses the same upper/lower case
   as the first character of a word being typed (e.g. Npp -> NPP...; npp -> npp...);
   also: now the auto-completion pop-up's height is limited by the number of its items
 + new commands: SCI_FIND, SCI_REPLACE
 + new commands: TEXT_SAVETO, TEXT_LOADFROM
 + Function Parser has been updated to support the bitwise operators: ~, |, &, << and >>
   (thanks to Juha Nieminen and Joel Yliluoma for their advice)
 + $(LAST_CMD_RESULT) - result of the last NppExec's command (1 - succeeded, 0 - failed, -1 - invalid arg)
 - fixed: parameters of sci_sendmsg/npp_sendmsg were truncated to 32-bit in 64-bit NppExec
 - fixed: now pasting to the Console via Shift+Insert is equal to Ctrl+V


 v0.6 alpha 1 - December 2017
 ----------------------------
 1. Project infrastructure updated:
    ! Now NppExec uses C++11 and requires at least VS 2013 Express
    ! 32-bit NppExec targets Windows XP SP3 and higher (because of new VS C++ compiler)
    + Project file for Visual Studio 2013 Express (NppExec_VC12.sln)
    + Project file for Visual Studio 2015 Express (NppExec_VC14.sln)
    + NppExec can be build with Orwell Dev-C++ (NppExec_DevCpp.dev)
    + 64-bit ready (the 64-bit NppExec targets Windows 7 x64 and higher)
 2. Third-party components updated: 
    + fparser updated to 4.5.2
    + Notepad++ and Scintilla header files updated to Notepad++ 7.5.1
 3. Scripting abilities enhanced:
    + added: "nppexec:" prefix to execute NppExec's commands from a child console process
    + added: "nppexec::" prefix to execute NppExec's command always in a collateral script
    + added: "@EXIT_CMD" and "@EXIT_CMD_SILENT" macro vars to specify child process'es exit command
      (type "help @exit_cmd" in the Console for details)
    + added: $(PID) macro var, e.g. to be able to "nppexec:taskkill /pid $(PID)"
    + new commands: NPP_SAVEAS, PROC_SIGNAL, SLEEP, NPE_QUEUE
    + set <var> ~ strfromhex <hex string>
    + set <var> ~ strtohex <string>
    + now IF...ELSE IF...ELSE...ENDIF sequences are supported (see "help if")
    + now commands can be executed from macro-vars (e.g. "set a = npp_run calc" and then "$(a)" to execute)
    + now a child console process is tried to be killed first, and only then terminated
    + now "set local" in NppExec's Console creates Console's local vars (not shared with NppExec's script)
    + NppExec's Plugin Interface has been enhanced; NppExecPluginMsgTester has been updated
 4. Improved and fixed:
    + Warning Analyser: more accurate detection of %LINE% and %CHAR%
    + NppExec's Console: Tab-completion improved; also works after "help " and "nppexec:"
    + NppExec's Console: "Tab = \t" allows to type Tab character in the Console
    + now SEL_LOADFROM and CON_LOADFROM detect UTF-8 without BOM
    + now SEL_SAVETO can save as UTF-8 without BOM
    - fixed: font of NppExec's Console could be set incorrectly
    - fixed: crash while calculating very big numbers (e.g. 10^72)
    - fixed: the "exit command" dialog did not use the "Key_Enter" value
    - fixed: now pasting (Ctrl+V) to the Console clears the text formatting
    - fixed: now the Console Filters masks' length is not limited by the edit controls' width
    - fixed: double-click on word-wrapped errors and warnings did not work in the Console
    * plugin's core has been rewritten (the layer of the CNppExecCommandExecutor)
    * other internal improvements
    * even more other internal improvements
    * better error reporting
    * output is suppressed while Notepad++ is exiting (applies to the exit script)
 5. Highlights:
    * The "nppexec:" prefix allows to execute NppExec's commands while working
      with a child console process (e.g. you can start cmd in NppExec's Console
      and type "nppexec: set local @exit_cmd = exit" in cmd's prompt or even
      type "nppexec: taskkill /pid $(PID)" to kill the cmd.exe). The command
      that is executed in such way is an example of so-called collateral
      NppExec's script. (It's collateral because it is executed while a child
      console process is already executing.)
    * The "nppexec::" prefix always starts a collateral (parallel) NppExec's
      script to execute the given command. Even if there is no child process or
      script running in NppExec, this prefix forces a collateral script to be
      created to execute the command.
    * All these variables are local now (similar to "set local"):
      - $(INPUT), $(INPUT[N])
      - $(OUTPUT), $(OUTPUT1), $(OUTPUTL)
      - $(EXITCODE), $(PID)
      - $(MSG_RESULT), $(MSG_WPARAM), $(MSG_LPARAM)
      Now, with collateral scripts, it makes perfect sense to make these
      variables to be local - as they are rather unique for each NppExec's
      script than to NppExec as a whole. (Any NppExec's script can call
      "inputbox", start a child process, send a message - and there is no
      sense to share the resulting variables with all the other scripts.)
    * NPE_CMDALIAS and NPE_QUEUE have additional parameters to enable/disable
      the delayed variables substitution. NPE_QUEUE additionally has a parameter
      that controls whether the queued command will be executed as a separate
      NppExec's script (similar to NPEM_EXECUTE_QUEUED) or not.
      Type "help npe_cmdalias" and "help npe_queue" in NppExec's Console for
      details.
    * NppExec's Plugin Interface has been enhanced; the NppExecPluginMsgTester
      has been updated correspondingly.
    * Now there are 32-bit and 64-bit versions of NppExec.
    * More than 10 years of NppExec! :)


 v0.5.3 - February 2015
 ----------------------
 + new menu items "Go to next error" and "Go to previous error" (based on
   the HighLight masks), thanks to mrmixer
 + now %FILE% is treated as both %ABSFILE% and %FILE%, so there's no need
   to duplicate the masks with %ABSFILE% and %FILE%
 + maximum number of user menu items increased to 100 (was 40)
 + when npes_saved.txt is saved, its previous version is kept as .bak
 + TAB navigation works in Filter/Replace/HighLight dialog
 + now the INPUTBOX command can accept 3 parameters
 + NppExec Manual updated
 + other small improvements


 v0.5.2.2 - December 2014
 ------------------------
 + now the IF-GOTO statement supports floating-point comparison as well
   (e.g. IF 2.5 < 10.1 GOTO Label2)
 + new advanced option "Console_NulChar" (enhancing v0.5.2.1)
 * 8 years since NppExec was born! :)


 v0.5.2.1 - November 2014
 ------------------------
 + Now NppExec is able to deal with NUL ('\0') character when external 
   application outputs it to the Console:
   (Unicode) 0x0000 "NUL" is replaced with 0x25E6 "White Bullet";
   (ANSI) 0x00 "NUL" is replaced with 0x17 "End of Text Block".
 + NppExec Manual updated a little bit.
 * The "Donate..." button has been removed. Since I did not receive any 
   donation - either due to ukrainian politics/restrictions as for PayPal
   or due to lack of donations itself - and since this situation is unlikely
   to change, the button has been removed.
   You can always donate to Notepad++ itself.


 v0.5.2 - January 2014
 ---------------------
 + set local <var> = <value>
 + NppExec's options are now automatically saved on NPPN_SHUTDOWN
 + Ctrl+Z sends ^Z (end-of-file) to current child process
 + new advanced option "Key_Enter", by default its value is "\n"
 + new advanced option "LogsDir" to create NppExec's log files
 + NPE_CONSOLE k0..3 to enable/disable NppExec's shortcut keys in the Console
 + NppExec Manual updated
 + different internal improvements & refactoring
 + fparser updated to v4.5.1
 * 7th anniversary of NppExec :)


 v0.5 - April 2013
 -----------------
 1. Scripting abilities enhanced:
    + new commands: IF, GOTO
    + set <var> ~ strupper/strlower/substr <string>
    + set <var> ~ strfind/strrfind <string>
    + set <var> ~ strreplace <string>
 2. Highlighting abilities enhanced:
    + now all the Console Filters support run-time $(vars)
    + new commands: CON_COLOUR, CON_FILTER
 3. New/enhanced commands:
    + new command: NPP_MENUCOMMAND
    + now sel_settext+ supports '\r'
 4. Fixed:
    - fixed: could not get/modify Notepad++'s main menu when it was hidden
 5. Other:
    + new Console's commands: MANUAL, ABOUT
    * different internal improvements


 v0.4.3.2 - November 2012
 ------------------------
 - fixed: partial file name matching in npp_exec, npp_close, npp_save and
   npp_switch (CNppExec::nppConvertToFullPathName, nppSwitchToDocument)
 + HELP ALL to show all available help (all the commands in detail)
 + @[] in CScriptEngine::doSendMsg


 v0.4.3.1 - September 2012
 -------------------------
 - fixed: "Follow $(CURRENT_DIRECTORY)" did not work on file save
 + NPE_CONSOLE f+/f- r+/r- to enable/disable the console output filters


 v0.4.3 - May 2012
 -----------------
 + now NppExec's Console keeps its shown/hidden state after Notepad++ restarts
   (* thanks to ufo-pu55y for help)
 + NPP_CONSOLE +/- to enable/disable output to the Console
 + some refactoring of NppExec's Console functions


 v0.4.2.1 - March 2012
 ---------------------
 - fixed: incorrect check of Notepad++ version


 v0.4.2 - November 2011
 ----------------------
 - fixed: $("var") was processed incorrectly
 + now npe_cmdalias uses run-time $(vars)
 + set <var> ~ strlen <string>
 + set <var> ~ strlenutf8 <string>
 + $(EXITCODE)
 + npp_sendmsgex, [] in CScriptEngine::doSendMsg
 + %CHAR% in WarningAnalyzer, thanks to Carlos (cabanela)
 + fparser updated to v4.4.3
 + NppExec Manual updated (one step closer to enlightenment :))


 v0.4.1 - July 2010
 ------------------
 - fix for NPP_SENDMSG/SCI_SENDMSG with parameters containing dbl-quote(s)
 + fparser updated to v4.2
 + NppExec Manual updated


 v0.4 - April 2010
 -----------------
 + new commands: NPP_SENDMSG, SCI_SENDMSG
 + SET command extension: set <var> ~ <math expression>
   (using Function Parser by Juha Nieminen, Joel Yliluoma)
 + several small improvements


 v0.3.3.1 - February 2010
 ------------------------
 - fixed: "WS_VSCROLL" was omitted for combo-boxes in IDD_ADVANCEDOPTIONS

 
 v0.3.3 - January 2010
 ---------------------
 - fix1: now "\r\n" (Enter key emulation) is sent to the pipe separately
 - fix2: n++'es unsubclassing on exit removed (might be dangerous)
 - fix3: now the Console should keep its font always (IMF_AUTOFONT turned off)
 + Help/Manual updated (sections 4.6.3 and 4.6.4 added)


 v0.3.2 - September 2009
 -----------------------
 + new feature: TAB autocompletes file paths (in the Console)
   (* thanks to ILme for initial implementation)
 + new feature: Shift+Tab (in the Console, the same as TAB but backwards)
 - fixed: 'cd \path', 'dir \path' did not jump to the root of current drive
 - fixed: on-exit script was not executed
 + new command: NPE_CMDALIAS
 + new menu item: Disable command aliases
 + new option in NPE_CONSOLE: q+/q-


 v0.3.1 - June 2009
 ------------------
 + new command: SEL_LOADFROM
 + new command: SEL_SETTEXT, SEL_SETTEXT+
 + new variable: $(OUTPUT), $(OUTPUTL), $(OUTPUT1), see NPE_CONSOLE V+
 + several small improvements


 v0.3 final - April 2009
 -----------------------
 * thanks to Greg Webb for the .chm help file (manual)
 + several small fixes
 + new command: NPE_CONSOLE


 v0.3 RC1 - January 2009
 -----------------------
 * thanks to John LaCasse for useful suggestions
 + new environment variable: $(CWD)
 + new commands: ENV_SET/ENV_UNSET
 + new menu item: Toggle Console Dialog (Ctrl+~)
 + now NPE_DEBUGLOG reports on Console double-clicked line's Highlight Filter
 + NppExecPluginMsgTester has been re-designed and updated
 + both Unicode and ANSI builds are supported
 + now the "Execute..." dialog is resizable
 + now NppExec supports remapped shortcut keys (requires Notepad++ >= 5.1.4)
 + other improvements
 + now NppExec has the manual (yes!)

 
 v0.3 beta1 - November 2008
 --------------------------
 + full Unicode compatibility (for Unicode version of Notepad++)
 + additional functionality
 + improvements in the script engine (architecture re-design)
 + improvements in the warning analyzer (spaces/tabs can be omitted)
 + other improvements and fixes
 + now NppExec gets its configuration directory from Notepad++
 + now you can exit a console process or interrupt current script at any time
 + now both Console Output and Input can be ANSI, OEM or UTF-8
 + now each copy of NppExec.dll has its own Console History
 + new option: Follow $(CURRENT_DIRECTORY)
 + new command: NPP_CONSOLE
 + new environment variable: $(PLUGINS_CONFIG_DIR)

 
 v0.2.5 release 2 - September 2008
 ---------------------------------
 - fixed: (potential) bug in CNppExec::DoExecScript()
 - fixed: several (potential) bugs, thanks to Patrick Corwin
 
 
 v0.2.5 - August 2008
 --------------------
 + added: word wrap in the Console dialog (Ctrl+W)
 * changed: Advanced Options dialog - SaveCmdHistory added, 
   RichEdit_MaxTextLength removed
 - fixed: script with spaces in its name was not started using
   associated menu item
 + some improvements
 + new command: NPP_CLOSE
 + new command: INPUTBOX
 * now NppExec uses Rich Edit 2.0 (instead of 1.0) in its Console window
 + added Notepad++'s variables: $(CURRENT_LINE), $(CURRENT_COLUMN)
 + external plugin interface messages:
   NPEM_GETVERDWORD [returns version as DWORD]
   NPEM_GETVERSTR [returns version as string]
   NPEM_GETSTATE [returns state: ready/busy]
   NPEM_PRINT [prints text which can be highlighted by the Highlight Filters]
   NPEM_EXECUTE [executes given commands]
   NPEM_NPPEXEC [executes script/file]
   NPEN_RESULT [as a result of NPEM_EXECUTE or NPEM_NPPEXEC]
 
 
 v0.2.4 - June 2008
 ------------------
 + added: "Replace" tab in the Console Filters dialog
 * improved: the code of the DlgConsoleOutputFilter has been rewritten
 + added: Console text and background colours can be changed
 + new command: SEL_SAVETO - saves the selected text to a file
 - fixed: CON_SAVETO did not work under Windows 9x
 + added: ability to execute specified script when Notepad++ starts/exits
 + added: ability to assign a menu item to specified script
   (then you can assign a shortcut key to this menu item in Notepad++)
 + added: Advanced Options dialog
 + other improvements

 
 v0.2 RC3.2 - February 2008
 --------------------------
 - fixed: NPP_RUN (ShellExecute) did not work with several arguments


 v0.2 RC3.1 - February 2008
 --------------------------
 - fixed: buffer overflow in the Warning Analyzer
 * changed: now NPP_RUN calls ShellExecute() instead of WinExec()

 
 v0.2 RC3 - January 2008
 -----------------------
 + search in the Console
 + history in the Console Filters dialog
 + several issues fixed
 + several cosmetic improvements

 
 v0.2 RC2 - December 2007
 ------------------------
 + speeding up the console output
 + watching for changes in "npes_saved.txt"
 + TAB-key in the console auto-completes current command
 + several cosmetic improvements


 v0.2 RC1 - December 2007
 ------------------------
 1. A lot of internal changes/improvements.
 In particular, now all plugin's options are stored, read and written by the
 Options Manager component.
 The Warning Analyzer component, developed by Nicolas Babled, allows to filter
 and recognize different compiler/interpreter error and warning messages - and
 extract useful information from these messages (such as file name and line 
 number).
 The Simple Logger component stores detailed information about last executed
 command (can be usefull for "debugging").
 * The console redirecting engine has been improved.
 * Now the plugin identifies itself by its dll-file name. Each copy of the 
 plugin (each dll-file) stores its options in corresponding ini-file.
 * Now plugin's environment variables (e.g. "$(FILE_NAME)") are available for
 child console process. For example, start "cmd.exe" and type "echo $(#1)".
 * Now system's environment variables are available in a form of "$(SYS.<var>)"
 e.g. "$(SYS.WINDIR)", "$(SYS.TEMP)" etc.
 * Now the NPP_EXEC command can pass arguments to the script. These arguments 
 can be accessed in the script through the following macro-variables:
 $(ARGC), $(ARGV), $(ARGV[n]).
 Example:  npp_exec "script name" param_1 "param 2".
 This example sets the following values for the script:
 $(ARGC) = 3;  $(ARGV) = param_1 "param 2";  $(ARGV[0]) = script name;
 $(ARGV[1]) = param_1;  $(ARGV[2]) = param 2.
 * Each command now have detailed help information. Type "help" or "help help" 
 in the NppExec's Console.
 * Now the console commands history is restored when you restart Notepad++.

 2. New menu items:
 (Thanks to Nicolas Babled & Geert Vancompernolle)
 Direct Execute Previous - directly executes previous script.
 No internal messages - do not show such messages as "Process started >>>", etc.
 Save all files on execute - saves all files before executing a script.
 Also NppExec's toolbar icon has been added.

 3. New dialog:  Console Output Filter -> Highlight.
 (Thanks to Nicolas Babled & Geert Vancompernolle)
 This dialog allows you to specify a string mask for compiler/interpreter error 
 and warning messages with such keywords as %FILE%, %ABSFILE% and %LINE%.
 These messages can be highlighted with different colours and allow you to go
 to certain line in certain file when double-clicked in the Console window.

 4. New messages:
 NPP_SAVEALL - saves all modified files.
 CON_LOADFROM - loads a file's content to the Console.
 CON_SAVETO - saves the Console's content to a file.
 ECHO - prints a text in the Console (supports internal env. variables)
 SET - sets the value of user's variable
 UNSET - removes user's variable


 v0.2 beta4 - July 2007
 ----------------------
 - fixed: buffer size was not checked before calling OemToChar() 
 - fixed/added: '\b' and '\r' handling
 + a lot of changes in plugin's internals (basic cpp-classes and NppExecEngine)
 + new command: cd, cd <path>, cd <drive:\path>
 + new command: dir, dir <mask>, dir <path\mask>
 + new feature: npp_open <mask>, npp_open <path\mask>
 + new environment variables: $(LEFT_VIEW_FILE) and $(RIGHT_VIEW_FILE)
 

 v0.2 beta3 - April 2007
 -----------------------
 - fixed: Delete button (Script name dialog) was not enabled sometimes
 + Menu: Show Console Dialog
 + Console Output Filter: Exclude all empty lines
 + Comment delimiter can be user-defined, // in :// is no more comment

 
 v0.2 beta2 - March 2007
 -----------------------
 + Menu: Console Commands History.
     When enabled, previous commands can be scrolled by pressing 
     arrow keys Up and Down.
 + Menu: Console Output Filter.
     When enabled, output messages from running console process
     can be filtered (i.e. particular lines can be excluded)
 - ConsoleDlg: Ctrl+A is unlocked
 - ConsoleDlg: Quotes "" are no more added automatically for paths
     with spaces (because of a bug with executables w/o extension
     such as "cmd /c calc.exe")
 - Several internal fixes
 * Thanks to Jim Granville for his suggestions

    
 v0.1 - March 2007
 -----------------
 * initial version
 

 ****************************************************************************
 */

#ifndef _npp_exec_h_
#define _npp_exec_h_
//--------------------------------------------------------------------
#include "base.h"

#include "NppExecCommandExecutor.h"
#include "npp_files/PluginInterface.h"
#include "CAnyWindow.h"
#include "CAnyRichEdit.h"
#include "cpp/CListT.h"
#include "cpp/CStrT.h"
#include "cpp/CBufT.h"
#include "cpp/StrSplitT.h"
#include "NppScriptList.h"
#include "CSimpleLogger.h"
#include "DlgConsole.h"
#include "DlgConsoleOutputFilter.h"
#include "CStaticOptionsManager.h"
#include "WarningAnalyzer.h"
#include "PluginCommunication/NppExecPluginInterface.h"
#include <map>
#include <list>
#include <functional>
#include <iterator>

#define NPPEXEC_VER_DWORD 0x08F0
#define NPPEXEC_VER_STR   _T("0.8")

#define SCRPTENGNID_DEBUG_OUTPUT 0

#define FILEPATH_BUFSIZE       500
#define CONSOLECOMMAND_BUFSIZE 4096
#define OUTPUTFILTER_BUFSIZE   200
#define CONSOLEPIPE_BUFSIZE    8000
#define MAX_USERMENU_ITEMS     100    // OPTS_USERMENU_ITEMxx, pUserMenuFuncs

#define DEFAULT_NPPEXEC_CMD_PREFIX_COLLATERAL_FORCED _T("nppexec::")
#define DEFAULT_NPPEXEC_CMD_PREFIX _T("nppexec:")
#define DEFAULT_ALIAS_CMD_NPPEXEC  _T("\\")
#define DEFAULT_ALIAS_CMD_LABEL    _T(':')
#define DEFAULT_KEY_ENTER          _T("\\n")

const COLORREF COLOR_CON_TEXTNORM = RGB(0x00, 0x00, 0x00); // black
const COLORREF COLOR_CON_TEXTERR  = RGB(0xA0, 0x10, 0x10); // ~red
const COLORREF COLOR_CON_TEXTMSG  = RGB(0x20, 0x80, 0x20); // ~green
const COLORREF COLOR_CON_BKGND    = 0xFFFFFFFF; // means system default

enum enumNFuncItems {
  N_DO_EXEC_DLG = 0,
  N_DIRECT_EXEC,
  N_EXEC_SELTEXT,
  N_EXEC_CLIPTEXT,
  N_SEPARATOR_1,
  N_SHOWCONSOLE,
  N_TOGGLECONSOLE,
  N_GOTO_NEXT_ERROR,
  N_GOTO_PREV_ERROR,
  N_SEPARATOR_2,
  N_CMDHISTORY,
  N_CONSOLE_ENC,

#ifdef _SCROLL_TO_LATEST  
  N_SCROLL2LATEST,
#endif

  N_NOINTMSGS,
  N_SAVEONEXECUTE,
  N_CDCURDIR,

#ifdef _DISABLE_CMD_ALIASES
  N_NOCMDALIASES,
#endif

  N_SEPARATOR_3,
  N_OUTPUT_FILTER,
  N_ADV_OPTIONS,
  N_CONSOLE_FONT,
  N_EXECDLG_FONT,
  N_SEPARATOR_4,
  N_HELP_MANUAL,
  N_HELP_DOCS,
  N_HELP_ABOUT,
  nbFunc
};

enum EPluginOptions {
    OPTS_PLUGIN_HOTKEY = 0,
    OPTU_PLUGIN_HOTKEY,
    OPTB_NPE_DEBUGLOG,
    OPTI_TOOLBARBTN,
    OPTS_SCRIPT_NPPSTART,
    OPTS_SCRIPT_NPPEXIT,
    OPTU_CHILDP_STARTUPTIMEOUT_MS,
    OPTU_CHILDP_CYCLETIMEOUT_MS,
    OPTU_CHILDP_EXITTIMEOUT_MS,
    OPTU_CHILDP_KILLTIMEOUT_MS,
    OPTU_CHILDP_RUNPOLICY,
    OPTS_CHILDP_COMSPECSWITCHES,
    OPTU_CHILDS_SYNCTIMEOUT_MS,
    OPTU_EXITS_TIMEOUT_MS,
    OPTB_PATH_AUTODBLQUOTES,
    OPTI_CMDHISTORY_MAXITEMS,
    OPTI_EXEC_MAXCOUNT,
    OPTI_GOTO_MAXCOUNT,
    OPTI_EXECTEXT_MAXCOUNT,
    OPTS_ALIAS_CMD_NPPEXEC,
    OPTS_KEY_ENTER,
    OPTD_CONSOLE_NULCHAR,
    OPTS_NPPEXEC_CMD_PREFIX,
    OPTI_UTF8_DETECT_LENGTH,
    OPTI_RICHEDIT_MAXTEXTLEN,
    OPTI_SENDMSG_MAXBUFLEN,
    OPTS_CALC_PRECISION,
    OPTS_COMMENTDELIMITER,
    OPTI_CONSOLE_VISIBLE,
    OPTB_CONSOLE_SHOWHELP,
    OPTU_CONSOLE_ENCODING,
    OPTB_CONSOLE_WORDWRAP,
    OPTB_CONSOLE_HIDETOGGLED,
    OPTB_CONSOLE_TABASCHAR,
    OPTB_CONSOLE_SAVEONEXECUTE,
    OPTB_CONSOLE_CDCURDIR,
    OPTI_CONSOLE_CD_UNNAMEDFILE,
    OPTB_CONSOLE_CMDHISTORY,
    OPTB_CONSOLE_SAVECMDHISTORY,

  #ifdef _SCROLL_TO_LATEST  
    OPTB_CONSOLE_SCROLL2LATEST,
  #endif

    OPTB_CONSOLE_NOINTMSGS,
    OPTB_CONSOLE_PRINTMSGREADY,
    OPTS_CONSOLE_CUSTOMMSGREADY,
    OPTB_CONSOLE_NOEMPTYVARS,
    OPTB_CONSOLE_NOCMDALIASES,
    OPTD_EXECDLG_FONT,
    OPTD_CONSOLE_FONT,
    OPTB_CONSOLE_APPENDMODE,
    OPTU_CONSOLE_CATCHSHORTCUTKEYS,
    OPTB_CONSOLE_SETOUTPUTVAR,
    OPTB_CONSOLE_KILLPROCTREE,
    OPTI_CONSOLE_ANSIESCSEQ,
    OPTI_CONSOLE_EXECCLIPTEXTMODE,
    OPTI_CONSOLE_EXECSELTEXTMODE,
    OPTB_CONFLTR_ENABLE,
    OPTB_CONFLTR_COMPILER_ERRORS,
    OPTB_CONFLTR_EXCLALLEMPTY,
    OPTB_CONFLTR_EXCLDUPEMPTY,
    OPTI_CONFLTR_INCLMASK,
    OPTI_CONFLTR_EXCLMASK,
    OPTS_CONFLTR_INCLLINE1,
    OPTS_CONFLTR_INCLLINE2, // = OPTS_CONFLTR_INCLLINE1 + 1,
    OPTS_CONFLTR_INCLLINE3, // = OPTS_CONFLTR_INCLLINE1 + 2,
    OPTS_CONFLTR_INCLLINE4, // = OPTS_CONFLTR_INCLLINE1 + 3,
    OPTS_CONFLTR_INCLLINE5, // = OPTS_CONFLTR_INCLLINE1 + 4,
    OPTS_CONFLTR_INCLLINE6, // = OPTS_CONFLTR_INCLLINE1 + 5,
    OPTS_CONFLTR_INCLLINE7, // = OPTS_CONFLTR_INCLLINE1 + 6,
    OPTS_CONFLTR_INCLLINE8, // = OPTS_CONFLTR_INCLLINE1 + 7,
    OPTS_CONFLTR_EXCLLINE1,  
    OPTS_CONFLTR_EXCLLINE2, // = OPTS_CONFLTR_EXCLLINE1 + 1,
    OPTS_CONFLTR_EXCLLINE3, // = OPTS_CONFLTR_EXCLLINE1 + 2,
    OPTS_CONFLTR_EXCLLINE4, // = OPTS_CONFLTR_EXCLLINE1 + 3,
    OPTS_CONFLTR_EXCLLINE5, // = OPTS_CONFLTR_EXCLLINE1 + 4,
    OPTS_CONFLTR_EXCLLINE6, // = OPTS_CONFLTR_EXCLLINE1 + 5,
    OPTS_CONFLTR_EXCLLINE7, // = OPTS_CONFLTR_EXCLLINE1 + 6,
    OPTS_CONFLTR_EXCLLINE8, // = OPTS_CONFLTR_EXCLLINE1 + 7,
    OPTB_CONFLTR_R_ENABLE,
    OPTB_CONFLTR_R_EXCLEMPTY,
    OPTI_CONFLTR_R_FINDMASK,
    OPTI_CONFLTR_R_CASEMASK,
    OPTS_CONFLTR_R_FIND1,
    OPTS_CONFLTR_R_FIND2,
    OPTS_CONFLTR_R_FIND3,
    OPTS_CONFLTR_R_FIND4,
    OPTS_CONFLTR_R_FIND5,
    OPTS_CONFLTR_R_FIND6,
    OPTS_CONFLTR_R_FIND7,
    OPTS_CONFLTR_R_FIND8,
    OPTS_CONFLTR_R_RPLC1,
    OPTS_CONFLTR_R_RPLC2,
    OPTS_CONFLTR_R_RPLC3,
    OPTS_CONFLTR_R_RPLC4,
    OPTS_CONFLTR_R_RPLC5,
    OPTS_CONFLTR_R_RPLC6,
    OPTS_CONFLTR_R_RPLC7,
    OPTS_CONFLTR_R_RPLC8,
    OPTS_CONFLTR_RCGNMSK1,
    OPTS_CONFLTR_RCGNMSK2,  // = OPTS_CONFLTR_RCGNMSK1 + 1,
    OPTS_CONFLTR_RCGNMSK3,  // = OPTS_CONFLTR_RCGNMSK1 + 2,
    OPTS_CONFLTR_RCGNMSK4,  // = OPTS_CONFLTR_RCGNMSK1 + 3,
    OPTS_CONFLTR_RCGNMSK5,  // = OPTS_CONFLTR_RCGNMSK1 + 4,
    OPTS_CONFLTR_RCGNMSK6,  // = OPTS_CONFLTR_RCGNMSK1 + 5,
    OPTS_CONFLTR_RCGNMSK7,  // = OPTS_CONFLTR_RCGNMSK1 + 6,
    OPTS_CONFLTR_RCGNMSK8,  // = OPTS_CONFLTR_RCGNMSK1 + 7,
    OPTS_CONFLTR_RCGNMSK9,  // = OPTS_CONFLTR_RCGNMSK1 + 8,
    OPTS_CONFLTR_RCGNMSK10, // = OPTS_CONFLTR_RCGNMSK1 + 9,
    OPTS_CONFLTR_RCGNEFF1,
    OPTS_CONFLTR_RCGNEFF2,  // = OPTS_CONFLTR_RCGNEFF1 + 1,
    OPTS_CONFLTR_RCGNEFF3,  // = OPTS_CONFLTR_RCGNEFF1 + 2,
    OPTS_CONFLTR_RCGNEFF4,  // = OPTS_CONFLTR_RCGNEFF1 + 3,
    OPTS_CONFLTR_RCGNEFF5,  // = OPTS_CONFLTR_RCGNEFF1 + 4,
    OPTS_CONFLTR_RCGNEFF6,  // = OPTS_CONFLTR_RCGNEFF1 + 5,
    OPTS_CONFLTR_RCGNEFF7,  // = OPTS_CONFLTR_RCGNEFF1 + 6,
    OPTS_CONFLTR_RCGNEFF8,  // = OPTS_CONFLTR_RCGNEFF1 + 7,
    OPTS_CONFLTR_RCGNEFF9,  // = OPTS_CONFLTR_RCGNEFF1 + 8,
    OPTS_CONFLTR_RCGNEFF10, // = OPTS_CONFLTR_RCGNEFF1 + 9,
    OPTB_WATCHSCRIPTFILE,
    OPTB_SAVELASTSCRIPT,

    OPTS_FILTERS_EXCL1,
    OPTS_FILTERS_EXCL2,
    OPTS_FILTERS_EXCL3,
    OPTS_FILTERS_EXCL4,
    OPTS_FILTERS_EXCL5,
    OPTS_FILTERS_EXCL6,
    OPTS_FILTERS_EXCL7,
    OPTS_FILTERS_EXCL8,
    OPTS_FILTERS_EXCL9,
    OPTS_FILTERS_EXCL10,
    OPTS_FILTERS_EXCL11,
    OPTS_FILTERS_EXCL12,
    OPTS_FILTERS_EXCL13,
    OPTS_FILTERS_EXCL14,
    OPTS_FILTERS_EXCL15,
    OPTS_FILTERS_EXCL16,
    OPTS_FILTERS_EXCL17,
    OPTS_FILTERS_EXCL18,
    OPTS_FILTERS_EXCL19,
    OPTS_FILTERS_EXCL20,

    OPTS_FILTERS_INCL1,
    OPTS_FILTERS_INCL2,
    OPTS_FILTERS_INCL3,
    OPTS_FILTERS_INCL4,
    OPTS_FILTERS_INCL5,
    OPTS_FILTERS_INCL6,
    OPTS_FILTERS_INCL7,
    OPTS_FILTERS_INCL8,
    OPTS_FILTERS_INCL9,
    OPTS_FILTERS_INCL10,
    OPTS_FILTERS_INCL11,
    OPTS_FILTERS_INCL12,
    OPTS_FILTERS_INCL13,
    OPTS_FILTERS_INCL14,
    OPTS_FILTERS_INCL15,
    OPTS_FILTERS_INCL16,
    OPTS_FILTERS_INCL17,
    OPTS_FILTERS_INCL18,
    OPTS_FILTERS_INCL19,
    OPTS_FILTERS_INCL20,

    OPTS_FILTERS_R_FIND1,
    OPTS_FILTERS_R_FIND2,
    OPTS_FILTERS_R_FIND3,
    OPTS_FILTERS_R_FIND4,
    OPTS_FILTERS_R_FIND5,
    OPTS_FILTERS_R_FIND6,
    OPTS_FILTERS_R_FIND7,
    OPTS_FILTERS_R_FIND8,
    OPTS_FILTERS_R_FIND9,
    OPTS_FILTERS_R_FIND10,
    OPTS_FILTERS_R_FIND11,
    OPTS_FILTERS_R_FIND12,
    OPTS_FILTERS_R_FIND13,
    OPTS_FILTERS_R_FIND14,
    OPTS_FILTERS_R_FIND15,
    OPTS_FILTERS_R_FIND16,
    OPTS_FILTERS_R_FIND17,
    OPTS_FILTERS_R_FIND18,
    OPTS_FILTERS_R_FIND19,
    OPTS_FILTERS_R_FIND20,
    
    OPTS_FILTERS_R_RPLC1,
    OPTS_FILTERS_R_RPLC2,
    OPTS_FILTERS_R_RPLC3,
    OPTS_FILTERS_R_RPLC4,
    OPTS_FILTERS_R_RPLC5,
    OPTS_FILTERS_R_RPLC6,
    OPTS_FILTERS_R_RPLC7,
    OPTS_FILTERS_R_RPLC8,
    OPTS_FILTERS_R_RPLC9,
    OPTS_FILTERS_R_RPLC10,
    OPTS_FILTERS_R_RPLC11,
    OPTS_FILTERS_R_RPLC12,
    OPTS_FILTERS_R_RPLC13,
    OPTS_FILTERS_R_RPLC14,
    OPTS_FILTERS_R_RPLC15,
    OPTS_FILTERS_R_RPLC16,
    OPTS_FILTERS_R_RPLC17,
    OPTS_FILTERS_R_RPLC18,
    OPTS_FILTERS_R_RPLC19,
    OPTS_FILTERS_R_RPLC20,
    
    OPTS_FILTERS_HGLT1,
    OPTS_FILTERS_HGLT2,
    OPTS_FILTERS_HGLT3,
    OPTS_FILTERS_HGLT4,
    OPTS_FILTERS_HGLT5,
    OPTS_FILTERS_HGLT6,
    OPTS_FILTERS_HGLT7,
    OPTS_FILTERS_HGLT8,
    OPTS_FILTERS_HGLT9,
    OPTS_FILTERS_HGLT10,
    OPTS_FILTERS_HGLT11,
    OPTS_FILTERS_HGLT12,
    OPTS_FILTERS_HGLT13,
    OPTS_FILTERS_HGLT14,
    OPTS_FILTERS_HGLT15,
    OPTS_FILTERS_HGLT16,
    OPTS_FILTERS_HGLT17,
    OPTS_FILTERS_HGLT18,
    OPTS_FILTERS_HGLT19,
    OPTS_FILTERS_HGLT20,

    OPTI_SELECTED_SCRIPT,
    OPTI_DOEXECDLG_WIDTH,
    OPTI_DOEXECDLG_HEIGHT,

    OPTD_COLOR_TEXTNORM,
    OPTD_COLOR_TEXTERR,
    OPTD_COLOR_TEXTMSG,
    OPTD_COLOR_BKGND,

    OPTB_USERMENU_NPPMACROS,
    OPTS_USERMENU_ITEM01,
    OPTS_USERMENU_ITEM02,
    OPTS_USERMENU_ITEM03,
    OPTS_USERMENU_ITEM04,
    OPTS_USERMENU_ITEM05,
    OPTS_USERMENU_ITEM06,
    OPTS_USERMENU_ITEM07,
    OPTS_USERMENU_ITEM08,
    OPTS_USERMENU_ITEM09,
    OPTS_USERMENU_ITEM10,
    OPTS_USERMENU_ITEM11,
    OPTS_USERMENU_ITEM12,
    OPTS_USERMENU_ITEM13,
    OPTS_USERMENU_ITEM14,
    OPTS_USERMENU_ITEM15,
    OPTS_USERMENU_ITEM16,
    OPTS_USERMENU_ITEM17,
    OPTS_USERMENU_ITEM18,
    OPTS_USERMENU_ITEM19,
    OPTS_USERMENU_ITEM20,
    OPTS_USERMENU_ITEM21,
    OPTS_USERMENU_ITEM22,
    OPTS_USERMENU_ITEM23,
    OPTS_USERMENU_ITEM24,
    OPTS_USERMENU_ITEM25,
    OPTS_USERMENU_ITEM26,
    OPTS_USERMENU_ITEM27,
    OPTS_USERMENU_ITEM28,
    OPTS_USERMENU_ITEM29,
    OPTS_USERMENU_ITEM30,
    OPTS_USERMENU_ITEM31,
    OPTS_USERMENU_ITEM32,
    OPTS_USERMENU_ITEM33,
    OPTS_USERMENU_ITEM34,
    OPTS_USERMENU_ITEM35,
    OPTS_USERMENU_ITEM36,
    OPTS_USERMENU_ITEM37,
    OPTS_USERMENU_ITEM38,
    OPTS_USERMENU_ITEM39,
    OPTS_USERMENU_ITEM40,
    OPTS_USERMENU_ITEM41,
    OPTS_USERMENU_ITEM42,
    OPTS_USERMENU_ITEM43,
    OPTS_USERMENU_ITEM44,
    OPTS_USERMENU_ITEM45,
    OPTS_USERMENU_ITEM46,
    OPTS_USERMENU_ITEM47,
    OPTS_USERMENU_ITEM48,
    OPTS_USERMENU_ITEM49,
    OPTS_USERMENU_ITEM50,
    OPTS_USERMENU_ITEM51,
    OPTS_USERMENU_ITEM52,
    OPTS_USERMENU_ITEM53,
    OPTS_USERMENU_ITEM54,
    OPTS_USERMENU_ITEM55,
    OPTS_USERMENU_ITEM56,
    OPTS_USERMENU_ITEM57,
    OPTS_USERMENU_ITEM58,
    OPTS_USERMENU_ITEM59,
    OPTS_USERMENU_ITEM60,
    OPTS_USERMENU_ITEM61,
    OPTS_USERMENU_ITEM62,
    OPTS_USERMENU_ITEM63,
    OPTS_USERMENU_ITEM64,
    OPTS_USERMENU_ITEM65,
    OPTS_USERMENU_ITEM66,
    OPTS_USERMENU_ITEM67,
    OPTS_USERMENU_ITEM68,
    OPTS_USERMENU_ITEM69,
    OPTS_USERMENU_ITEM70,
    OPTS_USERMENU_ITEM71,
    OPTS_USERMENU_ITEM72,
    OPTS_USERMENU_ITEM73,
    OPTS_USERMENU_ITEM74,
    OPTS_USERMENU_ITEM75,
    OPTS_USERMENU_ITEM76,
    OPTS_USERMENU_ITEM77,
    OPTS_USERMENU_ITEM78,
    OPTS_USERMENU_ITEM79,
    OPTS_USERMENU_ITEM80,
    OPTS_USERMENU_ITEM81,
    OPTS_USERMENU_ITEM82,
    OPTS_USERMENU_ITEM83,
    OPTS_USERMENU_ITEM84,
    OPTS_USERMENU_ITEM85,
    OPTS_USERMENU_ITEM86,
    OPTS_USERMENU_ITEM87,
    OPTS_USERMENU_ITEM88,
    OPTS_USERMENU_ITEM89,
    OPTS_USERMENU_ITEM90,
    OPTS_USERMENU_ITEM91,
    OPTS_USERMENU_ITEM92,
    OPTS_USERMENU_ITEM93,
    OPTS_USERMENU_ITEM94,
    OPTS_USERMENU_ITEM95,
    OPTS_USERMENU_ITEM96,
    OPTS_USERMENU_ITEM97,
    OPTS_USERMENU_ITEM98,
    OPTS_USERMENU_ITEM99,
    OPTS_USERMENU_ITEM100,

    OPTS_INPUTBOX_VALUE1,
    OPTS_INPUTBOX_VALUE2,
    OPTS_INPUTBOX_VALUE3,
    OPTS_INPUTBOX_VALUE4,
    OPTS_INPUTBOX_VALUE5,
    OPTS_INPUTBOX_VALUE6,
    OPTS_INPUTBOX_VALUE7,
    OPTS_INPUTBOX_VALUE8,
    OPTS_INPUTBOX_VALUE9,
    OPTS_INPUTBOX_VALUE10,
    OPTS_INPUTBOX_VALUE11,
    OPTS_INPUTBOX_VALUE12,
    OPTS_INPUTBOX_VALUE13,
    OPTS_INPUTBOX_VALUE14,
    OPTS_INPUTBOX_VALUE15,
    OPTS_INPUTBOX_VALUE16,
    OPTS_INPUTBOX_VALUE17,
    OPTS_INPUTBOX_VALUE18,
    OPTS_INPUTBOX_VALUE19,
    OPTS_INPUTBOX_VALUE20,

    OPTS_EXITBOX_VALUE1,
    OPTS_EXITBOX_VALUE2,
    OPTS_EXITBOX_VALUE3,
    OPTS_EXITBOX_VALUE4,
    OPTS_EXITBOX_VALUE5,
    OPTS_EXITBOX_VALUE6,
    OPTS_EXITBOX_VALUE7,
    OPTS_EXITBOX_VALUE8,
    OPTS_EXITBOX_VALUE9,
    OPTS_EXITBOX_VALUE10,
    OPTS_EXITBOX_VALUE11,
    OPTS_EXITBOX_VALUE12,
    OPTS_EXITBOX_VALUE13,
    OPTS_EXITBOX_VALUE14,
    OPTS_EXITBOX_VALUE15,
    OPTS_EXITBOX_VALUE16,
    OPTS_EXITBOX_VALUE17,
    OPTS_EXITBOX_VALUE18,
    OPTS_EXITBOX_VALUE19,
    OPTS_EXITBOX_VALUE20,

    OPTS_PLUGIN_HELPFILE,
    OPTS_PLUGIN_TEMPSCRIPTFILE,
    OPTS_PLUGIN_LASTSCRIPTFILE,
    OPTS_PLUGIN_SAVEDSCRIPTSFILE,
    OPTS_PLUGIN_LOGSDIR,
    OPTS_PLUGIN_SCRIPTSDIR,
    OPTU_PLUGIN_AUTOSAVE_SECONDS,

    OPT_COUNT
};

class CScriptEngine;
class CNppExec;
class CInputBoxDlg;

class CNppConsoleRichEdit : public CAnyRichEdit
{
public:
    CNppConsoleRichEdit();

//  The following two functions are commented, because NppExec actually
//  expects and deals with null-terminated strings that end with '\0'
//  everywhere. It means that all the strings that come to AddLine() or
//  AddStr() are null-terminated - i.e. there's nothing after the first
//  '\0' in them.
//  And readPipesAndOutput() is the only function that deals 
//  with '\0' characters inside of it.

//    void  AddLine(const TCHAR* cszLine, BOOL bScrollText = FALSE, 
//                  COLORREF color = RGB(0,0,0), 
//                  DWORD dwMask = CFM_EFFECTS, DWORD dwEffects = 0);

//    void  AddStr(const TCHAR* cszStr, BOOL bScrollText = FALSE, 
//                 COLORREF color = RGB(0,0,0), 
//                 DWORD dwMask = CFM_EFFECTS, DWORD dwEffects = 0);

    static TCHAR GetNulChar();
};

class CNppExecConsole
{
public:
    typedef DWORD ScriptEngineId;

public:
    CNppExecConsole();
    ~CNppExecConsole();

    // NppExec...
    //CNppExec* GetNppExec() const;
    //void      SetNppExec(CNppExec* pNppExec);

    // colors...
    COLORREF GetCurrentColorTextNorm() const;
    COLORREF GetCurrentColorTextMsg() const;
    COLORREF GetCurrentColorTextErr() const;
    COLORREF GetCurrentColorBkgnd() const;

    void SetCurrentColorTextNorm(COLORREF colorTextNorm);
    void SetCurrentColorTextMsg(COLORREF colorTextMsg);
    void SetCurrentColorTextErr(COLORREF colorTextErr);
    void SetCurrentColorBkgnd(COLORREF colorBkgnd);

    // print...
    enum ePrintFlags {
        pfLogThisMsg    = 0x01,
        pfNewLine       = 0x02,
        pfIsInternalMsg = 0x04
    };
    void PrintError(LPCTSTR cszMessage, UINT nPrintFlags = pfLogThisMsg);
    void PrintMessage(LPCTSTR cszMessage, UINT nPrintFlags);
    void PrintOutput(LPCTSTR cszMessage, UINT nPrintFlags = pfLogThisMsg|pfNewLine);
    void PrintStr(LPCTSTR cszStr, UINT nPrintFlags = pfLogThisMsg|pfNewLine);
    void PrintSysError(LPCTSTR cszFunctionName, DWORD dwErrorCode, UINT nPrintFlags = pfLogThisMsg);

    void LockConsolePos(INT nPos, bool bForce = false);
    void LockConsoleEndPos(bool bForce = false);
    void LockConsoleEndPosAfterEnterPressed();

    // get/set...
    CAnyRichEdit& GetConsoleEdit();

    HWND GetConsoleWnd() const;
    void SetConsoleWnd(HWND hWndRichEdit);

    HWND GetDialogWnd() const;
    void SetDialogWnd(HWND hDlg);
    
    int  IsOutputEnabledN() const;
    bool IsOutputEnabled() const;
    void SetOutputEnabled(bool bEnabled);
    void SetOutputEnabledDirectly(ScriptEngineId scrptEngnId, int nEnabled);
    int  GetOutputEnabledDirectly(ScriptEngineId scrptEngnId) const;

    // other...
    void ClearText(bool bForce = false);
    BOOL IsScrollToEnd() const;
    void RestoreDefaultTextStyle(bool bLockPos);
    void UpdateColours();

    // special characters...
    void ProcessSlashR(); // "\r"
    void ProcessSlashB(int nCount); // "\b"

    void OnScriptEngineStarted();
    void OnScriptEngineFinished();
    void OnScriptEngineAborting(DWORD dwThreadId);

protected:
    static inline ScriptEngineId GetScriptEngineId() { return ::GetCurrentThreadId(); }

    class ConsoleState
    {
    public:
        ConsoleState();
        ~ConsoleState();

        void execPostponedCalls(CCriticalSection* csState);

        // state parameters
        int      nOutputEnabled;
        //COLORREF colorTextNorm;
        //COLORREF colorTextMsg;
        //COLORREF colorTextErr;
        //COLORREF colorBkgnd;

        // script engine identifier
        ScriptEngineId ScrptEngnId;

        // this state is ready to be removed
        bool IsFinished;

        // abort requested
        volatile bool IsAbortRequested;
        
        // postponed calls
        std::list< std::function<void ()> > PostponedCalls;
        std::list< tstr > PostponedStrings;
    };

protected:
    // critical sections are created first and destroyed last...
    mutable CCriticalSection m_csStateList;
    // data...
    //CNppExec* m_pNppExec;
    CNppConsoleRichEdit m_reConsole;
    HWND m_hDlg;
    COLORREF m_colorTextNorm;
    COLORREF m_colorTextMsg;
    COLORREF m_colorTextErr;
    COLORREF m_colorBkgnd;
    std::list<ConsoleState> m_StateList;

    const ConsoleState& _getState(ScriptEngineId scrptEngnId) const;
    ConsoleState& _getState(ScriptEngineId scrptEngnId);
    
    bool postponeThisCall(ScriptEngineId scrptEngnId);

    COLORREF _getCurrentColorTextNorm() const;
    COLORREF _getCurrentColorTextMsg() const;
    COLORREF _getCurrentColorTextErr() const;
    COLORREF _getCurrentColorBkgnd() const;

    void _setCurrentColorTextNorm(COLORREF colorTextNorm);
    void _setCurrentColorTextMsg(COLORREF colorTextMsg);
    void _setCurrentColorTextErr(COLORREF colorTextErr);
    void _setCurrentColorBkgnd(COLORREF colorBkgnd);

    void _printError(ScriptEngineId scrptEngnId, LPCTSTR cszMessage, UINT nPrintFlags);
    void _printMessage(ScriptEngineId scrptEngnId, LPCTSTR cszMessage, UINT nPrintFlags);
    void _printOutput(ScriptEngineId scrptEngnId, LPCTSTR cszMessage, UINT nPrintFlags);
    void _printStr(ScriptEngineId scrptEngnId, LPCTSTR cszStr, UINT nPrintFlags);
    void _printSysError(ScriptEngineId scrptEngnId, LPCTSTR cszFunctionName, DWORD dwErrorCode, UINT nPrintFlags);

    void _lockConsolePos(ScriptEngineId scrptEngnId, INT nPos);
    void _lockConsoleEndPos(ScriptEngineId scrptEngnId);
    void _lockConsoleEndPosAfterEnterPressed(ScriptEngineId scrptEngnId);

    int  _isOutputEnabledN(ScriptEngineId scrptEngnId) const;
    bool _isOutputEnabled(ScriptEngineId scrptEngnId) const;
    void _setOutputEnabled(ScriptEngineId scrptEngnId, bool bEnabled);

    void _clearText(ScriptEngineId scrptEngnId);
    BOOL _isScrollToEnd() const;
    void _restoreDefaultTextStyle(ScriptEngineId scrptEngnId, bool bLockPos);
    void _updateColours(ScriptEngineId scrptEngnId);

    void _processSlashR(ScriptEngineId scrptEngnId);
    void _processSlashB(ScriptEngineId scrptEngnId, int nCount);
};

class CNppExecMacroVars
{
public:
    enum eSetMacroVarFlags
    {
        svRemoveVar = 0x01,
        svLocalVar  = 0x02
    };

public:
    typedef std::map<tstr, tstr> tMacroVars;

public:
    CNppExecMacroVars();

    // NppExec...
    CNppExec*   GetNppExec() const;
    void        SetNppExec(CNppExec* pNppExec);

    // helpers...
    static bool IsLocalMacroVar(tstr& varName);
    static bool ContainsMacroVar(const tstr& S);
    tMacroVars& GetUserLocalMacroVars(CScriptEngine* pScriptEngine); // use with GetCsUserMacroVars()
    tMacroVars& GetUserConsoleMacroVars(); // use with GetCsUserMacroVars()
    tMacroVars& GetUserMacroVars(); // use with GetCsUserMacroVars()
    tMacroVars& GetCmdAliases(); // use with GetCsCmdAliases()

    CCriticalSection& GetCsUserMacroVars();
    CCriticalSection& GetCsCmdAliases();

    // check macro vars...
    static bool CheckCmdArgs(tstr& Cmd, int& pos, const CStrSplitT<TCHAR>& args);
    void        CheckCmdAliases(tstr& S, bool useLogging);
    bool        CheckNppMacroVars(tstr& S, int& pos);
    bool        CheckPluginMacroVars(tstr& S, int& pos);
    bool        CheckUserMacroVars(CScriptEngine* pScriptEngine, tstr& S, int& pos);
    static bool CheckEmptyMacroVars(tstr& S, int& pos);
    bool        CheckAllMacroVars(CScriptEngine* pScriptEngine, tstr& S, bool useLogging, int nCmdType = 0);
    bool        CheckInnerMacroVars(CScriptEngine* pScriptEngine, tstr& S, int& pos, bool useLogging);
    bool        SetUserMacroVar(CScriptEngine* pScriptEngine, tstr& varName, const tstr& varValue, unsigned int nFlags = 0);

    static void MakeCompleteVarName(tstr& varName);

protected:
    static void logInput(const TCHAR* funcName, const TCHAR* inputVar, int pos);
    static void logInput(const TCHAR* funcName, const TCHAR* inputVar);
    static void logOutput(const TCHAR* outputVar);
    static void logNoOutput();
    bool substituteMacroVar(const tstr& Cmd, tstr& S, int& pos,
                            const TCHAR* varName,
                            tstr (*getValue)(CNppExec* pNppExec) );

public:
    class StrCalc
    {
    protected:
        enum eCalcType {
            CT_FPARSER = 0,
            CT_STRLEN,
            CT_STRLENUTF8,
            CT_STRLENSCI,
            CT_STRUPPER,
            CT_STRLOWER,
            CT_SUBSTR,
            CT_STRFIND,
            CT_STRRFIND,
            CT_STRREPLACE,
            CT_STRQUOTE,
            CT_STRUNQUOTE,
            CT_STRESCAPE,
            CT_STRUNESCAPE,
            CT_STREXPAND,
            CT_NORMPATH,
            CT_STRFROMHEX,
            CT_STRTOHEX,
            CT_CHR,
            CT_ORD,
            CT_ORDX
        };

    public:
        StrCalc(tstr& varValue, CNppExec* pNppExec);
        
        StrCalc& operator=(const StrCalc&) = delete;

        bool Process();
        
    protected:
        bool calcFParser();
        bool calcStrLen();
        bool calcStrCase();
        bool calcSubStr();
        bool calcStrFind();
        bool calcStrRplc();
        bool calcStrFromHex();
        bool calcStrToHex();
        bool calcChr();
        bool calcOrd();
        bool calcStrQuote();
        bool calcStrEscape();
        bool calcStrExpand();
        bool calcNormPath();

    protected:
        tstr& m_varValue;
        CNppExec* m_pNppExec;
        int m_calcType;
        const TCHAR* m_pVar;
        tstr m_param;
    };

protected:
    // critical sections are created first and destroyed last...
    CCriticalSection m_csUserMacroVars;
    CCriticalSection m_csCmdAliases;
    // data...
    CNppExec*  m_pNppExec;
    tMacroVars m_UserLocalMacroVars0; // <-- just in case, actually the vars are inside ScriptContext
    tMacroVars m_UserConsoleMacroVars; // local user macro vars in NppExec's Console (Console only, not scripts!)
    tMacroVars m_UserMacroVars; // shared user macro vars (shared by all NppExec's scripts)
    tMacroVars m_CmdAliases;
};

class CNppExec
{
private:
  // critical sections are created first and destroyed last...
  mutable CCriticalSection m_csScriptCmdList;
  CCriticalSection m_csAutoSave;
  // data...
  CStaticOptionsManager m_Options;
  CSimpleLogger m_Logger;
  CWarningAnalyzer m_WarningAnalyzer;
  CNppExecCommandExecutor m_CommandExecutor;
  CNppExecPluginInterfaceImpl m_PluginInterfaceImpl;
  CNppExecConsole m_Console;
  CNppExecMacroVars m_MacroVars;
  CListT<tstr> m_ScriptCmdList;
  CListT<tstr> m_LastSavedCmdList;

  TCHAR   m_szPluginDllPath[FILEPATH_BUFSIZE];
  TCHAR   m_szConfigPath[FILEPATH_BUFSIZE];
  TCHAR   m_szIniFilePathName[FILEPATH_BUFSIZE];

  HMODULE m_hRichEditDll;

  UINT_PTR m_idTimerAutoSave;
  bool     m_isSavingOptions;

  CEvent m_ExitScriptIsDone;

  static void CALLBACK OnTimer_AutoSaveProc(HWND, UINT, UINT_PTR, DWORD);
  static DWORD WINAPI ExitScriptTimeoutThreadProc(LPVOID lpParam);

  HWND    getCurrentScintilla(INT which);

public:
  typedef int (WINAPI *MSGBOXTIMEOUTFUNC)(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType, WORD wLanguageId, DWORD dwMilliseconds);

  toolbarIcons             m_TB_Icons;
  toolbarIconsWithDarkMode m_TB_IconsWithDarkMode;
    
  HMODULE         m_hDllModule;
  NppData         m_nppData;
  HWND            m_hFocusedWindowBeforeScriptStarted;

  bool            m_bAnotherNppExecDllExists;
      
  CNppScript      m_TempScript;
  bool            m_TempScriptIsModified;

  CNppScriptList  m_ScriptsList;

  int             npp_nbFiles;
  CBufT<TCHAR*>   npp_bufFileNames;

  MSGBOXTIMEOUTFUNC m_lpMsgBoxTimeoutFunc;

  CFileModificationWatcher m_FileWatcher;
  CNppScriptFileChangeListener m_ScriptFileChangeListener;
  
public:
  static bool     _bIsNppReady;
  static bool     _bIsNppShutdown; // should exist even after ~CNppExec()
  bool            _bStopTheExitScript;
  bool            _bOptionsSavedOnNppnShutdown;

  HFONT           _execdlgFont;
  HFONT           _consoleFont;
  bool            _consoleIsVisible;
  bool            _consoleCommandBreak;
  bool            _consoleCommandIsRunning;

public:
  enum eTextEnc {
    encAsIs = 0,
    encANSI,
    encUTF8_BOM,
    encUTF8_NoBOM,
    encUCS2LE
  };

  enum eExecTextFlags {
    etfNone                    = 0,
    etfMacroVarsNoChildProc    = 0x01,
    etfMacroVarsWithChildProc  = 0x02,
    etfCollateralNoChildProc   = 0x04,
    etfCollateralWithChildProc = 0x08,
    etfNppExecPrefix           = 0x10,
    etfLastScript              = 0x20,
    etfShareLocalVars          = 0x40
  };

public:
  CNppExec();
  ~CNppExec();

  CSimpleLogger& GetLogger() { return m_Logger; }
  CWarningAnalyzer& GetWarningAnalyzer() { return m_WarningAnalyzer; }
  CStaticOptionsManager& GetOptions() { return m_Options; }
  const CStaticOptionsManager& GetOptions() const { return m_Options; }

  CListT<tstr> GetCmdList() const;
  void SetCmdList(const CListT<tstr>& CmdList);
  bool IsCmdListEmpty() const;

  int  conLoadFrom(LPCTSTR cszFile); // returns -1 if can't load
  int  conSaveTo(LPCTSTR cszFile); // returns -1 if can't save
  int  textLoadFrom(LPCTSTR cszFile, bool bSelectionOnly); // returns -1 if can't load
  int  textSaveTo(LPTSTR szFileAndEncoding, bool bSelectionOnly); // returns -1 if can't save
  void textSetText(LPCTSTR cszText, bool bSelectionOnly);
  char* sciGetText(bool bSelectionOnly, int* pnTextLen, int* pnSciCodePage);
  tstr  sciGetSelText();
  static char* convertSciText(char* pSciText, int nTextLen, int nSciCodePage, eTextEnc outEnc, int* pnOutLen);
  int  nppConvertToFullPathName(tstr& fileName, bool bGetOpenFileNames, int nView = ALL_OPEN_FILES);
  int  nppGetMenuItemIdByName(const tstr& menuItemPathName, tstr& parsedPath, tstr& parsedSep);
  int  nppGetOpenFileNames();
  int  nppGetOpenFileNamesInView(int nView = PRIMARY_VIEW, int nFiles = -1);
  bool nppSwitchToDocument(const tstr& fileName, bool bGetOpenFileNames, int nView = ALL_OPEN_FILES);
  bool nppSaveAllFiles();
  /* bool nppGetWorkspaceRootFolders(CListT<tstr>& listOfRootFolders); */
  bool nppGetWorkspaceItemPath(tstr& itemPath);
  bool nppGetWorkspaceRootItemPath(tstr& rootItemPath);
  tstr nppGetSettingsCloudPath();
  tstr GetSettingsCloudPluginDir();
  int  findFileNameIndexInNppOpenFileNames(const tstr& fileName, bool bGetOpenFileNames, int nView = ALL_OPEN_FILES);

  static void CreateCloudDirIfNeeded(const tstr& cloudDir);

private:
  void SaveConfiguration();
  
public: 
  void InitPluginName(HMODULE hDllModule); // called _before_ Init()
  void Init();
  void Uninit();
  void OnCmdHistory();
  void OnDoExecDlg();
  void OnDirectExec(const tstr& id, bool bCanSaveAll, unsigned int nRunFlags = 0);
  void OnExecSelText();
  void OnExecClipText();
  void OnConsoleEncoding();
  void OnSaveOnExecute();
  void OnCdCurDir();
  void OnNoInternalMsgs();

#ifdef _DISABLE_CMD_ALIASES
  void OnNoCmdAliases();
#endif

  void OnOutputFilter();
  void OnAdvancedOptions();
  void OnSelectConsoleFont();
  void OnSelectExecDlgFont();
  void OnUserMenuItem(int nItemNumber);

  void DoExecScript(const tstr& id, LPCTSTR szScriptName, bool bCanSaveAll, LPCTSTR szScriptArguments = NULL, unsigned int nRunFlags = 0);
  void DoRunScript(const CListT<tstr>& CmdList, unsigned int nRunFlags = 0);
  void DoExecText(const tstr& sText, int nExecTextMode);

  void RunTheStartScript();
  void RunTheExitScript();

#ifdef _SCROLL_TO_LATEST  
  void OnScrollToLatest();
#endif

  void OnShowConsoleDlg();
  void OnToggleConsoleDlg();
  void OnGoToNextError();
  void OnGoToPrevError();
  void OnHelpManual();
  void OnHelpDocs();
  void OnHelpAbout();
  void ReadOptions();
  void SaveOptions();
  void StartAutoSaveTimer();
  void StopAutoSaveTimer();

  enum eSaveScriptsFlags {
      ssfSaveLastScript = 0x01
  };
  void SaveScripts(unsigned int nSaveFlags);
  
  HWND GetScintillaHandle();
  HMENU GetNppMainMenu();
  void SetConsoleFont(HWND hEd, const LOGFONT* plf);
  void ShowError(LPCTSTR szMessage);
  void ShowWarning(LPCTSTR szMessage);
  INT_PTR PluginDialogBox(UINT idDlg, DLGPROC lpDlgProc);
  CInputBoxDlg& GetInputBoxDlg();

  tstr ExpandToFullConfigPath(const TCHAR* cszFileName, bool bTryCloud = false);

  void setConsoleVisible(bool consoleIsVisible);

  enum eShowConsoleAction {
      showIfHidden,
      hideIfShown,
      hideAlways
  };
  enum eShowConsoleFlags {
      scfCmdNppConsole = ConsoleDlg::ccfCmdNppConsole
  };
  void showConsoleDialog(eShowConsoleAction showAction, unsigned int nShowFlags);
  
  bool initConsoleDialog();

  bool checkCmdListAndPrepareConsole(const CListT<tstr>& CmdList, bool bCanClearConsole = true);

  enum eDlgExistResult {
      dlgNotExist,
      dlgExistedAlready,
      dlgJustCreated
  };
  eDlgExistResult verifyConsoleDialogExists();
  bool isConsoleDialogVisible();
  void UpdateConsoleEncoding();
  void updateConsoleEncodingFlags();
  
  void printConsoleHelpInfo();

  void UpdateOutputFilterMenuItem();
  void UpdateGoToErrorMenuItem();

  LPCTSTR getPluginDllPath() const  { return m_szPluginDllPath; }
  LPCTSTR getConfigPath() const  { return m_szConfigPath; }

  static void printScriptLog(const TCHAR* str, int len);
  static void printScriptString(const TCHAR* str, int len);

  CNppExecConsole& GetConsole() { return m_Console; }
  CNppExecMacroVars& GetMacroVars() { return m_MacroVars; }
  CNppExecCommandExecutor& GetCommandExecutor() { return m_CommandExecutor; }
  CNppExecPluginInterfaceImpl& GetPluginInterfaceImpl() { return m_PluginInterfaceImpl; }

  LRESULT SendNppMsg(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0); // to Notepad++
};

namespace Runtime
{
    CNppExec& GetNppExec();
    CSimpleLogger& GetLogger();
};

extern TCHAR SCRIPTFILE_TEMP[100];
extern TCHAR SCRIPTFILE_LAST[100];
extern TCHAR SCRIPTFILE_SAVED[100];
extern TCHAR CMDHISTORY_FILENAME[100];


//--------------------------------------------------------------------
#endif
