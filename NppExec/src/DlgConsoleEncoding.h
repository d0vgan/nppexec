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

#ifndef _console_encoding_dlg_h_
#define _console_encoding_dlg_h_
//-------------------------------------------------------------------------
#include "base.h"
#include "resource.h"
#include "CAnyWindow.h"
#include "NppExec.h"


INT_PTR CALLBACK ConsoleEncodingDlgProc(HWND, UINT, WPARAM, LPARAM);


class CConsoleEncodingDlg : public CAnyWindow
{
    public:
        enum eEncodings {
            ENC_ANSI = 0,
            ENC_OEM,
            ENC_UTF8,

            ENC_TOTAL
        };

    protected:
        static const TCHAR* szEncNames[ENC_TOTAL];
        static unsigned int getEnc(unsigned int enc);

        void updateUI();
    
    public:
        unsigned int m_enc_opt;

        CConsoleEncodingDlg();
        ~CConsoleEncodingDlg();

        void OnBtOK();
        void OnChInputAsOutput();
        void OnInitDialog(HWND hDlg);

        static unsigned int defaultEncodingOption();
        static unsigned int getPseudoConsoleEncoding();
        static unsigned int getInputEncoding(unsigned int enc_opt);
        static const TCHAR* getInputEncodingName(unsigned int enc_opt);
        static unsigned int getOutputEncoding(unsigned int enc_opt);
        static const TCHAR* getOutputEncodingName(unsigned int enc_opt);
};

//-------------------------------------------------------------------------
#endif
