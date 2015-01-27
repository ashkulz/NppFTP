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
#include "InputDialog.h"

#include <windowsx.h>
#include "resource.h"

InputDialog::InputDialog() :
	Dialog(IDD_DIALOG_INPUT),
	m_commentCtrl(NULL),
	m_valueCtrl(NULL),
	m_password(false)
{
	m_comment = SU::DupString(TEXT(""));
	m_value = SU::DupString(TEXT(""));
}

InputDialog::~InputDialog() {
	SU::FreeTChar(m_comment);
	SU::FreeTChar(m_value);
}

int InputDialog::Create(HWND hParent, const TCHAR * title, const TCHAR * comment, const TCHAR * initialValue, bool password) {
	SU::FreeTChar(m_comment);
	m_comment = SU::DupString(comment);
	SU::FreeTChar(m_value);
	m_value = SU::DupString(initialValue);
	m_password = password;

	int res = Dialog::Create(hParent, true, title);

	return res;
}

const TCHAR* InputDialog::GetValue() {
	return m_value;
}

INT_PTR InputDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	INT_PTR result = FALSE;
	bool doDefaultProc = false;

	switch(uMsg) {
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDB_BUTTON_CANCEL: {
					EndDialog(m_hwnd, 2);
					result = TRUE;
					break; }
				case IDB_BUTTON_OK: {
					int len = ::SendMessage(m_valueCtrl, WM_GETTEXTLENGTH, 0, 0);
					TCHAR * buffer = new TCHAR[len+1];
					GetWindowText(m_valueCtrl, buffer, len+1);
					SU::FreeTChar(m_value);
					m_value = SU::DupString(buffer);
					delete [] buffer;
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

INT_PTR InputDialog::OnInitDialog() {
	Dialog::OnInitDialog();

	m_commentCtrl = GetDlgItem(m_hwnd, IDC_STATIC_COMMENT);
	m_valueCtrl = GetDlgItem(m_hwnd, IDC_EDIT_INPUT);

	SetWindowText(m_commentCtrl, m_comment);
	SetWindowText(m_valueCtrl, m_value);

	if (m_password) {
		Edit_SetPasswordChar(::GetDlgItem(m_hwnd, IDC_EDIT_INPUT), (UINT)'*');
	}

	return TRUE;
}
