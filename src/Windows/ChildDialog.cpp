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
#include "ChildDialog.h"

ChildDialog::ChildDialog(int dialogResource) :
	Dialog(dialogResource),
	m_hOwner(NULL)
{
}

ChildDialog::~ChildDialog() {
}

int ChildDialog::Create(HWND hParent, HWND hOwner, const TCHAR * title) {
	int res = Dialog::Create(hParent, false, title);
	if (res == -1)
		return -1;

	m_hOwner = hOwner;
	return 0;
}

INT_PTR ChildDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	INT_PTR result;

	switch(uMsg) {
		case WM_COMMAND:
		case WM_NOTIFY: {
			result = (INT_PTR)::SendMessage(m_hOwner, uMsg, wParam, lParam);
			break; }
		default:
			result = Dialog::DlgMsgProc(uMsg, wParam, lParam);
	}

	return result;
}
