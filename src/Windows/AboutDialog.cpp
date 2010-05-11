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
#include "AboutDialog.h"

#include "resource.h"
#include "symbols.h"
#include <libssh/libssh.h>
#include <openssl/ssl.h>

AboutDialog::AboutDialog() :
	Dialog(IDD_DIALOG_ABOUT)
{
}

AboutDialog::~AboutDialog() {
}

int AboutDialog::Create(HWND hParent) {
	return Dialog::Create(hParent, true, TEXT("About NppFTP"));
}

INT_PTR AboutDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return Dialog::DlgMsgProc(uMsg, wParam, lParam);
}

INT_PTR AboutDialog::OnInitDialog() {
	const TCHAR * sshVersion = TEXT(SSH_STRINGIFY(LIBSSH_VERSION));
	const TCHAR * sslVersion = TEXT(OPENSSL_VERSION_TEXT);
	const TCHAR * nppFTPVersion = TEXT(IDT_VERSION_TEXT);

	::SetDlgItemText(m_hwnd, IDC_STATIC_SSHVERSION, sshVersion);
	::SetDlgItemText(m_hwnd, IDC_STATIC_SSLVERSION, sslVersion);
	::SetDlgItemText(m_hwnd, IDC_STATIC_NPPFTPVERSION, nppFTPVersion);

	const TCHAR * aboutMessage = TEXT(
		"NppFTP, Copyright 2010\r\n"
		"Created by Harry ( harrybharry@users.sourceforge.net )\r\n"
		"\r\n"
		"Press Show FTP Window to get started, and read the documentation (if any) when you get stuck.\r\n"
		"For help, info and updates, visit the site by clicking the button below.\r\n"
		"\r\n"
		"Enjoy the comfort of transferring your files from your favorite editor! =)"
		"\r\n\r\n"
		"NppFTP works because of the effort put in the following libraries:\r\n"
		"- OpenSSL\r\n"
		"- libssh\r\n"
		"- Ultimate TCP/IP\r\n"
		"- TinyXML\r\n"
		"\r\n"
		"And not to forget:\r\n"
		"Silk icons from famfamfam\r\n"
	);

	::EnableWindow(GetDlgItem(m_hwnd, IDC_EDIT_ABOUTMSG), TRUE);
	::SetDlgItemText(m_hwnd, IDC_EDIT_ABOUTMSG, aboutMessage);

	::SetFocus(GetDlgItem(m_hwnd, IDC_BUTTON_CLOSE));
	::SendMessage(GetDlgItem(m_hwnd, IDC_EDIT_ABOUTMSG), EM_SETSEL, 0, 0);

	Dialog::OnInitDialog();

	return FALSE;
}

INT_PTR AboutDialog::OnCommand(int ctrlId, int notifCode, HWND idHwnd) {
	INT_PTR result = 0;

	switch(ctrlId) {
		case IDC_BUTTON_CLOSE: {
			EndDialog(m_hwnd, 0);
			result = TRUE;
			break; }
		case IDC_BUTTON_VISIT: {
			ShellExecute(NULL, TEXT("open"), TEXT("http://sourceforge.net/projects/nppftp"), NULL, NULL, SW_SHOWNORMAL);
			result = TRUE;
			break; }
		default: {
			result = Dialog::OnCommand(ctrlId, notifCode, idHwnd);
			break; }
	}
	return result;
}

INT_PTR AboutDialog::OnNotify(NMHDR * pnmh) {
	INT_PTR result = 0;

	result = Dialog::OnNotify(pnmh);

	return result;
}
