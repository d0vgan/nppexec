#ifndef _str_func_h_
#define _str_func_h_
//---------------------------------------------------------------------------
#include "../base.h"
#include "types.h"

#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif

int is_any_space_char( char ch );

// returns:  == 0  if matched
//            > 0  if str1 > str2
//            < 0  if str1 < str2
int   str_safe_cmp( const char* str1, const char* str2 ); 
int   str_safe_cmpn( const char* str1, const char* str2, int len );

// returns length of dst
int   str_safe_cpy( char* dst, const char* src );   

// src can be part of dst
// returns length of dst
int   str_safe_cpyn( char* dst, const char* src, int len ); 

// returns index of first substr in str or -1
int   str_safe_find( const char* str, const char* substr );

// does not find ch == 0
// returns index of first ch in str or -1
int   str_safe_findch( const char* str, const char ch );

// returns index of first character from charset in str or -1
int   str_safe_findoneof( const char* str, const char* charset );

// calculates length of str
int   str_safe_len( const char* str ); 

// reverse order of characters (first character becomes last etc.)
// this function modifies the original string
void  str_safe_reverse( char* str );
void  str_safe_reversen( char* str, int len );

// returns index of last substr in str or -1
int   str_safe_rfind( const char* str, int len, const char* substr );

// returns index of last ch in str or -1
int   str_safe_rfindch( const char* str, int len, const char ch );

// returns index of last character from charset in str or -1
int   str_safe_rfindoneof( const char* str, int len, const char* charset );

// skips trailing characters ch
// returns a pointer to last character != ch
char* str_safe_rskip_ch( const char* str, int len, const char ch );

// skips trailing tabs and spaces
// returns a pointer to last non-anyspace character in str
char* str_safe_rskip_anyspaces( const char* str, int len );

// skips leading characters ch
// returns a pointer to first character != ch
char* str_safe_skip_ch( const char* str, const char ch );

// skips leading tabs and spaces
// returns a pointer to first non-anyspace character in str
char* str_safe_skip_anyspaces( const char* str );

// returns:  == 0  if matched
//            > 0  if str1 > str2
//            < 0  if str1 < str2
// CONDITION: str1 != 0 && str2 != 0
int   str_unsafe_cmp( const char* str1, const char* str2 ); 
// CONDITION: str1 != 0 && str2 != 0 && len >= 0
int   str_unsafe_cmpn( const char* str1, const char* str2, int len );

// returns length of dst
// CONDITION: dst != 0 && src != 0
int   str_unsafe_cpy( char* dst, const char* src );   

// src can be part of dst
// returns length of dst
// CONDITION: dst != 0 && src != 0 && len >= 0
int   str_unsafe_cpyn( char* dst, const char* src, int len ); 

// returns index of first substr in str or -1
// CONDITION: str != 0 && substr != 0
int   str_unsafe_find( const char* str, const char* substr );

// does not find ch == 0
// returns index of first ch in str or -1
// CONDITION: str != 0
int   str_unsafe_findch( const char* str, const char ch );

// returns index of first character from charset in str or -1
// CONDITION: str != 0 && charset != 0
int   str_unsafe_findoneof( const char* str, const char* charset );

// calculates length of str
// CONDITION: str != 0
int   str_unsafe_len( const char* str ); 

// reverse order of characters (first character becomes last etc.)
// this function modifies the original string
// CONDITION: str != 0
void str_unsafe_reverse( char* str );
// CONDITION: str != 0 && len >= 0
void str_unsafe_reversen( char* str, int len );

// returns index of last substr in str or -1
// CONDITION: str != 0 && len >= 0 && substr != 0
int   str_unsafe_rfind( const char* str, int len, const char* substr );

// returns index of last ch in str or -1
// CONDITION: str != 0 && len >= 0
int   str_unsafe_rfindch( const char* str, int len, const char ch );

