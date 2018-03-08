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

#include "CStaticOptionsManager.h"

CStaticOptionsManager::tstr CStaticOptionsManager::_emptyStr;
CByteBuf CStaticOptionsManager::_emptyBuf;

CStaticOptionsManager::CStaticOptionsManager(const TCHAR* name,
  const OPT_ITEM* optArray, unsigned int optCount)
{
    static TCHAR TEMP_STR[MAX_STRSIZE];
    
    _tempStr = (TCHAR *) TEMP_STR;
    
    _optArray = optArray;
    _optCount = optCount;
    _name = name;
    
    unsigned int maxId = 0;
    for (int i = optCount - 1; i >= 0; i--)
    {
        if ( maxId < optArray[i].id )
            maxId = optArray[i].id;
    }
    
    _itemByIdArray.SetSize(maxId + 1);
    for (unsigned int i = 0; i <= maxId; i++)
    {
        const ITEM_INFO itmInf = {0, 0};
        _itemByIdArray.Append( itmInf );
    }
    
    #if CSTCOPTMNGR_DEBUGCHECK
        TCHAR __szTitle[256];
        TCHAR __szInfo[1024];
        
        unsigned int totalId = 0;
        unsigned int minId = maxId;
        for (unsigned int i = 0; i < optCount; i++)
        {
            const unsigned int id = optArray[i].id;
            if ( minId > id )  minId = id;
            
            ++totalId;
            for (int j = i - 1; j >= 0; j--)
            {
                if ( id == optArray[j].id )
                {
                    --totalId; // this id is not unique
                    break;
                }
            }
        }
        
        ::wsprintf( __szTitle, 
          _T("CStaticOptionsManager ( %s ) Debug Info"), 
          _name.c_str() );
        ::wsprintf( __szInfo, 
          _T("option items: min id = %u\n") \
          _T("option items: max id = %u\n") \
          _T("option items: total id count = %u%s\n") \
          _T("_itemByIdArray.size() = %u"),
          minId, maxId, totalId, 
          (totalId == maxId + 1) ? _T("") : _T("\nWARNING: some id values are not used!"),
          _itemByIdArray.size() );
        ::MessageBox(NULL, __szInfo, __szTitle, MB_OK );
        
        ::wsprintf( __szTitle,
          _T("CStaticOptionsManager ( %s ) Debug Error"),
          _name.c_str() );
        for (unsigned int i = 0; i < optCount; i++)
        {
            const unsigned int id = optArray[i].id;
            const TCHAR*       szOptionName = optArray[i].szOptionName;
            const TCHAR*       szOptionGroup = optArray[i].szOptionGroup;
            for (unsigned int j = i + 1; j < optCount; j++)
            {
                if ( id == optArray[j].id )
                {
                    if ( id != 0 )
                    {
                        ::wsprintf( __szInfo,
                          _T("optArray[%u].id == optArray[%u].id == %u,\n") \
                          _T("but each option item id must be unique!"),
                          i, j, id );
                        ::MessageBox(NULL, __szInfo, __szTitle, MB_OK | MB_ICONERROR );
                    }
                    break;
                }
                if ( (szOptionName && optArray[j].szOptionName) && 
                     (lstrcmpi(szOptionName, optArray[j].szOptionName) == 0) )
                {
                    if ( (szOptionGroup && optArray[j].szOptionGroup) &&
                         (lstrcmpi(szOptionGroup, optArray[j].szOptionGroup) == 0) )
                    {
                        ::wsprintf( __szInfo,
                          _T("optArray[%u].szOptionName == optArray[%u].szOptionName == %s\n") \
                          _T("AND optArray[%u].szOptionGroup == optArray[%u].szOptionGroup == %s,\n") \
                          _T("but szOptionName or szOptionGroup must be different!"),
                          i, j, szOptionName, i, j, szOptionGroup );
                        ::MessageBox(NULL, __szInfo, __szTitle, MB_OK | MB_ICONERROR );
                        break;
                    }
                }
            }
        }
    #endif
    
    for (unsigned int i = 0; i < optCount; i++)
    {
        const unsigned int type = optArray[i].flags & OPTTMASK;
        
        if ( (type == OPTT_INT) || (type == OPTT_BOOL) )
        {
            ITEM_INFO   itmInf;
            INTINT_ITEM itm = {0, 0};
            
            itmInf.optArrayIndex = i;
            itmInf.listItemPtr = (void *) _intValuesList.Add( itm );
            _itemByIdArray[optArray[i].id] = itmInf;
            // initializing actual & saved values
            setIntListValue( itmInf.listItemPtr, optArray[i].nDefaultValue, true );
            setIntListValue( itmInf.listItemPtr, optArray[i].nDefaultValue, false );
        }
        else if ( type == OPTT_STR )
        {
            ITEM_INFO   itmInf;
            STRSTR_ITEM itm;
            
            itmInf.optArrayIndex = i;
            itmInf.listItemPtr = (void *) _strValuesList.Add( itm );
            _itemByIdArray[optArray[i].id] = itmInf;
            // initializing actual & saved values
            setStrListValue( itmInf.listItemPtr, optArray[i].szDefaultValue, true );
            setStrListValue( itmInf.listItemPtr, optArray[i].szDefaultValue, false );
        }
        else if ( type == OPTT_DATA )
        {
            ITEM_INFO   itmInf;
            BUFBUF_ITEM itm;
            
            itmInf.optArrayIndex = i;
            itmInf.listItemPtr = (void *) _bufValuesList.Add( itm );
            _itemByIdArray[optArray[i].id] = itmInf;
            // initializing actual & saved values
            setBufListValue( itmInf.listItemPtr,
              (const void *) optArray[i].szDefaultValue, // as "data"
              optArray[i].nDefaultValue,                 // as "data_size"
              true );                                    // actual value
            setBufListValue( itmInf.listItemPtr,
              (const void *) optArray[i].szDefaultValue, // as "data"
              optArray[i].nDefaultValue,                 // as "data_size"
              false );                                   // saved value
        }
    }
}

