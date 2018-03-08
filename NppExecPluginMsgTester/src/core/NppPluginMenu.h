#ifndef _npp_plugin_menu_h_
#define _npp_plugin_menu_h_
//----------------------------------------------------------------------------
#include "base.h"
#include "SciMessager.h"
#include "NppMessager.h"


class CNppPluginMenu
{
    protected:
        static void funcEmpty() { }
        
    protected:
        static CNppMessager m_nppMsgr;

    public:
        CNppPluginMenu();
        virtual ~CNppPluginMenu();
        
        void setNppData(const NppData& nppd);
};

//----------------------------------------------------------------------------
#endif
