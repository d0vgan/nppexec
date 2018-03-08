#define WM_COMMAND     0x0111
#define WM_SYSCOMMAND  0x0112
#define WM_USER        0x0400
#define NPPMSG         (WM_USER + 1000)

// NppExec's Search Flags for sci_find and sci_replace:
#define NPE_SF_MATCHCASE    0x00000001 // "text" finds only "text", not "Text" or "TEXT"
#define NPE_SF_WHOLEWORD    0x00000010 // "word" finds only "word", not "sword" or "words" 
#define NPE_SF_WORDSTART    0x00000020 // "word" finds "word" and "words", not "sword"
#define NPE_SF_REGEXP       0x00000100 // search using a regular expression
#define NPE_SF_POSIX        0x00000200 // search using a POSIX-compatible regular expression
#define NPE_SF_CXX11REGEX   0x00000400 // search using a C++11 regular expression
#define NPE_SF_BACKWARD     0x00010000 // search backward (from the bottom to the top)
#define NPE_SF_NEXT         0x00020000 // search from current_position + 1
#define NPE_SF_INSELECTION  0x00100000 // search only in the selected text
#define NPE_SF_INWHOLETEXT  0x00200000 // search in the whole text, not only from the current position
#define NPE_SF_SETPOS       0x01000000 // move the caret to the position of the occurrence found
#define NPE_SF_SETSEL       0x02000000 // move the caret + select the occurrence found
#define NPE_SF_REPLACEALL   0x10000000 // replace all the occurrences from the current pos to the end
#define NPE_SF_PRINTALL     0x20000000 // print all the occurrences from the current pos to the end

// add more definitions if needed...
#define pi 3.14159265359
#define e  2.718281828459
