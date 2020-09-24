#include "NppExecHelpers.h"
#include "encodings/SysUniConv.h"
#include <shlwapi.h>

namespace
{
    // Compares two strings case-insensitively.
    // Returns 0 when equal; 1 when S1 > S2; -1 when S1 < S2.
    inline int strCompareNoCase(const TCHAR* s1, int len1, const TCHAR* s2, int len2)
    {
        int ret = ::CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE, s1, len1, s2, len2);
        return (ret - CSTR_EQUAL);
    }

    inline bool isPathSep(const TCHAR ch)
    {
        return (ch == _T('\\') || ch == _T('/'));
    }

    bool isPathRoot(const tstr& part)
    {
        if ((part == _T("\\")) || (part == _T("\\\\")))
            return true;  // i.e. "\" or "\\"
        if ((part.length() == 3) && (part[1] == _T(':')) && (part[2] == _T('\\')))
            return true;  // e.g. "C:\"
        return false;
    };

    inline bool isNullOrPathSep(const TCHAR ch)
    {
        return (ch == 0 || ch == _T('\\') || ch == _T('/'));
    }

    inline int findPathSep(const tstr& path, int nStartPos = 0)
    {
        return path.FindOneOf(_T("\\/"), nStartPos);
    }

    inline int rfindPathSep(const tstr& path, int nStartPos = -1)
    {
        return path.RFindOneOf(_T("\\/"), nStartPos);
    }

    bool impl_createDir(const TCHAR* dir)
    {
        if ( !::CreateDirectory(dir, NULL) )
        {
            if ( ::GetLastError() != ERROR_ALREADY_EXISTS )
                return false;
        }
        return true;
    }

    bool impl_createDirectoryTree(const tstr& dir)
    {
        tstr inter_dir = dir;
        inter_dir.Replace( _T('/'), _T('\\') );
        int n = inter_dir.Find( _T('\\'), 3 ); // skip "C:\"
        while ( n >= 0 )
        {
            inter_dir[n] = 0;
            if ( !impl_createDir(inter_dir.c_str()) )
                return false;

            inter_dir[n] = _T('\\');
            n = inter_dir.Find( _T('\\'), n + 1 );
        }
        return impl_createDir(inter_dir.c_str());
    }
}

namespace NppExecHelpers
{
    bool CreateNewThread(LPTHREAD_START_ROUTINE lpFunc, LPVOID lpParam, HANDLE* lphThread /* = NULL */)
    {
        DWORD  dwThreadID;
        HANDLE hThread;

        hThread = ::CreateThread(
            NULL,
            0,
            lpFunc,
            lpParam,
            CREATE_SUSPENDED,
            &dwThreadID);

        if ( hThread == NULL )
            return false;

        ::ResumeThread(hThread);
        if ( !lphThread )
        {
            ::CloseHandle(hThread);
        }
        else
        {
            *lphThread = hThread;
        }
        return true;
    }

    HWND GetFocusedWnd()
    {
        HWND hFocusedWnd = NULL;
        HWND hForegroundWnd = ::GetForegroundWindow();
        if (hForegroundWnd)
        {
            DWORD dwProcessId = 0;
            DWORD dwThreadId = ::GetWindowThreadProcessId(hForegroundWnd, &dwProcessId);
            if (dwThreadId != 0)
            {
                GUITHREADINFO gti;
                ::ZeroMemory(&gti, sizeof(GUITHREADINFO));
                gti.cbSize = sizeof(GUITHREADINFO);
                if (GetGUIThreadInfo(dwThreadId, &gti))
                {
                    hFocusedWnd = gti.hwndFocus;
                }
            }
        }
        return hFocusedWnd;
    }

