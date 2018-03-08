/****************************************************************************
 *
 * PackDataStr ver. 1.0
 * --------------------
 *
 * (C) Jul 2007, DV
 *
 ***************************************************************************/

#include "PackDataStr.h"
#include "int2str.h"
#include "str2int.h"

#ifdef __cplusplus
namespace c_base {
#endif


// char
int pack_data_str(const char* datastr, 
  char* out_packedstr, int packedstr_size)
{
  int packed_len = 0;
  if (datastr && out_packedstr && (packedstr_size > 0))
  {
    if (*datastr) // ensure the datastr is not empty
    {
      char ch;
      char prev_ch = 0;
      int  dups = 0;

      for (;;)
      {
        ch = *datastr;
        if (prev_ch == ch)
        {
          ++dups;
        }
        else
        {
          if (dups > 0)
          {
            if (dups >= 4)
            {
              char numstr[2*sizeof(int) + 1];
              int  len;

              len = uint2strhex(dups + 1, numstr);
              if (packed_len + len + 2 < packedstr_size)
              {
                int i = 0;
                out_packedstr[packed_len] = '(';
                ++packed_len;
                while (i < len)
                {
                  out_packedstr[packed_len] = numstr[i];
                  ++packed_len;
                  ++i;
                }
                out_packedstr[packed_len] = ')';
                ++packed_len;
              }
              else
              {
                // packedstr_size is not enough 
                break;
              }
            }
            else
            {
              if (packed_len + dups < packedstr_size)
              {
                while (dups > 0)
                {
                  out_packedstr[packed_len] = prev_ch;
                  ++packed_len;
                  --dups;
                }
              }
              else
              {
                // packedstr_size is not enough 
                break;
              }
            }
            dups = 0;
          }
          if (ch)
          {
            if ((ch != '(') && (ch != '\\'))
            {
              if (packed_len + 1 < packedstr_size)
              {
                out_packedstr[packed_len] = ch;
              }
              else
              {
                // packedstr_size is not enough 
                break;
              }
            }
            else
            {
              if (packed_len + 2 < packedstr_size)
              {
                out_packedstr[packed_len] = '\\';
                ++packed_len;
                out_packedstr[packed_len] = ch;
              }
              else
              {
                // packedstr_size is not enough 
                break;
              }
            }
            ++packed_len;  
            prev_ch = ch;
          }
          else
          {
            // ch == 0 --> end of datastr
            break;
          }
        }
        ++datastr;
      } // while
    } // if
    out_packedstr[packed_len] = 0;
  }
  return packed_len;
}

// char
int unpack_data_str(const char* packedstr,
  char* out_datastr, int datastr_size)
{
  int data_len = 0;
  if (packedstr && out_datastr && (datastr_size > 0))
  {
    if (*packedstr) // ensure the packedstr is not empty
    {
      char ch, prev_ch = 0;
      
      while ((ch = *packedstr) && (data_len + 1 < datastr_size))
      {
        if ((ch == '(') && (prev_ch))
        {
          int dups;
          
          ++packedstr; // next character: hex number expected
          dups = (int) strbase2uint(packedstr, 16);
          if (dups > 0)
          {
            --dups;
            if (dups > datastr_size - data_len - 1)
              dups = datastr_size - data_len - 1;
            while (dups--)
            {
              out_datastr[data_len++] = prev_ch;
            }
            while ((ch = *(++packedstr)) && (ch != ')')) ;
            if (ch == 0)
            {
              // end of packedstr
              break;
            }
          }
          else
          {
            prev_ch = '(';
            out_datastr[data_len] = '(';
            ++data_len;
            --packedstr;
          }
        }
        else if (ch == '\\')
        {
          ++packedstr; // next character
          ch = *packedstr;
          if ((ch == '(') || (ch == '\\'))
          {
            prev_ch = ch;
            out_datastr[data_len] = ch;
            ++data_len;
          }
          else
          {
            prev_ch = '\\';
            out_datastr[data_len] = '\\';
            ++data_len;
            --packedstr;
          }
        }
        else
        {
          prev_ch = ch;
          out_datastr[data_len] = ch;
          ++data_len;
        }
        ++packedstr;
      } // while
    } // if
    out_datastr[data_len] = 0;
  }
  return data_len;
}

/////////////////////////////////////////////////////////////////////////////

// wchar_t
int pack_data_strw(const wchar_t* datastrw,
  wchar_t* out_packedstrw, int packedstrw_size)
{
  int packedw_len = 0;
  if (datastrw && out_packedstrw && (packedstrw_size > 0))
  {
    if (*datastrw) // ensure the datastrw is not empty
    {
      wchar_t wch;
      wchar_t prev_wch = 0;
      int     dups = 0;

      for (;;)
      {
        wch = *datastrw;
        if (prev_wch == wch)
        {
          ++dups;
        }
        else
        {
          if (dups > 0)
          {
            if (dups >= 4)
            {
              wchar_t numstrw[2*sizeof(int) + 1];
              int     len;

              len = uint2strhexw(dups + 1, numstrw);
              if (packedw_len + len + 2 < packedstrw_size)
              {
                int i = 0;
                out_packedstrw[packedw_len] = L'(';
                ++packedw_len;
                while (i < len)
                {
                  out_packedstrw[packedw_len] = numstrw[i];
                  ++packedw_len;
                  ++i;
                }
                out_packedstrw[packedw_len] = L')';
                ++packedw_len;
              }
              else
              {
                // packedstrw_size is not enough
                break;
              }
            }
            else
            {
              if (packedw_len + dups < packedstrw_size)
              {
                while (dups > 0)
                {
                  out_packedstrw[packedw_len] = prev_wch;
                  ++packedw_len;
                  --dups;
                }
              }
              else
              {
                // packedstrw_size is not enough
                break;
              }
            }
            dups = 0;
          }
          if (wch)
          {
            if ((wch != L'(') && (wch != L'\\'))
            {
              if (packedw_len + 1 < packedstrw_size)
              {
                out_packedstrw[packedw_len] = wch;
              }
              else
              {
                // packedstrw_size is not enough
                break;
              }
            }
            else
            {
              if (packedw_len + 2 < packedstrw_size)
              {
                out_packedstrw[packedw_len] = L'\\';
                ++packedw_len;
                out_packedstrw[packedw_len] = wch;
              }
              else
              {
                // packedstrw_size is not enough
                break;
              }
            }
            ++packedw_len;
            prev_wch = wch;
          }
          else
          {
            // wch == 0 --> end of datastrw
            break;
          }
        }
        ++datastrw;
      } // while
    } // if
    out_packedstrw[packedw_len] = 0;
  }
  return packedw_len;
}

// wchar_t
int unpack_data_strw(const wchar_t* packedstrw,
  wchar_t* out_datastrw, int datastrw_size)
{
  int dataw_len = 0;
  if (packedstrw && out_datastrw && (datastrw_size > 0))
  {
    if (*packedstrw) // ensure the packedstr is not empty
    {
      wchar_t wch, prev_wch = 0;

      while ((wch = *packedstrw) && (dataw_len + 1 < datastrw_size))
      {
        if ((wch == L'(') && (prev_wch))
        {
          int dups;

          ++packedstrw; // next character: hex number expected
          dups = (int) wstrbase2uint(packedstrw, 16);
          if (dups > 0)
          {
            --dups;
            if (dups > datastrw_size - dataw_len - 1)
              dups = datastrw_size - dataw_len - 1;
            while (dups--)
            {
              out_datastrw[dataw_len++] = prev_wch;
            }
            while ((wch = *(++packedstrw)) && (wch != L')')) ;
            if (wch == 0)
            {
              // end of packedstrw
              break;
            }
          }
          else
          {
            prev_wch = L'(';
            out_datastrw[dataw_len] = L'(';
            ++dataw_len;
            --packedstrw;
          }
        }
        else if (wch == L'\\')
        {
          ++packedstrw; // next character
          wch = *packedstrw;
          if ((wch == L'(') || (wch == L'\\'))
          {
            prev_wch = wch;
            out_datastrw[dataw_len] = wch;
            ++dataw_len;
          }
          else
          {
            prev_wch = L'\\';
            out_datastrw[dataw_len] = L'\\';
            ++dataw_len;
            --packedstrw;
          }
        }
        else
        {
          prev_wch = wch;
          out_datastrw[dataw_len] = wch;
          ++dataw_len;
        }
        ++packedstrw;
      } // while
    } // if
    out_datastrw[dataw_len] = 0;
  }
  return dataw_len;
}


#ifdef __cplusplus
}
#endif
