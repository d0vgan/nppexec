#include "../NppExec/src/WarningMaskMatcher.h"
#include "../NppExec/src/WarningAnalyzer.h"
#include "../NppExec/src/NppExecHelpers.h"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#define  TCM_FILE1  _T('1')
#define  TCM_FILE2  _T('2')
#define  TCM_LINE   _T('3')
#define  TCM_CHAR   _T('4')

// Reference implementation: copied from the old recursive match_mask_2()
// (kept only for test verification).
static bool is_num_str_ref( const TCHAR* s )
{
    if ( *s )
    {
        while ( NppExecHelpers::IsAnySpaceChar(*s) )  ++s;
        if ( *s )
        {
            if ( (*s == _T('-')) || (*s == _T('+')) )
                ++s;

            if ( (*s >= _T('0')) && (*s <= _T('9')) )
                return true;
        }
    }

    return false;
}

static bool match_mask_2_recursive_ref( const TCHAR* mask
                                     , const TCHAR* str
                                     ,       TCHAR* postr1
                                     ,       TCHAR* postr2
                                     ,       TCHAR* postr3
                                     ,       TCHAR* postr4
                                     )
{
    if ( mask && str )
    {
        const TCHAR* pMask = mask;
        const TCHAR* pStr  = str;
        TCHAR* postr   = NULL;
        TCHAR* postr0  = NULL;
        bool   matched = false;
        bool   done    = false;

        while (!done)
        {
            if ( *pMask == _T('*') ) // 0 or more characters
            {
                ++pMask;
                if ( *pMask == TCM_FILE1 )
                {
                    postr = postr1;
                }
                else if ( *pMask == TCM_FILE2 )
                {
                    postr = postr2;
                }
                else if ( *pMask == TCM_LINE )
                {
                    postr = postr3; // a number expected
                }
                else if ( *pMask == TCM_CHAR )
                {
                    postr = postr4; // a number expected
                }
                else
                {
                    postr = NULL;
                }
                postr0 = postr;

                if ( ( (postr == postr3) || (postr == postr4) )
                  && ( !is_num_str_ref(pStr) )
                   )
                {
                    // pStr is expected to contain a number, but it's not
                    matched = false;
                }
                else
                {
                    if (*pMask) ++pMask;

                    if (*pMask == 0)
                    {
                        matched = true;
                        if ( postr )
                        {
                            while ( (postr < postr0 + WARN_MAX_FILENAME) && (*postr++ = *pStr++) ) ;
                            *postr = 0;
                        }
                    }
                    else
                    {
                        while ( ( !( matched = match_mask_2_recursive_ref( pMask, pStr, postr1, postr2, postr3, postr4 ) ) )
                              &&( *pStr != 0 )
                              )
                        {
                            if ( postr )
                            {
                                if ( postr < postr0 + WARN_MAX_FILENAME )
                                {
                                    *postr++ = *pStr;
                                }
                            }
                            pStr++;
                        }
                        if ( postr )
                        {
                            *postr = 0;
                        }
                    }
                }
                done = true;
            }
            else if ( ( pMask[0] == _T(':')   ) // ABS PATH can be drive:\... or drive:/...
                    &&( pMask[1] == _T('\\')  )
                    &&( pMask[2] == _T('*')   )
                    &&( pMask[3] == TCM_FILE2 )
                    &&( pStr[0] == _T(':')    )
                    &&( ( pStr[1] == _T('\\') )
                      ||( pStr[1] == _T('/')  )
                      )
                    )
            {
                ++pMask;
                ++pStr;
                ++pMask;
                ++pStr;
            }
            else if ( *pMask == 0 ) // mask is over
            {
                matched = (*pStr == 0) ? true : false;
                done = true;
            }
            else
            {
                if ( ( *pMask == *pStr ) // exact match, case-sensitive
                  || ( (*pMask == _T('?')) && (*pStr != 0) ) // any character
                  || ( (*pMask == _T(' ')) && NppExecHelpers::IsAnySpaceChar(*pStr) ) // tab treated as space
                   )
                {
                    ++pMask;
                    ++pStr;
                }
                else
                {
                    matched = false;
                    done = true;
                }
            }
        }
        return matched;
    }
    return false;
}

namespace
{
    constexpr size_t OUT_BUF_LEN = (WARN_MAX_FILENAME + 5);

    static void init_out( TCHAR* buf )
    {
        for ( size_t i = 0; i < OUT_BUF_LEN; i++ )
            buf[i] = _T('#');

        buf[0] = 0;
    }

    static void assert_out_eq( const TCHAR* a, const TCHAR* b )
    {
        for ( size_t i = 0; i < OUT_BUF_LEN; i++ )
            assert( a[i] == b[i] );
    }

    static void assert_cstr_eq_up_to_nul( const TCHAR* a, const TCHAR* b )
    {
        for ( size_t i = 0; i < OUT_BUF_LEN; i++ )
        {
            assert( a[i] == b[i] );
            if ( a[i] == 0 )
                return;
        }
        // If we got here, no terminator in range.
        assert( false );
    }

    static void run_case_bool_only( const TCHAR* mask, const TCHAR* str )
    {
        TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
        TCHAR r1[OUT_BUF_LEN], r2[OUT_BUF_LEN], r3[OUT_BUF_LEN], r4[OUT_BUF_LEN];

        init_out( o1 ); init_out( o2 ); init_out( o3 ); init_out( o4 );
        init_out( r1 ); init_out( r2 ); init_out( r3 ); init_out( r4 );

        const bool prod = match_mask_2( mask, str, o1, o2, o3, o4 );
        const bool ref  = match_mask_2_recursive_ref( mask, str, r1, r2, r3, r4 );
        assert( prod == ref );
    }

    static void run_case( const TCHAR* mask, const TCHAR* str )
    {
        TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
        TCHAR r1[OUT_BUF_LEN], r2[OUT_BUF_LEN], r3[OUT_BUF_LEN], r4[OUT_BUF_LEN];

        init_out( o1 ); init_out( o2 ); init_out( o3 ); init_out( o4 );
        init_out( r1 ); init_out( r2 ); init_out( r3 ); init_out( r4 );

        const bool prod = match_mask_2( mask, str, o1, o2, o3, o4 );
        const bool ref  = match_mask_2_recursive_ref( mask, str, r1, r2, r3, r4 );

        assert( prod == ref );
        if ( !prod )
            return; // On failure, outputs may differ by design (written only on success).

        // Iterative matcher fills captures only after success; recursive may leave '#' padding.
        assert( _tcscmp( o1, r1 ) == 0 );
        assert( _tcscmp( o2, r2 ) == 0 );
        assert( _tcscmp( o3, r3 ) == 0 );
        assert( _tcscmp( o4, r4 ) == 0 );
    }

