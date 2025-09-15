#include "../NppExec/src/NppExecHelpers.h"
#include <cassert>

namespace
{
    void test_GetFileNamePart()
    {
        using namespace NppExecHelpers;

        const tstr empty = tstr();

        tstr path = _T("C");
        assert( GetFileNamePart(path, fnpDrive)   == empty );
        assert( GetFileNamePart(path, fnpDirPath) == empty );
        assert( GetFileNamePart(path, fnpNameExt) == _T("C") );
        assert( GetFileNamePart(path, fnpName)    == _T("C") );
        assert( GetFileNamePart(path, fnpExt)     == empty );

        path = _T("C:");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:") );
        assert( GetFileNamePart(path, fnpNameExt) == empty );
        assert( GetFileNamePart(path, fnpName)    == empty );
        assert( GetFileNamePart(path, fnpExt)     == empty );

        path = _T("C:\\");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:\\") );
        assert( GetFileNamePart(path, fnpNameExt) == empty );
        assert( GetFileNamePart(path, fnpName)    == empty );
        assert( GetFileNamePart(path, fnpExt)     == empty );

        path = _T("C:/");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:/") );
        assert( GetFileNamePart(path, fnpNameExt) == empty );
        assert( GetFileNamePart(path, fnpName)    == empty );
        assert( GetFileNamePart(path, fnpExt)     == empty );

        path = _T(".C:");
        assert( GetFileNamePart(path, fnpDrive)   == empty );
        assert( GetFileNamePart(path, fnpDirPath) == empty );
        assert( GetFileNamePart(path, fnpNameExt) == _T(".C:") );
        assert( GetFileNamePart(path, fnpName)    == _T(".C:") );
        assert( GetFileNamePart(path, fnpExt)     == _T(".C:") );

        path = _T("\\\\");
        assert( GetFileNamePart(path, fnpDrive)   == empty );
        assert( GetFileNamePart(path, fnpDirPath) == _T("\\\\") );
        assert( GetFileNamePart(path, fnpNameExt) == empty );
        assert( GetFileNamePart(path, fnpName)    == empty );
        assert( GetFileNamePart(path, fnpExt)     == empty );

        path = _T("\\\\f.n");
        assert( GetFileNamePart(path, fnpDrive)   == empty );
        assert( GetFileNamePart(path, fnpDirPath) == _T("\\\\") );
        assert( GetFileNamePart(path, fnpNameExt) == _T("f.n") );
        assert( GetFileNamePart(path, fnpName)    == _T("f") );
        assert( GetFileNamePart(path, fnpExt)     == _T(".n") );

        path = _T("C:\\.n");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:\\") );
        assert( GetFileNamePart(path, fnpNameExt) == _T(".n") );
        assert( GetFileNamePart(path, fnpName)    == _T(".n") );
        assert( GetFileNamePart(path, fnpExt)     == _T(".n") );

        path = _T("C:\\d\\f.n");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:\\d\\") );
        assert( GetFileNamePart(path, fnpNameExt) == _T("f.n") );
        assert( GetFileNamePart(path, fnpName)    == _T("f") );
        assert( GetFileNamePart(path, fnpExt)     == _T(".n") );
        
        path = _T("C:\\d.e\\f");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:\\d.e\\") );
        assert( GetFileNamePart(path, fnpNameExt) == _T("f") );
        assert( GetFileNamePart(path, fnpName)    == _T("f") );
        assert( GetFileNamePart(path, fnpExt)     == empty );

        path = _T("C:\\d.e\\.n");
        assert( GetFileNamePart(path, fnpDrive)   == _T("C") );
        assert( GetFileNamePart(path, fnpDirPath) == _T("C:\\d.e\\") );
        assert( GetFileNamePart(path, fnpNameExt) == _T(".n") );
        assert( GetFileNamePart(path, fnpName)    == _T(".n") );
        assert( GetFileNamePart(path, fnpExt)     == _T(".n") );

        path = _T("f.n");
        assert( GetFileNamePart(path, fnpDrive)   == empty );
        assert( GetFileNamePart(path, fnpDirPath) == empty );
        assert( GetFileNamePart(path, fnpNameExt) == _T("f.n") );
        assert( GetFileNamePart(path, fnpName)    == _T("f") );
        assert( GetFileNamePart(path, fnpExt)     == _T(".n") );
    }

    void test_IsFullLocalPath()
    {
        using namespace NppExecHelpers;

        assert( IsFullLocalPath(_T("")) == false );
        assert( IsFullLocalPath(_T("\\")) == false );
        assert( IsFullLocalPath(_T("/")) == false );
        assert( IsFullLocalPath(_T("\\.")) == false );
        assert( IsFullLocalPath(_T("/.")) == false );
        assert( IsFullLocalPath(_T("\\\\")) == false );
        assert( IsFullLocalPath(_T("//")) == false );
        assert( IsFullLocalPath(_T("\\\\X")) == false );
        assert( IsFullLocalPath(_T("//X")) == false );
        assert( IsFullLocalPath(_T("\\\\X:")) == false );
        assert( IsFullLocalPath(_T("//X:")) == false );
        assert( IsFullLocalPath(_T("\\\\\\")) == false );
        assert( IsFullLocalPath(_T("///")) == false );
        assert( IsFullLocalPath(_T("\\\\.")) == false );
        assert( IsFullLocalPath(_T("//.")) == false );
        assert( IsFullLocalPath(_T("\\\\.\\")) == false );
        assert( IsFullLocalPath(_T("//./")) == false );
        assert( IsFullLocalPath(_T("\\\\.\\X")) == false );
        assert( IsFullLocalPath(_T("//./X")) == false );
        assert( IsFullLocalPath(_T("\\\\.\\X:")) == true );
        assert( IsFullLocalPath(_T("//./X:")) == true );
        assert( IsFullLocalPath(_T("\\\\.\\X:\\")) == true );
        assert( IsFullLocalPath(_T("//./X:/")) == true );
        assert( IsFullLocalPath(_T("\\\\.\\X:/")) == true );
        assert( IsFullLocalPath(_T("//./X:\\")) == true );
        assert( IsFullLocalPath(_T("\\\\.\\X:\\abc")) == true );
        assert( IsFullLocalPath(_T("//./X:/abc")) == true );
        assert( IsFullLocalPath(_T("\\\\.\\X:/abc")) == true );
        assert( IsFullLocalPath(_T("//./X:\\abc")) == true );
        assert( IsFullLocalPath(_T("\\:")) == false );
        assert( IsFullLocalPath(_T("/:")) == false );
        assert( IsFullLocalPath(_T("\\:\\")) == false );
        assert( IsFullLocalPath(_T("/:/")) == false );
        assert( IsFullLocalPath(_T("X:")) == true );
        assert( IsFullLocalPath(_T("X:\\")) == true );
        assert( IsFullLocalPath(_T("X:/")) == true );
        assert( IsFullLocalPath(_T("X:\\abc")) == true );
        assert( IsFullLocalPath(_T("X:/abc")) == true );
        assert( IsFullLocalPath(_T("abc")) == false );
        assert( IsFullLocalPath(_T("a:bc")) == false );
        assert( IsFullLocalPath(_T("\abc")) == false );
        assert( IsFullLocalPath(_T("/abc")) == false );
        assert( IsFullLocalPath(_T("\\abc")) == false );
        assert( IsFullLocalPath(_T("//abc")) == false );
    }

    void tests()
    {
        test_GetFileNamePart();
        test_IsFullLocalPath();
    }
}

int main()
{
    tests();
    return 0;
}
