/****************************************************************************
 *
 * int2str Lib ver. 1.1
 * --------------------
 *
 * (C) Jul 2007, DV
 *
 ***************************************************************************/

#include "int2str.h"


#ifdef __cplusplus
namespace c_base {
#endif


static int get_max_hex_pos_from_uint(const unsigned int value)
{
  unsigned int cmp_val = 0x0F;
  int          pos = 0;

  while (value > cmp_val)
  {
    cmp_val <<= 4;
    cmp_val |= 0x0F;
    ++pos;
  }
  return pos;
}

static int get_max_oct_pos_from_uint(const unsigned int value)
{
  unsigned int cmp_val = 0x07;
  int          pos = 0;

  while (value > cmp_val)
  {
    cmp_val <<= 3;
    cmp_val |= 0x07;
    ++pos;
  }
  return pos;
}

static int get_max_bin_pos_from_uint(const unsigned int value)
{
  unsigned int cmp_val = 0x01;
  int          pos = 0;

  while (value > cmp_val)
  {
    cmp_val <<= 1;
    cmp_val |= 0x01;
    ++pos;
  }
  return pos;
}

static int get_max_dec_pos_from_uint(const unsigned int value)
{
  unsigned int prev_cmp_val = 0;
  unsigned int cmp_val = 10;
  int          pos = 0;
 
  while (value >= cmp_val)
  {
    if (cmp_val < prev_cmp_val)
    {
      // integer overflow
      --pos;
      break;
    }
    prev_cmp_val = cmp_val;
    cmp_val *= 10;
    ++pos;
  }
  return pos;
}

static int get_max_dec_pos_from_uint64(const unsigned __int64 value)
{
    unsigned __int64 prev_cmp_val = 0;
    unsigned __int64 cmp_val = 10;
    int              pos = 0;

    while (value >= cmp_val)
    {
        if (cmp_val < prev_cmp_val)
        {
            // integer overflow
            --pos;
            break;
        }
        prev_cmp_val = cmp_val;
        cmp_val *= 10;
        ++pos;
    }
    return pos;
}

//---------------------------------------------------------------------------

// size of out_str can be up to (2*sizeof(int) + 1)
int uint2strhex(unsigned int value, char* out_str)
{
  unsigned int bt;
  int          pos;
  int          len;

  pos = get_max_hex_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    bt = value & 0x0F;
    out_str[pos] = (char)((bt < 10) ? ('0' + bt) : ('A' + (bt - 10)));
    value >>= 4;
    --pos;
  }
  out_str[len] = 0;
  return len;
}

// size of out_str can be up to (3*sizeof(int) + 1)
int uint2stroct(unsigned int value, char* out_str)
{
  int pos;
  int len;

  pos = get_max_oct_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    out_str[pos] = '0' + (value & 0x07);
    value >>= 3;
    --pos;
  }
  out_str[len] = 0;
  return len;
}

// size of out_str can be up to (8*sizeof(int) + 1)
int uint2strbin(unsigned int value, char* out_str)
{
  int pos;
  int len;

  pos = get_max_bin_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    out_str[pos] = '0' + (value & 0x01);
    value >>= 1;
    --pos;
  }
  out_str[len] = 0;
  return len;
}

// size of out_str can be up to (3*sizeof(int) + 1)
int uint2str(unsigned int value, char* out_str)
{
  int pos;
  int len;
  
  pos = get_max_dec_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    out_str[pos] = '0' + (value % 10);
    value /= 10;
    --pos;
  }
  out_str[len] = 0;
  return len;
}

int uint64_to_str(unsigned __int64 value, char* out_str)
{
    int pos;
    int len;

    pos = get_max_dec_pos_from_uint64(value);
    len = pos + 1;
    while (pos >= 0)
    {
        out_str[pos] = '0' + (value % 10);
        value /= 10;
        --pos;
    }
    out_str[len] = 0;
    return len;
}

// size of out_str can be up to (3*sizeof(int) + 1)
int uint2strdec(unsigned int value, char* out_str)
{
  return uint2str(value, out_str);
}

// size of out_str can be up to (2*sizeof(int) + 2)
int int2strhex(int value, char* out_str)
{
  if (value < 0)
  {
    out_str[0] = '-';
    return uint2strhex( (unsigned int) (-value), out_str + 1 ) + 1;
  }
  return uint2strhex( (unsigned int) value, out_str );
}

// size of out_str can be up to (3*sizeof(int) + 2)
int int2stroct(int value, char* out_str)
{
  if (value < 0)
  {
    out_str[0] = '-';
    return uint2stroct( (unsigned int) (-value), out_str + 1 ) + 1;
  }
  return uint2stroct( (unsigned int) value, out_str );
}

// size of out_str can be up to (8*sizeof(int) + 2)
int int2strbin(int value, char* out_str)
{
  if (value < 0)
  {
    out_str[0] = '-';
    return uint2strbin( (unsigned int) (-value), out_str + 1 ) + 1;
  }
  return uint2strbin( (unsigned int) value, out_str );
}

