#include "NppPluginMenu.h"

CNppMessager CNppPluginMenu::m_nppMsgr;

CNppPluginMenu::CNppPluginMenu()
{
}

CNppPluginMenu::~CNppPluginMenu()
{
}

void CNppPluginMenu::setNppData(const NppData& nppd)
{
    m_nppMsgr.setNppData(nppd);
}
