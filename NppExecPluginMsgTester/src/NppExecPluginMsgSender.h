#ifndef _npp_exec_plugin_msg_sender_h_
#define _npp_exec_plugin_msg_sender_h_
//---------------------------------------------------------------------------
#include "core/NppPluginMsgSender.h"
#include "nppexec_msgs.h"

class CNppExecPluginMsgSender : public CNppPluginMsgSender
{
    protected:
        std::basic_string<TCHAR> m_destModuleName;

    public:
        CNppExecPluginMsgSender(HWND hNppWnd, const TCHAR* srcModuleName, 
          const TCHAR* destModuleName = _T("NppExec.dll")) : 
            CNppPluginMsgSender(hNppWnd, srcModuleName),
            m_destModuleName(destModuleName)
        {
        }

        void setDestModuleName(const TCHAR* destModuleName)
        {
            m_destModuleName = destModuleName;
        }

        BOOL SendNpeMsg(long internalMsg, void* info)
        {
            return SendMsg( m_destModuleName.c_str(), internalMsg, info );
        }

        DWORD NpeGetVerDword()
        {
            DWORD dwVersion = 0;
            SendNpeMsg( NPEM_GETVERDWORD, (void *) &dwVersion );
            return dwVersion;
        }

        BOOL NpeGetVerStr(TCHAR pszVersion[NPE_MAXVERSTR])
        {
            pszVersion[0] = 0;
            return SendNpeMsg( NPEM_GETVERSTR, (void *) pszVersion );
        }

        DWORD NpeGetState()
        {
            DWORD dwState = 0;
            SendNpeMsg( NPEM_GETSTATE, (void *) &dwState );
            return dwState;
        }

        DWORD NpePrint(const TCHAR* szText)
        {
            DWORD dwState = NpeGetState();
            if ( dwState == NPE_STATEREADY )
            {
                SendNpeMsg( NPEM_PRINT, (void *) szText );
            }
            return dwState;
        }

        DWORD NpeExecute(NpeExecuteParam* nep)
        {
            DWORD dwState = NpeGetState();
            if ( dwState == NPE_STATEREADY )
            {
                SendNpeMsg( NPEM_EXECUTE, (void *) nep );
                return nep->dwResult;
            }
            return dwState;
        }

        DWORD NpeNppExec(NpeNppExecParam* npep) 
        {
            DWORD dwState = NpeGetState();
            if ( dwState == NPE_STATEREADY )
            {
                SendNpeMsg( NPEM_NPPEXEC, (void *) npep );
                return npep->dwResult;
            }
            return dwState;
        }

        DWORD NpeExecuteCollateral(NpeExecuteCollateralParam* necp)
        {
            SendNpeMsg( NPEM_EXECUTE_COLLATERAL, (void *) necp );
            return necp->dwResult;
        }

        DWORD NpeExecuteQueued(NpeExecuteQueuedParam* neqp)
        {
            SendNpeMsg( NPEM_EXECUTE_QUEUED, (void *) neqp );
            return neqp->dwResult;
        }

        void NpeFreePtr(void* ptr)
        {
            SendNpeMsg( NPEM_FREEPTR, ptr );
        }

        DWORD NpeGetScriptNames(NpeGetScriptNamesParam* nsn)
        {
            SendNpeMsg( NPEM_GETSCRIPTNAMES, (void *) nsn );
            return nsn->dwResult;
        }

        DWORD NpeGetScriptByName(NpeGetScriptByNameParam* nsn)
        {
            SendNpeMsg( NPEM_GETSCRIPTBYNAME, (void *) nsn );
            return nsn->dwResult;
        }

};

//---------------------------------------------------------------------------
#endif
