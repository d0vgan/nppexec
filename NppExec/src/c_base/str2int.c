// >>>>>>>>>>>> str2int v.1.3 (Nov 2016) >>>>>>>>>>>>
// >>>>
// >>>> usage:
// >>>>   str2int("-1235.890") -> -1235    (DEC, signed)
// >>>>   str2int("-0xFAE37")  -> -1027639 (HEX, signed)
// >>>>   str2int("0b1001110)  ->  78      (BIN, signed)
// >>>>   str2int("053162")    ->  22130   (OCT, signed)
// >>>>   str2int("$FAE37")    ->  1027639 (HEX, signed)
// >>>>
// >>>> new from ver. 1.0.3: 
// >>>>   str2int("10M")   -> 10*1024*1024 [M or m == Mega]
// >>>>   str2int("$10k")  -> 16*1024      [k or K == Kilo]
// >>>>   str2int("0x10m") -> 16*1024*1024 [M or m == Mega]

#include "str2int.h"
#include "max_int.h"


#ifdef __cplusplus
namespace c_base {
#endif


static unsigned int char2uint(const char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ( (unsigned int) (ch - '0') );
    
    if ((ch >= 'A') && (ch <= 'Z'))
        return ( (unsigned int) (ch - 'A' + 10) );

    if ((ch >= 'a') && (ch <= 'z'))
        return ( (unsigned int) (ch - 'a' + 10) );

    return 0;
}

static int find_wrongsymb_pos(const char* str, unsigned int base)
{
    int  i = 0;
    char ch;
    char max_ch = 'A' + ((char)base - 11);
    
    while ((ch = str[i]) != 0)
    {
        if ((ch < '0') || (ch > '9'))
        {
            if ((ch >= 'a') && (ch <= 'z'))
            {
                ch -= 'a';
                ch += 'A';
            }
            if ((ch < 'A') || (ch > max_ch))
                break;
        }
        ++i;
    }
    return i;
}

static int find_wrongint_pos(const char* str, unsigned int base)
{
    int  i = 0;
    char ch;
    char max_ch = '0' + ((char)base - 1);
    while ((ch = str[i]) != 0)
    {
        if ((ch < '0') || (ch > max_ch))
            break;
        ++i;
    }
    return i;
}

unsigned int strbase2uint(const char* str, unsigned int base)
{
    if (str && (str[0]) && (base < 36))
    {
        int len = ((base <= 10) ? 
          find_wrongint_pos(str, base) : find_wrongsymb_pos(str, base));

        if (len > 0)
        {
            unsigned int result = 0;
            unsigned int digit_multiplier = 1;
            int          i = len;

            while (--i >= 0)
            {
                result += char2uint(str[i])*digit_multiplier;
                digit_multiplier *= base;
            }
          
            if (result && (base <= 20))
            {
                if ((str[len] == 'M') || (str[len] == 'm'))
                    result *= (1024*1024);
                else if ((str[len] == 'k') || (str[len] == 'K'))
                    result *= 1024;
            }
            return result;
        }
    }
    return 0;
}

unsigned __int64 strbase2uint64(const char* str, unsigned int base)
{
    if (str && (str[0]) && (base < 36))
    {
        int len = ((base <= 10) ? 
          find_wrongint_pos(str, base) : find_wrongsymb_pos(str, base));

        if (len > 0)
        {
            unsigned __int64 result = 0;
            unsigned __int64 digit_multiplier = 1;
            int              i = len;

            while (--i >= 0)
            {
                result += char2uint(str[i])*digit_multiplier;
                digit_multiplier *= base;
            }
          
            if (result && (base <= 20))
            {
                if ((str[len] == 'M') || (str[len] == 'm'))
                    result *= (1024*1024);
                else if ((str[len] == 'k') || (str[len] == 'K'))
                    result *= 1024;
            }
            return result;
        }
    }
    return 0;
}

// (str != 0) && (str[0] != 0)
static unsigned int str2uint_subfunc(const char* str, bool_t* sign)
{
    *sign = 0;

    if (str[0] == '-')  // -<number>
    {
        ++str;
        *sign = 1;
    }
    else if (str[0] == '+')  // +<number>
    {
        ++str;
    }

    if (str[0] == '0')
    {
        switch (str[1])
        {
            case 0: // "0"
            {
                return 0;
            }
            case 'x': // "0x..."
            case 'X': // "0X..."
            {
                // HEX value
                return strbase2uint(str + 2, 16);
            }
            case 'b': // "0b..."
            case 'B': // "0B..."
            {
                // BIN value
                return strbase2uint(str + 2, 2);
            }
        #if LEADING_00_IS_DEC_VALUE
            case '0': // "00..."
            {
                // DEC value with leading zeros
                return strbase2uint(str + 2, 10);
            }
        #endif
            default: // "0..."
            {
                // OCT value
                return strbase2uint(str + 1, 8);
            }
        }
    }
    if (str[0] == '$')
    {
        // HEX value
        return strbase2uint(str + 1, 16);
    }

    // DEC value
    return strbase2uint(str, 10);
}

// (str != 0) && (str[0] != 0)
static unsigned __int64 str2uint64_subfunc(const char* str, bool_t* sign)
{
    *sign = 0;

    if (str[0] == '-')  // -<number>
    {
        ++str;
        *sign = 1;
    }
    else if (str[0] == '+')  // +<number>
    {
        ++str;
    }

    if (str[0] == '0')
    {
        switch (str[1])
        {
            case 0: // "0"
            {
                return 0;
            }
            case 'x': // "0x..."
            case 'X': // "0X..."
            {
                // HEX value
                return strbase2uint64(str + 2, 16);
            }
            case 'b': // "0b..."
            case 'B': // "0B..."
            {
                // BIN value
                return strbase2uint64(str + 2, 2);
            }
        #if LEADING_00_IS_DEC_VALUE
            case '0': // "00..."
            {
                // DEC value with leading zeros
                return strbase2uint64(str + 2, 10);
            }
        #endif
            default: // "0..."
            {
                // OCT value
                return strbase2uint64(str + 1, 8);
            }
        }
    }
    if (str[0] == '$')
    {
        // HEX value
        return strbase2uint64(str + 1, 16);
    }

    // DEC value
    return strbase2uint64(str, 10);
}

unsigned int str2uint(const char* str)
{
    if (str && (str[0]))
    {
        bool_t       sign = 0;
        unsigned int result = str2uint_subfunc(str, &sign);
      
        return ( sign ? ((max_uint() - result) + 1) : result );
    }
    return 0;
}

unsigned __int64 str2uint64(const char* str)
{
    if (str && (str[0]))
    {
        bool_t           sign = 0;
        unsigned __int64 result = str2uint64_subfunc(str, &sign);
      
        return ( sign ? ((max_uint64() - result) + 1) : result );
    }
    return 0;
}

int str2int(const char* str)
{
    if (str && (str[0]))
    {
        bool_t sign = 0;
        int    result = (int) str2uint_subfunc(str, &sign);

        return ( sign ? (-result) : result );
    }
    return 0;
}

__int64 str2int64(const char* str)
{
    if (str && (str[0]))
    {
        bool_t  sign = 0;
        __int64 result = (__int64) str2uint64_subfunc(str, &sign);

        return ( sign ? (-result) : result );
    }
    return 0;
}

bool_t is_dec_value(const char* str)
{
    if ( (!str) || (str[0] == 0) )  // empty string
        return 0;
    
    if ( (str[0] == '-') || (str[0] == '+') )
        ++str;
    
    if ( (str[0] < '0') || (str[0] > '9') )
        return 0;
    
    if ( str[0] == '0' )  // 0...
    {
        switch ( str[1] )
        {
            case 0:       // 0
                return 1;  
            case 'x':     // 0x<HEX>
            case 'X':     // 0X<HEX>
            case 'b':     // 0b<BIN>
            case 'B':     // 0B<BIN>
                return 0;
        #if LEADING_00_IS_DEC_VALUE
            case '0':     // 00<DEC>
                return 1;
        #endif
            default:      // 0<OCT>
                return 0;  
        }
    }
  
    return 1;
}

//---------------------------------------------------------------------------

static unsigned int char2uintw(const wchar_t wch)
{
    if ((wch >= L'0') && (wch <= L'9'))
        return ( (unsigned int) (wch - L'0') );
    
    if ((wch >= L'A') && (wch <= L'Z'))
        return ( (unsigned int) (wch - L'A' + 10) );

    if ((wch >= L'a') && (wch <= L'z'))
        return ( (unsigned int) (wch - L'a' + 10) );

    return 0;
}

static int find_wrongsymb_posw(const wchar_t* strw, unsigned int base)
{
    int     i = 0;
    wchar_t wch;
    wchar_t max_wch = L'A' + ((wchar_t)base - 11);
    
    while ((wch = strw[i]) != 0)
    {
        if ((wch < L'0') || (wch > L'9'))
        {
            if ((wch >= L'a') && (wch <= L'z'))
            {
                wch -= L'a';
                wch += L'A';
            }
            if ((wch < L'A') || (wch > max_wch))
                break;
        }
        ++i;
    }
    return i;
}

static int find_wrongint_posw(const wchar_t* strw, unsigned int base)
{
    int     i = 0;
    wchar_t wch;
    wchar_t max_wch = L'0' + ((wchar_t)base - 1);
    
    while ((wch = strw[i]) != 0)
    {
        if ((wch < L'0') || (wch > max_wch))
            break;
        ++i;
    }
    return i;
}

unsigned int wstrbase2uint(const wchar_t* strw, unsigned int base)
{
    if (strw && (strw[0]) && (base < 36))
    {
        int len = ((base <= 10) ? 
          find_wrongint_posw(strw, base) : find_wrongsymb_posw(strw, base));

        if (len > 0)
        {
            unsigned int result = 0;
            unsigned int digit_multiplier = 1;
            int          i = len;

            while (--i >= 0)
            {
                result += char2uintw(strw[i])*digit_multiplier;
                digit_multiplier *= base;
            }
          
            if (result && (base <= 20))
            {
                if ((strw[len] == L'M') || (strw[len] == L'm'))
                    result *= (1024*1024);
                else if ((strw[len] == L'k') || (strw[len] == L'K'))
                    result *= 1024;
            }
            return result;
        }
    }
    return 0;
}

unsigned __int64 wstrbase2uint64(const wchar_t* strw, unsigned int base)
{
    if (strw && (strw[0]) && (base < 36))
    {
        int len = ((base <= 10) ? 
          find_wrongint_posw(strw, base) : find_wrongsymb_posw(strw, base));

        if (len > 0)
        {
            unsigned __int64 result = 0;
            unsigned __int64 digit_multiplier = 1;
            int              i = len;

            while (--i >= 0)
            {
                result += char2uintw(strw[i])*digit_multiplier;
                digit_multiplier *= base;
            }
          
            if (result && (base <= 20))
            {
                if ((strw[len] == L'M') || (strw[len] == L'm'))
                    result *= (1024*1024);
                else if ((strw[len] == L'k') || (strw[len] == L'K'))
                    result *= 1024;
            }
            return result;
        }
    }
    return 0;
}

// (strw != 0) && (strw[0] != 0)
static unsigned int str2uint_subfuncw(const wchar_t* strw, bool_t* sign)
{
    *sign = 0;

    if (strw[0] == L'-')  // -<number>
    {
        ++strw;
        *sign = 1;
    }
    else if (strw[0] == L'+')  // +<number>
    {
        ++strw;
    }

    if (strw[0] == L'0')
    {
        switch (strw[1])
        {
            case 0: // "0"
            {
                return 0;
            }
            case L'x': // "0x..."
            case L'X': // "0X..."
            {
                // HEX value
                return wstrbase2uint(strw + 2, 16);
            }
            case L'b': // "0b..."
            case L'B': // "0B..."
            {
                // BIN value
                return wstrbase2uint(strw + 2, 2);
            }
        #if LEADING_00_IS_DEC_VALUE
            case L'0': // "00..."
            {
                // DEC value with leading zeros
                return wstrbase2uint(strw + 2, 10);
            }
        #endif
            default: // "0..."
            {
                // OCT value
                return wstrbase2uint(strw + 1, 8);
            }
        }
    }
    if (strw[0] == L'$')
    {
        // HEX value
        return wstrbase2uint(strw + 1, 16);
    }

    // DEC value
    return wstrbase2uint(strw, 10);
}

// (strw != 0) && (strw[0] != 0)
static unsigned __int64 str2uint64_subfuncw(const wchar_t* strw, bool_t* sign)
{
    *sign = 0;

    if (strw[0] == L'-')  // -<number>
    {
        ++strw;
        *sign = 1;
    }
    else if (strw[0] == L'+')  // +<number>
    {
        ++strw;
    }

    if (strw[0] == L'0')
    {
        switch (strw[1])
        {
            case 0: // "0"
            {
                return 0;
            }
            case L'x': // "0x..."
            case L'X': // "0X..."
            {
                // HEX value
                return wstrbase2uint64(strw + 2, 16);
            }
            case L'b': // "0b..."
            case L'B': // "0B..."
            {
                // BIN value
                return wstrbase2uint64(strw + 2, 2);
            }
        #if LEADING_00_IS_DEC_VALUE
            case L'0': // "00..."
            {
                // DEC value with leading zeros
                return wstrbase2uint64(strw + 2, 10);
            }
        #endif
            default: // "0..."
            {
                // OCT value
                return wstrbase2uint64(strw + 1, 8);
            }
        }
    }
    if (strw[0] == L'$')
    {
        // HEX value
        return wstrbase2uint64(strw + 1, 16);
    }

    // DEC value
    return wstrbase2uint64(strw, 10);
}

unsigned int wstr2uint(const wchar_t* strw)
{
    if (strw && (strw[0]))
    {
        bool_t       sign = 0;
        unsigned int result = str2uint_subfuncw(strw, &sign);
      
        return ( sign ? ((max_uint() - result) + 1) : result );
    }
    return 0;
}

unsigned __int64 wstr2uint64(const wchar_t* strw)
{
    if (strw && (strw[0]))
    {
        bool_t           sign = 0;
        unsigned __int64 result = str2uint64_subfuncw(strw, &sign);
      
        return ( sign ? ((max_uint64() - result) + 1) : result );
    }
    return 0;
}

int wstr2int(const wchar_t* strw)
{
    if (strw && (strw[0]))
    {
        bool_t sign = 0;
        int    result = (int) str2uint_subfuncw(strw, &sign);

        return ( sign ? (-result) : result );
    }
    return 0;
}

__int64 wstr2int64(const wchar_t* strw)
{
    if (strw && (strw[0]))
    {
        bool_t  sign = 0;
        __int64 result = (__int64) str2uint64_subfuncw(strw, &sign);

        return ( sign ? (-result) : result );
    }
    return 0;
}

bool_t is_dec_valuew(const wchar_t* strw)
{
    if ( (!strw) || (strw[0] == 0) )  // empty string
        return 0;
    
    if ( (strw[0] == L'-') || (strw[0] == L'+') )
        ++strw;

    if ( (strw[0] < L'0') || (strw[0] > L'9') )
        return 0;
    
    if ( strw[0] == L'0' )  // 0...
    {
        switch ( strw[1] )
        {
            case 0:       // 0
                return 1;  
            case L'x':     // 0x<HEX>
            case L'X':     // 0X<HEX>
            case L'b':     // 0b<BIN>
            case L'B':     // 0B<BIN>
                return 0;
        #if LEADING_00_IS_DEC_VALUE
            case L'0':     // 00<DEC>
                return 1;
        #endif
            default:      // 0<OCT>
                return 0;  
        }
    }
  
    return 1;
}


#ifdef __cplusplus
}
#endif
