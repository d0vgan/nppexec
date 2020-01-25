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

#include "NppScriptList.h"
#include "NppExecHelpers.h"

CNppScriptList::CNppScriptList()
{
  _bIsModified = false;
}

CNppScriptList::~CNppScriptList()
{
  Free();
}

void CNppScriptList::Free()
{
  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    CNppScript* pScript = p->GetItem();
    delete pScript;
    p->SetItem(NULL);
    p = p->GetNext();
  }
}

bool CNppScriptList::AddScript(const tstr& ScriptName, const CNppScript& newScript)
{
  if (!ModifyScript(ScriptName, newScript))
  {
    CCriticalSectionLockGuard lock(_csScripts);

    CNppScript* pScript = new CNppScript(ScriptName, newScript.GetCmdList());
    if (pScript)
    {      
      if (_Scripts.Add(pScript))
      {
        // script is added -> list is modified
        _bIsModified = true;
        return true;
      }
      else
      {
        delete pScript;
        return false;
      }
    }
    else
    {
      return false;
    }
  }
  else
  {
    // ModifyScript() modifies _bIsModified here
    return true;
  }
}

bool CNppScriptList::DeleteScript(const tstr& ScriptName)
{
  bool bRet = false;

  CCriticalSectionLockGuard lock(_csScripts);

  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    CNppScript* pScript = p->GetItem();
    if (pScript->GetScriptName() == ScriptName)
    {
      delete pScript;
      _Scripts.Delete(p);

      // script is deleted -> list is modified
      _bIsModified = true;
      bRet = true;
      p = NULL; // break condition
    }
    else
      p = p->GetNext();
  }

  return bRet;
}

bool CNppScriptList::GetScript(const tstr& ScriptName, CNppScript& outScript)
{
  bool bRet = false;

  outScript.GetCmdList().DeleteAll();

  CCriticalSectionLockGuard lock(_csScripts);

  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    const CNppScript* pScript = p->GetItem();
    if (pScript->GetScriptName() == ScriptName)
    {
      outScript = *pScript;
      bRet = true;
      p = NULL; // break condition
    }
    else
      p = p->GetNext();
  }

  return bRet;
}

int CNppScriptList::GetScriptCount() const
{
  CCriticalSectionLockGuard lock(_csScripts);

  return _Scripts.GetCount(); 
}

CListT<tstr> CNppScriptList::GetScriptNames() const
{
  CCriticalSectionLockGuard lock(_csScripts);

  CListT<tstr> scriptNames;
  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    const CNppScript* pScript = p->GetItem();
    scriptNames.Add(pScript->GetScriptName());
    p = p->GetNext();
  }

  return scriptNames;
}

CListT<CNppScript> CNppScriptList::GetScripts() const
{
  CCriticalSectionLockGuard lock(_csScripts);

  CListT<CNppScript> scriptsList;
  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    scriptsList.Add(*p->GetItem()); // a copy of a script
    p = p->GetNext();
  }

  return scriptsList;
}

void CNppScriptList::LoadFromFile(const TCHAR* cszFileName, int nUtf8DetectLength)
{
  CFileBufT<TCHAR> fbuf;

  CCriticalSectionLockGuard lock(_csScripts);

  Free();
  _Scripts.DeleteAll();
  _bIsModified = false;
  
  if (fbuf.LoadFromFile(cszFileName, true, nUtf8DetectLength))
  {
    CStrT<TCHAR> S;
    CStrT<TCHAR> ScriptName;
    int          i;
    int          iScript;
    CNppScript*  pScript;
    
    iScript = 0;
    pScript = NULL;
    while (fbuf.GetLine(S) >= 0)
    {
      i = 0;
      while ((i < S.length()) && NppExecHelpers::IsTabSpaceChar(S[i]))
      {
        i++;
      }
      if ((S.GetAt(i) == ':') && (S.GetAt(i+1) == ':'))
      {
        // it is a script name
        ScriptName = S.GetData() + i + 2;
        i = ScriptName.length() - 1;
        while ((i >= 0) && NppExecHelpers::IsTabSpaceChar(ScriptName[i]))
        {
          ScriptName.Delete(i);
          i--;
        }
        iScript++;
        pScript = new CNppScript(ScriptName);
        _Scripts.Add(pScript);
      }
      else
      {
        // it is a script line
        if (pScript)
        {
          pScript->GetCmdList().Add(S);
        }
      }
    }
  }
}

