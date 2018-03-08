#ifndef _match_mask_h_
#define _match_mask_h_
//---------------------------------------------------------------------------
#include "../base.h"
#include "types.h"


#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif


/////////////////////////////////////////////////////////////////
// match_mask - understands these wildcards:
//              * (0 or more characters)
//              ? (1 character)
// current implementation is case-sensitive
// returns 1 if match, otherwise 0
//
bool_t match_mask(const char* mask, const char* str);

/////////////////////////////////////////////////////////////////
// match_maskw - version for wchar_t
//
bool_t match_maskw(const wchar_t* maskw, const wchar_t* strw);

/////////////////////////////////////////////////////////////////
// definitions
/////////////////////////////////////////////////////////////////
#ifdef UNICODE
  #define _tmatch_mask match_maskw
#else
  #define _tmatch_mask match_mask
#endif // !UNICODE


#ifdef __cplusplus
}
}
#endif


//---------------------------------------------------------------------------
#endif
