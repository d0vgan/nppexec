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

#include "CFileModificationChecker.h"
#include "NppExecHelpers.h"


namespace
{
    tstr getFileDir(LPCTSTR cszFilePath, LPCTSTR* ppFileName = nullptr)
    {
        tstr sDir = cszFilePath;
        int nPos = sDir.RFindOneOf(_T("\\/"));
        if ( nPos != -1 )
        {
            sDir.SetSize(nPos);
            if ( ppFileName )
                *ppFileName = cszFilePath + nPos + 1;
        }
        else
        {
            sDir = _T(".");
            if ( ppFileName )
                *ppFileName = cszFilePath;
        }

        return sDir;
    }
}


// CDirectoryWatcher
CDirectoryWatcher::CDirectoryWatcher()
{
    m_hStopWatchThreadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hWatchThreadDoneEvent = ::CreateEvent(NULL, TRUE, TRUE, NULL);
    m_isWatchThreadStarted = false;
}

CDirectoryWatcher::~CDirectoryWatcher()
{
    StopWatching();
    ::CloseHandle(m_hWatchThreadDoneEvent);
    ::CloseHandle(m_hStopWatchThreadEvent);
}

DWORD WINAPI CDirectoryWatcher::WatchThreadProc(LPVOID lpParam)
{
    CDirectoryWatcher* pDirWatcher = (CDirectoryWatcher *) lpParam;

    ::ResetEvent(pDirWatcher->m_hWatchThreadDoneEvent);

    const DWORD nObjs = static_cast<DWORD>(pDirWatcher->m_Dirs.size()) + 1;
    std::unique_ptr<HANDLE[]> hWaitObjs(new HANDLE[nObjs]);

    hWaitObjs[0] = pDirWatcher->m_hStopWatchThreadEvent;

    DWORD i = 1;
    for ( const auto& pDir : pDirWatcher->m_Dirs )
    {
        hWaitObjs[i] = ::FindFirstChangeNotification(pDir->sDirectory.c_str(), pDir->bRecursive, pDir->dwNotifyFilter);
        ++i;
    }

    for ( ; ; )
    {
        DWORD dwWaitStatus = ::WaitForMultipleObjects(nObjs, hWaitObjs.get(), FALSE, INFINITE);

        if ( dwWaitStatus == WAIT_OBJECT_0 ) // m_hStopWatchThreadEvent
            break;

        if ( (dwWaitStatus > WAIT_OBJECT_0) && (dwWaitStatus < WAIT_OBJECT_0 + nObjs) )
        {
            i = dwWaitStatus - WAIT_OBJECT_0;
            auto& pDir = pDirWatcher->m_Dirs[i - 1];
            if ( pDir->pChangeListener )
            {
                pDir->pChangeListener->HandleDirectoryChange(pDir.get());
            }

            HANDLE hChange = hWaitObjs[i];
            ::FindNextChangeNotification(hChange);
        }
    }

    for ( i = 1; i < nObjs; ++i )
    {
        HANDLE hChange = hWaitObjs[i];
        ::FindCloseChangeNotification(hChange);
    }

    ::SetEvent(pDirWatcher->m_hWatchThreadDoneEvent);
    return 0;
}

CDirectoryWatcher::dir_items_type::iterator CDirectoryWatcher::findDir(const tstr& sDirectory)
{
    auto itrEnd = m_Dirs.end();
    for ( auto itr = m_Dirs.begin(); itr != itrEnd; ++itr )
    {
        const auto& pDir = *itr;
        if ( pDir->sDirectory == sDirectory )
            return itr;
    }
    return itrEnd;
}

void CDirectoryWatcher::AddDir(LPCTSTR cszDirectory, IDirectoryChangeListener* pChangeListener, DWORD dwNotifyFilter, BOOL bRecursive)
{
    if ( findDir(cszDirectory) == m_Dirs.end() )
    {
        m_Dirs.push_back( std::make_unique<DirWatchStruct>(this, pChangeListener, dwNotifyFilter, bRecursive, cszDirectory) );
    }
}

void CDirectoryWatcher::AddFile(LPCTSTR cszFilePath, IFileChangeListener* pChangeListener, DWORD dwNotifyFilter)
{
    tstr sDir = getFileDir(cszFilePath);

    auto itr = findDir(sDir);
    if ( itr == m_Dirs.end() )
    {
        m_Dirs.push_back( std::make_unique<DirWatchStruct>(this, &m_DirChangeListener, dwNotifyFilter, FALSE, sDir.c_str()) );
        m_Dirs.back()->AddFile(pChangeListener, cszFilePath);
    }
    else
    {
        auto& pDir = *itr;
        pDir->dwNotifyFilter |= dwNotifyFilter;
        pDir->AddFile(pChangeListener, cszFilePath);
    }
}

