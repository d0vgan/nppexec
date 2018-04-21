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

#ifndef _npp_exec_msgs_h_
#define _npp_exec_msgs_h_
//---------------------------------------------------------------------------
#include <windows.h>

/*

Reference to "Notepad_plus_msgs.h":
 
  #define NPPM_MSGTOPLUGIN (NPPMSG + 47)
  //BOOL NPPM_MSGTOPLUGIN(TCHAR *destModuleName, CommunicationInfo *info)
  // return value is TRUE when the message arrive to the destination plugins.
  // if destModule or info is NULL, then return value is FALSE
  struct CommunicationInfo {
    long internalMsg;
    const TCHAR * srcModuleName;
    void * info; // defined by plugin
  };
 
The following messages can be used as internalMsg parameter.

 */


typedef struct sPluginResultStruct {
    DWORD        dwResult; // this _must_ be the first item for backward compatibility with NppExec's pre-v0.6 plugin interface
    const TCHAR* szID;     // the ID of the original message
} tPluginResultStruct;


#define  NPEM_GETVERDWORD       0x0201  // message
  /*
  Returns plugin's version as DWORD e.g. 0x02B4:
    0x03A1 - means "0.3 alpha 1" (A - Alpha).
    0x02B4 - means "0.2 beta 4" (B - Beta);
    0x02C1 - means "0.2 RC1" (C - release Candidate);
    0x02F0 - means "0.2 final" (F - Final);
    0x02F1 - means "0.2.1" (patched Final version);

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  DWORD dwVersion = 0;
  CommunicationInfo ci = { NPEM_GETVERDWORD, 
                           cszMyPlugin, 
                           (void *) &dwVersion };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  Possible results:
  1) dwVersion remains 0 - the plugin is not available or does not support
     external messages;
  2) dwVersion >= 0x02F5 - the plugin is available and other messages can
     be used (such as NPEM_GETSTATE, NPEM_PRINT, NPEM_EXECUTE, NPEM_NPPEXEC);
  3) dwVersion >= 0x06A0 - more messages are available (such as NPEM_EXECUTE2,
     NPEM_NPPEXEC2, NPEM_EXECUTE_COLLATERAL, NPEM_EXECUTE_QUEUED).

  if ( dwVersion >= 0x02F5 )
  {
     // other messages are accessible
     // (check for 0x06A0 to ensure all the latest messages
     // such as NPEM_EXECUTE_QUEUED are accessible)
  }
  else
  {
    // plugin is not available or does not support external messages
  }
  */


#define  NPEM_GETVERSTR         0x0202  // message (NppExec version >= 0x02F5)
  #define  NPE_MAXVERSTR            32
  /*
  Returns plugin's version as string e.g. "0.2 beta 4".

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  TCHAR szVersion[NPE_MAXVERSTR] = { 0 };
  CommunicationInfo ci = { NPEM_GETVERSTR, 
                           cszMyPlugin, 
                           (void *) szVersion };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );
  */


#define  NPEM_GETSTATE          0x0301  // message (NppExec version >= 0x02F5)
  #define  NPE_STATEDISABLED      0x00
  #define  NPE_STATEREADY         0x01
  #define  NPE_STATEBUSY          0x10
  #define  NPE_STATECHILDPROC     (NPE_STATEBUSY | 0x20)
  /*
  Returns plugin's state as DWORD:
    NPE_STATEREADY     - the plugin is "ready";
    NPE_STATEBUSY      - the plugin is "busy" (another script is being executed);
    NPE_STATECHILDPROC - the plugin is "busy" (a child process is being run).

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  DWORD dwState = 0;
  CommunicationInfo ci = { NPEM_GETSTATE, 
                           cszMyPlugin, 
                           (void *) &dwState };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( dwState == NPE_STATEREADY )
  {
    // the plugin is "ready"
  }
  else if ( dwState & NPE_STATEBUSY )
  {
    // the plugin is "busy"
  }
  else
  {
    // unknown state: maybe the plugin does not support external messages
  }
  */