    static void run_case_expect_captures( const TCHAR* mask
                                          , const TCHAR* str
                                          , const TCHAR* e1
                                          , const TCHAR* e2
                                          , const TCHAR* e3
                                          , const TCHAR* e4
                                          )
    {
        TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
        TCHAR r1[OUT_BUF_LEN], r2[OUT_BUF_LEN], r3[OUT_BUF_LEN], r4[OUT_BUF_LEN];

        init_out( o1 ); init_out( o2 ); init_out( o3 ); init_out( o4 );
        init_out( r1 ); init_out( r2 ); init_out( r3 ); init_out( r4 );

        const bool prod = match_mask_2( mask, str, o1, o2, o3, o4 );
        const bool ref  = match_mask_2_recursive_ref( mask, str, r1, r2, r3, r4 );

        assert( prod == ref );
        assert( prod == true ); // this helper is for success cases only

        // Verify against recursive reference (prevents “both wrong”).
        assert( _tcscmp( o1, r1 ) == 0 );
        assert( _tcscmp( o2, r2 ) == 0 );
        assert( _tcscmp( o3, r3 ) == 0 );
        assert( _tcscmp( o4, r4 ) == 0 );

        assert( _tcscmp( o1, e1 ) == 0 );
        assert( _tcscmp( o2, e2 ) == 0 );
        assert( _tcscmp( o3, e3 ) == 0 );
        assert( _tcscmp( o4, e4 ) == 0 );
    }

    static void build_random_internal_mask( std::mt19937& rng, std::basic_string<TCHAR>& outMask )
    {
        struct Token { const TCHAR* s; };

        static const Token tokens[] = {
            { _T("**") },  // wildcard
            { _T("*1") },  // file1 capture
            { _T("*2") },  // file2 capture
            { _T("*3") },  // line capture
            { _T("*4") },  // char capture
        };

        static const TCHAR literalChars[] = {
            _T('A'), _T('B'), _T('C'), _T('X'), _T('Y'),
            _T('a'), _T('b'), _T('c'),
            _T('0'), _T('1'), _T('-'), _T('+'),
            _T('?'),
            _T(':'), _T(' '), _T('\\'), _T('/'),
        };

        std::uniform_int_distribution<int> tokPick( 0, 4 );
        std::uniform_int_distribution<int> litPick( 0, (int)(sizeof(literalChars)/sizeof(literalChars[0])) - 1 );

        std::uniform_int_distribution<int> lenPick( 1, 8 );
        const int tokenCount = lenPick( rng );

        // Limit the amount of '*' driven choices to keep fuzz fast.
        int starTokenCount = 0;

        outMask.clear();
        outMask.reserve( 32 );

        for ( int i = 0; i < tokenCount; i++ )
        {
            const int choice = tokPick( rng );
            if ( starTokenCount < 3 && (choice <= 4) && (rng() % 100 < 70) )
            {
                outMask += tokens[ choice ].s;
                starTokenCount++;
            }
            else
            {
                outMask += literalChars[ litPick( rng ) ];
            }
        }

        // Ensure the mask is non-empty.
        if ( outMask.empty() )
            outMask = _T("**A");
    }

    static void build_random_string( std::mt19937& rng, std::basic_string<TCHAR>& outStr )
    {
        static const TCHAR chars[] = {
            _T('A'), _T('B'), _T('C'), _T('X'), _T('Y'),
            _T('a'), _T('b'), _T('c'),
            _T('0'), _T('1'), _T('2'), _T('9'),
            _T('-'), _T('+'),
            _T('?'),
            _T(':'), _T('\\'), _T('/'),
            _T(' '), _T('\t'),
        };

        std::uniform_int_distribution<int> pick( 0, (int)(sizeof(chars)/sizeof(chars[0])) - 1 );
        std::uniform_int_distribution<int> lenPick( 0, 10 );

        const int len = lenPick( rng );
        outStr.clear();
        outStr.reserve( 16 );
        for ( int i = 0; i < len; i++ )
            outStr += chars[ pick( rng ) ];
    }

    // Mirror of WarningAnalyzer::preprocessMask() (not exported from the .cpp).
    static void preprocessMask_test( TCHAR* outMask, const TCHAR* inMask, unsigned int& outMaskType )
    {
        outMaskType = CWarningAnalyzer::MT_NONE;

        while ( *inMask )
        {
            if ( *inMask == _T('%') )
            {
                int len = 0;

                if ( ( inMask[ 1 ] == _T('A') ) && ( inMask[ 2 ] == _T('%') ) )
                    len = 3;
                else if ( ( inMask[ 1 ] == _T('A') ) && ( inMask[ 2 ] == _T('B') )
                       && ( inMask[ 3 ] == _T('S') ) && ( inMask[ 4 ] == _T('F') )
                       && ( inMask[ 5 ] == _T('I') ) && ( inMask[ 6 ] == _T('L') )
                       && ( inMask[ 7 ] == _T('E') ) && ( inMask[ 8 ] == _T('%') ) )
                    len = 9;

                if ( len > 0 )
                {
                    inMask += len;
                    *outMask++ = _T('*');
                    *outMask++ = TCM_FILE1;
                    *outMask++ = _T(':');
                    *outMask++ = _T('\\');
                    *outMask++ = _T('*');
                    *outMask++ = TCM_FILE2;
                    outMaskType |= CWarningAnalyzer::MT_ABSFILE;
                }

                if ( len == 0 )
                {
                    if ( ( inMask[ 1 ] == _T('F') ) && ( inMask[ 2 ] == _T('%') ) )
                        len = 3;
                    else if ( ( inMask[ 1 ] == _T('F') ) && ( inMask[ 2 ] == _T('I') )
                           && ( inMask[ 3 ] == _T('L') ) && ( inMask[ 4 ] == _T('E') )
                           && ( inMask[ 5 ] == _T('%') ) )
                        len = 6;

                    if ( len > 0 )
                    {
                        inMask += len;
                        *outMask++ = _T('*');
                        *outMask++ = TCM_FILE1;
                        outMaskType |= CWarningAnalyzer::MT_FILE;
                    }
                }

                if ( len == 0 )
                {
                    if ( ( inMask[ 1 ] == _T('L') ) && ( inMask[ 2 ] == _T('%') ) )
                        len = 3;
                    else if ( ( inMask[ 1 ] == _T('L') ) && ( inMask[ 2 ] == _T('I') )
                           && ( inMask[ 3 ] == _T('N') ) && ( inMask[ 4 ] == _T('E') )
                           && ( inMask[ 5 ] == _T('%') ) )
                        len = 6;

                    if ( len > 0 )
                    {
                        inMask += len;
                        *outMask++ = _T('*');
                        *outMask++ = TCM_LINE;
                        outMaskType |= CWarningAnalyzer::MT_LINE;
                    }
                }

                if ( len == 0 )
                {
                    if ( ( inMask[ 1 ] == _T('C') ) && ( inMask[ 2 ] == _T('%') ) )
                        len = 3;
                    else if ( ( inMask[ 1 ] == _T('C') ) && ( inMask[ 2 ] == _T('H') )
                           && ( inMask[ 3 ] == _T('A') ) && ( inMask[ 4 ] == _T('R') )
                           && ( inMask[ 5 ] == _T('%') ) )
                        len = 6;

                    if ( len > 0 )
                    {
                        inMask += len;
                        *outMask++ = _T('*');
                        *outMask++ = TCM_CHAR;
                        outMaskType |= CWarningAnalyzer::MT_CHAR;
                    }
                }

                if ( len == 0 )
                    *outMask++ = *inMask++;
            }
            else if ( *inMask == _T('*') )
            {
                *outMask++ = *inMask;
                *outMask++ = *inMask++;
            }
            else
                *outMask++ = *inMask++;
        }
        *outMask = 0;
    }

