// >>>>>>>>>>>> match_mask v.1.1 (Jul 2007) >>>>>>>>>>>>
// >>>>
// >>>> usage:
// >>>>   match_mask("a*d?fg?*", "abcdefgh")
// >>>>     (returns 1)
// >>>>   match_mask("*??*c*f*g?.??e", "cccfgabcdefgh.exe")
// >>>>     (returns 1)
// >>>>

#include "MatchMask.h"


#ifdef __cplusplus
namespace c_base {
#endif


bool_t match_mask(const char* mask, const char* str)
{
  if (mask && str)
  {
    bool_t matched = 0;
    bool_t done = 0;
    
    while (!done)
    {
      if (*mask == '*') // 0 or more characters
      {
        ++mask;
        if (*mask == 0)
        {
          matched = 1;
        }
        else
        {
          matched = match_mask(mask, str);
          while ((!matched) && (*str != 0))
          {
            ++str;
            matched = match_mask(mask, str);
          }
        }
        done = 1;
      }
      else if (*mask == 0) // mask is over
      {
        matched = (*str == 0) ? 1 : 0;
        done = 1;
      }
      else
      { 
        if ( (*mask == *str) ||  // exact match, case-sensitive
             ((*mask == '?') && (*str != 0)) ) // any character
        {
          ++mask;
          ++str;
        }
        else
        {
          matched = 0;
          done = 1;
        }
      }
    }
    return matched;
  }
  return 0;
}

bool_t match_maskw(const wchar_t* maskw, const wchar_t* strw)
{
  if (maskw && strw)
  {
    bool_t matched = 0;
    bool_t done = 0;
    
    while (!done)
    {
      if (*maskw == L'*') // 0 or more characters
      {
        ++maskw;
        if (*maskw == 0)
        {
          matched = 1;
        }
        else
        {
          matched = match_maskw(maskw, strw);
          while ((!matched) && (*strw != 0))
          {
            ++strw;
            matched = match_maskw(maskw, strw);
          }
        }
        done = 1;
      }
      else if (*maskw == 0) // mask is over
      {
        matched = (*strw == 0) ? 1 : 0;
        done = 1;
      }
      else
      { 
        if ( (*maskw == *strw) ||  // exact match, case-sensitive
             ((*maskw == L'?') && (*strw != 0)) ) // any character
        {
          ++maskw;
          ++strw;
        }
        else
        {
          matched = 0;
          done = 1;
        }
      }
    }
    return matched;
  }
  return 0;
}


#ifdef __cplusplus
}
#endif
