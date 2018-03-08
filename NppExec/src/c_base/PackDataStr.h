#ifndef _pack_data_str_h_
#define _pack_data_str_h_
//---------------------------------------------------------------------------
#include "../base.h"
#include "types.h"

#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif

/////////////////////////////////////////////////////////////////////////////
// Use these functions to "pack" and "unpack" strings with many repeating
// symbols such as hex-strings with a lot of zeros e.g. "00000000111111122"
// (this string will be "packed" to "0(8)1(7)22").

/////////////////////////////////////////////////////////////////////////////
// char
//
int pack_data_str(const char* datastr, 
  char* out_packedstr, int packedstr_size); // returns out_packedstr length

int unpack_data_str(const char* packedstr,
  char* out_datastr, int datastr_size);  // returns out_datastr length

/////////////////////////////////////////////////////////////////////////////
// wchar_t
//
int pack_data_strw(const wchar_t* datastrw,
  wchar_t* out_packedstrw, int packedstrw_size); // returns packedstrw length

int unpack_data_strw(const wchar_t* packedstrw,
  wchar_t* out_datastrw, int datastrw_size);  // returns datastrw length


/////////////////////////////////////////////////////////////////////////////
// definitions
/////////////////////////////////////////////////////////////////////////////
#ifdef UNICODE
  #define _tpack_data_str   pack_data_strw
  #define _tunpack_data_str unpack_data_strw
#else
  #define _tpack_data_str   pack_data_str
  #define _tunpack_data_str unpack_data_str
#endif // !UNICODE


#ifdef __cplusplus
}
}
#endif

//---------------------------------------------------------------------------
#endif