#define  NPEM_PRINT             0x0401  // message (NppExec version >= 0x02F5)
  /*
  Prints (shows) the given text in NppExec's Console window. 
  You can separate the text lines using _T('\n') or _T("\r\n").
  This text can be highlighted if NppExec's Console Highlight Filters are used.

  If plugin's state is "busy", this message is ignored.

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  DWORD dwState = 0;
  CommunicationInfo ci = { NPEM_GETSTATE, 
                           cszMyPlugin, 
                           (void *) &dwState };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( dwState == NPE_STATEREADY )
  {
    // the plugin is "ready"
    const TCHAR* szText = _T("Hello from my plugin!\n(test message)")
    CommunicationInfo ci = { NPEM_PRINT,
                             cszMyPlugin, 
                             (void *) szText };
    ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
        (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );
  }
  */


#define  NPEM_EXECUTE           0x0101  // message (NppExec version >= 0x02F5)
  #define  NPE_EXECUTE_OK       NPE_STATEREADY
  #define  NPE_EXECUTE_FAILED   NPE_STATEBUSY
  typedef struct sNpeExecuteParam {
      const TCHAR* szScriptBody; // text of the script (i.e. set of commands)
      DWORD        dwResult;     // [out] NPE_EXECUTE_OK - OK; otherwise failed
  } NpeExecuteParam;
  /*
  Executes the given commands. 
  
  If plugin's state is "busy", then nothing happens (the message is ignored).
  If szScriptBody is NULL or empty, then nothing happens also.
  In this case [out] dwResult will contain NPE_EXECUTE_FAILED.

  !!! NOTE !!!
  Set [in] dwResult to 1 if you want NppExec to send a message (notification) 
  to your plugin after the script is executed.
  Otherwise set [in] dwResult to 0.

  !!! NOTE !!!
  The szScriptBody parameter can contain several text lines (commands).
  You can separate text lines using _T('\n') or _T("\r\n").

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  DWORD dwState = 0;
  CommunicationInfo ci = { NPEM_GETSTATE, 
                           cszMyPlugin, 
                           (void *) &dwState };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( dwState == NPE_STATEREADY )
  {
    // the plugin is "ready"
    NpeExecuteParam nep;
    nep.szScriptBody = _T("cls \n npp_open *.txt \n npp_switch $(#2)");
    nep.dwResult = 0; // don't send a message (notification) after the script is executed
    CommunicationInfo ci = { NPEM_EXECUTE, 
                             cszMyPlugin, 
                             (void *) &nep };
    ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
        (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

    if ( nep.dwResult == NPE_EXECUTE_OK )
    {
      // OK, the script is being executed
    }
    else
    {
      // failed, the plugin is "busy"
    }
  }
  else
  {
    // the plugin is "busy", NPEM_EXECUTE is not possible
  }
  */


#define  NPEM_NPPEXEC           0x0102  // message (NppExec version >= 0x02F5)
  #define  NPE_NPPEXEC_OK       NPE_STATEREADY
  #define  NPE_NPPEXEC_FAILED   NPE_STATEBUSY
  typedef struct sNpeNppExecParam {
      const TCHAR* szScriptName;      // name of existing script/file
      const TCHAR* szScriptArguments; // arguments, can be NULL (i.e. none)
      DWORD        dwResult;          // [out] NPE_NPPEXEC_OK - OK; otherwise failed
  } NpeNppExecParam;
  /*
  This makes NppExec to do either (1.a + 1.b) or (2.a + 2.b) described below:

  1.a. Substitutes given arguments instead of $(ARGV[n]) inside the script;
  1.b. Executes the specified script (internal NppExec's script, if it exists).

  2.a. Substitutes given arguments instead of $(ARGV[n]) inside the script;
  2.b. Executes commands from the specified file.

  If plugin's state is "busy", then nothing happens (the message is ignored).
  If szScriptName is NULL or empty, then nothing happens also.
  In this case [out] dwResult will contain NPE_NPPEXEC_FAILED.

  !!! NOTE !!!
  Set [in] dwResult to 1 if you want NppExec to send a message (notification) 
  to your plugin after the script is executed.
  Otherwise set [in] dwResult to 0.

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  DWORD dwState = 0;
  CommunicationInfo ci = { NPEM_GETSTATE, 
                           cszMyPlugin, 
                           (void *) &dwState };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( dwState == NPE_STATEREADY )
  {
    // the plugin is "ready"
    NpeNppExecParam npep;
    npep.szScriptName = _T("C:\\Program Files\\My NppExec Scripts\\test script.txt");
    npep.szScriptArguments = _T("\"arg 1\" \"arg 2\""); // [1] = "arg 1", [2] = "arg 2"
    npep.dwResult = 1; // send a message (notification) after the script is executed
    CommunicationInfo ci = { NPEM_EXECUTE, 
                             cszMyPlugin, 
                             (void *) &npep };
    ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
        (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

    if ( npep.dwResult == NPE_NPPEXEC_OK )
    {
      // OK, the script is being executed
      // you'll get NPEN_RESULT when the script is finished
    }
    else
    {
      // failed, the plugin is "busy"
    }
  }
  else
  {
    // the plugin is "busy", NPEM_EXECUTE is not possible
  }
  */

