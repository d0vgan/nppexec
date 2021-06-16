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

#ifndef _input_box_dlg_h_
#define _input_box_dlg_h_
//-------------------------------------------------------------------------
#include "base.h"
#include "resource.h"
#include "CAnyWindow.h"
#include "CAnyComboBox.h"
#include "NppExec.h"


INT_PTR CALLBACK InputBoxDlgProc(HWND, UINT, WPARAM, LPARAM);


class CInputBoxDlg : public CAnyWindow
{
    public:
        static const int HISTORY_ITEMS = 16;

        enum eInputBoxType {
            IBT_INPUTBOX = 0,
            IBT_EXITPROCESS,

            IBT_TOTAL
        };

        enum eReturnResult {
            RET_CANCEL    = 0,
            RET_OK        = 1,
            RET_KILL      = 2
        };

    protected:
        int          m_type;
        int          m_nInputIndex[IBT_TOTAL];

        int          m_nFrstBtnLeftPos;
        int          m_nScndBtnLeftPos;

        unsigned int m_nTimeout;
        UINT_PTR     m_idTimerTimeout;

        CAnyWindow   m_btOK;
        CAnyWindow   m_btKill;
        CAnyWindow   m_btCancel;
        CAnyWindow   m_stMessage;
        CAnyWindow   m_stVarName;
        CAnyComboBox m_cbVarValue;

        void updateCaptionWithTimeout(HWND hDlg);
        void applyInputBoxType();

    public:
        tstr m_InputMessage;
        tstr m_InputVarName;
        tstr m_OutputValue;

        CListT<tstr> m_InputHistory[IBT_TOTAL];
    
    public:
        CInputBoxDlg();
        ~CInputBoxDlg();

        void CloseDialog(HWND hDlg, INT_PTR nResult);
        void OnBtOK();
        void OnTimer(HWND hDlg, WPARAM wParam);
        void OnShowWindow(HWND hDlg, WPARAM wParam);
        void OnInitDialog(HWND hDlg);

        int  getInputBoxType() const  { return m_type; }
        void setInputBoxType(eInputBoxType type)  { m_type = type; }
};

//-------------------------------------------------------------------------
#endif