    tstr GetClipboardText(HWND hWndOwner )
    {
        tstr sText;

        if ( ::OpenClipboard(hWndOwner) )
        {
          #ifdef UNICODE
            const UINT uClipboardFormat = CF_UNICODETEXT;
          #else
            const UINT uClipboardFormat = CF_TEXT;
          #endif
            HANDLE hClipboardTextData = ::GetClipboardData(uClipboardFormat);
            if ( hClipboardTextData )
            {
                LPTSTR pszText = (LPTSTR) ::GlobalLock(hClipboardTextData);
                if ( pszText )
                {
                    sText = pszText;
                    ::GlobalUnlock(pszText);
                }
            }
            ::CloseClipboard();
        }

        return sText;
    }

    bool SetClipboardText(const tstr& text, HWND hWndOwner )
    {
        bool bSucceeded = false;

        if ( ::OpenClipboard(hWndOwner) )
        {
            HGLOBAL hTextMem = ::GlobalAlloc( GMEM_MOVEABLE, (text.length() + 1)*sizeof(TCHAR) );
            if ( hTextMem != NULL )
            {
                LPTSTR pszText = (LPTSTR) ::GlobalLock(hTextMem);
                if ( pszText != NULL )
                {
                    lstrcpy(pszText, text.c_str());
                    ::GlobalUnlock(hTextMem);

                    ::EmptyClipboard();

                  #ifdef UNICODE
                    const UINT uClipboardFormat = CF_UNICODETEXT;
                  #else
                    const UINT uClipboardFormat = CF_TEXT;
                  #endif
                    if ( ::SetClipboardData(uClipboardFormat, hTextMem) != NULL )
                        bSucceeded = true;
                }
            }
            ::CloseClipboard();
        }

        return bSucceeded;
    }

    tstr GetEnvironmentVariable(const TCHAR* szVarName)
    {
        tstr sValue;
        DWORD nLen = ::GetEnvironmentVariable(szVarName, NULL, 0);
        if ( nLen > 0 )
        {
            if ( sValue.Reserve(nLen + 1) )
            {
                nLen = ::GetEnvironmentVariable(szVarName, sValue.c_str(), nLen + 1);
                if ( nLen > 0 )
                {
                    sValue.SetLengthValue(nLen);
                }
            }
        }

        return sValue;
    }

    tstr GetEnvironmentVariable(const tstr& sVarName)
    {
        return GetEnvironmentVariable(sVarName.c_str());
    }

    bool IsFullPath(const tstr& path)
    {
        return IsFullPath( path.c_str() );
    }

    bool IsFullPath(const TCHAR* path)
    {
        if ( path[0] )  // not empty
        {
            switch ( path[1] )
            {
            case _T(':') :
                return true;                  // "X:..."
            case _T('\\') :
            case _T('/') :
                return (path[0] == path[1]);  // "\\..." or "//..."
            }
        }
        return false;
    }

    tstr NormalizePath(const tstr& path)
    {
        return NormalizePath(path.c_str());
    }

    tstr NormalizePath(const TCHAR* path)
    {
        bool isEndOfString = false;
        tstr part;
        CListT<tstr> pathParts;

        for ( const TCHAR* p = path; !isEndOfString; ++p )
        {
            const TCHAR ch = *p;
            if ( ch == 0 )
                isEndOfString = true;

            if ( isEndOfString || isPathSep(ch) )
            {
                if ( part.IsEmpty() )
                {
                    if ( pathParts.IsEmpty() && !isEndOfString )
                    {
                        const TCHAR next_ch = *(p + 1);
                        if ( isPathSep(next_ch) )
                        {
                            ++p;
                            pathParts.Add( _T("\\\\") );
                        }
                        else
                            pathParts.Add( _T("\\") );
                    }
                    continue;
                }

                if ( part == _T(".") )
                {
                    part.Clear();
                    continue;
                }

                if ( part == _T("..") )
                {
                    auto itr = pathParts.GetLast();
                    if ( itr )
                    {
                        if ( !isPathRoot(itr->GetItem()) )
                        {
                            pathParts.DeleteLast();
                            if ( isEndOfString )
                            {
                                itr = pathParts.GetLast();
                                if ( itr )
                                {
                                    tstr& path_part = itr->GetItem();
                                    if ( !isPathRoot(path_part) )
                                        path_part.DeleteLastChar();
                                }
                            }
                        }
                    }
                    part.Clear();
                    continue;
                }

                if ( isPathSep(ch) )
                    part += _T('\\');

                pathParts.Add(part);
                part.Clear();
            }
            else
                part += ch;
        }

        int expectedLength = 0;
        for ( auto itr = pathParts.GetFirst(); itr != NULL; itr = itr->GetNext() )
        {
            expectedLength += itr->GetItem().length();
        }

        tstr nrmPath;
        nrmPath.Reserve(expectedLength);

        for ( auto itr = pathParts.GetFirst(); itr != NULL; itr = itr->GetNext() )
        {
            nrmPath += itr->GetItem();
        }

        return nrmPath;
    }

