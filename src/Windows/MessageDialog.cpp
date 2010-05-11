/*
    NppFTP: FTP/SFTP functionality for Notepad++
    Copyright (C) 2010  Harry (harrybharry@users.sourceforge.net)

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

#include "StdInc.h"
#include "MessageDialog.h"

#include "resource.h"

MessageDialog::MessageDialog() :
	Dialog(IDD_DIALOG_MESSAGE),
	m_messageCtrl(NULL)
{
	m_message = SU::DupString(TEXT(""));
}

MessageDialog::~MessageDialog() {
	SU::FreeTChar(m_message);
}

int MessageDialog::Create(HWND hParent, const TCHAR * title, const TCHAR * message) {
	SU::FreeTChar(m_message);
	m_message = SU::DupString(message);

	int res = Dialog::Create(hParent, true, title);

	return res;
}

INT_PTR MessageDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	INT_PTR result = FALSE;
	bool doDefaultProc = false;

	switch(uMsg) {
		case WM_CLOSE: {
			EndDialog(m_hwnd, 2);
			result = TRUE;
			break; }
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDB_BUTTON_NO: {
					EndDialog(m_hwnd, 2);
					result = TRUE;
					break; }
				case IDB_BUTTON_YES: {
					EndDialog(m_hwnd, 1);
					result = TRUE;
					break; }
				default: {
					doDefaultProc = true;
					break; }
			}
			break; }
		default: {
			doDefaultProc = true;
			break; }
	}

	if (doDefaultProc)
		result = Dialog::DlgMsgProc(uMsg, wParam, lParam);

	return result;
}

INT_PTR MessageDialog::OnInitDialog() {
	Dialog::OnInitDialog();

	m_messageCtrl = GetDlgItem(m_hwnd, IDC_STATIC_MESSAGE);

	SetWindowText(m_messageCtrl, m_message);
	return TRUE;
}