    static bool is_valid_absfile_drive_capture( const TCHAR* s )
    {
        if ( !s || !s[0] || s[1] )
            return false;

        const TCHAR c = s[0];
        return ( ( c >= _T('A') ) && ( c <= _T('Z') ) )
            || ( ( c >= _T('a') ) && ( c <= _T('z') ) );
    }

    static void transform_file_mask_to_abs( TCHAR* outAbs, const TCHAR* internalMask )
    {
        TCHAR* pMskAbs = outAbs;
        const TCHAR* pMsk = internalMask;
        TCHAR ch = 1;
        do {
            ch = *(pMsk++);
            *(pMskAbs++) = ch;
            if ( ch == _T('*') )
            {
                ch = *(pMsk++);
                *(pMskAbs++) = ch;
                if ( ch == TCM_FILE1 )
                {
                    *(pMskAbs++) = _T(':');
                    *(pMskAbs++) = _T('\\');
                    *(pMskAbs++) = _T('*');
                    *(pMskAbs++) = TCM_FILE2;
                }
            }
        } while ( ch != 0 );
    }

    // Mirrors CWarningAnalyzer::match() mask logic for a single %FILE% filter.
    static bool match_like_warning_analyzer( const TCHAR* internalMask
                                            , unsigned int maskType
                                            , const TCHAR* str
                                            ,       TCHAR* o1
                                            ,       TCHAR* o2
                                            ,       TCHAR* o3
                                            ,       TCHAR* o4
                                            )
    {
        if ( (maskType & CWarningAnalyzer::MT_FILE)
          && !(maskType & CWarningAnalyzer::MT_ABSFILE) )
        {
            TCHAR absMask[WARN_MASK_SIZE + 10];
            transform_file_mask_to_abs( absMask, internalMask );
            if ( ( _tcsstr( internalMask, _T("\"*1") ) == NULL )
              && match_mask_2( absMask, str, o1, o2, o3, o4 )
              && is_valid_absfile_drive_capture( o1 ) )
                return true;
        }
        return match_mask_2( internalMask, str, o1, o2, o3, o4 );
    }

    // Full WarningAnalyzer-style match (%FILE% / %ABSFILE% promotion included).
    static void run_user_mask_expect_captures( const TCHAR* userMask
                                             , const TCHAR* str
                                             , const TCHAR* e1
                                             , const TCHAR* e2
                                             , const TCHAR* e3
                                             , const TCHAR* e4
                                             )
    {
        TCHAR internalMask[WARN_MASK_SIZE];
        unsigned int maskType = CWarningAnalyzer::MT_NONE;
        preprocessMask_test( internalMask, userMask, maskType );

        TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
        init_out( o1 ); init_out( o2 ); init_out( o3 ); init_out( o4 );

        const bool ok = match_like_warning_analyzer( internalMask, maskType, str, o1, o2, o3, o4 );
        assert( ok == true );
        assert( _tcscmp( o1, e1 ) == 0 );
        assert( _tcscmp( o3, e3 ) == 0 );
        if ( e2 && e2[0] )
            assert( _tcscmp( o2, e2 ) == 0 );
        if ( e4 && e4[0] )
            assert( _tcscmp( o4, e4 ) == 0 );
    }

    static void run_user_mask_expect_fail( const TCHAR* userMask, const TCHAR* str )
    {
        TCHAR internalMask[WARN_MASK_SIZE];
        unsigned int maskType = CWarningAnalyzer::MT_NONE;
        preprocessMask_test( internalMask, userMask, maskType );

        TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
        init_out( o1 ); init_out( o2 ); init_out( o3 ); init_out( o4 );

        const bool ok = match_like_warning_analyzer( internalMask, maskType, str, o1, o2, o3, o4 );
        assert( ok == false );

        run_case_bool_only( internalMask, str );
    }

    static void run_user_mask_case( const TCHAR* userMask
                                  , const TCHAR* str
                                  , const TCHAR* e1
                                  , const TCHAR* e3
                                  )
    {
        run_user_mask_expect_captures( userMask, str, e1, _T(""), e3, _T("") );
    }

    // Iterative matcher vs recursive reference, plus user-mask path with the same captures.
    static bool user_mask_has_placeholders( const TCHAR* userMask )
    {
        return ( userMask != NULL ) && ( _tcschr( userMask, _T( '%' ) ) != NULL );
    }

    static void run_trusted_compiler_case( const TCHAR* userMask
                                         , const TCHAR* internalMask
                                         , const TCHAR* str
                                         , const TCHAR* e1
                                         , const TCHAR* e2
                                         , const TCHAR* e3
                                         , const TCHAR* e4
                                         )
    {
        run_case_expect_captures( internalMask, str, e1, e2, e3, e4 );
        if ( user_mask_has_placeholders( userMask ) )
            run_user_mask_expect_captures( userMask, str, e1, e2, e3, e4 );
    }

