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
#include "SettingsDialog.h"

#include "Encryption.h"
#include "resource.h"
#include <Windowsx.h>

SettingsDialog::SettingsDialog() :
	Dialog(IDD_DIALOG_GLOBAL),
	m_globalCache(NULL)
{
}

SettingsDialog::~SettingsDialog() {
}

int SettingsDialog::Create(HWND hParent, FTPCache * globalCache) {
	m_globalCache = globalCache;
	return Dialog::Create(hParent, true, NULL);
}

INT_PTR SettingsDialog::OnInitDialog() {
	const PathMap & pathmap = m_globalCache->GetPathMap(0);

	Edit_LimitText(::GetDlgItem(m_hwnd, IDC_EDIT_MASTERPASS), Encryption::KeySize);

	::SetDlgItemText(m_hwnd, IDC_EDIT_CACHE, pathmap.localpath);


	if (!Encryption::IsDefaultKey()) {
		char password[Encryption::KeySize+1];
		memcpy(password, Encryption::GetDefaultKey(), Encryption::KeySize);
		password[Encryption::KeySize] = 0;
		::SetDlgItemTextA(m_hwnd, IDC_EDIT_MASTERPASS, password);
	}

	return Dialog::OnInitDialog();
}

INT_PTR SettingsDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	bool doDefProc = false;
	INT_PTR result = FALSE;

	switch(uMsg) {
		default: {
			doDefProc = true;
			break; }
	}

	if (doDefProc)
		result = Dialog::DlgMsgProc(uMsg, wParam, lParam);

	return result;
}

INT_PTR SettingsDialog::OnCommand(int ctrlId, int notifCode, HWND idHwnd) {
	switch(ctrlId) {
		case IDC_BUTTON_CLOSE: {
			SaveGlobalPath();
			SaveMasterPassword();
			EndDialog(m_hwnd, 0);
			break; }
		default: {
			return Dialog::OnCommand(ctrlId, notifCode, idHwnd);
			break; }
	}

	return TRUE;
}

INT_PTR SettingsDialog::OnNotify(NMHDR * pnmh) {
	return Dialog::OnNotify(pnmh);
}

int SettingsDialog::SaveGlobalPath() {
	TCHAR TTextBuffer[MAX_PATH];
	::GetDlgItemText(m_hwnd, IDC_EDIT_CACHE, TTextBuffer, MAX_PATH);

	const PathMap & pathmaporig = m_globalCache->GetPathMap(0);
	PathMap pathnew;
	pathnew.localpath = SU::DupString(TTextBuffer);
	pathnew.externalpath = SU::strdup(pathmaporig.externalpath);
	m_globalCache->SetPathMap(pathnew, 0);

	return 0;
}

int SettingsDialog::SaveMasterPassword() {
	char password[Encryption::KeySize+1];
	::GetDlgItemTextA(m_hwnd, IDC_EDIT_MASTERPASS, password, Encryption::KeySize);
	Encryption::SetDefaultKey(password);


	char * challenge = Encryption::Encrypt(NULL, -1, "NppFTP", -1);
	OutMsg("challenge %s", challenge);
	char * data = Encryption::Decrypt(NULL, -1, challenge, true);
	OutMsg("data %s", data);


	return 0;
}
