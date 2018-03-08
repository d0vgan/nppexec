// >>>>>>>>>>>> str_func v.1.0.1 (November 2007) >>>>>>>>>>>>
// >>>>

#include "str_func.h"


#ifdef __cplusplus
namespace c_base {
#endif


int str_safe_cmp( const char* str1, const char* str2 )
{
    if ( str1 )
    {
        if ( str2 )
        {
            return str_unsafe_cmp( str1, str2 );
        }
        return ( str1[0] ? 1 : 0 );  // is str1 not empty?
    }
    return ( (str2 && str2[0]) ? (-1) : 0 );  // is str2 not empty?
}

// CONDITION: str1 != 0 && str2 != 0
int str_unsafe_cmp( const char* str1, const char* str2 )
{
    while ( (*str1) && (*str1 == *str2) )
    {
        ++str1;
        ++str2;
    }
    return (int) ( ((unsigned int) (*str1)) - ((unsigned int) (*str2)) );
}

int str_safe_cmpn( const char* str1, const char* str2, int len )
{
    if ( len >= 0 )
    {
        if ( str1 )
        {
            if ( str2 )
            {
                return str_unsafe_cmpn( str1, str2, len );
            }
            return ( str1[0] ? 1 : 0 );  // is str1 not empty?
        }
        return ( (str2 && str2[0]) ? (-1) : 0 );  // is str2 not empty?
    }
    return 0;
}

// CONDITION: str1 != 0 && str2 != 0 && len >= 0
int str_unsafe_cmpn( const char* str1, const char* str2, int len )
{
    while ( (--len > 0) && (*str1) && (*str1 == *str2) )
    {
        ++str1;
        ++str2;
    }
    return ( (len >= 0) ? ( (int) (((unsigned int) (*str1)) - ((unsigned int) (*str2))) ) : 0 );
}

int str_safe_cpy( char* dst, const char* src )
{
    if ( dst )
    {
        if ( src )
        {
            return str_unsafe_cpy( dst, src );
        }
        *dst = 0;
    }
    return 0;
}

// CONDITION: dst != 0 && src != 0
int str_unsafe_cpy( char* dst, const char* src )
{
    char* dst0 = dst;
    while ( (*(dst++) = *(src++)) != 0 ) ;
    return (int) (dst - dst0 - 1);
}

int str_safe_cpyn( char* dst, const char* src, int len )
{
    if ( dst )
    {
        if ( src && (len > 0) )
        {
            return str_unsafe_cpyn( dst, src, len );
        }
        *dst = 0;
    }
    return 0;
} 

// CONDITION: dst != 0 && src != 0 && len >= 0
int str_unsafe_cpyn( char* dst, const char* src, int len )
{
    int len0 = len;
    if ( (dst > src) && (dst < src + len) )
    {
        src += len;
        dst += len;
        *dst = 0;
        while ( len-- > 0 )
        {
            *(--dst) = *(--src);
        }
    }
    else
    {
        while ( (len-- > 0) && (*dst = *src) )
        {
            ++dst;
            ++src;
        }
        *dst = 0;
        len0 -= (len + 1);
    }
    return len0;
} 

int str_safe_find( const char* str, const char* substr )
{
    return ( (str && substr) ? str_unsafe_find(str, substr) : (-1) );
}

static int str_sub_cmp( const char* str, const char* substr )
{
    while ( (*str) && (*str == *substr) )
    {
        ++str;
        ++substr;
    }
    if ( !(*substr) )
    {
        return 0;
    }
    return (int) ( ((unsigned int) (*str)) - ((unsigned int) (*substr)) );
}

// CONDITION: str != 0 && substr != 0
int str_unsafe_find( const char* str, const char* substr )
{
    if ( substr[0] )
    {
        const char* str0 = str;
        while ( *str )
        {
            if ( (*str != *substr) || (str_sub_cmp(str+1, substr+1) != 0) )
                ++str;
            else
                return ( (int) (str - str0) );
        }
    }
    return -1;
}

// does not find ch == 0
int str_safe_findch( const char* str, const char ch )
{
    return ( str ? str_unsafe_findch(str, ch) : (-1) );
}

// does not find ch == 0
// CONDITION: str != 0
int str_unsafe_findch( const char* str, const char ch )
{
    const char* str0 = str;
    while ( *str )
    {
        if ( *str != ch )
            ++str;
        else
            return ( (int) (str - str0) );
    }
    return -1;
}

int str_safe_findoneof( const char* str, const char* charset )
{
    return ( (str && charset) ? str_unsafe_findoneof(str, charset) : (-1) );
}

// CONDITION: str != 0 && charset != 0
int str_unsafe_findoneof( const char* str, const char* charset )
{
    if ( charset[0] )
    {
        const char* str0 = str;
        const char* charset0 = charset;
        while ( *str )
        {
            while ( *charset )
            {
                if ( *str == *(charset++) )
                    return ( (int) (str - str0) );
            }
            charset = charset0;
            ++str;
        }
    }
    return -1;
}

int str_safe_len( const char* str )
{
    return ( str ? str_unsafe_len(str) : 0 );
}

// CONDITION: str != 0
int str_unsafe_len( const char* str )
{
    const char* str0 = str;
    while ( *str )  ++str;
    return ( (int) (str - str0) );
}

void str_safe_reverse( char* str )
{
    int len = str_safe_len(str);
    if ( len > 0 )
    {
        str_unsafe_reversen(str, len);
    }
}

// CONDITION: str != 0
void str_unsafe_reverse( char* str )
{
    int len = str_unsafe_len(str);
    if ( len > 0 )
    {
        str_unsafe_reversen(str, len);
    }
}

void str_safe_reversen( char* str, int len )
{
    if ( str && (len > 0) )
    {
        str_unsafe_reversen(str, len);
    }
}

// CONDITION: str != 0 && len >= 0
void str_unsafe_reversen( char* str, int len )
{
    char  ch;
    char* p = str + len - 1;
    while ( str < p )
    {
        ch = *str;
        *(str++) = *p;
        *(p--) = ch;
    }
}

int str_safe_rfind( const char* str, int len, const char* substr )
{
    return ( (str && (len > 0) && substr) ? str_unsafe_rfind(str, len, substr) : (-1) );
}

// CONDITION: str != 0 && len >= 0 && substr != 0
int str_unsafe_rfind( const char* str, int len, const char* substr )
{
    if ( substr[0] )
    {
        const char* str0 = str;
        str += len;
        while ( len-- > 0 )
        {
            --str;
            if ( (*str == *substr) && (str_sub_cmp(str+1, substr+1) == 0) )
                return ( (int) (str - str0) );
        }
    }
    return -1;
}

int str_safe_rfindch( const char* str, int len, const char ch )
{
    return ( (str && (len > 0)) ? str_unsafe_rfindch(str, len, ch) : (-1) );
}

// CONDITION: str != 0 && len >= 0
int str_unsafe_rfindch( const char* str, int len, const char ch )
{
    const char* str0 = str;
    str += len;
    while ( len-- > 0 )
    {
        --str;
        if ( *str == ch )
            return ( (int) (str - str0) );
    }
    return -1;
}

int str_safe_rfindoneof( const char* str, int len, const char* charset )
{
    return ( (str && (len > 0) && charset) ? str_unsafe_rfindoneof(str, len, charset) : (-1) );
}

// CONDITION: str != 0 && len >= 0 && charset != 0
int str_unsafe_rfindoneof( const char* str, int len, const char* charset )
{
    if ( charset[0] )
    {
        const char* str0 = str;
        const char* charset0 = charset;
        str += len;
        while ( len-- > 0 )
        {
            --str;
            while ( *charset )
            {
                if ( *str == *(charset++) )
                    return ( (int) (str - str0) );
            }
            charset = charset0;
        }
    }
    return -1;
}

char* str_safe_rskip_ch( const char* str, int len, const char ch )
{
    return ( (str && (len > 0)) ? str_unsafe_rskip_ch(str, len, ch) : ((char *) str) );
}

// CONDITION: str != 0 && len >= 0
char* str_unsafe_rskip_ch( const char* str, int len, const char ch )
{
    str += len;    
    if ( ch )
    {
        while ( len-- > 0 )
        { 
            if ( *(--str) != ch ) 
                break;
        }
    }
    else if ( len > 0 )
    {
        --str;
    }
    return ( (char *) str );
}

char* str_safe_rskip_tabspaces( const char* str, int len )
{
    return ( (str && (len > 0)) ? str_unsafe_rskip_tabspaces(str, len) : ((char *) str) );
}

// CONDITION: str != 0 && len >= 0
char* str_unsafe_rskip_tabspaces( const char* str, int len )
{
    str += len;
    while ( len-- > 0)
    {
        --str;
        if ((*str != ' ') && (*str != '\t'))
            break;
    }
    return ( (char *) str );
}

char* str_safe_skip_ch( const char* str, const char ch )
{
    return ( str ? str_unsafe_skip_ch(str, ch) : ((char *) str) );
}

// CONDITION: str != 0
char* str_unsafe_skip_ch( const char* str, const char ch )
{
    if ( ch )
    {
        while ( *str == ch )
        {
            ++str;
        }
    }
    return ( (char *) str );
}

char* str_safe_skip_tabspaces( const char* str )
{
    return ( str ? str_unsafe_skip_tabspaces(str) : ((char *) str) );
}

// CONDITION: str != 0
char* str_unsafe_skip_tabspaces( const char* str )
{
    while ( (*str == ' ') || (*str == '\t') )
    {
        ++str;
    }
    return ( (char *) str );
}

//---------------------------------------------------------------------------

int strw_safe_cmp( const wchar_t* strw1, const wchar_t* strw2 )
{
    if ( strw1 )
    {
        if ( strw2 )
        {
            return strw_unsafe_cmp( strw1, strw2 );
        }
        return ( strw1[0] ? 1 : 0 );  // is strw1 not empty?
    }
    return ( (strw2 && strw2[0]) ? (-1) : 0 );  // is strw2 not empty?
}

// CONDITION: strw1 != 0 && strw2 != 0
int strw_unsafe_cmp( const wchar_t* strw1, const wchar_t* strw2 )
{
    while ( (*strw1) && (*strw1 == *strw2) )
    {
        ++strw1;
        ++strw2;
    }
    return (int) ( ((unsigned int) (*strw1)) - ((unsigned int) (*strw2)) );
}

int strw_safe_cmpn( const wchar_t* strw1, const wchar_t* strw2, int len )
{
    if ( len >= 0 )
    {
        if ( strw1 )
        {
            if ( strw2 )
            {
                return strw_unsafe_cmpn( strw1, strw2, len );
            }
            return ( strw1[0] ? 1 : 0 );  // is strw1 not empty?
        }
        return ( (strw2 && strw2[0]) ? (-1) : 0 );  // is strw2 not empty?
    }
    return 0;
}

// CONDITION: strw1 != 0 && strw2 != 0 && len >= 0
int strw_unsafe_cmpn( const wchar_t* strw1, const wchar_t* strw2, int len )
{
    while ( (--len > 0) && (*strw1) && (*strw1 == *strw2) )
    {
        ++strw1;
        ++strw2;
    }
    return ( (len >= 0) ? ( (int) (((unsigned int) (*strw1)) - ((unsigned int) (*strw2))) ) : 0 );
}

int strw_safe_cpy( wchar_t* dstw, const wchar_t* srcw )
{
    if ( dstw )
    {
        if ( srcw )
        {
            return strw_unsafe_cpy( dstw, srcw );
        }
        *dstw = 0;
    }
    return 0;
}

// CONDITION: dstw != 0 && srcw != 0
int strw_unsafe_cpy( wchar_t* dstw, const wchar_t* srcw )
{
    wchar_t* dstw0 = dstw;
    while ( (*(dstw++) = *(srcw++)) != 0 ) ;
    return (int) (dstw - dstw0 - 1);
}

int strw_safe_cpyn( wchar_t* dstw, const wchar_t* srcw, int len )
{
    if ( dstw )
    {
        if ( srcw && (len > 0) )
        {
            return strw_unsafe_cpyn( dstw, srcw, len );
        }
        *dstw = 0;
    }
    return 0;
} 

// CONDITION: dstw != 0 && srcw != 0 && len >= 0
int strw_unsafe_cpyn( wchar_t* dstw, const wchar_t* srcw, int len )
{
    int len0 = len;
    if ( (dstw > srcw) && (dstw < srcw + len) )
    {
        srcw += len;
        dstw += len;
        *dstw = 0;
        while ( len-- > 0 )
        {
            *(--dstw) = *(--srcw);
        }
    }
    else
    {
        while ( (len-- > 0) && (*dstw = *srcw) )
        {
            ++dstw;
            ++srcw;
        }
        *dstw = 0;
        len0 -= (len + 1);
    }
    return len0;
} 

int strw_safe_find( const wchar_t* strw, const wchar_t* substrw )
{
    return ( (strw && substrw) ? strw_unsafe_find(strw, substrw) : (-1) );
}

static int strw_sub_cmp( const wchar_t* strw, const wchar_t* substrw )
{
    while ( (*strw) && (*strw == *substrw) )
    {
        ++strw;
        ++substrw;
    }
    if ( !(*substrw) )
    {
        return 0;
    }
    return (int) ( ((unsigned int) (*strw)) - ((unsigned int) (*substrw)) );
}

// CONDITION: strw != 0 && substrw != 0
int strw_unsafe_find( const wchar_t* strw, const wchar_t* substrw )
{
    if ( substrw[0] )
    {
        const wchar_t* strw0 = strw;
        while ( *strw )
        {
            if ( (*strw != *substrw) || (strw_sub_cmp(strw+1, substrw+1) != 0) )
                ++strw;
            else
                return ( (int) (strw - strw0) );
        }
    }
    return -1;
}

// does not find wch == 0
int strw_safe_findch( const wchar_t* strw, const wchar_t wch )
{
    return ( strw ? strw_unsafe_findch(strw, wch) : (-1) );
}

// does not find wch == 0
// CONDITION: strw != 0
int strw_unsafe_findch( const wchar_t* strw, const wchar_t wch )
{
    const wchar_t* strw0 = strw;
    while ( *strw )
    {
        if ( *strw != wch )
            ++strw;
        else
            return ( (int) (strw - strw0) );
    }
    return -1;
}

int strw_safe_findoneof( const wchar_t* strw, const wchar_t* charsetw )
{
    return ( (strw && charsetw) ? strw_unsafe_findoneof(strw, charsetw) : (-1) );
}

// CONDITION: strw != 0 && charsetw != 0
int strw_unsafe_findoneof( const wchar_t* strw, const wchar_t* charsetw )
{
    if ( charsetw[0] )
    {
        const wchar_t* strw0 = strw;
        const wchar_t* charsetw0 = charsetw;
        while ( *strw )
        {
            while ( *charsetw )
            {
                if ( *strw == *(charsetw++) )
                    return ( (int) (strw - strw0) );
            }
            charsetw = charsetw0;
            ++strw;
        }
    }
    return -1;
}

int strw_safe_len( const wchar_t* strw )
{
    return ( strw ? strw_unsafe_len(strw) : 0 );
}

// CONDITION: strw != 0
int strw_unsafe_len( const wchar_t* strw )
{
    const wchar_t* strw0 = strw;
    while ( *strw )  ++strw;
    return ( (int) (strw - strw0) );
}

void strw_safe_reverse( wchar_t* strw )
{
    int len = strw_safe_len(strw);
    if ( len > 0 )
    {
        strw_unsafe_reversen(strw, len);
    }
}

// CONDITION: strw != 0
void strw_unsafe_reverse( wchar_t* strw )
{
    int len = strw_unsafe_len(strw);
    if ( len > 0 )
    {
        strw_unsafe_reversen(strw, len);
    }
}

void strw_safe_reversen( wchar_t* strw, int len )
{
    if ( strw && (len > 0) )
    {
        strw_unsafe_reversen(strw, len);
    }
}

// CONDITION: strw != 0 && len >= 0
void strw_unsafe_reversen( wchar_t* strw, int len )
{
    wchar_t  wch;
    wchar_t* pw = strw + len - 1;
    while ( strw < pw )
    {
        wch = *strw;
        *(strw++) = *pw;
        *(pw--) = wch;
    }
}

int strw_safe_rfind( const wchar_t* strw, int len, const wchar_t* substrw )
{
    return ( (strw && (len > 0) && substrw) ? strw_unsafe_rfind(strw, len, substrw) : (-1) );
}

// CONDITION: strw != 0 && len >= 0 && substrw != 0
int strw_unsafe_rfind( const wchar_t* strw, int len, const wchar_t* substrw )
{
    if ( substrw[0] )
    {
        const wchar_t* strw0 = strw;
        strw += len;
        while ( len-- > 0 )
        {
            --strw;
            if ( (*strw == *substrw) && (strw_sub_cmp(strw+1, substrw+1) == 0) )
                return ( (int) (strw - strw0) );
        }
    }
    return -1;
}

int strw_safe_rfindch( const wchar_t* strw, int len, const wchar_t wch )
{
    return ( (strw && (len > 0)) ? strw_unsafe_rfindch(strw, len, wch) : (-1) );
}

// CONDITION: strw != 0 && len >= 0
int strw_unsafe_rfindch( const wchar_t* strw, int len, const wchar_t wch )
{
    const wchar_t* strw0 = strw;
    strw += len;
    while ( len-- > 0 )
    {
        --strw;
        if ( *strw == wch )
            return ( (int) (strw - strw0) );
    }
    return -1;
}

int strw_safe_rfindoneof( const wchar_t* strw, int len, const wchar_t* charsetw )
{
    return ( (strw && (len > 0) && charsetw) ? strw_unsafe_rfindoneof(strw, len, charsetw) : (-1) );
}

// CONDITION: strw != 0 && len >= 0 && charsetw != 0
int strw_unsafe_rfindoneof( const wchar_t* strw, int len, const wchar_t* charsetw )
{
    if ( charsetw[0] )
    {
        const wchar_t* strw0 = strw;
        const wchar_t* charsetw0 = charsetw;
        strw += len;
        while ( len-- > 0 )
        {
            --strw;
            while ( *charsetw )
            {
                if ( *strw == *(charsetw++) )
                    return ( (int) (strw - strw0) );
            }
            charsetw = charsetw0;
        }
    }
    return -1;
}

wchar_t* strw_safe_rskip_ch( const wchar_t* strw, int len, const wchar_t wch )
{
    return ( (strw && (len > 0)) ? strw_unsafe_rskip_ch(strw, len, wch) : ((wchar_t *) strw) );
}

// CONDITION: strw != 0 && len >= 0
wchar_t* strw_unsafe_rskip_ch( const wchar_t* strw, int len, const wchar_t wch )
{
    strw += len;    
    if ( wch )
    {
        while ( len-- > 0 )
        { 
            if ( *(--strw) != wch ) 
                break;
        }
    }
    else if ( len > 0 )
    {
        --strw;
    }
    return ( (wchar_t *) strw );
}

wchar_t* strw_safe_rskip_tabspaces( const wchar_t* strw, int len )
{
    return ( (strw && (len > 0)) ? strw_unsafe_rskip_tabspaces(strw, len) : ((wchar_t *) strw) );
}

// CONDITION: strw != 0 && len >= 0
wchar_t* strw_unsafe_rskip_tabspaces( const wchar_t* strw, int len )
{
    strw += len;
    while ( len-- > 0)
    {
        --strw;
        if ((*strw != L' ') && (*strw != L'\t'))
            break;
    }
    return ( (wchar_t *) strw );
}

wchar_t* strw_safe_skip_ch( const wchar_t* strw, const wchar_t wch )
{
    return ( strw ? strw_unsafe_skip_ch(strw, wch) : ((wchar_t *) strw) );
}

// CONDITION: strw != 0
wchar_t* strw_unsafe_skip_ch( const wchar_t* strw, const wchar_t wch )
{
    if ( wch )
    {
        while ( *strw == wch )
        {
            ++strw;
        }
    }
    return ( (wchar_t *) strw );
}

wchar_t* strw_safe_skip_tabspaces( const wchar_t* strw )
{
    return ( strw ? strw_unsafe_skip_tabspaces(strw) : ((wchar_t *) strw) );
}

// CONDITION: strw != 0
wchar_t* strw_unsafe_skip_tabspaces( const wchar_t* strw )
{
    while ( (*strw == L' ') || (*strw == L'\t') )
    {
        ++strw;
    }
    return ( (wchar_t *) strw );
}


#ifdef __cplusplus
}
#endif