    static void run_trusted_compiler_reject( const TCHAR* userMask
                                           , const TCHAR* internalMask
                                           , const TCHAR* str
                                           )
    {
        run_case_bool_only( internalMask, str );
        if ( user_mask_has_placeholders( userMask ) )
            run_user_mask_expect_fail( userMask, str );
    }

    static void tests_compiler_message_styles()
    {
        // --- C / C++ toolchains (GCC, Clang, MinGW) ---
        {
            const TCHAR* line = _T("C:/project/src/main.c:42:5: error: expected ';' before '}' token");

            run_trusted_compiler_case(
                _T("%ABSFILE%:%LINE%:%CHAR%: error:*")
              , _T("*1:\\*2:*3:*4: **error**")
              , line
              , _T("C"), _T("project/src/main.c"), _T("42"), _T("5") );

            run_trusted_compiler_case(
                _T("%ABSFILE%:%LINE%: warning:*")
              , _T("*1:\\*2:*3: **warning**")
              , _T("D:/build/foo.cpp:1001: warning: unused variable 'x'")
              , _T("D"), _T("build/foo.cpp"), _T("1001"), _T("") );

            // Windows backslash path: %FILE% uses plain *1 (drive-letter ABS promotion is rejected).
            run_case_expect_captures(
                _T("*1:*3: **warning**")
              , _T("D:\\build\\foo.cpp:1001: warning: unused variable 'x'")
              , _T("D:\\build\\foo.cpp"), _T(""), _T("1001"), _T("") );
            // %FILE% promotion splits D: + path (o1=D, o2=build\foo.cpp) — not a single %FILE% capture.
            run_user_mask_expect_captures(
                _T("%FILE%:%LINE%: warning:*")
              , _T("D:\\build\\foo.cpp:1001: warning: unused variable 'x'")
              , _T("D"), _T("build\\foo.cpp"), _T("1001"), _T("") );

            run_trusted_compiler_reject(
                _T("%ABSFILE%:%LINE%: error:*")
              , _T("*1:\\*2:*3 : **error**")
              , _T("C:/project/src/main.c: error: malformed line") );
        }

        // --- Microsoft Visual C++ / MSBuild ---
        {
            run_trusted_compiler_case(
                _T("%FILE%(%LINE%) : *error*")
              , _T("*1(*3) : **error**")
              , _T("main.cpp(128) : error C2065: 'foo' : undeclared identifier")
              , _T("main.cpp"), _T(""), _T("128"), _T("") );

            run_trusted_compiler_case(
                _T("%FILE%(%LINE%,%CHAR%) : *fatal error*")
              , _T("*1(*3,*4) : **fatal error**")
              , _T("..\\src\\app.cpp(90,17) : fatal error C1083: Cannot open include file")
              , _T("..\\src\\app.cpp"), _T(""), _T("90"), _T("17") );
        }

        // --- Pascal / Delphi ---
        {
            run_trusted_compiler_case(
                _T("%FILE%(%LINE%,%CHAR%): Error:*")
              , _T("*1(*3,*4): Error:**")
              , _T("Unit1.pas(45,12): Error: Identifier not found 'MyVar'")
              , _T("Unit1.pas"), _T(""), _T("45"), _T("12") );
        }

        // --- Java / javac ---
        {
            run_trusted_compiler_case(
                _T("%ABSFILE%:%LINE%: error:*")
              , _T("*1:\\*2:*3: **error**")
              , _T("C:/workspace/App.java:27: error: cannot find symbol")
              , _T("C"), _T("workspace/App.java"), _T("27"), _T("") );
        }

        // --- Rust (rustc / cargo) ---
        {
            run_trusted_compiler_case(
                _T("**--> %FILE%:%LINE%:%CHAR%")
              , _T("**--> *1:*3:*4")
              , _T("   --> src\\lib.rs:19:9")
              , _T("src\\lib.rs"), _T(""), _T("19"), _T("9") );

            run_user_mask_expect_captures(
                _T("**--> %FILE%:%LINE%:%CHAR%")
              , _T("   --> C:/crate/src/main.rs:3:14")
              , _T("C"), _T("crate/src/main.rs"), _T("3"), _T("14") );
        }

        // --- Go (gc) ---
        {
            run_trusted_compiler_case(
                _T("*1:*3:*4: *")
              , _T("*1:*3:*4: *")
              , _T("./cmd/app/main.go:88:2: undefined: fmt.Prinln")
              , _T("./cmd/app/main.go"), _T(""), _T("88"), _T("2") );
        }

        // --- Python tracebacks ---
        {
            run_trusted_compiler_case(
                _T("**File \"*1\", line *3,*")
              , _T("**File \"*1\", line *3,*")
              , _T("  File \"./scripts/run.py\", line 51, in <module>")
              , _T("./scripts/run.py"), _T(""), _T("51"), _T("") );

            run_user_mask_expect_captures(
                _T("**File \"%FILE%\", line %LINE%,*")
              , _T("  File \"C:\\venv\\tool.py\", line 9, in main")
              , _T("C:\\venv\\tool.py"), _T(""), _T("9"), _T("") );
        }

        // --- PHP ---
        {
            run_trusted_compiler_case(
                _T("Parse error in *1 on line *3")
              , _T("Parse error in *1 on line *3")
              , _T("Parse error in index.php on line 12")
              , _T("index.php"), _T(""), _T("12"), _T("") );
        }

        // --- Ruby ---
        {
            run_trusted_compiler_case(
                _T("*1:*3:in *")
              , _T("*1:*3:in *")
              , _T("lib/helper.rb:33:in validate_user")
              , _T("lib/helper.rb"), _T(""), _T("33"), _T("") );
        }

        // --- C# / .NET ---
        {
            run_trusted_compiler_case(
                _T("%FILE%(%LINE%,%CHAR%): error *")
              , _T("*1(*3,*4): error *")
              , _T("Program.cs(18,9): error CS1002: ; expected")
              , _T("Program.cs"), _T(""), _T("18"), _T("9") );
        }

        // --- Lua ---
        {
            run_trusted_compiler_case(
                _T("*1:*3: *")
              , _T("*1:*3: *")
              , _T("init.lua:4: attempt to call a nil value")
              , _T("init.lua"), _T(""), _T("4"), _T("") );
        }

        // --- Perl ---
        {
            run_trusted_compiler_case(
                _T("syntax error at *1 line *3")
              , _T("syntax error at *1 line *3")
              , _T("syntax error at parser.pl line 207")
              , _T("parser.pl"), _T(""), _T("207"), _T("") );
        }

        // --- Fortran (gfortran style) ---
        {
            run_trusted_compiler_case(
                _T("*1:*3:*4: Error:*")
              , _T("*1:*3:*4: Error:**")
              , _T("solver.f90:112:18: Error: Type mismatch")
              , _T("solver.f90"), _T(""), _T("112"), _T("18") );
        }

        // --- Verilog / SystemVerilog ---
        {
            run_trusted_compiler_case(
                _T("*1:*3: *error*")
              , _T("*1:*3: **error**")
              , _T("top.v:55: syntax error near 'endmodule'")
              , _T("top.v"), _T(""), _T("55"), _T("") );
        }

        // --- Nim ---
        {
            run_trusted_compiler_case(
                _T("%FILE%(%LINE%, %CHAR%) Error*")
              , _T("*1(*3, *4) Error**")
              , _T("src/module.nim(61, 7) Error: undeclared identifier")
              , _T("src/module.nim"), _T(""), _T("61"), _T("7") );
        }

        // --- Haskell (GHC) ---
        {
            run_trusted_compiler_case(
                _T("*1:*3:*4: error:*")
              , _T("*1:*3:*4: **error**")
              , _T("Main.hs:8:5: error: parse error on input `}'")
              , _T("Main.hs"), _T(""), _T("8"), _T("5") );
        }

        // --- Scala ---
        {
            run_trusted_compiler_case(
                _T("[error] *1:*3: *")
              , _T("[error] *1:*3: *")
              , _T("[error] src/main/scala/App.scala:25: value x is not a member")
              , _T("src/main/scala/App.scala"), _T(""), _T("25"), _T("") );
        }

        // --- AutoIt (quoted path, parenthesized line) ---
        {
            const TCHAR* line =
                _T("\"C:\\Color.au3\" (22) : ==> Unknown function name.:");

            run_trusted_compiler_case(
                _T("%FILE% (%LINE%) : *")
              , _T("*1 (*3) : **")
              , line
              , _T("\"C:\\Color.au3\""), _T(""), _T("22"), _T("") );

            run_case_expect_captures( _T("\"*1\" (*3) : **"), line
                                     , _T("C:\\Color.au3"), _T(""), _T("22"), _T("") );
            run_user_mask_case( _T("\"%FILE%\" (%LINE%) : *"), line
                               , _T("C:\\Color.au3"), _T("22") );
        }

        // --- Ten fictional / exotic compiler message styles ---
        {
            // ZephyrLang: angle-bracket path with symbolic line tag
            run_trusted_compiler_case(
                _T("<<< ZEPHYR @ *1 :: coil *3 >> *")
              , _T("<<< ZEPHYR @ *1 :: coil *3 >> *")
              , _T("<<< ZEPHYR @ disk[vol]/forge/parser.zep :: coil 42 >> fragment 'α' undeclared")
              , _T("disk[vol]/forge/parser.zep"), _T(""), _T("42"), _T("") );

            // CrystalForge: hex-ish row index and phase
            run_trusted_compiler_case(
                _T("[!!] module=*1 @0x*3 (phase *) *")
              , _T("[!!] module=*1 @0x*3 (phase *) *")
              , _T("[!!] module=src/core/init.cf @0x2A (phase 7) SYNTAX rupture near '~'")
              , _T("src/core/init.cf"), _T(""), _T("2A"), _T("") );

            // NovaScript: parenthesized pseudo-fraction line (no literal '*' near *1 — clashes with '**')
            run_trusted_compiler_case(
                _T("⟨NovaScript⟩ *1(π-line:*3/char:*4) — *")
              , _T("⟨NovaScript⟩ *1(π-line:*3/char:*4) — *")
              , _T("⟨NovaScript⟩ path/to/blob.nova(π-line:999/char:3) — hypertype mismatch")
              , _T("path/to/blob.nova"), _T(""), _T("999"), _T("3") );

            // QuarkCC: drive path with L/C markers
            run_trusted_compiler_case(
                _T("Quark> *1#L*3C*4 | *")
              , _T("Quark> *1#L*3C*4 | *")
              , _T("Quark> C:\\proj\\main.qk#L120C5 | graviton: expected flux tensor")
              , _T("C:\\proj\\main.qk"), _T(""), _T("120"), _T("5") );

            // MistWing: row label instead of "line"
            run_trusted_compiler_case(
                _T("{mist} file \"*1\" row *3 -- *")
              , _T("{mist} file \"*1\" row *3 -- *")
              , _T("{mist} file \"src\\wing.mw\" row 88 -- aura leak in ritual block")
              , _T("src\\wing.mw"), _T(""), _T("88"), _T("") );

            // HelixOS: hex address as line surrogate
            run_trusted_compiler_case(
                _T("helix:*1:0x*3: *")
              , _T("helix:*1:0x*3: *")
              , _T("helix:./kernel/sched.hl:0x1F3: panic crystal — deadline sprite lost")
              , _T("./kernel/sched.hl"), _T(""), _T("1F3"), _T("") );

            // RuneStack: pipe-separated Ln/Col labels
            run_trusted_compiler_case(
                _T("RS|*1|Ln *3 Col *4| *")
              , _T("RS|*1|Ln *3 Col *4| *")
              , _T("RS|.\\src\\app.rs|Ln 501 Col 12| Rune error E-void-7: sigil overflow")
              , _T(".\\src\\app.rs"), _T(""), _T("501"), _T("12") );

            // AetherPad: strand index (integer line; %LINE% / *3 are digit-led, not fractional)
            run_trusted_compiler_case(
                _T("◇ *1 ◇ strand *3 ◇ *")
              , _T("◇ *1 ◇ strand *3 ◇ *")
              , _T("◇ src\\pad.ae ◇ strand 12 ◇ harmonic drift (wanted 440Hz label)")
              , _T("src\\pad.ae"), _T(""), _T("12"), _T("") );

            // KelpVM: query-string line reference
            run_trusted_compiler_case(
                _T("kelp://%FILE%?at=line %LINE%; *")
              , _T("kelp://*1?at=line *3; *")
              , _T("kelp://workspace/blob.kp?at=line 77; tide ERROR: barnacle index")
              , _T("workspace/blob.kp"), _T(""), _T("77"), _T("") );

            // Orbityl: ORBIT / CHAR keywords
            run_trusted_compiler_case(
                _T("[orbityl] FILE=*1 ORBIT *3 CHAR *4 | *")
              , _T("[orbityl] FILE=*1 ORBIT *3 CHAR *4 | *")
              , _T("[orbityl] FILE=rel/pkg/main.ot ORBIT 303 CHAR 9 | singularity clip failed")
              , _T("rel/pkg/main.ot"), _T(""), _T("303"), _T("9") );
        }

        // --- Stability: near-miss lines must not match strict masks ---
        {
            run_trusted_compiler_reject(
                _T("%ABSFILE%:%LINE%: error:*")
              , _T("*1:\\*2:*3: **error**")
              , _T("C:/src/main.c: error: missing line number") );

            run_trusted_compiler_reject(
                _T("%FILE%(%LINE%) : *error*")
              , _T("*1(*3) : **error**")
              , _T("main.cpp : error C2065: 'foo'") );

            run_trusted_compiler_reject(
                _T("**File \"%FILE%\", line %LINE%,*")
              , _T("**File \"*1\", line *3,*")
              , _T("File \"run.py\", line 51") ); // missing leading spaces / quotes context
        }
    }

