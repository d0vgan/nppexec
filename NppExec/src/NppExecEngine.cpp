/*
This file is part of NppExec
Copyright (C) 2013 DV <dvv81 (at) ukr (dot) net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "NppExecEngine.h"
//#include "NppExec.h"
#include "ChildProcess.h"
#include "DlgConsole.h"
#include "DlgConsoleEncoding.h"
#include "DlgInputBox.h"
#include "c_base/MatchMask.h"
#include "c_base/int2str.h"
#include "c_base/str2int.h"
#include "c_base/str_func.h"
#include "c_base/HexStr.h"
#include "cpp/StrSplitT.h"
#include "CSimpleLogger.h"
#include "DirFileLister.h"
#include "fparser/fparser.hh"
#include "npp_files/menuCmdID.h"
#include "npp_files/BoostRegexSearch.h"
#include <stdio.h>
#include <shellapi.h>
#include <limits>
#include <algorithm>

#ifdef _DEBUG
  #include <cassert>
#endif

#ifdef UNICODE
  #define _t_sprintf  swprintf
  #ifndef __MINGW32__
    #define _t_str2f(x) _wtof(x)
  #else
    #define _t_str2f(x) wcstof(x, 0)
  #endif
#else
  #define _t_sprintf  sprintf
  #define _t_str2f(x) atof(x)
#endif

#define MAX_VAR_LENGTH2SHOW 200

const TCHAR MACRO_FILE_FULLPATH[]       = _T("$(FULL_CURRENT_PATH)");
const TCHAR MACRO_FILE_DIRPATH[]        = _T("$(CURRENT_DIRECTORY)");
const TCHAR MACRO_FILE_FULLNAME[]       = _T("$(FILE_NAME)");
const TCHAR MACRO_FILE_NAMEONLY[]       = _T("$(NAME_PART)");
const TCHAR MACRO_FILE_EXTONLY[]        = _T("$(EXT_PART)");
const TCHAR MACRO_NPP_DIRECTORY[]       = _T("$(NPP_DIRECTORY)");
const TCHAR MACRO_NPP_FULL_FILE_PATH[]  = _T("$(NPP_FULL_FILE_PATH)");
const TCHAR MACRO_CURRENT_WORD[]        = _T("$(CURRENT_WORD)");
const TCHAR MACRO_SELECTED_TEXT[]       = _T("$(SELECTED_TEXT)");
const TCHAR MACRO_FILE_NAME_AT_CURSOR[] = _T("$(FILE_NAME_AT_CURSOR)");
const TCHAR MACRO_WORKSPACE_ITEM_DIR[]  = _T("$(WORKSPACE_ITEM_DIR)");
const TCHAR MACRO_WORKSPACE_ITEM_NAME[] = _T("$(WORKSPACE_ITEM_NAME)");
const TCHAR MACRO_WORKSPACE_ITEM_PATH[] = _T("$(WORKSPACE_ITEM_PATH)");
const TCHAR MACRO_WORKSPACE_ITEM_ROOT[] = _T("$(WORKSPACE_ITEM_ROOT)");
/* const TCHAR MACRO_WORKSPACE_FOLDER[]    = _T("$(WORKSPACE_FOLDER"); */
const TCHAR MACRO_CLOUD_LOCATION_PATH[] = _T("$(CLOUD_LOCATION_PATH)");
const TCHAR MACRO_CURRENT_LINE[]        = _T("$(CURRENT_LINE)");
const TCHAR MACRO_CURRENT_LINESTR[]     = _T("$(CURRENT_LINESTR)");
const TCHAR MACRO_CURRENT_COLUMN[]      = _T("$(CURRENT_COLUMN)");
const TCHAR MACRO_DOCNUMBER[]           = _T("$(#");
const TCHAR MACRO_SYSVAR[]              = _T("$(SYS.");
const TCHAR MACRO_LEFT_VIEW_FILE[]      = _T("$(LEFT_VIEW_FILE)");
const TCHAR MACRO_RIGHT_VIEW_FILE[]     = _T("$(RIGHT_VIEW_FILE)");
const TCHAR MACRO_PLUGINS_CONFIG_DIR[]  = _T("$(PLUGINS_CONFIG_DIR)");
const TCHAR MACRO_CURRENT_WORKING_DIR[] = _T("$(CWD)");
const TCHAR MACRO_ARGC[]                = _T("$(ARGC)");
const TCHAR MACRO_ARGV[]                = _T("$(ARGV");
const TCHAR MACRO_RARGV[]               = _T("$(RARGV");
const TCHAR MACRO_INPUT[]               = _T("$(INPUT)");
const TCHAR MACRO_INPUTFMT[]            = _T("$(INPUT[%d])");
const TCHAR MACRO_EXITCODE[]            = _T("$(EXITCODE)");
const TCHAR MACRO_PID[]                 = _T("$(PID)");
const TCHAR MACRO_IS_PROCESS[]          = _T("$(IS_PROCESS)");
const TCHAR MACRO_OUTPUT[]              = _T("$(OUTPUT)");
const TCHAR MACRO_OUTPUT1[]             = _T("$(OUTPUT1)");
const TCHAR MACRO_OUTPUTL[]             = _T("$(OUTPUTL)");
const TCHAR MACRO_MSG_RESULT[]          = _T("$(MSG_RESULT)");
const TCHAR MACRO_MSG_WPARAM[]          = _T("$(MSG_WPARAM)");
const TCHAR MACRO_MSG_LPARAM[]          = _T("$(MSG_LPARAM)");
const TCHAR MACRO_EXIT_CMD[]            = _T("$(@EXIT_CMD)");
const TCHAR MACRO_EXIT_CMD_SILENT[]     = _T("$(@EXIT_CMD_SILENT)");
const TCHAR MACRO_LAST_CMD_RESULT[]     = _T("$(LAST_CMD_RESULT)");
const TCHAR MACRO_CLIPBOARD_TEXT[]      = _T("$(CLIPBOARD_TEXT)");
const TCHAR MACRO_NPP_HWND[]            = _T("$(NPP_HWND)");
const TCHAR MACRO_NPP_PID[]             = _T("$(NPP_PID)");
const TCHAR MACRO_SCI_HWND[]            = _T("$(SCI_HWND)");
const TCHAR MACRO_SCI_HWND1[]           = _T("$(SCI_HWND1)");
const TCHAR MACRO_SCI_HWND2[]           = _T("$(SCI_HWND2)");
const TCHAR MACRO_CON_HWND[]            = _T("$(CON_HWND)");
const TCHAR MACRO_FOCUSED_HWND[]        = _T("$(FOCUSED_HWND)");

const TCHAR DIRECTIVE_PREFIX_CHAR       = _T('!');
const TCHAR DIRECTIVE_COLLATERAL[]      = _T("!COLLATERAL");

// NppExec's Search Flags for sci_find and sci_replace:
#define NPE_SF_MATCHCASE    0x00000001 // "text" finds only "text", not "Text" or "TEXT"
#define NPE_SF_WHOLEWORD    0x00000010 // "word" finds only "word", not "sword" or "words" 
#define NPE_SF_WORDSTART    0x00000020 // "word" finds "word" and "words", not "sword"
#define NPE_SF_REGEXP       0x00000100 // search using a regular expression
#define NPE_SF_POSIX        0x00000200 // search using a POSIX-compatible regular expression
#define NPE_SF_CXX11REGEX   0x00000400 // search using a C++11 regular expression
#define NPE_SF_REGEXP_EMPTYMATCH_NOTAFTERMATCH 0x00002000
#define NPE_SF_REGEXP_EMPTYMATCH_ALL           0x00004000
#define NPE_SF_REGEXP_EMPTYMATCH_ALLOWATSTART  0x00008000
#define NPE_SF_BACKWARD     0x00010000 // search backward (from the bottom to the top)
#define NPE_SF_NEXT         0x00020000 // search from current_position + 1
#define NPE_SF_INSELECTION  0x00100000 // search only in the selected text
#define NPE_SF_INENTIRETEXT 0x00200000 // search in the entire text, not only from the current position
#define NPE_SF_SETPOS       0x01000000 // move the caret to the position of the occurrence found
#define NPE_SF_SETSEL       0x02000000 // move the caret + select the occurrence found
#define NPE_SF_REPLACEALL   0x10000000 // replace all the occurrences from the current pos to the end
#define NPE_SF_PRINTALL     0x20000000 // print all the occurrences from the current pos to the end

enum eParamType {
    PT_UNKNOWN = 0,
    PT_INT,
    PT_PINT,
    PT_STR,
    PT_PSTR,
    PT_HEXSTR,
    PT_PHEXSTR,

    PT_COUNT
};

const TCHAR* STR_PARAMTYPE[PT_COUNT] =
{
    _T(""),
    _T("int"),
    _T("@int"),
    _T("str"),
    _T("@str"),
    _T("hex"),
    _T("@hex")
};

extern FuncItem              g_funcItem[nbFunc + MAX_USERMENU_ITEMS + 1];

extern COLORREF              g_colorTextNorm;

BOOL     g_bIsNppUnicode = FALSE;
WNDPROC  nppOriginalWndProc;

CMacroVars g_GlobalEnvVars;
CMacroVars g_LocalEnvVarNames;
CCriticalSection g_csEnvVars;

static int  FileFilterPos(const TCHAR* szFilePath);
static void GetPathAndFilter(const TCHAR* szPathAndFilter, int nFilterPos, tstr& out_Path, tstr& out_Filter);
static void GetFilePathNamesList(const TCHAR* szPath, const TCHAR* szFilter, CListT<tstr>& FilesList);
static bool PrintDirContent(CNppExec* pNppExec, const TCHAR* szPath, const TCHAR* szFilter);
static void runInputBox(CScriptEngine* pScriptEngine, const tstr& msg);
static CScriptEngine::eCmdResult runMessageBox(CScriptEngine* pScriptEngine, const tstr& params);


class PrintMacroVarFunc
{
    public:
        PrintMacroVarFunc(CNppExec* pNppExec) : m_pNppExec(pNppExec)
        {
        }

        bool operator()(const CMacroVars::item_type& v, bool isLocalVar)
        {
            tstr S = isLocalVar ? _T("local ") : _T("");
            S += v.name;
            S += _T(" = ");
            if ( v.value.length() > MAX_VAR_LENGTH2SHOW )
            {
                S.Append( v.value.c_str(), MAX_VAR_LENGTH2SHOW - 5 );
                S += _T("(...)");
            }
            else
            {
                S += v.value;
            }
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );

            return false; // don't stop
        }

    protected:
        CNppExec* m_pNppExec;
};

class SubstituteMacroVarFunc
{
    public:
        SubstituteMacroVarFunc(tstr& Value, int& pos) : m_Value(Value), m_Pos(pos)
        {
            m_ValueUpper = Value;
            NppExecHelpers::StrUpper(m_ValueUpper);
        }

        SubstituteMacroVarFunc& operator=(const SubstituteMacroVarFunc&) = delete;

        bool operator()(const CMacroVars::item_type& v, bool /*isLocalVar*/)
        {
            if ( StrUnsafeSubCmp(m_ValueUpper.c_str() + m_Pos, v.name.c_str()) == 0 )
            {
                m_Value.Replace( m_Pos, v.name.length(), v.value.c_str(), v.value.length() );
                m_Pos += v.value.length();
                return true; // substituted; stop!
            }

            return false; // don't stop
        }

    protected:
        int& m_Pos;
        tstr& m_Value;
        tstr m_ValueUpper;
};

template<class MacroVarFunc> bool IterateUserMacroVars(
    const CMacroVars& userMacroVars,
    const CMacroVars& userLocalMacroVars,
    MacroVarFunc func)
{
    // checking local vars first
    for ( const auto& v : userLocalMacroVars )
    {
        if ( func(v, true) )
            return true;
    }

    // then checking global vars not overridden by local ones
    auto localVarsEnd = userLocalMacroVars.cend();
    for ( const auto& v : userMacroVars )
    {
        if ( userLocalMacroVars.find(v.name) == localVarsEnd )
        {
            if ( func(v, false) )
                return true;
        }
    }

    return false;
}


#define abs_val(x) (((x) < 0) ? (-(x)) : (x))


static bool IsAnySpaceOrEmptyChar(const TCHAR ch)
{
  return ( (ch == 0) || NppExecHelpers::IsAnySpaceChar(ch) );
}

/*
static bool isHexNumChar(const TCHAR ch)
{
    return ( ( (ch >= _T('0') && ch <= _T('9')) ||
               (ch >= _T('A') && ch <= _T('F')) ||
               (ch >= _T('a') && ch <= _T('f')) ) ? true : false );
}*/
/**/
static bool isDecNumChar(const TCHAR ch)
{
    return ( (ch >= _T('0') && ch <= _T('9')) ? true : false );
}

/**/
#define  SEP_TABSPACE  0

enum eQuoteType {
    QT_DOUBLE  = 0x01, // "
    QT_SINGLE1 = 0x02, // '
    QT_SINGLE2 = 0x04  // `
};

static int getQuoteType(const TCHAR ch)
{
    if ( ch == _T('\"') )  return QT_DOUBLE;
    if ( ch == _T('\'') )  return QT_SINGLE1;
    if ( ch == _T('`') )   return QT_SINGLE2;
    return 0;
}

// gets the current param and returns a pointer to the next param
// checks for a bracket only when pBracket != nullptr
const TCHAR* get_param(const TCHAR* s, tstr& param, const TCHAR sep = SEP_TABSPACE, 
                       unsigned int* pnQuotes = nullptr, bool* pBracket = nullptr)
{
    param.Clear();

    if ( pnQuotes )  *pnQuotes = 0;
    if ( pBracket )  *pBracket = false;

    while ( NppExecHelpers::IsAnySpaceChar(*s) )  ++s;  // skip leading tabs/spaces

    int i = 0;
    int n = 0;
    unsigned int nBracketDepth = 0;
    bool isComplete = false;
    bool isDblQuote = false;
    bool isSglQuote1 = false; // '
    bool isSglQuote2 = false; // `
    bool isEnvVar = false;
    bool isBracket = false;

    if ( pBracket )
    {
        if ( *s == _T('@') )
        {
            param += _T('@');
            ++s;
        }
        if ( *s == _T('[') )
        {
            isBracket = true;
            *pBracket = true;
            param += _T('[');
            ++s;
        }
    }

    while ( !isComplete )
    {
        const TCHAR ch = *s;
        
        switch ( ch )
        {
            case _T('\"'):
                if ( !isEnvVar && !isSglQuote1 && !isSglQuote2 )
                {
                    isDblQuote = !isDblQuote;
                    if ( isDblQuote && pnQuotes )
                    {
                        *pnQuotes |= QT_DOUBLE;
                    }
                    if ( isBracket )
                        param += ch;
                }
                else
                {
                    param += ch;
                }
                ++s; // to next character
                break;

            case _T('\''):
                if ( !isEnvVar && !isDblQuote && !isSglQuote2 )
                {
                    isSglQuote1 = !isSglQuote1;
                    if ( isSglQuote1 && pnQuotes )
                    {
                        *pnQuotes |= QT_SINGLE1;
                    }
                    if ( isBracket )
                        param += ch;
                }
                else
                {
                    param += ch;
                }
                ++s; // to next character
                break;

            case _T('`'):
                if ( !isEnvVar && !isDblQuote && !isSglQuote1 )
                {
                    isSglQuote2 = !isSglQuote2;
                    if ( isSglQuote2 && pnQuotes )
                    {
                        *pnQuotes |= QT_SINGLE2;
                    }
                    if ( isBracket )
                        param += ch;
                }
                else
                {
                    param += ch;
                }
                ++s; // to next character
                break;

            case _T(' '):
            case _T('\t'):
            case _T('\v'):
            case _T('\f'):
                if ( sep == SEP_TABSPACE && !isDblQuote && !isSglQuote1 && !isSglQuote2 && !isBracket && !isEnvVar )
                {
                    isComplete = true;
                }
                else
                {
                    param += ch;
                    if ( isDblQuote || isSglQuote1 || isSglQuote2 || isBracket )
                        n = param.length();
                }
                ++s; // to next character
                break;

            case _T('$'):
                if ( (!isEnvVar) && (*(s + 1) == _T('(')) )
                {
                    isEnvVar = true;
                    param += _T('$');
                    param += _T('(');
                    ++s; // skip '$'
                    ++s; // skip '('
                }
                else
                {
                    param += ch;
                    ++s; // to next character
                }
                break;

            case _T(')'):
                if ( isEnvVar )
                {
                    if ( nBracketDepth == 0 )
                        isEnvVar = false;
                    else
                        --nBracketDepth;
                }
                param += ch;
                ++s; // to next character
                break;

            case _T('('):
                if ( isEnvVar )
                {
                    ++nBracketDepth;
                }
                param += ch;
                ++s; // to next character
                break;

            case _T(']'):
                if ( isBracket && !isEnvVar && !isDblQuote && !isSglQuote1 && !isSglQuote2 )
                {
                    isBracket = false;
                }
                param += ch;
                ++s; // to next character
                break;

            case 0:
                isComplete = true;
                break;

            default:
                if ( ch == sep && !isDblQuote && !isSglQuote1 && !isSglQuote2 && !isEnvVar )
                {
                    isComplete = true;
                }
                else
                {
                    param += ch;
                }
                ++s; // to next character
                break;
        }
    }
    
    for ( i = param.length() - 1; i >= n; i-- )
    {
        if ( !NppExecHelpers::IsAnySpaceChar(param[i]) )
            break;
    }
    n = param.length() - 1;
    if ( i < n )
    {
        param.Delete(i + 1, n - i); // remove trailing tabs/spaces
    }

    while ( NppExecHelpers::IsAnySpaceChar(*s) )  ++s;  // skip trailing tabs/spaces

    return s;
}

static tstr substituteMacroVarsIfNotDelayed(CScriptEngine* pScriptEngine, const tstr& params, bool bUseDelayedSubstitution)
{
    tstr key;
    const TCHAR* p = get_param( params.c_str(), key, SEP_TABSPACE );
    if ( (*p != 0) && (*p != _T('=')) && !key.IsEmpty() )
    {
        NppExecHelpers::StrUpper(key);
        if ( key == _T("+V") )
            bUseDelayedSubstitution = true;
        else if ( key == _T("-V") )
            bUseDelayedSubstitution = false;
        else
            p = NULL;
    }
    else
        p = NULL;

    tstr paramsToReturn = (p == NULL) ? params : tstr(p);
    if ( !bUseDelayedSubstitution )
    {
        CNppExec* pNppExec = pScriptEngine->GetNppExec();
        pNppExec->GetMacroVars().CheckAllMacroVars(pScriptEngine, paramsToReturn, true);
    }

    return paramsToReturn;
}

static tstr getQueuedCommand(const tstr& params, bool& bUseSeparateScript)
{
    tstr key;
    const TCHAR* p1 = params.c_str(); // beginning of the key
    const TCHAR* p2 = get_param( p1, key, SEP_TABSPACE ); // end of the key
    bool bContinue = true;
    while ( bContinue )
    {
        bContinue = false;
        if ( (*p2 != 0) && !key.IsEmpty() )
        {
            NppExecHelpers::StrUpper(key);
            if ( key == _T("+V") || key == _T("-V") )
            {
                p1 = p2; // beginning of the next key
                p2 = get_param( p1, key, SEP_TABSPACE ); // end of the next key
                bContinue = true;
            }
            else if ( key == _T("+S") )
                bUseSeparateScript = true;
            else if ( key == _T("-S") )
                bUseSeparateScript = false;
            else
                p2 = NULL;
        }
        else
            p2 = NULL;
    }

    if ( p2 == NULL ) // "+S" or "-S" was not found
        return params;
    
    tstr Cmd;
    Cmd.Append(params.c_str(), static_cast<int>(p1 - params.c_str()));
    Cmd.Append(p2, params.length() - static_cast<int>(p2 - params.c_str()));
    return Cmd;
}

/**/
class FParserWrapper
{
    public:
        typedef FunctionParser fparser_type;

    public:
        typedef struct sUserConst {
            CStrT<char> constName;
            CStrT<char> constValue;
            int         nLine{};
        } tUserConst;

    public:
        FParserWrapper()
          : m_fp(nullptr)
          , m_calc_precision(0.000001)
        {
            lstrcpy( m_szCalcDefaultFmt, _T("%.6f") );
            lstrcpy( m_szCalcSmallFmt,   _T("%.6G") );
            lstrcpy( m_szCalcBigFmt,     _T("%.7G") );
        }

        ~FParserWrapper()
        {
            if ( m_fp != nullptr )
                delete m_fp;
        }

        bool Calculate(CNppExec* pNppExec, const tstr& func, tstr& calcError, tstr& calcResult)
        {
            const CStr funcA = NppExecHelpers::TStrToCStr(func);
            return calc2(pNppExec, funcA, calcError, calcResult);
        }

        fparser_type::value_type CalcValue(CNppExec* pNppExec, const tstr& func, tstr& calcError)
        {
            const CStr funcA = NppExecHelpers::TStrToCStr(func);
            return calcValue(pNppExec, funcA, calcError);
        }

    private:
        enum eParsingState {
            stNormal = 0,
            stEnumDeclaration,  // the "enum" keyword found
            stEnumBody          // inside "{ ... }" part of the enum
        };

        struct sParseContext {
            CNppExec*          pNppExec;
            CFileBufT<char>    fbuf;
            CStrT<char>        line;
            CListT<tUserConst> unparsedUserConstsList;
            int                nItemsOK;
            int                nNextEnumValue;
            eParsingState      state;
        };

        void parse_Define(sParseContext& context)
        {
            NppExecHelpers::StrDelLeadingAnySpaces(context.line);
            if ( context.line.IsEmpty() )
                return;

            tUserConst userConst;
            userConst.nLine = context.fbuf.GetLineNumber() - 1;

            int i = 0;
            while ( i < context.line.length() && !NppExecHelpers::IsAnySpaceChar(context.line[i]) )
            {
                userConst.constName += context.line[i];
                ++i;
            }

            context.line.Delete(0, i); // delete const name
            NppExecHelpers::StrDelLeadingAnySpaces(context.line);
            NppExecHelpers::StrDelTrailingAnySpaces(context.line);
            if ( context.line.IsEmpty() )
                return;

            tstr calcError;
            fparser_type::value_type val = calcValue(context.pNppExec, context.line, calcError);
            if ( !calcError.IsEmpty() )
            {
                userConst.constValue = context.line;
                context.unparsedUserConstsList.Add(userConst);
                return;
            }

            m_fp->AddConstant(userConst.constName.c_str(), val);
            ++context.nItemsOK;
        }

        void parse_EnumDeclaration(sParseContext& context)
        {
            NppExecHelpers::StrDelTrailingAnySpaces(context.line);
            if ( context.line.EndsWith('\\') )
                context.line.DeleteLastChar();

            int i = context.line.Find('{');
            if ( i >= 0 )
            {
                context.line.Delete(0, i + 1);
                NppExecHelpers::StrDelLeadingAnySpaces(context.line);

                context.state = stEnumBody;
                context.nNextEnumValue = 0;
                parse_EnumBody(context);
            }
        }

        void parse_EnumBody(sParseContext& context)
        {
            NppExecHelpers::StrDelTrailingAnySpaces(context.line);
            if ( context.line.EndsWith('\\') )
                context.line.DeleteLastChar();

            int i = context.line.Find('}');
            if ( i >= 0 )
            {
                context.line.Delete(i, -1);

                context.state = stNormal;
            }

            NppExecHelpers::StrDelTrailingAnySpaces(context.line);
            NppExecHelpers::StrDelLeadingAnySpaces(context.line);
            if ( context.line.IsEmpty() )
                return;

            CStr varName;
            CStr varValue;
            CStrSplitT<char> args;
            const int n = args.Split(context.line, ",");
            for ( i = 0; i < n; i++ )
            {
                const CStr& S = args.Arg(i);
                if ( S.IsEmpty() )
                    continue;

                int j = S.Find('=');
                if ( j >= 0 )
                {
                    varName.Assign(S.c_str(), j);
                    NppExecHelpers::StrDelTrailingAnySpaces(varName);
                    NppExecHelpers::StrDelLeadingAnySpaces(varName);
                    if ( varName.IsEmpty() )
                        continue;

                    varValue.Assign(S.c_str() + j + 1, S.length() - j - 1);
                    NppExecHelpers::StrDelTrailingAnySpaces(varValue);
                    NppExecHelpers::StrDelLeadingAnySpaces(varValue);
                    if ( !varValue.IsEmpty() )
                    {
                        tstr calcError;
                        fparser_type::value_type val = calcValue(context.pNppExec, varValue, calcError);
                        if ( calcError.IsEmpty() )
                        {
                            m_fp->AddConstant(varName.c_str(), val);
                            context.nNextEnumValue = static_cast<int>(val);
                            ++context.nNextEnumValue;
                            ++context.nItemsOK;
                        }
                        else
                        {
                            tstr Name = NppExecHelpers::CStrToTStr(varName);
                            tstr Value = NppExecHelpers::CStrToTStr(varValue);

                            Runtime::GetLogger().AddEx( _T("error at line %d: \"%s = %s\""), context.fbuf.GetLineNumber() - 1, Name.c_str(), Value.c_str() );
                            Runtime::GetLogger().AddEx( _T(" - %s"), calcError.c_str() );

                        }
                    }
                    else
                    {
                        m_fp->AddConstant(varName.c_str(), fparser_type::value_type(context.nNextEnumValue));
                        ++context.nNextEnumValue;
                        ++context.nItemsOK;
                    }
                }
                else
                {
                    varName = S;
                    NppExecHelpers::StrDelTrailingAnySpaces(varName);
                    NppExecHelpers::StrDelLeadingAnySpaces(varName);
                    if ( !varName.IsEmpty() )
                    {
                        m_fp->AddConstant(varName.c_str(), fparser_type::value_type(context.nNextEnumValue));
                        ++context.nNextEnumValue;
                        ++context.nItemsOK;
                    }
                }
            }
        }

        void process_unparsedUserConsts(sParseContext& context)
        {
            if ( !context.unparsedUserConstsList.IsEmpty() )
            {
                CListItemT<tUserConst>* pItem = context.unparsedUserConstsList.GetFirst();
                while ( pItem )
                {
                    tUserConst& uc = pItem->GetItem();

                    tstr calcError;
                    fparser_type::value_type val = calcValue(context.pNppExec, uc.constValue, calcError);
                    if ( calcError.IsEmpty() )
                    {
                        m_fp->AddConstant(uc.constName.c_str(), val);
                        ++context.nItemsOK;
                    }
                    else
                    {
                        tstr Value = NppExecHelpers::CStrToTStr(uc.constValue);

                        Runtime::GetLogger().AddEx( _T("error at line %d: \"%s\""), uc.nLine, Value.c_str() );
                        Runtime::GetLogger().AddEx( _T(" - %s"), calcError.c_str() );

                    }

                    pItem = pItem->GetNext();
                }
            }
        }

        void readConstsFromFile(CNppExec* pNppExec, const tstr& path)
        {
            sParseContext context;
            context.nItemsOK = 0;
            context.nNextEnumValue = 0;
            context.state = stNormal;

            if ( context.fbuf.LoadFromFile(path.c_str(), true, pNppExec->GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH)) )
            {
                Runtime::GetLogger().Add(   path.c_str() );
                Runtime::GetLogger().Add(   _T("(") );
                Runtime::GetLogger().IncIndentLevel();

                while ( context.fbuf.GetLine(context.line) >= 0 )
                {
                    if ( context.line.IsEmpty() )
                        continue;

                    int i = context.line.Find("//");
                    if ( i >= 0 )
                        context.line.Delete(i, -1);

                    NppExecHelpers::StrDelLeadingAnySpaces(context.line);
                    if ( context.line.IsEmpty() )
                        continue;

                    switch ( context.state )
                    {
                        case stNormal:
                            if ( context.line.StartsWith('#') )
                            {
                                int offset = 1; // the leading '#'
                                while ( NppExecHelpers::IsAnySpaceChar(context.line[offset]) )
                                {
                                    ++offset; // skipping ' ' and '\t'
                                }
                                
                                const char* s = context.line.c_str() + offset;
                                if ( memcmp(s, "define", 6*sizeof(char)) == 0 )
                                {
                                    const char ch = context.line.GetAt(offset + 6);
                                    if ( NppExecHelpers::IsAnySpaceChar(ch) )
                                    {
                                        context.line.Delete(0, offset + 6); // delete "#define" or "# define"
                                        parse_Define(context);
                                    }
                                }
                            }
                            else if ( context.line.StartsWith("enum") )
                            {
                                const char ch = context.line.GetAt(4);
                                if ( IsAnySpaceOrEmptyChar(ch) )
                                {
                                    context.line.Delete(0, 4); // delete "enum"
                                    context.state = stEnumDeclaration;
                                    parse_EnumDeclaration(context);
                                }
                            }
                            break;

                        case stEnumDeclaration:
                            parse_EnumDeclaration(context);
                            break;

                        case stEnumBody:
                            parse_EnumBody(context);
                            break;
                    }
                }

                process_unparsedUserConsts(context);

                Runtime::GetLogger().AddEx( _T("%d definitions added."), context.nItemsOK );
                Runtime::GetLogger().DecIndentLevel();
                Runtime::GetLogger().Add(   _T(")") );

            }
        }

        void initFParserConsts(CNppExec* pNppExec)
        {

            Runtime::GetLogger().Add(   _T("initFParserConsts()") );
            Runtime::GetLogger().Add(   _T("{") );
            Runtime::GetLogger().IncIndentLevel();

            m_fp->AddConstant("WM_COMMAND", fparser_type::value_type(WM_COMMAND));
            m_fp->AddConstant("WM_USER", fparser_type::value_type(WM_USER));
            m_fp->AddConstant("NPPMSG", fparser_type::value_type(NPPMSG));

            CDirFileLister FileLst;
            tstr           path;

            path = pNppExec->getPluginDllPath();
            path += _T("\\NppExec\\*.h");
            if ( FileLst.FindNext(path.c_str(), CDirFileLister::ESF_FILES | CDirFileLister::ESF_SORTED) )
            {
                do {
                    path = pNppExec->getPluginDllPath();
                    path += _T("\\NppExec\\");
                    path += FileLst.GetItem();

                    readConstsFromFile(pNppExec, path);
                } 
                while ( FileLst.GetNext() );
            }
            else
            {
                path = pNppExec->getPluginDllPath();
                path += _T("\\NppExec");

                Runtime::GetLogger().AddEx( _T("; no *.h files found in \"%s\""), path.c_str() );

                const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                pNppExec->GetConsole().PrintMessage( _T("- Warning: fparser's constants have not been initialized because"), nMsgFlags );
                pNppExec->GetConsole().PrintMessage( _T("the following folder either does not exist or is empty:"), nMsgFlags );
                pNppExec->GetConsole().PrintMessage( path.c_str(), nMsgFlags );
            }

            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );

        }

        static fparser_type::value_type fp_hex(const fparser_type::value_type* p)
        {
            return p[0];
        }

        void initFParserFuncs()
        {
            m_fp->AddFunction("hex", fp_hex, 1);
        }

        void initCalc(CNppExec* pNppExec)
        {
            m_fp = new fparser_type();
            int len = 0;
            const TCHAR* pPrecision = pNppExec->GetOptions().GetStr(OPTS_CALC_PRECISION, &len);
            if ( len > 0 )
            {
                double precision = _t_str2f(pPrecision);
                precision = abs_val(precision);
                if ( precision > 0.0 )
                {
                    int d = 0;

                    m_calc_precision = precision;

                    while ( precision < 0.99 )
                    {
                        precision *= 10;
                        ++d;
                    }
                    wsprintf(m_szCalcDefaultFmt, _T("%%.%df"), d);
                    wsprintf(m_szCalcSmallFmt,   _T("%%.%dG"), d);
                    wsprintf(m_szCalcBigFmt,     _T("%%.%dG"), d + 1);
                }
            }

            initFParserFuncs();
            initFParserConsts(pNppExec);
        }

        fparser_type::value_type calc(CNppExec* pNppExec, const CStr& func, tstr& calcError)
        {
            if ( func.IsEmpty() )
            {
                calcError = _T("Input function is empty");
                return fparser_type::value_type(0);
            }

            {
                CCriticalSectionLockGuard lock(m_cs);

                if ( !m_fp )
                {
                    initCalc(pNppExec);
                }
            }

            int errPos;
            fparser_type::value_type ret(0);

            calcError.Clear();

            {
                CCriticalSectionLockGuard lock(m_cs);

                errPos = m_fp->Parse(func.c_str(), "");
                if ( errPos == -1 )
                {
                    fparser_type::value_type val(0);

                    val = m_fp->Eval( &val );
                    int err = m_fp->EvalError();
                    if ( err == 0 )
                        ret = val;
                    else
                        calcError.Format(50, _T("Eval error (%d)"), err);
                }
                else
                {
                    calcError = NppExecHelpers::CStrToTStr( m_fp->ErrorMsg() );
                }
            }

            if ( errPos != -1 )
            {
                TCHAR szNum[50];

                calcError += _T(" at pos ");
                c_base::_tint2str(errPos, szNum);
                calcError += szNum;
            }

            return ret;
        }

        template<typename T> void format_result(const std::complex<T>& val, tstr& result, const CStr& func)
        {
            if ( val.imag() == 0 )
            {
                format_result(val.real(), result, func);
            }
            else
            {
                TCHAR szNum[120];
                // ha-ha, wsprintf does not support neither "%f" nor "%G"
                _t_sprintf(szNum, _T("%g %c %gi"), val.real(), val.imag() < 0 ? _T('-') : _T('+'), std::abs(val.imag()));
                result = szNum;
            }
        }

        template<typename T> void format_result(const T& val, tstr& result, const CStr& func)
        {
            // Note: 
            // As we are using FunctionParser that operates with 'double',
            // its accuracy with regards to 64-bit integers is limited by
            // the mantissa of a 'double'.
          #ifndef __MINGW32__
            const __int64 max_accurate_i64_value = (1i64) << std::numeric_limits<T>::digits; // 2**53 for the 'double' type
          #else
            const long long max_accurate_i64_value = (1LL) << std::numeric_limits<T>::digits; // 2**53 for the 'double' type
          #endif

            TCHAR szNum[80];
            szNum[0] = 0;

            if ( (func.StartsWith("hex(") || func.StartsWith("hex (")) &&
                 (abs_val(val) < max_accurate_i64_value) )
            {
                unsigned __int64 n = static_cast<unsigned __int64>(val);
                _t_sprintf( szNum, _T("0x%I64X"), n );
            }
            else if ( abs_val(val) > m_calc_precision*100 )
            {
                if ( abs_val(val) < max_accurate_i64_value )
                {
                    __int64 n = static_cast<__int64>(val);
                    double  diff = static_cast<double>(val - n);

                    if ( abs_val(diff) < m_calc_precision )
                    {
                        // result can be rounded
                        _t_sprintf( szNum, _T("%I64d"), n ); 
                        // ha-ha, wsprintf does not support neither "%I64d" nor "%f"
                    }
                    else
                    {
                        int nn = _t_sprintf( szNum, m_szCalcDefaultFmt, val );
                        if ( nn > 0 )
                        {
                            while ( nn > 0 && szNum[--nn] == _T('0') ) ;
                            if ( szNum[nn] == _T('.') )  ++nn; // keep one '0' after '.'
                            szNum[nn + 1] = 0; // exclude all trailing '0' after '.'
                        }
                    }
                }
                else
                {
                    // result is too big
                    _t_sprintf( szNum, m_szCalcBigFmt, val );
                }
            }
            else
            {
                // result is too small
                _t_sprintf( szNum, m_szCalcSmallFmt, val );
                // ha-ha, wsprintf does not support neither "%f" nor "%G"
            }

            result = szNum;
        }

        bool calc2(CNppExec* pNppExec, const CStr& func, tstr& calcError, tstr& calcResult)
        {
            fparser_type::value_type fret = calc(pNppExec, func, calcError);

            if ( calcError.IsEmpty() )
            {
                format_result(fret, calcResult, func);
            }

            return calcError.IsEmpty();
        }

        fparser_type::value_type calcValue(CNppExec* pNppExec, const CStr& func, tstr& calcError)
        {
            // Note:
            //  When this method is called from readConstsFromFile(),
            //  NppExec's logger is not used within calc().

            fparser_type::value_type val = calc(pNppExec, func, calcError);

            if ( calcError.IsEmpty() )
            {
              #ifndef __MINGW32__
                const __int64 max_accurate_i64_value = (1i64) << std::numeric_limits<fparser_type::value_type>::digits; // 2**53 for the 'double' type
              #else
                const long long max_accurate_i64_value = (1LL) << std::numeric_limits<fparser_type::value_type>::digits; // 2**53 for the 'double' type
              #endif

                if ( func.StartsWith("hex(") || func.StartsWith("hex (") )
                {
                    if ( abs_val(val) < max_accurate_i64_value )
                    {
                        unsigned __int64 n = static_cast<unsigned __int64>(val);
                        val = fparser_type::value_type(n);
                    }
                }
                else if ( abs_val(val) > m_calc_precision*100 )
                {
                    if ( abs_val(val) < max_accurate_i64_value )
                    {
                        __int64 n = static_cast<__int64>(val);
                        double  diff = static_cast<double>(val - n);

                        if ( abs_val(diff) < m_calc_precision )
                        {
                            // result can be rounded
                            val = fparser_type::value_type(n);
                        }
                    }
                }
            }

            return val;
        }

    private:
        CCriticalSection m_cs;
        fparser_type* m_fp;
        double m_calc_precision;
        TCHAR  m_szCalcDefaultFmt[12];
        TCHAR  m_szCalcSmallFmt[12];
        TCHAR  m_szCalcBigFmt[12];
};

static FParserWrapper g_fp;
/**/

/*
 * CScriptEngine
 *
 *
 * Internal Commands:
 * ------------------
 * cls
 *   - clear Console screen
 * cd
 *   - shows current path
 * cd <path>
 *   - changes current directory (absolute or relative)
 * cd <drive:\path>
 *   - changes current drive and directory
 * dir
 *   - lists subdirs and files
 * dir <mask>
 *   - lists subdirs and files matched the mask
 * dir <path\mask>
 *   - lists subdirs and files matched the mask
 * echo <text>
 *   - prints a text in the Console
 * echo~ <math expression>
 *   - calculates and prints in the Console
 * if <condition> goto <label>
 *   - jumps to the label if the condition is true
 * if~ <condition> goto <label>
 *   - calculates and checks the condition
 * if ... else if ... else ... endif
 *   - conditional execution
 * if~ ... else if~ ... else ... endif
 *   - conditional execution
 * goto <label>
 *   - jumps to the label
 * exit
 *   - exits the current NppExec's script
 * exit <type>
 *   - exits the NppExec's script
 * set 
 *   - shows all user's variables
 * set <var> 
 *   - shows the value of user's variable <var>
 * set <var> = <value> 
 *   - sets the value of user's variable <var>
 * set <var> ~ <math expression>
 *   - calculates the math expression
 * set <var> ~ strlen <string>
 *   - calculates the string length
 * set <var> ~ strlenutf8 <string>
 *   - calculates the UTF-8 string length
 * set <var> ~ strlensci <string>
 *   - string length, using Scintilla's encoding
 * set <var> ~ strupper <string>
 *   - returns the string in upper case
 * set <var> ~ strlower <string>
 *   - returns the string in lower case
 * set <var> ~ substr <pos> <len> <string>
 *   - returns the substring
 * set <var> ~ strfind <s> <t>
 *   - returns the first position of <t> in <s>
 * set <var> ~ strrfind <s> <t>
 *   - returns the last position of <t> in <s>
 * set <var> ~ strreplace <s> <t0> <t1>
 *   - replaces all <t0> with <t1>
 * set <var> ~ strquote <s>
 *   - surrounds <s> with "" quotes
 * set <var> ~ strunquote <s>
 *   - removes the surrounding "" quotes
 * set <var> ~ strescape <s>
 *   - simple character escaping (e.g. <TAB> to '\t')
 * set <var> ~ strunescape <s>
 *   - simple character unescaping (e.g. '\n' to <LF>)
 * set <var> ~ strexpand <s>
 *   - expands all $(sub) values within <s>
 * set <var> ~ strfromhex <hs>
 *   - returns a string from the hex-string
 * set <var> ~ strtohex <s>
 *   - returns a hex-string from the string
 * set <var> ~ chr <n>
 *   - returns a character from a character code <n>
 * set <var> ~ ord <c>
 *   - returns a decimal character code of a character <c>
 * set <var> ~ ordx <c>
 *   - returns a hexadecimal character code of a character <c>
 * set <var> ~ normpath <path>
 *   - returns a normalized path
 * set <var> ~ fileexists <path>
 *   - checks if a given file exists
 * set <var> ~ direxists <path>
 *   - checks if a given directory exists
 * set local
 *   - shows all user's local variables
 * set local <var>
 *   - shows the value of user's local variable <var>
 * set local <var> = ...
 *   - sets the value of user's local variable <var>
 * set local <var> ~ ...
 *   - calculates the value of user's local variable <var>
 * set +v <var> = ...
 *   - sets the value of <var> using delayed vars substitution
 * set +v local <var> = ...
 *   - sets the local <var> using delayed vars substitution
 * unset <var>
 *   - removes user's variable <var>
 * unset local <var>
 *   - removes user's local variable <var>
 * env_set <var>
 *   - shows the value of environment variable <var>
 * env_set <var> = <value>
 *   - sets the value of environment variable <var>
 * env_set local ...
 *   - sets an environment variable locally (within the current script)
 * env_unset <var>
 *   - removes/restores the environment variable <var>
 * inputbox "message"
 *   - shows InputBox, sets $(INPUT)
 * inputbox "message" : initial_value
 *   - shows InputBox with specified initial value, sets $(INPUT)
 * inputbox "message" : "value_name" : initial_value
 *   - InputBox customization
 * inputbox "message" : "value_name" : "initial_value" : time_ms
 *   - expirable InputBox
 * messagebox "text"
 *   - shows a simple MessageBox
 * messagebox "text" : "title"
 *   - shows a MessageBox with a custom title
 * messagebox "text" : "title" : type
 *   - shows a MessageBox of a given type
 * messagebox "text" : "title" : type : time_ms
 *   - expirable MessageBox
 * con_colour <colours>
 *   - sets the Console's colours
 * con_colour local ...
 *   - sets the colours locally (within the current script)
 * con_filter <filters>
 *   - enables/disables the Console's output filters
 * con_filter local ...
 *   - sets the filters locally (within the current script)
 * con_loadfrom <file> 
 *   - loads a file's content to the Console
 * con_load <file>
 *   - see "con_loadfrom"
 * con_saveto <file>
 *   - saves the Console's content to a file
 * con_save <file>
 *   - see "con_saveto"
 * sel_loadfrom <file> 
 *   - replace current selection with a file's content
 * sel_load <file> 
 *   - see "sel_loadfrom"
 * sel_saveto <file>
 *   - save the selected text (in current encoding) to a file
 * sel_saveto <file> : <encoding>
 *   - save the selected text (in specified encoding) to a file
 * sel_save <file> : <encoding>
 *   - see "sel_saveto"
 * sel_settext <text>
 *   - replace current selection with the text specified
 * sel_settext+ <text>
 *   - replace current selection with the text specified
 * text_loadfrom <file> 
 *   - replace the entire text with a file's content
 * text_load <file> 
 *   - see "text_loadfrom"
 * text_saveto <file>
 *   - save the entire text (in current encoding) to a file
 * text_saveto <file> : <encoding>
 *   - save the entire text (in specified encoding) to a file
 * text_save <file> : <encoding>
 *   - see "text_saveto"
 * clip_settext <text>
 *   - set the clipboard text
 * npp_exec <script>
 *   - executes commands from specified NppExec's script
 * npp_exec <file>
 *   - executes commands from specified NppExec's file
 *   - works with a partial file path/name
 * npp_exectext <mode> <text>
 *   - execute the given text
 * npp_close
 *   - closes current file in Notepad++
 * npp_close <file>
 *   - closes specified file opened in Notepad++
 *   - works with a partial file path/name
 * npp_console <on/off/keep>
 *   - show/hide the Console window
 * npp_console <enable/disable>
 *   - enable/disable output to the Console
 * npp_console <1/0/?> 
 *   - show/hide the Console window
 * npp_console <+/->
 *   - enable/disable output to the Console
 * npp_console local ...
 *   - Console on/off locally (within the current script)
 * npp_menucommand <menu\item\name>
 *   - executes (invokes) a menu item
 * npp_open <file>
 *   - opens specified file in Notepad++
 * npp_open <mask>
 *   - opens file(s) matched the mask
 * npp_open <path\mask>
 *   - opens file(s) matched the mask
 * npp_run <command> 
 *   - the same as Notepad++'s Run command
 *   - executes command (runs a child process) w/o waiting until it returns
 * npp_save 
 *   - saves current file in Notepad++
 * npp_save <file>
 *   - saves specified file in Notepad++ (if it's opened in Notepad++)
 *   - works with a partial file path/name
 * npp_saveas <file>
 *   - saves current file with a new (path)name
 * npp_saveall
 *   - saves all modified files
 * npp_switch <file> 
 *   - switches to specified file (if it's opened in Notepad++)
 *   - works with a partial file path/name
 * npp_setfocus
 *    - sets the keyboard focus
 * npp_sendmsg <msg>
 *   - sends a message (msg) to Notepad++
 * npp_sendmsg <msg> <wparam>
 *   - sends a message (msg) with parameter (wparam) to Notepad++
 * npp_sendmsg <msg> <wparam> <lparam>
 *   - sends a message (msg) with parameters (wparam, lparam) to Notepad++
 * npp_sendmsgex <hwnd> <msg> <wparam> <lparam>
 *   - sends a message (msg) with parameters (wparam, lparam) to hwnd
 * sci_sendmsg <msg>
 *   - sends a message (msg) to current Scintilla
 * sci_sendmsg <msg> <wparam>
 *   - sends a message (msg) with parameter (wparam) to current Scintilla
 * sci_sendmsg <msg> <wparam> <lparam>
 *   - sends a message (msg) with parameters (wparam, lparam) to current Scintilla
 * sci_find <flags> <find_what>
 *   - finds a string in the current editing (Scintilla's) window
 * sci_replace <flags> <find_what> <replace_with>
 *   - replaces a string in the current editing (Scintilla's) window
 * proc_input <string>
 *   - send a string to a child process
 * proc_signal <signal>
 *   - signal to a child process
 * sleep <ms>
 *   - sleep for ms milliseconds
 * sleep <ms> <text>
 *   - print the text and sleep for ms milliseconds
 * npe_cmdalias
 *   - show all command aliases
 * npe_cmdalias <alias>
 *   - shows the value of command alias
 * npe_cmdalias <alias> =
 *   - removes the command alias
 * npe_cmdalias <alias> = <command>
 *   - sets the command alias
 * npe_cmdalias local ...
 *   - local command alias (within the current script)
 * npe_console <options>
 *   - set/modify Console options/mode
 * npe_console local ...
 *   - sets Console's mode locally (within the current script)
 * npe_debuglog <on/off>
 *   - enable/disable Debug Log
 * npe_debuglog local ...
 *   - enable/disable Debug Log locally (within the current script)
 * npe_noemptyvars <1/0>
 *   - enable/disable replacement of empty vars
 * npe_noemptyvars local ...
 *   - sets empty vars on/off locally (within the current script)
 * npe_queue <command>
 *   - queue NppExec's command to be executed
 * npe_sendmsgbuflen <max_len>
 *   - set npp_sendmsg/sci_sendmsg's buffer length
 * npe_sendmsgbuflen local ...
 *   - sets the buffer length locally (within the current script)
 *
 * Internal Macros (environment variables):
 * ----------------------------------------
 * The same as here: http://notepad-plus.sourceforge.net/uk/run-HOWTO.php
 *
 * $(FULL_CURRENT_PATH)  : E:\my Web\main\welcome.html
 * $(CURRENT_DIRECTORY)  : E:\my Web\main
 * $(FILE_NAME)          : welcome.html
 * $(NAME_PART)          : welcome
 * $(EXT_PART)           : .html
 * $(NPP_DIRECTORY)      : full path of directory with notepad++.exe
 * $(NPP_FULL_FILE_PATH) : full path to notepad++.exe
 * $(CURRENT_WORD)       : word(s) you selected in Notepad++
 * $(CURRENT_LINE)       : current line number
 * $(CURRENT_LINESTR)    : text of the current line
 * $(CURRENT_COLUMN)     : current column number
 *
 * Additional environment variables:
 * ---------------------------------
 * $(SELECTED_TEXT)      : the text you selected in Notepad++
 * $(FILE_NAME_AT_CURSOR): file name selected in the editor
 * $(WORKSPACE_ITEM_PATH): full path to the current item in the workspace pane
 * $(WORKSPACE_ITEM_DIR) : directory containing the current item in the workspace pane
 * $(WORKSPACE_ITEM_NAME): file name of the current item in the workspace pane
 * $(WORKSPACE_ITEM_ROOT): root path of the current item in the workspace pane
 * $(CLOUD_LOCATION_PATH): cloud location path (in Notepad++'s settings)
 * $(CLIPBOARD_TEXT)     : text from the clipboard
 * $(#0)                 : C:\Program Files\Notepad++\notepad++.exe
 * $(#N), N=1,2,3...     : full path of the Nth opened document
 * $(LEFT_VIEW_FILE)     : current file path-name in primary (left) view
 * $(RIGHT_VIEW_FILE)    : current file path-name in second (right) view
 * $(PLUGINS_CONFIG_DIR) : full path of the plugins configuration directory
 * $(CWD)                : current working directory of NppExec (use "cd" to change it)
 * $(ARGC)               : number of arguments passed to the NPP_EXEC command
 * $(ARGV)               : all arguments passed to the NPP_EXEC command after the script name
 * $(ARGV[0])            : script name - first parameter of the NPP_EXEC command
 * $(ARGV[N])            : Nth argument (N=1,2,3...)
 * $(RARGV)              : all arguments in reverse order (except the script name)
 * $(RARGV[N])           : Nth argument in reverse order (N=1,2,3...)
 * $(INPUT)              : this value is set by the 'inputbox' command
 * $(INPUT[N])           : Nth field of the $(INPUT) value (N=1,2,3...)
 * $(OUTPUT)             : this value can be set by the child process, see npe_console v+
 * $(OUTPUT1)            : first line in $(OUTPUT)
 * $(OUTPUTL)            : last line in $(OUTPUT)
 * $(EXITCODE)           : exit code of the last executed child process
 * $(PID)                : process id of the current (or the last) child process
 * $(IS_PROCESS)         : is child process running (1 - yes, 0 - no)
 * $(LAST_CMD_RESULT)    : result of the last NppExec's command
 *                           (1 - succeeded, 0 - failed, -1 - invalid arg)
 * $(MSG_RESULT)         : result of 'npp_sendmsg[ex]' or 'sci_sendmsg'
 * $(MSG_WPARAM)         : wParam (output) of 'npp_sendmsg[ex]' or 'sci_sendmsg'
 * $(MSG_LPARAM)         : lParam (output) of 'npp_sendmsg[ex]' or 'sci_sendmsg'
 * $(NPP_HWND)           : Notepad++'s main window handle
 * $(NPP_PID)            : Notepad++'s process id
 * $(SCI_HWND)           : current Scintilla's window handle
 * $(SCI_HWND1)          : primary Scintilla's window handle (main view)
 * $(SCI_HWND2)          : secondary Scintilla's window handle (second view)
 * $(CON_HWND)           : NppExec's Console window handle (RichEdit control)
 * $(FOCUSED_HWND)       : focused window handle
 * $(SYS.<var>)          : system's environment variable, e.g. $(SYS.PATH)
 * $(@EXIT_CMD)          : a callback exit command for a child process
 * $(@EXIT_CMD_SILENT)   : a silent (non-printed) callback exit command
 *
 */

CScriptEngine::CScriptCommandRegistry CScriptEngine::m_CommandRegistry;
std::atomic_int CScriptEngine::nTotalRunningEnginesCount = 0;
std::atomic_int CScriptEngine::nExecTextEnginesCount = 0;

CScriptEngine::CScriptEngine(CNppExec* pNppExec, const CListT<tstr>& CmdList, const tstr& id)
{
    m_strInstance = NppExecHelpers::GetInstanceAsString(this);

    m_pNppExec = pNppExec;
    m_CmdList.Assign(CmdList); // own m_CmdList
    m_id = id;

    m_nCmdType = CMDTYPE_UNKNOWN;
    m_nRunFlags = 0;
    m_dwThreadId = 0;
    m_nPrintingMsgReady = -1;
    m_bTriedExitCmd = false;
    m_isClosingConsole = false;

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine - create (instance = %s)"), GetInstanceStr() );

    ++nTotalRunningEnginesCount;
}

CScriptEngine::~CScriptEngine()
{
    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine - destroy (instance = %s)"), GetInstanceStr() );

    if ( --nTotalRunningEnginesCount == 0 )
    {
        nExecTextEnginesCount = 0;
    }
}

const TCHAR* CScriptEngine::GetInstanceStr() const
{
    return m_strInstance.c_str();
}

#define INVALID_TSTR_LIST_ITEM ((CListItemT<tstr>*)(-1))

void CScriptEngine::Run(unsigned int nRunFlags)
{
    m_nRunFlags = nRunFlags;
    m_dwThreadId = ::GetCurrentThreadId();
    m_nPrintingMsgReady = -1;
    m_bTriedExitCmd = false;
    m_isClosingConsole = false;

    if ( m_eventRunIsDone.IsNull() )
        m_eventRunIsDone.Create(NULL, TRUE, FALSE, NULL); // manual reset, non-signaled
    else
        m_eventRunIsDone.Reset();

    if ( m_eventAbortTheScript.IsNull() )
        m_eventAbortTheScript.Create(NULL, TRUE, FALSE, NULL); // manual reset, non-signaled
    else
        m_eventAbortTheScript.Reset();

    m_pNppExec->GetConsole().OnScriptEngineStarted();

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine::Run - start (instance = %s)"), GetInstanceStr() );

    tstr S;
    bool isNppExec = false;
    bool isFirstCmd = true;

    m_execState.nExecCounter = 0;
    m_execState.nExecMaxCount = m_pNppExec->GetOptions().GetInt(OPTI_EXEC_MAXCOUNT);
    m_execState.nGoToMaxCount = m_pNppExec->GetOptions().GetInt(OPTI_GOTO_MAXCOUNT);
    m_execState.nExecTextCounter = 0;
    m_execState.nExecTextMaxCount = m_pNppExec->GetOptions().GetInt(OPTI_EXECTEXT_MAXCOUNT);
    m_execState.pScriptLineCurrent = NULL;
    m_execState.SetScriptLineNext(INVALID_TSTR_LIST_ITEM);
    m_execState.pChildProcess.reset();

    m_execState.ScriptContextList.Add( ScriptContext() );
    {
        ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
        currentScript.CmdRange.pBegin = m_CmdList.GetFirst();
        currentScript.CmdRange.pEnd = 0;
        currentScript.IsNppExeced = false;
        currentScript.IsPrintingMsgReady = -1;
        if ( m_nRunFlags & rfShareLocalVars )
        {
            if ( m_pParentScriptEngine )
            {
                ScriptContext& parentScript = m_pParentScriptEngine->GetExecState().GetCurrentScriptContext();

                // inheriting parent script's local variables
                {
                    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                    currentScript.LocalMacroVars = parentScript.LocalMacroVars;
                }
                {
                    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                    currentScript.LocalCmdAliases = parentScript.LocalCmdAliases;
                }
                // We could use swap() instead of copying here, but if something
                // goes wrong in that case we risk to end with empty local vars
                // in the parent script. So copying is safer.
            }
            else if ( m_nRunFlags & rfShareConsoleLocalVars )
            {
                // inheriting Console's local variables
                {
                    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                    currentScript.LocalMacroVars = m_pNppExec->GetMacroVars().GetUserConsoleMacroVars();
                }
                {
                    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                    currentScript.LocalCmdAliases = m_pNppExec->GetMacroVars().GetConsoleCmdAliases();
                }
            }
        }
        else if ( m_nRunFlags & rfConsoleLocalVarsRead )
        {
            // inheriting Console's local variables
            {
                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                currentScript.LocalMacroVars = m_pNppExec->GetMacroVars().GetUserConsoleMacroVars();
            }
            {
                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                currentScript.LocalCmdAliases = m_pNppExec->GetMacroVars().GetConsoleCmdAliases();
            }
        }
        if ( ((m_nRunFlags & rfShareConsoleState) != 0) || 
             (!m_pParentScriptEngine) || (!m_pParentScriptEngine->IsCollateral()) )
        {
            // inheriting Console's enabled state
            CNppExecConsole& Console = m_pNppExec->GetConsole();
            DWORD scrptEngnId = 0;
            if ( ((m_nRunFlags & rfConsoleLocalVars) == 0) && m_pParentScriptEngine )
                scrptEngnId = m_pParentScriptEngine->GetThreadId();
            int nEnabled = Console.GetOutputEnabledDirectly(scrptEngnId);
            Console.SetOutputEnabledDirectly(m_dwThreadId, nEnabled);
        }
    }

    CListItemT<tstr>* p = m_CmdList.GetFirst();
    while ( p && ContinueExecution() )
    {
        eCmdType nCmdType = CMDTYPE_COMMENT_OR_EMPTY;

        m_execState.pScriptLineCurrent = p;
        m_execState.SetScriptLineNext(INVALID_TSTR_LIST_ITEM);

        S = p->GetItem();

        if ( S.length() > 0 )
        {

            if ( (m_nRunFlags & rfCollateralScript) == 0 )
            {
                Runtime::GetLogger().Clear();
            }
            Runtime::GetLogger().Add(   _T("; command info") );
            Runtime::GetLogger().AddEx( _T("Current command item:  p = 0x%X"), p );
            Runtime::GetLogger().Add(   _T("{") );
            Runtime::GetLogger().IncIndentLevel();
            Runtime::GetLogger().AddEx( _T("_item  = \"%s\""), p->GetItem().c_str() );
            Runtime::GetLogger().AddEx( _T("_prev  = 0x%X"), p->GetPrev() );
            if ( p->GetPrev() )
            {
                const CListItemT<tstr>* pPrev = p->GetPrev();
                Runtime::GetLogger().Add(   _T("{") );
                Runtime::GetLogger().IncIndentLevel();
                Runtime::GetLogger().AddEx( _T("_item = \"%s\""), pPrev->GetItem().c_str() );
                Runtime::GetLogger().AddEx( _T("_prev = 0x%X;  _next = 0x%X;  _owner = 0x%X"), 
                    pPrev->GetPrev(), pPrev->GetNext(), pPrev->GetOwner() );
                Runtime::GetLogger().DecIndentLevel();
                Runtime::GetLogger().Add(   _T("}") );
            }
            Runtime::GetLogger().AddEx( _T("_next  = 0x%X"), p->GetNext() ); 
            if ( p->GetNext() )
            {
                const CListItemT<tstr>* pNext = p->GetNext();
                Runtime::GetLogger().Add(   _T("{") );
                Runtime::GetLogger().IncIndentLevel();
                Runtime::GetLogger().AddEx( _T("_item = \"%s\""), pNext->GetItem().c_str() );
                Runtime::GetLogger().AddEx( _T("_prev = 0x%X;  _next = 0x%X;  _owner = 0x%X"), 
                    pNext->GetPrev(), pNext->GetNext(), pNext->GetOwner() );
                Runtime::GetLogger().DecIndentLevel();
                Runtime::GetLogger().Add(   _T("}") );
            }
            Runtime::GetLogger().AddEx( _T("_owner = 0x%X"), p->GetOwner() );
            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );
        
            Runtime::GetLogger().Add(   _T("; executing ModifyCommandLine") );

            ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
            const int ifDepth = currentScript.GetIfDepth();
            const eIfState ifState = currentScript.GetIfState().state;

            nCmdType = modifyCommandLine(this, S, ifState);
            if ( nCmdType != CMDTYPE_COMMENT_OR_EMPTY )
            {
                if ( isSkippingThisCommandDueToIfState(nCmdType, ifState) )
                {
                    Runtime::GetLogger().AddEx( _T("; skipping - waiting for %s"),
                        (ifState == IF_WANT_ENDIF || ifState == IF_WANT_SILENT_ENDIF || ifState == IF_EXECUTING || ifState == IF_EXECUTING_ELSE) ? DoEndIfCommand::Name() : DoElseCommand::Name() );
                
                    if ( nCmdType == CMDTYPE_IF || nCmdType == CMDTYPE_CALCIF )
                    {
                        // nested IF inside a conditional block that is being skipped
                        bool bWantSilentEndIf = false;
                        int n = -1;
                        eIfMode mode = getIfMode(S, n);
                        if ( mode != IF_GOTO )
                        {
                            bWantSilentEndIf = true;
                        }
                        else if ( p->GetNext() )
                        {
                            tstr S2 = p->GetNext()->GetItem();
                            if ( modifyCommandLine(this, S2, IF_NONE) == CMDTYPE_ELSE ) // IF-GOTO ... ELSE
                                bWantSilentEndIf = true;
                        }
                        if ( bWantSilentEndIf )
                        {
                            currentScript.PushIfState({IF_WANT_SILENT_ENDIF, m_execState.pScriptLineCurrent});
                            // skip the nested IF...ENDIF block as well - mark it as "silent"
                        }
                    }
                    else if ( nCmdType == CMDTYPE_ENDIF )
                    {
                        // nested ENDIF inside a conditional block that is being skipped
                        if ( ifState == IF_WANT_SILENT_ENDIF ) // <-- this condition is redundant, but let it be just in case
                        {
                            // nested "silent" IF...ENDIF block is completed
                            currentScript.PopIfState();
                        }
                    }
                    else if ( nCmdType == CMDTYPE_ELSE )
                    {
                        if ( ifState == IF_EXECUTING )
                        {
                            // the IF...ELSE block completed
                            currentScript.SetIfState(IF_WANT_ENDIF);
                        }
                        else if ( ifState == IF_EXECUTING_ELSE )
                        {
                            tstr Msg;
                            NppExecHelpers::StrDelTrailingAnySpaces(S);
                            NppExecHelpers::StrDelLeadingAnySpaces(S);
                            if ( !S.IsEmpty() )  S.Insert(0, _T(' '));
                            Msg.Format(64 + S.length(), _T("; \"ELSE%s\" is ignored because of preceding plain ELSE"), S.c_str());
                            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                            m_pNppExec->GetConsole().PrintError(Msg.c_str(), nMsgFlags);

                            currentScript.SetIfState(IF_WANT_ENDIF);
                        }
                    }
                }
                else
                {
                    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

                    if ( nCmdType == CMDTYPE_COLLATERAL_FORCED )
                    {
                        CListT<tstr> CmdList(S);
                        if ( !m_pNppExec->GetCommandExecutor().ExecuteCollateralScript(CmdList, tstr()) )
                            nCmdResult = CMDRESULT_FAILED;
                    }
                    else
                    {
                        m_nCmdType = nCmdType;
                        if ( isFirstCmd && (m_nCmdType == CMDTYPE_NPPEXEC) )
                        {
                            isNppExec = true;
                        }
                        else 
                        {
                            if ( isFirstCmd )
                            {
                                isFirstCmd = false;

                                if ( m_nCmdType == CMDTYPE_NPPCONSOLE )
                                {
                                    tstr Sp(S);
                                    isLocalParam(Sp); // removes "local" from Sp
                                    int param = getOnOffParam(Sp);
                                    if ( param == PARAM_ENABLE || param == PARAM_DISABLE )
                                    {
                                        if ( !m_pNppExec->isConsoleDialogVisible() )
                                        {
                                            m_pNppExec->showConsoleDialog(CNppExec::showIfHidden, 0);
                                        }
                                    }
                                }
                            }
                            if ( isNppExec )
                            {
                                if ( m_nCmdType != CMDTYPE_NPPCONSOLE )
                                {
                                    m_pNppExec->showConsoleDialog(CNppExec::showIfHidden, 0);
                                }
                                isNppExec = false;
                            }
                        }

                        m_sCmdParams = S;
                        EXECFUNC pCmdExecFunc = m_CommandRegistry.GetCmdExecFunc(m_nCmdType);
                        nCmdResult = pCmdExecFunc(this, S);
                    }

                    // The same currentScript object is used here to handle NPP_EXEC as well
                    // (it's safe because ScriptContextList.DeleteLast() is not called before)
                    if ( (m_nCmdType != CMDTYPE_ELSE && m_nCmdType != CMDTYPE_IF) &&
                         (ifState == IF_MAYBE_ELSE) &&
                         (currentScript.GetIfState().state == IF_MAYBE_ELSE) &&
                         (currentScript.GetIfDepth() == ifDepth) )
                    {
                        // The IfState was not changed, so remove IF_MAYBE_ELSE
                        currentScript.PopIfState();
                    }

                    {
                        TCHAR szCmdResult[50];
                        c_base::_tint2str(nCmdResult, szCmdResult);

                        tstr varName = MACRO_LAST_CMD_RESULT;
                        m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, szCmdResult, CNppExecMacroVars::svLocalVar ); // local var
                    }
                }
            }

            Runtime::GetLogger().Add(   _T("") );
        }

        p = (m_execState.pScriptLineNext == INVALID_TSTR_LIST_ITEM) ? p->GetNext() : m_execState.pScriptLineNext;

        ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
        if ( currentScript.IsNppExeced )
        {
            if ( p == currentScript.CmdRange.pEnd )
            {
                CListItemT<tstr>* pItem = currentScript.CmdRange.pBegin;
                while ( pItem != currentScript.CmdRange.pEnd )
                {
                    CListItemT<tstr>* pNext = pItem->GetNext();
                    m_CmdList.Delete(pItem);
                    pItem = pNext;
                }

                // In case of several npp_exec-ed scripts with the same CmdRange.pEnd,
                // the currentScript reference will become invalid after the first call
                // of m_execState.ScriptContextList.DeleteLast() below.
                // That is why we should not refer to currentScript within the loop below.
                // Instead, we are saving the values of currentScript.IsSharingLocalVars
                // and currentScript.LocalMacroVars here.
                CMacroVars localMacroVars;
                CMacroVars localCmdAliases;
                bool isSharingLocalVars = currentScript.IsSharingLocalVars;
                if ( isSharingLocalVars )
                {
                    {
                        CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                        localMacroVars.swap(currentScript.LocalMacroVars);
                    }
                    {
                        CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                        localCmdAliases.swap(currentScript.LocalCmdAliases);
                    }
                }

                // There can be several npp_exec-ed scripts with the same CmdRange.pEnd
                // Note: do _not_ use the currentScript within this loop (see the comment above)
                while ( !m_execState.ScriptContextList.IsEmpty() )
                {
                    ScriptContext& currScript = m_execState.GetCurrentScriptContext();
                    if ( currScript.IsNppExeced && (p == currScript.CmdRange.pEnd) )
                    {
                        Runtime::GetLogger().AddEx( _T("; script context removed: { Name = \"%s\"; CmdRange = [0x%X, 0x%X) }"), 
                            currScript.ScriptName.c_str(), currScript.CmdRange.pBegin, currScript.CmdRange.pEnd ); 

                        if ( nRunFlags & rfStartScript )
                        {
                            auto pLastContext = m_execState.ScriptContextList.GetLast();
                            if ( pLastContext->GetItem().IsNppExeced )
                            {
                                auto pPrevContext = pLastContext->GetPrev();
                                if ( pPrevContext && pPrevContext == m_execState.ScriptContextList.GetFirst() )
                                {
                                    pPrevContext->GetItem().IsPrintingMsgReady = pLastContext->GetItem().IsPrintingMsgReady;
                                }
                            }
                        }

                        m_execState.ScriptContextList.DeleteLast();
                    }
                    else
                    {
                        if ( isSharingLocalVars )
                        {
                            {
                                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                                currScript.LocalMacroVars.swap(localMacroVars);
                            }
                            {
                                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                                currScript.LocalCmdAliases.swap(localCmdAliases);
                            }
                        }

                        break;
                    }
                }

                Runtime::GetLogger().Add(   _T("") );
            }
        }
    } // while

    m_nPrintingMsgReady = m_execState.GetCurrentScriptContext().IsPrintingMsgReady;

    if ( isNppExec )
    {
        // the whole script contains just NPP_EXEC commands
        // the Console is still hidden, it must be shown
        m_pNppExec->showConsoleDialog(CNppExec::showIfHidden, 0);
    }

    if ( !m_pNppExec->GetCommandExecutor().GetRunningScriptEngine() )
    {
        // a collateral script may be running
        m_pNppExec->_consoleIsVisible = m_pNppExec->isConsoleDialogVisible();
    }

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine::Run - end (instance = %s)"), GetInstanceStr() );

    if ( m_nRunFlags & rfShareLocalVars )
    {
        if ( m_pParentScriptEngine )
        {
            ScriptContext& currentScriptContext = m_execState.GetCurrentScriptContext();
            ScriptContext& parentScriptContext = m_pParentScriptEngine->GetExecState().GetCurrentScriptContext();
            {
                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                parentScriptContext.LocalMacroVars.swap(currentScriptContext.LocalMacroVars);
            }
            {
                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                parentScriptContext.LocalCmdAliases.swap(currentScriptContext.LocalCmdAliases);
            }
            parentScriptContext.IsPrintingMsgReady = currentScriptContext.IsPrintingMsgReady;
        }
        else if ( m_nRunFlags & rfShareConsoleLocalVars )
        {
            ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
            {
                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                m_pNppExec->GetMacroVars().GetUserConsoleMacroVars() = currentScript.LocalMacroVars; // copying
            }
            {
                CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                m_pNppExec->GetMacroVars().GetConsoleCmdAliases() = currentScript.LocalCmdAliases; // copying
            }
        }
    }
    else if ( m_nRunFlags & rfConsoleLocalVarsWrite )
    {
        ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
        {
            CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
            m_pNppExec->GetMacroVars().GetUserConsoleMacroVars() = currentScript.LocalMacroVars; // copying
        }
        {
            CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
            m_pNppExec->GetMacroVars().GetConsoleCmdAliases() = currentScript.LocalCmdAliases; // copying
        }
    }
    if ( m_nRunFlags & rfShareConsoleState ) // <-- another script can enable the output, but usually that does not affect the parent's state
    {
        CNppExecConsole& Console = m_pNppExec->GetConsole();
        int nEnabled = Console.GetOutputEnabledDirectly(m_dwThreadId);
        DWORD scrptEngnId = 0;
        if ( ((m_nRunFlags & rfConsoleLocalVars) == 0) && m_pParentScriptEngine )
            scrptEngnId = m_pParentScriptEngine->GetThreadId();
        Console.SetOutputEnabledDirectly(scrptEngnId, nEnabled);
    }

    m_pNppExec->GetConsole().OnScriptEngineFinished();

    m_eventRunIsDone.Set();
}

bool CScriptEngine::IsParentOf(const std::shared_ptr<CScriptEngine> pScriptEngine) const
{
    std::shared_ptr<CScriptEngine> pEngine = GetChildScriptEngine();
    while ( pEngine )
    {
        if ( pEngine == pScriptEngine )
            return true;

        pEngine = pEngine->GetChildScriptEngine();
    }
    return false;
}

bool CScriptEngine::IsChildOf(const std::shared_ptr<CScriptEngine> pScriptEngine) const
{
    std::shared_ptr<CScriptEngine> pEngine = GetParentScriptEngine();
    while ( pEngine )
    {
        if ( pEngine == pScriptEngine )
            return true;

        pEngine = pEngine->GetParentScriptEngine();
    }
    return false;
}

bool CScriptEngine::ContinueExecution() const
{
    return (m_pNppExec->_consoleIsVisible || ((m_nRunFlags & rfExitScript) != 0 && !m_pNppExec->_bStopTheExitScript)); 
}

void CScriptEngine::ScriptError(eErrorType type, const TCHAR* cszErrorMessage)
{
    switch ( type )
    {
        case ET_REPORT:
            m_pNppExec->GetConsole().PrintError( GetLastLoggedCmd().c_str() );
            m_pNppExec->GetConsole().PrintError( cszErrorMessage );
            break;

        case ET_ABORT:
          #if SCRPTENGNID_DEBUG_OUTPUT
            {
                tstr S;
                S.Format(1020, _T("CScriptEngine::ScriptError - scrptEngnId=%u\n"), m_dwThreadId);
                ::OutputDebugString(S.c_str());
            }
          #endif
            m_execState.pScriptLineNext = NULL; // stop the script
            m_eventAbortTheScript.Set();
            m_pNppExec->GetConsole().OnScriptEngineAborting(m_dwThreadId);
            Runtime::GetLogger().Add_WithoutOutput( cszErrorMessage );
            Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine::ScriptError(ET_ABORT) (instance = %s)"), GetInstanceStr() );
            break;

        case ET_UNPREDICTABLE:
            m_execState.pScriptLineNext = NULL; // stop the script
            m_eventAbortTheScript.Set();
            // do not call it here: m_pNppExec->GetConsole().OnScriptEngineAborting(m_dwThreadId);
            m_pNppExec->GetConsole().PrintError( GetLastLoggedCmd().c_str() );
            m_pNppExec->GetConsole().PrintError( cszErrorMessage );
            m_pNppExec->GetConsole().PrintError( _T("To prevent unpredictable behavior, the script is stopped.") );
            Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine::ScriptError(ET_UNPREDICTABLE)  (instance = %s)"), GetInstanceStr() );
            break;
    }
}

void CScriptEngine::UndoAbort(const TCHAR* cszMessage)
{
    m_execState.pScriptLineNext = m_execState.pScriptLineNextBackupCopy;
    m_eventAbortTheScript.Reset();

    Runtime::GetLogger().AddEx_WithoutOutput( _T("; CScriptEngine::UndoAbort() (instance = %s) : %s"), GetInstanceStr(), cszMessage );
}

std::shared_ptr<CChildProcess> CScriptEngine::GetRunningChildProcess()
{
    std::shared_ptr<CChildProcess> pChildProc;
    std::shared_ptr<CScriptEngine> pScriptEngine = m_pNppExec->GetCommandExecutor().GetRunningScriptEngine(); // shared_ptr(this)
    while ( pScriptEngine && !pChildProc )
    {
        pChildProc = pScriptEngine->GetExecState().GetRunningChildProcess();
        pScriptEngine = pScriptEngine->GetParentScriptEngine();
    }
    return pChildProc;
}

bool CScriptEngine::IsChildProcessRunning() const
{
    bool isChildProcRunning = false;
    std::shared_ptr<CScriptEngine> pScriptEngine = m_pNppExec->GetCommandExecutor().GetRunningScriptEngine(); // shared_ptr(this)
    while ( pScriptEngine && !isChildProcRunning )
    {
        isChildProcRunning = pScriptEngine->GetExecState().IsChildProcessRunning();
        pScriptEngine = pScriptEngine->GetParentScriptEngine();
    }
    return isChildProcRunning;
}

void CScriptEngine::ChildProcessMustBreakAll()
{
    std::shared_ptr<CScriptEngine> pScriptEngine = m_pNppExec->GetCommandExecutor().GetRunningScriptEngine(); // shared_ptr(this)
    while ( pScriptEngine )
    {
        std::shared_ptr<CChildProcess> pChildProc = pScriptEngine->GetExecState().GetRunningChildProcess();
        if ( pChildProc )
            pChildProc->MustBreak(CProcessKiller::killCtrlBreak);
        pScriptEngine = pScriptEngine->GetParentScriptEngine();
    }
}

bool CScriptEngine::WaitUntilDone(DWORD dwTimeoutMs) const
{
    return (m_eventRunIsDone.IsNull() || (m_eventRunIsDone.Wait(dwTimeoutMs) == WAIT_OBJECT_0));
}

bool CScriptEngine::IsPrintingMsgReady() const 
{
    return (m_nPrintingMsgReady == -1) ? m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_PRINTMSGREADY) : (m_nPrintingMsgReady != 0);
}

CScriptEngine::eNppExecCmdPrefix CScriptEngine::checkNppExecCmdPrefix(const CNppExec* pNppExec, tstr& Cmd, bool bRemovePrefix)
{
    eNppExecCmdPrefix ret = CmdPrefixNone;
    int nPrefixLen = 0;
    const TCHAR* pszPrefix = pNppExec->GetOptions().GetStr(OPTS_NPPEXEC_CMD_PREFIX, &nPrefixLen);
    if ( nPrefixLen != 0 )
    {
        if ( Cmd.StartsWith(pszPrefix) )
        {
            ret = CmdPrefixCollateralOrRegular;
            if ( Cmd.GetAt(nPrefixLen) == pszPrefix[nPrefixLen - 1] ) // is the last symbol doubled?
            {
                // if the prefix is "nppexec:" (default), then "nppexec::" is expected here
                // if the prefix was changed to e.g. "npe-", then "npe--" is expected here
                ++nPrefixLen;
                ret = CmdPrefixCollateralForced;
            }
            if ( bRemovePrefix )
            {
                Cmd.Delete(0, nPrefixLen);
                NppExecHelpers::StrDelLeadingAnySpaces(Cmd);
            }
        }
    }
    return ret;
}

CScriptEngine::eCmdType CScriptEngine::getCmdType(CNppExec* pNppExec, CScriptEngine* pScriptEngine, tstr& Cmd, unsigned int nFlags)
{
    const bool useLogging = ((nFlags & ctfUseLogging) != 0);
    const bool ignorePrefix = ((nFlags & ctfIgnorePrefix) != 0);
    const bool reportError = ((nFlags & ctfReportError) != 0);

    if ( useLogging )
    {
        Runtime::GetLogger().Add(   _T("GetCmdType()") );
        Runtime::GetLogger().Add(   _T("{") );
        Runtime::GetLogger().IncIndentLevel();
        Runtime::GetLogger().AddEx( _T("[in]  \"%s\""), Cmd.c_str() );
    }

    NppExecHelpers::StrDelLeadingAnySpaces(Cmd);
    NppExecHelpers::StrDelTrailingAnySpaces(Cmd);

    CScriptEngine::eNppExecCmdPrefix cmdPrefix = checkNppExecCmdPrefix(pNppExec, Cmd);

    if ( Cmd.IsEmpty() )
    {
        if ( useLogging )
        {
            Runtime::GetLogger().AddEx( _T("[ret] %d (empty command)"), CMDTYPE_COMMENT_OR_EMPTY );
            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );
        }

        return CMDTYPE_COMMENT_OR_EMPTY;
    }

    if ( (!ignorePrefix) && (cmdPrefix == CmdPrefixCollateralForced) )
    {
        if ( useLogging )
        {
            Runtime::GetLogger().AddEx( _T("[ret] %d (forced collateral command)"), CMDTYPE_COLLATERAL_FORCED );
            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );
        }

        return CMDTYPE_COLLATERAL_FORCED;
    }

    const TCHAR* pAliasNppExec = pNppExec->GetOptions().GetStr(OPTS_ALIAS_CMD_NPPEXEC);
    if ( pAliasNppExec && pAliasNppExec[0] )
    {
        if ( pAliasNppExec[0] == Cmd.GetAt(0) ) // the alias is one character
        {
            if ( (Cmd.GetAt(0) != _T('\\') || Cmd.GetAt(1) != _T('\\')) &&  // not "\\..."
                 (Cmd.GetAt(0) != _T('/') || Cmd.GetAt(1) != _T('/')) )     // not "//..."
            {
                Cmd.Delete(0, 1);

                if ( useLogging )
                {
                    Runtime::GetLogger().AddEx( _T("[ret] 0x%X (%s)"), CMDTYPE_NPPEXEC, DoNppExecCommand::Name() );
                    Runtime::GetLogger().DecIndentLevel();
                    Runtime::GetLogger().Add(   _T("}") );
                }

                return CMDTYPE_NPPEXEC;
            }
        }
    }

    pNppExec->GetMacroVars().CheckCmdAliases(pScriptEngine, Cmd, useLogging);

    if ( Cmd.GetAt(0) == _T(':') && Cmd.GetAt(1) == _T(':') )
    {
        if ( reportError )
        {
            pNppExec->GetConsole().PrintError( _T("- can not use \"::\" at the beginning of line!") );
        }

        if ( useLogging )
        {
            Runtime::GetLogger().AddEx( _T("[ret] %d (command starts with ::)"), CMDTYPE_COMMENT_OR_EMPTY );
            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );
        }

        return CMDTYPE_COMMENT_OR_EMPTY;
    }

    if ( DEFAULT_ALIAS_CMD_LABEL == Cmd.GetAt(0) )
    {
        Cmd.Delete(0, 1);

        if ( useLogging )
        {
            Runtime::GetLogger().AddEx( _T("[ret] 0x%X (%s)"), CMDTYPE_LABEL, DoLabelCommand::Name() );
            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );
        }

        return CMDTYPE_LABEL;
    }

    tstr S = Cmd;
    NppExecHelpers::StrUpper(S);
  
    eCmdType nCmdType = CMDTYPE_UNKNOWN;
    if ( S.StartsWith(DoCdCommand::Name()) )
    {
        int i = lstrlen(DoCdCommand::Name());
        const TCHAR next_ch = S.GetAt(i);
        if ( IsAnySpaceOrEmptyChar(next_ch) || next_ch == _T('\\') || next_ch == _T('/') || next_ch == _T('.') )
        {
            nCmdType = DoCdCommand::Type();
            Cmd.Delete(0, i);
        }
    }
    if ( nCmdType == CMDTYPE_UNKNOWN )
    {
        int i = S.FindOneOf(_T(" \t\v\f"));
        S.Delete(i);
        nCmdType = m_CommandRegistry.GetCmdTypeByName(S);
        if ( nCmdType != CMDTYPE_UNKNOWN )
            Cmd.Delete(0, i);
    }

    if ( useLogging )
    {
        if ( nCmdType != CMDTYPE_UNKNOWN )
        {
            Runtime::GetLogger().AddEx( _T("[ret] 0x%X (%s)"), nCmdType, m_CommandRegistry.GetCmdNameByType(nCmdType) );
        }
        else
        {
            Runtime::GetLogger().AddEx( _T("[ret] 0x%X (unknown)"), nCmdType );
        }
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );
    }
  
    return nCmdType;
}

bool CScriptEngine::isSkippingThisCommandDueToIfState(eCmdType cmdType, eIfState ifState)
{
    return ( (ifState == IF_WANT_SILENT_ENDIF) ||
             ((ifState == IF_WANT_ENDIF) && (cmdType != CMDTYPE_ENDIF)) ||
             ((ifState == IF_WANT_ELSE) && (cmdType != CMDTYPE_ELSE && cmdType != CMDTYPE_ENDIF)) || 
             ((ifState == IF_EXECUTING || ifState == IF_EXECUTING_ELSE) && (cmdType == CMDTYPE_ELSE)) );
}

bool CScriptEngine::usesDelayedVarSubstitution(eCmdType cmdType)
{
    switch ( cmdType )
    {
        case CMDTYPE_SET:
        case CMDTYPE_UNSET:
        case CMDTYPE_IF:
        case CMDTYPE_CALCIF:
        case CMDTYPE_ELSE:
        case CMDTYPE_NPPSENDMSG:
        case CMDTYPE_NPPSENDMSGEX:
        case CMDTYPE_SCISENDMSG:
        case CMDTYPE_SCIFIND:
        case CMDTYPE_SCIREPLACE:
        case CMDTYPE_NPECMDALIAS:
        case CMDTYPE_NPEQUEUE:
            return true;
    }
    return false;
}

CScriptEngine::eCmdType CScriptEngine::modifyCommandLine(CScriptEngine* pScriptEngine, tstr& Cmd, eIfState ifState)
{
    Runtime::GetLogger().Add(   _T("ModifyCommandLine()") );
    Runtime::GetLogger().Add(   _T("{") );
    Runtime::GetLogger().IncIndentLevel();
    Runtime::GetLogger().AddEx( _T("[in]  \"%s\""), Cmd.c_str() );

    CNppExec* pNppExec = pScriptEngine->GetNppExec();
    CScriptEngine::eNppExecCmdPrefix cmdPrefix = checkNppExecCmdPrefix(pNppExec, Cmd);

    const bool bCommEmpty = isCmdCommentOrEmpty(pNppExec, Cmd);
    const bool bDirective = bCommEmpty ? false : isCmdDirective(pNppExec, Cmd);
    if ( bCommEmpty || bDirective )
    {
        Runtime::GetLogger().Add(   bCommEmpty ? _T("; it\'s a comment or empty string") : _T("; it\'s a directive") );
        Runtime::GetLogger().Add(   _T("; command argument(s):") );
        Runtime::GetLogger().AddEx( _T("[out] \"%s\""), Cmd.c_str() );
        Runtime::GetLogger().Add(   _T("; command type:") );
        Runtime::GetLogger().AddEx( _T("[ret] %d"), CMDTYPE_COMMENT_OR_EMPTY );
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );
      
        return CMDTYPE_COMMENT_OR_EMPTY;
    }

    if ( cmdPrefix == CmdPrefixCollateralForced )
    {
        Runtime::GetLogger().Add(   _T("; it\'s a forced collateral command") );
        Runtime::GetLogger().Add(   _T("; command type:") );
        Runtime::GetLogger().AddEx( _T("[ret] %d (forced collateral command)"), CMDTYPE_COLLATERAL_FORCED );
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

        return CMDTYPE_COLLATERAL_FORCED;
    }
  
    // ... checking commands ...

    eCmdType nCmdType = getCmdType(pNppExec, pScriptEngine, Cmd);
    
    if ( nCmdType == CMDTYPE_COMMENT_OR_EMPTY )
    {
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

        return nCmdType;
    }

    if ( (nCmdType != CMDTYPE_UNKNOWN) && 
         isSkippingThisCommandDueToIfState(nCmdType, ifState) )
    {
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

        return nCmdType;
    }

    NppExecHelpers::StrDelLeadingAnySpaces(Cmd);
    if ( Cmd.IsEmpty() || (nCmdType == CMDTYPE_CLS) )
    {
        Runtime::GetLogger().Add(   _T("; no arguments given") );
        Runtime::GetLogger().Add(   _T("; command argument(s):") );
        Runtime::GetLogger().AddEx( _T("[out] \"%s\""), Cmd.c_str() );
        Runtime::GetLogger().Add(   _T("; command type:") );
        Runtime::GetLogger().AddEx( _T("[ret] 0x%X"), nCmdType );
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );
      
        return nCmdType;
    }
  
    if ( !usesDelayedVarSubstitution(nCmdType) )
    {
        bool bCmdStartsWithMacroVar = Cmd.StartsWith(_T("$("));

        CNppExecMacroVars& MacroVars = pNppExec->GetMacroVars();

        // ... checking all the macro-variables ...
        MacroVars.CheckAllMacroVars(pScriptEngine, Cmd, true);
        
        if ( bCmdStartsWithMacroVar && (nCmdType == CMDTYPE_UNKNOWN) )
        {
            // re-check nCmdType after macro-var substitution
            nCmdType = getCmdType(pNppExec, pScriptEngine, Cmd);
            if ( nCmdType == CMDTYPE_COLLATERAL_FORCED )
            {
                Runtime::GetLogger().Add(   _T("; it\'s a forced collateral command") );
                Runtime::GetLogger().Add(   _T("; command type:") );
                Runtime::GetLogger().AddEx( _T("[ret] %d (forced collateral command)"), CMDTYPE_COLLATERAL_FORCED );
                Runtime::GetLogger().DecIndentLevel();
                Runtime::GetLogger().Add(   _T("}") );

                return CMDTYPE_COLLATERAL_FORCED;
            }
            
            if ( nCmdType != CMDTYPE_UNKNOWN )
            {
                NppExecHelpers::StrDelLeadingAnySpaces(Cmd);
            }
        }

        if ( isSkippingThisCommandDueToIfState(nCmdType, ifState) )
        {
            Runtime::GetLogger().DecIndentLevel();
            Runtime::GetLogger().Add(   _T("}") );

            return nCmdType;
        }
    }
    // we have to process macro-vars inside doSendMsg()
    // because macro-var's string may contain double-quotes
  
    // ... do we need "" around the command's argument? ...
    
    bool bDone = false;
    if ( (nCmdType == CMDTYPE_CONCOLOUR) ||
         (nCmdType == CMDTYPE_CONFILTER) ||
         (nCmdType == CMDTYPE_GOTO) || 
         (nCmdType == CMDTYPE_SLEEP) ||
         usesDelayedVarSubstitution(nCmdType) )
    {
        bDone = true;
    }
    else if ( (nCmdType != 0) &&
              (nCmdType != CMDTYPE_NPPRUN) &&
              (nCmdType != CMDTYPE_NPPEXEC) &&
              (nCmdType != CMDTYPE_ECHO) &&
              (nCmdType != CMDTYPE_SELSAVETO) &&
              (nCmdType != CMDTYPE_INPUTBOX) &&
              (nCmdType != CMDTYPE_MESSAGEBOX) )
    {
        NppExecHelpers::StrUnquote(Cmd);

        bDone = true; // we don't need '\"' in file_name
    }

    // ... adding '\"' to the command if it's needed ...
    if ( !bDone )
    {
        bool bHasSpaces = false;
        // disabled by default  because of problems 
        // for executables without extension i.e. 
        // "cmd /c app.exe"  <-- "cmd" is without extension
        if (pNppExec->GetOptions().GetBool(OPTB_PATH_AUTODBLQUOTES))
        {
            if (!bDone && (Cmd.GetAt(0) != _T('\"')))
            {
                int i = 0;
                int j = 0;
                while (!bDone && (i < Cmd.length()))
                {
                    if (Cmd[i] == _T(' '))
                    {
                        bHasSpaces = true;
                        j = i - 1;
                        while (!bDone && j >= 0)
                        {
                            const TCHAR ch = Cmd[j];
                            if (ch == _T('.'))
                            {
                                Cmd.Insert(i, _T('\"'));
                                Cmd.Insert(0, _T('\"'));
                                bDone = true;
                            }
                            else if (ch == _T('\\') || ch == _T('/'))
                            {
                                j = 0; // j-- makes j<0 so this loop is over
                            }
                            j--;
                        }
                    }
                    i++;
                }
            }
        }
    }
    
    Runtime::GetLogger().Add(   _T("; command argument(s):") );
    Runtime::GetLogger().AddEx( _T("[out] \"%s\""), Cmd.c_str() );
    Runtime::GetLogger().Add(   _T("; command type:") );
    Runtime::GetLogger().AddEx( _T("[ret] 0x%X"), nCmdType );
    Runtime::GetLogger().DecIndentLevel();
    Runtime::GetLogger().Add(   _T("}") );

    return nCmdType;
}

bool CScriptEngine::isLocalParam(tstr& param)
{
    bool isLocal = false;
    int n = param.FindOneOf(_T(" \t\v\f"));
    if ( n < 0 )
    {
        n = param.length();
    }
    if ( n == 5 ) // length of "local"
    {
        if ( NppExecHelpers::StrCmpNoCase(param.c_str(), 5, _T("local"), 5) == 0 )
        {
            isLocal = true;
            param.Delete(0, n);
            NppExecHelpers::StrDelLeadingAnySpaces(param);
        }
    }
    return isLocal;
}

bool CScriptEngine::isDelayedSubstVar(tstr& param, bool& bKeywordPresent)
{
    bool isDelayed = false;
    bool isKeyword = false;
    int n = param.FindOneOf(_T(" \t\v\f"));
    if ( n == 2 ) // length of "+v"
    {
        if ( NppExecHelpers::LatinCharLower(param[1]) == _T('v') )
        {
            switch ( param[0] )
            {
                case _T('+'): // "+v"
                    isKeyword = true;
                    isDelayed = true;
                    break;
                case _T('-'): // "-v"
                    isKeyword = true;
                    isDelayed = false;
                    break;
            }
            if ( isKeyword )
            {
                param.Delete(0, n);
                NppExecHelpers::StrDelLeadingAnySpaces(param);
            }
        }
    }
    bKeywordPresent = isKeyword;
    return isDelayed;
}

// Note: be sure to call isLocalParam() prior to getOnOffParam()!
int CScriptEngine::getOnOffParam(const tstr& param)
{
    if ( param.IsEmpty() )
        return PARAM_EMPTY; // no param

    if ( param == _T("1") )
        return PARAM_ON;

    if ( param == _T("0") )
        return PARAM_OFF;

    if ( param == _T("?") )
        return PARAM_KEEP;

    if ( param == _T("+") )
        return PARAM_ENABLE;

    if ( param == _T("-") )
        return PARAM_DISABLE;

    tstr S = param;
    NppExecHelpers::StrUpper(S);

    if ( (S == _T("ON")) || (S == _T("TRUE")) )
        return PARAM_ON;

    if ( (S == _T("OFF")) || (S == _T("FALSE")) )
        return PARAM_OFF;

    if ( S == _T("KEEP") )
        return PARAM_KEEP;

    if ( S == _T("ENABLE") )
        return PARAM_ENABLE;

    if ( S == _T("DISABLE") )
        return PARAM_DISABLE;

    return PARAM_UNKNOWN; // unknown
}

bool CScriptEngine::isCmdCommentOrEmpty(const CNppExec* pNppExec, tstr& Cmd)
{
    const tstr comment = pNppExec->GetOptions().GetStr(OPTS_COMMENTDELIMITER);
    if (comment.length() > 0)
    {
        int i = Cmd.Find(comment.c_str()); // comment
        if ((i >= 0) && 
            ((comment != _T("//")) || (Cmd.GetAt(i-1) != _T(':')))) // skip :// e.g. http://
        {
            Cmd.Delete(i, -1); // delete all after "//"

            Runtime::GetLogger().AddEx( _T("; comment removed: everything after %s"), comment.c_str() );

        }
    }
  
    NppExecHelpers::StrDelLeadingAnySpaces(Cmd);
    NppExecHelpers::StrDelTrailingAnySpaces(Cmd);

    return Cmd.IsEmpty();
}

bool CScriptEngine::isCmdDirective(const CNppExec* , tstr& Cmd)
{
    NppExecHelpers::StrDelLeadingAnySpaces(Cmd);

    if ( Cmd.StartsWith(DIRECTIVE_PREFIX_CHAR) )
    {
        NppExecHelpers::StrDelTrailingAnySpaces(Cmd);
        NppExecHelpers::StrUpper(Cmd);

        if ( Cmd == DIRECTIVE_COLLATERAL )
            return true;
    }

    return false;
}

int CScriptEngine::isCmdNppExecPrefixed(CNppExec* pNppExec, CScriptEngine* pScriptEngine, tstr& cmd, unsigned int nFlags)
{
    bool bRemovePrefix = ((nFlags & npfRemovePrefix) != 0);
    bool bSubstituteMacroVars = ((nFlags & npfSubstituteMacroVars) != 0);

    // We don't call StrDelLeadingAnySpaces for 'cmd' as the leading space(s)
    // can be a meaningful part of a command given to the child process 
    // (example: Python, where indentation is important).
    tstr s = cmd;
    NppExecHelpers::StrDelLeadingAnySpaces(s);
    eNppExecCmdPrefix cmdPrefix = checkNppExecCmdPrefix(pNppExec, s, bRemovePrefix);
    if ( cmdPrefix != CmdPrefixNone )
    {
        cmd = s;
        const eCmdType cmdType = getCmdType(pNppExec, pScriptEngine, s, ctfIgnorePrefix);

        if ( bSubstituteMacroVars && !usesDelayedVarSubstitution(cmdType) )
        {
            CNppExecMacroVars& MacroVars = pNppExec->GetMacroVars();
            MacroVars.CheckCmdAliases(pScriptEngine, cmd, true);
            MacroVars.CheckAllMacroVars(pScriptEngine, cmd, true);
        }
    }
    else if ( bSubstituteMacroVars )
    {
        CNppExecMacroVars& MacroVars = pNppExec->GetMacroVars();
        MacroVars.CheckCmdAliases(pScriptEngine, cmd, true);
        MacroVars.CheckAllMacroVars(pScriptEngine, cmd, true);

        s = cmd;
        NppExecHelpers::StrDelLeadingAnySpaces(s);
        cmdPrefix = checkNppExecCmdPrefix(pNppExec, s, bRemovePrefix);
        if ( cmdPrefix != CmdPrefixNone )
            cmd = s;
    }

    return cmdPrefix;
}

bool CScriptEngine::isScriptCollateral(const CNppExec* pNppExec, const CListT<tstr>& CmdList)
{
    bool isCollateral = false;
    tstr S;

    const CListItemT<tstr>* p = CmdList.GetFirst();
    for ( ; p != NULL; p = p->GetNext() )
    {
        if ( p->GetItem().length() > 0 )
        {
            S = p->GetItem();
            removeLineEnding(S);
            if ( !isCmdCommentOrEmpty(pNppExec, S) )
            {
                if ( isCmdDirective(pNppExec, S) )
                {
                    if ( S == DIRECTIVE_COLLATERAL )
                        isCollateral = true;
                }
                break;
            }
        }
    }

    return isCollateral;
}

void CScriptEngine::addCommandToList(CListT<tstr>& CmdList, tstr& Cmd, unsigned int nFlags)
{
    if ( Cmd.length() > 0 || (nFlags & acfAddEmptyLines) != 0 )
    {
        CmdList.Add( Cmd );
    }
}

tCmdList CScriptEngine::getCmdListFromText(const TCHAR* pszText, unsigned int nFlags)
{
    TCHAR ch;
    tstr Line;
    tCmdList CmdList;

    while ( (ch = *pszText) != 0 )
    {
        bool bAddLine = false;

        if ( ch == _T('\n') )
        {
            if ( nFlags & acfKeepLineEndings )
            {
                Line += _T('\n');
            }
            bAddLine = true;
        }
        else if ( ch == _T('\r') )
        {
            if ( nFlags & acfKeepLineEndings )
            {
                Line += _T('\r');
            }
            if ( *(pszText + 1) == _T('\n') )
            {
                if ( nFlags & acfKeepLineEndings )
                {
                    Line += _T('\n');
                }
                ++pszText;
            }
            bAddLine = true;
        }
        else
        {
            Line += ch;
        }

        if ( bAddLine )
        {
            addCommandToList( CmdList, Line, nFlags );
            Line.Clear();
        }

        ++pszText;
    }

    addCommandToList( CmdList, Line, nFlags );

    return CmdList;
}

void CScriptEngine::removeLineEndings(CListT<tstr>& CmdList)
{
    for ( auto pItem = CmdList.GetFirst(); pItem != NULL; pItem = pItem->GetNext() )
    {
        tstr& Cmd = pItem->GetItem();
        removeLineEnding(Cmd);
    }
}

void CScriptEngine::removeLineEnding(tstr& Cmd)
{
    if ( Cmd.EndsWith(_T('\n')) )
    {
        Cmd.DeleteLastChar();
    }
    if ( Cmd.EndsWith(_T('\r')) )
    {
        Cmd.DeleteLastChar();
    }
}

tCmdList CScriptEngine::getCollateralCmdListForChildProcess(CNppExec* pNppExec, const tCmdList& CmdList)
{
    tCmdList CollateralCmdList;
    tstr Cmd;
    tstr CollateralCmd;
    tstr ChildProcCmds;

    for ( auto pItem = CmdList.GetFirst(); pItem != NULL; pItem = pItem->GetNext() )
    {
        Cmd = pItem->GetItem();
        int cmdPrefix = isCmdNppExecPrefixed(pNppExec, nullptr, Cmd, npfRemovePrefix);
        if ( cmdPrefix != CmdPrefixNone )
        {
            removeLineEnding(Cmd);
            if ( !ChildProcCmds.IsEmpty() )
            {
                CollateralCmd = DoProcInputCommand::Name();
                CollateralCmd += _T(" ");
                CollateralCmd += ChildProcCmds;
                CollateralCmdList.Add(CollateralCmd);
                ChildProcCmds.Clear();
            }
            if ( cmdPrefix == CmdPrefixCollateralForced )
            {
                CollateralCmd = pNppExec->GetOptions().GetStr(OPTS_NPPEXEC_CMD_PREFIX);
                CollateralCmd += CollateralCmd.GetLastChar();
                CollateralCmd += Cmd;
                CollateralCmdList.Add(CollateralCmd);
            }
            else
            {
                CollateralCmdList.Add(Cmd);
            }
        }
        else
            ChildProcCmds += Cmd;
    }
    if ( !CollateralCmdList.IsEmpty() && !ChildProcCmds.IsEmpty() )
    {
        CollateralCmd = DoProcInputCommand::Name();
        CollateralCmd += _T(" ");
        CollateralCmd += ChildProcCmds;
        CollateralCmdList.Add(CollateralCmd);
        ChildProcCmds.Clear();
    }
    return CollateralCmdList;
}

void CScriptEngine::errorCmdNotEnoughParams(const TCHAR* cszCmd, const TCHAR* cszErrorMessage)
{
    if ( cszErrorMessage )
        Runtime::GetLogger().AddEx( _T("; %s"), cszErrorMessage );
    else
        Runtime::GetLogger().Add(   _T("; argument(s) expected, but none given") );

    tstr Cmd = cszCmd;
    tstr Err = Cmd;
    Err += _T(':');
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
    m_pNppExec->GetConsole().PrintMessage( Err.c_str(), nMsgFlags );
    NppExecHelpers::StrLower(Cmd);
    Err.Format( 250, _T("- %s; type \"help %s\" for help"), cszErrorMessage ? cszErrorMessage : _T("empty command (no parameters given)"), Cmd.c_str() );
    m_pNppExec->GetConsole().PrintError( Err.c_str() );

    m_nCmdType = CMDTYPE_UNKNOWN;
}

void CScriptEngine::errorCmdNoParam(const TCHAR* cszCmd)
{
    errorCmdNotEnoughParams(cszCmd, NULL);
}

void CScriptEngine::messageConsole(const TCHAR* cszCmd, const TCHAR* cszParams)
{
    tstr S = cszCmd;
    S += _T(": ");
    S += cszParams;
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
    m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
}

bool CScriptEngine::reportCmdAndParams(const TCHAR* cszCmd, const tstr& params, unsigned int uFlags)
{
    if ( params.IsEmpty() )
    {
        if ( uFlags & fReportEmptyParam )
            errorCmdNoParam(cszCmd);

        if ( uFlags & fFailIfEmptyParam )
            return false;
    
        Runtime::GetLogger().AddEx( _T("; executing: %s"), cszCmd );
        m_sLoggedCmd.Format( 1020, _T("; executing: %.960s"), cszCmd );
    }
    else
    {
        Runtime::GetLogger().AddEx( _T("; executing: %s %s"), cszCmd, params.c_str() );
        m_sLoggedCmd.Format( 1020, _T("; executing: %.480s %.480s"), cszCmd, params.c_str() );
    }

    if ( uFlags & fMessageToConsole )
    {
        if ( params.length() > MAX_VAR_LENGTH2SHOW )
        {
            tstr S;

            S.Append( params.c_str(), MAX_VAR_LENGTH2SHOW - 5 );
            S += _T("(...)");
            messageConsole( cszCmd, S.c_str() );
        }
        else
        {
            if ( params.IsEmpty() )
            {
                const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                m_pNppExec->GetConsole().PrintMessage( cszCmd, nMsgFlags );
            }
            else
                messageConsole( cszCmd, params.c_str() );
        }
    }

    return true;
}

void CScriptEngine::updateFocus()
{
    if ( (!m_pNppExec->m_hFocusedWindowBeforeScriptStarted) ||
         (m_pNppExec->m_hFocusedWindowBeforeScriptStarted == m_pNppExec->GetConsole().GetConsoleWnd()) ||
         (m_pNppExec->m_hFocusedWindowBeforeScriptStarted == m_pNppExec->GetConsole().GetDialogWnd()) ||
         (m_pNppExec->m_hFocusedWindowBeforeScriptStarted == m_pNppExec->GetScintillaHandle()) )
    {
        ::SendMessage( m_pNppExec->GetScintillaHandle(), WM_SETFOCUS, 0, 0 );
        m_pNppExec->m_hFocusedWindowBeforeScriptStarted = m_pNppExec->GetScintillaHandle();
    }
}

CScriptEngine::eCmdResult CScriptEngine::Do(const tstr& params)
{
    if ( params.IsEmpty() )
    {
        return CMDRESULT_INVALIDPARAM;
    }

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    Runtime::GetLogger().AddEx( _T("; about to start a child process: \"%s\""), params.c_str() );
    m_sLoggedCmd.Format( 1020, _T("; about to start a child process: \"%.960s\""), params.c_str() );

    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
    m_pNppExec->GetConsole().PrintMessage( params.c_str(), nMsgFlags );

    std::shared_ptr<CChildProcess> proc(new CChildProcess(this));
    m_execState.pChildProcess = proc;
    // Note: proc->Create() does not return until the child process exits
    if ( proc->Create(m_pNppExec->GetConsole().GetDialogWnd(), params.c_str()) )
    {
        Runtime::GetLogger().Add(   _T("; child process finished") );
    }
    else
    {
        Runtime::GetLogger().Add(   _T("; failed to start a child process") );

        nCmdResult = CMDRESULT_FAILED;
    }

    TCHAR szExitCode[50];
    c_base::_tint2str(proc->GetExitCode(), szExitCode);

    tstr varName = MACRO_EXITCODE;
    m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, szExitCode, CNppExecMacroVars::svLocalVar ); // local var

    if ( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SETOUTPUTVAR) )
    {
        tstr& OutputVar = proc->GetOutput();
        if ( OutputVar.GetLastChar() == _T('\n') )
            OutputVar.SetSize(OutputVar.length() - 1);
        if ( OutputVar.GetFirstChar() == _T('\n') )
            OutputVar.Delete(0, 1);
        
        // $(OUTPUT)
        varName = MACRO_OUTPUT;
        m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, OutputVar, CNppExecMacroVars::svLocalVar ); // local var

        // $(OUTPUTL)
        varName = MACRO_OUTPUTL;

        int i = OutputVar.RFind( _T('\n') );
        if ( i >= 0 )
        {
            tstr varValue;

            ++i;
            varValue.Assign( OutputVar.c_str() + i, OutputVar.length() - i );
            m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, varValue, CNppExecMacroVars::svLocalVar ); // local var
        }
        else
            m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, OutputVar, CNppExecMacroVars::svLocalVar ); // local var

        // $(OUTPUT1)
        varName = MACRO_OUTPUT1;

        i = OutputVar.Find( _T('\n') );
        if ( i >= 0 )
        {
            tstr varValue;

            varValue.Assign( OutputVar.c_str(), i );
            m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, varValue, CNppExecMacroVars::svLocalVar ); // local var
        }
        else
            m_pNppExec->GetMacroVars().SetUserMacroVar( this, varName, OutputVar, CNppExecMacroVars::svLocalVar ); // local var
    }

    m_execState.pChildProcess.reset();

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoCd(const tstr& params)
{
    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    TCHAR szPath[FILEPATH_BUFSIZE];

    reportCmdAndParams( DoCdCommand::Name(), params, fMessageToConsole );

    // changing current directory

    if ( params.length() > 0 )
    {
        if ( ((params[0] == _T('\\')) && (params.GetAt(1) != _T('\\'))) || 
            ((params[0] == _T('/')) && (params.GetAt(1) != _T('/'))) )
        {
            // root directory of current drive e.g. 'C:\', 'D:\' etc.
            GetCurrentDirectory( FILEPATH_BUFSIZE - 1, szPath );
            if ( szPath[1] == _T(':') )
            {
                szPath[2] = _T('\\');
                szPath[3] = 0;
              
                Runtime::GetLogger().AddEx( _T("; changed to \"%s%s\""), szPath, params.c_str() + 1 );
              
                nCmdResult = SetCurrentDirectory( szPath ) ? CMDRESULT_SUCCEEDED : CMDRESULT_FAILED;
                if ( params[1] )
                {
                    nCmdResult = SetCurrentDirectory( params.c_str() + 1 ) ? CMDRESULT_SUCCEEDED : CMDRESULT_FAILED;
                }
            }
        }
        else
        {
            int ofs = 0;

            if ( (params[1] == _T(':')) && 
                 (params[2] == _T('\\') || params[2] == _T('/')) && 
                 (params[3] != 0) )
            {
                // changing the drive
                szPath[0] = params[0];
                szPath[1] = params[1];
                szPath[2] = 0;
                nCmdResult = SetCurrentDirectory( szPath ) ? CMDRESULT_SUCCEEDED : CMDRESULT_FAILED;
                ofs = 2;
            }
            if ( nCmdResult == CMDRESULT_SUCCEEDED ) // the same drive or successfully changed
            {
                // set current directory
                nCmdResult = SetCurrentDirectory( params.c_str() + ofs ) ? CMDRESULT_SUCCEEDED : CMDRESULT_FAILED;
                if ( nCmdResult == CMDRESULT_FAILED )
                {
                    // trying to set the parent directory at least...
                    // (and nCmdResult is still CMDRESULT_FAILED)
                    for ( int i = params.length() - 1; i > ofs; i-- )
                    {
                        if ( params[i] == _T('\\') || params[i] == _T('/') )
                        {
                            lstrcpyn( szPath, params.c_str() + ofs, i - ofs + 1 );
                            szPath[i - ofs + 1] = 0;
                            SetCurrentDirectory( szPath );
                            break;
                        }
                    }
                }
            }
        }
    }
    GetCurrentDirectory( FILEPATH_BUFSIZE - 1, szPath );
    tstr S = _T("Current directory: ");
    S += szPath;
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
    m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoCls(const tstr& /*params*/)
{
    m_pNppExec->GetConsole().ClearText(true);

    return CMDRESULT_SUCCEEDED;
}

static BOOL getColorFromStr(const TCHAR* szColor, COLORREF* pColor)
{
    if ( szColor && *szColor )
    {
        c_base::byte_t bt[4];

        int n = c_base::_thexstr2buf( szColor, bt, 4 );
        if ( n >= 3 )
        {
            *pColor = RGB( bt[0], bt[1], bt[2] );
            return TRUE;
        }

        if ( n <= 1 )
        {
            if ( szColor[0] == _T('0') && szColor[1] == 0 )
            {
                *pColor = 0; // handle "0" as correct default value
                return TRUE;
            }
        }
    }

    return FALSE;
}

CScriptEngine::eCmdResult CScriptEngine::DoConColour(const tstr& params)
{
    reportCmdAndParams( DoConColourCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    tstr colorParams = params;
    bool isLocal = isLocalParam(colorParams);

    if ( colorParams.length() > 0 )
    {
        NppExecHelpers::StrUpper(colorParams);

        const int posFG = colorParams.Find(_T("FG"));
        const int posBG = colorParams.Find(_T("BG"));

        if ( posFG >= 0 )
        {
            tstr colorFG;
            int n = (posBG > posFG) ? (posBG - posFG) : (colorParams.length() - posFG);
            
            colorFG.Append(colorParams.c_str() + posFG, n);
            n = colorFG.Find(_T('='));
            if ( n >= 0 )
            {
                COLORREF color;

                colorFG.Delete(0, n + 1);
                NppExecHelpers::StrDelLeadingAnySpaces(colorFG);
                NppExecHelpers::StrDelTrailingAnySpaces(colorFG);
                if ( (!colorFG.IsEmpty()) && getColorFromStr(colorFG.c_str(), &color) )
                {
                    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
                    SavedConfiguration& savedConf = currentScript.SavedConf;
                    if ( isLocal )
                    {
                        if ( !savedConf.hasUseEditorColorsInConsole() )
                            savedConf.setUseEditorColorsInConsole( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_USEEDITORCOLORS) );
                        if ( !savedConf.hasColorTextNorm() )
                            savedConf.setColorTextNorm( m_pNppExec->GetConsole().GetCurrentColorTextNorm() );
                    }
                    else
                    {
                        if ( savedConf.hasUseEditorColorsInConsole() )
                            savedConf.removeUseEditorColorsInConsole();
                        if ( savedConf.hasColorTextNorm() )
                            savedConf.removeColorTextNorm();
                    }
                    m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_USEEDITORCOLORS, false);
                    m_pNppExec->GetConsole().SetCurrentColorTextNorm(color);
                }
                else
                {
                    ScriptError( ET_REPORT, _T("- incorrect value of \'FG\'") );
                    nCmdResult = CMDRESULT_INVALIDPARAM;
                }
            }
            else
            {
                ScriptError( ET_REPORT, _T("- \'FG\' found, but no value specified") );
                nCmdResult = CMDRESULT_INVALIDPARAM;
            }
        }

        if ( posBG >= 0 )
        {
            tstr colorBG;
            int n = (posFG > posBG) ? (posFG - posBG) : (colorParams.length() - posBG);

            colorBG.Append(colorParams.c_str() + posBG, n);
            n = colorBG.Find(_T('='));
            if ( n >= 0 )
            {
                COLORREF color;

                colorBG.Delete(0, n + 1);
                NppExecHelpers::StrDelLeadingAnySpaces(colorBG);
                NppExecHelpers::StrDelTrailingAnySpaces(colorBG);
                if ( (!colorBG.IsEmpty()) && getColorFromStr(colorBG.c_str(), &color) )
                {
                    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
                    SavedConfiguration& savedConf = currentScript.SavedConf;
                    if ( isLocal )
                    {
                        if ( !savedConf.hasUseEditorColorsInConsole() )
                            savedConf.setUseEditorColorsInConsole( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_USEEDITORCOLORS) );
                        if ( !savedConf.hasColorBkgnd() )
                            savedConf.setColorBkgnd( m_pNppExec->GetConsole().GetCurrentColorBkgnd() );
                    }
                    else
                    {
                        if ( savedConf.hasUseEditorColorsInConsole() )
                            savedConf.removeUseEditorColorsInConsole();
                        if ( savedConf.hasColorBkgnd() )
                            savedConf.removeColorBkgnd();
                    }
                    m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_USEEDITORCOLORS, false);
                    m_pNppExec->GetConsole().SetCurrentColorBkgnd(color);
                    m_pNppExec->GetConsole().UpdateColours();
                }
                else
                {
                    ScriptError( ET_REPORT, _T("- incorrect value of \'BG\'") );
                    nCmdResult = CMDRESULT_INVALIDPARAM;
                }
            }
            else
            {
                ScriptError( ET_REPORT, _T("- \'BG\' found, but no value specified") );
                nCmdResult = CMDRESULT_INVALIDPARAM;
            }
        }

        if ( (posFG < 0) && (posBG < 0) )
        {
            tstr Err = _T("- unknown parameter: ");
            Err += params;
            ScriptError( ET_REPORT, Err.c_str() );
            nCmdResult = CMDRESULT_INVALIDPARAM;
        }
    }
    else
    {
        tstr S;
        TCHAR buf[16];
        COLORREF color;

        color = m_pNppExec->GetConsole().GetCurrentColorTextNorm();
        buf[0] = 0;
        c_base::_tbuf2hexstr( (const c_base::byte_t*) &color, 3, buf, 16, _T(" ") );
        S = _T("Foreground (text) colour:  ");
        S += buf;
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );

        color = m_pNppExec->GetConsole().GetCurrentColorBkgnd();
        buf[0] = 0;
        c_base::_tbuf2hexstr( (const c_base::byte_t*) &color, 3, buf, 16, _T(" ") );
        S = _T("Background colour:         ");
        S += buf;
        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoConFilter(const tstr& params)
{
    if ( !reportCmdAndParams( DoConFilterCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // +(-)i1..5    Include
    // +(-)x1..5    eXclude
    // +(-)fr1..4   Find+Replace (case-insensitive)
    // +(-)frc1..4  Find+Replace (match case)
    // +(-)h1..10   Highlight

    enum eOptionState {
        stateUnknown = 0,
        stateOn,
        stateOff
    };

    enum eOptionType {
        typeUnknown = 0,
        typeInclude,
        typeExclude,
        typeFindReplaceIgnoreCase,
        typeFindReplaceMatchCase,
        typeHighlight
    };

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    SavedConfiguration& savedConf = currentScript.SavedConf;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    bool isHighlightChanged = false;

    tstr sOption;
    tstr sParams = params;
    bool isLocal = isLocalParam(sParams);
    CStrSplitT<TCHAR> args;
    const int n = args.SplitToArgs(sParams);
    for ( int i = 0; i < n; i++ )
    {
        sOption = args.GetArg(i);
        NppExecHelpers::StrUpper(sOption);

        bool bWrongIndex = false;
        eOptionType type = typeUnknown;
        eOptionState state = stateUnknown;
        if ( sOption.GetAt(0) == _T('+') )
            state = stateOn;
        else if ( sOption.GetAt(0) == _T('-') )
            state = stateOff;

        if ( state != stateUnknown )
        {
            switch ( sOption.GetAt(1) )
            {
                case _T('I'):
                    type = typeInclude;
                    break;
                case _T('X'):
                    type = typeExclude;
                    break;
                case _T('F'):
                    if ( sOption.GetAt(2) == _T('R') )
                    {
                        if ( sOption.GetAt(3) == _T('C') )
                            type = typeFindReplaceMatchCase;
                        else
                            type = typeFindReplaceIgnoreCase;
                    }
                    break;
                case _T('H'):
                    type = typeHighlight;
                    break;
            }

            if ( type != typeUnknown )
            {
                const TCHAR* pIndex = sOption.c_str();
                if ( type == typeFindReplaceMatchCase )
                    pIndex += 4; // skip "+FRC"
                else if ( type == typeFindReplaceIgnoreCase )
                    pIndex += 3; // skip "+FR"
                else
                    pIndex += 2; // skip "+I" or "+X" or "+H"

                const int nIndex = c_base::_tstr2int(pIndex) - 1;
                const bool bEnable = (state == stateOn) ? true : false;
                switch ( type )
                {
                    case typeInclude:
                        if ( nIndex >= 0 && nIndex < CConsoleOutputFilterDlg::FILTER_ITEMS )
                        {
                            const int nIndexMask = (0x01 << nIndex);
                            int nConFltrInclMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_INCLMASK);
                            if ( isLocal )
                            {
                                if ( !savedConf.hasConFltrInclMask() )
                                    savedConf.setConFltrInclMask(nConFltrInclMask);
                                if ( !savedConf.hasConFltrEnable() )
                                    savedConf.setConFltrEnable( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE) );
                            }
                            else
                            {
                                if ( savedConf.hasConFltrInclMask() )
                                {
                                    int nVal = savedConf.getConFltrInclMask();
                                    nVal |= nIndexMask;
                                    if ( !bEnable )  nVal ^= nIndexMask;
                                    savedConf.setConFltrInclMask(nVal);
                                }
                                if ( savedConf.hasConFltrEnable() )
                                    savedConf.removeConFltrEnable();
                            }
                            nConFltrInclMask |= nIndexMask;
                            if ( !bEnable )  nConFltrInclMask ^= nIndexMask;
                            m_pNppExec->GetOptions().SetInt(OPTI_CONFLTR_INCLMASK, nConFltrInclMask);
                            if ( bEnable )  m_pNppExec->GetOptions().SetBool(OPTB_CONFLTR_ENABLE, true);
                        }
                        else
                        {
                            bWrongIndex = true;
                        }
                        break;
                    case typeExclude:
                        if ( nIndex >= 0 && nIndex < CConsoleOutputFilterDlg::FILTER_ITEMS )
                        {
                            const int nIndexMask = (0x01 << nIndex);
                            int nConFltrExclMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_EXCLMASK);
                            if ( isLocal )
                            {
                                if ( !savedConf.hasConFltrExclMask() )
                                    savedConf.setConFltrExclMask(nConFltrExclMask);
                                if ( !savedConf.hasConFltrEnable() )
                                    savedConf.setConFltrEnable( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE) );
                            }
                            else
                            {
                                if ( savedConf.hasConFltrExclMask() )
                                {
                                    int nVal = savedConf.getConFltrExclMask();
                                    nVal |= nIndexMask;
                                    if ( !bEnable )  nVal ^= nIndexMask;
                                    savedConf.setConFltrExclMask(nVal);
                                }
                                if ( savedConf.hasConFltrEnable() )
                                    savedConf.removeConFltrEnable();
                            }
                            nConFltrExclMask |= nIndexMask;
                            if ( !bEnable )  nConFltrExclMask ^= nIndexMask;
                            m_pNppExec->GetOptions().SetInt(OPTI_CONFLTR_EXCLMASK, nConFltrExclMask);
                            if ( bEnable )  m_pNppExec->GetOptions().SetBool(OPTB_CONFLTR_ENABLE, true);
                        }
                        else
                        {
                            bWrongIndex = true;
                        }
                        break;
                    case typeFindReplaceIgnoreCase:
                    case typeFindReplaceMatchCase:
                        if ( nIndex >= 0 && nIndex < CConsoleOutputFilterDlg::REPLACE_ITEMS )
                        {
                            const int nIndexMask = (0x01 << nIndex);
                            const bool bMatchCase = (type == typeFindReplaceMatchCase) ? true : false;
                            int nRplcFltrFindMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_R_FINDMASK);
                            if ( isLocal )
                            {
                                if ( !savedConf.hasRplcFltrFindMask() )
                                    savedConf.setRplcFltrFindMask(nRplcFltrFindMask);
                                if ( !savedConf.hasConFltrRplcEnable() )
                                    savedConf.setConFltrRplcEnable( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_ENABLE) );
                            }
                            else
                            {
                                if ( savedConf.hasRplcFltrFindMask() )
                                {
                                    int nVal = savedConf.getRplcFltrFindMask();
                                    nVal |= nIndexMask;
                                    if ( !bEnable )  nVal ^= nIndexMask;
                                    savedConf.setRplcFltrFindMask(nVal);
                                }
                                if ( savedConf.hasConFltrRplcEnable() )
                                    savedConf.removeConFltrRplcEnable();
                            }
                            nRplcFltrFindMask |= nIndexMask;
                            if ( !bEnable )  nRplcFltrFindMask ^= nIndexMask;
                            m_pNppExec->GetOptions().SetInt(OPTI_CONFLTR_R_FINDMASK, nRplcFltrFindMask);

                            int nRplcFltrCaseMask = m_pNppExec->GetOptions().GetInt(OPTI_CONFLTR_R_CASEMASK);
                            if ( isLocal )
                            {
                                if ( !savedConf.hasRplcFltrCaseMask() )
                                    savedConf.setRplcFltrCaseMask(nRplcFltrCaseMask);
                            }
                            else
                            {
                                if ( savedConf.hasRplcFltrCaseMask() )
                                {
                                    int nVal = savedConf.getRplcFltrCaseMask();
                                    nVal |= nIndexMask;
                                    if ( !bMatchCase )  nVal ^= nIndexMask;
                                    savedConf.setRplcFltrCaseMask(nVal);
                                }
                            }
                            nRplcFltrCaseMask |= nIndexMask;
                            if ( !bMatchCase )  nRplcFltrCaseMask ^= nIndexMask;
                            m_pNppExec->GetOptions().SetInt(OPTI_CONFLTR_R_CASEMASK, nRplcFltrCaseMask);
                            if ( bEnable )  m_pNppExec->GetOptions().SetBool(OPTB_CONFLTR_R_ENABLE, true);
                        }
                        else
                        {
                            bWrongIndex = true;
                        }
                        break;
                    case typeHighlight:
                        if ( nIndex >= 0 && nIndex < CConsoleOutputFilterDlg::RECOGNITION_ITEMS )
                        {
                            if ( isLocal )
                            {
                                if ( !savedConf.hasWarnEffectEnabled() )
                                {
                                    const CWarningAnalyzer& WarnAn = m_pNppExec->GetWarningAnalyzer();
                                    bool WarnEffectEnabled[WARN_MAX_FILTER];
                                    for ( int j = 0; j < WARN_MAX_FILTER; ++j )
                                    {
                                        WarnEffectEnabled[j] = WarnAn.IsEffectEnabled(j);
                                    }
                                    savedConf.setWarnEffectEnabled(WarnEffectEnabled);
                                }
                            }
                            else
                            {
                                if ( savedConf.hasWarnEffectEnabled() )
                                {
                                    const bool* savedWarnEffectEnabled = savedConf.getWarnEffectEnabled();
                                    bool updatedWarnEffectEnabled[WARN_MAX_FILTER];
                                    for ( int j = 0; j < WARN_MAX_FILTER; ++j )
                                    {
                                        updatedWarnEffectEnabled[j] = savedWarnEffectEnabled[j];
                                    }
                                    updatedWarnEffectEnabled[nIndex] = bEnable;
                                    savedConf.setWarnEffectEnabled(updatedWarnEffectEnabled);
                                }
                            }
                            m_pNppExec->GetWarningAnalyzer().EnableEffect(nIndex, bEnable);
                            isHighlightChanged = true;
                        }
                        else
                        {
                            bWrongIndex = true;
                        }
                        break;
                }
            }
        }
        
        if ( state == stateUnknown || type == typeUnknown )
        {
            tstr Err = _T("- unknown option: ");
            Err += sOption;
            ScriptError( ET_REPORT, Err.c_str() );
            nCmdResult = CMDRESULT_INVALIDPARAM;
        }
        else if ( bWrongIndex )
        {
            tstr Err = _T("- wrong index: ");
            Err += sOption;
            ScriptError( ET_REPORT, Err.c_str() );
            nCmdResult = CMDRESULT_INVALIDPARAM;
        }
    }

    if ( isHighlightChanged )
    {
        m_pNppExec->UpdateGoToErrorMenuItem();
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoConLoadFrom(const tstr& params)
{
    if ( !reportCmdAndParams( DoConLoadFromCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    int nBytes = m_pNppExec->conLoadFrom(params.c_str());
    if ( nBytes < 0 )
    {
        ScriptError( ET_REPORT, _T("- can not open the file") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoConSaveTo(const tstr& params)
{
    if ( !reportCmdAndParams( DoConSaveToCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;
        
    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr S;
    int  nBytes = m_pNppExec->conSaveTo(params.c_str());
    if ( nBytes >= 0 )
      S.Format(120, _T("- OK, %d bytes have been written to \""), nBytes);
    else
      S = _T("- failed to write to \"");
    S += params;
    S += _T("\"");
    if ( nBytes >= 0 )
    {
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
    }
    else
    {
        ScriptError( ET_REPORT, S.c_str() );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoDir(const tstr& params)
{
    const TCHAR* cszPathAndFilter = params.IsEmpty() ? _T("*") : params.c_str();
    reportCmdAndParams( DoDirCommand::Name(), cszPathAndFilter, fMessageToConsole );

    tstr Path;
    tstr Filter;
    int  nFilterPos = FileFilterPos( cszPathAndFilter );
    
    GetPathAndFilter( cszPathAndFilter, nFilterPos, Path, Filter );

    Runtime::GetLogger().AddEx( _T("; searching \"%s\" for \"%s\""), Path.c_str(), Filter.c_str() );

    return ( PrintDirContent(m_pNppExec, Path.c_str(), Filter.c_str()) ? CMDRESULT_SUCCEEDED : CMDRESULT_FAILED );
}

CScriptEngine::eCmdResult CScriptEngine::doEcho(const tstr& params, bool isCalc)
{
    reportCmdAndParams( isCalc ? DoCalcEchoCommand::Name() : DoEchoCommand::Name(), params, 0 );

    const TCHAR* cszMessage = params.c_str();
    tstr calcErr;
    tstr calcResult;

    if ( isCalc )
    {
        g_fp.Calculate(m_pNppExec, params, calcErr, calcResult);
        if ( calcErr.IsEmpty() )
        {
            cszMessage = calcResult.c_str();
        }
        else
        {
            calcErr.Insert(0, _T("- fparser calc error: "));
            m_pNppExec->GetConsole().PrintError(calcErr.c_str());
            // cszMessage remains params.c_str()
        }
    }

    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
    m_pNppExec->GetConsole().PrintMessage(cszMessage, nMsgFlags);

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoEcho(const tstr& params)
{
    return doEcho(params, false);
}

CScriptEngine::eCmdResult CScriptEngine::DoCalcEcho(const tstr& params)
{
    return doEcho(params, true);
}

CScriptEngine::eCmdResult CScriptEngine::DoElse(const tstr& params)
{
    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    const eIfState ifState = currentScript.GetIfState().state;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    unsigned int uFlags = 0;
    if ( ifState == IF_NONE || ifState == IF_WANT_ELSE || ifState == IF_MAYBE_ELSE )
        uFlags |= fMessageToConsole;
    reportCmdAndParams( DoElseCommand::Name(), params, uFlags );
    
    if ( ifState == IF_NONE )
    {
        ScriptError( ET_UNPREDICTABLE, _T("- Unexpected ELSE found, without preceding IF.") );
        nCmdResult = CMDRESULT_FAILED;
    }
    else if ( ifState == IF_EXECUTING )
    {
        Runtime::GetLogger().Add(   _T("; IF ... ELSE found, skipping everything up to ENDIF") );

        currentScript.SetIfState(IF_WANT_ENDIF);
    }
    else if ( ifState == IF_EXECUTING_ELSE )
    {
        tstr Msg;
        Msg.Format(64 + params.length(), _T("; \"ELSE %s\" is ignored because of preceding plain ELSE"), params.c_str());
        ScriptError( ET_REPORT, Msg.c_str() );

        nCmdResult = CMDRESULT_FAILED;
        currentScript.SetIfState(IF_WANT_ENDIF);
    }
    else if ( ifState == IF_WANT_ELSE || ifState == IF_MAYBE_ELSE )
    {
        Runtime::GetLogger().Add( _T("; ELSE found") );

        if ( params.IsEmpty() )
        {
            // ELSE without condition
            currentScript.SetIfState(IF_EXECUTING_ELSE);
        }
        else
        {
            // ELSE with condition
            tstr paramsUpperCase = params;
            NppExecHelpers::StrUpper(paramsUpperCase);

            static const TCHAR* arrIf[] = {
                _T("IF "),
                _T("IF\t")
            };

            static const TCHAR* arrCalcIf[] = {
                _T("IF~ "),
                _T("IF~\t")
            };

            bool isCalc = false;
            int n = -1;
            for ( const TCHAR* const& i : arrIf )
            {
                n = paramsUpperCase.RFind(i);
                if ( n >= 0 )
                    break;
            }

            if ( n < 0 )
            {
                for ( const TCHAR* const& i : arrCalcIf )
                {
                    n = paramsUpperCase.RFind(i);
                    if ( n >= 0 )
                    {
                        isCalc = true;
                        break;
                    }
                }
            }

            if ( n < 0 )
            {
                const TCHAR* const errMsg = 
                  (paramsUpperCase.EndsWith(_T("IF")) || paramsUpperCase.EndsWith(_T("IF~")))
                    ? _T("- ELSE IF found, but IF-condition is absent.")
                    : _T("- ELSE IF expected, but IF was not found.");
                ScriptError( ET_UNPREDICTABLE, errMsg );
                nCmdResult = CMDRESULT_FAILED;
            }
            else
            {
                tstr ifParams;
                ifParams.Assign( params.c_str() + n + (isCalc ? 4 : 3) );
                NppExecHelpers::StrDelLeadingAnySpaces(ifParams);
                NppExecHelpers::StrDelTrailingAnySpaces(ifParams);

                doIf(ifParams, true, isCalc);
            }
        }
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoEndIf(const tstr& params)
{
    reportCmdAndParams( DoEndIfCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    if ( !params.IsEmpty() )
    {
        ScriptError( ET_REPORT, _T("- unexpected parameter(s)") );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    const eIfState ifState = currentScript.GetIfState().state;
    if ( ifState == IF_NONE )
    {
        ScriptError( ET_UNPREDICTABLE, _T("- Unexpected ENDIF found, without preceding IF.") );
        nCmdResult = CMDRESULT_FAILED;
    }
    else
    {

        Runtime::GetLogger().Add( _T("; IF ... ENDIF found, done") );

        currentScript.PopIfState();
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoEnvSet(const tstr& params)
{
    if ( !reportCmdAndParams( DoEnvSetCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    bool isInternal = false;

    CStrSplitT<TCHAR> args;
    if ( args.Split(params, _T("="), 2) == 2 )
    {
        isInternal = true;
        
        // set the value
        tstr varName = args.Arg(0);
        bool isLocal = isLocalParam(varName);
        tstr& varValue = args.Arg(1);
        
        NppExecHelpers::StrDelLeadingAnySpaces(varName);
        NppExecHelpers::StrDelTrailingAnySpaces(varName);
        NppExecHelpers::StrDelLeadingAnySpaces(varValue);
        NppExecHelpers::StrDelTrailingAnySpaces(varValue);

        if ( isLocal && varName.IsEmpty() )
        {
            // env_set local = ...
            varName = _T("LOCAL");
            isLocal = false;
        }

        if ( varName.length() > 0 )
        {
            NppExecHelpers::StrUpper(varName);

            {
                CCriticalSectionLockGuard lock(g_csEnvVars);
                if ( g_LocalEnvVarNames.find(varName) == g_LocalEnvVarNames.end() )
                {
                    // not Local Env Var; maybe Global?
                    CMacroVars::const_iterator itrGVar = g_GlobalEnvVars.find(varName);
                    if ( itrGVar == g_GlobalEnvVars.end() )
                    {
                        // not in the Global Env Vars List; but maybe it's Global?
                        tstr sValue = NppExecHelpers::GetEnvironmentVar(varName);
                        if ( !sValue.IsEmpty() )
                        {
                            g_GlobalEnvVars[varName] = sValue;
                            itrGVar = g_GlobalEnvVars.find(varName);
                        }
                        if ( itrGVar == g_GlobalEnvVars.end() )
                        {
                            // not Global Env Var; add to the Local
                            g_LocalEnvVarNames[varName] = 0;
                        }
                    }
                }
            }

            ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
            SavedConfiguration& savedConf = currentScript.SavedConf;
            if ( isLocal )
            {
                if ( !savedConf.hasEnvVar(varName) )
                    savedConf.setEnvVar( varName, NppExecHelpers::GetEnvironmentVar(varName) );
            }
            else
            {
                if ( savedConf.hasEnvVar(varName) )
                    savedConf.removeEnvVar(varName);
            }

            if ( !SetEnvironmentVariable(varName.c_str(), varValue.c_str()) )
                nCmdResult = CMDRESULT_FAILED;
        }
        else
        {
            ScriptError( ET_REPORT, _T("- no variable name specified") );
            return CMDRESULT_INVALIDPARAM;
        }
    }

    // show the value
    tstr varName = args.Arg(0);
    bool isLocal = isLocalParam(varName);
        
    NppExecHelpers::StrDelLeadingAnySpaces(varName);
    NppExecHelpers::StrDelTrailingAnySpaces(varName);

    if ( isLocal && varName.IsEmpty() )
    {
        // env_set local = ...
        varName = _T("LOCAL");
        isLocal = false;
    }

    if ( varName.length() > 0 )
    {
        NppExecHelpers::StrUpper(varName);
            
        bool  bSysVarOK = false;
        tstr sValue = NppExecHelpers::GetEnvironmentVar(varName);
        if ( !sValue.IsEmpty() )
        {
            tstr S = _T("$(SYS.");
            S += varName;
            S += _T(") = ");
            if ( sValue.length() > MAX_VAR_LENGTH2SHOW )
            {
                S.Append( sValue.c_str(), MAX_VAR_LENGTH2SHOW - 5 );
                S += _T("(...)");
            }
            else
            {
                S += sValue;
            }
            UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            if ( isInternal )
                nMsgFlags |= CNppExecConsole::pfIsInternalMsg;
            m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
            bSysVarOK = true;
        }
        if ( !bSysVarOK )
        {
            tstr S = _T("$(SYS.");
            S += varName;
            S += _T(") is empty or does not exist");
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
        }
    }
    else
    {
        ScriptError( ET_REPORT, _T("- no variable name specified") );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoEnvUnset(const tstr& params)
{
    if ( !reportCmdAndParams( DoEnvUnsetCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // removes the value of environment variable, restores the initial value

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr varName = params;
    
    NppExecHelpers::StrDelLeadingAnySpaces(varName);
    NppExecHelpers::StrDelTrailingAnySpaces(varName);

    if ( varName.length() > 0 )
    {
        NppExecHelpers::StrUpper(varName);

        BOOL bResult = FALSE;
        BOOL bRemoved = FALSE;

        {
            CCriticalSectionLockGuard lock(g_csEnvVars);
            CMacroVars::iterator itrLVar = g_LocalEnvVarNames.find(varName);
            if ( itrLVar != g_LocalEnvVarNames.end() )
            {
                // is Local Env Var; remove
                bResult = SetEnvironmentVariable( varName.c_str(), NULL );
                g_LocalEnvVarNames.erase(itrLVar);
                bRemoved = TRUE;
            }
            else
            {
                // not Local Env Var; maybe Global?
                CMacroVars::iterator itrGVar = g_GlobalEnvVars.find(varName);
                if ( itrGVar != g_GlobalEnvVars.end() )
                {
                    // is modified Global Env Var; restore the initial value
                    bResult = SetEnvironmentVariable( varName.c_str(), itrGVar->value.c_str() );
                    g_GlobalEnvVars.erase(itrGVar);
                }
                else
                {
                    // unsafe: maybe non-modified Global Env Var
                }
            }
        }

        tstr S = _T("$(SYS.");
        S += varName;
        if ( bResult )
        {
            S += _T(") has been ");
            S += (bRemoved ? _T("removed") : _T("restored"));
        }
        else
        {
            S += _T(")");
            S.Insert(0, _T("- can not unset this environment variable: "));
            nCmdResult = CMDRESULT_FAILED;
        }
        UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        if ( bResult )
            nMsgFlags |= CNppExecConsole::pfIsInternalMsg;
        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
    }
    else
    {
        ScriptError( ET_REPORT, _T("- no variable name specified") );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoExit(const tstr& params)
{
    reportCmdAndParams( DoExitCommand::Name(), params, fMessageToConsole );

    bool bHardExit = false;
    if ( params.IsEmpty() || params == _T("0") )
    {
        bHardExit = false;
    }
    else if ( params == _T("-1") )
    {
        bHardExit = true;
    }
    else
    {
        ScriptError( ET_REPORT, _T("- unknown exit argument; assuming \"exit -1\"") );
        bHardExit = true;
    }

    if ( bHardExit || !m_execState.GetCurrentScriptContext().IsNppExeced )
    {
        // stop the whole script
        m_execState.pScriptLineNext = NULL;
        m_eventAbortTheScript.Set();
    }
    else
    {
        // stop only the npp_exec'ed part of the script
        m_execState.pScriptLineNext = m_execState.GetCurrentScriptContext().CmdRange.pEnd;
    }

    return CMDRESULT_SUCCEEDED;
}


void CScriptEngine::getLabelName(tstr& labelName)
{
    NppExecHelpers::StrDelLeadingAnySpaces(labelName);
    NppExecHelpers::StrDelTrailingAnySpaces(labelName);
    NppExecHelpers::StrUpper(labelName);

    if ( labelName.StartsWith(_T(':')) )
        labelName.DeleteFirstChar();
}

CScriptEngine::eCmdResult CScriptEngine::DoGoTo(const tstr& params)
{
    if ( !reportCmdAndParams( DoGoToCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    tstr labelName = params;
    getLabelName(labelName);

    if ( labelName.IsEmpty() )
    {
        ScriptError( ET_REPORT, _T("- label name is empty") );
        return CMDRESULT_INVALIDPARAM;
    }

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    tLabels& Labels = currentScript.Labels;
    tLabels::iterator itrLabel = Labels.find(labelName);
    if ( itrLabel == Labels.end() )
    {
        Runtime::GetLogger().Activate(false);

        tstr Cmd;
        CListItemT<tstr>* p = m_execState.pScriptLineCurrent->GetNext();
        while ( p && (p != currentScript.CmdRange.pEnd) )
        {
            Cmd = p->GetItem();
            if ( getCmdType(m_pNppExec, this, Cmd, ctfUseLogging) == CMDTYPE_LABEL )
            {
                getLabelName(Cmd);
                if ( Labels.find(Cmd) == Labels.end() )
                {
                    Labels[Cmd] = {p->GetNext(), 0}; // label points to the next command
                }
                if ( Cmd == labelName )
                {
                    itrLabel = Labels.find(labelName);
                    break;
                }
            }
            p = p->GetNext();
        }

        Runtime::GetLogger().Activate(true);

        if ( itrLabel == Labels.end() )
        {
            tstr err = _T("- no label found: ");
            err += labelName;
            ScriptError( ET_REPORT, err.c_str() );
            return CMDRESULT_FAILED;
        }
    }
    else
    {
        ScriptContext::tIfState ifState = currentScript.GetIfState();
        if ( ifState.state != IF_NONE || currentScript.GetIfDepth() != 0 )
        {
            // handling GOTO out of IF...ELSE...ENDIF
            const CListItemT<tstr>* pBegin = m_execState.GetCurrentScriptContext().CmdRange.pBegin;
            const CListItemT<tstr>* pGoToLine = itrLabel->second.pLine;
            const CListItemT<tstr>* pLine = m_execState.pScriptLineCurrent;

            for ( ; ; )
            {
                // process the ifState first...
                if ( pLine == ifState.pStartLine )
                {
                    m_execState.GetCurrentScriptContext().PopIfState();
                    ifState = currentScript.GetIfState();
                    if ( ifState.state == IF_NONE && currentScript.GetIfDepth() == 0 )
                        break;
                }
                // ...and only then break if it's time to
                if ( pLine != pBegin && pLine != pGoToLine )
                    pLine = pLine->GetPrev();
                else
                    break;
            }
        }
    }

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    bool bContinue = true;

    ++itrLabel->second.nGoToCounter;
    if ( itrLabel->second.nGoToCounter > m_execState.nGoToMaxCount )
    {
        TCHAR szMsg[240];

        itrLabel->second.nGoToCounter = 0;
        bContinue = false;
        ::wsprintf(szMsg, 
            _T("%s %s was performed more than %d times.\n") \
            _T("Abort execution of this script?\n") \
            _T("(Press Yes to abort or No to continue execution)"),
            DoGoToCommand::Name(),
            labelName.c_str(),
            m_execState.nGoToMaxCount
        );
        if ( ::MessageBox(m_pNppExec->m_nppData._nppHandle, szMsg, 
                _T("NppExec Warning: Possible infinite loop"),
                  MB_YESNO | MB_ICONWARNING) == IDNO )
        {
            bContinue = true;
        }
    }

    if ( bContinue )
    {
        if ( m_execState.pScriptLineNext == NULL )
        {

            Runtime::GetLogger().Add( _T("; Ignoring GOTO as the script is being stopped") );

        }
        else
        {
            m_execState.SetScriptLineNext(itrLabel->second.pLine);

            Runtime::GetLogger().AddEx( _T("; Jumping to command item p = 0x%X { \"%s\" }"),
                m_execState.pScriptLineNext, 
                m_execState.pScriptLineNext ? m_execState.pScriptLineNext->GetItem().c_str() : _T("<NULL>") );

        }
    }
    else
    {
        ScriptError( ET_ABORT, _T("; Script execution aborted by user (from DoGoTo())") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoHelp(const tstr& params)
{
    reportCmdAndParams( DoHelpCommand::Name(), params, fMessageToConsole );

    tstr S = DoHelpCommand::Name();
    if ( !params.IsEmpty() )
    {
        S += _T(' ');
        S += params;
    }

    if ( ConsoleDlg::IsConsoleHelpCommand(S, true) )
        return CMDRESULT_SUCCEEDED;

    S = _T("- unknown parameter: ");
    S += params;
    ScriptError( ET_REPORT, S.c_str() );
    return CMDRESULT_INVALIDPARAM;
}

enum eDecNumberType {
  DNT_NOTNUMBER = 0,
  DNT_INTNUMBER,
  DNT_FLOATNUMBER
};

static eDecNumberType isPureDecNumber(const TCHAR* s)
{
    if ( s )
    {
        if ( (*s == _T('-')) || (*s == _T('+')) )
        {
            ++s;
        }

        if ( !isDecNumChar(*s) )
            return DNT_NOTNUMBER; // we need at least one numeric char

        while ( isDecNumChar( *(++s) ) ) ;
        
        if ( *s == 0 )
            return DNT_INTNUMBER; // OK, we are at the end of string

        // end of floating number's mantissa...
        if ( *s == _T('.') )
        {
            while ( isDecNumChar( *(++s) ) ) ;

            if ( *s == 0 )
                return DNT_FLOATNUMBER; // e.g. "123." or "123.45"
        }

        // check for floating number's exponent...
        if ( (*s == _T('e')) || (*s == _T('E')) )
        {
            ++s;
            if ( (*s == _T('-')) || (*s == _T('+')) )
            {
                ++s;
            }

            while ( isDecNumChar(*s) ) 
            {
                if ( *(++s) == 0 )
                    return DNT_FLOATNUMBER; // OK, we are at the end of string
            }
        }
    }
    return DNT_NOTNUMBER;
}

enum eCondType {
    COND_NONE = 0,
    COND_EQUAL,
    COND_NOTEQUAL,
    COND_LESSTHAN,
    COND_LESSEQUAL,
    COND_GREATTHAN,
    COND_GREATEQUAL,
    COND_EQUALNOCASE
};

template<typename T> class OperandComparator
{
    public:
        OperandComparator(const T& t1, const T& t2) : m_t1(t1), m_t2(t2)
        {
        }

        OperandComparator& operator=(const OperandComparator&) = delete;

        bool compare(int condType)
        {
            bool ret = false;
            switch ( condType )
            {
                case COND_EQUAL:       ret = eq(); break;
                case COND_NOTEQUAL:    ret = ne(); break;
                case COND_LESSTHAN:    ret = lt(); break;
                case COND_LESSEQUAL:   ret = le(); break;
                case COND_GREATTHAN:   ret = gt(); break;
                case COND_GREATEQUAL:  ret = ge(); break;
                case COND_EQUALNOCASE: ret = eq_i(); break;
            }
            return ret;
        }

        bool eq() const { return (m_t1 == m_t2); }
        bool ne() const { return (m_t1 != m_t2); }
        bool lt() const { return (m_t1 < m_t2); }
        bool le() const { return (m_t1 <= m_t2); }
        bool gt() const { return (m_t1 > m_t2); }
        bool ge() const { return (m_t1 >= m_t2); }
        bool eq_i() const { return (m_t1 == m_t2); }

    protected:
        const T& m_t1;
        const T& m_t2;
};

template<> bool OperandComparator<tstr>::eq_i() const
{
    return ( NppExecHelpers::StrCmpNoCase(m_t1, m_t2) == 0 );
}

class CondOperand
{
    public:
        CondOperand(const tstr& s) : m_value_str(s), m_value_dbl(0), m_value_int64(0)
        {
            m_type = isPureDecNumber(s.c_str());
            if ( m_type == DNT_INTNUMBER )
            {
                m_value_int64 = c_base::_tstr2int64(s.c_str());
                m_value_dbl = static_cast<double>(m_value_int64); // to be able to compare as double
            }
            else if ( m_type == DNT_FLOATNUMBER )
            {
                m_value_dbl = _t_str2f(s.c_str());
            }
        }

        const TCHAR* type_as_str() const
        {
            switch ( type() )
            {
                case DNT_NOTNUMBER:
                    return _T("str");
                case DNT_INTNUMBER:
                    return _T("int");
                case DNT_FLOATNUMBER:
                    return _T("dbl");
            }
            return _T("");
        }

        inline eDecNumberType type() const { return m_type; }
        inline const tstr& value_str() const { return m_value_str; }
        inline double value_dbl() const { return m_value_dbl; }
        inline __int64 value_int64() const { return m_value_int64; }

    protected:
        tstr    m_value_str;
        double  m_value_dbl;
        __int64 m_value_int64;
        eDecNumberType m_type;
};

static bool IsConditionTrue(CScriptEngine* pScriptEngine, const tstr& Condition, bool isCalc, bool* pHasSyntaxError)
{
    if ( pHasSyntaxError )  *pHasSyntaxError = false;

    typedef struct sCond {
        const TCHAR* szCond;
        int          nCondLen;
        int          nCondType;
    } tCond;

    static const tCond arrCond[] = {
        { _T("=="), 2, COND_EQUAL       },
        { _T("!="), 2, COND_NOTEQUAL    },
        { _T("<>"), 2, COND_NOTEQUAL    },
        { _T(">="), 2, COND_GREATEQUAL  },
        { _T("<="), 2, COND_LESSEQUAL   },
        { _T("~="), 2, COND_EQUALNOCASE },
        { _T("="),  1, COND_EQUAL       },
        { _T("<"),  1, COND_LESSTHAN    },
        { _T(">"),  1, COND_GREATTHAN   }
    };

    enum eState {
        stNone = 0,
        stGotOp1,
        stGotCond,
        stGotOp2,
        stError
    };

    int    pos = 0;
    int    condType = COND_NONE;
    eState state = stNone;
    tstr   cond;
    tstr   op1;
    tstr   op2;

    while ( NppExecHelpers::IsAnySpaceChar(Condition.GetAt(pos)) )  ++pos;  // skip spaces before op1

    if ( pos != Condition.length() )
    {
        const TCHAR ch = Condition[pos];
        int qt = getQuoteType(ch);
        if ( qt != 0 )
        {
            int pos2 = Condition.Find(ch, pos + 1);
            if ( pos2 != -1 )
            {
                ++pos2;
                op1.Assign(Condition.c_str() + pos, pos2 - pos);
                pos = pos2;  // after op1
                while ( NppExecHelpers::IsAnySpaceChar(Condition.GetAt(pos)) )  ++pos;  // skip spaces after op1
                state = stGotOp1;
            }
            else
                state = stError;
        }
    }
    else
    {
        state = stError;
    }
        
    if ( state != stError )
    {
        if ( state == stGotOp1 )
        {
            const TCHAR* ptr = Condition.c_str() + pos;

            for ( const tCond& c : arrCond )
            {
                if ( StrUnsafeCmpN(ptr, c.szCond, c.nCondLen) == 0 )
                {
                    condType = c.nCondType;
                    cond.Assign(c.szCond, c.nCondLen);
                    break;
                }
            }
        }
        else
        {
            int cond_pos = -1;

            for ( const tCond& c : arrCond )
            {
                int pos2 = Condition.Find(c.szCond, pos);
                if ( pos2 >= 0 && (cond_pos == -1 || pos2 < cond_pos) )
                {
                    cond_pos = pos2;
                    condType = c.nCondType;
                    cond.Assign(c.szCond, c.nCondLen);
                }
            }

            if ( cond_pos != -1 )
            {
                pos = cond_pos;
                op1.Assign(Condition.c_str(), pos);
                NppExecHelpers::StrDelLeadingAnySpaces(op1);
                NppExecHelpers::StrDelTrailingAnySpaces(op1);
                state = stGotOp1;
            }
        }

        if ( condType != COND_NONE )
        {
            state = stGotCond;
            pos += cond.length();  // after cond
            while ( NppExecHelpers::IsAnySpaceChar(Condition.GetAt(pos)) )  ++pos;  // skip spaces after cond

            op2.Assign(Condition.c_str() + pos);
            NppExecHelpers::StrDelLeadingAnySpaces(op2);
            NppExecHelpers::StrDelTrailingAnySpaces(op2);
            state = stGotOp2;
        }
    }

    bool ret = false;

    if ( condType != COND_NONE )
    {
        CNppExec* pNppExec = pScriptEngine->GetNppExec();
        CNppExecMacroVars& MacroVars = pNppExec->GetMacroVars();

        Runtime::GetLogger().Add(   _T("; IF: op1: calling CheckAllMacroVars") );
        MacroVars.CheckAllMacroVars(pScriptEngine, op1, true);

        Runtime::GetLogger().Add(   _T("; IF: op2: calling CheckAllMacroVars") );
        MacroVars.CheckAllMacroVars(pScriptEngine, op2, true);

        Runtime::GetLogger().Add(   _T("IsConditionTrue()") );
        Runtime::GetLogger().Add(   _T("{") );
        Runtime::GetLogger().IncIndentLevel();

        if ( isCalc )
        {
            Runtime::GetLogger().AddEx( _T("op1 :  %s"), op1.c_str() );
            Runtime::GetLogger().AddEx( _T("op2 :  %s"), op2.c_str() );
            Runtime::GetLogger().AddEx( _T("condition :  %s"), cond.c_str() );

            // making fparser-compatible condition...
            switch ( condType )
            {
                case COND_EQUAL:
                case COND_EQUALNOCASE:
                    cond = _T("=");
                    break;
                case COND_NOTEQUAL:
                    cond = _T("!=");
                    break;
            }

            // fparser expression...
            tstr expr = op1;
            expr += cond;
            expr += op2;

            // calculating...
            tstr calcErr, calcResult;
            if ( g_fp.Calculate(pNppExec, expr, calcErr, calcResult) )
            {
                ret = (calcResult == _T("1"));
            }
            else
            {
                if ( pHasSyntaxError )  *pHasSyntaxError = true;
                ret = false;
            }
        }
        else
        {
            // As the original quotes around op1 and op2 are kept (if any),
            // we treat "123" as a string and 123 as a number
            CondOperand co1(op1);
            CondOperand co2(op2);

            Runtime::GetLogger().AddEx( _T("op1 (%s) :  %s"), co1.type_as_str(), op1.c_str() );
            Runtime::GetLogger().AddEx( _T("op2 (%s) :  %s"), co2.type_as_str(), op2.c_str() );
            Runtime::GetLogger().AddEx( _T("condition :  %s"), cond.c_str() );

            if ( (co1.type() == DNT_NOTNUMBER) || (co2.type() == DNT_NOTNUMBER) )
            {
                // compare as string values
                ret = OperandComparator<tstr>(co1.value_str(), co2.value_str()).compare(condType);
            }
            else if ( (co1.type() == DNT_FLOATNUMBER) || (co2.type() == DNT_FLOATNUMBER) )
            {
                // compare as floating-point values
                ret = OperandComparator<double>(co1.value_dbl(), co2.value_dbl()).compare(condType);
            }
            else
            {
                // compare as integer values
                ret = OperandComparator<__int64>(co1.value_int64(), co2.value_int64()).compare(condType);
            }
        }

        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

    }
    else
    {
        if ( pHasSyntaxError )  *pHasSyntaxError = true;
        ret = false;
    }

    return ret;
}

CScriptEngine::eCmdResult CScriptEngine::DoIf(const tstr& params)
{
    return doIf(params, false, false);
}

CScriptEngine::eCmdResult CScriptEngine::DoCalcIf(const tstr& params)
{
    return doIf(params, false, true);
}

CScriptEngine::eIfMode CScriptEngine::getIfMode(const tstr& params, int& n)
{
    tstr paramsUpperCase = params;
    NppExecHelpers::StrUpper(paramsUpperCase);

    static const TCHAR* arrGoTo[] = {
        _T(" GOTO "),
        _T("\tGOTO\t"),
        _T("\tGOTO "),
        _T(" GOTO\t")
    };

    static const TCHAR* arrThen[] = {
        _T(" THEN"),
        _T("\tTHEN")
    };

    int nGoTo = -1;
    for ( const TCHAR* const& i : arrGoTo )
    {
        nGoTo = paramsUpperCase.RFind(i);
        if ( nGoTo >= 0 )
            break;
    }

    if ( nGoTo < 0 )
    {
        if ( paramsUpperCase.EndsWith(_T(" GOTO")) || paramsUpperCase.EndsWith(_T("\tGOTO")) )
        {
            n = -1;
            return EMPTY_GOTO;
        }
    }

    int nThen = -1;
    for ( const TCHAR* const& i : arrThen )
    {
        nThen = paramsUpperCase.RFind(i);
        if ( nThen >= 0 )
            break;
    }

    eIfMode mode = IF_GOTO;

    if ( nThen < 0 )
    {
        if ( nGoTo < 0 )
        {
            n = -1;
            mode = IF_ASSUMING_THEN;
        }
        else
            n = nGoTo;
    }
    else // nThen >= 0
    {
        if ( nGoTo < nThen )
        {
            n = nThen;
            mode = IF_THEN;
        }
        else
            n = nGoTo;
    }

    return mode;
}

CScriptEngine::eCmdResult CScriptEngine::doIf(const tstr& params, bool isElseIf, bool isCalc)
{
    if ( !reportCmdAndParams( isCalc ? DoCalcIfCommand::Name() : DoIfCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    int n = -1;
    eIfMode mode = getIfMode(params, n);

    if ( mode == EMPTY_GOTO )
    {
        ScriptError(ET_UNPREDICTABLE, _T("- Empty goto-label specified."));
        return CMDRESULT_INVALIDPARAM;
    }

    if ( mode == IF_ASSUMING_THEN )
    {
        mode = IF_THEN;
        Runtime::GetLogger().Add(   _T("- no \"goto\" or \"then\" specified, assuming \"then\"") );
    }    

    tstr ifCondition;
    ifCondition.Assign( params.c_str(), n );
    NppExecHelpers::StrDelLeadingAnySpaces(ifCondition);
    NppExecHelpers::StrDelTrailingAnySpaces(ifCondition);

    if ( ifCondition.IsEmpty() )
    {
        ScriptError( ET_UNPREDICTABLE, _T("- Empty if-condition specified.") );
        return CMDRESULT_INVALIDPARAM;
    }

    tstr labelName;

    if ( mode == IF_GOTO )
    {
        labelName.Assign( params.c_str() + n + 6 );
        getLabelName(labelName);

        if ( labelName.IsEmpty() )
        {
            ScriptError( ET_UNPREDICTABLE, _T("- Empty goto-label specified.") );
            return CMDRESULT_INVALIDPARAM;
        }
    }

    bool hasSyntaxError = false;
    bool isConditionOK = IsConditionTrue(this, ifCondition, isCalc, &hasSyntaxError);

    if ( hasSyntaxError )
    {
        ScriptError( ET_UNPREDICTABLE, _T("- Syntax error in the if-condition.") );
        return CMDRESULT_FAILED;
    }

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    if ( mode == IF_GOTO )
    {
        if ( isConditionOK )
        {

            Runtime::GetLogger().Add(   _T("; The condition is true.") );

            DoGoTo(labelName);
        }
        else
        {

            Runtime::GetLogger().Add(   _T("; The condition is false; proceeding to next line") );

            if ( !isElseIf )
                currentScript.PushIfState({IF_MAYBE_ELSE, m_execState.pScriptLineCurrent}); // ELSE may appear after IF ... GOTO
            else
                currentScript.SetIfState(IF_MAYBE_ELSE);
        }
    }
    else // IF_THEN
    {
        if ( isConditionOK )
        {

            Runtime::GetLogger().Add(   _T("; The condition is true; executing lines under the IF") );

            if ( !isElseIf )
                currentScript.PushIfState({IF_EXECUTING, m_execState.pScriptLineCurrent});
            else
                currentScript.SetIfState(IF_EXECUTING);
        }
        else
        {

            Runtime::GetLogger().Add(   _T("; The condition is false; waiting for ELSE or ENDIF") );

            if ( !isElseIf )
                currentScript.PushIfState({IF_WANT_ELSE, m_execState.pScriptLineCurrent});
            else
                currentScript.SetIfState(IF_WANT_ELSE);
        }
    }

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoInputBox(const tstr& params)
{
    if ( !reportCmdAndParams( DoInputBoxCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    runInputBox(this, params);

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoMessageBox(const tstr& params)
{
    if ( !reportCmdAndParams( DoMessageBoxCommand::Name(), params, fMessageToConsole ) )
        return CMDRESULT_INVALIDPARAM;

    return runMessageBox(this, params);
}

CScriptEngine::eCmdResult CScriptEngine::DoLabel(const tstr& params)
{
    if ( !reportCmdAndParams( DoLabelCommand::Name(), params, fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    tstr labelName = params;
    getLabelName(labelName);

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    tLabels& Labels = currentScript.Labels;
    tLabels::const_iterator itrLabel = Labels.find(labelName);
    if ( itrLabel != Labels.end() )
    {
        if ( itrLabel->second.pLine != m_execState.pScriptLineCurrent->GetNext() )
        {
            tstr err = _T("- duplicated label found: ");
            err += labelName;
            ScriptError( ET_REPORT, err.c_str() );
            return CMDRESULT_FAILED;
        }
    }

    CListItemT<tstr>* p = m_execState.pScriptLineCurrent->GetNext();
    Labels[labelName] = {p, 0}; // label points to the next command

    Runtime::GetLogger().AddEx( _T("; label %s -> command item p = 0x%X { \"%s\" }"), 
        labelName.c_str(), p, p ? p->GetItem().c_str() : _T("<NULL>") );

    return CMDRESULT_SUCCEEDED;
}

static void appendOnOff(bool bOn, tstr& S1, tstr& S2)
{
    if ( bOn )
    {
        S1 += _T("+");
        S2 += _T("on");
    }
    else
    {
        S1 += _T("-");
        S2 += _T("off");
    }
}

typedef struct sIntMapping {
    int n;
    const TCHAR* str;
} tIntMapping;

static void appendInt(int n, tstr& S1, tstr& S2, const tIntMapping* mappings)
{
    TCHAR num[16];

    wsprintf( num, _T("%u"), n );
    S1 += num;
    for ( ; mappings->str != NULL; ++mappings )
    {
        if ( mappings->n == n )
        {
            S2 += mappings->str;
            break;
        }
    }
}

static void appendEnc(unsigned int enc_opt, bool bInput, tstr& S1, tstr& S2)
{
    unsigned int enc;
    const TCHAR* encName;
    TCHAR        encNum[5];

    if ( bInput )
    {
        enc = CConsoleEncodingDlg::getInputEncoding(enc_opt);
        encName = CConsoleEncodingDlg::getInputEncodingName(enc_opt);
    }
    else
    {
        enc = CConsoleEncodingDlg::getOutputEncoding(enc_opt);
        encName = CConsoleEncodingDlg::getOutputEncodingName(enc_opt);
    }

    wsprintf( encNum, _T("%u"), enc );

    S1 += encNum;
    S2 += encName;
}

static void appendExecText(int nMode, tstr& S1, tstr& S2)
{
    TCHAR num[16];

    wsprintf( num, _T("%u"), nMode );
    S1 += num;

    tstr sMode;
    if ( nMode == CNppExec::etfNone )
    {
        sMode += _T('0');
    }
    else
    {
        auto addMode = [&sMode](const TCHAR* s)
        {
            if ( !sMode.IsEmpty() )  sMode += _T('+');
            sMode += s;
        };

        if ( nMode & CNppExec::etfMacroVarsNoChildProc )
            addMode( _T("vs") );
        if ( nMode & CNppExec::etfMacroVarsWithChildProc )
            addMode( _T("vp") );
        if ( nMode & CNppExec::etfCollateralNoChildProc )
            addMode( _T("cs") );
        if ( nMode & CNppExec::etfCollateralWithChildProc )
            addMode( _T("cp") );
        if ( nMode & CNppExec::etfNppExecPrefix )
            addMode( _T("ne") );
        if ( nMode & CNppExec::etfLastScript )
            addMode( _T("ls") );
        if ( nMode & CNppExec::etfShareLocalVars )
            addMode( _T("sv") );
    }
    S2 += sMode;
};

CScriptEngine::eCmdResult CScriptEngine::DoNpeCmdAlias(const tstr& params)
{
    reportCmdAndParams( DoNpeCmdAliasCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr aliasName;
    tstr aliasValue;
    tstr S;
    tstr sParams(params);

    bool isLocal = isLocalParam(sParams);

    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());

    CMacroVars& localCmdAliases = m_execState.GetCurrentScriptContext().LocalCmdAliases;
    CMacroVars& globalCmdAliases = m_pNppExec->GetMacroVars().GetCmdAliases();
    CMacroVars* cmdAliases = isLocal ? &localCmdAliases : &globalCmdAliases;

    auto buildAliasString = [](tstr& S, const tstr& name, const tstr& value, bool isLocal)
    {
        S = isLocal ? _T("local ") : _T("");
        S += name;
        S += _T(" -> ");
        S += value;
    };

    if ( sParams.IsEmpty() )
    {

        Runtime::GetLogger().Add(   _T("; no arguments given - showing all command aliases") );

    }
    else
    {
        CStrSplitT<TCHAR> args;
        if ( args.Split( substituteMacroVarsIfNotDelayed(this, sParams, true), _T("="), 2) == 2 )
        {
            // set the value
            aliasName = args.Arg(0);
            aliasValue = args.Arg(1);

            NppExecHelpers::StrDelLeadingAnySpaces(aliasName);
            NppExecHelpers::StrDelTrailingAnySpaces(aliasName);
            NppExecHelpers::StrDelLeadingAnySpaces(aliasValue);
            NppExecHelpers::StrDelTrailingAnySpaces(aliasValue);

            if ( aliasName.IsEmpty() )
            {
                // in case of 'sParams' is a string which starts with "="
                if ( args.Split(aliasValue, _T("="), 2) == 2 )
                {
                    aliasName = _T("=");
                    aliasName += args.Arg(0);
                    aliasValue = args.Arg(1);

                    NppExecHelpers::StrDelTrailingAnySpaces(aliasName);
                    NppExecHelpers::StrDelLeadingAnySpaces(aliasValue);
                }
                else if ( isLocal && !aliasValue.IsEmpty() )
                {
                    isLocal = false;
                    cmdAliases = &globalCmdAliases;
                    aliasName = _T("local");
                }
            }

            if ( aliasName.length() > 0 )
            {
                NppExecHelpers::StrUpper(aliasName);

                CMacroVars::iterator itrAlias = cmdAliases->find(aliasName);
                if ( itrAlias != cmdAliases->end() )
                {
                    if ( aliasValue.length() > 0 )
                    {
                        // update
                        itrAlias->value = aliasValue;

                        buildAliasString(S, aliasName, aliasValue, isLocal);
                    }
                    else
                    {
                        // remove
                        cmdAliases->erase(itrAlias);

                        S = isLocal ? _T("- local command alias has been removed: ") : _T("- command alias has been removed: ");
                        S += aliasName;
                    }

                    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                    m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
                }
                else
                {
                    if ( aliasValue.length() > 0 )
                    {
                        // add
                        (*cmdAliases)[aliasName] = aliasValue;

                        buildAliasString(S, aliasName, aliasValue, isLocal);
                        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
                    }
                    else
                    {
                        // tried to remove non-existent command alias
                        S = isLocal ? _T("- no such local command alias: ") : _T("- no such command alias: ");
                        S += aliasName;
                        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
                        nCmdResult = CMDRESULT_FAILED;
                    }
                }

              #ifdef _DISABLE_CMD_ALIASES
                HMENU hMenu = m_pNppExec->GetNppMainMenu();
                if ( hMenu )
                {
                    const bool bEnable = !cmdAliases.empty();
                    ::EnableMenuItem( hMenu, g_funcItem[N_NOCMDALIASES]._cmdID,
                        MF_BYCOMMAND | (bEnable ? MF_ENABLED : MF_GRAYED) );
                }
              #endif

                return nCmdResult;
            }
            else
            {
                if ( aliasValue.length() > 0 )
                {
                    ScriptError( ET_REPORT, isLocal ? _T("- no local command alias name specified") : _T("- no command alias name specified") );
                    return CMDRESULT_INVALIDPARAM;
                }
                else
                {
                    aliasName = _T("=");
                }
            }
        }
        else
        {
            aliasName = args.Arg(0);
            NppExecHelpers::StrDelLeadingAnySpaces(aliasName);
            NppExecHelpers::StrDelTrailingAnySpaces(aliasName);

            NppExecHelpers::StrUpper(aliasName);
        }
        
    }

    if ( cmdAliases->empty() )
    {
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        m_pNppExec->GetConsole().PrintMessage( isLocal ? _T("- no local command aliases") : _T("- no command aliases"), nMsgFlags );
    }
    else
    {
        if ( aliasName.IsEmpty() )
        {
            for ( const auto& v : localCmdAliases )
            {
                buildAliasString(S, v.name, v.value, true);
                const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
            }

            if ( !isLocal )
            {
                // printing global cmd aliases not overridden by local ones
                CMacroVars::const_iterator localAliasesEnd = localCmdAliases.cend();
                for ( const auto& v : globalCmdAliases )
                {
                    if ( localCmdAliases.find(v.name) == localAliasesEnd )
                    {
                        buildAliasString(S, v.name, v.value, false);
                        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
                    }
                }
            }
        }
        else
        {
            CMacroVars::const_iterator itrAlias = cmdAliases->find(aliasName);
            if ( itrAlias != cmdAliases->end() )
            {
                buildAliasString(S, itrAlias->name, itrAlias->value, isLocal);
            }
            else
            {
                S = isLocal ? _T("- no such local command alias: ") : _T("- no such command alias: ");
                S += aliasName;
                nCmdResult = CMDRESULT_FAILED;
            }

            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
        }
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNpeConsole(const tstr& params)
{
    reportCmdAndParams( DoNpeConsoleCommand::Name(), params, fMessageToConsole );

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    SavedConfiguration& savedConf = currentScript.SavedConf;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr sParams = params;
    bool isLocal = isLocalParam(sParams);
    bool isSilent = false;
    
    if ( sParams.length() > 0 )
    {
        CStrSplitT<TCHAR> args;
        int n = args.SplitToArgs(sParams);
        if ( n > 0 )
        {
            bool isEncChanged = false;
            bool isFilterChanged = false;

            auto checkArgLength = [](const tstr& arg)
            {
                const int argLen = arg.length();
                if ( argLen == 2 )
                    return true;

                if ( argLen == 3 || argLen == 4 )
                {
                    const TCHAR ch = NppExecHelpers::LatinCharUpper(arg[0]);
                    if ( ch == _T('C') || ch == _T('S') )
                        return true;
                }

                return false;
            };
            
            for ( int i = 0; i < n; i++ )
            {
                bool isOK = false;
                tstr arg = args.GetArg(i);
                if ( checkArgLength(arg) )
                {
                    // a+/a-     append mode on/off
                    // d+/d-     follow current directory on/off
                    // e0/e1     ansi escape sequences: raw/remove
                    // u+/u-     pseudoconsole on/off (experimental)
                    // h+/h-     console commands history on/off
                    // m+/m-     console internal messages on/off
                    // p+/p-     print "==== READY ====" on/off
                    // q+/q-     command aliases on/off
                    // v+/v-     set the $(OUTPUT) variable on/off
                    // j+/j-     kill process tree on/off
                    // f+/f-     console output filter on/off
                    // r+/r-     console output replace filter on/off
                    // x+/x-     compiler errors filter on/off
                    // k0..3     catch NppExec's shortcut keys on/off
                    // c<N>      text processing for Execute Clipboard Text
                    // s<N>      text processing for Execute Selected Text
                    // o0/o1/o2  console output encoding: ANSI/OEM/UTF8
                    // i0/i1/i2  console input encoding: ANSI/OEM/UTF8
                    // --        silent (don't print Console mode info)

                    switch ( NppExecHelpers::LatinCharUpper(arg[0]) )
                    {
                        case _T('A'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleAppendMode() )
                                        savedConf.setConsoleAppendMode( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_APPENDMODE) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleAppendMode() )
                                        savedConf.removeConsoleAppendMode();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_APPENDMODE, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('D'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleCdCurDir() )
                                        savedConf.setConsoleCdCurDir( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_CDCURDIR) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleCdCurDir() )
                                        savedConf.removeConsoleCdCurDir();
                                }
                                HMENU hMenu = m_pNppExec->GetNppMainMenu();
                                if ( hMenu )
                                {
                                    ::CheckMenuItem(hMenu, g_funcItem[N_CDCURDIR]._cmdID,
                                        MF_BYCOMMAND | (bOn ? MF_CHECKED : MF_UNCHECKED));
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_CDCURDIR, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('E'):
                            {
                                int nAnsiEscSeq = (int) (arg[1] - _T('0'));
                                if ( (nAnsiEscSeq >= CChildProcess::escKeepRaw) &&
                                     (nAnsiEscSeq < CChildProcess::escTotalCount) )
                                {
                                    if ( isLocal )
                                    {
                                        if ( !savedConf.hasConsoleAnsiEscSeq() )
                                            savedConf.setConsoleAnsiEscSeq( m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_ANSIESCSEQ) );
                                    }
                                    else
                                    {
                                        if ( savedConf.hasConsoleAnsiEscSeq() )
                                            savedConf.removeConsoleAnsiEscSeq();
                                    }
                                    m_pNppExec->GetOptions().SetInt(OPTI_CONSOLE_ANSIESCSEQ, nAnsiEscSeq);
                                    isOK = true;
                                }
                            }
                            break;

                        case _T('U'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConPseudoConsole() )
                                        savedConf.setConPseudoConsole( m_pNppExec->GetOptions().GetBool(OPTB_CHILDP_PSEUDOCONSOLE) );
                                }
                                else
                                {
                                    if ( savedConf.hasConPseudoConsole() )
                                        savedConf.removeConPseudoConsole();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CHILDP_PSEUDOCONSOLE, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('H'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleCmdHistory() )
                                        savedConf.setConsoleCmdHistory( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_CMDHISTORY) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleCmdHistory() )
                                        savedConf.removeConsoleCmdHistory();
                                }
                                HMENU hMenu = m_pNppExec->GetNppMainMenu();
                                if ( hMenu )
                                {
                                    ::CheckMenuItem(hMenu, g_funcItem[N_CMDHISTORY]._cmdID,
                                        MF_BYCOMMAND | (bOn ? MF_CHECKED : MF_UNCHECKED));
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_CMDHISTORY, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('M'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('-') ); // inverse
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleNoIntMsgs() )
                                        savedConf.setConsoleNoIntMsgs( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleNoIntMsgs() )
                                        savedConf.removeConsoleNoIntMsgs();
                                }
                                HMENU hMenu = m_pNppExec->GetNppMainMenu();
                                if ( hMenu )
                                {
                                    ::CheckMenuItem(hMenu, g_funcItem[N_NOINTMSGS]._cmdID,
                                        MF_BYCOMMAND | (bOn ? MF_CHECKED : MF_UNCHECKED));
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_NOINTMSGS, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('P'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsolePrintMsgReady() )
                                        savedConf.setConsolePrintMsgReady( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_PRINTMSGREADY) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsolePrintMsgReady() )
                                        savedConf.removeConsolePrintMsgReady();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_PRINTMSGREADY, bOn);
                                m_execState.GetCurrentScriptContext().IsPrintingMsgReady = bOn ? 1 : 0;
                                isOK = true;
                            }
                            break;

                        case _T('O'):
                            {
                                int enc = (int) (arg[1] - _T('0'));
                                if ( (enc >= CConsoleEncodingDlg::ENC_ANSI) &&
                                     (enc < CConsoleEncodingDlg::ENC_TOTAL) )
                                {
                                    unsigned int enc_opt = m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_ENCODING);
                                    if ( isLocal )
                                    {
                                        if ( !savedConf.hasConsoleEncoding() )
                                            savedConf.setConsoleEncoding( enc_opt );
                                    }
                                    else
                                    {
                                        if ( savedConf.hasConsoleEncoding() )
                                        {
                                            unsigned int nVal = savedConf.getConsoleEncoding();
                                            nVal &= 0xFFF0;
                                            nVal |= enc;
                                            savedConf.setConsoleEncoding(nVal);
                                        }
                                    }
                                    enc_opt &= 0xFFF0;
                                    enc_opt |= enc;
                                    m_pNppExec->GetOptions().SetUint(OPTU_CONSOLE_ENCODING, enc_opt);
                                    isEncChanged = true;
                                    isOK = true;
                                }
                            }
                            break;

                        case _T('I'):
                            {
                                int enc = (int) (arg[1] - _T('0'));
                                if ( (enc >= CConsoleEncodingDlg::ENC_ANSI) &&
                                     (enc < CConsoleEncodingDlg::ENC_TOTAL) )
                                {
                                    unsigned int enc_opt = m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_ENCODING);
                                    if ( isLocal )
                                    {
                                        if ( !savedConf.hasConsoleEncoding() )
                                            savedConf.setConsoleEncoding( enc_opt );
                                    }
                                    else
                                    {
                                        if ( savedConf.hasConsoleEncoding() )
                                        {
                                            unsigned int nVal = savedConf.getConsoleEncoding();
                                            nVal &= 0xFF0F;
                                            nVal |= (enc * 0x10);
                                            savedConf.setConsoleEncoding(nVal);
                                        }
                                    }
                                    enc_opt &= 0xFF0F;
                                    enc_opt |= (enc * 0x10);
                                    m_pNppExec->GetOptions().SetUint(OPTU_CONSOLE_ENCODING, enc_opt);
                                    isEncChanged = true;
                                    isOK = true;
                                }
                            }
                            break;

                        case _T('Q'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('-') ); // inverse
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleNoCmdAliases() )
                                        savedConf.setConsoleNoCmdAliases( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOCMDALIASES) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleNoCmdAliases() )
                                        savedConf.removeConsoleNoCmdAliases();
                                }
                              #ifdef _DISABLE_CMD_ALIASES
                                HMENU hMenu = m_pNppExec->GetNppMainMenu();
                                if ( hMenu )
                                {
                                    ::CheckMenuItem(hMenu, g_funcItem[N_NOCMDALIASES]._cmdID,
                                        MF_BYCOMMAND | (bOn ? MF_CHECKED : MF_UNCHECKED));
                                }
                              #endif
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_NOCMDALIASES, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('V'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleSetOutputVar() )
                                        savedConf.setConsoleSetOutputVar( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SETOUTPUTVAR) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleSetOutputVar() )
                                        savedConf.removeConsoleSetOutputVar();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_SETOUTPUTVAR, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('F'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConFltrEnable() )
                                        savedConf.setConFltrEnable( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE) );
                                }
                                else
                                {
                                    if ( savedConf.hasConFltrEnable() )
                                        savedConf.removeConFltrEnable();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONFLTR_ENABLE, bOn);
                                isFilterChanged = true;
                                isOK = true;
                            }
                            break;

                        case _T('R'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConFltrRplcEnable() )
                                        savedConf.setConFltrRplcEnable( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_ENABLE) );
                                }
                                else
                                {
                                    if ( savedConf.hasConFltrRplcEnable() )
                                        savedConf.removeConFltrRplcEnable();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONFLTR_R_ENABLE, bOn);
                                isFilterChanged = true;
                                isOK = true;
                            }
                            break;

                        case _T('X'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConFltrCompilerErrors() )
                                        savedConf.setConFltrCompilerErrors( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_COMPILER_ERRORS) );
                                }
                                else
                                {
                                    if ( savedConf.hasConFltrCompilerErrors() )
                                        savedConf.removeConFltrCompilerErrors();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONFLTR_COMPILER_ERRORS, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('K'):
                            {
                                unsigned int k = (unsigned int) (arg[1] - _T('0'));
                                if ( (k >= ConsoleDlg::CSK_OFF) &&
                                     (k <= ConsoleDlg::CSK_ALL) )
                                {
                                    if ( isLocal )
                                    {
                                        if ( !savedConf.hasConsoleCatchShortcutKeys() )
                                            savedConf.setConsoleCatchShortcutKeys( m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_CATCHSHORTCUTKEYS) );
                                    }
                                    else
                                    {
                                        if ( savedConf.hasConsoleCatchShortcutKeys() )
                                            savedConf.removeConsoleCatchShortcutKeys();
                                    }
                                    m_pNppExec->GetOptions().SetUint(OPTU_CONSOLE_CATCHSHORTCUTKEYS, k);
                                    isOK = true;
                                }
                            }
                            break;

                        case _T('C'):
                            {
                                int t = c_base::_tstr2int(arg.c_str() + 1);
                                if ( t >= CNppExec::etfNone )
                                {
                                    if ( isLocal )
                                    {
                                        if ( !savedConf.hasExecClipTextMode() )
                                            savedConf.setExecClipTextMode( m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_EXECCLIPTEXTMODE) );
                                    }
                                    else
                                    {
                                        if ( savedConf.hasExecClipTextMode() )
                                            savedConf.removeExecClipTextMode();
                                    }
                                    m_pNppExec->GetOptions().SetInt(OPTI_CONSOLE_EXECCLIPTEXTMODE, t);
                                    isOK = true;
                                }
                            }
                            break;

                        case _T('S'):
                            {
                                int t = c_base::_tstr2int(arg.c_str() + 1);
                                if ( t >= CNppExec::etfNone )
                                {
                                    if ( isLocal )
                                    {
                                        if ( !savedConf.hasExecSelTextMode() )
                                            savedConf.setExecSelTextMode( m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_EXECSELTEXTMODE) );
                                    }
                                    else
                                    {
                                        if ( savedConf.hasExecSelTextMode() )
                                            savedConf.removeExecSelTextMode();
                                    }
                                    m_pNppExec->GetOptions().SetInt(OPTI_CONSOLE_EXECSELTEXTMODE, t);
                                    isOK = true;
                                }
                            }
                            break;

                        case _T('J'):
                            if ( arg[1] == _T('+') || arg[1] == _T('-') )
                            {
                                bool bOn = ( arg[1] == _T('+') );
                                if ( isLocal )
                                {
                                    if ( !savedConf.hasConsoleKillProcTree() )
                                        savedConf.setConsoleKillProcTree( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_KILLPROCTREE) );
                                }
                                else
                                {
                                    if ( savedConf.hasConsoleKillProcTree() )
                                        savedConf.removeConsoleKillProcTree();
                                }
                                m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_KILLPROCTREE, bOn);
                                isOK = true;
                            }
                            break;

                        case _T('-'):
                            if ( arg[1] == _T('-') )
                            {
                                isSilent = true;
                                isOK = true;
                            }
                            break;
                    }
                }
                if ( !isOK )
                {
                    arg.Insert( 0, _T("- unknown parameter: ") );
                    ScriptError( ET_REPORT, arg.c_str() );
                    nCmdResult = CMDRESULT_INVALIDPARAM;
                }
            }

            if ( isEncChanged )
            {
                m_pNppExec->updateConsoleEncodingFlags();
                m_pNppExec->UpdateConsoleEncoding();
            }

            if ( isFilterChanged )
            {
                m_pNppExec->UpdateOutputFilterMenuItem();
            }
        }
    }

    if ( !isSilent )
    {
        tstr S1 = _T("Console mode: ");
        tstr S2 = _T("; ");
        // append
        S1 += _T("a");
        S2 += _T("append: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_APPENDMODE), S1, S2 );
        // cd_curdir
        S1 += _T(" d");
        S2 += _T(", cd_curdir: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_CDCURDIR), S1, S2 );
        // esc_seq
        S1 += _T(" e");
        S2 += _T(", esc_seq: ");
        {
            const tIntMapping escMappings[] = {
                { CChildProcess::escKeepRaw, _T("raw") },
                { CChildProcess::escRemove, _T("remove") },
                { CChildProcess::escProcess, _T("process") },
                { 0x00, NULL } // trailing element with .str=NULL
            };
            appendInt( m_pNppExec->GetCommandExecutor().GetChildProcessAnsiEscSeq(), S1, S2, escMappings );
        }
        // pseudocon
        S1 += _T(" u");
        S2 += _T(", pseudocon: ");
        appendOnOff( m_pNppExec->GetCommandExecutor().IsChildProcessPseudoCon(), S1, S2 );
        // cmd_history
        S1 += _T(" h");
        S2 += _T_RE_EOL _T("; cmd_history: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_CMDHISTORY), S1, S2 );
        // int_msgs
        S1 += _T(" m");
        S2 += _T(", int_msgs: ");
        appendOnOff( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOINTMSGS), S1, S2 );
        // print_ready
        S1 += _T(" p");
        S2 += _T(", print_ready: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_PRINTMSGREADY), S1, S2 );
        // cmd_aliases
        S1 += _T(" q");
        S2 += _T_RE_EOL _T("; cmd_aliases: ");
        appendOnOff( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOCMDALIASES), S1, S2 );
        // $(OUTPUT)
        S1 += _T(" v");
        S2 += _T(", output_var: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_SETOUTPUTVAR), S1, S2 );
        S1 += _T(" j");
        S2 += _T(", kill_tree: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_KILLPROCTREE), S1, S2 );
        // filters
        S1 += _T(" f");
        S2 += _T_RE_EOL _T("; filter: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_ENABLE), S1, S2 );
        S1 += _T(" r");
        S2 += _T(", replace_filter: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_R_ENABLE), S1, S2 );
        // compiler errors:
        S1 += _T(" x");
        S2 += _T(", compiler_errors: ");
        appendOnOff( m_pNppExec->GetOptions().GetBool(OPTB_CONFLTR_COMPILER_ERRORS), S1, S2 );
        // out_enc
        unsigned int enc_opt = m_pNppExec->GetCommandExecutor().GetChildProcessEncoding();
        S1 += _T(" o");
        S2 += _T_RE_EOL _T("; out_enc: ");
        appendEnc( enc_opt, false, S1, S2 );
        // in_enc
        S1 += _T(" i");
        S2 += _T(", in_enc: ");
        appendEnc( enc_opt, true, S1, S2 );
        // shortcut_keys
        S1 += _T(" k");
        S2 += _T(", shortcut_keys: ");
        {
            const tIntMapping skMappings[] = {
                { ConsoleDlg::CSK_OFF, _T("off") },
                { ConsoleDlg::CSK_STD, _T("std") },
                { ConsoleDlg::CSK_USR, _T("usr") },
                { ConsoleDlg::CSK_ALL, _T("std+usr") },
                { 0x00, NULL } // trailing element with .str=NULL
            };
            appendInt( m_pNppExec->GetOptions().GetUint(OPTU_CONSOLE_CATCHSHORTCUTKEYS), S1, S2, skMappings );
        }
        // exec text mode
        S1 += _T(" c");
        S2 += _T_RE_EOL _T("; exec_clip: ");
        appendExecText( m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_EXECCLIPTEXTMODE), S1, S2 );
        S1 += _T(" s");
        S2 += _T(", exec_sel: ");
        appendExecText( m_pNppExec->GetOptions().GetInt(OPTI_CONSOLE_EXECSELTEXTMODE), S1, S2 );
        // finally...
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        m_pNppExec->GetConsole().PrintMessage( S1.c_str(), nMsgFlags );
        m_pNppExec->GetConsole().PrintMessage( S2.c_str(), nMsgFlags );
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNpeDebugLog(const tstr& params)
{
    reportCmdAndParams( DoNpeDebugLogCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr sParams = params;
    bool isLocal = isLocalParam(sParams);
    int nParam = getOnOffParam(sParams);
    if (nParam == PARAM_ON || nParam == PARAM_OFF)
    {
        ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
        SavedConfiguration& savedConf = currentScript.SavedConf;
        if (isLocal)
        {
            if ( !savedConf.hasConsoleDebugLog() )
                savedConf.setConsoleDebugLog( m_pNppExec->GetOptions().GetBool(OPTB_NPE_DEBUGLOG) );
            if ( !savedConf.hasLoggerOutputMode() )
                savedConf.setLoggerOutputMode( Runtime::GetLogger().IsOutputMode() );
        }
        else
        {
            if ( savedConf.hasConsoleDebugLog() )
                savedConf.removeConsoleDebugLog();
            if ( savedConf.hasLoggerOutputMode() )
                savedConf.removeLoggerOutputMode();
        }
        if (nParam == PARAM_ON)
        {
            m_pNppExec->GetOptions().SetBool(OPTB_NPE_DEBUGLOG, true);
            Runtime::GetLogger().SetOutputMode(true, CNppExec::printScriptString);
        }
        else // nParam == PARAM_OFF
        {
            m_pNppExec->GetOptions().SetBool(OPTB_NPE_DEBUGLOG, false);
            Runtime::GetLogger().SetOutputMode(false);
        }
    }
    else if (nParam != PARAM_EMPTY)
    {
        tstr Err = _T("- unknown parameter: ");
        Err += params;
        ScriptError( ET_REPORT, Err.c_str() );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
    m_pNppExec->GetConsole().PrintMessage( m_pNppExec->GetOptions().GetBool(OPTB_NPE_DEBUGLOG) ? 
        _T("Debug Log is On (1)") : _T("Debug Log is Off (0)"), nMsgFlags );

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNpeNoEmptyVars(const tstr& params)
{
    reportCmdAndParams( DoNpeNoEmptyVarsCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr sParams = params;
    bool isLocal = isLocalParam(sParams);
    int nParam = getOnOffParam(sParams);
    if (nParam == PARAM_ON || nParam == PARAM_OFF)
    {
        bool bOnOff = (nParam == PARAM_ON) ? true : false;
        ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
        SavedConfiguration& savedConf = currentScript.SavedConf;
        if (isLocal)
        {
            if ( !savedConf.hasConsoleNoEmptyVars() )
                savedConf.setConsoleNoEmptyVars( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOEMPTYVARS) );
        }
        else
        {
            if ( savedConf.hasConsoleNoEmptyVars() )
                savedConf.removeConsoleNoEmptyVars();
        }
        m_pNppExec->GetOptions().SetBool(OPTB_CONSOLE_NOEMPTYVARS, bOnOff);
    }
    else if (nParam != PARAM_EMPTY)
    {
        tstr Err = _T("- unknown parameter: ");
        Err += params;
        ScriptError( ET_REPORT, Err.c_str() );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
    m_pNppExec->GetConsole().PrintMessage( m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOEMPTYVARS) ? 
          _T("Replace empty (uninitialized) vars with empty str:  On (1)") : 
            _T("Replace empty (uninitialized) vars with empty str:  Off (0)"), nMsgFlags );

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNpeQueue(const tstr& params)
{
    if ( !reportCmdAndParams( DoNpeQueueCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // command to be queued
    bool bUseSeparateScript = true;
    tstr Cmd = getQueuedCommand(params, bUseSeparateScript);
    m_pNppExec->GetMacroVars().CheckCmdAliases(this, Cmd, true);
    Cmd = substituteMacroVarsIfNotDelayed(this, Cmd, true);

    if ( bUseSeparateScript && (m_nRunFlags & rfExitScript) == 0 )
    {
        // queuing, similar to NppExecPluginInterface's npemExecuteQueuedScript
        CListT<tstr> CmdList(Cmd);
        CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::DoRunScriptCommand(tstr(), CmdList, 0, CNppExecCommandExecutor::ExpirableCommand::NonExpirable);
        m_pNppExec->GetCommandExecutor().ExecuteCommand(pCommand);
    }
    else
    {
        // no queuing, using the very same script
        m_CmdList.Add(Cmd); // will be executed as the last command of m_CmdList

        //m_execState.pScriptLineCurrent->SetItem(tstr()); <-- Don't do this!
        // If this command is inside a goto-loop, then on the next iteration
        // of the loop this command would be empty!
    }

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoNpeSendMsgBufLen(const tstr& params)
{
    reportCmdAndParams( DoNpeSendMsgBufLenCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    int nBufLen = 0;

    tstr sParams = params;
    bool isLocal = isLocalParam(sParams);

    if ( sParams.IsEmpty() )
    {
        nBufLen = m_pNppExec->GetOptions().GetInt(OPTI_SENDMSG_MAXBUFLEN);
    }
    else
    {
        nBufLen = c_base::_tstr2int(sParams.c_str());
        if ( nBufLen > 0 )
        {
            ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
            SavedConfiguration& savedConf = currentScript.SavedConf;
            if ( isLocal )
            {
                if ( !savedConf.hasSendMsgBufLen() )
                    savedConf.setSendMsgBufLen( m_pNppExec->GetOptions().GetInt(OPTI_SENDMSG_MAXBUFLEN) );
            }
            else
            {
                if ( savedConf.hasSendMsgBufLen() )
                    savedConf.removeSendMsgBufLen();
            }

            if ( nBufLen < 0x10000 )
            {
                nBufLen = 0x10000;

                Runtime::GetLogger().AddEx( _T("; BufLen adjusted to %d"), nBufLen );
            }

            m_pNppExec->GetOptions().SetInt(OPTI_SENDMSG_MAXBUFLEN, nBufLen);
        }
        else if ( nBufLen == 0 )
        {
            ScriptError( ET_REPORT, _T("- must be a non-zero integer value") );
            nCmdResult = CMDRESULT_INVALIDPARAM;
        }
        else if ( nBufLen < 0 )
        {
            ScriptError( ET_REPORT, _T("- must be a non-negative integer value") );
            nCmdResult = CMDRESULT_INVALIDPARAM;
        }
    }

    if ( nCmdResult == CMDRESULT_SUCCEEDED )
    {
        TCHAR szMsg[64];
        wsprintf(szMsg, _T("SendMsgBufLen = %d"), nBufLen);
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        m_pNppExec->GetConsole().PrintMessage(szMsg, nMsgFlags);
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppClose(const tstr& params)
{
    TCHAR szFileName[FILEPATH_BUFSIZE];

    reportCmdAndParams( DoNppCloseCommand::Name(), params, 0 );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    bool bCurrentFile = params.IsEmpty();
    if ( bCurrentFile )
    {
        Runtime::GetLogger().Add(   _T("; no file name specified - closing current file") );

        // file name is not specified i.e. closing current file
        m_pNppExec->SendNppMsg(NPPM_GETFULLCURRENTPATH,
          (WPARAM) (FILEPATH_BUFSIZE - 1), (LPARAM) szFileName);
    }
    else
    {
        Runtime::GetLogger().Add(   _T("; retrieving full file name") );
    }

    messageConsole( DoNppCloseCommand::Name(), bCurrentFile ? szFileName : params.c_str() );

    if ( bCurrentFile || m_pNppExec->nppSwitchToDocument(params.c_str(), true) )
    {
        m_pNppExec->SendNppMsg(NPPM_MENUCOMMAND, 0, IDM_FILE_CLOSE);
    }
    else
    {
        ScriptError( ET_REPORT, _T("- no such file opened") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppConsole(const tstr& params)
{
    reportCmdAndParams( DoNppConsoleCommand::Name(), params, 0 );

    ScriptContext& currentScript = m_execState.GetCurrentScriptContext();
    SavedConfiguration& savedConf = currentScript.SavedConf;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    tstr sParams = params;
    bool isLocal = isLocalParam(sParams);

    switch ( getOnOffParam(sParams) )
    {
        case PARAM_EMPTY:
            errorCmdNoParam(DoNppConsoleCommand::Name());
            nCmdResult = CMDRESULT_INVALIDPARAM;
            break;

        case PARAM_ON:
            messageConsole( DoNppConsoleCommand::Name(), _T("On") );
            if ( isLocal )
            {
                if ( !m_pNppExec->isConsoleDialogVisible() )
                {
                    if ( !savedConf.hasConsoleDialogVisible() )
                        savedConf.setConsoleDialogVisible(false);
                }
            }
            else
            {
                if ( savedConf.hasConsoleDialogVisible() )
                    savedConf.removeConsoleDialogVisible();
            }
            m_pNppExec->showConsoleDialog(CNppExec::showIfHidden, CNppExec::scfCmdNppConsole);
            break;

        case PARAM_OFF:
            messageConsole( DoNppConsoleCommand::Name(), _T("Off") );
            //m_pNppExec->verifyConsoleDialogExists();
            if ( m_pNppExec->isConsoleDialogVisible() )
            {
                if ( isLocal )
                {
                    if ( !savedConf.hasConsoleDialogVisible() )
                        savedConf.setConsoleDialogVisible(true);
                }
                else
                {
                    if ( savedConf.hasConsoleDialogVisible() )
                        savedConf.removeConsoleDialogVisible();
                }
                m_isClosingConsole = true;
                m_pNppExec->showConsoleDialog(CNppExec::hideIfShown, CNppExec::scfCmdNppConsole);
                m_isClosingConsole = false;
            }
            m_pNppExec->_consoleIsVisible = true;
            updateFocus();
            break;

        case PARAM_KEEP:
            messageConsole( DoNppConsoleCommand::Name(), _T("?") );
            //m_pNppExec->verifyConsoleDialogExists();
            if ( !m_pNppExec->isConsoleDialogVisible() )
            {
                m_pNppExec->_consoleIsVisible = true;
                updateFocus();
            }
            break;

        case PARAM_ENABLE:
            messageConsole( DoNppConsoleCommand::Name(), _T("+") );
            if ( isLocal )
            {
                if ( !savedConf.hasConsoleIsOutputEnabled() )
                {
                    if ( !m_pNppExec->GetConsole().IsOutputEnabled() )
                        savedConf.setConsoleIsOutputEnabled(false);
                }
            }
            else
            {
                if ( savedConf.hasConsoleIsOutputEnabled() )
                    savedConf.removeConsoleIsOutputEnabled();
            }
            m_pNppExec->GetConsole().SetOutputEnabled(true);
            m_pNppExec->GetConsole().LockConsoleEndPos();
            break;

        case PARAM_DISABLE:
            // messageConsole( DoNppConsoleCommand::Name(), _T("-") );  --  don't output anything
            if ( isLocal )
            {
                if ( !savedConf.hasConsoleIsOutputEnabled() )
                {
                    if ( m_pNppExec->GetConsole().IsOutputEnabled() )
                        savedConf.setConsoleIsOutputEnabled(true);
                }
            }
            else
            {
                if ( savedConf.hasConsoleIsOutputEnabled() )
                    savedConf.removeConsoleIsOutputEnabled();
            }
            m_pNppExec->GetConsole().LockConsoleEndPos();
            m_pNppExec->GetConsole().SetOutputEnabled(false);
            break;

        default:
            // show error
            m_pNppExec->showConsoleDialog(CNppExec::showIfHidden, CNppExec::scfCmdNppConsole);
            {
                tstr Err = _T("- unknown parameter: ");
                Err += params;
                ScriptError( ET_REPORT, Err.c_str() );
                nCmdResult = CMDRESULT_INVALIDPARAM;
            }
            break;
    }

    Runtime::GetLogger().AddEx( _T("; the Console is %s"), m_pNppExec->isConsoleDialogVisible() ? _T("On") : _T("Off") );

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppExec(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppExecCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // inserting commands from a script or a file into m_pNppExec->m_CmdList

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    CStrSplitT<TCHAR> args;
    args.SplitToArgs(params);

    Runtime::GetLogger().Add(   _T("; args") );
    Runtime::GetLogger().Add(   _T("; {") );
    const tstr logIndent = Runtime::GetLogger().GetIndentStr();
    for (int i = 0; i < args.GetArgCount(); i++)
    {
        Runtime::GetLogger().AddEx( _T("; %s[%d], \"%s\""), logIndent.c_str(), i, args.GetArg(i).c_str() );
    }
    Runtime::GetLogger().Add(   _T("; }") );

    tstr scriptName = args.GetArg(0);
    NppExecHelpers::StrUpper(scriptName);
    if (!m_execState.GetScriptContextItemPtr(scriptName))
    {        
        bool bContinue = true;

        ++m_execState.nExecCounter;
        if (m_execState.nExecCounter > m_execState.nExecMaxCount)
        {
            TCHAR szMsg[240];

            m_execState.nExecCounter = 0;
            bContinue = false;
            ::wsprintf(szMsg, 
                _T("%s was performed more than %d times.\n") \
                _T("Abort execution of this script?\n") \
                _T("(Press Yes to abort or No to continue execution)"),
                DoNppExecCommand::Name(),
                m_execState.nExecMaxCount
            );
            if (::MessageBox(m_pNppExec->m_nppData._nppHandle, szMsg, 
                    _T("NppExec Warning: Possible infinite loop"),
                      MB_YESNO | MB_ICONWARNING) == IDNO)
            {
                bContinue = true;
            }
        }

        if (bContinue)
        {  
            CFileBufT<TCHAR> fbuf;
            CNppScript       Script;
            tstr             line;
            ScriptContext    scriptContext;

            if (m_pNppExec->m_ScriptsList.GetScript(args.GetArg(0), Script))
            {
                Runtime::GetLogger().AddEx( _T("; adding commands from the script \"%s\""), args.GetArg(0).c_str() );  
                
                scriptContext.ScriptName = scriptName;
                scriptContext.CmdRange.pBegin = m_execState.pScriptLineCurrent;
                scriptContext.CmdRange.pEnd = m_execState.pScriptLineCurrent->GetNext();
                scriptContext.Args = args;
                scriptContext.IsNppExeced = true;

                int n = 0;
                CListItemT<tstr>* pline = m_execState.pScriptLineCurrent;
                CListItemT<tstr>* pscriptline = Script.GetCmdList().GetFirst();
                while (pscriptline)
                {
                    line = pscriptline->GetItem();
                    if (line.length() > 0)
                    {
                        ++n;

                        Runtime::GetLogger().AddEx( _T("; + line %d:  %s"), n, line.c_str() );
                    
                        pline = m_CmdList.Insert(pline, true, line);
                    }
                    pscriptline = pscriptline->GetNext();
                }

                if (n != 0)
                {
                    scriptContext.CmdRange.pBegin = scriptContext.CmdRange.pBegin->GetNext();
                    m_execState.ScriptContextList.Add(scriptContext);

                    Runtime::GetLogger().AddEx( _T("; script context added: { Name = \"%s\"; CmdRange = [0x%X, 0x%X) }"), 
                        scriptContext.ScriptName.c_str(), scriptContext.CmdRange.pBegin, scriptContext.CmdRange.pEnd ); 

                }
            }
            else 
            {
                Runtime::GetLogger().AddEx( _T("; there is no script with such name (\"%s\")"), args.GetArg(0).c_str() ); 
                Runtime::GetLogger().Add(   _T("; trying to load the script from a file") ); 
              
                tstr fileName = args.GetArg(0);

                if ( fileName.EndsWith(_T(".exe")) ||
                     fileName.EndsWith(_T(".com")) ||
                     fileName.EndsWith(_T(".bat")) ||
                     fileName.EndsWith(_T(".cmd")) ||
                     fileName.EndsWith(_T(".ps1")) ||
                     fileName.EndsWith(_T(".js"))  ||
                     fileName.EndsWith(_T(".vbs")) )
                {
                    // executable or command file
                    ScriptError( ET_REPORT, _T("- syntax error: NppExec\'s script file expected, executable file given") );
                    m_pNppExec->GetConsole().PrintError( _T("; If you want to run an executable file, don\'t use NPP_EXEC command!") );
                    m_pNppExec->GetConsole().PrintError( _T("; (type HELP NPP_EXEC in the Console window for details)") );
                    m_pNppExec->GetConsole().PrintError( _T("; To run program.exe type just") );
                    m_pNppExec->GetConsole().PrintError( _T(";     program.exe") );
                    m_pNppExec->GetConsole().PrintError( _T("; instead of") );
                    m_pNppExec->GetConsole().PrintError( _T(";     npp_exec program.exe") );
                    nCmdResult = CMDRESULT_FAILED;
                }
                else
                {
                    // not executable or command file

                    if ( fileName.length() > 1 )
                    {
                        m_pNppExec->nppConvertToFullPathName(fileName);
                    }

                    if ( fbuf.LoadFromFile(fileName.c_str(), true, m_pNppExec->GetOptions().GetInt(OPTI_UTF8_DETECT_LENGTH)) )
                    {
                        Runtime::GetLogger().AddEx( _T("; loading the script from a file \"%s\""), fileName.c_str() );
                  
                        scriptContext.ScriptName = scriptName;
                        scriptContext.CmdRange.pBegin = m_execState.pScriptLineCurrent;
                        scriptContext.CmdRange.pEnd = m_execState.pScriptLineCurrent->GetNext();
                        scriptContext.Args = args;
                        scriptContext.IsNppExeced = true;

                        int n = 0;
                        CListItemT<tstr>* pline = m_execState.pScriptLineCurrent;
                        while (fbuf.GetLine(line) >= 0)
                        {
                            if (line.length() > 0)
                            {
                                ++n;
                    
                                Runtime::GetLogger().AddEx( _T("; + line %d:  %s"), n, line.c_str() );
                      
                                pline = m_CmdList.Insert(pline, true, line);
                            }
                        }

                        if (n != 0)
                        {
                            scriptContext.CmdRange.pBegin = scriptContext.CmdRange.pBegin->GetNext();
                            m_execState.ScriptContextList.Add(scriptContext);

                            Runtime::GetLogger().AddEx( _T("; script context added: { Name = \"%s\"; CmdRange = [0x%X, 0x%X) }"), 
                                scriptContext.ScriptName.c_str(), scriptContext.CmdRange.pBegin, scriptContext.CmdRange.pEnd ); 

                        }
                    }
                    else
                    {
                        Runtime::GetLogger().Add(   _T("; there is no file with such name ") ); 
                  
                        ScriptError( ET_REPORT, _T("- can not open specified file or it is empty") );
                        nCmdResult = CMDRESULT_FAILED;
                    }
                }
                
            }
        }
        else
        {
            ScriptError( ET_ABORT, _T("; Script execution aborted to prevent possible infinite loop (from DoNppExec())") );
            nCmdResult = CMDRESULT_FAILED;
        }

    }
    else
    {
        Runtime::GetLogger().Add(   _T("; the script with the same name is being executed") );
            
        ScriptError( ET_REPORT, _T("- can\'t exec the same script at the same time") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppExecText(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppExecTextCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    const TCHAR ch = params.GetFirstChar();
    if ( ch < _T('0') || ch > _T('9') )
    {
        ScriptError( ET_REPORT, _T("- the first parameter must be a number!") );
        return CMDRESULT_INVALIDPARAM;
    }

    // executing the given text as NppExec's script
    // or sending the text to the running child process as an input

    unsigned int nExecTextMode = c_base::_tstr2uint(params.c_str());

    int n = params.FindOneOf(_T(" \t\v\f"));
    if ( n == -1 )
        n = params.length();

    const bool isChildProcess = IsChildProcessRunning();

    tstr sProcessedText;
    const TCHAR* pszText = params.c_str() + n;
    while ( NppExecHelpers::IsAnySpaceChar(*pszText) )  ++pszText;
    if ( ((nExecTextMode & CNppExec::etfMacroVarsWithChildProc) != 0 && isChildProcess) ||
         ((nExecTextMode & CNppExec::etfMacroVarsNoChildProc) != 0 && !isChildProcess) )
    {
        sProcessedText = pszText;
        m_pNppExec->GetMacroVars().CheckAllMacroVars(this, sProcessedText, true);
        pszText = sProcessedText.c_str();
    }

    const unsigned int nCmdFlags = isChildProcess ? (acfKeepLineEndings | acfAddEmptyLines) : 0;
    tCmdList CmdList = getCmdListFromText(pszText, nCmdFlags);

    bool isCollateral = false;
    if ( ((nExecTextMode & CNppExec::etfCollateralWithChildProc) != 0 && isChildProcess) ||
         ((nExecTextMode & CNppExec::etfCollateralNoChildProc) != 0 && !isChildProcess) )
    {
        isCollateral = isScriptCollateral(m_pNppExec, CmdList);
    }

    CNppExecCommandExecutor& CommandExecutor = m_pNppExec->GetCommandExecutor();

    if ( isCollateral || !isChildProcess )
    {
        m_execState.nExecTextCounter = ++nExecTextEnginesCount;
        if ( m_execState.nExecTextCounter > m_execState.nExecTextMaxCount )
        {
            TCHAR szMsg[240];

            nExecTextEnginesCount = 0;
            ::wsprintf(szMsg, 
                _T("%s was performed more than %d times.\n") \
                _T("Abort execution of this script?\n") \
                _T("(Press Yes to abort or No to continue execution)"),
                DoNppExecTextCommand::Name(),
                m_execState.nExecTextMaxCount
            );
            if (::MessageBox(m_pNppExec->m_nppData._nppHandle, szMsg, 
                _T("NppExec Warning: Possible infinite loop"),
                MB_YESNO | MB_ICONWARNING) != IDNO)
            {
                ScriptError( ET_ABORT, _T("; Script execution aborted to prevent possible infinite loop (from DoNppExecText())") );
                return CMDRESULT_FAILED;
            }
        }
    }

    unsigned int nRunFlags = 0;
    if ( nExecTextMode & CNppExec::etfShareLocalVars )
    {
        nRunFlags |= (rfShareLocalVars | rfShareConsoleLocalVars);
    }

    bool bCreateScriptContextFromCmdList = false;

    if ( isCollateral )
    {
        Runtime::GetLogger().Add( _T("; running a collateral script") );

        if ( nCmdFlags & acfKeepLineEndings )
        {
            removeLineEndings(CmdList);
        }
        CommandExecutor.ExecuteCollateralScript(CmdList, tstr(), nRunFlags | rfCollateralScript);
    }
    else if ( isChildProcess )
    {
        if ( nExecTextMode & CNppExec::etfNppExecPrefix )
        {
            tCmdList CollateralCmdList = getCollateralCmdListForChildProcess(m_pNppExec, CmdList);
            if ( !CollateralCmdList.IsEmpty() )
            {
                CollateralCmdList.Swap(CmdList);
                bCreateScriptContextFromCmdList = true;
            }
        }

        if ( !bCreateScriptContextFromCmdList )
        {
            Runtime::GetLogger().Add( _T("; sending the text to the running child process") );

            CommandExecutor.WriteChildProcessInput( pszText );
            CommandExecutor.WriteChildProcessInputNewLine();
        }
    }
    else
    {
        bCreateScriptContextFromCmdList = true;
    }

    if ( bCreateScriptContextFromCmdList )
    {
        Runtime::GetLogger().Add( _T("; adding script commands") );

        if ( !CmdList.IsEmpty() )
        {
            ScriptContext scriptContext;

            scriptContext.ScriptName = _T("");
            scriptContext.CmdRange.pBegin = m_execState.pScriptLineCurrent;
            scriptContext.CmdRange.pEnd = m_execState.pScriptLineCurrent->GetNext();
            scriptContext.Args = m_execState.GetCurrentScriptContext().Args;
            scriptContext.IsNppExeced = true;
            if ( nRunFlags & rfShareLocalVars )
            {
                scriptContext.IsSharingLocalVars = true;
                {
                    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
                    scriptContext.LocalMacroVars = m_execState.GetCurrentScriptContext().LocalMacroVars;
                }
                {
                    CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsCmdAliases());
                    scriptContext.LocalCmdAliases = m_execState.GetCurrentScriptContext().LocalCmdAliases;
                }
            }

            int nLines = 0;
            CListItemT<tstr>* pline = m_execState.pScriptLineCurrent;
            for ( CListItemT<tstr>* p = CmdList.GetFirst(); p != NULL; p = p->GetNext() )
            {
                const tstr& line = p->GetItem();
                if (line.length() > 0)
                {
                    ++nLines;

                    Runtime::GetLogger().AddEx( _T("; + line %d:  %s"), nLines, line.c_str() );

                    pline = m_CmdList.Insert(pline, true, line);
                }
            }

            if (nLines != 0)
            {
                scriptContext.CmdRange.pBegin = scriptContext.CmdRange.pBegin->GetNext();
                m_execState.ScriptContextList.Add(scriptContext);

                Runtime::GetLogger().AddEx( _T("; script context added: { Name = \"%s\"; CmdRange = [0x%X, 0x%X) }"), 
                    scriptContext.ScriptName.c_str(), scriptContext.CmdRange.pBegin, scriptContext.CmdRange.pEnd ); 

            }
        }
    }

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppMenuCommand(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppMenuCommandCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr parsedItemPath, parsedSep;
    int menuItemId = m_pNppExec->nppGetMenuItemIdByName(params, parsedItemPath, parsedSep);
    if ( menuItemId > 0 )
    {
        Runtime::GetLogger().AddEx( _T("; menu item separator: %s"), 
            parsedSep.IsEmpty() ? _T("<none>") : parsedSep.c_str() );
        Runtime::GetLogger().AddEx( _T("; menu item found: id = %d"), menuItemId );

        m_pNppExec->SendNppMsg(NPPM_MENUCOMMAND, 0, menuItemId);
    }
    else
    {
        tstr err = _T("- the menu item was not found. Parsed item path: ");
        if ( !parsedItemPath.IsEmpty() )
            err += parsedItemPath;
        else
            err += _T("<none>");

        Runtime::GetLogger().AddEx( _T("; menu item separator: %s"), 
            parsedSep.IsEmpty() ? _T("<none>") : parsedSep.c_str() );

        ScriptError( ET_REPORT, err.c_str() );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppOpen(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppOpenCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // opening a file in Notepad++

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    int nFilterPos = FileFilterPos( params.c_str() );
    if ( nFilterPos < 0 )
    {
        Runtime::GetLogger().Add(   _T("; direct file (path)name specified") );
        
        // direct file (path)name
        if ( !m_pNppExec->SendNppMsg(NPPM_RELOADFILE, (WPARAM) FALSE, (LPARAM) params.c_str()) )
        {
            int nFiles = (int) m_pNppExec->SendNppMsg(NPPM_GETNBOPENFILES, 0, 0);

            if ( !m_pNppExec->SendNppMsg(NPPM_DOOPEN, (WPARAM) 0, (LPARAM) params.c_str()) )
            {
                nCmdResult = CMDRESULT_FAILED;

                DWORD dwAttr = ::GetFileAttributes(params.c_str());
                if ( (dwAttr != INVALID_FILE_ATTRIBUTES) &&
                     ((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) )
                {
                    // NPPM_DOOPEN unexpectedly returns 0 when a folder is specified
                    // as its argument, though in fact it does open all the files
                    // from that folder.
                    int nFilesNow = (int) m_pNppExec->SendNppMsg(NPPM_GETNBOPENFILES, 0, 0);
                    if ( nFilesNow != nFiles )
                    {
                        nCmdResult = CMDRESULT_SUCCEEDED;
                    }
                }
            }
        }
    }
    else
    {
          
        Runtime::GetLogger().Add(   _T("; file (path)name mask specified") );
            
        // file (path)name mask (e.g. "C:\Documents\*.txt")
        tstr         Path;
        tstr         Filter;
        CListT<tstr> FilesList;

        GetPathAndFilter( params.c_str(), nFilterPos, Path, Filter );

        Runtime::GetLogger().AddEx( _T("; searching \"%s\" for \"%s\""), Path.c_str(), Filter.c_str() );
          
        GetFilePathNamesList( Path.c_str(), Filter.c_str(), FilesList );

        tstr S;
        CListItemT<tstr>* ptr = FilesList.GetFirst();
        while ( ptr )
        {
            S = ptr->GetItem();
            if ( !m_pNppExec->SendNppMsg(NPPM_RELOADFILE, (WPARAM) FALSE, (LPARAM) S.c_str()) )
            {
                if ( !m_pNppExec->SendNppMsg(NPPM_DOOPEN, (WPARAM) 0, (LPARAM) S.c_str()) )
                    nCmdResult = CMDRESULT_FAILED;
            }
            ptr = ptr->GetNext();
        }
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppRun(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppRunCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // run a command

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    tstr arg1, arg2;
    if ( SearchPath(NULL, params.c_str(), NULL, 0, NULL, NULL) != 0 )
    {
        arg1 = params;
    }
    else
    {
        CStrSplitT<TCHAR> args;
        args.SplitToArgs(params, 2);
        arg1 = args.Arg(0);
        arg2 = args.Arg(1);
    }

    Runtime::GetLogger().Add(   _T("; ShellExecute() arguments") ); 
    Runtime::GetLogger().Add(   _T("; {") );
    const tstr logIndent = Runtime::GetLogger().GetIndentStr();
    Runtime::GetLogger().AddEx( _T("; %s[file]   %s"), logIndent.c_str(), arg1.c_str() );
    Runtime::GetLogger().AddEx( _T("; %s[params] %s"), logIndent.c_str(), arg2.c_str() );
    Runtime::GetLogger().Add(   _T("; }") );
    
    tstr S;
    int  nResult = (int) (INT_PTR) ShellExecute( NULL, _T("open"), arg1.c_str(), 
           (arg2.length() > 0) ? arg2.c_str() : NULL, NULL, SW_SHOWNORMAL );
    switch ( nResult )
    {
        case 0:
            S = _T("- the operating system is out of memory or resources");
            break;
        case ERROR_FILE_NOT_FOUND:
            S = _T("- the specified file was not found");
            break;
        case ERROR_PATH_NOT_FOUND:
            S = _T("- the specified path was not found");
            break;
        case ERROR_BAD_FORMAT:
            S = _T("- the .exe file is invalid (non-Microsoft Win32 .exe or error in .exe image)");
            break;
        case SE_ERR_ACCESSDENIED:
            S = _T("- the operating system denied access to the specified file");
            break;
        case SE_ERR_ASSOCINCOMPLETE:
            S = _T("- the file name association is incomplete or invalid");
            break; 
        case SE_ERR_DDEBUSY:
            S = _T("- the DDE transaction could not be completed because other DDE transactions were being processed");
            break;
        case SE_ERR_DDEFAIL:
            S = _T("- the DDE transaction failed");
            break;
        case SE_ERR_DDETIMEOUT:
            S = _T("- the DDE transaction could not be completed because the request timed out");
            break;
        case SE_ERR_DLLNOTFOUND:
            S = _T("- the specified dynamic-link library (DLL) was not found");
            break;
        case SE_ERR_NOASSOC:
            S = _T("- there is no application associated with the given file name extension");
            break;
        case SE_ERR_OOM:
            S = _T("- there was not enough memory to complete the operation");
            break;
        case SE_ERR_SHARE:
            S = _T("- a sharing violation occurred");
            break;
        default:
            S = _T("");
            break;
    }
    if ( S.length() > 0 )
    {
        ScriptError( ET_REPORT, S.c_str() );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSave(const tstr& params)
{
    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    TCHAR szFileName[FILEPATH_BUFSIZE];

    reportCmdAndParams( DoNppSaveCommand::Name(), params, 0 );
          
    // save a file
    bool bCurrentFile = params.IsEmpty();
    if ( bCurrentFile )
    {
          
        Runtime::GetLogger().Add(   _T("; no file name specified - saving current file") );
            
        // file name is not specified i.e. saving current file
        m_pNppExec->SendNppMsg( NPPM_GETFULLCURRENTPATH,
          (WPARAM) (FILEPATH_BUFSIZE - 1), (LPARAM) szFileName );
    }
    else
    {
          
        Runtime::GetLogger().Add(   _T("; retrieving full file name") );
            
        // file name is specified
        if ( !m_pNppExec->nppSwitchToDocument(params.c_str(), true) )
        {
            ScriptError( ET_REPORT, _T("- no such file opened") );
            return CMDRESULT_FAILED;
        }
    }
        
    messageConsole( DoNppSaveCommand::Name(), bCurrentFile ? szFileName : params.c_str() );
    
    Runtime::GetLogger().Add(   _T("; saving") );

    // TODO: use Notepad++'s message to check the document's state
    HWND hSciWnd = m_pNppExec->GetScintillaHandle();
    BOOL bModified = (BOOL) ::SendMessage(hSciWnd, SCI_GETMODIFY, 0, 0);

    if ( !m_pNppExec->SendNppMsg(NPPM_SAVECURRENTFILE, 0, 0) )
    {
        if ( bModified )
        {
            ScriptError( ET_REPORT, _T("- could not save the file") );
            nCmdResult = CMDRESULT_FAILED;
        }
        // else the document is unmodified - nothing to save
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSaveAs(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppSaveAsCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    Runtime::GetLogger().Add(   _T("; saving") );

    if ( !m_pNppExec->SendNppMsg(NPPM_SAVECURRENTFILEAS, 0, (LPARAM) params.c_str()) )
    {
        ScriptError( ET_REPORT, _T("- failed to save the file") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSaveAll(const tstr& params)
{
    reportCmdAndParams( DoNppSaveAllCommand::Name(), params, 0 );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    if ( !params.IsEmpty() )
    {
        ScriptError( ET_REPORT, _T("- unexpected parameter(s)") );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
    m_pNppExec->GetConsole().PrintMessage(DoNppSaveAllCommand::Name(), nMsgFlags);
    if ( !m_pNppExec->nppSaveAllFiles() )
        nCmdResult = CMDRESULT_FAILED;

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSwitch(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppSwitchCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    if ( !m_pNppExec->nppSwitchToDocument(params.c_str(), true) )
    {
        ScriptError( ET_REPORT, _T("- no such file opened") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSetFocus(const tstr& params)
{
    if ( !reportCmdAndParams( DoNppSetFocusCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_FAILED;
    tstr type = params;
    NppExecHelpers::StrUpper(type);

    if ( type == _T("CON") )
    {
        HWND hConDlg = m_pNppExec->GetConsole().GetDialogWnd();
        if ( ::IsWindowVisible(hConDlg) )
        {
            // Note: since this code is executed in the scope of
            // CNppExecCommandExecutor::BackgroundExecuteThreadFunc,
            // it is not straightforward to set the focus
            HWND hCon = m_pNppExec->GetConsole().GetConsoleWnd();
            HWND hFocused = NppExecHelpers::GetFocusedWnd();
            if ( hFocused != hCon && hFocused != hConDlg )
            {
                //m_pNppExec->SendNppMsg(NPPM_DMMSHOW, 0, (LPARAM) hConDlg);
                ::SetFocus(hConDlg);
                ::SendMessage(hConDlg, WM_SETFOCUS, 0, 0);
            }
            m_pNppExec->m_hFocusedWindowBeforeScriptStarted = hCon;
            nCmdResult = CMDRESULT_SUCCEEDED;
        }
    }
    else if ( type == _T("SCI") )
    {
        HWND hSci = m_pNppExec->GetScintillaHandle();
        if ( ::IsWindowVisible(hSci) )
        {
            // Note: since this code is executed in the scope of
            // CNppExecCommandExecutor::BackgroundExecuteThreadFunc,
            // it is not straightforward to set the focus
            HWND hFocused = NppExecHelpers::GetFocusedWnd();
            if ( hFocused != hSci )
                ::SendMessage(hSci, SCI_SETFOCUS, 1, 0);
            m_pNppExec->m_hFocusedWindowBeforeScriptStarted = hSci;
            nCmdResult = CMDRESULT_SUCCEEDED;
        }
    }
    else
    {
        tstr Err = _T("- unknown parameter: ");
        Err += params;
        ScriptError( ET_REPORT, Err.c_str() );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSendMsg(const tstr& params)
{
    return doSendMsg(params, CMDTYPE_NPPSENDMSG);
}

CScriptEngine::eCmdResult CScriptEngine::DoNppSendMsgEx(const tstr& params)
{
    return doSendMsg(params, CMDTYPE_NPPSENDMSGEX);
}

CScriptEngine::eCmdResult CScriptEngine::DoSciSendMsg(const tstr& params)
{
    return doSendMsg(params, CMDTYPE_SCISENDMSG);
}

CScriptEngine::eCmdResult CScriptEngine::doSendMsg(const tstr& params, int cmdType)
{
    tstr cmdName;
    HWND hWnd = NULL;
    bool isMsgEx = false;
    
    switch ( cmdType )
    {
        case CMDTYPE_NPPSENDMSG:
            cmdName = DoNppSendMsgCommand::Name();
            hWnd = m_pNppExec->m_nppData._nppHandle;
            break;

        case CMDTYPE_SCISENDMSG:
            cmdName = DoSciSendMsgCommand::Name();
            hWnd = m_pNppExec->GetScintillaHandle();
            break;

        case CMDTYPE_NPPSENDMSGEX:
            cmdName = DoNppSendMsgExCommand::Name();
            isMsgEx = true;
            break;

        default:
            // unsupported command
            return CMDRESULT_FAILED;
    }

    if ( !reportCmdAndParams( cmdName.c_str(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // send message to Notepad++
    
    // npp(sci)_msg <uMsg>
    // npp(sci)_msg <uMsg> <wParam>
    // npp(sci)_msg <uMsg> <wParam> <lParam>
    
    tstr val;
    tstr calcErr;
    const TCHAR* s = params.c_str();

    for ( int n = 0; n < (isMsgEx ? 2 : 1); n++ )
    {
        bool isValidParam = true;
        unsigned int hasQuotes = 0;

        s = get_param(s, val, SEP_TABSPACE, &hasQuotes); // uMsg or hWnd
        if ( !val.IsEmpty() )
        {

            if ( isMsgEx && n == 0 )
                Runtime::GetLogger().Add(   _T("; hWnd parameter...") );
            else
                Runtime::GetLogger().Add(   _T("; uMsg parameter...") );

            m_pNppExec->GetMacroVars().CheckAllMacroVars(this, val, true);

            if ( isMsgEx && n == 0 )
            {
                // hWnd
                if ( hasQuotes || val.GetAt(0) == _T('@') )
                {
                    isValidParam = false;
                }
                else
                {

                  #ifdef _WIN64
                    hWnd = (HWND) (UINT_PTR) c_base::_tstr2uint64( val.c_str() ); // 64-bit pointer
                  #else
                    hWnd = (HWND) (UINT_PTR) c_base::_tstr2uint( val.c_str() ); // 32-bit pointer
                  #endif

                    if ( !::IsWindow(hWnd) )
                        isValidParam = false;
                }
            }
        }
        else
        {
            if ( isMsgEx && n == 0 )
                isValidParam = false;
        }

        if ( isMsgEx && !isValidParam )
        {
            // hwnd should be a valid window handle
            ScriptError( ET_REPORT, _T("- incorrect hWnd: should be a valid window handle") );
            return CMDRESULT_FAILED;
        }
    }

    if ( (!isDecNumChar(val.GetAt(0))) &&
         (val.GetAt(0) != _T('-') || !isDecNumChar(val.GetAt(1))) )
    {
        g_fp.Calculate(m_pNppExec, val, calcErr, val); // try to calculate
    }

    UINT uMsg = c_base::_tstr2uint( val.c_str() );
    if ( uMsg == 0 )
    {
        ScriptError( ET_REPORT, _T("- uMsg is 0 (no message to send)") );
        return CMDRESULT_FAILED;
    }

    if ( *s == 0 )
    {
        // npp(sci)_msg <uMsg>

        if ( isMsgEx )
            Runtime::GetLogger().AddEx( _T("; hWnd  = %d (0x%X)"), hWnd, hWnd );
        Runtime::GetLogger().AddEx( _T("; uMsg  = %d (0x%X)"), uMsg, uMsg );
        Runtime::GetLogger().AddEx( _T("; wType = %s,  wParam = \"0\""), STR_PARAMTYPE[PT_INT] );
        Runtime::GetLogger().AddEx( _T("; lType = %s,  lParam = \"0\""), STR_PARAMTYPE[PT_INT] );

        Runtime::GetLogger().Add(   _T("; calling SendMessage ...") );

        LRESULT lResult = ::SendMessage( hWnd, uMsg, 0, 0 );

        Runtime::GetLogger().Add(   _T("; ... SendMessage has been called") );
        
        // RESULT
        tstr varName;
        tstr varValue;
        TCHAR szNum[50];

        varName = MACRO_MSG_RESULT;
        szNum[0] = 0;
      #ifdef _WIN64
        c_base::_tint64_to_str( static_cast<INT_PTR>(lResult), szNum );
      #else
        c_base::_tint2str( static_cast<INT_PTR>(lResult), szNum );
      #endif
        varValue = szNum;
        m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
        
        // WPARAM
        varName = MACRO_MSG_WPARAM;
        varValue.Clear();
        m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
        
        // LPARAM
        varName = MACRO_MSG_LPARAM;
        varValue.Clear();
        m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
        
        return CMDRESULT_SUCCEEDED;
    }

    int  paramType[2] = { PT_INT, PT_INT };
    tstr paramValue[2] = { _T("0"), _T("0") };

    // npp(sci)_msg <uMsg> <wParam>
    // npp(sci)_msg <uMsg> <wParam> <lParam>
    
    for ( int n = 0; n < 2; n++ )
    {
        unsigned int hasQuotes = 0;
        bool hasBracket = false;
        tstr& value = paramValue[n];
        s = get_param(s, value, SEP_TABSPACE, &hasQuotes, &hasBracket); // wType or lType
        if ( !value.IsEmpty() )
        {

            if ( n == 0 )
                Runtime::GetLogger().Add(   _T("; wParam parameter...") );
            else
                Runtime::GetLogger().Add(   _T("; lParam parameter...") );

            m_pNppExec->GetMacroVars().CheckAllMacroVars(this, value, true);

            if ( hasBracket )
            {
                if ( value.GetLastChar() != _T(']') )
                {
                    //  starts with '[', but no ']' in the end
                    ScriptError( ET_REPORT, _T("- hex-string parameter should end with \']\'") );
                    return CMDRESULT_INVALIDPARAM;
                }
            }
        }

        if ( value.GetAt(0) == _T('@') )
        {
            value.Delete(0, 1);
            if ( value.IsEmpty() && !hasQuotes )
            {
                // empty, no '"'
                ScriptError( ET_REPORT, _T("- empty pointer parameter specified") );
                return CMDRESULT_INVALIDPARAM;
            }

            if ( hasBracket )
                paramType[n] = PT_PHEXSTR;
            else if ( hasQuotes )
                paramType[n] = PT_PSTR;
            else
                paramType[n] = PT_PINT;
        }
        else
        {
            if ( hasBracket )
                paramType[n] = PT_HEXSTR;
            else if ( hasQuotes )
                paramType[n] = PT_STR;
            else
                paramType[n] = PT_INT;
        }

        switch ( paramType[n] )
        {
            case PT_INT:
            case PT_PINT:
            {
                if ( (!isDecNumChar(value.GetAt(0))) &&
                     (value.GetAt(0) != _T('-') || !isDecNumChar(value.GetAt(1))) )
                {
                    if ( !g_fp.Calculate(m_pNppExec, value, calcErr, value) ) // try to calculate
                    {
                        ScriptError( ET_REPORT, _T("- string parameter specified without \"\"") );
                        return CMDRESULT_INVALIDPARAM;
                    }
                }
                break;
            }
        }

        if ( *s == 0 )
            break;
    }

    if ( isMsgEx )
        Runtime::GetLogger().AddEx( _T("; hWnd  = %d (0x%X)"), hWnd, hWnd );
    Runtime::GetLogger().AddEx( _T("; uMsg  = %d (0x%X)"), uMsg, uMsg );
    Runtime::GetLogger().AddEx( _T("; wType = %s,  wParam = \"%s\""), 
      STR_PARAMTYPE[paramType[0]], paramValue[0].c_str() );
    Runtime::GetLogger().AddEx( _T("; lType = %s,  lParam = \"%s\""), 
      STR_PARAMTYPE[paramType[1]], paramValue[1].c_str() );
        
    // npp(sci)_msg <uMsg> <wParam> <lParam>

    const int PARAM_BUF_SIZE = m_pNppExec->GetOptions().GetInt(OPTI_SENDMSG_MAXBUFLEN);
    TCHAR*   pParam[2] = { NULL, NULL };
    char*    pSciParam[2] = { NULL, NULL };
    UINT_PTR uiParam[2] = { 0, 0 };
    WPARAM   wParam = 0;
    LPARAM   lParam = 0;
    TCHAR    szNum[50];

    for ( int n = 0; n < 2; n++ )
    {
        switch ( paramType[n] )
        {
            case PT_INT:
              #ifdef _WIN64
                uiParam[n] = c_base::_tstr2int64( paramValue[n].c_str() );
              #else
                uiParam[n] = c_base::_tstr2int( paramValue[n].c_str() );
              #endif
                if ( n == 0 )
                    wParam = uiParam[n];
                else
                    lParam = uiParam[n];
                break;

            case PT_PINT:
              #ifdef _WIN64
                uiParam[n] = c_base::_tstr2int64( paramValue[n].c_str() );
              #else
                uiParam[n] = c_base::_tstr2int( paramValue[n].c_str() );
              #endif
                if ( n == 0 )
                    wParam = (WPARAM) &uiParam[n];
                else
                    lParam = (LPARAM) &uiParam[n];
                break;

            case PT_STR:
            case PT_PSTR:
            case PT_HEXSTR:
            case PT_PHEXSTR:
                if ( paramType[n] == PT_HEXSTR || paramType[n] == PT_PHEXSTR )
                {
                    int nHexBufSize = PARAM_BUF_SIZE;
                    if ( cmdType != CMDTYPE_SCISENDMSG )
                        nHexBufSize *= sizeof(TCHAR);
                    c_base::byte_t* pHexBuf = new c_base::byte_t[nHexBufSize];
                    if ( pHexBuf )
                    {
                        memset( pHexBuf, 0, nHexBufSize );

                        tstr& value = paramValue[n];
                        const TCHAR* pHexStr = value.c_str() + 1; // skip leading '['
                        value.SetSize( value.length() - 1 ); // exlude trailing ']'
                        c_base::_thexstrex2buf( pHexStr, pHexBuf, nHexBufSize - sizeof(TCHAR) );

                        if ( cmdType == CMDTYPE_SCISENDMSG )
                            pSciParam[n] = (char*) pHexBuf;
                        else
                            pParam[n] = (TCHAR*) pHexBuf;
                    }
                }
                else
                {
                    if ( cmdType == CMDTYPE_SCISENDMSG )
                    {
                        pSciParam[n] = new char[PARAM_BUF_SIZE];
                        if ( pSciParam[n] )
                        {
                            memset( pSciParam[n], 0, PARAM_BUF_SIZE*sizeof(char) );
                            if ( !paramValue[n].IsEmpty() )
                            {
                                char* pp = SciTextFromLPCTSTR(paramValue[n].c_str(), hWnd);
                                if ( pp )
                                {
                                    lstrcpyA(pSciParam[n], pp);
                                    SciTextDeleteResultPtr(pp, paramValue[n].c_str());
                                }
                            }
                        }
                    }
                    else
                    {
                        pParam[n] = new TCHAR[PARAM_BUF_SIZE];
                        if ( pParam[n] )
                        {
                            memset( pParam[n], 0, PARAM_BUF_SIZE*sizeof(TCHAR) );
                            lstrcpy( pParam[n], paramValue[n].c_str() );
                        }
                    }
                }
                if ( pParam[n] )
                {
                    if ( n == 0 )
                        wParam = (WPARAM) pParam[n];
                    else
                        lParam = (LPARAM) pParam[n];
                }
                else if ( pSciParam[n] )
                {
                    if ( n == 0 )
                        wParam = (WPARAM) pSciParam[n];
                    else
                        lParam = (LPARAM) pSciParam[n];
                }
                else
                {
                    ScriptError( ET_REPORT, _T("- could not allocate memory for a parameter") );
                    return CMDRESULT_FAILED;
                }
                break;
        }
    }

    Runtime::GetLogger().Add(   _T("; calling SendMessage ...") );

    LRESULT lResult = ::SendMessage( hWnd, uMsg, wParam, lParam );

    Runtime::GetLogger().Add(   _T("; ... SendMessage has been called") );

    for ( int n = 0; n < 2; n++ )
    {
        if ( paramType[n] == PT_STR || paramType[n] == PT_HEXSTR )
        {
            if ( pSciParam[n] )
            {
                #ifdef UNICODE
                    delete [] pSciParam[n];
                #else
                    if ( pSciParam[n] != paramValue[n].c_str() )
                        delete [] pSciParam[n];
                #endif

                pSciParam[n] = NULL;
            }
            if ( pParam[n] )
            {
                delete [] pParam[n];
                pParam[n] = NULL;
            }
        }
    }

    int  i;
    tstr varName;
    tstr varValue;
    
    for ( int n = 0; n < 2; n++ )
    {
        varName = (n == 0) ? MACRO_MSG_WPARAM : MACRO_MSG_LPARAM;

        switch ( paramType[n] )
        {
            case PT_PINT:
                szNum[0] = 0;
              #ifdef _WIN64
                c_base::_tint64_to_str( static_cast<INT_PTR>(uiParam[n]), szNum );
              #else
                c_base::_tint2str( static_cast<INT_PTR>(uiParam[n]), szNum );
              #endif
                varValue = szNum;
                m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
                break;

            case PT_PSTR:
                if ( cmdType == CMDTYPE_SCISENDMSG )
                {
                    TCHAR* pp = SciTextToLPTSTR(pSciParam[n], hWnd);
                    varValue = pp;
                    SciTextDeleteResultPtr(pp, pSciParam[n]);
                    delete [] pSciParam[n];
                    pSciParam[n] = NULL;
                }
                else
                {
                    varValue = pParam[n];
                    delete [] pParam[n];
                    pParam[n] = NULL;
                }
                for ( i = varValue.length() - 1; i >= 0; i-- )
                {
                    const TCHAR ch = varValue[i];
                    if ( ch == _T('\r') || ch == _T('\n') )
                        varValue.Delete(i, 1); // exclude trailing '\r' or '\n'
                    else
                        break;
                }
                m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
                break;

            case PT_PHEXSTR:
                varValue = _T("[ ");
                {
                    const int nBytesInLine = 16;
                    const int nBytesToShow = 256;
                    c_base::byte_t* pHexBuf;
                    c_base::byte_t* p;
                    TCHAR szBytesLine[3*nBytesInLine + 2];

                    if ( cmdType == CMDTYPE_SCISENDMSG )
                        pHexBuf = (c_base::byte_t *) pSciParam[n];
                    else
                        pHexBuf = (c_base::byte_t *) pParam[n];

                    p = pHexBuf;
                    while ( p < pHexBuf + nBytesToShow )
                    {
                        if ( p != pHexBuf )  varValue += _T_RE_EOL _T("  ");
                        c_base::_tbuf2hexstr(p, nBytesInLine, szBytesLine, 3*nBytesInLine + 2, _T(" "));
                        varValue += szBytesLine;
                        p += nBytesInLine;
                    }

                    delete [] pHexBuf;
                    if ( cmdType == CMDTYPE_SCISENDMSG )
                        pSciParam[n] = NULL;
                    else
                        pParam[n] = NULL;
                }
                varValue += _T(" ]");
                m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
                break;

            default:
                varValue.Clear();
                m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
                break;
        }
    }

    varName = MACRO_MSG_RESULT;
    szNum[0] = 0;
  #ifdef _WIN64
    c_base::_tint64_to_str( static_cast<INT_PTR>(lResult), szNum );
  #else
    c_base::_tint2str( static_cast<INT_PTR>(lResult), szNum );
  #endif
    varValue = szNum;
    m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::doSciFindReplace(const tstr& params, eCmdType cmdType)
{
    if ( !reportCmdAndParams( GetCommandRegistry().GetCmdNameByType(cmdType), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    // 1. Preparing the arguments...
    CStrSplitT<TCHAR> args;
    const int nArgs = args.SplitToArgs(params, (cmdType == CMDTYPE_SCIFIND) ? 2 : 3);
    if ( nArgs < 2 )
    {
        TCHAR szErr[64];
        wsprintf(szErr, _T("not enough parameters: %s expected, %d given"), (cmdType == CMDTYPE_SCIFIND) ? _T("2") : _T("at least 2"), nArgs );
        errorCmdNotEnoughParams( GetCommandRegistry().GetCmdNameByType(cmdType), szErr );
        return CMDRESULT_INVALIDPARAM;
    }

    for ( int i = 0; i < nArgs; i++ )
    {
        tstr& val = args.Arg(i);
        NppExecHelpers::StrUnquoteEx(val);
        m_pNppExec->GetMacroVars().CheckAllMacroVars(this, val, true);
    }

    // 2. Search flags...
    tstr sFlags = args.Arg(0);
    NppExecHelpers::StrUnquoteEx(sFlags);
    CNppExecMacroVars::StrCalc(sFlags, m_pNppExec).Process();
    HWND hSci = m_pNppExec->GetScintillaHandle();
    unsigned int nSearchFlags = 0;
    unsigned int nFlags = c_base::_tstr2uint(sFlags.c_str());
    if ( nFlags & NPE_SF_MATCHCASE )
        nSearchFlags |= SCFIND_MATCHCASE;
    if ( nFlags & NPE_SF_WHOLEWORD )
        nSearchFlags |= SCFIND_WHOLEWORD;
    if ( nFlags & NPE_SF_WORDSTART )
        nSearchFlags |= SCFIND_WORDSTART;
    if ( nFlags & NPE_SF_REGEXP )
        nSearchFlags |= SCFIND_REGEXP;
    if ( nFlags & NPE_SF_POSIX )
        nSearchFlags |= SCFIND_POSIX;
    if ( nFlags & NPE_SF_CXX11REGEX )
        nSearchFlags |= SCFIND_CXX11REGEX;
    if ( nFlags & NPE_SF_REGEXP_EMPTYMATCH_NOTAFTERMATCH )
        nSearchFlags |= SCFIND_REGEXP_EMPTYMATCH_NOTAFTERMATCH;
    if ( nFlags & NPE_SF_REGEXP_EMPTYMATCH_ALL )
        nSearchFlags |= SCFIND_REGEXP_EMPTYMATCH_ALL;
    if ( nFlags & NPE_SF_REGEXP_EMPTYMATCH_ALLOWATSTART )
        nSearchFlags |= SCFIND_REGEXP_EMPTYMATCH_ALLOWATSTART;
    ::SendMessage(hSci, SCI_SETSEARCHFLAGS, nSearchFlags, 0);

    // 3. Search range...
    INT_PTR nSelStart = (INT_PTR) ::SendMessage(hSci, SCI_GETSELECTIONSTART, 0, 0);
    INT_PTR nSelEnd = (INT_PTR) ::SendMessage(hSci, SCI_GETSELECTIONEND, 0, 0);
    const bool bNoSelection = (nSelStart == nSelEnd);
    const bool bReplacingAll = ((nFlags & NPE_SF_REPLACEALL) && (cmdType == CMDTYPE_SCIREPLACE));
    INT_PTR nTextLength = (INT_PTR) ::SendMessage(hSci, SCI_GETTEXTLENGTH, 0, 0);
    INT_PTR nRangeStart = -1, nRangeEnd = -1;
    if ( (nFlags & NPE_SF_PRINTALL) || bReplacingAll )
    {
        if ( nFlags & NPE_SF_INENTIRETEXT )
        {
            // in the entire text
            nRangeStart = 0;
            nRangeEnd = nTextLength;
        }
        else if ( nFlags & NPE_SF_INSELECTION )
        {
            // only in the selected text
            nRangeStart = nSelStart;
            nRangeEnd = nSelEnd;
        }
        else
        {
            if ( nFlags & NPE_SF_BACKWARD )
            {
                nRangeStart = 0;
                nRangeEnd = nSelStart;
            }
            else
            {
                nRangeStart = nSelEnd;
                nRangeEnd = nTextLength;
            }
        }
    }
    else if ( nFlags & NPE_SF_INSELECTION )
    {
        nRangeStart = nSelStart;
        nRangeEnd = nSelEnd;
    }
    else
    {
        if ( nFlags & NPE_SF_BACKWARD )
        {
            nRangeStart = 0;
            nRangeEnd = nSelStart;
            // in case of NPE_SF_NEXT, no need to adjust nRangeEnd here
        }
        else
        {
            nRangeStart = nSelEnd;
            if ( ((nFlags & NPE_SF_NEXT) != 0) &&
                 bNoSelection && (nRangeStart != nTextLength) )
            {
                nRangeStart = (INT_PTR) ::SendMessage(hSci, SCI_POSITIONRELATIVE, (WPARAM) nRangeStart, (LPARAM) (1));
                // Note: just ++nRangeStart will not work for UTF-8
            }
            nRangeEnd = nTextLength;
        }
    }

    // 4. Strings to find & replace...
    int nFindStrLen = 0;
    char* pFindStr = SciTextFromLPCTSTR(args.GetArg(1).c_str(), hSci, &nFindStrLen);

    if ( nFindStrLen == 0 )
    {
        ScriptError( ET_REPORT, _T("- can not search for an empty string") );
        return CMDRESULT_INVALIDPARAM;
    }

    int nReplaceStrLen = 0;
    char* pReplaceStr = NULL;
    if ( cmdType == CMDTYPE_SCIREPLACE )
    {
        pReplaceStr = SciTextFromLPCTSTR(args.GetArg(2).c_str(), hSci, &nReplaceStrLen);
    }

    INT_PTR nFindStrLenOut = nFindStrLen;
    INT_PTR nReplaceStrLenOut = nReplaceStrLen;
    
    // 5. Searching...
    INT_PTR nPos = -1;
    if ( (nFlags & NPE_SF_PRINTALL) || bReplacingAll )
    {
        tstr S;
        unsigned int nOccurrences = 0;

        if ( bReplacingAll )
            ::SendMessage(hSci, SCI_BEGINUNDOACTION, 0, 0);

        for ( ; ; )
        {
            if ( nFlags & NPE_SF_BACKWARD )
                ::SendMessage(hSci, SCI_SETTARGETRANGE, nRangeEnd, nRangeStart);
            else
                ::SendMessage(hSci, SCI_SETTARGETRANGE, nRangeStart, nRangeEnd);

            INT_PTR pos = (INT_PTR) ::SendMessage( hSci, SCI_SEARCHINTARGET, (WPARAM) nFindStrLen, (LPARAM) (pFindStr ? pFindStr : "") );
            if ( pos < 0 )
                break;

            nPos = pos;

            pos = (INT_PTR) ::SendMessage(hSci, SCI_GETTARGETEND, 0, 0);
            nFindStrLenOut = pos - nPos;

            ++nOccurrences;

            bool isReplacing = (bReplacingAll || ((cmdType == CMDTYPE_SCIREPLACE) && (nOccurrences == 1)));
            if ( isReplacing )
            {
                if ( nFlags & (NPE_SF_REGEXP | NPE_SF_POSIX | NPE_SF_CXX11REGEX) )
                {
                    nReplaceStrLenOut = (INT_PTR) ::SendMessage( hSci, SCI_REPLACETARGETRE, (WPARAM) nReplaceStrLen, (LPARAM) (pReplaceStr ? pReplaceStr : "") );
                }
                else
                {
                    nReplaceStrLenOut = (INT_PTR) ::SendMessage( hSci, SCI_REPLACETARGET, (WPARAM) nReplaceStrLen, (LPARAM) (pReplaceStr ? pReplaceStr : "") );
                }

                if ( nFlags & NPE_SF_BACKWARD )
                {
                    nRangeEnd = nPos;
                }
                else
                {
                    nRangeStart = nPos + nReplaceStrLenOut;
                    nRangeEnd += (nReplaceStrLenOut - nFindStrLenOut);
                }

                if ( nPos + nFindStrLen <= nSelEnd )
                {
                    // each replacement affects the selection range
                    nSelEnd += (nReplaceStrLenOut - nFindStrLenOut);
                    if ( bNoSelection )
                        nSelStart = nSelEnd;
                }
            }
            else
            {
                if ( nFlags & NPE_SF_BACKWARD )
                {
                    nRangeEnd = nPos;
                }
                else
                {
                    nRangeStart = nPos + nFindStrLenOut;
                }
            }

            if ( nFlags & NPE_SF_PRINTALL )
            {
                const int MAX_STR_TO_SHOW = 128;
                const int UTF8_MAX_BYTES_PER_CHAR = 4;
                INT_PTR nLine = (INT_PTR) ::SendMessage(hSci, SCI_LINEFROMPOSITION, (WPARAM) nPos, 0);
                INT_PTR nLinePos = (INT_PTR) ::SendMessage(hSci, SCI_POSITIONFROMLINE, (WPARAM) nLine, 0);
                INT_PTR nLen = isReplacing ? nReplaceStrLenOut : nFindStrLenOut;
                int     nLenToShow = static_cast<int>(nLen);
                bool    isUtf8 = (::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0) == SC_CP_UTF8);
                bool    isLenTruncated = false;
                if ( isUtf8 )
                {
                    if ( nLen > (MAX_STR_TO_SHOW*UTF8_MAX_BYTES_PER_CHAR) )
                    {
                        nLen = MAX_STR_TO_SHOW*UTF8_MAX_BYTES_PER_CHAR - 5; // number of chars, plus "(...)" will be added
                        nLenToShow = MAX_STR_TO_SHOW - 5; // number of TCHARs
                        isLenTruncated = true;
                    }
                    else
                        nLenToShow = MAX_STR_TO_SHOW; // will be adjusted later
                }
                else
                {
                    if ( nLen > MAX_STR_TO_SHOW )
                    {
                        nLen = MAX_STR_TO_SHOW - 5; // number of chars, plus "(...)" will be added
                        nLenToShow = MAX_STR_TO_SHOW - 5; // number of TCHARs
                        isLenTruncated = true;
                    }
                }
                Sci_TextRangeFull tr;
                tr.chrg.cpMin = static_cast<decltype(tr.chrg.cpMin)>(nPos);        // I believe Sci_CharacterRange will use INT_PTR
                tr.chrg.cpMax = static_cast<decltype(tr.chrg.cpMax)>(nPos + nLen); // or UINT_PTR to deal with 64-bit ranges
                S.Reserve(50 + static_cast<int>(nLen)); // enough for both char* and TCHAR* buffer
                tr.lpstrText = (char *) S.c_str(); // temporary using S as a char* buffer
                ::SendMessage(hSci, SCI_GETTEXTRANGEFULL, 0, (LPARAM) &tr);
                TCHAR* pText = SciTextToLPTSTR(tr.lpstrText, hSci); // now we have the text as TCHAR*
                INT_PTR nCharPos = ::SendMessage(hSci, SCI_COUNTCHARACTERS, nLinePos, nPos);
                // Note: nCharPos can't be just (nPos - nLinePos) because it will not work for UTF-8
                // where each character occupies up to several bytes in Scintilla's buffer
                if ( isUtf8 )
                {
                    int n = pText ? lstrlen(pText) : 0;
                    if ( n > nLenToShow )
                    {
                        if ( !isLenTruncated )
                        {
                            isLenTruncated = true;
                            nLenToShow = MAX_STR_TO_SHOW - 5;
                        }
                        pText[nLenToShow] = 0;
                    }
                }
                nLen = _t_sprintf( S.data(), // now using S as a TCHAR* buffer
                                 #ifdef _WIN64
                                   _T("(%I64d,%I64d)\t %s%s"), 
                                 #else
                                   _T("(%d,%d)\t %s%s"), 
                                 #endif
                                   nLine + 1, 
                                   nCharPos + 1, 
                                   pText ? pText : _T("(empty)"), 
                                   isLenTruncated ? _T("(...)") : _T("")
                                 );
                SciTextDeleteResultPtr(pText, tr.lpstrText);
                S.SetLengthValue(static_cast<int>(nLen));
                m_pNppExec->GetConsole().PrintOutput(S.c_str());
            }
        }

        if ( bReplacingAll )
            ::SendMessage(hSci, SCI_ENDUNDOACTION, 0, 0);

        S.Format(50, _T("- %u occurrences %s."), nOccurrences, bReplacingAll ? _T("replaced") : _T("found"));
        UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
        if ( (nFlags & NPE_SF_PRINTALL) == 0 )
            nMsgFlags |= CNppExecConsole::pfIsInternalMsg;
        m_pNppExec->GetConsole().PrintMessage(S.c_str(), nMsgFlags);
    }
    else
    {
        if ( nFlags & NPE_SF_BACKWARD )
            ::SendMessage(hSci, SCI_SETTARGETRANGE, nRangeEnd, nRangeStart);
        else
            ::SendMessage(hSci, SCI_SETTARGETRANGE, nRangeStart, nRangeEnd);

        nPos = (INT_PTR) ::SendMessage( hSci, SCI_SEARCHINTARGET, (WPARAM) nFindStrLen, (LPARAM) (pFindStr ? pFindStr : "") );
        if ( (nPos < 0) && (nFlags & NPE_SF_INENTIRETEXT) )
        {
            // search again - in the rest of the document...
            if ( nFlags & NPE_SF_INSELECTION )
            {
                nRangeStart = 0;
                nRangeEnd = nTextLength;
            }
            else
            {
                if ( nFlags & NPE_SF_BACKWARD )
                {
                    nRangeStart = nSelStart - nFindStrLen;
                    if ( nRangeStart < 0 )
                        nRangeStart = 0;
                    nRangeEnd = nTextLength;
                }
                else
                {
                    nRangeStart = 0;
                    nRangeEnd = nSelEnd + nFindStrLen;
                    if ( nRangeEnd > nTextLength )
                        nRangeEnd = nTextLength;
                }
            }

            if ( nFlags & NPE_SF_BACKWARD )
                ::SendMessage(hSci, SCI_SETTARGETRANGE, nRangeEnd, nRangeStart);
            else
                ::SendMessage(hSci, SCI_SETTARGETRANGE, nRangeStart, nRangeEnd);
            
            nPos = (INT_PTR) ::SendMessage( hSci, SCI_SEARCHINTARGET, (WPARAM) nFindStrLen, (LPARAM) (pFindStr ? pFindStr : "") );
        }

        if ( nPos >= 0 )
        {
            INT_PTR pos = (INT_PTR) ::SendMessage(hSci, SCI_GETTARGETEND, 0, 0);
            nFindStrLenOut = pos - nPos;

            if ( cmdType == CMDTYPE_SCIREPLACE )
            {
                if ( nFlags & (NPE_SF_REGEXP | NPE_SF_POSIX | NPE_SF_CXX11REGEX) )
                {
                    nReplaceStrLenOut = (INT_PTR) ::SendMessage( hSci, SCI_REPLACETARGETRE, (WPARAM) nReplaceStrLen, (LPARAM) (pReplaceStr ? pReplaceStr : "") );
                }
                else
                {
                    nReplaceStrLenOut = (INT_PTR) ::SendMessage( hSci, SCI_REPLACETARGET, (WPARAM) nReplaceStrLen, (LPARAM) (pReplaceStr ? pReplaceStr : "") );
                }
            }
        }
    }

    SciTextDeleteResultPtr(pFindStr, args.GetArg(1).c_str());
    SciTextDeleteResultPtr(pReplaceStr, args.GetArg(2).c_str());

    // 6. Result
    {
        tstr varName;
        tstr varValue;
        TCHAR szNum[50];

        varName = MACRO_MSG_RESULT;
        szNum[0] = 0;
      #ifdef _WIN64
        c_base::_tint64_to_str(nPos, szNum);
      #else
        c_base::_tint2str(nPos, szNum);
      #endif
        varValue = szNum;
        m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var

        if ( ((nFlags & NPE_SF_PRINTALL) == 0) && !bReplacingAll )
        {
            if ( nPos >= 0 )
            {
                tstr S = (cmdType == CMDTYPE_SCIREPLACE) ? _T("- replaced") : _T("- found");
                S += _T(" at pos ");
                S += szNum;
                const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
            }
            else
            {
                const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
                m_pNppExec->GetConsole().PrintMessage( _T("- not found"), nMsgFlags );
            }
        }

        varName = MACRO_MSG_WPARAM;
        szNum[0] = 0;
      #ifdef _WIN64
        c_base::_tint64_to_str(nFindStrLenOut, szNum);
      #else
        c_base::_tint2str(nFindStrLenOut, szNum);
      #endif
        varValue = szNum;
        m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var

        varName = MACRO_MSG_LPARAM;
        if ( cmdType == CMDTYPE_SCIREPLACE )
        {
            szNum[0] = 0;
          #ifdef _WIN64
            c_base::_tint64_to_str(nReplaceStrLenOut, szNum);
          #else
            c_base::_tint2str(nReplaceStrLenOut, szNum);
          #endif
            varValue = szNum;
            m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar); // local var
        }
        else
        {
            varValue.Clear();
            m_pNppExec->GetMacroVars().SetUserMacroVar(this, varName, varValue, CNppExecMacroVars::svLocalVar | CNppExecMacroVars::svRemoveVar); // local var
        }
    }

    // 7. Set pos/sel
    if ( nPos >= 0 )
    {
        if ( nFlags & NPE_SF_SETSEL )
        {
            INT_PTR nEndPos = (cmdType == CMDTYPE_SCIFIND) ? (nPos + nFindStrLenOut) : (nPos + nReplaceStrLenOut);
            ::SendMessage(hSci, SCI_SETSEL, (WPARAM) nPos, (LPARAM) nEndPos);
        }
        else if ( nFlags & NPE_SF_SETPOS )
        {
            ::SendMessage(hSci, SCI_GOTOPOS, (WPARAM) nPos, 0);
        }
        else if ( cmdType == CMDTYPE_SCIREPLACE )
        {
            // adjust the selection after the replacement(s)
            ::SendMessage(hSci, SCI_SETSEL, (WPARAM) nSelStart, (LPARAM) nSelEnd);
        }
    }

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoSciFind(const tstr& params)
{
    return doSciFindReplace(params, CMDTYPE_SCIFIND);
}

CScriptEngine::eCmdResult CScriptEngine::DoSciReplace(const tstr& params)
{
    return doSciFindReplace(params, CMDTYPE_SCIREPLACE);
}

CScriptEngine::eCmdResult CScriptEngine::DoProcInput(const tstr& params)
{
    reportCmdAndParams( DoProcInputCommand::Name(), params, fMessageToConsole );

    if ( !m_pNppExec->GetCommandExecutor().IsChildProcessRunning() )
    {
        ScriptError( ET_REPORT, _T("- child console process is not running") );
        return CMDRESULT_FAILED;
    }

    CNppExecCommandExecutor& CommandExecutor = m_pNppExec->GetCommandExecutor();
    CommandExecutor.WriteChildProcessInput( params.c_str() );
    const TCHAR ch = params.GetLastChar();
    if ( ch != _T('\n') && ch != _T('\r') )
    {
        CommandExecutor.WriteChildProcessInputNewLine();
    }

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoProcSignal(const tstr& params)
{
    if ( !reportCmdAndParams( DoProcSignalCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    if ( !m_pNppExec->GetCommandExecutor().IsChildProcessRunning() )
    {
        ScriptError( ET_REPORT, _T("- child console process is not running") );
        return CMDRESULT_FAILED;
    }

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    const int nMaxKillMethods = 8;
    int nKillMethods = 0;
    CProcessKiller::eKillMethod arrKillMethods[nMaxKillMethods];
    CProcessKiller::eKillMethod nSignal;
    unsigned int nWaitTimeout = 50; // just a _small_ hard-coded timeout; meaningful value to be specified as the PROC_SIGNAL argument
    tstr arg;
    tstr unknown_args;
    CStrSplitT<TCHAR> args;
    const int n = args.SplitToArgs(params);
    for ( int i = 0; i < n && nKillMethods < nMaxKillMethods; i++ )
    {
        nSignal = CProcessKiller::killNone;
        arg = args.GetArg(i);
        NppExecHelpers::StrUpper(arg);

        if ( arg == _T("CTRLBREAK") || arg == _T("CTRL-BREAK") || arg == _T("CTRL+BREAK") )
            nSignal = CProcessKiller::killCtrlBreak;
        else if ( arg == _T("CTRLC") || arg == _T("CTRL-C") || arg == _T("CTRL+C") )
            nSignal = CProcessKiller::killCtrlC;
        else if ( arg == _T("WMCLOSE") || arg == _T("WM_CLOSE") )
            nSignal = CProcessKiller::killWmClose;
        else if ( c_base::_tis_dec_value(arg.c_str()) )
            nWaitTimeout = c_base::_tstr2uint(arg.c_str());
        else
        {
            if ( !unknown_args.IsEmpty() )
                unknown_args += _T(", ");
            unknown_args += arg;
        }

        if ( nSignal != CProcessKiller::killNone )
            arrKillMethods[nKillMethods++] = nSignal;
    }

    if ( !unknown_args.IsEmpty() )
    {
        unknown_args.Insert( 0, _T("- unknown PROC_SIGNAL parameter(s): ") );
        ScriptError( ET_REPORT, unknown_args.c_str() );
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    if ( nKillMethods == 0 )
    {
        const TCHAR* const cszErr = _T("- no kill method recognized");
        if ( !unknown_args.IsEmpty() )
            m_pNppExec->GetConsole().PrintError( cszErr );
        else
            ScriptError( ET_REPORT, cszErr );
        return CMDRESULT_INVALIDPARAM;
    }

    std::shared_ptr<CChildProcess> pChildProc = GetRunningChildProcess();
    if ( pChildProc )
    {
        if ( pChildProc->Kill(arrKillMethods, nKillMethods, nWaitTimeout, &nSignal) )
            return CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoSleep(const tstr& params)
{
    if ( !reportCmdAndParams( DoSleepCommand::Name(), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    CStrSplitT<TCHAR> args;
    const int n = args.SplitToArgs(params, 2);

    if ( n == 2 )
    {
        tstr Text = args.GetArg(1);
        NppExecHelpers::StrUnquoteEx(Text);
        if ( !Text.IsEmpty() )
        {
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( Text.c_str(), nMsgFlags );
        }
    }

    const TCHAR* pszMilliseconds = args.GetArg(0).c_str();
    if ( (pszMilliseconds[0] < _T('0')) || (pszMilliseconds[0] > _T('9')) )
    {
        tstr Err = _T("- positive integer expected: ");
        Err += pszMilliseconds;
        ScriptError( ET_REPORT, Err.c_str() );
        return CMDRESULT_INVALIDPARAM;
    }
    
    unsigned int nMilliseconds = c_base::_tstr2uint(pszMilliseconds);
    m_eventAbortTheScript.Wait(nMilliseconds);

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::doTextLoad(const tstr& params, eCmdType cmdType)
{
    bool bSelectionOnly = (cmdType == CMDTYPE_SELLOADFROM);
    if ( !reportCmdAndParams( GetCommandRegistry().GetCmdNameByType(cmdType), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    int nBytes = m_pNppExec->textLoadFrom(params.c_str(), bSelectionOnly);
    if ( nBytes < 0 )
    {
        ScriptError( ET_REPORT, _T("- can not open the file") );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::doTextSave(const tstr& params, eCmdType cmdType)
{
    bool bSelectionOnly = (cmdType == CMDTYPE_SELSAVETO);
    if ( !reportCmdAndParams( GetCommandRegistry().GetCmdNameByType(cmdType), params, fMessageToConsole | fReportEmptyParam | fFailIfEmptyParam ) )
        return CMDRESULT_INVALIDPARAM;

    tstr  S;
    TCHAR szCmdLine[FILEPATH_BUFSIZE + 20];
    lstrcpy(szCmdLine, params.c_str());

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    int nBytes = m_pNppExec->textSaveTo(szCmdLine, bSelectionOnly);
    if ( nBytes >= 0 )
        S.Format(60, _T("- OK, %d bytes have been written to \""), nBytes);
    else
        S = _T("- failed to write to \"");
    S += (szCmdLine[0] == _T('\"')) ? (szCmdLine + 1) : szCmdLine;
    S += _T("\"");
    if ( nBytes >= 0 )
    {
        const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
        m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
    }
    else
    {
        if ( nBytes == -2 )
            S += _T(" (text length exceeds 2 GB)");
        ScriptError( ET_REPORT, S.c_str() );
        nCmdResult = CMDRESULT_FAILED;
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoSelLoadFrom(const tstr& params)
{
    return doTextLoad(params, CMDTYPE_SELLOADFROM);
}

CScriptEngine::eCmdResult CScriptEngine::DoSelSaveTo(const tstr& params)
{
    return doTextSave(params, CMDTYPE_SELSAVETO);
}

CScriptEngine::eCmdResult CScriptEngine::DoSelSetText(const tstr& params)
{
    if ( !reportCmdAndParams( DoSelSetTextCommand::Name(), params, fMessageToConsole ) )
        return CMDRESULT_INVALIDPARAM;

    m_pNppExec->textSetText( params.c_str(), true );

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoSelSetTextEx(const tstr& params)
{
    if ( !reportCmdAndParams( DoSelSetTextExCommand::Name(), params, fMessageToConsole ) )
        return CMDRESULT_INVALIDPARAM;

    if ( params.Find(_T('\\')) >= 0 )
    {
        tstr S = params;
        NppExecHelpers::StrUnescape(S);
        m_pNppExec->textSetText( S.c_str(), true );
    }
    else
        m_pNppExec->textSetText( params.c_str(), true );

    return CMDRESULT_SUCCEEDED;
}

CScriptEngine::eCmdResult CScriptEngine::DoTextLoadFrom(const tstr& params)
{
    return doTextLoad(params, CMDTYPE_TEXTLOADFROM);
}

CScriptEngine::eCmdResult CScriptEngine::DoTextSaveTo(const tstr& params)
{
    return doTextSave(params, CMDTYPE_TEXTSAVETO);
}

CScriptEngine::eCmdResult CScriptEngine::DoClipSetText(const tstr& params)
{
    if ( !reportCmdAndParams( DoClipSetTextCommand::Name(), params, fMessageToConsole ) )
        return CMDRESULT_INVALIDPARAM;

    HWND hWndOwner = m_pNppExec->m_nppData._nppHandle;
    return NppExecHelpers::SetClipboardText(params, hWndOwner) ? CMDRESULT_SUCCEEDED : CMDRESULT_FAILED;
}

CScriptEngine::eCmdResult CScriptEngine::DoSet(const tstr& params)
{
    reportCmdAndParams( DoSetCommand::Name(), params, fMessageToConsole );
    
    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;
    bool isInternalMsg = false;
    bool bLocalVar = false;
    bool bDelayedSubst = false;
    bool bDelayedSubstKeyword = false;
    tstr varName;

    // sets the value of user's variable
    if ( !params.IsEmpty() )
    {
        const TCHAR* const DEF_OP  = _T("=");
        const TCHAR* const CALC_OP = _T("~");
        const int i1 = params.Find(DEF_OP);    
        const int i2 = params.Find(CALC_OP);
        const bool bSep1 = ((i1 < i2 && i1 >= 0) || (i2 < 0));
        const TCHAR* const sep = bSep1 ? DEF_OP : CALC_OP;
        CStrSplitT<TCHAR> args;

        if ( args.Split(params, sep, 2) == 2 )
        {
            varName = args.Arg(0);
            isInternalMsg = true;

            tstr& varValue = args.Arg(1);
            NppExecHelpers::StrDelLeadingAnySpaces(varName);
            NppExecHelpers::StrDelTrailingAnySpaces(varName);
            NppExecHelpers::StrDelLeadingAnySpaces(varValue);
            NppExecHelpers::StrDelTrailingAnySpaces(varValue);

            CNppExecMacroVars& MacroVars = m_pNppExec->GetMacroVars();
            MacroVars.CheckAllMacroVars(this, varName, true, CMDTYPE_SET);

            // Note: it is possible to have a variable named "+v" or "-v"
            bDelayedSubst = isDelayedSubstVar(varName, bDelayedSubstKeyword); // checking for "+v" before "local"
            bLocalVar = CNppExecMacroVars::IsLocalMacroVar(varName);
            if ( bLocalVar && varName.IsEmpty() )
            {
                // set local = ...
                varName = _T("LOCAL");
                bLocalVar = false;
            }

            if ( bLocalVar && !bDelayedSubstKeyword)
            {
                bDelayedSubst = isDelayedSubstVar(varName, bDelayedSubstKeyword); // checking for "+v" after "local"
            }

            if ( !bDelayedSubst )
            {
                MacroVars.CheckAllMacroVars(this, varValue, true);
            }

            if ( !bSep1 )
            {
                if ( !CNppExecMacroVars::StrCalc(varValue, m_pNppExec).Process() )
                    nCmdResult = CMDRESULT_FAILED;
            }

            tstr S = bLocalVar ? _T("local ") : _T("");
            S += varName;
            S += _T(" = ");
            S += varValue;

            if ( MacroVars.SetUserMacroVar(this, varName, varValue, bLocalVar ? CNppExecMacroVars::svLocalVar : 0) )
            {
                Runtime::GetLogger().AddEx( _T("; OK: %s%s = %s"), bLocalVar ? _T("local ") : _T(""), varName.c_str(), varValue.c_str() );
            }
            else
            {
                Runtime::GetLogger().AddEx( _T("; failed to set %s%s = %s"), bLocalVar ? _T("local ") : _T(""), varName.c_str(), varValue.c_str() );
                nCmdResult = CMDRESULT_FAILED;
            }
        }
        else
        {
            // "set $(var)" returns the value of var
            // "set" returns all vars and values
            // "set local" returns all local vars and values
            varName = params; // let's check for "set local"
            NppExecHelpers::StrDelLeadingAnySpaces(varName);
            NppExecHelpers::StrDelTrailingAnySpaces(varName);

            // Note: it is possible to have a variable named "+v" or "-v"
            bDelayedSubst = isDelayedSubstVar(varName, bDelayedSubstKeyword); // checking for "+v" before "local"
            bLocalVar = CNppExecMacroVars::IsLocalMacroVar(varName);
            if ( bLocalVar && !bDelayedSubstKeyword)
            {
                bDelayedSubst = isDelayedSubstVar(varName, bDelayedSubstKeyword); // checking for "+v" after "local"
            }

            CNppExecMacroVars::MakeCompleteVarName(varName);
            NppExecHelpers::StrUpper(varName);
        }
    }
    else
    {
        Runtime::GetLogger().Add(   _T("; no arguments given - showing all user\'s variables") );
    }
    
    {
        CCriticalSectionLockGuard lock(m_pNppExec->GetMacroVars().GetCsUserMacroVars());
        const CMacroVars& userLocalMacroVars = m_pNppExec->GetMacroVars().GetUserLocalMacroVars(this);
        const CMacroVars& userMacroVars = m_pNppExec->GetMacroVars().GetUserMacroVars();

        if ( userLocalMacroVars.empty() && bLocalVar )
        {
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( _T("- no user-defined local variables"), nMsgFlags );
            nCmdResult = CMDRESULT_FAILED;
        }
        else if ( userLocalMacroVars.empty() && userMacroVars.empty() )
        {
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( _T("- no user-defined variables"), nMsgFlags );
            nCmdResult = CMDRESULT_FAILED;
        }
        else
        {
            if ( varName.IsEmpty() )
            {
                if ( bLocalVar )
                {
                    PrintMacroVarFunc func(m_pNppExec);
                    for ( const auto& v : userLocalMacroVars )
                    {
                        func(v, true);
                    }
                }
                else
                    IterateUserMacroVars(userMacroVars, userLocalMacroVars, PrintMacroVarFunc(m_pNppExec));
            }
            else
            {
                const CMacroVars& macroVars = bLocalVar ? userLocalMacroVars : userMacroVars;
                CMacroVars::const_iterator itrVar = macroVars.find(varName);
                if ( itrVar != macroVars.end() )
                {
                    tstr S = bLocalVar ? _T("local ") : _T("");
                    S += itrVar->name;
                    S += _T(" = ");
                    if ( itrVar->value.length() > MAX_VAR_LENGTH2SHOW )
                    {
                        S.Append( itrVar->value.c_str(), MAX_VAR_LENGTH2SHOW - 5 );
                        S += _T("(...)");
                    }
                    else
                    {
                        S += itrVar->value;
                    }
                    UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                    if ( isInternalMsg )
                        nMsgFlags |= CNppExecConsole::pfIsInternalMsg;
                    m_pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );
                }
                else
                {
                    tstr t = _T("- no such user\'s ");
                    if ( bLocalVar ) t += _T("local ");
                    t += _T("variable: ");
                    t += varName;
                    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
                    m_pNppExec->GetConsole().PrintMessage( t.c_str(), nMsgFlags );
                    nCmdResult = CMDRESULT_FAILED;
                }
            }
        }
    }

    return nCmdResult;
}

CScriptEngine::eCmdResult CScriptEngine::DoUnset(const tstr& params)
{
    reportCmdAndParams( DoUnsetCommand::Name(), params, fMessageToConsole );

    eCmdResult nCmdResult = CMDRESULT_SUCCEEDED;

    // unsets the user's variable
    if ( !params.IsEmpty() )
    {
        tstr varName = params;
        int k = varName.Find( _T("=") );
        if ( k >= 0 )  varName.SetSize(k);
        NppExecHelpers::StrDelLeadingAnySpaces(varName);
        NppExecHelpers::StrDelTrailingAnySpaces(varName);

        CNppExecMacroVars& MacroVars = m_pNppExec->GetMacroVars();
        MacroVars.CheckAllMacroVars(this, varName, true, CMDTYPE_UNSET);

        bool bLocalVar = CNppExecMacroVars::IsLocalMacroVar(varName);
        if ( bLocalVar && varName.IsEmpty() )
        {
            // unset local
            varName = _T("LOCAL");
            bLocalVar = false;
        }
        unsigned int nFlags = CNppExecMacroVars::svRemoveVar;
        if ( bLocalVar )
            nFlags |= CNppExecMacroVars::svLocalVar;
        if ( MacroVars.SetUserMacroVar(this, varName, _T(""), nFlags) )
        {
            Runtime::GetLogger().AddEx( _T("; OK: %s%s has been removed"), bLocalVar ? _T("local ") : _T(""), varName.c_str() );

            tstr t = _T("- the user\'s ");
            if ( bLocalVar ) t += _T("local ");
            t += _T("variable has been removed: ");
            t += varName;
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
            m_pNppExec->GetConsole().PrintMessage( t.c_str(), nMsgFlags );
        }
        else
        {
            Runtime::GetLogger().AddEx( _T("; failed to unset %s%s (no such user\'s variable)"), bLocalVar ? _T("local ") : _T(""), varName.c_str() );

            tstr t = _T("- no such user\'s ");
            if ( bLocalVar ) t += _T("local ");
            t += _T("variable: ");
            t += varName;
            const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine;
            m_pNppExec->GetConsole().PrintMessage( t.c_str(), nMsgFlags );

            nCmdResult = CMDRESULT_FAILED;
        }
    }
    else
    {
        errorCmdNoParam(DoUnsetCommand::Name());
        nCmdResult = CMDRESULT_INVALIDPARAM;
    }

    return nCmdResult;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

CMacroVars::const_iterator CMacroVars::begin() const noexcept
{
    return m_MacroVars.begin();
}

CMacroVars::iterator CMacroVars::begin() noexcept
{
    return m_MacroVars.begin();
}

CMacroVars::const_iterator CMacroVars::cbegin() const noexcept
{
    return m_MacroVars.cbegin();
}

CMacroVars::const_iterator CMacroVars::cend() const noexcept
{
    return m_MacroVars.cend();
}

CMacroVars::const_iterator CMacroVars::end() const noexcept
{
    return m_MacroVars.end();
}

CMacroVars::iterator CMacroVars::end() noexcept
{
    return m_MacroVars.end();
}

bool CMacroVars::empty() const noexcept
{
    return m_MacroVars.empty();
}

CMacroVars::iterator CMacroVars::erase(const_iterator position)
{
    return m_MacroVars.erase(position);
}

CMacroVars::const_iterator CMacroVars::find(const tstr& varName) const
{
    auto end = m_MacroVars.end();
    auto itr = std::lower_bound( m_MacroVars.begin(), end, varName,
        [](const item_type& v, const tstr& name) { return v.name < name; } );
    return (itr != end && itr->name == varName) ? itr : end;
}

CMacroVars::iterator CMacroVars::find(const tstr& varName)
{
    auto end = m_MacroVars.end();
    auto itr = std::lower_bound( m_MacroVars.begin(), end, varName,
        [](const item_type& v, const tstr& name) { return v.name < name; } );
    return (itr != end && itr->name == varName) ? itr : end;
}

void CMacroVars::swap(CMacroVars& other) noexcept
{
    m_MacroVars.swap(other.m_MacroVars);
}

tstr& CMacroVars::operator[](const tstr& varName)
{
    auto end = m_MacroVars.end();
    auto itr = std::lower_bound( m_MacroVars.begin(), end, varName,
        [](const item_type& v, const tstr& name) { return v.name < name; } );
    if ( itr == end || itr->name != varName )
    {
        if ( m_MacroVars.empty() )
        {
            m_MacroVars.reserve(8);
            itr = end = m_MacroVars.end();
        }
        itr = m_MacroVars.insert(itr, item_type{varName, tstr()});
    }
    return itr->value;
}

tstr& CMacroVars::operator[](tstr&& varName)
{
    auto end = m_MacroVars.end();
    auto itr = std::lower_bound( m_MacroVars.begin(), end, varName,
        [](const item_type& v, const tstr& name) { return v.name < name; } );
    if ( itr == end || itr->name != varName )
    {
        if ( m_MacroVars.empty() )
        {
            m_MacroVars.reserve(8);
            itr = end = m_MacroVars.end();
        }
        itr = m_MacroVars.insert(itr, item_type{varName, tstr()});
    }
    return itr->value;
}

//////////////////////////////////////////////////////////////////////////////

CNppExecMacroVars::CNppExecMacroVars() : m_pNppExec(0)
{
}

CNppExec* CNppExecMacroVars::GetNppExec() const
{
    return m_pNppExec;
}

void CNppExecMacroVars::SetNppExec(CNppExec* pNppExec)
{
    m_pNppExec = pNppExec;
}

bool CNppExecMacroVars::IsLocalMacroVar(tstr& varName)
{
    return CScriptEngine::isLocalParam(varName);
}

bool CNppExecMacroVars::ContainsMacroVar(const tstr& S)
{
    return ( (S.Find(_T("$(")) >= 0) ? true : false );
}

static CListItemT<CScriptEngine::ScriptContext>* getScriptContextItemPtr(CNppExec* pNppExec, CScriptEngine* pScriptEngine)
{
    CListItemT<CScriptEngine::ScriptContext>* pScriptContextItemPtr = NULL;

    if ( pScriptEngine != nullptr )
    {
        CScriptEngine::ExecState& execState = pScriptEngine->GetExecState();
        pScriptContextItemPtr = execState.ScriptContextList.GetLast();
    }
    else
    {
        std::shared_ptr<CScriptEngine> pRunningScriptEngine = pNppExec->GetCommandExecutor().GetRunningScriptEngine();
        if ( pRunningScriptEngine )
        {
            CScriptEngine::ExecState& execState = pRunningScriptEngine->GetExecState();
            pScriptContextItemPtr = execState.ScriptContextList.GetLast();
        }
    }

    return pScriptContextItemPtr;
}

CMacroVars& CNppExecMacroVars::GetUserLocalMacroVars(CScriptEngine* pScriptEngine)
{
    auto pScriptContextItemPtr = getScriptContextItemPtr(m_pNppExec, pScriptEngine);
    
    if ( pScriptContextItemPtr != NULL )
    {
        CScriptEngine::ScriptContext& scriptContext = pScriptContextItemPtr->GetItem();
        return scriptContext.LocalMacroVars;
    }

    return m_UserLocalMacroVars0;
}

CMacroVars& CNppExecMacroVars::GetLocalCmdAliases(CScriptEngine* pScriptEngine)
{
    auto pScriptContextItemPtr = getScriptContextItemPtr(m_pNppExec, pScriptEngine);

    if ( pScriptContextItemPtr != NULL )
    {
        CScriptEngine::ScriptContext& scriptContext = pScriptContextItemPtr->GetItem();
        return scriptContext.LocalCmdAliases;
    }

    return m_LocalCmdAliases0;
}

CMacroVars& CNppExecMacroVars::GetUserConsoleMacroVars()
{
    return m_UserConsoleMacroVars;
}

CMacroVars& CNppExecMacroVars::GetConsoleCmdAliases()
{
    return m_ConsoleCmdAliases;
}

CMacroVars& CNppExecMacroVars::GetUserMacroVars()
{
    return m_UserMacroVars;
}

CMacroVars& CNppExecMacroVars::GetCmdAliases()
{
    return m_CmdAliases;
}

CCriticalSection& CNppExecMacroVars::GetCsUserMacroVars()
{
    return m_csUserMacroVars;
}

CCriticalSection& CNppExecMacroVars::GetCsCmdAliases()
{
    return m_csCmdAliases;
}

bool CNppExecMacroVars::CheckCmdArgs(tstr& Cmd, int& pos, const CStrSplitT<TCHAR>& args)
{
  logInput( _T("CheckCmdArgs()"), Cmd.c_str(), pos );

#ifdef _DEBUG
  assert( StrUnsafeSubCmp(Cmd.c_str() + pos, _T("$(")) == 0 );
#endif
  //if ( StrUnsafeSubCmp(Cmd.c_str() + pos, _T("$(")) == 0 )
  {
    TCHAR szNum[3*sizeof(int) + 2];
    tstr S = Cmd;
    NppExecHelpers::StrUpper(S);

    if ( StrUnsafeSubCmp(S.c_str() + pos, MACRO_ARGC) == 0 )
    {
      c_base::_tint2str(args.GetArgCount(), szNum);
      const int len = lstrlen(MACRO_ARGC);
      Cmd.Replace(pos, len, szNum);
      pos += lstrlen(szNum);

      logOutput( Cmd.c_str() );
      return true;
    }

    int len = -1;
    bool bReverse = false;
    if ( StrUnsafeSubCmp(S.c_str() + pos, MACRO_ARGV) == 0 )
    {
      len = lstrlen(MACRO_ARGV);
      bReverse = false;
    }
    else if ( StrUnsafeSubCmp(S.c_str() + pos, MACRO_RARGV) == 0 )
    {
      len = lstrlen(MACRO_RARGV);
      bReverse = true;
    }

    if ( len >= 0 )
    {
      switch ( S.GetAt(pos+len) )
      {
        case 0:
        {
          const tstr& t = bReverse ? args.GetRArgs() : args.GetArgs();
          Cmd.Replace( pos, len, t.c_str(), t.length() );
          pos += t.length();
          break;
        }
        case _T(')'):
        {
          const tstr& t = bReverse ? args.GetRArgs() : args.GetArgs();
          Cmd.Replace( pos, len + 1, t.c_str(), t.length() );
          pos += t.length();
          break;
        }
        case _T('['):
        {
          unsigned int k = 0;
          TCHAR ch = S.GetAt(pos + len + k + 1);

          szNum[0] = 0;
          while ( isDecNumChar(ch) && (k < 3*sizeof(int) + 1) )
          {
            szNum[k] = ch;
            ++k;
            ch = S.GetAt(pos + len + k + 1);
          }
          szNum[k] = 0;
          while ( (ch != _T(')')) && (ch != 0) )
          {
            ++k;
            ch = S.GetAt(pos + len + k + 1);
          }
          const tstr& t = bReverse ? args.GetRArg( _ttoi(szNum) ) : args.GetArg( _ttoi(szNum) );
          Cmd.Replace( pos, len + k + 2, t.c_str(), t.length() );
          pos += t.length();
          break;
        }
        default:
        {
          // may be "$(ARGVx" or "$(RARGVx"
          logNoOutput();
          return false;
        }
      }

      logOutput( Cmd.c_str() );
      return true;
    }
  }

  logNoOutput();
  return false;
}

void CNppExecMacroVars::CheckCmdAliases(CScriptEngine* pScriptEngine, tstr& S, bool useLogging)
{
    if ( useLogging )
    {
        Runtime::GetLogger().Add(   _T("CheckCmdAliases()") );
        Runtime::GetLogger().Add(   _T("{") );
        Runtime::GetLogger().IncIndentLevel();
    }

    if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOCMDALIASES) )
    {
        if ( useLogging )
        {
            Runtime::GetLogger().Add(   _T("; command aliases enabled") );
            Runtime::GetLogger().AddEx( _T("[in]  \"%s\""), S.c_str() );
        }
        
        //NppExecHelpers::StrDelLeadingAnySpaces(S);
        
        if ( S.length() > 0 )
        {
            int len = 0;
            while ( len < S.length() && !NppExecHelpers::IsAnySpaceChar(S[len]) ) ++len;

            tstr t(S.c_str(), len);
            if ( t.length() > 0 )
            {
                NppExecHelpers::StrUpper(t);

                CCriticalSectionLockGuard lock(GetCsCmdAliases());

                bool bSubstituted = false;
                for ( int n = 0; n < 2 && !bSubstituted; ++n )
                {
                    bool isLocal = (n == 0);
                    CMacroVars& cmdAliases = isLocal ? GetLocalCmdAliases(pScriptEngine) : GetCmdAliases();

                    auto itrAlias = cmdAliases.find(t);
                    if ( itrAlias != cmdAliases.end() )
                    {
                        const auto& v = *itrAlias;
                        S.Replace( 0, v.name.length(), v.value.c_str(), v.value.length() );
                        bSubstituted = true;
                    }
                }
            }
        }

        if ( useLogging )
        {
            Runtime::GetLogger().AddEx( _T("[out] \"%s\""), S.c_str() );
        }
    }
    else
    {
        if ( useLogging )
        {
            Runtime::GetLogger().Add(   _T("; command aliases disabled") );
        }
    }

    if ( useLogging )
    {
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );
    }
}

bool CNppExecMacroVars::CheckNppMacroVars(tstr& S, int& pos)
{
  logInput( _T("CheckNppMacroVars()"), S.c_str(), pos );

#ifdef _DEBUG
  assert( StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0 );
#endif
  //if (StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0)
  {
    typedef struct sNppVarMsg {
        const TCHAR* str;
        UINT msg;
    } tNppVarMsg;

    const int    NPPSTR_COUNT = 10; // strings
    const int    NPPVAR_COUNT = NPPSTR_COUNT + 2; // strings + numbers
    const tNppVarMsg NPPVARMSGS[NPPVAR_COUNT] = {
      // getting strings:
      { MACRO_FILE_FULLPATH,       NPPM_GETFULLCURRENTPATH },
      { MACRO_FILE_DIRPATH,        NPPM_GETCURRENTDIRECTORY },
      { MACRO_FILE_FULLNAME,       NPPM_GETFILENAME },
      { MACRO_FILE_NAMEONLY,       NPPM_GETNAMEPART },
      { MACRO_FILE_EXTONLY,        NPPM_GETEXTPART },
      { MACRO_NPP_DIRECTORY,       NPPM_GETNPPDIRECTORY },
      { MACRO_NPP_FULL_FILE_PATH,  NPPM_GETNPPFULLFILEPATH },
      { MACRO_CURRENT_WORD,        NPPM_GETCURRENTWORD },
      { MACRO_FILE_NAME_AT_CURSOR, NPPM_GETFILENAMEATCURSOR },
      { MACRO_CURRENT_LINESTR,     NPPM_GETCURRENTLINESTR },
      // getting numbers:
      { MACRO_CURRENT_LINE,        NPPM_GETCURRENTLINE },   // (int) line number
      { MACRO_CURRENT_COLUMN,      NPPM_GETCURRENTCOLUMN }  // (int) column number
    };

    tstr Cmd = S;
    NppExecHelpers::StrUpper(Cmd);

    for (int j = 0; j < NPPVAR_COUNT; j++)
    {
      const TCHAR* macro_str = NPPVARMSGS[j].str;

      if (StrUnsafeSubCmp(Cmd.c_str() + pos, macro_str) == 0)
      {
        const int macro_len = lstrlen(macro_str);
        const UINT uNppMsg = NPPVARMSGS[j].msg;
        const int MACRO_SIZE = 16*1024;
        TCHAR     szMacro[MACRO_SIZE];

        szMacro[0] = 0;
        if (j < NPPSTR_COUNT)
        {
          INT_PTR nAnchor = 0, nCaret = 0;

          if (uNppMsg == NPPM_GETFILENAMEATCURSOR)
          {
            HWND hSci = m_pNppExec->GetScintillaHandle();
            nAnchor = (INT_PTR) ::SendMessage(hSci, SCI_GETANCHOR, 0, 0);
            nCaret = (INT_PTR) ::SendMessage(hSci, SCI_GETCURRENTPOS, 0, 0);
            ::SendMessage(hSci, WM_SETREDRAW, FALSE, 0);
          }

          m_pNppExec->SendNppMsg(uNppMsg, (WPARAM) (MACRO_SIZE - 1), (LPARAM) szMacro);

          if (uNppMsg == NPPM_GETFILENAMEATCURSOR)
          {
            HWND hSci = m_pNppExec->GetScintillaHandle();
            ::SendMessage(hSci, SCI_SETSEL, nAnchor, nCaret);
            ::SendMessage(hSci, WM_SETREDRAW, TRUE, 0);
          }
        }
        else
        {
          int nn = (int) m_pNppExec->SendNppMsg(uNppMsg, 0, 0);
          wsprintf(szMacro, _T("%d"), nn);
        }

        int len = lstrlen(szMacro);
        S.Replace(pos, macro_len, szMacro, len);
        pos += len;

        logOutput( S.c_str() );
        return true;
      }
    }
  }

  logNoOutput();
  return false;
}

static tstr uptr2tstr(UINT_PTR uptr)
{
  TCHAR szNum[64];

#ifdef _WIN64
  c_base::_tuint64_to_strhex((unsigned __int64)uptr, szNum);
#else
  c_base::_tuint2strhex((unsigned int)uptr, szNum);
#endif

  tstr S = _T("0x");
  S += szNum;
  return S;
};

static tstr uint2tstr(UINT u)
{
  TCHAR szNum[16];

  c_base::_tuint2str(u, szNum);

  return tstr(szNum);
};

void CNppExecMacroVars::logInput(const TCHAR* funcName, const TCHAR* inputVar, int pos)
{
  Runtime::GetLogger().Add( funcName );
  Runtime::GetLogger().Add(   _T("{") );
  Runtime::GetLogger().IncIndentLevel();
  if ( pos == 0 )
    Runtime::GetLogger().AddEx( _T("[in]  \"%.896s\", pos = 0"), inputVar );
  else
    Runtime::GetLogger().AddEx( _T("[in]  \"%.896s\", pos = %d: \"%.80s\""), inputVar, pos, inputVar + pos );
}

void CNppExecMacroVars::logInput(const TCHAR* funcName, const TCHAR* inputVar)
{
  Runtime::GetLogger().Add( funcName );
  Runtime::GetLogger().Add(   _T("{") );
  Runtime::GetLogger().IncIndentLevel();
  Runtime::GetLogger().AddEx( _T("[in]  \"%.896s\""), inputVar );
}

void CNppExecMacroVars::logOutput(const TCHAR* outputVar)
{
  Runtime::GetLogger().AddEx( _T("[out] \"%s\""), outputVar );
  Runtime::GetLogger().DecIndentLevel();
  Runtime::GetLogger().Add(   _T("}") );
}

void CNppExecMacroVars::logNoOutput()
{
  Runtime::GetLogger().Add(   _T("; no changes") );
  Runtime::GetLogger().DecIndentLevel();
  Runtime::GetLogger().Add(   _T("}") );
}

bool CNppExecMacroVars::substituteMacroVar(const tstr& Cmd, tstr& S, int& pos,
                                           const TCHAR* varName,
                                           tstr (*getValue)(CNppExec* pNppExec) )
{
  if (StrUnsafeSubCmp(Cmd.c_str() + pos, varName) == 0)
  {
    tstr varValue = getValue(m_pNppExec);
    int nVarNameLen = lstrlen(varName);

    S.Replace(pos, nVarNameLen, varValue);
    pos += varValue.length();

    return true;
  }

  return false;
}

bool CNppExecMacroVars::CheckPluginMacroVars(tstr& S, int& pos)
{
  logInput( _T("CheckPluginMacroVars()"), S.c_str(), pos );

#ifdef _DEBUG
  assert( StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0 );
#endif
  //if (StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0)
  {
    tstr Cmd = S;
    NppExecHelpers::StrUpper(Cmd);

    if (StrUnsafeSubCmp(Cmd.c_str() + pos, MACRO_DOCNUMBER) == 0)
    {
      const int len = lstrlen(MACRO_DOCNUMBER); // "$(#"
      int   k;
      tstr  snum = _T("");
      
      for (k = pos + len; k < Cmd.length(); k++)
      {
        if (isDecNumChar(Cmd[k]))
          snum += Cmd[k];
        else
          break;
      }
      for (; k < Cmd.length(); k++)
      {
        if (Cmd[k] == _T(')'))
          break;
      }
      k -= pos;
      S.Delete(pos, k+1);

      if (snum.length() > 0)
      {
        k = _ttoi(snum.c_str());
        if (k > 0) 
        {
          // #doc = 1..nbFiles
          int nView = (int) m_pNppExec->SendNppMsg(NPPM_GETCURRENTVIEW);
          tstr fileName = m_pNppExec->nppGetOpenFileName(k - 1, nView);
          S.Insert(pos, fileName);
          pos += fileName.length();
        }
        else if (k == 0)
        {
          const int MACRO_SIZE = 0x200;
          TCHAR     szMacro[MACRO_SIZE];

          // #doc = 0 means notepad++ full path
          ::GetModuleFileName(NULL, szMacro, MACRO_SIZE-1);
          S.Insert(pos, szMacro);
          pos += lstrlen(szMacro);
        }
      }

      logOutput( S.c_str() );
      return true;
    }

    if (StrUnsafeSubCmp(Cmd.c_str() + pos, MACRO_SYSVAR) == 0)
    {
      const int len = lstrlen(MACRO_SYSVAR); // $(SYS.
      bool bSysVarOK = false;
      int  i1 = pos + len;
      int  i2 = i1;
      unsigned int nBracketDepth = 0;
      for (; i2 < Cmd.length(); ++i2)
      {
        if (Cmd[i2] == _T(')'))
        {
          if (nBracketDepth != 0)
            --nBracketDepth;
          else
            break;
        }
        else if (Cmd[i2] == _T('('))
        {
          ++nBracketDepth;
        }
      }
      tstr sub(Cmd.c_str() + i1, i2 - i1);
      if (sub.length() > 0)
      {
        tstr sValue = NppExecHelpers::GetEnvironmentVar(sub);
        if (!sValue.IsEmpty())
        {
          S.Replace(pos, i2 - pos + 1, sValue);
          pos += sValue.length();
          bSysVarOK = true;
        }
      }
      if (!bSysVarOK)
      {
        S.Replace(pos, i2 - pos + 1, _T(""), 0);
      }

      logOutput( S.c_str() );
      return true;
    }

    if ( substituteMacroVar(
           Cmd, S, pos,
           MACRO_SELECTED_TEXT,
           [](CNppExec* pNppExec)
           {
             return pNppExec->sciGetSelText();
           }
         ) || 
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_LEFT_VIEW_FILE,
           [](CNppExec* pNppExec)
           {
             int ind = (int) pNppExec->SendNppMsg(NPPM_GETCURRENTDOCINDEX, MAIN_VIEW, MAIN_VIEW);
             return pNppExec->nppGetOpenFileName(ind, MAIN_VIEW);
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_RIGHT_VIEW_FILE,
           [](CNppExec* pNppExec)
           {
             int ind = (int) pNppExec->SendNppMsg(NPPM_GETCURRENTDOCINDEX, SUB_VIEW, SUB_VIEW);
             return pNppExec->nppGetOpenFileName(ind, SUB_VIEW);
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_CURRENT_WORKING_DIR,
           [](CNppExec* )
           {
             return NppExecHelpers::GetCurrentDir();
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_PLUGINS_CONFIG_DIR,
           [](CNppExec* pNppExec)
           {
             return tstr(pNppExec->getConfigPath());
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_CLIPBOARD_TEXT,
           [](CNppExec* )
           {
             return NppExecHelpers::GetClipboardText();
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_CLOUD_LOCATION_PATH,
           [](CNppExec* pNppExec)
           {
             return pNppExec->nppGetSettingsCloudPath();
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_NPP_HWND,
           [](CNppExec* pNppExec)
           {
             return uptr2tstr((UINT_PTR)(pNppExec->m_nppData._nppHandle));
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_NPP_PID,
           [](CNppExec* )
           {
             return uint2tstr(::GetCurrentProcessId());
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_SCI_HWND,
           [](CNppExec* pNppExec)
           {
             return uptr2tstr((UINT_PTR)(pNppExec->GetScintillaHandle()));
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_SCI_HWND1,
           [](CNppExec* pNppExec)
           {
             return uptr2tstr((UINT_PTR)(pNppExec->m_nppData._scintillaMainHandle));
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_SCI_HWND2,
           [](CNppExec* pNppExec)
           {
             return uptr2tstr((UINT_PTR)(pNppExec->m_nppData._scintillaSecondHandle));
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_CON_HWND,
           [](CNppExec* pNppExec)
           {
             return uptr2tstr((UINT_PTR)(pNppExec->GetConsole().GetConsoleWnd()));
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_FOCUSED_HWND,
           [](CNppExec* )
           {
             return uptr2tstr((UINT_PTR)(NppExecHelpers::GetFocusedWnd()));
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_WORKSPACE_ITEM_PATH,
           [](CNppExec* pNppExec)
           {
             tstr sWorkspaceItem;
             pNppExec->nppGetWorkspaceItemPath(sWorkspaceItem);
             return sWorkspaceItem;
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_WORKSPACE_ITEM_DIR,
           [](CNppExec* pNppExec)
           {
             tstr sWorkspaceItem;
             if (pNppExec->nppGetWorkspaceItemPath(sWorkspaceItem))
             {
               sWorkspaceItem = NppExecHelpers::GetFileNamePart(sWorkspaceItem, NppExecHelpers::fnpDirPath);
               if (sWorkspaceItem.EndsWith(_T('\\')) || sWorkspaceItem.EndsWith(_T('/')))
                 sWorkspaceItem.DeleteLastChar();
             }
             return sWorkspaceItem;
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_WORKSPACE_ITEM_NAME,
           [](CNppExec* pNppExec)
           {
             tstr sWorkspaceItem;
             if (pNppExec->nppGetWorkspaceItemPath(sWorkspaceItem))
             {
               sWorkspaceItem = NppExecHelpers::GetFileNamePart(sWorkspaceItem, NppExecHelpers::fnpNameExt);
             }
             return sWorkspaceItem;
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_WORKSPACE_ITEM_ROOT,
           [](CNppExec* pNppExec)
           {
             tstr sWorkspaceItem;
             pNppExec->nppGetWorkspaceRootItemPath(sWorkspaceItem);
             return sWorkspaceItem;
           }
         ) ||
         substituteMacroVar(
           Cmd, S, pos,
           MACRO_IS_PROCESS,
           [](CNppExec* pNppExec)
           {
             TCHAR ch = pNppExec->GetCommandExecutor().IsChildProcessRunning() ? _T('1') : _T('0');
             return tstr(ch);
           }
         )
       )
    {
      logOutput( S.c_str() );
      return true;
    }
  }

  logNoOutput();
  return false;
}

bool CNppExecMacroVars::CheckUserMacroVars(CScriptEngine* pScriptEngine, tstr& S, int& pos)
{
  logInput( _T("CheckUserMacroVars()"), S.c_str(), pos );

#ifdef _DEBUG
  assert( ContainsMacroVar(S) );
#endif
  //if ( ContainsMacroVar(S) )
  {
  #ifdef _DEBUG
    assert( !CheckInnerMacroVars(pScriptEngine, S, pos, false) ); // only assert, no logging here!
  #endif

    bool bResult = false;

    {
      CCriticalSectionLockGuard lock(GetCsUserMacroVars());
      const CMacroVars& userLocalMacroVars = GetUserLocalMacroVars(pScriptEngine);
      const CMacroVars& userMacroVars = GetUserMacroVars();
      bResult = IterateUserMacroVars(userMacroVars, userLocalMacroVars, SubstituteMacroVarFunc(S, pos));
    }

    if ( bResult )
    {
      logOutput( S.c_str() );
      return true;
    }
  }

  logNoOutput();
  return false;
}

bool CNppExecMacroVars::CheckEmptyMacroVars(tstr& S, int& pos)
{
  // Note: be sure to check the value of OPTB_CONSOLE_NOEMPTYVARS
  // _before_ you call CheckEmptyMacroVars!!!

  logInput( _T("CheckEmptyMacroVars()"), S.c_str(), pos );

#ifdef _DEBUG
  assert( StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0 );
#endif
  //if ( StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0 )
  //{
    int j = pos + 2;
    unsigned int nBracketDepth = 0;
    for ( ; j < S.length(); ++j )
    {
      const TCHAR ch = S[j];
      if ( ch == _T(')') )
      {
        if ( nBracketDepth != 0 )
          --nBracketDepth;
        else
          break;
      }
      else if ( ch == _T('(') )
      {
        ++nBracketDepth;
      }
    }
    
    if ( j < S.length() )
      S.Delete(pos, j - pos + 1);
    else
      S.Delete(pos, -1);

    logOutput( S.c_str() );
    return true;
  //}

  //logNoOutput();
  //return false;
}

bool CNppExecMacroVars::CheckAllMacroVars(CScriptEngine* pScriptEngine, tstr& S, bool useLogging, int nCmdType )
{
    // Note: pScriptEngine can be nullptr!

    if ( useLogging )
    {
        logInput( _T("CheckAllMacroVars()"), S.c_str() );
    }

    int pos = S.Find(_T("$("));
    if ( pos >= 0 )
    {
        bool isLoggerActive = true;

        if ( !useLogging )
        {
            isLoggerActive = Runtime::GetLogger().IsActive();
            if ( isLoggerActive )
                Runtime::GetLogger().Activate(false);
        }

        while ( pos >= 0 )
        {
            if ( !CheckInnerMacroVars(pScriptEngine, S, pos, useLogging) )
            {
                if ( nCmdType == CScriptEngine::CMDTYPE_SET ||  // to keep $(varName) in 'set $(varName) = ...'
                     nCmdType == CScriptEngine::CMDTYPE_UNSET ) // to keep $(varName) in 'unset $(varName)'
                {
                    pos += 2; // right after "$("
                }
                else
                {
                    bool bCmdArg = false;
                    if ( nCmdType != CScriptEngine::CMDTYPE_UNSET )
                    {
                        if ( pScriptEngine && pScriptEngine->GetExecState().ScriptContextList.GetLast() )
                        {
                            const CScriptEngine::ScriptContext& scriptContext = pScriptEngine->GetExecState().GetCurrentScriptContext();
                            const CStrSplitT<TCHAR>& args = scriptContext.Args;
                            bCmdArg = CheckCmdArgs(S, pos, args);
                        }
                        else
                        {
                            CStrSplitT<TCHAR> args;
                            bCmdArg = CheckCmdArgs(S, pos, args);
                        }
                    }

                    if ( !bCmdArg &&
                         !CheckNppMacroVars(S, pos) &&
                         !CheckPluginMacroVars(S, pos) )
                    {
                        if ( !CheckUserMacroVars(pScriptEngine, S, pos) )
                        {
                            if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOEMPTYVARS) ||
                                 !CheckEmptyMacroVars(S, pos) )
                            {
                                pos += 2; // right after "$("
                            }
                        }
                    }
                }
            }

            pos = S.Find(_T("$("), pos);
        }

        if ( !useLogging )
        {
            if ( isLoggerActive )
                Runtime::GetLogger().Activate(true);
        }
        else
        {
            logOutput( S.c_str() );
        }

        return true;
    }

    if ( useLogging )
    {
        logNoOutput();
    }
    return false;
}

bool CNppExecMacroVars::CheckInnerMacroVars(CScriptEngine* pScriptEngine, tstr& S, int& pos, bool useLogging)
{
    // Note: pScriptEngine can be nullptr!

    if ( useLogging )
    {
        logInput( _T("CheckInnerMacroVars()"), S.c_str(), pos );
    }

#ifdef _DEBUG
    assert( StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0 );
#endif
    //if (StrUnsafeSubCmp(S.c_str() + pos, _T("$(")) == 0)
    {
        int n1 = pos; // outer "$("
        //  $( ...
        //  n1

        int n2 = S.Find(_T("$("), n1 + 2); // inner "$(" or next "$("
        if ( n2 >= 0 )
        {
            //  $( ... $(
            //  n1     n2
            unsigned int nBracketDepth = 0;
            int n1_end = n1 + 2;
            for ( ; n1_end < S.length(); ++n1_end )
            {
                const TCHAR ch = S[n1_end];
                if ( ch == _T(')') )
                {
                    if ( nBracketDepth != 0 )
                        --nBracketDepth;
                    else
                        break;
                }
                else if ( ch == _T('(') )
                {
                    ++nBracketDepth;
                }
            }

            if ( n2 < n1_end )
            {
                //  $( ... $( ... )  ...
                //  n1     n2     n1_end
                tstr SubVal(S.c_str() + n2, n1_end - n2);

                if ( useLogging )
                {
                    Runtime::GetLogger().Add(   _T("; checking inner vars...") );
                }

                CheckAllMacroVars(pScriptEngine, SubVal, useLogging);
                if ( !m_pNppExec->GetOptions().GetBool(OPTB_CONSOLE_NOEMPTYVARS) )
                {
                    // forcing no empty vars
                    int sub_pos = 0;
                    while ( (sub_pos = SubVal.Find(_T("$("), sub_pos)) >= 0 )
                    {
                        CheckEmptyMacroVars(SubVal, sub_pos);
                    }
                }

                S.Replace(n2, n1_end - n2, SubVal);
                
                if ( useLogging )
                {
                    logOutput( S.c_str() );
                }
                return true;
            }
        }
    }

    if ( useLogging )
    {
        logNoOutput();
    }
    return false;
}

bool CNppExecMacroVars::SetUserMacroVar(CScriptEngine* pScriptEngine, tstr& varName, const tstr& varValue, unsigned int nFlags )
{
  bool bSuccess = false;

  if ( varName.length() > 0 )
  {
    CNppExecMacroVars::MakeCompleteVarName(varName);
    NppExecHelpers::StrUpper(varName);

    {
      CCriticalSectionLockGuard lock(GetCsUserMacroVars());
      CMacroVars& macroVars = (nFlags & svLocalVar) != 0 ? GetUserLocalMacroVars(pScriptEngine) : GetUserMacroVars();
      CMacroVars::iterator itrVar = macroVars.find(varName);
      if ( itrVar != macroVars.end() )
      {
        // existing variable
        if ( (nFlags & svRemoveVar) == 0 )
          itrVar->value = varValue;
        else
          macroVars.erase(itrVar);
        bSuccess = true;
      }
      else if ( (nFlags & svRemoveVar) == 0 )
      {
        // new variable
        macroVars[varName] = varValue;
        bSuccess = true;
      }
    }
  }

  if ( bSuccess )
  {
    // handling "special" variables
    if ( (varName == MACRO_EXIT_CMD) || (varName == MACRO_EXIT_CMD_SILENT) )
    {
      // we need it for the both "set" and "unset"
      
      if ( (nFlags & svRemoveVar) == 0 )
      {
        // either @EXIT_CMD or @EXIT_CMD_SILENT can exist
        tstr pairVarName;
        if ( varName == MACRO_EXIT_CMD )
          pairVarName = MACRO_EXIT_CMD_SILENT;
        else
          pairVarName = MACRO_EXIT_CMD;
        SetUserMacroVar(pScriptEngine, pairVarName, varValue, (nFlags & svLocalVar) | svRemoveVar); // remove the other one
      }

      std::shared_ptr<CScriptEngine> pParentScriptEngine;
      bool bShareLocalVars = false;

      if ( pScriptEngine != nullptr )
      {
        pScriptEngine->SetTriedExitCmd(false);
        bShareLocalVars = pScriptEngine->IsSharingLocalVars();
        pParentScriptEngine = pScriptEngine->GetParentScriptEngine();
      }
      else
      {
        std::shared_ptr<CScriptEngine> pRunningScriptEngine = m_pNppExec->GetCommandExecutor().GetRunningScriptEngine();
        if ( pRunningScriptEngine )
        {
          pRunningScriptEngine->SetTriedExitCmd(false);
          bShareLocalVars = pRunningScriptEngine->IsSharingLocalVars();
          pParentScriptEngine = pRunningScriptEngine->GetParentScriptEngine();
        }
      }

      while ( pParentScriptEngine && bShareLocalVars )
      {
        pParentScriptEngine->SetTriedExitCmd(false);
        bShareLocalVars = pParentScriptEngine->IsSharingLocalVars();
        pParentScriptEngine = pParentScriptEngine->GetParentScriptEngine();
      }
    }
  }

  return bSuccess;
}

void CNppExecMacroVars::MakeCompleteVarName(tstr& varName)
{
    if ( !varName.IsEmpty() )
    {
        if ( varName.StartsWith(_T("$(")) == false )
            varName.Insert(0, _T("$("));
        if ( varName.GetLastChar() != _T(')') )
            varName.Append(_T(')'));

        int n1 = varName.Count(_T('('));
        int n2 = varName.Count(_T(')'));
        while ( n1 > n2 )
        {
            varName.Append(_T(')'));
            ++n2;
        }
    }
}


CNppExecMacroVars::StrCalc::StrCalc(tstr& varValue, CNppExec* pNppExec)
  : m_varValue(varValue), m_pNppExec(pNppExec), m_calcType(CT_FPARSER), m_pVar(0)
{
}

bool CNppExecMacroVars::StrCalc::Process()
{
    m_calcType = StrCalc::CT_FPARSER;
    m_param.Clear();
        
    // check for 'strlen', 'strupper', 'strlower', 'substr' and so on
    m_pVar = m_varValue.c_str();
    m_pVar = get_param(m_pVar, m_param);
    if ( m_param.length() >= 3 )
    {
        typedef struct sCalcType {
            const TCHAR* szCalcType;
            int nCalcType;
        } tCalcType;

        static const tCalcType arrCalcType[] = {
            { _T("STRLENSCI"),   CT_STRLENSCI   },
            { _T("STRLENS"),     CT_STRLENSCI   },
            { _T("STRLENUTF8"),  CT_STRLENUTF8  },
            { _T("STRLENU"),     CT_STRLENUTF8  },
            { _T("STRLENA"),     CT_STRLEN      },
            { _T("STRLEN"),      CT_STRLEN      },
            { _T("STRUPPER"),    CT_STRUPPER    },
            { _T("STRLOWER"),    CT_STRLOWER    },
            { _T("SUBSTR"),      CT_SUBSTR      },
            { _T("STRFIND"),     CT_STRFIND     },
            { _T("STRRFIND"),    CT_STRRFIND    },
            { _T("STRREPLACE"),  CT_STRREPLACE  },
            { _T("STRRPLC"),     CT_STRREPLACE  },
            { _T("STRQUOTE"),    CT_STRQUOTE    },
            { _T("STRUNQUOTE"),  CT_STRUNQUOTE  },
            { _T("STRESCAPE"),   CT_STRESCAPE   },
            { _T("STRUNESCAPE"), CT_STRUNESCAPE },
            { _T("STREXPAND"),   CT_STREXPAND   },
            { _T("STRFROMHEX"),  CT_STRFROMHEX  },
            { _T("STRTOHEX"),    CT_STRTOHEX    },
            { _T("CHR"),         CT_CHR         },
            { _T("ORD"),         CT_ORD         },
            { _T("ORDX"),        CT_ORDX        },
            { _T("NORMPATH"),    CT_NORMPATH    },
            { _T("FILEEXISTS"),  CT_FILEEXISTS  },
            { _T("DIREXISTS"),   CT_DIREXISTS   }
        };

        NppExecHelpers::StrUpper(m_param);

        for ( const tCalcType& ct : arrCalcType )
        {
            if ( m_param == ct.szCalcType )
            {
                m_calcType = ct.nCalcType;
                break;
            }
        }
    }

    bool bSucceded = false;

    switch ( m_calcType )
    {
        case CT_FPARSER:
            bSucceded = calcFParser(); // calc
            break;
        case CT_STRLEN:
        case CT_STRLENUTF8:
        case CT_STRLENSCI:
            bSucceded = calcStrLen();
            break;
        case CT_STRUPPER:
        case CT_STRLOWER:
            bSucceded = calcStrCase();
            break;
        case CT_SUBSTR:
            bSucceded = calcSubStr();
            break;
        case CT_STRFIND:
        case CT_STRRFIND:
            bSucceded = calcStrFind();
            break;
        case CT_STRREPLACE:
            bSucceded = calcStrRplc();
            break;
        case CT_STRQUOTE:
        case CT_STRUNQUOTE:
            bSucceded = calcStrQuote();
            break;
        case CT_STRESCAPE:
        case CT_STRUNESCAPE:
            bSucceded = calcStrEscape();
            break;
        case CT_STREXPAND:
            bSucceded = calcStrExpand();
            break;
        case CT_STRFROMHEX:
            bSucceded = calcStrFromHex();
            break;
        case CT_STRTOHEX:
            bSucceded = calcStrToHex();
            break;
        case CT_CHR:
            bSucceded = calcChr();
            break;
        case CT_ORD:
        case CT_ORDX:
            bSucceded = calcOrd();
            break;
        case CT_NORMPATH:
            bSucceded = calcNormPath();
            break;
        case CT_FILEEXISTS:
        case CT_DIREXISTS:
            bSucceded = calcFileExists();
            break;
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcFParser()
{
    tstr calcError;
    bool bSucceded = g_fp.Calculate(m_pNppExec, m_varValue, calcError, m_varValue);

    if ( bSucceded )
    {
      
        Runtime::GetLogger().AddEx( _T("; fparser calc result: %s"), m_varValue.c_str() );
    
    }
    else
    {
        calcError.Insert( 0, _T("- fparser calc error: ") );
        m_pNppExec->GetConsole().PrintError( calcError.c_str() );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcStrLen()
{
    int len = 0;

    if ( *m_pVar )
    {
        if ( m_calcType == CT_STRLENSCI )
        {
            HWND hSci = m_pNppExec->GetScintillaHandle();
            int nSciCodePage = (int) ::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0);
            if ( nSciCodePage == SC_CP_UTF8 )
                m_calcType = CT_STRLENUTF8;
            else
                m_calcType = CT_STRLEN;

            Runtime::GetLogger().AddEx( _T("; scintilla's encoding: %d"), nSciCodePage );
        }

        if ( m_calcType == CT_STRLEN )
        {
            len = GetStrUnsafeLength(m_pVar);
        }
        else
        {
            #ifdef UNICODE
                len = ::WideCharToMultiByte(CP_UTF8, 0, m_pVar, -1, NULL, 0, NULL, NULL);
            #else
                wchar_t* pwVar = SysUniConv::newMultiByteToUnicode(m_pVar);
                len = ::WideCharToMultiByte(CP_UTF8, 0, pwVar, -1, NULL, 0, NULL, NULL);
                delete [] pwVar;
            #endif

            --len; // without trailing '\0'
        }
    }

    TCHAR szNum[50];
    c_base::_tint2str( len, szNum );
    m_varValue = szNum;

    Runtime::GetLogger().AddEx( 
      _T("; strlen%s: %s"), 
      (m_calcType == CT_STRLEN) ? _T("") : _T("utf8"),
      m_varValue.c_str() 
    );

    return true;
}

bool CNppExecMacroVars::StrCalc::calcStrCase()
{
    if ( *m_pVar )
    {
        m_varValue = m_pVar;
        if ( m_calcType == CT_STRUPPER )
            NppExecHelpers::StrUpper(m_varValue);
        else
            NppExecHelpers::StrLower(m_varValue);
    }
    else
    {
        m_varValue.Clear();
    }

    Runtime::GetLogger().AddEx( 
      _T("; %s: %s"), 
      (m_calcType == CT_STRUPPER) ? _T("strupper") : _T("strlower"),
      m_varValue.c_str() 
    );

    return true;
}

bool CNppExecMacroVars::StrCalc::calcSubStr()
{
    bool bSucceded = false;

    if ( *m_pVar )
    {
        m_pVar = get_param(m_pVar, m_param);
        if ( isDecNumChar(m_param.GetAt(0)) || 
             (m_param.GetAt(0) == _T('-') && isDecNumChar(m_param.GetAt(1))) )
        {
            int pos = c_base::_tstr2int( m_param.c_str() );
            if ( *m_pVar )
            {
                m_pVar = get_param(m_pVar, m_param);
                if ( isDecNumChar(m_param.GetAt(0)) ||
                     (m_param.GetAt(0) == _T('-')) )
                {
                    int len = lstrlen(m_pVar);
                    int count = (m_param == _T("-")) ? len : c_base::_tstr2int( m_param.c_str() );

                    if ( pos < 0 )
                    {
                        // get pos characters from the end of string
                        pos += len;
                        if ( pos < 0 )
                            pos = 0;
                    }
                    else
                    {
                        if ( pos > len )
                            count = 0;
                    }

                    if ( count < 0 )
                    {
                        count += (len - pos);
                        if ( count < 0 )
                            count = 0;
                    }

                    if ( len < pos )
                    {
                        count = 0;
                    }
                    else if ( count > len - pos )
                    {
                        count = len - pos;
                    }

                    if ( count == 0 )
                        m_varValue.Clear();
                    else
                        m_varValue.Assign(m_pVar + pos, count);

                    Runtime::GetLogger().AddEx( 
                      _T("; substr: %s"), 
                      m_varValue.c_str() 
                    );

                    bSucceded = true;
                }
                else
                {
                    m_pNppExec->GetConsole().PrintError( _T("- failed to get 2nd parameter of SUBSTR: a number or \'-\' expected") );
                }
            }
            else
            {
                m_pNppExec->GetConsole().PrintError( _T("- failed to get 2nd parameter of SUBSTR") );
            }
        }
        else
        {
            m_pNppExec->GetConsole().PrintError( _T("- failed to get 1st parameter of SUBSTR: a number expected") );
        }
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- failed to get 1st parameter of SUBSTR") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcStrFind()
{
    bool bSucceded = false;
    CStrSplitT<TCHAR> args;

    const int n = args.SplitToArgs(m_pVar);
    if ( n == 2 )
    {
        const tstr& S = args.GetArg(0);
        const tstr& SFind = args.GetArg(1);
        int pos = (m_calcType == CT_STRFIND) ? S.Find(SFind) : S.RFind(SFind);

        TCHAR szNum[50];
        c_base::_tint2str( pos, szNum );
        m_varValue = szNum;

        Runtime::GetLogger().AddEx( 
          _T("; %s: %s"), 
          (m_calcType == CT_STRFIND) ? _T("strfind") : _T("strrfind"),
          m_varValue.c_str() 
        );

        bSucceded = true;
    }
    else if ( n < 2 )
    {
        m_pNppExec->GetConsole().PrintError( _T("- not enough STRFIND parameters given: 2 parameters expected") );
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- too much STRFIND parameters given: 2 parameters expected") );
        m_pNppExec->GetConsole().PrintError( _T("- try to enclose the STRFIND parameters with quotes, e.g. \"s\" \"sfind\"") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcStrRplc()
{
    bool bSucceded = false;
    CStrSplitT<TCHAR> args;

    const int n = args.SplitToArgs(m_pVar);
    if ( n == 3 )
    {
        const tstr& SFind = args.GetArg(1);
        const tstr& SReplace = args.GetArg(2);
        m_varValue = args.GetArg(0);
        m_varValue.Replace( SFind.c_str(), SReplace.c_str() );

        Runtime::GetLogger().AddEx( 
          _T("; strreplace: %s"), 
          m_varValue.c_str() 
        );

        bSucceded = true;
    }
    else if ( n < 3 )
    {
        m_pNppExec->GetConsole().PrintError( _T("- not enough STRREPLACE parameters given: 3 parameters expected") );
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- too much STRREPLACE parameters given: 3 parameters expected") );
        m_pNppExec->GetConsole().PrintError( _T("- try to enclose the STRREPLACE parameters with quotes, e.g. \"s\" \"sfind\" \"sreplace\"") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcStrQuote()
{
    const bool isQuote = (m_calcType == CT_STRQUOTE);
    tstr S(m_pVar);

    if ( isQuote ) // strquote
    {
        if ( NppExecHelpers::IsStrNotQuotedEx(S) )
            NppExecHelpers::StrQuote(S);
    }
    else // strunquote
    {
        NppExecHelpers::StrUnquoteEx(S);
    }

    m_varValue.Swap(S);

    Runtime::GetLogger().AddEx( 
      _T("; %s: %s"), 
      isQuote ? _T("strquote") : _T("strunquote"),
      m_varValue.c_str() 
    );

    return true;
}

bool CNppExecMacroVars::StrCalc::calcStrEscape()
{
    const bool isEscape = (m_calcType == CT_STRESCAPE);
    tstr S(m_pVar);

    if ( isEscape ) // strescape
    {
        NppExecHelpers::StrEscape(S);
    }
    else // strunescape
    {
        NppExecHelpers::StrUnescape(S);
    }

    m_varValue.Swap(S);

    Runtime::GetLogger().AddEx( 
      _T("; %s: %s"), 
      isEscape ? _T("strescape") : _T("strunescape"),
      m_varValue.c_str() 
    );

    return true;
}

bool CNppExecMacroVars::StrCalc::calcStrExpand()
{
    if ( *m_pVar )
    {
        tstr S(m_pVar);
        std::shared_ptr<CScriptEngine> pScriptEngine = m_pNppExec->GetCommandExecutor().GetRunningScriptEngine();
        m_pNppExec->GetMacroVars().CheckAllMacroVars(pScriptEngine.get(), S, true);
        m_varValue.Swap(S);
    }
    else
    {
        m_varValue.Clear();
    }

    Runtime::GetLogger().AddEx( 
        _T("; strexpand: %s"), 
        m_varValue.c_str() 
    );

    return true;
}

bool CNppExecMacroVars::StrCalc::calcStrFromHex()
{
    bool bSucceded = false;

    if ( *m_pVar )
    {
        tstr hexStr = m_pVar;
        m_varValue.Reserve(hexStr.length());
        int nBytes = c_base::_thexstrex2buf(hexStr.c_str(), (c_base::byte_t *)(m_varValue.c_str()), (m_varValue.GetMemSize() - 1)*sizeof(TCHAR));
        if ( nBytes % sizeof(TCHAR) != 0 )
        {
            ((c_base::byte_t *)(m_varValue.c_str()))[nBytes] = 0;
            ++nBytes;
        }
        m_varValue.SetLengthValue(nBytes / sizeof(TCHAR));

        Runtime::GetLogger().AddEx( 
          _T("; strfromhex: %s"), 
          m_varValue.c_str() 
        );

        bSucceded = true;
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- not enough STRFROMHEX parameters given: 1 parameter expected") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcStrToHex()
{
    bool bSucceded = false;

    if ( *m_pVar )
    {
        tstr Str = m_pVar;
        NppExecHelpers::StrUnquoteEx(Str);

        m_varValue.Reserve(1 + Str.length() * sizeof(TCHAR) * 3);
        int nLen = c_base::_tbuf2hexstr((const c_base::byte_t *)(Str.c_str()), Str.length()*sizeof(TCHAR), 
                                        m_varValue.data(), m_varValue.GetMemSize(),
                                       _T(" "));
        m_varValue.SetLengthValue(nLen);

        Runtime::GetLogger().AddEx( 
          _T("; strtohex: %s"), 
          m_varValue.c_str() 
        );

        bSucceded = true;
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- not enough STRTOHEX parameters given: 1 parameter expected") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcChr()
{
    bool bSucceded = false;

    if ( *m_pVar )
    {
        const TCHAR ch = static_cast<TCHAR>(c_base::_tstr2int(m_pVar));
        if ( ch != 0 )
        {
            m_varValue = ch;

            Runtime::GetLogger().AddEx( 
              _T("; chr: %s"), 
              m_varValue.c_str() 
            );

            bSucceded = true;
        }
        else
        {
            m_pNppExec->GetConsole().PrintError( _T("- wrong CHR parameter given: a character code expected") );
        }
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- not enough CHR parameters given: 1 parameter expected") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcOrd()
{
    bool bSucceded = false;

    if ( *m_pVar )
    {
        tstr Str = m_pVar;
        NppExecHelpers::StrUnquoteEx(Str);

        if ( Str.length() == 1 )
        {
            const bool isHex = (m_calcType == CT_ORDX);
            const int n = static_cast<int>(Str[0]);

            const TCHAR* szFmt = _T("%d");
            if ( isHex )
            {
                szFmt = (n > 0xFF) ? _T("0x%04X") : _T("0x%X");
            }
            m_varValue.Format(16, szFmt, n);

            Runtime::GetLogger().AddEx( 
              _T("; %s: %s"), 
              isHex ? _T("ordx") : _T("ord"),
              m_varValue.c_str() 
            );

            bSucceded = true;
        }
        else
        {
            m_pNppExec->GetConsole().PrintError( _T("- wrong ORD parameter given: 1 character expected") );
        }
    }
    else
    {
        m_pNppExec->GetConsole().PrintError( _T("- not enough ORD parameters given: 1 parameter expected") );
    }

    return bSucceded;
}

bool CNppExecMacroVars::StrCalc::calcNormPath()
{
    if ( *m_pVar )
    {
        tstr path(m_pVar);
        const bool isQuoted = NppExecHelpers::IsStrQuoted(path);
        if ( isQuoted )
        {
            NppExecHelpers::StrUnquote(path);
        }
        path = NppExecHelpers::NormalizePath(path);
        if ( isQuoted )
        {
            NppExecHelpers::StrQuote(path);
        }
        m_varValue.Swap(path);
    }
    else
    {
        m_varValue.Clear();
    }

    Runtime::GetLogger().AddEx( 
      _T("; normpath: %s"), 
      m_varValue.c_str() 
    );

    return true;
}

bool CNppExecMacroVars::StrCalc::calcFileExists()
{
    bool exists = false;

    if ( *m_pVar )
    {
        tstr Path = m_pVar;
        NppExecHelpers::StrUnquote(Path);

        if ( m_calcType == CT_FILEEXISTS )
            exists = NppExecHelpers::CheckFileExists(Path);
        else
            exists = NppExecHelpers::CheckDirectoryExists(Path);
    }

    m_varValue = exists ? _T("1") : _T("0");

    Runtime::GetLogger().AddEx(
      _T("; %s: %s"),
      (m_calcType == CT_FILEEXISTS) ? _T("fileexists") : _T("direxists"),
      m_varValue.c_str()
    );

    return true;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK nppPluginWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  if (uMessage == WM_CLOSE)
  {
    Runtime::GetLogger().Add/*_WithoutOutput*/( _T("; WM_CLOSE from Notepad++") );
      
    CNppExecCommandExecutor& CommandExecutor = Runtime::GetNppExec().GetCommandExecutor();

    if (CommandExecutor.IsChildProcessRunning() || CommandExecutor.IsScriptRunningOrQueued())
    {
        CNppExecCommandExecutor::ScriptableCommand * pCommand = new CNppExecCommandExecutor::NppExitCommand(tstr());
        CommandExecutor.ExecuteCommand(pCommand);
        return 0;
    }
  }

#ifdef UNICODE
  return ::CallWindowProcW(nppOriginalWndProc, hWnd, uMessage, wParam, lParam);
#else
  return ( g_bIsNppUnicode ?
             ::CallWindowProcW(nppOriginalWndProc, hWnd, uMessage, wParam, lParam) :
                 ::CallWindowProcA(nppOriginalWndProc, hWnd, uMessage, wParam, lParam) );
#endif
}


int FileFilterPos(const TCHAR* szFilePath)
{
  if ( !szFilePath )
    return -1;

  const TCHAR* p = szFilePath;
  while ( *p )
  {
    if ( (*p == _T('*')) || (*p == _T('?')) )
      return ( (int) (p - szFilePath) );
    ++p;
  }
  return -1;
}

void GetPathAndFilter(const TCHAR* szPathAndFilter, 
       int nFilterPos, tstr& out_Path, tstr& out_Filter)
{
  TCHAR path[FILEPATH_BUFSIZE];

  out_Path.Clear();
  int pos = nFilterPos;
  while ( --pos >= 0 )
  {
    if ( (szPathAndFilter[pos] == _T('\\')) || 
         (szPathAndFilter[pos] == _T('/')) )
    {
      out_Path.Assign( szPathAndFilter, pos );
      break;
    }
  }

  if ( (nFilterPos == 1) && 
       (szPathAndFilter[0] == _T('\\') || szPathAndFilter[0] == _T('/')) )
  {
    out_Path.Assign( szPathAndFilter, 1 );
  }
  
  if ( nFilterPos >= 0)
  {
    if ( pos < 0 )  pos = -1;
    out_Filter.Assign( szPathAndFilter + 1 + pos );
  }
  else
  {
    out_Path = szPathAndFilter;
    out_Filter = _T("*");
  }
  
  if ( out_Path.IsEmpty() )
  {
    path[0] = 0;
    GetCurrentDirectory( FILEPATH_BUFSIZE - 1, path );
    out_Path = path;
  }
  else if ( !NppExecHelpers::IsFullPath(out_Path) )
  {
    // is not full path
    path[0] = 0;
    GetCurrentDirectory( FILEPATH_BUFSIZE - 1, path );
    if ( (out_Path.GetAt(0) == _T('\\') && out_Path.GetAt(1) != _T('\\')) || 
         (out_Path.GetAt(0) == _T('/') && out_Path.GetAt(1) != _T('/')) )
    {
      path[2] = 0;
    }
    else
    {
      int len = lstrlen( path );
      if ( path[len - 1] != _T('\\') )
      {
        path[len++] = _T('\\');
        path[len] = 0;
      }
    }
    out_Path.Insert( 0, path );
  }
}

int FilterOK(const TCHAR* mask, const TCHAR* str)
{
  tstr S = str;
  NppExecHelpers::StrLower(S);
  return c_base::_tmatch_mask( mask, S.c_str() );
}

void GetFilePathNamesList(const TCHAR* szPath, const TCHAR* szFilter, 
       CListT<tstr>& FilesList)
{
    FilesList.DeleteAll();
  
    if ( (!szPath) || (szPath[0] == 0) )
        return;

    tstr S = szPath;
    if ( S.GetLastChar() != _T('\\') )  S += _T('\\');
    S += _T("*.*");

    CDirFileLister FileLst;

    if ( !FileLst.FindNext(S.c_str(), 
            CDirFileLister::ESF_FILES | CDirFileLister::ESF_SORTED) )
        return;
  
    tstr  Fltr;
    tstr  FilePathName;
    bool  bFilter = (szFilter && (szFilter[0] != 0)) ? true : false;
    if ( bFilter )
    {
        Fltr = szFilter;
        NppExecHelpers::StrLower(Fltr);
    }
    S.SetSize( S.length() - 3 ); // removing last "*.*"

    const TCHAR* pszFileName;
    unsigned int len;
    
    do
    {
        pszFileName = FileLst.GetItem(&len);
        if ( (!bFilter) || FilterOK(Fltr.c_str(), pszFileName) )
        {
            FilePathName = S;
            FilePathName.Append( pszFileName, len );
            FilesList.Add( FilePathName );
        }
    }
    while ( FileLst.GetNext() );
  
}

bool PrintDirContent(CNppExec* pNppExec, const TCHAR* szPath, const TCHAR* szFilter)
{
    if ( (!szPath) || (szPath[0] == 0) )
        return false;

    tstr S = szPath;
    if ( S.GetLastChar() != _T('\\') )  S += _T('\\');
    S += _T("*.*");

    CDirFileLister DirFileLst;

    if ( !DirFileLst.FindNext(S.c_str(), 
            CDirFileLister::ESF_DIRS | CDirFileLister::ESF_FILES | 
              CDirFileLister::ESF_PLACEDIRSFIRST | CDirFileLister::ESF_SORTED) )
        return false;

    tstr  Fltr;
    bool  bFilter = (szFilter && (szFilter[0] != 0)) ? true : false;
    if ( bFilter )
    {
        Fltr = szFilter;
        NppExecHelpers::StrLower(Fltr);
    }

    const TCHAR* pszFileName;
    unsigned int len;
    bool         isDir;

    do
    {
        pszFileName = DirFileLst.GetItem(&len, &isDir);
        if ( (!bFilter) || FilterOK(Fltr.c_str(), pszFileName) )
        {
            if ( isDir )
            {
                S = _T("<DIR> ");
                S.Append( pszFileName, len );
                pszFileName = S.c_str();
            }
            pNppExec->GetConsole().PrintStr( pszFileName );
        }
    }
    while ( DirFileLst.GetNext() );

    return true;
}

void runInputBox(CScriptEngine* pScriptEngine, const tstr& msg)
{
    const int MAX_VAR_FIELDS = 20;

    TCHAR szVarFldName[40];
    tstr  varName;
    tstr  S;
    CNppExec* pNppExec = pScriptEngine->GetNppExec();

    Runtime::GetLogger().Add(   _T("runInputBox()") );
    Runtime::GetLogger().Add(   _T("{") );
    Runtime::GetLogger().IncIndentLevel();
    Runtime::GetLogger().AddEx( _T("[in]  %s"), msg.c_str() );
    
    // init the InputBox dialog values
    CInputBoxDlg& InputBoxDlg = pNppExec->GetInputBoxDlg();
    InputBoxDlg.m_InputMessage = msg;
    InputBoxDlg.m_InputVarName = MACRO_INPUT;
    InputBoxDlg.m_InputVarName += _T(" =");
    InputBoxDlg.setInputBoxType(CInputBoxDlg::IBT_INPUTBOX);
    
    if ( CNppExec::_bIsNppShutdown )
    {
        Runtime::GetLogger().Add(   _T("; InputBox suppressed as Notepad++ is exiting") );

        InputBoxDlg.m_OutputValue.Clear();
    }
    else
    {
        // show the InputBox dialog
        INT_PTR nRet = pNppExec->PluginDialogBox(IDD_INPUTBOX, InputBoxDlgProc);
        if ( nRet != CInputBoxDlg::RET_OK )
        {
            InputBoxDlg.m_OutputValue.Clear();
        }
    }

    Runtime::GetLogger().Add(   _T("; InputBox returned") );

    // preparing the output value
    pNppExec->GetMacroVars().CheckAllMacroVars(pScriptEngine, InputBoxDlg.m_OutputValue, true);

    // removing previous values
    varName = MACRO_INPUT;
    pNppExec->GetMacroVars().SetUserMacroVar(pScriptEngine, varName, _T(""), CNppExecMacroVars::svRemoveVar | CNppExecMacroVars::svLocalVar); // local var
    for ( int i = 1; i <= MAX_VAR_FIELDS; i++ )
    {
        wsprintf(szVarFldName, MACRO_INPUTFMT, i);
        varName = szVarFldName;
        pNppExec->GetMacroVars().SetUserMacroVar(pScriptEngine, varName, _T(""), CNppExecMacroVars::svRemoveVar | CNppExecMacroVars::svLocalVar); // local var
    }
    
    // getting new values
    CStrSplitT<TCHAR> fields;
    int nFields = fields.SplitToArgs(InputBoxDlg.m_OutputValue, MAX_VAR_FIELDS);

    // setting new values
    varName = MACRO_INPUT;
    pNppExec->GetMacroVars().SetUserMacroVar(pScriptEngine, varName, InputBoxDlg.m_OutputValue, CNppExecMacroVars::svLocalVar); // local var
    S = _T("local ");
    S += varName;
    S += _T(" = ");
    S += InputBoxDlg.m_OutputValue;
    const UINT nMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
    pNppExec->GetConsole().PrintMessage( S.c_str(), nMsgFlags );

    Runtime::GetLogger().AddEx( _T("[out] %s"), S.c_str() );

    for ( int i = 1; i <= nFields; i++ )
    {
        wsprintf(szVarFldName, MACRO_INPUTFMT, i);
        varName = szVarFldName;
        pNppExec->GetMacroVars().SetUserMacroVar(pScriptEngine, varName, fields.GetArg(i - 1), CNppExecMacroVars::svLocalVar); // local var
        S = _T("local ");
        S += varName;
        S += _T(" = ");
        S += fields.GetArg(i - 1);
        const UINT nPrintMsgFlags = CNppExecConsole::pfLogThisMsg | CNppExecConsole::pfNewLine | CNppExecConsole::pfIsInternalMsg;
        pNppExec->GetConsole().PrintMessage( S.c_str(), nPrintMsgFlags );

        Runtime::GetLogger().AddEx( _T("[out] %s"), S.c_str() );

    }

    Runtime::GetLogger().DecIndentLevel();
    Runtime::GetLogger().Add(   _T("}") );
    
    
    // restore the focus
    if ( (pNppExec->m_hFocusedWindowBeforeScriptStarted == pNppExec->GetConsole().GetConsoleWnd()) ||
         (pNppExec->m_hFocusedWindowBeforeScriptStarted == pNppExec->GetConsole().GetDialogWnd()) )
    {
        if ( ::IsWindowVisible(pNppExec->GetConsole().GetDialogWnd()) )
        {
            pNppExec->SendNppMsg(NPPM_DMMSHOW, 0, (LPARAM) pNppExec->GetConsole().GetDialogWnd());
        }
    }
}

CScriptEngine::eCmdResult runMessageBox(CScriptEngine* pScriptEngine, const tstr& params)
{
    Runtime::GetLogger().Add(   _T("runMessageBox()") );
    Runtime::GetLogger().Add(   _T("{") );
    Runtime::GetLogger().IncIndentLevel();
    Runtime::GetLogger().AddEx( _T("[in]  %s"), params.c_str() );

    if ( CNppExec::_bIsNppShutdown )
    {
        Runtime::GetLogger().Add(   _T("; MessageBox suppressed as Notepad++ is exiting") );
        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

        return CScriptEngine::CMDRESULT_SUCCEEDED;
    }

    tstr text, title, type, timeout;
    CStrSplitT<TCHAR> args;
    int n = args.SplitAsArgs(params, _T(':'), 4);
    if ( n > 0 )  text    = args.GetArg(0);
    if ( n > 1 )  title   = args.GetArg(1);
    if ( n > 2 )  type    = args.GetArg(2);
    if ( n > 3 )  timeout = args.GetArg(3);

    n = -1;
    NppExecHelpers::StrLower(type);
    if ( type.IsEmpty() || type == _T("0") || type == _T("msg") )
    {
        n = MB_OK;
        type = _T("msg");
        if ( title.IsEmpty() )  title = _T("NppExec - Message");
    }
    else if ( type == _T("1") || type == _T("warn") )
    {
        n = MB_OK | MB_ICONWARNING;
        type = _T("warn");
        if ( title.IsEmpty() )  title = _T("NppExec - Warning");
    }
    else if ( type == _T("2") || type == _T("err") )
    {
        n = MB_OK | MB_ICONERROR;
        type = _T("err");
        if ( title.IsEmpty() )  title = _T("NppExec - Error");
    }

    if ( n == -1 )
    {
        pScriptEngine->ScriptError( IScriptEngine::ET_REPORT, _T("- unknown type of MessageBox specified") );

        Runtime::GetLogger().DecIndentLevel();
        Runtime::GetLogger().Add(   _T("}") );

        return CScriptEngine::CMDRESULT_INVALIDPARAM;
    }

    Runtime::GetLogger().AddEx( _T("; type: %s"), type.c_str() );

    unsigned int nTimeout = 0;
    CNppExec::MSGBOXTIMEOUTFUNC lpMsgBoxTimeoutFunc = pScriptEngine->GetNppExec()->m_lpMsgBoxTimeoutFunc;
    if ( lpMsgBoxTimeoutFunc && !timeout.IsEmpty() )
    {
        nTimeout = c_base::_tstr2uint(timeout.c_str());
    }

    HWND hWnd = pScriptEngine->GetNppExec()->m_nppData._nppHandle;
    if ( nTimeout != 0 )
        lpMsgBoxTimeoutFunc( hWnd, text.c_str(), title.c_str(), n, 0, nTimeout );
    else
        ::MessageBox( hWnd, text.c_str(), title.c_str(), n );

    Runtime::GetLogger().DecIndentLevel();
    Runtime::GetLogger().Add(   _T("}") );

    return CScriptEngine::CMDRESULT_SUCCEEDED;
}

char* SciTextFromLPCTSTR(LPCTSTR pText, HWND hSci, int* pnLen )
{
    char* pOutSciText = NULL;
    int   nSciCodePage = (int) ::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0);

    switch ( nSciCodePage )
    {
        case 0:          // ANSI
            #ifdef UNICODE
                pOutSciText = SysUniConv::newUnicodeToMultiByte(pText, -1, CP_ACP, pnLen);
            #else
                pOutSciText = (char *) pText;
            #endif
            break;

        case SC_CP_UTF8: // UTF-8
            #ifdef UNICODE
                pOutSciText = SysUniConv::newUnicodeToUTF8(pText, -1, pnLen);
            #else
                pOutSciText = SysUniConv::newMultiByteToUTF8(pText, -1, CP_ACP);
            #endif
            break;

        default:         // multi-byte encoding
            #ifdef UNICODE
                pOutSciText = SysUniConv::newUnicodeToMultiByte(pText, -1, CP_ACP, pnLen);
            #else
                pOutSciText = (char *) pText;
            #endif
            break;
    }

    return pOutSciText;
}

LPTSTR SciTextToLPTSTR(const char* pSciText, HWND hSci)
{
    LPTSTR pOutText = NULL;
    int    nSciCodePage = (int) ::SendMessage(hSci, SCI_GETCODEPAGE, 0, 0);

    switch ( nSciCodePage )
    {
        case SC_CP_UTF8:
            #ifdef UNICODE
                pOutText = SysUniConv::newUTF8ToUnicode(pSciText);
            #else
                pOutText = SysUniConv::newUTF8ToMultiByte(pSciText, -1, CP_ACP);
            #endif
            break;

        default:
            #ifdef UNICODE
                pOutText = SysUniConv::newMultiByteToUnicode(pSciText, -1, CP_ACP);
            #else
                pOutText = (char *) pSciText;
            #endif
            break;
    }

    return pOutText;
}
