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

#ifndef _static_options_manager_h_
#define _static_options_manager_h_
//---------------------------------------------------------------------------
#include "base.h"
#include <TCHAR.h>
#include "cpp/CBufT.h"
#include "cpp/CStrT.h"
#include "cpp/CListT.h"
#include "c_base/HexStr.h"

// !!! set this value to 1 each time you modify the optArray !!!
#define CSTCOPTMNGR_DEBUGCHECK  0
// set this value to 1 if you want to "pack" hex data strings
#define CSTCOPTMNGR_PACKHEXDATA 1

#if CSTCOPTMNGR_PACKHEXDATA
    #include "c_base/PackDataStr.h"
#endif


#define OPTT_INT       0x0000  // int-option
#define OPTT_BOOL      0x8000  // bool-option
#define OPTT_STR       0x4000  // str-option
#define OPTT_DATA      0x2000  // binary-data-option
#define OPTTMASK       0xFF00

#define OPTF_READWRITE 0x0000  // is read from and written to a file
#define OPTF_READONLY  0x0001  // is read from a file
#define OPTF_INTERNAL  0x0002  // internal option
#define OPTFMASK       0x00FF

// CStaticOptionsManager is "Static" because a set of its options is 
// hardcoded in a static optArray. 
// I.e. you can not add a new option or delete some old option dynamically.
//
class CStaticOptionsManager
{
    public:
        enum {
            MAX_STRSIZE  = 1024
        };

        typedef CStrT<TCHAR> tstr;
        
        typedef struct OPT_ITM {
            unsigned int id;             // option's unique id
            unsigned int flags;          // flags = (OPTT_xx | OPTF_yy)
            const TCHAR* szOptionGroup;  // can be NULL for internal options
            const TCHAR* szOptionName;   // can be NULL for internal options
            int          nDefaultValue;  // default int-value for initialization
            const TCHAR* szDefaultValue; // can be NULL for int-options
        } OPT_ITEM;
        // NOTE:
        //   For the OPTT_DATA-variable you must specify
        //   nDefaultValue as (int) data_size
        //   szDefaultValue as (const void *) data
        // EXAMPLE:
        //   OPT_ITEM binaryData = {
        //     OPTD_BINARY_DATA,                // id
        //     OPTT_DATA | OPTF_READWRITE,      // flags
        //     _T("BinaryOptions"),             // szOptionGroup
        //     _T("Value1"),                    // szOptionName
        //     3,                               // nDefaultValue == data_size
        //     (const TCHAR *) "\x00\x01\x02",  // szDefaultValue == data
        //   };  // Attention: data contains BYTEs, not TCHARs !!!
        
        CStaticOptionsManager(const TCHAR* name,
          const OPT_ITEM* optArray, unsigned int optCount);
        ~CStaticOptionsManager();

        int          ReadOptions(const TCHAR* szFilePathName);
        int          SaveOptions(const TCHAR* szFilePathName);

        bool         GetBool(unsigned int id, bool actual = true) const;
        int          GetData(unsigned int id, void* buf, int buf_size,
                       bool actual = true) const;
        const void*  GetData(unsigned int id, int* data_size,
                       bool actual = true) const;
        int          GetInt(unsigned int id, bool actual = true) const;
        const TCHAR* GetStr(unsigned int id, bool actual = true) const;
        const TCHAR* GetStr(unsigned int id, int* str_len, 
                       bool actual = true) const;
        unsigned int GetUint(unsigned int id, bool actual = true) const;

        bool         SetBool(unsigned int id, bool bValue);
        bool         SetData(unsigned int id, const void* data, int data_size);
        bool         SetInt(unsigned int id, int iValue);
        bool         SetStr(unsigned int id, const TCHAR* str);
        bool         SetUint(unsigned int id, unsigned int uValue);
        
    protected:
        typedef struct ITM_INFO {
            void*        listItemPtr;
            unsigned int optArrayIndex;
        } ITEM_INFO;
        
        typedef struct INTINT_ITM {
            int actual;
            int saved;
        } INTINT_ITEM;
        
        typedef struct STRSTR_ITM {
            tstr actual;
            tstr saved;
        } STRSTR_ITEM;
        
        typedef struct BUFBUF_ITM {
            CByteBuf actual;
            CByteBuf saved;
        } BUFBUF_ITEM;
        
        typedef CListT<INTINT_ITEM> CIntIntList;
        typedef CListT<STRSTR_ITEM> CStrStrList;
        typedef CListT<BUFBUF_ITEM> CBufBufList;
        
        const OPT_ITEM*  _optArray;
        int              _optCount;
        tstr             _name;
        CBufT<ITEM_INFO> _itemByIdArray;
        CIntIntList      _intValuesList;
        CStrStrList      _strValuesList;
        CBufBufList      _bufValuesList;
        static tstr      _emptyStr;
        static CByteBuf  _emptyBuf;
        TCHAR*           _tempStr;
        
        const CByteBuf& getBufListItem(void* listItemPtr, bool actual) const;
        int getIntListItem(void* listItemPtr, bool actual) const;
        const tstr& getStrListItem(void* listItemPtr, bool actual) const;
        void setBufListValue(void* listItemPtr, 
          const void* data, int data_size, bool actual);
        void setIntListValue(void* listItemPtr, const int value, bool actual);
        void setStrListValue(void* listItemPtr, const TCHAR* str, bool actual);
        
};

//---------------------------------------------------------------------------
#endif