bool CNppScriptList::ModifyScript(const tstr& ScriptName, const CNppScript& newScript)
{
  tstr S2;
  bool bModified = false;

  CCriticalSectionLockGuard lock(_csScripts);

  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    CNppScript* pScript = p->GetItem();
    if (pScript->GetScriptName() == ScriptName)
    {
      // ScriptName is matched
      // Now let's check if newScript is different than pScript
      CListItemT<tstr>* p1 = pScript->GetCmdList().GetFirst();
      CListItemT<tstr>* p2 = newScript.GetCmdList().GetFirst();
      while (p2)
      {
        // Walking through newScript
        S2 = p2->GetItem();
        if (p1)
        {
          if (S2 != p1->GetItem())
          {
            // list is modified
            _bIsModified = true;
            p1->SetItem(S2);
          }
          p1 = p1->GetNext();
        }
        else
        {
          // list is modified
          _bIsModified = true;
          pScript->GetCmdList().Add(S2);
        }
        p2 = p2->GetNext();
      }
      if (p1)
      {
        // pScript is greater than newScript
        // list is modified
        _bIsModified = true;
        while (((p2 = pScript->GetCmdList().GetLast()) != NULL) && (p2 != p1))
        {
          pScript->GetCmdList().DeleteLast();
        }
        pScript->GetCmdList().Delete(p1);
      }
      bModified = true;
      p = NULL; // break condition
    }
    else
      p = p->GetNext();
  }

  return bModified;
}

void CNppScriptList::SaveToFile(const TCHAR* cszFileName)
{
  tstr             ScriptName;
  tstr             ScriptLine;
  CFileBufT<TCHAR> fbuf;

  fbuf.GetBufPtr()->Reserve(_Scripts.GetCount() * 128);

  CCriticalSectionLockGuard lock(_csScripts);

  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p)
  {
    CNppScript* pScript = p->GetItem();
    ScriptName = pScript->GetScriptName();
    ScriptName.Insert(0, _T("::"), 2);
    ScriptName += _T("\r\n");
    fbuf.GetBufPtr()->Append(ScriptName.c_str(), ScriptName.length());

    CListItemT<tstr>* pline = pScript->GetCmdList().GetFirst();
    while (pline)
    {
      ScriptLine = pline->GetItem();
      ScriptLine += _T("\r\n");
      fbuf.GetBufPtr()->Append(ScriptLine.c_str(), ScriptLine.length());
      pline = pline->GetNext();
    }

    p = p->GetNext();
  }

  // .bak file...
  tstr bakFileName = cszFileName;
  bakFileName += _T(".bak");
  ::DeleteFile(bakFileName.c_str()); // delete the old .bak file, if any
  ::MoveFile(cszFileName, bakFileName.c_str()); // create the new .bak file

  fbuf.SaveToFile(cszFileName);
  // list is saved -> clear modification flag
  _bIsModified = false;
}

bool CNppScriptList::IsScriptPresent(const tstr& ScriptName) const
{
  bool bRet = false;

  CCriticalSectionLockGuard lock(_csScripts);

  CListItemT<CNppScript *> * p = _Scripts.GetFirst();
  while (p && !bRet)
  {
    const CNppScript* pScript = p->GetItem();
    if (pScript->GetScriptName() == ScriptName)
      bRet = true;
    else
      p = p->GetNext();
  }

  return bRet;
}
