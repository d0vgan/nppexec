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

#ifndef _file_modification_watcher_h_
#define _file_modification_watcher_h_
//---------------------------------------------------------------------------
#include "base.h"
#include "NppExec.h"

class CFileModificationChecker
{
private:
  tstr     m_path;
  FILETIME m_modificationTime;
  DWORD    m_dwFileSize;
  BOOL     m_bTimeIsValid;
  BOOL     m_bSizeIsValid;

  BOOL     getFileTime(HANDLE hFile, FILETIME* lpLastWriteTime);
  DWORD    getFileSize(HANDLE hFile);
  HANDLE   openTheFile();

public:
  CFileModificationChecker();
  ~CFileModificationChecker();

  BOOL AssignFile(const TCHAR* cszFilePathName);   // assign a file
  long IsFileSizeChanged(DWORD dwFileSize);        // comparison
  long IsFileTimeChanged(const FILETIME* lpFileTime);
  BOOL RequestFileSize(DWORD* lpdwSize);           // no internal update
  BOOL RequestFileTime(FILETIME* lpLastWriteTime); // no internal update
  void SetSize(DWORD dwFileSize);                  // internal update
  void SetTime(const FILETIME* lpFileTime);        // internal update
  BOOL UpdateFileInfo(); // request & update values of file time & size
  
};

//---------------------------------------------------------------------------
#endif

