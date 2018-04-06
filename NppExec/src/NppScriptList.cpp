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

#include "NppScriptList.h"
#include "NppExecHelpers.h"

CNppScriptList::CNppScriptList()
{
  _bIsModified = false;
}

CNppScriptList::~CNppScriptList()
{
  free();
}

void CNppScriptList::free()
{
  CListItemT<PNppScript>* p = _Scripts.GetFirst();
  while (p)
  {
    CNppScript* pScript = p->GetItem();
    if (pScript)
    {
      pScript->DeleteAll();
      delete pScript;
      p->SetItem(NULL);
    }
    p = p->GetNext();
  }
}

bool CNppScriptList::AddScript(const tstr& ScriptName, const CNppScript& newScript)
{
  if (!ModifyScript(ScriptName, newScript))
  {
    CCriticalSectionLockGuard lock(_csScripts);

    if (_ScriptNames.Add(ScriptName))
    {
      CNppScript* pScript = new CNppScript;
      if (pScript)
      {      
        pScript->AddItems(newScript);
        if (_Scripts.Add(pScript))
        {
          // script is added -> list is modified
          _bIsModified = true;
          return true;
        }
        else
        {
          pScript->DeleteAll();
          delete pScript;
          _ScriptNames.DeleteLast();
          return false;
        }

      }
      else
      {
        _ScriptNames.DeleteLast();
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

  CListItemT<tstr>* p = _ScriptNames.GetFirst();
  CListItemT<PNppScript>* p1 = _Scripts.GetFirst();
  while (p && p1)
  {
    if (p->GetItem() == ScriptName)
    {
      CNppScript* pScript;

      pScript = p1->GetItem();
      if (pScript)
      {
        pScript->DeleteAll();
        delete pScript;
      }
      _Scripts.Delete(p1);
      _ScriptNames.Delete(p);

      // script is deleted -> list is modified
      _bIsModified = true;
      bRet = true;
      p = NULL; // break condition
    }
    else
    {
      p = p->GetNext();
      p1 = p1->GetNext();
    }
  }
  
  return bRet;
}

bool CNppScriptList::GetScript(const tstr& ScriptName, CNppScript& outScript)
{
  bool bRet = false;

  outScript.DeleteAll();

  CCriticalSectionLockGuard lock(_csScripts);

  CListItemT<tstr>* p = _ScriptNames.GetFirst();
  CListItemT<PNppScript>* p1 = _Scripts.GetFirst();
  while (p && p1)
  {
    if (p->GetItem() == ScriptName)
    {
      CNppScript* pScript = p1->GetItem();
      if (pScript)
      {
        outScript.AddItems(*pScript);
      }
      bRet = true;
      p = NULL; // break condition
    }
    else
    {
      p = p->GetNext();
      p1 = p1->GetNext();
    }
  }
  
  return bRet;
}

int CNppScriptList::GetScriptCount() const
{
  CCriticalSectionLockGuard lock(_csScripts);

  return _ScriptNames.GetCount(); 
}

CListT<tstr> CNppScriptList::GetScriptNames() const
{
  CCriticalSectionLockGuard lock(_csScripts);

  return _ScriptNames; // returns a copy
}

CListT<CNppScript> CNppScriptList::GetScripts(CListT<tstr>* pScriptNames ) const
{
  if (pScriptNames)
    pScriptNames->Clear();

  CCriticalSectionLockGuard lock(_csScripts);

  CListT<CNppScript> ScriptsList;
  CListItemT<tstr>* pName = _ScriptNames.GetFirst();
  CListItemT<PNppScript>* pScript = _Scripts.GetFirst();
  while (pName && pScript)
  {
    ScriptsList.Add(*pScript->GetItem()); // a copy of a script
    if (pScriptNames)
      pScriptNames->Add(pName->GetItem());
    pName = pName->GetNext();
    pScript = pScript->GetNext();
  }

  return ScriptsList;
}

void CNppScriptList::LoadFromFile(const TCHAR* cszFileName, int nUtf8DetectLength)
{
  CFileBufT<TCHAR> fbuf;

  CCriticalSectionLockGuard lock(_csScripts);

  free();
  _ScriptNames.DeleteAll();
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
        _ScriptNames.Add(ScriptName);
        iScript++;
        pScript = new CNppScript;
        _Scripts.Add(pScript);
      }
      else
      {
        // it is a script line
        if (pScript)
        {
          pScript->Add(S);
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

  CListItemT<tstr>* p = _ScriptNames.GetFirst();
  CListItemT<PNppScript>* p1 = _Scripts.GetFirst();
  while (p && p1)
  {
    if (p->GetItem() == ScriptName)
    {
      // ScriptName is matched
      CNppScript* pScript = NULL;

      pScript = p1->GetItem();
      if (pScript)
      {
        // Now let's check if newScript is different than pScript
        p = pScript->GetFirst();
        CListItemT<tstr>* p2 = newScript.GetFirst();
        while (p2)
        {
          // Walking through newScript
          S2 = p2->GetItem();
          if (p)
          {
            if (S2 != p->GetItem())
            {
              // list is modified
              _bIsModified = true;
              p->SetItem(S2);
            }
            p = p->GetNext();
          }
          else
          {
            // list is modified
            _bIsModified = true;
            pScript->Add(S2);
          }
          p2 = p2->GetNext();
        }
        if (p)
        {
          // pScript is greater than newScript
          // list is modified
          _bIsModified = true;
          while (((p2 = pScript->GetLast()) != NULL) && (p2 != p))
          {
            pScript->DeleteLast();
          }
          pScript->Delete(p);
          p = NULL; // break condition
        }
      }
      else
      {
        pScript = new CNppScript;
        if (pScript)
        {
          // list is modified
          _bIsModified = true;
          p1->SetItem(pScript);
          pScript->AddItems(newScript);
        }
      }
      bModified = true;
    }
    else
    {
      p = p->GetNext();
      p1 = p1->GetNext();
    }
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

  CListItemT<PNppScript>* p = _Scripts.GetFirst();
  CListItemT<tstr>* pname = _ScriptNames.GetFirst();
  while (p && pname)
  {
    ScriptName = pname->GetItem();
    ScriptName.Insert(0, _T("::"), 2);
    ScriptName += _T("\r\n");
    fbuf.GetBufPtr()->Append(ScriptName.c_str(), ScriptName.length());
    CNppScript* pScript = p->GetItem();
    if (pScript)
    {
      CListItemT<tstr>* pline = pScript->GetFirst();
      while (pline)
      {
        ScriptLine = pline->GetItem();
        ScriptLine += _T("\r\n");
        fbuf.GetBufPtr()->Append(ScriptLine.c_str(), ScriptLine.length());
        pline = pline->GetNext();
      }
    }
    p = p->GetNext();
    pname = pname->GetNext();
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
    CCriticalSectionLockGuard lock(_csScripts);

    return (_ScriptNames.FindExact(ScriptName) ? true : false);
}