#define  NPEM_EXECUTE2           0x0151  // message (NppExec version >= 0x06A0)
  #define  NPE_EXECUTE2_OK       NPE_STATEREADY
  #define  NPE_EXECUTE2_FAILED   NPE_STATEBUSY
  typedef struct sNpeExecuteParam2 {
      const TCHAR* szScriptBody; // text of the script (i.e. set of commands)
      const TCHAR* szID;         // the ID of this message (e.g. to be verified on NPEN_RESULT)
      DWORD        dwResult;     // [out] NPE_EXECUTE_OK - OK; otherwise failed
  } NpeExecuteParam2;
  /*
  The same as NPEM_EXECUTE, but with the additional parameter szID.

  If plugin's state is "busy", then nothing happens (the message is ignored).
  If szScriptBody is NULL or empty, then nothing happens also.
  In this case [out] dwResult will contain NPE_EXECUTE2_FAILED.

  !!! NOTE !!!
  Set [in] dwResult to 1 if you want NppExec to send a message (notification) 
  to your plugin after the script is executed. The szID parameter will be
  sent (duplicated) via NPEN_RESULT in this case. (If szID is NULL or empty, 
  then NPEN_RESULT will _not_ be sent.)
  Otherwise set [in] dwResult to 0.
  */

#define  NPEM_NPPEXEC2           0x0152  // message (NppExec version >= 0x06A0)
  #define  NPE_NPPEXEC2_OK       NPE_STATEREADY
  #define  NPE_NPPEXEC2_FAILED   NPE_STATEBUSY
  typedef struct sNpeNppExecParam2 {
      const TCHAR* szScriptName;      // name of existing script/file
      const TCHAR* szScriptArguments; // arguments, can be NULL (i.e. none)
      const TCHAR* szID;              // the ID of this message (e.g. to be verified on NPEN_RESULT)
      DWORD        dwResult;          // [out] NPE_NPPEXEC_OK - OK; otherwise failed
  } NpeNppExecParam2;
  /*
  The same as NPEM_NPPEXEC, but with the additional parameter szID.

  If plugin's state is "busy", then nothing happens (the message is ignored).
  If szScriptName is NULL or empty, then nothing happens also.
  In this case [out] dwResult will contain NPE_NPPEXEC2_FAILED.

  !!! NOTE !!!
  Set [in] dwResult to 1 if you want NppExec to send a message (notification) 
  to your plugin after the script is executed. The szID parameter will be
  sent (duplicated) via NPEN_RESULT in this case. (If szID is NULL or empty, 
  then NPEN_RESULT will _not_ be sent.)
  Otherwise set [in] dwResult to 0.
  */