// returns index of last character from charset in str or -1
// CONDITION: str != 0 && len >= 0 && charset != 0
int   str_unsafe_rfindoneof( const char* str, int len, const char* charset );

// skips trailing characters ch
// returns a pointer to last character != ch
// CONDITION: str != 0 && len >= 0
char* str_unsafe_rskip_ch( const char* str, int len, const char ch );

// skips trailing tabs and spaces
// returns a pointer to last non-anyspace character in str
// CONDITION: str != 0 && len >= 0
char* str_unsafe_rskip_anyspaces( const char* str, int len );

// skips leading characters ch
// returns a pointer to first character != ch
// CONDITION: str != 0
char* str_unsafe_skip_ch( const char* str, const char ch );

// skips leading tabs and spaces
// returns a pointer to first non-anyspace character in str
// CONDITION: str != 0
char* str_unsafe_skip_anyspaces( const char* str );

//---------------------------------------------------------------------------


int is_any_space_charw( wchar_t ch );

// returns:  == 0  if matched
//            > 0  if strw1 > strw2
//            < 0  if strw1 < strw2
int   strw_safe_cmp( const wchar_t* strw1, const wchar_t* strw2 ); 
int   strw_safe_cmpn( const wchar_t* strw1, const wchar_t* strw2, int len );

// returns length of dstw
int   strw_safe_cpy( wchar_t* dstw, const wchar_t* srcw );   

// srcw can be part of dstw
// returns length of dstw
int   strw_safe_cpyn( wchar_t* dstw, const wchar_t* srcw, int len ); 

// returns index of first substrw in strw or -1
int   strw_safe_find( const wchar_t* strw, const wchar_t* substrw );

// does not find wch == 0
// returns index of first wch in strw or -1
int   strw_safe_findch( const wchar_t* strw, const wchar_t wch );

// returns index of first character from charsetw in strw or -1
int   strw_safe_findoneof( const wchar_t* strw, const wchar_t* charsetw );

// calculates length of strw
int   strw_safe_len( const wchar_t* strw ); 

// reverse order of characters (first character becomes last etc.)
// this function modifies the original string
void  strw_safe_reverse( wchar_t* strw );
void  strw_safe_reversen( wchar_t* strw, int len );

// returns index of last substrw in strw or -1
int   strw_safe_rfind( const wchar_t* strw, int len, const wchar_t* substrw );

// returns index of last wch in strw or -1
int   strw_safe_rfindch( const wchar_t* strw, int len, const wchar_t wch );

// returns index of last character from charsetw in strw or -1
int   strw_safe_rfindoneof( const wchar_t* strw, int len, const wchar_t* charsetw );

// skips trailing characters wch
// returns a pointer to last character != wch
wchar_t* strw_safe_rskip_ch( const wchar_t* strw, int len, const wchar_t wch );

// skips trailing tabs and spaces
// returns a pointer to last non-anyspace character in strw
wchar_t* strw_safe_rskip_anyspaces( const wchar_t* strw, int len );

// skips leading characters wch
// returns a pointer to first character != wch
wchar_t* strw_safe_skip_ch( const wchar_t* strw, const wchar_t wch );

// skips leading tabs and spaces
// returns a pointer to first non-anyspace character in strw
wchar_t* strw_safe_skip_anyspaces( const wchar_t* strw );

// returns:  == 0  if matched
//            > 0  if strw1 > strw2
//            < 0  if strw1 < strw2
// CONDITION: strw1 != 0 && strw2 != 0
int   strw_unsafe_cmp( const wchar_t* strw1, const wchar_t* strw2 ); 
// CONDITION: strw1 != 0 && strw2 != 0 && len >= 0
int   strw_unsafe_cmpn( const wchar_t* strw1, const wchar_t* strw2, int len );

// returns length of dstw
// CONDITION: dstw != 0 && srcw != 0
int   strw_unsafe_cpy( wchar_t* dstw, const wchar_t* srcw );   

