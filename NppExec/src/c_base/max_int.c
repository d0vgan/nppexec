/****************************************************************************
 *
 * max_int Lib ver. 1.1
 * --------------------
 *
 * (C) Nov 2016, DV
 *
 ***************************************************************************/

#include "max_int.h"


#ifdef __cplusplus
namespace c_base {
#endif


char max_char(void)
{
  return max_uchar()/2;
}

int  max_int(void)
{
  return max_uint()/2;
}

long max_long(void)
{
  return max_ulong()/2;
}

short max_short(void)
{
  return max_ushort()/2;
}

__int64 max_int64(void)
{
  return max_uint64()/2;
}

unsigned char max_uchar(void)
{
  unsigned char max_val = 0xFF;
  unsigned int  pos = 0;

  while (pos < sizeof(unsigned char) - 1)
  {
    max_val <<= 8;
    max_val |= 0xFF;
    ++pos;
  }

  return max_val;
}

unsigned int max_uint(void)
{
  unsigned int max_val = 0xFF;
  unsigned int pos = 0;

  while (pos < sizeof(unsigned int) - 1)
  {
    max_val <<= 8;
    max_val |= 0xFF;
    ++pos;
  }

  return max_val;
}

unsigned long max_ulong(void)
{
  unsigned long max_val = 0xFFL;
  unsigned int  pos = 0;

  while (pos < sizeof(unsigned long) - 1)
  {
    max_val <<= 8;
    max_val |= 0xFF;
    ++pos;
  }

  return max_val;
}

unsigned short max_ushort(void)
{
  unsigned short max_val = 0xFF;
  unsigned int   pos = 0;

  while (pos < sizeof(unsigned short) - 1)
  {
    max_val <<= 8;
    max_val |= 0xFF;
    ++pos;
  }

  return max_val;
}

unsigned __int64 max_uint64(void)
{
  unsigned __int64 max_val = 0xFF;
  unsigned int     pos = 0;

  while (pos < sizeof(unsigned __int64) - 1)
  {
    max_val <<= 8;
    max_val |= 0xFF;
    ++pos;
  }

  return max_val;
}


#ifdef __cplusplus
}
#endif
