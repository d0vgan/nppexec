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

#ifndef _sys_dir_file_lister_h_
#define _sys_dir_file_lister_h_
//---------------------------------------------------------------------------
#include <windows.h>
#include <TCHAR.h>
#include <string>
#include <list>

class CDirFileLister 
{
    public:
        enum ESearchFlags {
            ESF_FILES           = 0x01,  // search for files
            ESF_DIRS            = 0x02,  // search for dirs
            ESF_PLACEDIRSFIRST  = 0x00,  // dirs first (searching for dirs & files)
            ESF_PLACEFILESFIRST = 0x10,  // files first (searching for files & dirs)
            ESF_SORTED          = 0x20   // dir & file names are sorted
        };

        typedef std::basic_string<TCHAR> TStr;
        typedef std::list<TStr> TStrList;

        static bool StrLess(const TStr& s1, const TStr& s2)
        {
            return ( ::CompareString(
                         LOCALE_USER_DEFAULT, NORM_IGNORECASE, 
                         s1.c_str(), (int) s1.length(), 
                         s2.c_str(), (int) s2.length() ) == CSTR_LESS_THAN );
        }
    
    protected:
        UINT                     _SearchFlags;
        bool                     _isDirItr;
        bool                     _isEndItr;
        TStrList::const_iterator _itr;
        TStr                     _PathName;
        TStrList                 _FilesList;
        TStrList                 _DirsList;

        bool initSearch(const TCHAR* szPathName, UINT uSearchFlags);

    public:
        CDirFileLister();
        ~CDirFileLister();

        void         Clear();
        
        const TCHAR* FindNext(const TCHAR* szPathName, 
                       UINT uSearchFlags = ESF_DIRS | ESF_FILES | ESF_PLACEDIRSFIRST | ESF_SORTED,
                       bool bAllowCyclicSearch = false);
        
        const TCHAR* FindPrev(const TCHAR* szPathName, 
                       UINT uSearchFlags = ESF_DIRS | ESF_FILES | ESF_PLACEDIRSFIRST | ESF_SORTED,
                       bool bAllowCyclicSearch = false);

        const TCHAR* GetItem(UINT* pLength = NULL, bool* pIsDir = NULL);
        const TCHAR* GetNext(bool bAllowCyclicSearch = false, UINT* pLength = NULL, bool* pIsDir = NULL);
        const TCHAR* GetPrev(bool bAllowCyclicSearch = false, UINT* pLength = NULL, bool* pIsDir = NULL);
};

//---------------------------------------------------------------------------
#endif