#define  NPEM_EXECUTE_COLLATERAL           0x0153  // message (NppExec version >= 0x06A0)
  #define  NPE_EXECUTE_COLLATERAL_OK       NPE_STATEREADY
  #define  NPE_EXECUTE_COLLATERAL_FAILED   NPE_STATEBUSY
  typedef struct sNpeExecuteCollateralParam {
      const TCHAR* szScriptBody; // text of the script (i.e. set of commands)
      const TCHAR* szID;         // the ID of this message (e.g. to be verified on NPEN_RESULT)
      DWORD        dwResult;     // [out] NPE_EXECUTE_OK - OK; otherwise failed
  } NpeExecuteCollateralParam;
  /*
  Executes the given commands collaterally. 
  "Collaterally" means the following: if there is an already running script in NppExec,
  the "collateral" one becomes a "child" of that running script, making the running
  script to be paused until the "collateral" one is done.
  This applies to an already running script as well as to a child process being run.
  
  This message does not depend on the plugin's state.
  If szScriptName is NULL or empty, then nothing happens (the message is ignored).
  In this case [out] dwResult will contain NPE_EXECUTE_COLLATERAL_FAILED.

  !!! NOTE !!!
  Set [in] dwResult to 1 if you want NppExec to send a message (notification) 
  to your plugin after the script is executed. The szID parameter will be
  sent (duplicated) via NPEN_RESULT in this case. (If szID is NULL or empty, 
  then NPEN_RESULT will _not_ be sent.)
  Otherwise set [in] dwResult to 0.

  !!! NOTE !!!
  The szScriptBody parameter can contain several text lines (commands).
  You can separate text lines using _T('\n') or _T("\r\n").

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  NpeExecuteCollateralParam necp;
  necp.szScriptBody = _T("cmd /c time /t");
  necp.szID = GenerateUniqueMessageId(); // any unique string
  necp.dwResult = 1; // send a message (notification) after the script is executed
  CommunicationInfo ci = { NPEM_EXECUTE_COLLATERAL, 
                           cszMyPlugin, 
                           (void *) &necp };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( necp.dwResult == NPE_EXECUTE_COLLATERAL_OK )
  {
    // OK, the script is being executed
    // you'll get NPEN_RESULT when the script is finished
  }
  else
  {
    // failed, maybe internal error in NppExec
  }
  */

#define  NPEM_EXECUTE_QUEUED           0x0154  // message (NppExec version >= 0x06A0)
  #define  NPE_EXECUTE_QUEUED_OK       NPE_STATEREADY
  #define  NPE_EXECUTE_QUEUED_FAILED   NPE_STATEBUSY
  typedef struct sNpeExecuteQueuedParam {
      const TCHAR* szScriptBody; // text of the script (i.e. set of commands)
      const TCHAR* szID;         // the ID of this message (e.g. to be verified on NPEN_RESULT)
      DWORD        dwResult;     // [out] NPE_EXECUTE_OK - OK; otherwise failed
  } NpeExecuteQueuedParam;
  /*
  Queues the given commands to be executed. 
  If NppExec is not "busy", it has the very same effect as NPEM_EXECUTE2.
  If NppExec is "busy", the given script body (set of commands) is queued and
  will be executed once NppExec is not "busy". NPEM_EXECUTE_QUEUED can be called
  again and again, resulting in queueing the scripts to be executed. The queued
  scripts will be executed in the same order they were queued.
  
  This message does not depend on the plugin's state.
  If szScriptName is NULL or empty, then nothing happens (the message is ignored).
  In this case [out] dwResult will contain NPE_EXECUTE_QUEUED_FAILED.

  !!! NOTE !!!
  Set [in] dwResult to 1 if you want NppExec to send a message (notification) 
  to your plugin after the script is executed. The szID parameter will be
  sent (duplicated) via NPEN_RESULT in this case. (If szID is NULL or empty, 
  then NPEN_RESULT will _not_ be sent.)
  Otherwise set [in] dwResult to 0.

  !!! NOTE !!!
  The szScriptBody parameter can contain several text lines (commands).
  You can separate text lines using _T('\n') or _T("\r\n").

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  NpeExecuteQueuedParam neqp;
  neqp.szScriptBody = _T("set A = 10");
  neqp.szID = GenerateUniqueMessageId(); // any unique string
  neqp.dwResult = 1; // send a message (notification) after the script is executed
  CommunicationInfo ci = { NPEM_EXECUTE_QUEUED, 
                           cszMyPlugin, 
                           (void *) &neqp };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( neqp.dwResult == NPE_EXECUTE_QUEUED_OK )
  {
    // OK, the script is queued
    // you'll get NPEN_RESULT when the script is finished
  }
  else
  {
    // failed, maybe internal error in NppExec
  }

  // queueing another message now:
  neqp.szScriptBody = _T("set B ~ $(A) + 10");
  neqp.szID = GenerateUniqueMessageId(); // another unique string
  neqp.dwResult = 1; // send a message (notification) after the script is executed
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );
  if ( neqp.dwResult == NPE_EXECUTE_QUEUED_OK )
  {
    // OK, the script is queued
    // you'll get NPEN_RESULT when the script is finished
  }
  else
  {
    // failed, maybe internal error in NppExec
  }
  */


