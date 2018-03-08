#ifndef _hex_str_h_
#define _hex_str_h_
//---------------------------------------------------------------------------
#include "../base.h"
#include "types.h"


#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif


/////////////////////////////////////////////////////////////////////////////
// hexstrex2buf(hexstrex, out_buf, buf_size)
//   converts a hex string with characters in quotes to binary data (buf)
//   returns number of bytes in out_buf
/////////////////////////////////////////////////////////////////////////////
// example: 
//   byte_t buf[16];
//   hexstrex2buf(" \"ijk\" 6C 6D 6E \'opq\' ", buf, 16); // returns 9
//   // output: buf = { 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71 }
//   //                  'i'   'j'   'k'   'l'   'm'   'n'   'o'   'p'   'q'
//
// required buf_size (max):  length(hexstrex)-1
// !!! NOTE:                 length(hexstrex) MUST be >= 2
// compatibility:            char only!
//
int hexstrex2buf(const char* hexstrex, byte_t* out_buf, int buf_size);

/////////////////////////////////////////////////////////////////////////////
// hexstr2buf(hexstr, out_buf, buf_size)
//   converts a hex string to binary data (buf)
//   returns number of bytes in out_buf
/////////////////////////////////////////////////////////////////////////////
// example:
//   byte_t buf[16];
//   hexstr2buf(" 6C 6D 6E ", buf, 16); // returns 3
//   // output: buf = { 0x6C, 0x6D, 0x6E }
//
// required buf_size (max):  length(hexstrex)/2
// compatibility:            char
//
int hexstr2buf(const char* hexstr, byte_t* out_buf, int buf_size);

/////////////////////////////////////////////////////////////////////////////
// buf2hexstr(buf, buf_size, out_hexstr, hexstr_size, bytes_delimiter)
//   converts binary data to a hex string
//   returns string length (number of char characters)
/////////////////////////////////////////////////////////////////////////////
// example:
//   byte_t buf[] = { 0x6C, 0x6D, 0x6E };
//   char str[16];
//   buf2hexstr(buf, 3, str, 16, " "); // returns 8
//   // output: str = "6C 6D 6E"
//
// required hexstr_size:  1 + buf_size*(2 + length(bytes_delimiter))
// compatibility:         char
//
int buf2hexstr(const byte_t* buf, int buf_size, 
               char* out_hexstr, int hexstr_size, 
               const char* bytes_delimiter);

/////////////////////////////////////////////////////////////////////////////
// hexstrexw2buf(hexstrexw, out_buf, buf_size)
//   converts a hex string with characters in quotes to binary data (buf)
//   returns number of bytes in out_buf
/////////////////////////////////////////////////////////////////////////////
// example:
//
//   byte_t buf[16];
//   hexstrexw2buf(L" \"ij\" 6B \'lm\' ", buf, 16); // returns 9
//   // output: buf = { 0x69,0x00, 0x6A,0x00, 0x6B, 0x6C,0x00, 0x6D,0x00 }
//   //                    L'i'       L'j'             L'l'       L'm'
//
// required buf_size (max):  1 + 2*(length(hexstrexw) - 2)
// !!! NOTE:                 length(hexstrexw) MUST be >= 2
// compatibility:            wchar_t only!
//
int hexstrexw2buf(const wchar_t* hexstrexw, byte_t* out_buf, int buf_size);

/////////////////////////////////////////////////////////////////////////////
// hexstrw2buf(hexstrw, out_buf, buf_size)
//   converts a hex string to binary data (buf)
//   returns number of bytes in out_buf
/////////////////////////////////////////////////////////////////////////////
// example:
//   see hexstr2buf
//
// required buf_size (max):  length(hexstrexw)/2
// compatibility:            wchar_t only!
//
int hexstrw2buf(const wchar_t* hexstrw, byte_t* out_buf, int buf_size);

/////////////////////////////////////////////////////////////////////////////
// buf2hexstrw(buf, buf_size, out_hexstrw, hexstrw_size, bytes_delimiterw)
//   converts binary data to a hex string
//   returns string length (number of wchar_t characters)
/////////////////////////////////////////////////////////////////////////////
// example:
//   see buf2hexstr
//
// required hexstrw_size:  1 + buf_size*(2 + length(bytes_delimiterw))
// compatibility:          wchar_t only!
//
int buf2hexstrw(const byte_t* buf, int buf_size, 
                wchar_t* out_hexstrw, int hexstrw_size, 
                const wchar_t* bytes_delimiterw);

/////////////////////////////////////////////////////////////////////////////
// definitions
/////////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
  #define _thexstrex2buf hexstrexw2buf
  #define _thexstr2buf   hexstrw2buf
  #define _tbuf2hexstr   buf2hexstrw
#else
  #define _thexstrex2buf hexstrex2buf
  #define _thexstr2buf   hexstr2buf
  #define _tbuf2hexstr   buf2hexstr
#endif // !UNICODE


#ifdef __cplusplus
}
}
#endif


//---------------------------------------------------------------------------
#endif

