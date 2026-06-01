#include "WarningMaskMatcher.h"
#include "NppExecHelpers.h"

#define  TCM_FILE1  _T('1')
#define  TCM_FILE2  _T('2')
#define  TCM_LINE   _T('3')
#define  TCM_CHAR   _T('4')

namespace {

constexpr int STACK_MAX_FRAMES = 20;

struct StarFrame
{
    const TCHAR* pm_rem;
    const TCHAR* ps_begin;
    const TCHAR* ps_current;
    TCHAR*       postr;
    bool         capture_enabled;
};

static inline const TCHAR* str_nul_end( const TCHAR* s )
{
    while ( *s ) ++s;
    return s;
}

static inline bool is_num_str( const TCHAR* s )
{
    if ( !*s )
        return false;

    while ( NppExecHelpers::IsAnySpaceChar( *s ) )
        ++s;

    if ( !*s )
        return false;

    if ( *s == _T( '-' ) || *s == _T( '+' ) )
        ++s;

    return ( *s >= _T( '0' ) ) && ( *s <= _T( '9' ) );
}

static inline bool mask_char_matches( TCHAR maskCh, TCHAR strCh )
{
    if ( maskCh == strCh )
        return true;
    if ( maskCh == _T( '?' ) && strCh != 0 )
        return true;
    if ( maskCh == _T( ' ' ) && NppExecHelpers::IsAnySpaceChar( strCh ) )
        return true;
    return false;
}

static inline bool is_absfile_drive_prefix( const TCHAR* pMask )
{
    return ( pMask[ 0 ] == _T( ':' ) )
        && ( pMask[ 1 ] == _T( '\\' ) )
        && ( pMask[ 2 ] == _T( '*' ) )
        && ( pMask[ 3 ] == TCM_FILE2 );
}

static inline bool str_has_abs_drive( const TCHAR* pStr )
{
    return ( pStr[ 0 ] == _T( ':' ) )
        && ( ( pStr[ 1 ] == _T( '\\' ) ) || ( pStr[ 1 ] == _T( '/' ) ) );
}

static inline bool try_match_abs_drive( const TCHAR*& pMask, const TCHAR*& pStr )
{
    if ( is_absfile_drive_prefix( pMask ) && str_has_abs_drive( pStr ) )
    {
        // Same as the legacy matcher: consume only ':' and '\\' in the mask and ':' + slash in the input.
        pMask += 2;
        pStr  += 2;
        return true;
    }
    return false;
}

static inline TCHAR* capture_buf_for_token( TCHAR token, TCHAR* postr1, TCHAR* postr2, TCHAR* postr3, TCHAR* postr4 )
{
    switch ( token )
    {
    case TCM_FILE1: return postr1;
    case TCM_FILE2: return postr2;
    case TCM_LINE:  return postr3;
    case TCM_CHAR:  return postr4;
    default:        return NULL;
    }
}

static inline bool is_line_or_char_capture( TCHAR* postr, TCHAR* postr3, TCHAR* postr4 )
{
    return ( postr == postr3 ) || ( postr == postr4 );
}

// True when remainder cannot use substring skip (?, space wildcard, ABS drive, more '*').
static bool mask_remainder_has_star_wildcard( const TCHAR* pMask )
{
    for ( const TCHAR* p = pMask; *p; ++p )
    {
        if ( *p == _T( '*' ) || *p == _T( '?' ) || *p == _T( ' ' ) )
            return true;
        if ( is_absfile_drive_prefix( p ) )
            return true;
    }
    return false;
}

static void write_capture( TCHAR* dst, const TCHAR* src, const TCHAR* stop )
{
    TCHAR* const dstEnd = dst + WARN_MAX_FILENAME;
    while ( ( dst < dstEnd ) && ( src < stop ) )
        *dst++ = *src++;
    *dst = 0;
}

static void finalize_captures( const StarFrame* frames, int frameCount )
{
    for ( int i = 0; i < frameCount; ++i )
    {
        if ( frames[ i ].capture_enabled && frames[ i ].postr )
            write_capture( frames[ i ].postr, frames[ i ].ps_begin, frames[ i ].ps_current );
    }
}

static bool push_star_frame( StarFrame* frames
                           , int& sp
                           , const TCHAR* pm_rem
                           , const TCHAR* ps_begin
                           , const TCHAR* ps_current
                           , TCHAR* postr )
{
    if ( sp >= STACK_MAX_FRAMES )
        return false;

    StarFrame& f    = frames[ sp++ ];
    f.pm_rem        = pm_rem;
    f.ps_begin      = ps_begin;
    f.ps_current    = ps_current;
    f.postr         = postr;
    f.capture_enabled = ( postr != NULL );
    return true;
}

// After a failed match, extend a '*' frame (plain wildcard uses substring skip when safe).
static bool advance_star_frame( StarFrame& frame )
{
    if ( *frame.ps_current == 0 )
        return false;

    if ( !frame.capture_enabled && *frame.pm_rem && !mask_remainder_has_star_wildcard( frame.pm_rem ) )
    {
        const TCHAR* const found = _tcsstr( frame.ps_current + 1, frame.pm_rem );
        frame.ps_current = found ? found : str_nul_end( frame.ps_begin );
        return true;
    }

    ++frame.ps_current;
    return true;
}

enum class MatchPhase { Forward, Backtrack };

} // namespace