// srcw can be part of dstw
// returns length of dstw
// CONDITION: dstw != 0 && srcw != 0 && len >= 0
int   strw_unsafe_cpyn( wchar_t* dstw, const wchar_t* srcw, int len ); 

// returns index of first substrw in strw or -1
// CONDITION: strw != 0 && substrw != 0
int   strw_unsafe_find( const wchar_t* strw, const wchar_t* substrw );

// does not find wch == 0
// returns index of first wch in strw or -1
// CONDITION: strw != 0
int   strw_unsafe_findch( const wchar_t* strw, const wchar_t wch );

// returns index of first character from charsetw in strw or -1
// CONDITION: strw != 0 && charsetw != 0
int   strw_unsafe_findoneof( const wchar_t* strw, const wchar_t* charsetw );

// calculates length of strw
// CONDITION: strw != 0
int   strw_unsafe_len( const wchar_t* strw ); 

// reverse order of characters (first character becomes last etc.)
// this function modifies the original string
// CONDITION: strw != 0
void  strw_unsafe_reverse( wchar_t* strw );
// CONDITION: strw != 0 && len >= 0
void  strw_unsafe_reversen( wchar_t* strw, int len );

// returns index of last substrw in strw or -1
// CONDITION: strw != 0 && len >= 0 && substrw != 0
int   strw_unsafe_rfind( const wchar_t* strw, int len, const wchar_t* substrw );

// returns index of last wch in strw or -1
// CONDITION: strw != 0 && len >= 0
int   strw_unsafe_rfindch( const wchar_t* strw, int len, const wchar_t wch );

// returns index of last character from charsetw in strw or -1
// CONDITION: strw != 0 && len >= 0 && charsetw != 0
int   strw_unsafe_rfindoneof( const wchar_t* strw, int len, const wchar_t* charsetw );

// skips trailing characters wch
// returns a pointer to last character != wch
// CONDITION: strw != 0 && len >= 0
wchar_t* strw_unsafe_rskip_ch( const wchar_t* strw, int len, const wchar_t wch );

// skips trailing tabs and spaces
// returns a pointer to last non-anyspace character in strw
// CONDITION: strw != 0 && len >= 0
wchar_t* strw_unsafe_rskip_anyspaces( const wchar_t* strw, int len );

// skips leading characters wch
// returns a pointer to first character != wch
// CONDITION: strw != 0
wchar_t* strw_unsafe_skip_ch( const wchar_t* strw, const wchar_t wch );

// skips leading tabs and spaces
// returns a pointer to first non-anyspace character in strw
// CONDITION: strw != 0
wchar_t* strw_unsafe_skip_anyspaces( const wchar_t* strw );

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// definitions
/////////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
  #define _tis_any_space_char          is_any_space_charw
  #define _tstr_safe_cmp               strw_safe_cmp
  #define _tstr_safe_cmpn              strw_safe_cmpn
  #define _tstr_safe_cpy               strw_safe_cpy
  #define _tstr_safe_cpyn              strw_safe_cpyn
  #define _tstr_safe_find              strw_safe_find
  #define _tstr_safe_findch            strw_safe_findch
  #define _tstr_safe_findoneof         strw_safe_findoneof
  #define _tstr_safe_len               strw_safe_len
  #define _tstr_safe_reverse           strw_safe_reverse
  #define _tstr_safe_reversen          strw_safe_reversen
  #define _tstr_safe_rfind             strw_safe_rfind
  #define _tstr_safe_rfindch           strw_safe_rfindch
  #define _tstr_safe_rfindoneof        strw_safe_rfindoneof
  #define _tstr_safe_rskip_ch          strw_safe_rskip_ch
  #define _tstr_safe_rskip_anyspaces   strw_safe_rskip_anyspaces
  #define _tstr_safe_skip_ch           strw_safe_skip_ch
  #define _tstr_safe_skip_anyspaces    strw_safe_skip_anyspaces
  #define _tstr_unsafe_cmp             strw_unsafe_cmp
  #define _tstr_unsafe_cmpn            strw_unsafe_cmpn
  #define _tstr_unsafe_cpy             strw_unsafe_cpy
  #define _tstr_unsafe_cpyn            strw_unsafe_cpyn
  #define _tstr_unsafe_find            strw_unsafe_find
  #define _tstr_unsafe_findch          strw_unsafe_findch
  #define _tstr_unsafe_findoneof       strw_unsafe_findoneof
  #define _tstr_unsafe_len             strw_unsafe_len
  #define _tstr_unsafe_reverse         strw_unsafe_reverse
  #define _tstr_unsafe_reversen        strw_unsafe_reversen
  #define _tstr_unsafe_rfind           strw_unsafe_rfind
  #define _tstr_unsafe_rfindch         strw_unsafe_rfindch
  #define _tstr_unsafe_rfindoneof      strw_unsafe_rfindoneof
  #define _tstr_unsafe_rskip_ch        strw_unsafe_rskip_ch
  #define _tstr_unsafe_rskip_anyspaces strw_unsafe_rskip_anyspaces
  #define _tstr_unsafe_skip_ch         strw_unsafe_skip_ch
  #define _tstr_unsafe_skip_anyspaces  strw_unsafe_skip_anyspaces
