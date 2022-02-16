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

#ifndef _npp_exec_plugin_interface_h_
#define _npp_exec_plugin_interface_h_
//---------------------------------------------------------------------------
#include "../base.h"
#include "../npp_files/Notepad_plus_msgs.h"
#include "../NppExecHelpers.h"
#include "nppexec_msgs.h"
#include <map>

class CNppExec;

class CNppExecPluginInterfaceImpl
{
    public:
        class CPluginMsg
        {
            public:
                enum eState {
                    stateWaiting = 0,
                    stateResultReady,
                    stateResultSent
                };

                // reflects the tPluginResultStruct
                struct ResultStruct {
                    DWORD             dwResult;
                    const TCHAR*      szID;

                    ResultStruct() : dwResult(0), szID(0)
                    {
                    }
                };

            public:
                tstr              SrcModuleName;
                tstr              ID;
                ResultStruct      Result;
                CommunicationInfo ci;

                CPluginMsg() { }
                CPluginMsg(const TCHAR* srcModuleName, const tstr& id) : SrcModuleName(srcModuleName), ID(id) { }

                static tstr GenerateUniqueId();
        };
    
    protected:
        typedef std::map<tstr, CPluginMsg> PluginMsgsType;

        class AsyncCmd
        {
            public:
                enum eType {
                    cmdCollateralScript,
                    cmdQueuedScript
                };
            public:
                AsyncCmd(const tstr& id, const tstr& scriptBody, eType type)
                  : m_id(id)
                  , m_scriptBody(scriptBody)
                  , m_type(type)
                {
                }

                const tstr& GetId() const { return m_id; }
                const tstr& GetScriptBody() const { return m_scriptBody; }
                eType GetType() const { return m_type; }
            
            protected:
                tstr  m_id;
                tstr  m_scriptBody;
                eType m_type;
        };

    public:
        CNppExecPluginInterfaceImpl();
        ~CNppExecPluginInterfaceImpl();

        void Enable(bool bEnable);

        CNppExec* GetNppExec() const { return m_pNppExec; }
        void SetNppExec(CNppExec* pNppExec) { m_pNppExec = pNppExec; }

        void SetPluginName(const TCHAR* szNppExecDll);

        void ProcessExternalPluginMsg(long nMsg, const TCHAR* srcModuleName, void* pInfo);
        void NotifyExternalPluginResult(const tstr& id, DWORD dwResultCode);

    protected:
        void registerAsyncCmd(const tstr& id, const TCHAR* szScriptBody, AsyncCmd::eType type);
        void registerPluginMsg(const CPluginMsg& msg);

        static DWORD WINAPI BackgroundExecAsyncCmdThreadFunc(LPVOID lpParam);
        static DWORD WINAPI BackgroundSendMsgThreadFunc(LPVOID lpParam);

        void initExecAsyncCmdThread();
        void stopExecAsyncCmdThread();

        void initSendMsgThread();
        void stopSendMsgThread();

        DWORD  npemGetState() const;
        void   npemExecuteScript(const tstr& id, const TCHAR* szScriptBody);
        void   npemExecuteCollateralScript(const tstr& id, const TCHAR* szScriptBody);
        void   npemExecuteQueuedScript(const tstr& id, const TCHAR* szScriptBody);
        void   npemPrint(const TCHAR* szText);
        void   npemFreePtr(TCHAR* p);
        TCHAR* npemGetScriptNames();
        TCHAR* npemGetScriptByName(const TCHAR* szScriptName);

        void conPrintLine(tstr& Line);

    public:
        static void addCommand(CListT<tstr>& CmdList, tstr& Cmd);
        static CListT<tstr> getCmdListFromScriptBody(const TCHAR* szScriptBody);

    protected:
        CNppExec* m_pNppExec;
        HANDLE m_hBackgroundExecAsyncCmdThread;
        HANDLE m_hBackgroundSendMsgThread;
        CEvent m_StopEvent; // used for both the ExecAsyncCmd and SendMsg
        CEvent m_ExecAsyncCmdEvent;
        CEvent m_ExecAsyncCmdThreadDoneEvent;
        CEvent m_SendMsgEvent;
        CEvent m_SendMsgThreadDoneEvent;
        PluginMsgsType m_PluginMsgs;
        CListT<AsyncCmd> m_ExecAsyncCmdQueue;
        CListT<tstr> m_SendMsgQueue;
        CCriticalSection m_csExecAsyncCmds;
        CCriticalSection m_csPluginMsgs;
        tstr m_sNppExecDll;
        bool m_isEnabled;
};

//---------------------------------------------------------------------------
#endif
