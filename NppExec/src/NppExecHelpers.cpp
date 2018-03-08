#include "NppExecHelpers.h"

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

//-------------------------------------------------------------------------

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

    void StrUnquote(tstr& S)
    {
        if ( (S.GetFirstChar() == _T('\"')) && (S.GetLastChar() == _T('\"')) )
        {
            S.DeleteLastChar();
            S.DeleteFirstChar();
        }
    }
}

//-------------------------------------------------------------------------