#else
  #define _tis_any_space_char          is_any_space_char
  #define _tstr_safe_cmp               str_safe_cmp
  #define _tstr_safe_cmpn              str_safe_cmpn
  #define _tstr_safe_cpy               str_safe_cpy
  #define _tstr_safe_cpyn              str_safe_cpyn
  #define _tstr_safe_find              str_safe_find
  #define _tstr_safe_findch            str_safe_findch
  #define _tstr_safe_findoneof         str_safe_findoneof
  #define _tstr_safe_len               str_safe_len
  #define _tstr_safe_reverse           str_safe_reverse
  #define _tstr_safe_reversen          str_safe_reversen
  #define _tstr_safe_rfind             str_safe_rfind
  #define _tstr_safe_rfindch           str_safe_rfindch
  #define _tstr_safe_rfindoneof        str_safe_rfindoneof
  #define _tstr_safe_rskip_ch          str_safe_rskip_ch
  #define _tstr_safe_rskip_anyspaces   str_safe_rskip_anyspaces
  #define _tstr_safe_skip_ch           str_safe_skip_ch
  #define _tstr_safe_skip_anyspaces    str_safe_skip_anyspaces
  #define _tstr_unsafe_cmp             str_unsafe_cmp
  #define _tstr_unsafe_cmpn            str_unsafe_cmpn
  #define _tstr_unsafe_cpy             str_unsafe_cpy
  #define _tstr_unsafe_cpyn            str_unsafe_cpyn
  #define _tstr_unsafe_find            str_unsafe_find
  #define _tstr_unsafe_findch          str_unsafe_findch
  #define _tstr_unsafe_findoneof       str_unsafe_findoneof
  #define _tstr_unsafe_len             str_unsafe_len
  #define _tstr_unsafe_reverse         str_unsafe_reverse
  #define _tstr_unsafe_reversen        str_unsafe_reversen
  #define _tstr_unsafe_rfind           str_unsafe_rfind
  #define _tstr_unsafe_rfindch         str_unsafe_rfindch
  #define _tstr_unsafe_rfindoneof      str_unsafe_rfindoneof
  #define _tstr_unsafe_rskip_ch        str_unsafe_rskip_ch
  #define _tstr_unsafe_rskip_anyspaces str_unsafe_rskip_anyspaces
  #define _tstr_unsafe_skip_ch         str_unsafe_skip_ch
  #define _tstr_unsafe_skip_anyspaces  str_unsafe_skip_anyspaces
#endif // !UNICODE


#ifdef __cplusplus
}
}
#endif
 
//---------------------------------------------------------------------------
#endif