    void tests_WarningMaskMatcher_internal()
    {
        // Null input handling.
        {
            TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
            TCHAR r1[OUT_BUF_LEN], r2[OUT_BUF_LEN], r3[OUT_BUF_LEN], r4[OUT_BUF_LEN];

            init_out(o1); init_out(o2); init_out(o3); init_out(o4);
            init_out(r1); init_out(r2); init_out(r3); init_out(r4);

            const bool prod = match_mask_2( nullptr, _T("abc"), o1, o2, o3, o4 );
            const bool ref  = match_mask_2_recursive_ref( nullptr, _T("abc"), r1, r2, r3, r4 );
            assert( prod == ref );
            // On failure, outputs are not part of the observable behavior.
        }

        // Exact matching & special wildcards.
        run_case( _T("abc"), _T("abc") );
        run_case( _T("abc"), _T("abD") );

        run_case( _T("A?B"), _T("A1B") );
        run_case( _T("A?B"), _T("AB") ); // '?' must not match empty

        run_case( _T("A B"), _T("A\tB") ); // mask space matches any whitespace-ish char

        // Plain '*' wildcard in internal mask is '**' (no capture).
        run_case( _T("**X"), _T("xxX") );
        run_case( _T("**X"), _T("X") );   // '*' can match empty
        run_case( _T("**X"), _T("xY") );  // trailing literal must match

        // Captures when the mask ends after the placeholder.
        run_case( _T("*1"), _T("C:\\abc   ") ); // %F% capture consumes the rest
        run_case( _T("*3"), _T("  -12abc") );  // numeric validation passes
        run_case( _T("*3"), _T("abc") );       // numeric validation fails => no match

        // Numeric validation includes optional sign, then first digit only.
        // It does NOT require the rest of the captured string to be numeric.
        run_case( _T("*3"), _T(" +7XYZ") );
        run_case( _T("*4"), _T("\t-9*") );

        // Reject: sign without digits after it.
        run_case( _T("*3"), _T(" +abc") );
        run_case( _T("*4"), _T("- abc") );

        // Leading spaces/tabs are ignored before numeric check.
        run_case( _T("*3"), _T("   \t+1a") );

        // Captures when there's remainder after the placeholder.
        run_case( _T("**X*3Y"), _T("ABX12Y") );
        run_case( _T("**X*3Y"), _T("ABX  34Y") );
        run_case( _T("**X*3Y"), _T("ABXz12Y") ); // numeric validation must fail

        // ABSFILE special-case: drive:\\... must match drive:/...
        run_case( _T("*1:\\*2"), _T("C:/abc") );
        run_case( _T("*1:\\*2"), _T("C:\\abc") );

        // Composite realistic cases: FilePath/FileName + :Line and optional :Char.
        // Note: these tests target match_mask_2 internal-mask format directly.
        // Line/Char absent => mask should fail when the placeholder is present.

        // FilePath (relative) : Line
        run_case_expect_captures( _T("*1:*3"), _T("dir/filename:123")
                                 , _T("dir/filename"), _T(""), _T("123"), _T("") );
        run_case_expect_captures( _T("*1:*3"), _T("dir\\filename.ext:456")
                                 , _T("dir\\filename.ext"), _T(""), _T("456"), _T("") );
        run_case( _T("*1:*3"), _T("filename.ext:") );             // Line absent => fail

        // FilePath (relative) : Line : Char
        run_case_expect_captures( _T("*1:*3:*4"), _T("dir/filename:123:10")
                                 , _T("dir/filename"), _T(""), _T("123"), _T("10") );
        run_case_expect_captures( _T("*1:*3:*4"), _T("dir\\filename.ext:456:20")
                                 , _T("dir\\filename.ext"), _T(""), _T("456"), _T("20") );
        run_case( _T("*1:*3:*4"), _T("dir/filename::10") );     // Line absent => fail
        run_case( _T("*1:*3:*4"), _T("dir/filename:123:") );     // Char absent => fail

        // FileName (no directories) : Line
        run_case_expect_captures( _T("*1:*3"), _T("filename:1")
                                 , _T("filename"), _T(""), _T("1"), _T("") );
        run_case_expect_captures( _T("*1:*3"), _T("filename.ext:999")
                                 , _T("filename.ext"), _T(""), _T("999"), _T("") );
        run_case( _T("*1:*3"), _T("filename.ext:") );           // Line absent => fail

        // FileName : Line : Char
        run_case_expect_captures( _T("*1:*3:*4"), _T("filename:1:10")
                                 , _T("filename"), _T(""), _T("1"), _T("10") );
        run_case_expect_captures( _T("*1:*3:*4"), _T("filename.ext:999:20")
                                 , _T("filename.ext"), _T(""), _T("999"), _T("20") );
        run_case( _T("*1:*3:*4"), _T("filename.ext:999:") );  // Char absent => fail

        // FilePath (absolute, with both slash variants) : Line
        run_case_expect_captures( _T("*1:\\*2:*3"), _T("C:/dir1/dir2/filename.ext:123")
                                 , _T("C"), _T("dir1/dir2/filename.ext"), _T("123"), _T("") );
        run_case_expect_captures( _T("*1:\\*2:*3"), _T("C:\\dir1\\filename.ext:456")
                                 , _T("C"), _T("dir1\\filename.ext"), _T("456"), _T("") );
        run_case( _T("*1:\\*2:*3"), _T("C:/dir1/filename.ext:") ); // Line absent => fail

        // FilePath (absolute) : Line : Char
        run_case_expect_captures( _T("*1:\\*2:*3:*4"), _T("C:/dir1/filename.ext:123:10")
                                 , _T("C"), _T("dir1/filename.ext"), _T("123"), _T("10") );
        run_case_expect_captures( _T("*1:\\*2:*3:*4"), _T("C:\\dir1\\dir2\\filename.ext:456:20")
                                 , _T("C"), _T("dir1\\dir2\\filename.ext"), _T("456"), _T("20") );
        run_case( _T("*1:\\*2:*3:*4"), _T("C:/dir1/filename.ext::10") ); // Line absent => fail
        run_case( _T("*1:\\*2:*3:*4"), _T("C:/dir1/filename.ext:123:") ); // Char absent => fail

        // Error patterns: "Error in FileName at Line:Char"
        run_case_expect_captures( _T("Error in *1 at *3:*4"),
                                   _T("Error in filename.ext at 123:10"),
                                   _T("filename.ext"), _T(""), _T("123"), _T("10") );
        run_case_expect_captures( _T("Error in *1 at *3:*4"),
                                   _T("Error in dir/filename at 456:20"),
                                   _T("dir/filename"), _T(""), _T("456"), _T("20") ); // FileName placeholder accepts full token
        run_case( _T("Error in *1 at *3:*4"),
                  _T("Error in filename.ext at :10") );     // Line absent => fail
        run_case( _T("Error in *1 at *3:*4"),
                  _T("Error in filename.ext at 123:") );    // Char absent => fail

        // Error patterns: "Error in FilePath at Line:Char" (absolute path)
        run_case_expect_captures( _T("Error in *1:\\*2 at *3:*4")
                                 , _T("Error in C:/dir1/filename.ext at 123:10")
                                 , _T("C"), _T("dir1/filename.ext"), _T("123"), _T("10") );
        run_case_expect_captures( _T("Error in *1:\\*2 at *3:*4")
                                 , _T("Error in C:\\dir1\\dir2\\filename.ext at 456:20")
                                 , _T("C"), _T("dir1\\dir2\\filename.ext"), _T("456"), _T("20") );
        run_case( _T("Error in *1:\\*2 at *3:*4"),
                  _T("Error in C:/dir1/filename.ext at :10") );   // Line absent => fail
        run_case( _T("Error in *1:\\*2 at *3:*4"),
                  _T("Error in C:/dir1/filename.ext at 123:") );  // Char absent => fail

        // Network (UNC) paths: \\\\server\\dir\\filename.ext
        // These are exercised via *1/*3/*4 masks (not the drive-letter ABSFILE special form).
        run_case_expect_captures( _T("*1:*3")
                                 , _T("\\\\server\\dir\\filename.ext:123")
                                 , _T("\\\\server\\dir\\filename.ext"), _T(""), _T("123"), _T("") );
        run_case_expect_captures( _T("*1:*3")
                                 , _T("\\\\server/dir/filename.ext:456")
                                 , _T("\\\\server/dir/filename.ext"), _T(""), _T("456"), _T("") );

        run_case_expect_captures( _T("*1:*3:*4")
                                 , _T("\\\\server\\dir\\filename.ext:123:10")
                                 , _T("\\\\server\\dir\\filename.ext"), _T(""), _T("123"), _T("10") );
        run_case_expect_captures( _T("*1:*3:*4")
                                 , _T("\\\\server/dir/filename.ext:456:20")
                                 , _T("\\\\server/dir/filename.ext"), _T(""), _T("456"), _T("20") );

        // Network path error strings.
        run_case_expect_captures( _T("Error in *1 at *3:*4")
                                 , _T("Error in \\\\server\\dir\\filename.ext at 123:10")
                                 , _T("\\\\server\\dir\\filename.ext"), _T(""), _T("123"), _T("10") );

        run_case( _T("Error in *1 at *3:*4")
                 , _T("Error in \\\\server\\dir\\filename.ext at :10") ); // Line absent => fail

        // Real-world compiler / exotic message styles (iterative vs recursive + user masks).
        tests_compiler_message_styles();

        // Regression: bogus %ABSFILE% promotion on quoted paths must not steal the match.
        {
            const TCHAR* autoItLine =
                _T("\"C:\\Color.au3\" (22) : ==> Unknown function name.:");
            TCHAR oa1[OUT_BUF_LEN], oa2[OUT_BUF_LEN], oa3[OUT_BUF_LEN], oa4[OUT_BUF_LEN];
            init_out( oa1 ); init_out( oa2 ); init_out( oa3 ); init_out( oa4 );
            const bool bogusAbs = match_mask_2( _T("*1:\\*2 (*3) : **"), autoItLine, oa1, oa2, oa3, oa4 );
            assert( bogusAbs == true );
            assert( is_valid_absfile_drive_capture( oa1 ) == false );
        }

        // Truncation: WARN_MAX_FILENAME is hard-capped for captures.
        {
            std::basic_string<TCHAR> big;
            big.reserve( WARN_MAX_FILENAME + 20 );
            for ( int i = 0; i < WARN_MAX_FILENAME + 20; i++ )
                big += _T('a');

            run_case( _T("*1"), big.c_str() );

            TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];
            init_out(o1); init_out(o2); init_out(o3); init_out(o4);
            const bool prod = match_mask_2( _T("*1"), big.c_str(), o1, o2, o3, o4 );
            assert( prod == true );
            assert( o1[WARN_MAX_FILENAME - 1] == _T('a') );
            assert( o1[WARN_MAX_FILENAME] == 0 ); // terminator at the cap
        }

