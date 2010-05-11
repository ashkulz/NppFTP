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

#include "resource.h"

SettingsDialog::SettingsDialog() :
	Dialog(IDD_DIALOG_GLOBAL),
	m_globalCache(NULL)
{
}

SettingsDialog::~SettingsDialog() {
}

int SettingsDialog::Create(HWND hParent, FTPCache * globalCache) {
	m_globalCache = globalCache;
	return Dialog::Create(hParent, true, TEXT("General settings"));
}

INT_PTR SettingsDialog::OnInitDialog() {
	_EditDefaultProc = (WNDPROC)::SetWindowLongPtr(::GetDlgItem(m_hwnd, IDC_EDIT_CACHE), GWL_WNDPROC, (DWORD)&Dialog::EditProc);

	const PathMap & pathmap = m_globalCache->GetPathMap(0);

	::SetDlgItemText(m_hwnd, IDC_EDIT_CACHE, pathmap.localpath);

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
		case IDC_EDIT_CACHEEXTERNAL: {
			if (notifCode == EN_KEYRETURN) {
				SaveGlobalPath();
			}
			break; }
		case IDC_BUTTON_CLOSE: {
			SaveGlobalPath();
			EndDialog(m_hwnd, 0);
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
