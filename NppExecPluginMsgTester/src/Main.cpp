// Main.cpp : Defines the entry point for the DLL application.
//

#include "core/base.h"
#include "MsgTester.h"


extern "C" BOOL APIENTRY DllMain( 
                        HINSTANCE hInstance, 
                        DWORD     dwReason, 
                        LPVOID    lpReserved
					 )
{
    switch ( dwReason )
    {
        case DLL_PROCESS_ATTACH:
            GetMsgTester().OnDllProcessAttach(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            GetMsgTester().OnDllProcessDetach();
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

extern "C" __declspec(dllexport) void setInfo(NppData nppd)
{
    GetMsgTester().nppSetInfo(nppd);
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
    return GetMsgTester().nppGetName();
}

extern "C" __declspec(dllexport) void beNotified(SCNotification* pscn)
{
    GetMsgTester().nppBeNotified(pscn);
}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
    return GetMsgTester().nppMessageProc(Message, wParam, lParam);
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int* pnbFuncItems)
{
    return GetMsgTester().nppGetFuncsArray(pnbFuncItems);
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif
