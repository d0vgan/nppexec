#ifndef _max_int_h_
#define _max_int_h_
//---------------------------------------------------------------------------
#include "../base.h"


#ifdef __cplusplus
namespace c_base {
extern "C" {
#endif


char             max_char(void);   // returns max char value
int              max_int(void);    // returns max int value
long             max_long(void);   // returns max long int value
short            max_short(void);  // returns max short int value
unsigned char    max_uchar(void);  // returns max unsigned char value
unsigned int     max_uint(void);   // returns max unsigned int value
unsigned long    max_ulong(void);  // returns max unsigned long int value
unsigned short   max_ushort(void); // returns max unsigned short int value
__int64          max_int64(void);  // returns max int64 value
unsigned __int64 max_uint64(void); // returns max unsigned int64 value


#ifdef __cplusplus
}
}
#endif


//---------------------------------------------------------------------------
#endif
