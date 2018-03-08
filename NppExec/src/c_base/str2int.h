#ifndef _str_to_int_h_
#define _str_to_int_h_
//-----------------------------------------------------------------------------
#include "../base.h"
#include "types.h"

// >>>>>>>>>>>> str2int v.1.3 (Nov 2016) >>>>>>>>>>>>
// >>>>
// >>>> usage:
// >>>>   str2int("-1235.890") -> -1235    (DEC, signed)
// >>>>   str2int("-0xFAE37")  -> -1027639 (HEX, signed)
// >>>>   str2int("0b1001110)  ->  78      (BIN, signed)
// >>>>   str2int("053162")    ->  22130   (OCT, signed)
// >>>>   str2int("$FAE37")    ->  1027639 (HEX, signed)
// >>>>
// >>>> new in ver. 1.0.3:
// >>>>   str2int("10M")   -> 10*1024*1024 [M or m == Mega]
// >>>>   str2int("$10k")  -> 16*1024      [k or K == Kilo]
// >>>>   str2int("0x10m") -> 16*1024*1024 [M or m == Mega]


#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif


// set the following value to 1 to interpret "00123" as decimal number
// set the following value to 0 to interpret "00123" as octal number
#define  LEADING_00_IS_DEC_VALUE  0

int              str2int(const char* str);
unsigned int     str2uint(const char* str);
unsigned int     strbase2uint(const char* str, unsigned int base);
__int64          str2int64(const char* str);
unsigned __int64 str2uint64(const char* str);
unsigned __int64 strbase2uint64(const char* str, unsigned int base);
bool_t           is_dec_value(const char* str);

int              wstr2int(const wchar_t* strw);
unsigned int     wstr2uint(const wchar_t* strw);
unsigned int     wstrbase2uint(const wchar_t* strw, unsigned int base);
__int64          wstr2int64(const wchar_t* strw);
unsigned __int64 wstr2uint64(const wchar_t* strw);
unsigned __int64 wstrbase2uint64(const wchar_t* strw, unsigned int base);
bool_t           is_dec_valuew(const wchar_t* strw);

/////////////////////////////////////////////////////////////////////////////
// definitions
/////////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
  #define _tstr2int        wstr2int
  #define _tstr2uint       wstr2uint
  #define _tstrbase2uint   wstrbase2uint
  #define _tstr2int64      wstr2int64
  #define _tstr2uint64     wstr2uint64
  #define _tstrbase2uint64 wstrbase2uint64
  #define _tis_dec_value   is_dec_valuew
#else
  #define _tstr2int        str2int
  #define _tstr2uint       str2uint
  #define _tstrbase2uint   strbase2uint
  #define _tstr2int64      str2int64
  #define _tstr2uint64     str2uint64
  #define _tstrbase2uint64 strbase2uint64
  #define _tis_dec_value   is_dec_value
#endif // !UNICODE


#ifdef __cplusplus
}
}
#endif


//-----------------------------------------------------------------------------
#endif