CStaticOptionsManager::~CStaticOptionsManager()
{
}

int CStaticOptionsManager::ReadOptions(const TCHAR* szFilePathName)
{
    if ( (!szFilePathName) || (szFilePathName[0] == 0) )
        return -1;
        
    int nRead = 0;
    for (int i = 0; i < _optCount; i++)
    {
        unsigned int type = _optArray[i].flags & OPTFMASK; // read-write flags
        if ( (type == OPTF_READWRITE) || (type == OPTF_READONLY) )
        {
            type = _optArray[i].flags & OPTTMASK; // option-type (int, str etc.)
            if ( (type == OPTT_INT) || (type == OPTT_BOOL) )
            {
                int value = ::GetPrivateProfileInt( 
                  _optArray[i].szOptionGroup, 
                  _optArray[i].szOptionName, 
                  _optArray[i].nDefaultValue, 
                  szFilePathName );
                if ( value != _optArray[i].nDefaultValue )
                {
                    const ITEM_INFO& itmInf = _itemByIdArray[_optArray[i].id];
                    // updating actual & saved values
                    setIntListValue( itmInf.listItemPtr, value, true );
                    setIntListValue( itmInf.listItemPtr, value, false );
                }
                ++nRead;
            }
            else if ( type == OPTT_STR )
            {
                const TCHAR* cszDefaultValue = _optArray[i].szDefaultValue ? _optArray[i].szDefaultValue : _T("");
                
                _tempStr[0] = 0;
                ::GetPrivateProfileString( 
                  _optArray[i].szOptionGroup,
                  _optArray[i].szOptionName,
                  cszDefaultValue,
                  _tempStr,
                  MAX_STRSIZE - 1,
                  szFilePathName );
                if ( lstrcmp(_tempStr, cszDefaultValue) != 0 )
                {
                    const ITEM_INFO& itmInf = _itemByIdArray[_optArray[i].id];
                    // updating actual & saved values
                    setStrListValue( itmInf.listItemPtr, _tempStr, true );
                    setStrListValue( itmInf.listItemPtr, _tempStr, false );
                }
                ++nRead;
            }
            else if ( type == OPTT_DATA )
            {
                _tempStr[0] = 0;
                ::GetPrivateProfileString(
                  _optArray[i].szOptionGroup,
                  _optArray[i].szOptionName,
                  _T(""),
                  _tempStr,
                  MAX_STRSIZE - 1,
                  szFilePathName );
                int len = lstrlen(_tempStr);
                if ( len > 0 )
                {
                    #if CSTCOPTMNGR_PACKHEXDATA
                        const int tmpSize = 8*(len + 1);
                        TCHAR* tmpStr = new TCHAR[tmpSize];
                        if ( tmpStr )
                        {
                            len = c_base::_tunpack_data_str( _tempStr, tmpStr, tmpSize );
                        }
                        else
                        {
                            tmpStr = _tempStr;
                        }
                    #else
                        TCHAR* tmpStr = _tempStr;
                    #endif
                    
                    len = 2 + len/2;
                    BYTE* buf = new BYTE[len];
                    if ( buf )
                    {
                        len = c_base::_thexstr2buf( tmpStr, buf, len );
                        const ITEM_INFO& itmInf = _itemByIdArray[_optArray[i].id];
                        const CByteBuf& actual = getBufListItem( itmInf.listItemPtr, true );
                        if ( actual.Compare(buf, len) != 0 )
                        {
                            // updating actual & saved values
                            setBufListValue( itmInf.listItemPtr, buf, len, true );
                            setBufListValue( itmInf.listItemPtr, buf, len, false );
                        }
                        delete [] buf;
                    }
                    
                    #if CSTCOPTMNGR_PACKHEXDATA
                        if ( tmpStr != _tempStr )  delete [] tmpStr;
                    #endif
                }
                ++nRead;
            }
        }
    }
    return nRead;
}

