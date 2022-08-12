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
#include "cpp/CStrT.h"
#include <vector>
#include <memory>

typedef CStrT<TCHAR> tstr;

struct FileInfoStruct;
struct DirWatchStruct;

class IDirectoryChangeListener
{
public:
    IDirectoryChangeListener() { }
    virtual ~IDirectoryChangeListener() { }

    virtual void HandleDirectoryChange(const DirWatchStruct* pDir) = 0;
};

class IFileChangeListener
{
public:
    IFileChangeListener() { }
    virtual ~IFileChangeListener() { }

    virtual void HandleFileChange(const FileInfoStruct* pFile) = 0;
};

class CDirectoryWatcher
{
public:
    typedef std::vector< std::unique_ptr<DirWatchStruct> > dir_items_type;
    typedef std::vector< std::unique_ptr<FileInfoStruct> > file_items_type;

public:
    CDirectoryWatcher();
    ~CDirectoryWatcher();

    // call these methods _before_ the StartWatching()
    // dwNotifyFilter: see FILE_NOTIFY_CHANGE_* notifications, e.g. FILE_NOTIFY_CHANGE_LAST_WRITE
    void AddDir(LPCTSTR cszDirectory, IDirectoryChangeListener* pChangeListener, DWORD dwNotifyFilter, BOOL bRecursive);
    void AddFile(LPCTSTR cszFilePath, IFileChangeListener* pChangeListener, DWORD dwNotifyFilter);

    // invoke StartWatching() _after_ all the AddDir/AddFile calls
    void StartWatching();
    void StopWatching();

private:
    dir_items_type::iterator findDir(const tstr& sDirectory);
    static DWORD WINAPI WatchThreadProc(LPVOID lpParam);

    class CInternalDirectoryChangeListener : public IDirectoryChangeListener
    {
    public:
        virtual void HandleDirectoryChange(const DirWatchStruct* pDir) override;
    };

private:
    HANDLE m_hStopWatchThreadEvent;
    HANDLE m_hWatchThreadDoneEvent;
    CInternalDirectoryChangeListener m_DirChangeListener;
    dir_items_type m_Dirs;
};

class CFileModificationWatcher
{
public:
    void AddFile(LPCTSTR cszFilePath, IFileChangeListener* pChangeListener);

    void StartWatching();
    void StopWatching();

private:
    CDirectoryWatcher m_DirWatcher;
};

struct FileInfoStruct
{
    IFileChangeListener* pChangeListener;
    tstr filePath;
    LARGE_INTEGER fileSize;
    FILETIME fileLastWriteTime;

    FileInfoStruct(IFileChangeListener* pChangeListener_, const tstr& filePath_);

    static BOOL GetFileSizeAndTime(LPCTSTR cszFilePath, LARGE_INTEGER* pliSize, FILETIME* pLastWriteTime);

    bool HasEqualSizeAndTime(const FileInfoStruct& other) const;
    void CopySizeAndTime(const FileInfoStruct& other);
};

struct DirWatchStruct
{
    typedef std::vector< std::unique_ptr<FileInfoStruct> > file_items_type;

    CDirectoryWatcher* pDirWatcher;
    IDirectoryChangeListener* pChangeListener;
    DWORD dwNotifyFilter;
    BOOL bRecursive;
    tstr sDirectory;
    file_items_type Files;

    DirWatchStruct(CDirectoryWatcher* pDirWatcher_, IDirectoryChangeListener* pChangeListener_, 
                   DWORD dwNotifyFilter_, BOOL bRecursive_, const tstr& sDirectory_);

    void AddFile(IFileChangeListener* pChangeListener_, const tstr& sFilePath);

    file_items_type::iterator findFile(const tstr& sFilePath);
};

//---------------------------------------------------------------------------
#endif

