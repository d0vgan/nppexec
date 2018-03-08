#ifndef _int_to_str_h_
#define _int_to_str_h_
//---------------------------------------------------------------------------
#include "../base.h"
#include "types.h"


#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif


// size of out_str can be up to (3*sizeof(int) + 2)
// returns string length + dec-string (signed) in out_str
int int2str(int value, char* out_str);

// size of out_str can be up to (8*sizeof(int) + 2)
// returns string length + bin-string (signed) in out_str
int int2strbin(int value, char* out_str);

// size of out_str can be up to (3*sizeof(int) + 2)
// returns int2str()
int int2strdec(int value, char* out_str);

// size of out_str can be up to (2*sizeof(int) + 2)
// returns string length + hex-string (signed) in out_str
int int2strhex(int value, char* out_str);

// size of out_str can be up to (3*sizeof(int) + 2)
// returns string length + oct-string (signed) in out_str
int int2stroct(int value, char* out_str);

// size of out_str can be up to (3*sizeof(int) + 1)
// returns string length + dec-string (unsigned) in out_str
int uint2str(unsigned int value, char* out_str);

// size of out_str can be up to (8*sizeof(int) + 1)
// returns string length + bin-string (unsigned) in out_str
int uint2strbin(unsigned int value, char* out_str);

// size of out_str can be up to (3*sizeof(int) + 1)
// returns uint2str()
int uint2strdec(unsigned int value, char* out_str);

// size of out_str can be up to (2*sizeof(int) + 1)
// returns string length + hex-string (unsigned) in out_str
int uint2strhex(unsigned int value, char* out_str);

// size of out_str can be up to (3*sizeof(int) + 1)
// returns string length + oct-string (unsigned) in out_str
int uint2stroct(unsigned int value, char* out_str);

// size of out_strw can be up to (3*sizeof(int) + 2)
// returns string length + dec-string (signed) in out_strw
int int2strw(int value, wchar_t* out_strw);

// size of out_strw can be up to (8*sizeof(int) + 2)
// returns string length + bin-string (signed) in out_strw
int int2strbinw(int value, wchar_t* out_strw);

// size of out_strw can be up to (3*sizeof(int) + 2)
// returns int2strw()
int int2strdecw(int value, wchar_t* out_strw);

// size of out_strw can be up to (2*sizeof(int) + 2)
// returns string length + hex-string (signed) in out_strw
int int2strhexw(int value, wchar_t* out_strw);

// size of out_strw can be up to (3*sizeof(int) + 2)
// returns string length + oct-string (signed) in out_strw
int int2stroctw(int value, wchar_t* out_strw);

// size of out_strw can be up to (3*sizeof(int) + 1)
// returns string length + dec-string (unsigned) in out_strw
int uint2strw(unsigned int value, wchar_t* out_strw);

// size of out_strw can be up to (8*sizeof(int) + 1)
// returns string length + bin-string (unsigned) in out_strw
int uint2strbinw(unsigned int value, wchar_t* out_strw);

// size of out_strw can be up to (3*sizeof(int) + 1)
// returns uint2strw()
int uint2strdecw(unsigned int value, wchar_t* out_strw);

// size of out_strw can be up to (2*sizeof(int) + 1)
// returns string length + hex-string (unsigned) in out_strw
int uint2strhexw(unsigned int value, wchar_t* out_strw);

// size of out_strw can be up to (3*sizeof(int) + 1)
// returns string length + oct-string (unsigned) in out_strw
int uint2stroctw(unsigned int value, wchar_t* out_strw);

int int64_to_str(__int64 value, char* out_str);
int uint64_to_str(unsigned __int64 value, char* out_str);
int int64_to_strw(__int64 value, wchar_t* out_strw);
int uint64_to_strw(unsigned __int64 value, wchar_t* out_strw);

/////////////////////////////////////////////////////////////////////////////
// definitions
/////////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
  #define _tint2str       int2strw
  #define _tint2strbin    int2strbinw
  #define _tint2strdec    int2strdecw
  #define _tint2strhex    int2strhexw
  #define _tint2stroct    int2stroctw
  #define _tuint2str      uint2strw
  #define _tuint2strbin   uint2strbinw
  #define _tuint2strdec   uint2strdecw
  #define _tuint2strhex   uint2strhexw
  #define _tuint2stroct   uint2stroctw
  #define _tint64_to_str  int64_to_strw
  #define _tuint64_to_str uint64_to_strw
#else
  #define _tint2str       int2str
  #define _tint2strbin    int2strbin
  #define _tint2strdec    int2strdec
  #define _tint2strhex    int2strhex
  #define _tint2stroct    int2stroct
  #define _tuint2str      uint2str
  #define _tuint2strbin   uint2strbin
  #define _tuint2strdec   uint2strdec
  #define _tuint2strhex   uint2strhex
  #define _tuint2stroct   uint2stroct
  #define _tint64_to_str  int64_to_str
  #define _tuint64_to_str uint64_to_str
#endif // !UNICODE


#ifdef __cplusplus
}
}
#endif


//---------------------------------------------------------------------------
#endif
