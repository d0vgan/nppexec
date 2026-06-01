#ifndef _warning_mask_matcher_h_
#define _warning_mask_matcher_h_
//---------------------------------------------------------------------------

#include "base.h"

#define WARN_MAX_FILENAME         ( 2000 )

// Matches NppExec "warning filter" internal masks against input text.
// The mask format is produced by preprocessMask() in WarningAnalyzer.cpp.
bool match_mask_2( const TCHAR* mask
                  , const TCHAR* str
                  ,       TCHAR* postr1
                  ,       TCHAR* postr2
                  ,       TCHAR* postr3
                  ,       TCHAR* postr4 );

//---------------------------------------------------------------------------
#endif