int CStaticOptionsManager::SaveOptions(const TCHAR* szFilePathName)
{
    if ( (!szFilePathName) || (szFilePathName[0] == 0) )
        return -1;
        
    int nWritten = 0;
    for (int i = 0; i < _optCount; i++)
    {
        unsigned int type = _optArray[i].flags & OPTFMASK; // read-write flags
        if ( type == OPTF_READWRITE )
        {
            type = _optArray[i].flags & OPTTMASK; // option-type (int, str etc.)
            if ( (type == OPTT_INT) || (type == OPTT_BOOL) )
            {
                const ITEM_INFO& itmInf = _itemByIdArray[_optArray[i].id];
                int actual = getIntListItem( itmInf.listItemPtr, true );
                int saved = getIntListItem( itmInf.listItemPtr, false );
                if ( type == OPTT_BOOL )
                {
                    actual = actual ? 1 : 0;
                    saved = saved ? 1 : 0;
                }
                if ( actual != saved )
                {
                    ::wsprintf( _tempStr, _T("%d"), actual );
                    ::WritePrivateProfileString( 
                      _optArray[i].szOptionGroup,
                      _optArray[i].szOptionName,
                      _tempStr,
                      szFilePathName );
                    // updating saved value
                    setIntListValue( itmInf.listItemPtr, actual, false );
                    ++nWritten;
                }
            }
            else if ( type == OPTT_STR )
            {
                const ITEM_INFO& itmInf = _itemByIdArray[_optArray[i].id];
                const tstr& actual = getStrListItem( itmInf.listItemPtr, true );
                const tstr& saved = getStrListItem( itmInf.listItemPtr, false );
                if ( actual != saved )
                {
                    tstr S;
                    S.Reserve( actual.length() + 2 );
                    S += _T("\"");
                    S += actual;
                    S += _T("\"");
                    ::WritePrivateProfileString(
                      _optArray[i].szOptionGroup,
                      _optArray[i].szOptionName,
                      S.c_str(),
                      szFilePathName );
                    // updating saved value
                    setStrListValue( itmInf.listItemPtr, actual.c_str(), false );
                    ++nWritten;
                }
            }
            else if ( type == OPTT_DATA )
            {
                const ITEM_INFO& itmInf = _itemByIdArray[_optArray[i].id];
                const CByteBuf& actual = getBufListItem( itmInf.listItemPtr, true );
                const CByteBuf& saved = getBufListItem( itmInf.listItemPtr, false );
                if ( actual != saved )
                {
                    const int len = c_base::_tbuf2hexstr( actual.GetData(), 
                      actual.GetCount(), _tempStr, MAX_STRSIZE, NULL );

                    #if CSTCOPTMNGR_PACKHEXDATA
                        TCHAR* tmpStr = new TCHAR[len + 2];
                        if ( tmpStr )
                        {
                            c_base::_tpack_data_str( _tempStr, tmpStr, len + 2 );
                        }
                        else
                        {
                            tmpStr = _tempStr;
                        }
                    #else
                        TCHAR* tmpStr = _tempStr;
                    #endif
                      
                    ::WritePrivateProfileString(
                      _optArray[i].szOptionGroup,
                      _optArray[i].szOptionName,
                      tmpStr,
                      szFilePathName );
                      
                    #if CSTCOPTMNGR_PACKHEXDATA
                        if ( tmpStr != _tempStr )  delete [] tmpStr;
                    #endif
                    
                    // updating saved value
                    setBufListValue( itmInf.listItemPtr, 
                      actual.GetData(), actual.GetCount(), false );
                    ++nWritten;
                }
            }
        }
    }
    return nWritten;
}

