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

#include "NppScript.h"

CNppScript::CNppScript()
{
}

CNppScript::CNppScript(const CNppScript& nppScript)
  : m_ScriptName(nppScript.m_ScriptName), m_CmdList(nppScript.m_CmdList)
{
}

CNppScript::CNppScript(const tstr& scriptName)
  : m_ScriptName(scriptName)
{
}

CNppScript::CNppScript(const tstr& scriptName, const tCmdList& cmdList)
  : m_ScriptName(scriptName), m_CmdList(cmdList)
{
}

CNppScript::CNppScript(const tstr& scriptName, const tstr& singleCmd)
  : m_ScriptName(scriptName), m_CmdList(singleCmd)
{
}

CNppScript::~CNppScript()
{
}

const tstr& CNppScript::GetScriptName() const
{
    return m_ScriptName;
}

const tCmdList& CNppScript::GetCmdList() const
{
    return m_CmdList;
}

tCmdList& CNppScript::GetCmdList()
{
    return m_CmdList;
}

tstr CNppScript::SerializeToString() const
{
    tstr serializedContent;

    serializedContent.Reserve(GetSerializedStringLength());

    serializedContent.Append( _T("::"), 2 );
    serializedContent.Append( m_ScriptName );
    serializedContent.Append( _T("\r\n"), 2 );

    for ( auto pline = m_CmdList.GetFirst(); pline != NULL; pline = pline->GetNext() )
    {
        serializedContent.Append( pline->GetItem() );
        serializedContent.Append( _T("\r\n"), 2 );
    }

    return serializedContent;
}

int CNppScript::GetSerializedStringLength() const
{
    int serializedLength = 0;

    serializedLength += m_ScriptName.length();
    serializedLength += 4; // "::" and "\r\n"

    for ( auto pline = m_CmdList.GetFirst(); pline != NULL; pline = pline->GetNext() )
    {
        serializedLength += pline->GetItem().length();
        serializedLength += 2; // "\r\n"
    }

    return serializedLength;
}