#define  NPEN_RESULT           0x0109  // notification (NppExec version >= 0x02F5)
  #define  NPE_RESULT_OK       NPE_STATEREADY
  #define  NPE_RESULT_FAILED   NPE_STATEBUSY
  #define  NPE_RESULT_ABORTED  0x05
  /*
  This notification is sent to your plugin as a result of complete execution 
  of previous message (such as NPEM_EXECUTE or NPEM_NPPEXEC), if the parameter
  dwResult of that message was set to 1.

  Parameters of the CommunicationInfo struct:
  {
    internalMsg = NPEN_RESULT,
    srcModuleName = _T("NppExec.dll"), 
    info = (tPluginResultStruct *) pResult // pResult->dwResult contains NPE_RESULT_OK or NPE_RESULT_FAILED or NPE_RESULT_ABORTED
                                           // pResult->szID contains the original message ID
  };

  !!! NOTE !!! 
  NppExec dll name may be different, so srcModuleName may contain different
  string (e.g. "NppExec1.dll" or "MyNppExec.dll" according to the dll file name).

  Example:

  extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
  {
    ...
    if ( Message == NPPM_MSGTOPLUGIN )
    {
      CommunicationInfo* pci = (CommunicationInfo *) lParam;
      if ( pci )
      {
        if ( pci->internalMsg == NPEN_RESULT )
        { 
          // NPEN_RESULT notification...
          if ( lstrcmpi(pci->srcModuleName, szNppExecModuleName) == 0 )
          {
            // ...from NppExec plugin
            tPluginResultStruct* pResult = ((tPluginResultStruct *) pci->info); // NppExec version >= 0x06A0
            const TCHAR* cszMessageID = pResult->szID; // e.g. to verify this ID
            if ( pResult->dwResult == NPE_RESULT_OK )
            {
              // OK, the script has been executed
            }
            else if ( pResult->dwResult == NPE_RESULT_ABORTED )
            {
              // script was aborted (e.g. because the NppExec's Console was closed)
            }
            else
            {
              // failed, maybe internal error in NppExec
            }

            // ... or ...

            // the same works as well, when the message ID is not necessary:
            DWORD dwResult = *((DWORD *) pci->info); // NppExec version >= 0x02F5
            if ( dwResult == NPE_RESULT_OK )
            {
              // OK, the script has been executed
            }
            else
            {
              // failed, maybe internal error in NppExec
            }
          }
        }
      }
    }
    ...
  }
  */