bool CStaticOptionsManager::GetBool(unsigned int id, bool actual ) const
{
    return ( GetInt(id, actual) ? true : false );
}

int CStaticOptionsManager::GetData(unsigned int id, 
  void* buf, int buf_size, bool actual ) const
{
    if ( buf && (buf_size > 0) )
    {
        ((BYTE *) buf)[0] = 0;
        if ( id < (unsigned int) _itemByIdArray.GetCount() )
        {
            const ITEM_INFO&   itmInf = _itemByIdArray[id];
            const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
            
            if ( type == OPTT_DATA )
            {
                const CByteBuf& data = getBufListItem( itmInf.listItemPtr, actual );
                if ( buf_size > data.GetCount() )
                    buf_size = data.GetCount();
                for (int i = 0; i < buf_size; i++)
                {
                    ((BYTE *) buf)[i] = data[i];
                }
                return buf_size;
            }
        }
    }
    return 0;
}

const void* CStaticOptionsManager::GetData(unsigned int id, 
  int* data_size, bool actual ) const
{
    if ( data_size )
    {
        *data_size = 0;
        if ( id < (unsigned int) _itemByIdArray.GetCount() )
        {
            const ITEM_INFO&   itmInf = _itemByIdArray[id];
            const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;

            if ( type == OPTT_DATA )
            {
                const CByteBuf& data = getBufListItem( itmInf.listItemPtr, actual );
                *data_size = data.GetCount();
                return ( (const void *) data.GetData() );
            }
        }
    }
    return 0;
}

int CStaticOptionsManager::GetInt(unsigned int id, bool actual ) const
{
    if ( id < (unsigned int) _itemByIdArray.GetCount() )
    {
        const ITEM_INFO&   itmInf = _itemByIdArray[id];
        const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
        
        if ( (type == OPTT_INT) || (type == OPTT_BOOL) )
        {
            return getIntListItem( itmInf.listItemPtr, actual );
        }
    }
    return 0;
}

const TCHAR* CStaticOptionsManager::GetStr(unsigned int id, bool actual ) const
{
    if ( id < (unsigned int) _itemByIdArray.GetCount() )
    {
        const ITEM_INFO&   itmInf = _itemByIdArray[id];
        const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
        
        if ( type == OPTT_STR )
        {
            return getStrListItem( itmInf.listItemPtr, actual ).c_str();
        }
    }
    return _T("");
}

const TCHAR* CStaticOptionsManager::GetStr(unsigned int id, 
  int* str_len, bool actual ) const
{
    if ( str_len )
    {
        *str_len = 0;
        if ( id < (unsigned int) _itemByIdArray.GetCount() )
        {
            const ITEM_INFO&   itmInf = _itemByIdArray[id];
            const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
        
            if ( type == OPTT_STR )
            {
                const tstr& str = getStrListItem( itmInf.listItemPtr, actual );
                *str_len = str.length();
                return str.c_str();
            }
        }
    }
    return _T("");
}

unsigned int CStaticOptionsManager::GetUint(unsigned int id, bool actual ) const
{
    return ( (unsigned int) GetInt(id, actual) );
}

bool CStaticOptionsManager::SetBool(unsigned int id, bool bValue)
{
    return SetInt( id, bValue ? 1 : 0 );
}

