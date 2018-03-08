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

#ifndef _npp_script_list_h_
#define _npp_script_list_h_
//--------------------------------------------------------------------
#include "base.h"
#include "cpp/CStrT.h"
#include "cpp/CBufT.h"
#include "cpp/CListT.h"
#include "cpp/CFileBufT.h"

typedef CStrT<TCHAR> tstr;
typedef CListT<tstr> CNppScript;
typedef CNppScript*  PNppScript;

class CNppScriptList 
{
private:
  CListT<tstr>        _ScriptNames;
  CListT<PNppScript>  _Scripts;
  bool                _bIsModified;

  void free();

public:
  CNppScriptList();
  ~CNppScriptList();
  int   GetScriptCount() const  { 
    return _ScriptNames.GetCount(); 
  }
  CListItemT<tstr>* GetFirstScriptNameItemPtr() const  { 
    return _ScriptNames.GetFirst(); 
  }
  CListItemT<PNppScript>* GetFirstScriptItemPtr() const  { 
    return _Scripts.GetFirst(); 
  }
  CListItemT<tstr>* GetLastScriptNameItemPtr() const  { 
    return _ScriptNames.GetLast(); 
  }
  CListItemT<PNppScript>* GetLastScriptItemPtr() const  { 
    return _Scripts.GetLast(); 
  }
  CListItemT<tstr>* GetNextScriptNameItemPtr(const CListItemT<tstr>* item_ptr) const  { 
    return ( item_ptr ? item_ptr->GetNext() : NULL );
  }
  CListItemT<PNppScript>* GetNextScriptItemPtr(const CListItemT<PNppScript>* item_ptr) const  { 
    return ( item_ptr ? item_ptr->GetNext() : NULL );
  }
  CListItemT<tstr>* GetPrevScriptNameItemPtr(const CListItemT<tstr>* item_ptr) const  {
    return ( item_ptr ? item_ptr->GetPrev() : NULL );
  }
  CListItemT<PNppScript>* GetPrevScriptItemPtr(const CListItemT<PNppScript>* item_ptr) const  { 
    return ( item_ptr ? item_ptr->GetPrev() : NULL ); 
  }
  bool  GetScriptNameItem(const CListItemT<tstr>* item_ptr, tstr& name) const  {
    return ( item_ptr ? (name = item_ptr->GetItem(), true) : false );
  }
  bool  GetScriptItem(const CListItemT<PNppScript>* item_ptr, PNppScript& pScript) const  {
    return ( item_ptr ? (pScript = item_ptr->GetItem(), true) : false );
  }
  bool  AddScript(const tstr& ScriptName, const CNppScript& newScript);
  bool  DeleteScript(const tstr& ScriptName);
  bool  GetScript(const tstr& ScriptName, CNppScript& outScript);
  bool  IsModified() const  { return _bIsModified; }
  void  LoadFromFile(const TCHAR* cszFileName, int nUtf8DetectLength = 16384);
  bool  ModifyScript(const tstr& ScriptName, const CNppScript& newScript);
  void  SaveToFile(const TCHAR* cszFileName);
  void  SetModified(bool bIsModified)  { _bIsModified = bIsModified; }
  bool  IsScriptPresent(const tstr& ScriptName) const {
    return (_ScriptNames.FindExact(ScriptName) ? true : false);
  }
};

//--------------------------------------------------------------------
#endif