// size of out_str can be up to (3*sizeof(int) + 2)
int int2str(int value, char* out_str)
{
  if (value < 0)
  {
    out_str[0] = '-';
    return uint2str( (unsigned int) (-value), out_str + 1 ) + 1;
  }
  return uint2str( (unsigned int) value, out_str );
}

int int64_to_str(__int64 value, char* out_str)
{
    if (value < 0)
    {
        out_str[0] = '-';
        return uint64_to_str( (unsigned __int64) (-value), out_str + 1 ) + 1;
    }
    return uint64_to_str( (unsigned __int64) value, out_str );
}

// size of out_str can be up to (3*sizeof(int) + 2)
int int2strdec(int value, char* out_str)
{
  return int2str(value, out_str);
}

//---------------------------------------------------------------------------

// size of out_strw can be up to (2*sizeof(int) + 1)
int uint2strhexw(unsigned int value, wchar_t* out_strw)
{
  unsigned int bt;
  int          pos;
  int          len;

  pos = get_max_hex_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    bt = value & 0x0F;
    out_strw[pos] = (wchar_t)((bt < 10) ? (L'0' + bt) : (L'A' + (bt - 10)));
    value >>= 4;
    --pos;
  }
  out_strw[len] = 0;
  return len;
}

// size of out_strw can be up to (3*sizeof(int) + 1)
int uint2stroctw(unsigned int value, wchar_t* out_strw)
{
  int pos;
  int len;

  pos = get_max_oct_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    out_strw[pos] = L'0' + (value & 0x07);
    value >>= 3;
    --pos;
  }
  out_strw[len] = 0;
  return len;
}

// size of out_strw can be up to (8*sizeof(int) + 1)
int uint2strbinw(unsigned int value, wchar_t* out_strw)
{
  int pos;
  int len;

  pos = get_max_bin_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    out_strw[pos] = L'0' + (value & 0x01);
    value >>= 1;
    --pos;
  }
  out_strw[len] = 0;
  return len;
}

// size of out_strw can be up to (3*sizeof(int) + 1)
int uint2strw(unsigned int value, wchar_t* out_strw)
{
  int pos;
  int len;

  pos = get_max_dec_pos_from_uint(value);
  len = pos + 1;
  while (pos >= 0)
  {
    out_strw[pos] = L'0' + (value % 10);
    value /= 10;
    --pos;
  }
  out_strw[len] = 0;
  return len;
}

int uint64_to_strw(unsigned __int64 value, wchar_t* out_strw)
{
    int pos;
    int len;

    pos = get_max_dec_pos_from_uint64(value);
    len = pos + 1;
    while (pos >= 0)
    {
        out_strw[pos] = L'0' + (value % 10);
        value /= 10;
        --pos;
    }
    out_strw[len] = 0;
    return len;
}

// size of out_strw can be up to (3*sizeof(int) + 1)
int uint2strdecw(unsigned int value, wchar_t* out_strw)
{
  return uint2strw(value, out_strw);
}

// size of out_strw can be up to (2*sizeof(int) + 2)
int int2strhexw(int value, wchar_t* out_strw)
{
  if (value < 0)
  {
    out_strw[0] = L'-';
    return uint2strhexw( (unsigned int) (-value), out_strw + 1 ) + 1;
  }
  return uint2strhexw( (unsigned int) value, out_strw );
}

// size of out_strw can be up to (3*sizeof(int) + 2)
int int2stroctw(int value, wchar_t* out_strw)
{
  if (value < 0)
  {
    out_strw[0] = L'-';
    return uint2stroctw( (unsigned int) (-value), out_strw + 1 ) + 1;
  }
  return uint2stroctw( (unsigned int) value, out_strw );
}

// size of out_strw can be up to (8*sizeof(int) + 2)
int int2strbinw(int value, wchar_t* out_strw)
{
  if (value < 0)
  {
    out_strw[0] = L'-';
    return uint2strbinw( (unsigned int) (-value), out_strw + 1 ) + 1;
  }
  return uint2strbinw( (unsigned int) value, out_strw );
}

// size of out_strw can be up to (3*sizeof(int) + 2)
int int2strw(int value, wchar_t* out_strw)
{
  if (value < 0)
  {
    out_strw[0] = L'-';
    return uint2strw( (unsigned int) (-value), out_strw + 1 ) + 1;
  }
  return uint2strw( (unsigned int) value, out_strw );
}

int int64_to_strw(__int64 value, wchar_t* out_strw)
{
    if (value < 0)
    {
        out_strw[0] = L'-';
        return uint64_to_strw( (unsigned __int64) (-value), out_strw + 1 ) + 1;
    }
    return uint64_to_strw( (unsigned __int64) value, out_strw );
}

// size of out_strw can be up to (3*sizeof(int) + 2)
int int2strdecw(int value, wchar_t* out_strw)
{
  return int2strw(value, out_strw);
}


#ifdef __cplusplus
}
#endif