        // Many wildcards: ensure deeper backtracking still matches reference.
        {
            std::basic_string<TCHAR> mask;
            std::basic_string<TCHAR> str;

            // "**" token sequence creates pending '*' frames with no captures.
            for ( int i = 0; i < 18; i++ )
                mask += _T("**");
            mask += _T("A");

            for ( int i = 0; i < 30; i++ )
                str += _T('B');
            str += _T("A");

            run_case( mask.c_str(), str.c_str() );
        }

        // Lightweight fuzz: compare iterative vs recursive reference.
        {
            std::mt19937 rng( 123456u );

            for ( int i = 0; i < 250; i++ )
            {
                std::basic_string<TCHAR> mask;
                std::basic_string<TCHAR> str;

                build_random_internal_mask( rng, mask );
                build_random_string( rng, str );

                run_case_bool_only( mask.c_str(), str.c_str() );
            }
        }

        // Performance micro-benchmark (iterative vs recursive reference).
        // This is observational; correctness is already verified above.
        {
            struct TCase
            {
                tstring mask;
                tstring str;
            };

            // Keep this moderate so unit tests remain quick.
            constexpr int BENCH_CASES = 8000;

            auto build_bench_mask = []( std::mt19937& rng, tstring& outMask )
            {
                static const TCHAR tokens[][3] = {
                    { _T('*'), _T('*'), 0 }, // wildcard '**'
                    { _T('*'), _T('1'), 0 }, // capture 1
                    { _T('*'), _T('2'), 0 }, // capture 2
                    { _T('*'), _T('3'), 0 }, // capture 3 (numeric)
                    { _T('*'), _T('4'), 0 }, // capture 4 (numeric)
                };

                static const TCHAR literalChars[] = {
                    _T('A'), _T('B'), _T('C'), _T('X'), _T('Y'),
                    _T('0'), _T('1'), _T(' '), _T('\\'), _T('/'),
                    _T('?'), _T(':'), _T('-'), _T('+'),
                };

                std::uniform_int_distribution<int> tokPick( 0, 4 );
                std::uniform_int_distribution<int> litPick( 0, (int)(sizeof(literalChars)/sizeof(literalChars[0])) - 1 );
                std::uniform_int_distribution<int> lenPick( 2, 7 );

                const int tokenCount = lenPick( rng );
                int starTokenCount = 0;

                outMask.clear();
                for ( int i = 0; i < tokenCount; i++ )
                {
                    const bool makeStar = (starTokenCount < 2) && (rng() % 100 < 60);
                    if ( makeStar )
                    {
                        const int choice = tokPick( rng );
                        outMask += tokens[ choice ];
                        starTokenCount++;
                    }
                    else
                    {
                        outMask += literalChars[ litPick( rng ) ];
                    }
                }

                if ( outMask.empty() )
                    outMask = _T("**A");
            };

            auto build_bench_string = []( std::mt19937& rng, tstring& outStr )
            {
                static const TCHAR chars[] = {
                    _T('A'), _T('B'), _T('C'), _T('X'), _T('Y'),
                    _T('0'), _T('1'), _T('2'), _T('9'),
                    _T('-'), _T('+'),
                    _T(':'), _T('\\'), _T('/'),
                    _T(' '), _T('\t'),
                    _T('a'), _T('b'), _T('c'),
                };

                std::uniform_int_distribution<int> pick( 0, (int)(sizeof(chars)/sizeof(chars[0])) - 1 );
                std::uniform_int_distribution<int> lenPick( 0, 12 );

                const int len = lenPick( rng );
                outStr.clear();
                outStr.reserve( 20 );
                for ( int i = 0; i < len; i++ )
                    outStr += chars[ pick( rng ) ];
            };

            std::vector<TCase> cases;
            cases.reserve( BENCH_CASES );

            std::mt19937 rng( 654321u );
            for ( int i = 0; i < BENCH_CASES; i++ )
            {
                TCase c;
                build_bench_mask( rng, c.mask );
                build_bench_string( rng, c.str );
                cases.push_back( c );
            }

            auto hash_captured_prefix = []( const TCHAR* b ) -> uint64_t
            {
                // Hash only the first N characters to keep this benchmark cheap.
                uint64_t h = 1469598103934665603ULL; // FNV offset basis
                for ( int i = 0; i < 8 && b[i] != 0; i++ )
                {
                    h ^= (uint64_t)(uint32_t)b[i];
                    h *= 1099511628211ULL; // FNV prime
                }
                return h;
            };

            // Run optimized matcher.
            uint64_t sink_prod = 0;
            {
                TCHAR o1[OUT_BUF_LEN], o2[OUT_BUF_LEN], o3[OUT_BUF_LEN], o4[OUT_BUF_LEN];

                const auto t1 = std::chrono::high_resolution_clock::now();
                for ( const auto& c : cases )
                {
                    o1[0]=0; o2[0]=0; o3[0]=0; o4[0]=0;
                    const bool ok = match_mask_2( c.mask.c_str(), c.str.c_str(), o1, o2, o3, o4 );

                    uint64_t h = ok ? 1ULL : 0ULL;
                    if ( ok )
                    {
                        h ^= hash_captured_prefix(o1);
                        h ^= hash_captured_prefix(o2);
                        h ^= hash_captured_prefix(o3);
                        h ^= hash_captured_prefix(o4);
                    }
                    sink_prod ^= (h + 0x52dce729ULL);
                }
                const auto t2 = std::chrono::high_resolution_clock::now();

                const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
                std::printf( "match_mask_2 (iterative): %lld us for %d cases\n", (long long)us, BENCH_CASES );
            }

            // Run recursive reference matcher.
            uint64_t sink_ref = 0;
            {
                TCHAR r1[OUT_BUF_LEN], r2[OUT_BUF_LEN], r3[OUT_BUF_LEN], r4[OUT_BUF_LEN];

                const auto t1 = std::chrono::high_resolution_clock::now();
                for ( const auto& c : cases )
                {
                    r1[0]=0; r2[0]=0; r3[0]=0; r4[0]=0;
                    const bool ok = match_mask_2_recursive_ref( c.mask.c_str(), c.str.c_str(), r1, r2, r3, r4 );

                    uint64_t h = ok ? 1ULL : 0ULL;
                    if ( ok )
                    {
                        h ^= hash_captured_prefix(r1);
                        h ^= hash_captured_prefix(r2);
                        h ^= hash_captured_prefix(r3);
                        h ^= hash_captured_prefix(r4);
                    }
                    sink_ref ^= (h + 0x9e3779b9ULL);
                }
                const auto t2 = std::chrono::high_resolution_clock::now();

                const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
                std::printf( "match_mask_2 (recursive ref): %lld us for %d cases\n", (long long)us, BENCH_CASES );
            }

            // Ensure the benchmark results are used.
            assert( (sink_prod ^ sink_ref) != 0 );
        }
    }
}

void tests_WarningMaskMatcher()
{
    tests_WarningMaskMatcher_internal();
}