#define  NPEM_FREEPTR                   0x0500  // message (NppExec version >= 0x06C1)
  /*
  Frees the memory previously allocated by NppExec
  (e.g. after NPEM_GETSCRIPTNAMES or NPEM_GETSCRIPTBYNAME).

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  NpeGetScriptNamesParam nsn;
  memset(&nsn, 0, sizeof(NpeGetScriptNamesParam));
  CommunicationInfo ci = { NPEM_GETSCRIPTNAMES, 
                           cszMyPlugin, 
                           (void *) &nsn };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( nsn.dwResult == NPE_GETSCRIPTNAMES_OK )
  {
      // ... using nsn.pScriptNames here ...

      // ... finally:
      CommunicationInfo ci2 = { NPEM_FREEPTR, 
                                cszMyPlugin, 
                                (void *) nsn.pScriptNames };
      ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
          (WPARAM) _T("NppExec.dll"), (LPARAM) &ci2 );
  }
  */


#define  NPEM_GETSCRIPTNAMES            0x0501  // message (NppExec version >= 0x06C1)
  #define  NPE_GETSCRIPTNAMES_OK        NPE_STATEREADY
  #define  NPE_GETSCRIPTNAMES_FAILED    NPE_STATEBUSY
  typedef struct sNpeGetScriptNamesParam {
      TCHAR* pScriptNames; // [out] returned pointer to script names
      DWORD  dwResult;     // [out] NPE_EXECUTE_OK - OK; otherwise failed
  } NpeGetScriptNamesParam;
  /*
  Returns a pointer (string) containing the script names.
  The script names are separated by '\n'.
  As pScriptNames is a pointer to a memory block allocated
  by NppExec, finally it *must* be freed via NPEM_FREEPTR.

  Example:

  const TCHAR* cszMyPlugin = _T("my_plugin");
  NpeGetScriptNamesParam nsn;
  memset(&nsn, 0, sizeof(NpeGetScriptNamesParam));
  CommunicationInfo ci = { NPEM_GETSCRIPTNAMES, 
                           cszMyPlugin, 
                           (void *) &nsn };
  ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
      (WPARAM) _T("NppExec.dll"), (LPARAM) &ci );

  if ( nsn.dwResult == NPE_GETSCRIPTNAMES_OK )
  {
      // OK, let's get the script names
      std::list< std::basic_string<TCHAR> > scriptNames;
      {
          std::basic_string<TCHAR> scriptName;
          const TCHAR* pszNames = nsn.pScriptNames;
          TCHAR ch;
          while ( (ch = *pszNames) != 0 )
          {
              if ( ch == _T('\n') )
              {
                  scriptNames.push_back(scriptName);
                  scriptName.clear();
              }
              else
              {
                  scriptName += ch;
              }
              ++pszNames;
          }
          if ( !scriptName.empty() )
          {
              scriptNames.push_back(scriptName);
          }
      }

      // free the memory allocated by NppExec
      CommunicationInfo ci2 = { NPEM_FREEPTR, 
                                cszMyPlugin, 
                                (void *) nsn.pScriptNames };
      ::SendMessage( hNppWnd, NPPM_MSGTOPLUGIN,
          (WPARAM) _T("NppExec.dll"), (LPARAM) &ci2 );

      // do something with scriptNames now...
  }
  else
  {
      // failed, the plugin is "busy"
  }
  */


#define NPEM_GETSCRIPTBYNAME            0x0502  // message (NppExec version >= 0x06C1)
  #define  NPE_GETSCRIPTBYNAME_OK       NPE_STATEREADY
  #define  NPE_GETSCRIPTBYNAME_FAILED   NPE_STATEBUSY
  typedef struct sNpeGetScriptByNameParam {
      const TCHAR* szScriptName;  // script name
      TCHAR*       pScriptBody;   // [out] returned pointer to the script body
      DWORD        dwResult;      // [out] NPE_EXECUTE_OK - OK; otherwise failed
  } NpeGetScriptByNameParam;
  /*
  Returns a pointer (string) containing the script body.
  The lines of the script body are separated by '\n'.
  As pScriptBody is a pointer to a memory block allocated
  by NppExec, finally it *must* be freed via NPEM_FREEPTR.

  ... TODO ...
  */


#define  NPEM_INTERNAL         0xFF01  // internal messages, don't use it!
#define  NPEM_SUSPENDEDACTION  (NPEM_INTERNAL + 1) // <-- obsolete

//---------------------------------------------------------------------------
#endif