    tstr GetFileNamePart(const TCHAR* path, eFileNamePart whichPart)
    {
        const TCHAR* p1;
        const TCHAR* p2;
        int n;

        switch ( whichPart )
        {
            case fnpDrive:   // "C" in "C:\User\Docs\name.ext"
                n = ::PathGetDriveNumber(path);
                if ( n >= 0 )
                {
                    return tstr(_T('A') + n);
                }
                break;

            case fnpDirPath:    // "C:\User\Docs\" in "C:\User\Docs\name.ext"
                p1 = path;
                p2 = ::PathFindFileName(p1);
                if ( p2 > p1 )
                {
                    return tstr(p1, static_cast<int>(p2 - p1));
                }
                else if ( ::PathIsRoot(p1) || ::PathIsDirectory(p1) )
                {
                    return tstr(p1);
                }
                break;

            case fnpNameExt: // "name.ext" in "C:\User\Docs\name.ext"
                p1 = ::PathFindFileName(path);
                if ( !::PathIsRoot(p1) && !::PathIsDirectory(p1) )
                {
                    return tstr(p1);
                }
                break;

            case fnpName:    // "name" in "C:\User\Docs\name.ext"
                p1 = ::PathFindFileName(path);
                if ( !::PathIsRoot(p1) && !::PathIsDirectory(p1) )
                {
                    p2 = ::PathFindExtension(path);
                    return ((p2 > p1) ? tstr(p1, static_cast<int>(p2 - p1)) : tstr(p1));
                }
                break;

            case fnpExt:     // ".ext" in "C:\User\Docs\name.ext"
                return tstr(::PathFindExtension(path));
        }

        return tstr(); // empty
    }

    tstr GetFileNamePart(const tstr& path, eFileNamePart whichPart)
    {
        return GetFileNamePart(path.c_str(), whichPart);
    }

    bool CreateDirectoryTree(const tstr& dir)
    {
        return impl_createDirectoryTree(dir);
    }

    bool CreateDirectoryTree(const TCHAR* dir)
    {
        return impl_createDirectoryTree(dir);
    }

    bool CheckDirectoryExists(const tstr& dir)
    {
        return CheckDirectoryExists(dir.c_str());
    }

