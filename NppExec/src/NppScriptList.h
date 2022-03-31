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

#ifndef _npp_script_list_h_
#define _npp_script_list_h_
//--------------------------------------------------------------------
#include "base.h"
#include "cpp/CStrT.h"
#include "cpp/CBufT.h"
#include "cpp/CListT.h"
#include "cpp/CFileBufT.h"
#include "NppExecHelpers.h"
#include "NppScript.h"

#include "CFileModificationChecker.h"


class CNppScriptList;


class CNppScriptFileChangeListener : public IFileChangeListener
{
public:
  CNppScriptFileChangeListener(CNppScriptList* pNppScriptList);

  virtual void HandleFileChange(const FileInfoStruct* pFile) override;

private:
  CNppScriptList* m_pNppScriptList;
};


class CNppScriptList
{
public:
  enum eFileStateFlags {
    fsfNeedsReload = 0x01,
    fsfWasReloaded = 0x02,
    fsfIsSaving    = 0x10
  };

private:
  CListT<CNppScript *>  _Scripts;
  tstr                  _ScriptFileName;
  int                   _nUtf8DetectLength;
  volatile int          _nFileState;
  bool                  _bIsModified;
  mutable CCriticalSection _csScripts;

  void Free();
  void LoadFromFile_NoLock(const TCHAR* cszFileName, int nUtf8DetectLength);
  void ReloadScriptFileIfNeeded_NoLock();

public:
  CNppScriptList();
  ~CNppScriptList();

  bool AddScript(const tstr& ScriptName, const CNppScript& newScript);
  bool DeleteScript(const tstr& ScriptName);
  bool GetScript(const tstr& ScriptName, CNppScript& outScript);
  int  GetScriptCount();
  CListT<tstr> GetScriptNames();
  CListT<CNppScript> GetScripts();
  int  GetFileState() const  { return _nFileState; }
  const tstr& GetScriptFileName() const { return _ScriptFileName; }
  bool IsModified() const  { return _bIsModified; }
  void LoadFromFile(const TCHAR* cszFileName, int nUtf8DetectLength = 16384);
  bool ModifyScript(const tstr& ScriptName, const CNppScript& newScript);
  void SaveToFile(const TCHAR* cszFileName);
  void SetModified(bool bIsModified)  { _bIsModified = bIsModified; }
  void SetFileState(int nFileState)  { _nFileState = nFileState; }
  bool IsScriptPresent(const tstr& ScriptName);
};

//--------------------------------------------------------------------
#endif