bool match_mask_2( const TCHAR* mask
                 , const TCHAR* str
                 ,       TCHAR* postr1
                 ,       TCHAR* postr2
                 ,       TCHAR* postr3
                 ,       TCHAR* postr4
                 )
{
    if ( !mask || !str )
        return false;

    const TCHAR* pMask = mask;
    const TCHAR* pStr  = str;

    StarFrame frames[ STACK_MAX_FRAMES ];
    int       sp = 0;

    MatchPhase phase = MatchPhase::Forward;

    for ( ;; )
    {
        if ( phase == MatchPhase::Forward )
        {
            bool star_pushed = false;

            while ( *pMask )
            {
                if ( *pMask == _T( '*' ) )
                {
                    const TCHAR* const pAfterStar = pMask + 1;
                    TCHAR* const postr = capture_buf_for_token( *pAfterStar, postr1, postr2, postr3, postr4 );

                    if ( is_line_or_char_capture( postr, postr3, postr4 ) && !is_num_str( pStr ) )
                    {
                        phase = MatchPhase::Backtrack;
                        break;
                    }

                    pMask = *pAfterStar ? ( pAfterStar + 1 ) : pAfterStar;

                    if ( *pMask == 0 )
                    {
                        if ( !push_star_frame( frames, sp, pMask, pStr, str_nul_end( pStr ), postr ) )
                            return false;

                        finalize_captures( frames, sp );
                        return true;
                    }

                    if ( !push_star_frame( frames, sp, pMask, pStr, pStr, postr ) )
                        return false;

                    star_pushed = true;
                    break;
                }

                if ( try_match_abs_drive( pMask, pStr ) )
                    continue;

                if ( !mask_char_matches( *pMask, *pStr ) )
                {
                    phase = MatchPhase::Backtrack;
                    break;
                }

                ++pMask;
                ++pStr;
            }

            if ( star_pushed )
                continue;

            if ( phase == MatchPhase::Forward )
            {
                if ( *pMask == 0 && *pStr == 0 )
                {
                    finalize_captures( frames, sp );
                    return true;
                }
                phase = MatchPhase::Backtrack;
            }
        }

        if ( phase == MatchPhase::Backtrack )
        {
            bool resumed = false;

            while ( sp > 0 )
            {
                StarFrame& frame = frames[ sp - 1 ];

                if ( advance_star_frame( frame ) )
                {
                    pMask = frame.pm_rem;
                    pStr  = frame.ps_current;
                    phase = MatchPhase::Forward;
                    resumed = true;
                    break;
                }

                --sp;
            }

            if ( !resumed )
                return false;
        }
    }
}