    bool CheckDirectoryExists(const TCHAR* dir)
    {
        DWORD dwAttr = ::GetFileAttributes(dir);
        return ((dwAttr != INVALID_FILE_ATTRIBUTES) && ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) != 0));
    }

    bool CheckFileExists(const tstr& filename)
    {
        return CheckFileExists(filename.c_str());
    }

    bool CheckFileExists(const TCHAR* filename)
    {
        DWORD dwAttr = ::GetFileAttributes(filename);
        return ((dwAttr != INVALID_FILE_ATTRIBUTES) && ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == 0));
    }

    tstr GetInstanceAsString(const void* pInstance)
    {
        tstr S;
        SYSTEMTIME t;

        ::GetLocalTime(&t);
        S.Format(60, _T("0x%X @ %02d:%02d:%02d.%03d"), pInstance, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
        return S;
    }

    void StrLower(tstr& S)
    {
        if ( S.length() != 0 )
            ::CharLower( S.c_str() );
    }

    void StrLower(TCHAR* S)
    {
        ::CharLower( S );
    }

    void StrUpper(tstr& S)
    {
        if ( S.length() != 0 )
            ::CharUpper( S.c_str() );
    }

    void StrUpper(TCHAR* S)
    {
        ::CharUpper( S );
    }

    void StrQuote(tstr& S)
    {
        if ( IsStrNotQuoted(S) )
        {
            S.Append( _T('\"') );
            S.Insert( 0, _T('\"') );
        }
    }

    void StrUnquote(tstr& S)
    {
        if ( IsStrQuoted(S) )
        {
            S.DeleteLastChar();
            S.DeleteFirstChar();
        }
    }

    bool IsStrQuoted(const tstr& S)
    {
        return ( (S.length() > 1) && (S.GetFirstChar() == _T('\"')) && (S.GetLastChar() == _T('\"')) );
    }

    bool IsStrNotQuoted(const tstr& S)
    {
        return ( (S.GetFirstChar() != _T('\"')) && (S.GetLastChar()  != _T('\"')) );
    }

    int StrCmpNoCase(const tstr& S1, const tstr& S2)
    {
        return strCompareNoCase( S1.c_str(), S1.length(), 
                                 S2.c_str(), S2.length() );
    }

    int StrCmpNoCase(const tstr& S1, const TCHAR* S2)
    {
        return strCompareNoCase( S1.c_str(), S1.length(), 
                                 S2 ? S2 : _T(""), GetStrSafeLength(S2) );
    }

    int StrCmpNoCase(const TCHAR* S1, const tstr& S2)
    {
        return strCompareNoCase( S1 ? S1 : _T(""), GetStrSafeLength(S1), 
                                 S2.c_str(), S2.length() );
    }

    int StrCmpNoCase(const TCHAR* S1, const TCHAR* S2)
    {
        return strCompareNoCase( S1 ? S1 : _T(""), GetStrSafeLength(S1), 
                                 S2 ? S2 : _T(""), GetStrSafeLength(S2) );
    }

    void StrDelLeadingTabSpaces(CStrT<char>& S)
    {
        int i = 0;
        while ( IsTabSpaceChar(S.GetAt(i)) )
        {
            ++i;
        }
        S.Delete(0, i);
    }

    void StrDelLeadingTabSpaces(CStrT<wchar_t>& S)
    {
        int i = 0;
        while ( IsTabSpaceChar(S.GetAt(i)) )
        {
            ++i;
        }
        S.Delete(0, i);
    }

    void StrDelTrailingTabSpaces(CStrT<char>& S)
    {
        int i = S.length() - 1;
        while ( IsTabSpaceChar(S.GetAt(i)) )
        {
            --i;
        }
        S.Delete(i + 1, -1);
    }

    void StrDelTrailingTabSpaces(CStrT<wchar_t>& S)
    {
        int i = S.length() - 1;
        while ( IsTabSpaceChar(S.GetAt(i)) )
        {
            --i;
        }
        S.Delete(i + 1, -1);
    }

    CWStr CStrToWStr(const CStr& S, UINT aCodePage )
    {
        if ( S.IsEmpty() )
            return CWStr();

        int nLenW = 0;
        wchar_t* pStrW = SysUniConv::newMultiByteToUnicode( S.c_str(), S.length(), aCodePage, &nLenW );
        CWStr ret(pStrW, nLenW);
        delete [] pStrW;
        return ret;
    }

    CStr WStrToCStr(const CWStr& S, UINT aCodePage )
    {
        if ( S.IsEmpty() )
            return CStr();

        int nLenA = 0;
        char* pStrA = SysUniConv::newUnicodeToMultiByte( S.c_str(), S.length(), aCodePage, &nLenA );
        CStr ret(pStrA, nLenA);
        delete [] pStrA;
        return ret;
    }

    tstr CStrToTStr(const CStr& S, UINT aCodePage )
    {
      #ifdef UNICODE
        return CStrToWStr(S, aCodePage);
      #else
        return S;
      #endif
    }

    CStr TStrToCStr(const tstr& S, UINT aCodePage )
    {
      #ifdef UNICODE
        return WStrToCStr(S, aCodePage);
      #else
        return S;
      #endif
    }
}