bool CStaticOptionsManager::SetData(unsigned int id, 
  const void* data, int data_size)
{
    if ( id < (unsigned int) _itemByIdArray.GetCount() )
    {
        const ITEM_INFO&   itmInf = _itemByIdArray[id];
        const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
        
        if ( type == OPTT_DATA )
        {
            // updating actual value
            setBufListValue( itmInf.listItemPtr, data, data_size, true );
            return true;
        }
    }
    return false;
}

bool CStaticOptionsManager::SetInt(unsigned int id, int iValue)
{
    if ( id < (unsigned int) _itemByIdArray.GetCount() )
    {
        const ITEM_INFO&   itmInf = _itemByIdArray[id];
        const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
        
        if ( (type == OPTT_INT) || (type == OPTT_BOOL) )
        {
            // updating actual value
            setIntListValue( itmInf.listItemPtr, iValue, true );
            return true;
        }
    }
    return false;
}

bool CStaticOptionsManager::SetStr(unsigned int id, const TCHAR* str)
{
    if ( id < (unsigned int) _itemByIdArray.GetCount() )
    {
        const ITEM_INFO&   itmInf = _itemByIdArray[id];
        const unsigned int type = _optArray[itmInf.optArrayIndex].flags & OPTTMASK;
        
        if ( type == OPTT_STR )
        {
            // updating actual value
            setStrListValue( itmInf.listItemPtr, str, true );
            return true;
        }
    }
    return false;
}

bool CStaticOptionsManager::SetUint(unsigned int id, unsigned int uValue)
{
    return SetInt( id, (int) uValue );
}

// Internal function. For OPTT_DATA only!!!
const CByteBuf& CStaticOptionsManager::getBufListItem(void* listItemPtr, 
  bool actual) const
{
    if ( !listItemPtr )
        return _emptyBuf;
    
    const BUFBUF_ITEM& itm = ((CBufBufList::CListItemPtr) listItemPtr)->GetItem();
    return ( actual ? itm.actual : itm.saved );
}

// Internal function. For OPTT_INT and OPTT_BOOL only!!!
int CStaticOptionsManager::getIntListItem(void* listItemPtr,
  bool actual) const
{
    if ( !listItemPtr )
        return 0;

    const INTINT_ITEM& itm = ((CIntIntList::CListItemPtr) listItemPtr)->GetItem();
    return ( actual ? itm.actual : itm.saved );
}

// Internal function. For OPTT_STR only!!!
const CStaticOptionsManager::tstr& CStaticOptionsManager::getStrListItem(
  void* listItemPtr, bool actual) const
{
    if ( !listItemPtr )
        return _emptyStr;

    const STRSTR_ITEM& itm = ((CStrStrList::CListItemPtr) listItemPtr)->GetItem();
    return ( actual ? itm.actual : itm.saved );
}

// Internal function. For OPTT_DATA only!!!
void CStaticOptionsManager::setBufListValue(void* listItemPtr,
  const void* data, int data_size, bool actual)
{
    if ( listItemPtr )
    {
        BUFBUF_ITEM& itm = ((CBufBufList::CListItemPtr) listItemPtr)->GetItem();
        if ( actual )
            itm.actual.Copy( (const BYTE *) data, data_size );
        else
            itm.saved.Copy( (const BYTE *) data, data_size );
    }
}

// Internal function. For OPTT_INT and OPTT_BOOL only!!!
void CStaticOptionsManager::setIntListValue(void* listItemPtr, 
  const int value, bool actual)
{
    if ( listItemPtr )
    {
        INTINT_ITEM& itm = ((CIntIntList::CListItemPtr) listItemPtr)->GetItem();
        if ( actual )
            itm.actual = value;
        else
            itm.saved = value;
    }
}

// Internal function. For OPTT_STR only!!!
void CStaticOptionsManager::setStrListValue(void* listItemPtr, 
  const TCHAR* str, bool actual)
{
    if ( listItemPtr )
    {
        STRSTR_ITEM& itm = ((CStrStrList::CListItemPtr) listItemPtr)->GetItem();
        if ( actual )
            itm.actual = str;
        else
            itm.saved = str;
    }
}

