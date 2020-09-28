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

#ifndef _npp_script_h_
#define _npp_script_h_
//--------------------------------------------------------------------
#include "base.h"
#include "cpp/CStrT.h"
#include "cpp/CBufT.h"
#include "cpp/CListT.h"

typedef CStrT<TCHAR> tstr;
typedef CListT<tstr> tCmdList;

class CNppScript
{
private:
    tstr         m_ScriptName;
    CListT<tstr> m_CmdList;

    template<class Dst> void serializeByAppendingTo(Dst& dest) const
    {
        dest.Append( _T("::"), 2 );
        dest.Append( m_ScriptName.c_str(), m_ScriptName.length() );
        dest.Append( _T("\r\n"), 2 );

        for ( auto pline = m_CmdList.GetFirst(); pline != NULL; pline = pline->GetNext() )
        {
            const tstr& line = pline->GetItem();
            dest.Append( line.c_str(), line.length() );
            dest.Append( _T("\r\n"), 2 );
        }
    }

public:
    enum eSerializeBufFlags : unsigned int {
        sbfReserveMemory = 0x01,  // calls Buf.Reserve()
        sbfAppendMode    = 0x02   // does not call Buf.Clear()
    };

    CNppScript();
    CNppScript(const CNppScript& nppScript);
    CNppScript(const tstr& scriptName);
    CNppScript(const tstr& scriptName, const tCmdList& cmdList);
    CNppScript(const tstr& scriptName, const tstr& singleCmd);
    ~CNppScript();

    const tstr& GetScriptName() const;
    const tCmdList& GetCmdList() const;
    tCmdList& GetCmdList();

    void SerializeToBuf(CBufT<TCHAR>& Buf, unsigned int flags = sbfReserveMemory) const;
    tstr SerializeToString() const;
    int GetSerializedStringLength() const;
};

//--------------------------------------------------------------------
#endif