//-------------------------------------------------------------------------

/*
CValue::CValue() : m_type(vtNone)
{
}

CValue::CValue(int i) : m_type(vtInt)
{
    m_data.i = i;
}

CValue::CValue(unsigned int u) : m_type(vtUint)
{
    m_data.u = u;
}

CValue::CValue(long l) : m_type(vtLong)
{
    m_data.l = l;
}

CValue::CValue(unsigned long ul) : m_type(vtUlong)
{
    m_data.ul = ul;
}

CValue::CValue(long long ll) : m_type(vtLongLong)
{
    m_data.ll = ll;
}

CValue::CValue(unsigned long long ull) : m_type(vtUlongLong)
{
    m_data.ull = ull;
}

CValue::CValue(bool b) : m_type(vtBool)
{
    m_data.u = b ? 1 : 0;
}

CValue::CValue(float f) : m_type(vtFloat)
{
    m_data.f = f;
}

CValue::CValue(double d) : m_type(vtDouble)
{
    m_data.d = d;
}

CValue::CValue(std::nullptr_t) : m_type(vtPtr)
{
    m_data.p = nullptr;
}

CValue::CValue(const void* p) : m_type(vtPtr)
{
    m_data.p = p;
}

CValue::CValue(const TCHAR* s) : m_type(vtStr)
{
    setStrData(s, s != nullptr ? lstrlen(s) : 0);
}

CValue::CValue(const TCHAR* s, size_t len)
{
    setStrData(s, len);
}

CValue::CValue(const CValue& v) : m_type(v.m_type)
{
    setData(v);
}

CValue::CValue(CValue&& v) : m_type(vtNone)
{
    Swap(v);
}

CValue::~CValue()
{
    clearData();
}

CValue& CValue::operator=(const CValue& v)
{
    clearData();
    m_type = vtNone;
    setData(v);
    m_type = v.m_type;
    return *this;
}

CValue& CValue::operator=(CValue&& v)
{
    clearData();
    m_type = vtNone;
    Swap(v);
    return *this;
}

CValue::eValueType CValue::GetType() const
{
    return m_type;
}

int CValue::GetInt() const
{
    return getNumber<int>();
}

unsigned int CValue::GetUint() const
{
    return getNumber<unsigned int>();
}

long CValue::GetLong() const
{
    return getNumber<long>();
}

unsigned long CValue::GetUlong() const
{
    return getNumber<unsigned long>();
}

long long CValue::GetLongLong() const
{
    return getNumber<long long>();
}

unsigned long long CValue::GetUlongLong() const
{
    return getNumber<unsigned long long>();
}

bool CValue::GetBool() const
{
    bool b = false;
    switch ( m_type )
    {
        case vtBool:
        case vtUint:
            b = (m_data.u != 0);
            break;
        case vtInt:
            b = (m_data.i != 0);
            break;
        case vtLong:
            b = (m_data.l != 0);
            break;
        case vtUlong:
            b = (m_data.ul != 0);
            break;
        case vtLongLong:
            b = (m_data.ll != 0);
            break;
        case vtUlongLong:
            b = (m_data.ull != 0);
            break;
        case vtFloat:
            b = (m_data.f != (float)(0));
            break;
        case vtDouble:
            b = (m_data.d != (double)(0));
            break;
        case vtPtr:
            b = (m_data.p != nullptr);
            break;
        case vtStr:
            b = (m_data.s.str != nullptr && m_data.s.len != 0);
            break;
    }
    return b;
}

float CValue::GetFloat() const
{
    return getNumber<float>();
}

double CValue::GetDouble() const
{
    return getNumber<double>();
}

const void* CValue::GetPtr() const
{
    const void* p = nullptr;
    switch ( m_type )
    {
        case vtPtr:
            p = m_data.p;
            break;
        case vtStr:
            p = m_data.s.str;
            break;
    }
    return p;
}

const TCHAR* CValue::GetStr(size_t* len) const
{
    static const TCHAR emptyStr[] = _T("");
    switch ( m_type )
    {
        case vtStr:
            if ( m_data.s.len != 0 )
            {
                if ( len != nullptr )
                    *len = m_data.s.len;
                return m_data.s.str;
            }
            break;
    }
    if ( len != nullptr )
        *len = 0;
    return emptyStr;
}

template<> int CValue::GetValue<int>() const
{
    return GetInt();
}

template<> unsigned int CValue::GetValue<unsigned int>() const
{
    return GetUint();
}

template<> long CValue::GetValue<long>() const
{
    return GetLong();
}

template<> unsigned long CValue::GetValue<unsigned long>() const
{
    return GetUlong();
}

template<> long long CValue::GetValue<long long>() const
{
    return GetLongLong();
}

template<> unsigned long long CValue::GetValue<unsigned long long>() const
{
    return GetUlongLong();
}

template<> bool CValue::GetValue<bool>() const
{
    return GetBool();
}

template<> float CValue::GetValue<float>() const
{
    return GetFloat();
}

template<> double CValue::GetValue<double>() const
{
    return GetDouble();
}

template<> const void* CValue::GetValue<const void*>() const
{
    return GetPtr();
}

template<> const TCHAR* CValue::GetValue<const TCHAR*>() const
{
    return GetStr();
}

void CValue::Swap(CValue& v)
{
    uData tempData;
    eValueType tempType;

    if ( m_type != vtNone)
    {
        memcpy(&tempData, &m_data, sizeof(uData));
    }
    tempType = m_type;

    if ( v.m_type != vtNone )
    {
        memcpy(&m_data, &v.m_data, sizeof(uData));
    }
    m_type = v.m_type;

    if ( tempType != vtNone)
    {
        memcpy(&v.m_data, &tempData, sizeof(uData));
    }
    v.m_type = tempType;
}

template<typename N> N CValue::getNumber() const
{
    N n = 0;
    switch ( m_type )
    {
        case vtInt:
            n = static_cast<N>(m_data.i);
            break;
        case vtUint:
            n = static_cast<N>(m_data.u);
            break;
        case vtLong:
            n = static_cast<N>(m_data.l);
            break;
        case vtUlong:
            n = static_cast<N>(m_data.ul);
            break;
        case vtLongLong:
            n = static_cast<N>(m_data.ll);
            break;
        case vtUlongLong:
            n = static_cast<N>(m_data.ull);
            break;
        case vtBool:
            n = static_cast<N>(m_data.u != 0 ? 1 : 0);
            break;
        case vtFloat:
            n = static_cast<N>(m_data.f);
            break;
        case vtDouble:
            n = static_cast<N>(m_data.d);
            break;
    }
    return n;
}

void CValue::setStrData(const TCHAR* s, size_t len)
{
    if ( len != 0 )
    {
        m_data.s.str = new TCHAR[len + 1];
        lstrcpy(m_data.s.str, s);
    }
    else
    {
        m_data.s.str = nullptr;
    }
    m_data.s.len = len;
}

void CValue::setData(const CValue& v)
{
    if ( v.m_type == vtStr )
    {
        setStrData(v.m_data.s.str, v.m_data.s.len);
    }
    else if ( v.m_type != vtNone )
    {
        memcpy(&m_data, &v.m_data, sizeof(uData));
    }
}

void CValue::clearData()
{
    if ( m_type == vtStr )
    {
        if ( m_data.s.str != nullptr )
        {
            delete [] m_data.s.str;
            m_data.s.len = 0;
        }
    }
}
*/

//-------------------------------------------------------------------------