void CDirectoryWatcher::StartWatching()
{
    if ( m_isWatchThreadStarted )
        return;

    ::ResetEvent(m_hStopWatchThreadEvent);
    if ( NppExecHelpers::CreateNewThread(WatchThreadProc, this) )
    {
        m_isWatchThreadStarted = true;
    }
}

void CDirectoryWatcher::StopWatching()
{
    ::SetEvent(m_hStopWatchThreadEvent);
    ::WaitForSingleObject(m_hWatchThreadDoneEvent, INFINITE);
    m_isWatchThreadStarted = false;
}

void CDirectoryWatcher::CInternalDirectoryChangeListener::HandleDirectoryChange(const DirWatchStruct* pDir)
{
    for ( auto& pFile : pDir->Files )
    {
        FileInfoStruct fileNow(pFile->pChangeListener, pFile->filePath);

        if ( !pFile->HasEqualSizeAndTime(fileNow) )
        {
            pFile->CopySizeAndTime(fileNow);
            if ( pFile->pChangeListener )
            {
                pFile->pChangeListener->HandleFileChange(pFile.get());
            }
        }
    }
}


// CFileModificationWatcher
void CFileModificationWatcher::AddFile(LPCTSTR cszFilePath, IFileChangeListener* pChangeListener)
{
    m_DirWatcher.AddFile(cszFilePath, pChangeListener, FILE_NOTIFY_CHANGE_LAST_WRITE);
}

void CFileModificationWatcher::StartWatching()
{
    m_DirWatcher.StartWatching();
}

void CFileModificationWatcher::StopWatching()
{
    m_DirWatcher.StopWatching();
}


// FileInfoStruct
FileInfoStruct::FileInfoStruct(IFileChangeListener* pChangeListener_, const tstr& filePath_) :
  pChangeListener(pChangeListener_), filePath(filePath_)
{
    GetFileSizeAndTime(filePath.c_str(), &fileSize, &fileLastWriteTime);
}

BOOL FileInfoStruct::GetFileSizeAndTime(LPCTSTR cszFilePath, LARGE_INTEGER* pliSize, FILETIME* pLastWriteTime)
{
    ::ZeroMemory(pliSize, sizeof(LARGE_INTEGER));
    ::ZeroMemory(pLastWriteTime, sizeof(FILETIME));

    BOOL bResult = FALSE;
    HANDLE hFile = ::CreateFile( cszFilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

    if ( hFile != INVALID_HANDLE_VALUE)
    {
        if ( ::GetFileSizeEx(hFile, pliSize) )
            bResult = TRUE;

        if ( !::GetFileTime(hFile, NULL, NULL, pLastWriteTime) )
            bResult = FALSE;

        ::CloseHandle(hFile);
    }

    return bResult;
}

bool FileInfoStruct::HasEqualSizeAndTime(const FileInfoStruct& other) const
{
    return ( fileSize.LowPart == other.fileSize.LowPart &&
             fileSize.HighPart == other.fileSize.HighPart &&
             ::CompareFileTime(&fileLastWriteTime, &other.fileLastWriteTime) == 0 );
}

void FileInfoStruct::CopySizeAndTime(const FileInfoStruct& other)
{
    ::CopyMemory(&fileSize, &other.fileSize, sizeof(fileSize));
    ::CopyMemory(&fileLastWriteTime, &other.fileLastWriteTime, sizeof(fileLastWriteTime));
}


// DirWatchStruct
DirWatchStruct::DirWatchStruct(CDirectoryWatcher* pDirWatcher_, IDirectoryChangeListener* pChangeListener_,
                               DWORD dwNotifyFilter_, BOOL bRecursive_, const tstr& sDirectory_) :
  pDirWatcher(pDirWatcher_), pChangeListener(pChangeListener_),
  dwNotifyFilter(dwNotifyFilter_), bRecursive(bRecursive_), sDirectory(sDirectory_)
{
}

void DirWatchStruct::AddFile(IFileChangeListener* pChangeListener_, const tstr& sFilePath)
{
    if ( findFile(sFilePath) == Files.end() )
    {
        Files.push_back( std::make_unique<FileInfoStruct>(pChangeListener_, sFilePath) );
    }
}

DirWatchStruct::file_items_type::iterator DirWatchStruct::findFile(const tstr& sFilePath)
{
    auto itrEnd = Files.end();
    for ( auto itr = Files.begin(); itr != itrEnd; ++itr )
    {
        const auto& pFile = *itr;
        if ( pFile->filePath == sFilePath )
            return itr;
    }
    return itrEnd;
}
