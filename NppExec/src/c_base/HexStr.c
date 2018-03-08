/****************************************************************************
 *
 * HexStr Lib ver. 1.3
 * -------------------
 *
 * (C) Jul 2007, DV
 *
 ***************************************************************************/

#include "HexStr.h"


#ifdef __cplusplus
namespace c_base {
#endif


int hexstrex2buf(const char* hexstrex, byte_t* out_buf, int buf_size)
{
  int buf_len = 0;
  if (hexstrex && out_buf && (buf_size > 0))
  {
    int    i, index;
    bool_t quote, dblquote;
    char   ch;
    int    bt[2];

    index = 0;
    quote = 0;
    dblquote = 0;
    for (i = 0, buf_len = 0; (ch = hexstrex[i]) && (buf_len < buf_size); i++)
    {
      if (dblquote)
      {
        if (ch != '\"')
        {
          out_buf[buf_len] = (byte_t) ch;
          ++buf_len;
        }
        else
        {
          dblquote = 0;
        }
      } // dblquote
      else if (quote)
      {
        if (ch != '\'')
        {
          out_buf[buf_len] = (byte_t) ch;
          ++buf_len;
        }
        else
        {
          quote = 0;
        }
      } // quote
      else
      {
        if ((ch >= '0') && (ch <= '9'))
        {
          bt[index] = (ch - '0');
          ++index;
        }
        else if ((ch >= 'A') && (ch <= 'F'))
        {
          bt[index] = ((ch - 'A') + 10);
          ++index;
        }
        else if ((ch >= 'a') && (ch <= 'f'))
        {
          bt[index] = ((ch - 'a') + 10);
          ++index;
        }
        else if (ch == '\"')
        {
          dblquote = 1;
          index = 0;    // abort current byte
        }
        else if (ch == '\'')
        {
          quote = 1;
          index = 0;    // abort current byte
        }
        if (index == 2)
        {
          index = 0;
          out_buf[buf_len] = (byte_t) (((bt[0] << 4) & 0xF0) + bt[1]);
          ++buf_len;
        }
      } // symbol
    } // for
  }
  return buf_len;
}

int hexstr2buf(const char* hexstr, byte_t* out_buf, int buf_size)
{
  int buf_len = 0;
  if (hexstr && out_buf && (buf_size > 0))
  {
    int   i, index;
    char  ch;
    int   bt[2];

    index = 0;
    for (i = 0, buf_len = 0; (ch = hexstr[i]) && (buf_len < buf_size); i++)
    {
      if ((ch >= '0') && (ch <= '9'))
      {
        bt[index] = (ch - '0');
        ++index;
      }
      else if ((ch >= 'A') && (ch <= 'F'))
      {
        bt[index] = ((ch - 'A') + 10);
        ++index;
      }
      else if ((ch >= 'a') && (ch <= 'f'))
      {
        bt[index] = ((ch - 'a') + 10);
        ++index;
      }
      if (index == 2)
      {
        index = 0;
        out_buf[buf_len] = (byte_t) (((bt[0] << 4) & 0xF0) + (bt[1] & 0x0F));
        ++buf_len;
      }
    }
  }
  return buf_len;
}

int buf2hexstr(const byte_t* buf, int buf_size, 
               char* out_hexstr, int hexstr_size,
               const char* bytes_delimiter)
{
  int hexstr_len = 0;
  if (out_hexstr && (hexstr_size > 0))
  {
    if (buf && (buf_size > 0))
    {
      int    i, j;
      int    delimiter_len;
      byte_t bt, val;
    
      delimiter_len = 0;
      if (bytes_delimiter)
      {
        while (bytes_delimiter[delimiter_len])
        {
          ++delimiter_len;
        }
      }

      for (i = 0, hexstr_len = 0; (i < buf_size) && (hexstr_len < hexstr_size-2); i++)
      {
        val = buf[i];
        bt = (val >> 4) & 0x0F;
        out_hexstr[hexstr_len] = (bt <= 9) ? ('0' + bt) : ('A' + (bt - 10));
        ++hexstr_len;
        bt = val & 0x0F;
        out_hexstr[hexstr_len] = (bt <= 9) ? ('0' + bt) : ('A' + (bt - 10));
        ++hexstr_len;
        if ((delimiter_len > 0) && (i < buf_size-1))
        {
          if (hexstr_len < hexstr_size-delimiter_len)
          {
            for (j = 0; j < delimiter_len; j++)
            {
              out_hexstr[hexstr_len] = bytes_delimiter[j];
              ++hexstr_len;
            }
          }
          else
          {
            break;
          }
        }
      }
    }
    out_hexstr[hexstr_len] = 0;
  }
  return hexstr_len;
}

//---------------------------------------------------------------------------

int hexstrexw2buf(const wchar_t* hexstrexw, byte_t* out_buf, int buf_size)
{
  int buf_len = 0;
  if (hexstrexw && out_buf && (buf_size > 0))
  {
    int     i, index;
    bool_t  quote, dblquote;
    wchar_t wch;
    int     bt[2];

    index = 0;
    quote = 0;
    dblquote = 0;
    for (i = 0, buf_len = 0; (wch = hexstrexw[i]) && (buf_len < buf_size); i++)
    {
      if (dblquote)
      {
        if (wch != L'\"')
        {
          if (buf_len+1 < buf_size)
          {
            out_buf[buf_len] = (byte_t) (wch & 0x00FF);
            ++buf_len;
            out_buf[buf_len] = (byte_t) ((wch >> 8) & 0x00FF);
            ++buf_len;
          }
          else
          {
            break;
          }
        }
        else
        {
          dblquote = 0;
        }
      } // dblquote
      else if (quote)
      {
        if (wch != L'\'')
        {
          if (buf_len+1 < buf_size)
          {
            out_buf[buf_len] = (byte_t) (wch & 0x00FF);
            ++buf_len;
            out_buf[buf_len] = (byte_t) ((wch >> 8) & 0x00FF);
            ++buf_len;
          }
          else
          {
            break;
          }
        }
        else
        {
          quote = 0;
        }
      } // quote
      else
      {
        if ((wch >= L'0') && (wch <= L'9'))
        {
          bt[index] = (wch - L'0');
          ++index;
        }
        else if ((wch >= L'A') && (wch <= L'F'))
        {
          bt[index] = ((wch - L'A') + 10);
          ++index;
        }
        else if ((wch >= L'a') && (wch <= L'f'))
        {
          bt[index] = ((wch - L'a') + 10);
          ++index;
        }
        else if (wch == L'\"')
        {
          dblquote = 1;
          index = 0;    // abort current byte
        }
        else if (wch == L'\'')
        {
          quote = 1;
          index = 0;    // abort current byte
        }
        if (index == 2)
        {
          index = 0;
          out_buf[buf_len] = (byte_t) (((bt[0] << 4) & 0xF0) + (bt[1] & 0x0F));
          ++buf_len;
        }
      } // symbol
    } // for
  }
  return buf_len;
}

int hexstrw2buf(const wchar_t* hexstrw, byte_t* out_buf, int buf_size)
{
  int buf_len = 0;
  if (hexstrw && out_buf && (buf_size > 0))
  {
    int     i, index;
    wchar_t wch;
    int     bt[2];

    index = 0;
    for (i = 0, buf_len = 0; (wch = hexstrw[i]) && (buf_len < buf_size); i++)
    {
      if ((wch >= L'0') && (wch <= L'9'))
      {
        bt[index] = (wch - L'0');
        ++index;
      }
      else if ((wch >= L'A') && (wch <= L'F'))
      {
        bt[index] = ((wch - L'A') + 10);
        ++index;
      }
      else if ((wch >= L'a') && (wch <= L'f'))
      {
        bt[index] = ((wch - L'a') + 10);
        ++index;
      }
      if (index == 2)
      {
        index = 0;
        out_buf[buf_len] = (byte_t) (((bt[0] << 4) & 0xF0) + (bt[1] & 0x0F));
        ++buf_len;
      }
    }
  }
  return buf_len;
}

int buf2hexstrw(const byte_t* buf, int buf_size, 
                wchar_t* out_hexstrw, int hexstrw_size, 
                const wchar_t* bytes_delimiterw)
{
  int hexstrw_len = 0;
  if (out_hexstrw && (hexstrw_size > 0))
  {
    if (buf && (buf_size > 0))
    {
      int    i, j;
      int    delimiterw_len;
      byte_t bt, val;
    
      delimiterw_len = 0;
      if (bytes_delimiterw)
      {
        while (bytes_delimiterw[delimiterw_len])
        {
          ++delimiterw_len;
        }
      }

      for (i = 0, hexstrw_len = 0; (i < buf_size) && (hexstrw_len < hexstrw_size-2); i++)
      {
        val = buf[i];
        bt = (val >> 4) & 0x0F;
        out_hexstrw[hexstrw_len] = (bt <= 9) ? (L'0' + bt) : (L'A' + (bt - 10));
        ++hexstrw_len;
        bt = val & 0x0F;
        out_hexstrw[hexstrw_len] = (bt <= 9) ? (L'0' + bt) : (L'A' + (bt - 10));
        ++hexstrw_len;
        if ((delimiterw_len > 0) && (i < buf_size-1))
        {
          if (hexstrw_len < hexstrw_size-delimiterw_len)
          {
            for (j = 0; j < delimiterw_len; j++)
            {
              out_hexstrw[hexstrw_len] = bytes_delimiterw[j];
              ++hexstrw_len;
            }
          }
          else
          {
            break;
          }
        }
      }
    }
    out_hexstrw[hexstrw_len] = 0;
  }
  return hexstrw_len;
}


#ifdef __cplusplus
}
#endif
