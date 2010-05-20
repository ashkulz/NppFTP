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
#include "ProfilesDialog.h"

#include "InputDialog.h"
#include "FTPWindow.h"
#include "resource.h"
#include <Windowsx.h>

ProfilesDialog::ProfilesDialog() :
	Dialog(IDD_DIALOG_PROFILES),
	m_profiles(NULL),
	m_currentProfile(NULL),
	m_globalCache(NULL),
	m_ftpWindow(NULL),
	m_pageConnection(IDD_DIALOG_PROFILESCONNECTION),
	m_pageAuthentication(IDD_DIALOG_PROFILESAUTHENTICATION),
	m_pageTransfer(IDD_DIALOG_PROFILESTRANSFERS),
	m_pageCache(IDD_DIALOG_PROFILESCACHE),
	m_hPageConnection(NULL),
	m_hPageTransfer(NULL),
	m_hPageCache(NULL)
{
}

ProfilesDialog::~ProfilesDialog() {
}

int ProfilesDialog::Create(HWND hParent, FTPWindow * ftpWindow, vProfile * profileVect, FTPCache * globalCache) {
	m_ftpWindow = ftpWindow;
	m_profiles = profileVect;
	m_globalCache = globalCache;
	return Dialog::Create(hParent, true, NULL);
}

INT_PTR ProfilesDialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

INT_PTR ProfilesDialog::OnCommand(int ctrlId, int notifCode, HWND idHwnd) {
	char aTextBuffer[MAX_PATH];
	TCHAR TTextBuffer[MAX_PATH];

	//things that can do without a profile
	switch(ctrlId) {
		case IDC_BUTTON_CLOSE: {
			EndDialog(m_hwnd, 0);
			break; }
		case IDC_BUTTON_PROFILE_ADD: {
			InputDialog id;
			int res = id.Create(m_hwnd, TEXT("Adding profile"), TEXT("Please enter the name of the new profile"), TEXT("New profile"));
			if (res == 1) {
				FTPProfile * newProfile = new FTPProfile(id.GetValue());
				newProfile->SetCacheParent(m_globalCache);
				m_profiles->push_back(newProfile);
				newProfile->AddRef();
				m_ftpWindow->OnProfileChange();
				LoadProfiles();
			}
			break; }
		case IDC_LIST_PROFILES: {
			if (notifCode == LBN_SELCHANGE) {
				int index = ListBox_GetCurSel(idHwnd);
				FTPProfile* profile = (FTPProfile*)ListBox_GetItemData(idHwnd, index);
				if (index != LB_ERR) {
					OnSelectProfile(profile);
				}
			}
			break; }
	}

	if (!m_currentProfile)
		return TRUE;

	//things that require a profile
	switch(ctrlId) {
		case IDC_BUTTON_PROFILE_RENAME: {
			InputDialog id;
			int res = id.Create(m_hwnd, TEXT("Renaming profile"), TEXT("Please enter the new name of the profile"), m_currentProfile->GetName());
			if (res == 1) {
				m_currentProfile->SetName(id.GetValue());
				m_ftpWindow->OnProfileChange();
				LoadProfiles();
			}
			break; }
		case IDC_BUTTON_PROFILE_DELETE: {
			for(size_t i = 0; i < m_profiles->size(); i++) {
				if (m_profiles->at(i) == m_currentProfile) {
					m_profiles->erase(m_profiles->begin()+i);
					break;
				}
			}
			m_currentProfile->Release();
			m_ftpWindow->OnProfileChange();
			LoadProfiles();
			OnSelectProfile(NULL);
			break; }
		case IDC_EDIT_HOSTNAME: {
			if (notifCode == EN_KEYPRESS) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetHostname(aTextBuffer);
			}
			break; }
		case IDC_EDIT_PORT: {
			if (notifCode == EN_KEYPRESS) {
				BOOL success = FALSE;
				int port = GetDlgItemInt(m_hPageConnection, ctrlId, &success, FALSE);
				if (success)
					m_currentProfile->SetPort(port);
			}
			break; }
		case IDC_EDIT_USERNAME: {
			if (notifCode == EN_KEYPRESS) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetUsername(aTextBuffer);
			}
			break; }
		case IDC_EDIT_PASSWORD: {
			if (notifCode == EN_KEYPRESS) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetPassword(aTextBuffer);
			}
			break; }
		case IDC_EDIT_TIMEOUT: {
			if (notifCode == EN_KEYPRESS) {
				BOOL success = FALSE;
				int timeout = GetDlgItemInt(m_hPageConnection, ctrlId, &success, FALSE);
				if (success)
					m_currentProfile->SetTimeout(timeout);
			}
			break; }
		case IDC_EDIT_INITDIR: {
			if (notifCode == EN_KEYPRESS) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetInitialDir(aTextBuffer);
			}
			break; }
		case IDC_COMBO_SECURITY: {
			if (notifCode == CBN_SELCHANGE) {
				int sel = ComboBox_GetCurSel(idHwnd);
				if (sel != CB_ERR) {
					m_currentProfile->SetSecurityMode((Security_Mode)sel);	//index matches enum
					int port = m_currentProfile->GetPort();
					bool setport = (port == 21 || port == 22 || port == 990);	//otherwise its custom so leave it as is
					if (setport) {
						switch ((Security_Mode)sel) {
							case Mode_FTP: {
								m_currentProfile->SetPort(21);
								break; }
							case Mode_FTPS: {
								m_currentProfile->SetPort(990);
								break; }
							case Mode_FTPES: {
								m_currentProfile->SetPort(21);
								break; }
							case Mode_SFTP: {
								m_currentProfile->SetPort(22);
								break; }
						}
						::SetDlgItemInt(m_hPageConnection, IDC_EDIT_PORT,  m_currentProfile->GetPort(), TRUE);
					}
				}
			}
			break; }

		case IDC_EDIT_KEYFILE: {
			if (notifCode == EN_KEYPRESS) {
				GetWindowText(idHwnd, TTextBuffer, MAX_PATH);
				m_currentProfile->SetKeyFile(TTextBuffer);
			}
			break; }
		case IDC_EDIT_PASSPHRASE: {
			if (notifCode == EN_KEYPRESS) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetPassphrase(aTextBuffer);
			}
			break; }
		case IDC_CHECK_AGENT: {
			if (notifCode == BN_CLICKED) {
				LRESULT checked = Button_GetCheck(idHwnd);
				m_currentProfile->SetUseAgent(checked == BST_CHECKED);
			}
			break; }
		case IDC_BUTTON_KEYBROWSE: {
			TTextBuffer[0] = 0;
			int res = PU::GetOpenFilename(TTextBuffer, MAX_PATH, m_hwnd);
			if (res != -1) {
				SetDlgItemText(m_hPageAuthentication, IDC_EDIT_KEYFILE, TTextBuffer);
				m_currentProfile->SetKeyFile(TTextBuffer);
			}
			break; }
		case IDC_CHECK_PASSWORD:
		case IDC_CHECK_KEY:
		case IDC_CHECK_INTERACTIVE: {
			if (notifCode == BN_CLICKED) {
				int methods = 0;
				LRESULT checked;
				checked = Button_GetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_PASSWORD));
				if (checked == BST_CHECKED)
					methods |= Method_Password;
				checked = Button_GetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_KEY));
				if (checked == BST_CHECKED)
					methods |= Method_Key;
				checked = Button_GetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_INTERACTIVE));
				if (checked == BST_CHECKED)
					methods |= Method_Interactive;

				m_currentProfile->SetAcceptedMethods((AuthenticationMethods)methods);
			}
			break; }

		case IDC_RADIO_ACTIVE: {
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, IDC_RADIO_ACTIVE);
				m_currentProfile->SetConnectionMode(Mode_Active);
			}
			break; }
		case IDC_RADIO_PASSIVE: {
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, IDC_RADIO_PASSIVE);
				m_currentProfile->SetConnectionMode(Mode_Passive);
			}
			break; }
		case IDC_RADIO_ASCII: {
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ASCII, IDC_RADIO_BINARY, IDC_RADIO_ASCII);
				m_currentProfile->SetTransferMode(Mode_ASCII);
			}
			break; }
		case IDC_RADIO_BINARY: {
			if (notifCode == BN_CLICKED) {
				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ASCII, IDC_RADIO_BINARY, IDC_RADIO_BINARY);
				m_currentProfile->SetTransferMode(Mode_Binary);
			}
			break; }


		case IDC_EDIT_ASCII: {
			if (notifCode == EN_KEYRETURN) {
				GetWindowText(idHwnd, TTextBuffer, MAX_PATH);
				int res = m_currentProfile->AddAsciiType(TTextBuffer);
				if (res != -1) {
					SetWindowText(idHwnd, TEXT(""));
					LoadFiletypes();
				} else {
					MessageBeep(MB_OK);
				}
			}
			break; }
		case IDC_EDIT_BINARY: {
			if (notifCode == EN_KEYRETURN) {
				GetWindowText(idHwnd, TTextBuffer, MAX_PATH);
				int res = m_currentProfile->AddBinaryType(TTextBuffer);
				if (res != -1) {
					SetWindowText(idHwnd, TEXT(""));
					LoadFiletypes();
				} else {
					MessageBeep(MB_OK);
				}
			}
			break; }
		case IDC_LIST_ASCII: {
			if (notifCode == LBN_DBLCLK) {
				int selection = ListBox_GetCurSel(idHwnd);
				if (selection != LB_ERR) {
					int textlen = ListBox_GetTextLen(idHwnd, selection)+1;
					TCHAR * extBuffer = new TCHAR[textlen];
					ListBox_GetText(idHwnd, selection, extBuffer);

					m_currentProfile->RemoveAsciiType(extBuffer);
					delete [] extBuffer;

					LoadFiletypes();
				}
			}
			break; }
		case IDC_LIST_BINARY: {
			if (notifCode == LBN_DBLCLK) {
				int selection = ListBox_GetCurSel(idHwnd);
				if (selection != LB_ERR) {
					int textlen = ListBox_GetTextLen(idHwnd, selection)+1;
					TCHAR * extBuffer = new TCHAR[textlen];
					ListBox_GetText(idHwnd, selection, extBuffer);

					m_currentProfile->RemoveBinaryType(extBuffer);
					delete [] extBuffer;

					LoadFiletypes();
				}
			}
			break; }

		case IDC_EDIT_CACHELOCAL:
		case IDC_EDIT_CACHEEXTERNAL: {
			if (notifCode == EN_KEYPRESS) {
				EnableCacheMapUI();
			}
			break; }
		case IDC_BUTTON_CACHE_ADD:
		case IDC_BUTTON_CACHE_EDIT: {
			PathMap pathmap;
			TCHAR local[MAX_PATH];
			char external[MAX_PATH];
			GetDlgItemText(m_hPageCache, IDC_EDIT_CACHELOCAL, local, MAX_PATH);
			GetDlgItemTextA(m_hPageCache, IDC_EDIT_CACHEEXTERNAL, external, MAX_PATH);
			pathmap.localpath = SU::DupString(local);
			pathmap.externalpath = SU::strdup(external);
			if (ctrlId == IDC_BUTTON_CACHE_ADD) {
				m_currentProfile->GetCache()->AddPathMap(pathmap);
			} else if (ctrlId == IDC_BUTTON_CACHE_EDIT) {
				HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
				int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);
				m_currentProfile->GetCache()->SetPathMap(pathmap, selectedindex);
			}
			LoadCacheMaps();
			break; }
		case IDC_BUTTON_CACHE_DELETE: {
			HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
			int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);
			if (selectedindex != -1) {
				m_currentProfile->GetCache()->DeletePathMap(selectedindex);
				LoadCacheMaps();
			}
			break; }
		case IDC_BUTTON_CACHEBROWSE: {
			int res = PU::BrowseDirectory(TTextBuffer, MAX_PATH, m_hwnd);
			if (res != -1) {
				SetDlgItemText(m_hPageCache, IDC_EDIT_CACHELOCAL, TTextBuffer);
				EnableCacheMapUI();
			}
			break; }
	}

	return TRUE;
}

INT_PTR ProfilesDialog::OnNotify(NMHDR * pnmh) {
	INT_PTR result = 0;

	if (pnmh->idFrom == IDC_LIST_CACHE) {
		NMLISTVIEW * pnml = (NMLISTVIEW*)pnmh;
		if (pnml->hdr.code == LVN_ITEMCHANGED) {
			if (pnml->uChanged | LVIS_SELECTED) {
				OnCacheMapSelect();
			}
		}
	} else if (pnmh->idFrom == IDC_TAB_PROFILEPAGE) {
		if (pnmh->code == TCN_SELCHANGE) {
			int index = TabCtrl_GetCurSel(pnmh->hwndFrom);
			m_pageConnection.Show(index == 0);
			m_pageAuthentication.Show(index == 1);
			m_pageTransfer.Show(index == 2);
			m_pageCache.Show(index == 3);
		}
	}

	return result;
}

INT_PTR ProfilesDialog::OnInitDialog() {
	m_currentProfile = NULL;

	HWND hTab = GetDlgItem(m_hwnd, IDC_TAB_PROFILEPAGE);

	TCITEM tci;
	tci.mask = TCIF_TEXT;

	tci.pszText = (TCHAR*)TEXT("Connection");
	TabCtrl_InsertItem(hTab, 0, &tci);

	tci.pszText = (TCHAR*)TEXT("Authentication");
	TabCtrl_InsertItem(hTab, 2, &tci);

	tci.pszText = (TCHAR*)TEXT("Transfers");
	TabCtrl_InsertItem(hTab, 3, &tci);

	tci.pszText = (TCHAR*)TEXT("Cache");
	TabCtrl_InsertItem(hTab, 4, &tci);

	TabCtrl_SetCurSel(hTab, 0);

	m_pageConnection.Create(hTab, m_hwnd, TEXT(""));
	m_pageAuthentication.Create(hTab, m_hwnd, TEXT(""));
	m_pageTransfer.Create(hTab, m_hwnd, TEXT(""));
	m_pageCache.Create(hTab, m_hwnd, TEXT(""));

	m_hPageConnection = m_pageConnection.GetHWND();
	m_hPageAuthentication = m_pageAuthentication.GetHWND();
	m_hPageTransfer= m_pageTransfer.GetHWND();
	m_hPageCache = m_pageCache.GetHWND();

	m_pageConnection.Show(true);

	_EditDefaultProc = (WNDPROC)::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_HOSTNAME), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_PORT), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_USERNAME), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_PASSWORD), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_TIMEOUT), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_INITDIR), GWL_WNDPROC, (DWORD)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_KEYFILE), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_PASSPHRASE), GWL_WNDPROC, (DWORD)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageTransfer, IDC_EDIT_ASCII), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageTransfer, IDC_EDIT_BINARY), GWL_WNDPROC, (DWORD)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHELOCAL), GWL_WNDPROC, (DWORD)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHEEXTERNAL), GWL_WNDPROC, (DWORD)&Dialog::EditProc);


	HWND hListCache = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	ListView_SetExtendedListViewStyle(hListCache, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;

	lvc.cx = 110;
	lvc.pszText = TEXT("Local path");
	ListView_InsertColumn(hListCache, 0, &lvc);

	lvc.cx = 110;
	lvc.pszText = TEXT("External path");
	ListView_InsertColumn(hListCache, 1, &lvc);

	::ShowWindow(::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE), SW_HIDE);

	HWND hCombobox = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY);
	ComboBox_AddString(hCombobox, TEXT("FTP"));
	ComboBox_AddString(hCombobox, TEXT("FTPES (Explicit)"));
	ComboBox_AddString(hCombobox, TEXT("FTPS (Implicit)"));
	ComboBox_AddString(hCombobox, TEXT("SFTP"));

	int ret = LoadProfiles();
	if (ret == -1) {
		EndDialog(m_hwnd, -1);
		return FALSE;
	}

	if (m_profiles->size() > 0) {
		HWND hListProfile = ::GetDlgItem(m_hwnd, IDC_LIST_PROFILES);
		ListBox_SelectItemData(hListProfile, 0, m_profiles->at(0));
		OnSelectProfile(m_profiles->at(0));
	} else {
		OnSelectProfile(NULL);
	}

	return FALSE;
}

int ProfilesDialog::LoadProfiles() {
	HWND hListProfile = ::GetDlgItem(m_hwnd, IDC_LIST_PROFILES);

	ListBox_ResetContent(hListProfile);

	for(size_t i = 0; i < m_profiles->size(); i++) {
		int index = ListBox_AddString(hListProfile, m_profiles->at(i)->GetName());
		if (index == LB_ERR || index == LB_ERRSPACE)
			return -1;
		ListBox_SetItemData(hListProfile, index, m_profiles->at(i));
	}

	return 0;
}

int ProfilesDialog::OnSelectProfile(FTPProfile * profile) {
	if (profile == m_currentProfile && profile != NULL)
		return 0;

	//BOOL prevEnable = (m_currentProfile!=NULL)?TRUE:FALSE;
	if (m_currentProfile)
		m_currentProfile->Release();
	m_currentProfile = profile;
	if (m_currentProfile)
		m_currentProfile->AddRef();

	int res = Clear();

	BOOL enableSettings = (profile!=NULL)?TRUE:FALSE;
	{ //if (enableSettings != prevEnable) {
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_RENAME), enableSettings);
		::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_DELETE), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_HOSTNAME), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_PORT), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_USERNAME), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_PASSWORD), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_TIMEOUT), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_INITDIR), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_ACTIVE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_PASSIVE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_ASCII), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_BINARY), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_LIST_ASCII), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_LIST_BINARY), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_EDIT_ASCII), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_EDIT_BINARY), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_LIST_CACHE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHELOCAL), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHEEXTERNAL), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_ADD), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_EDIT), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_DELETE), enableSettings);
	}

	if (!m_currentProfile || res == -1)
		return -1;

	BOOL enableProfileBtn = (profile!=NULL)?TRUE:FALSE;
	::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_RENAME), enableProfileBtn);
	::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_DELETE), enableProfileBtn);

	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_HOSTNAME, m_currentProfile->GetHostname());
	::SetDlgItemInt(m_hPageConnection, IDC_EDIT_PORT, m_currentProfile->GetPort(), FALSE);
	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_USERNAME, m_currentProfile->GetUsername());
	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_PASSWORD, m_currentProfile->GetPassword());

	::SetDlgItemInt(m_hPageConnection, IDC_EDIT_TIMEOUT, m_currentProfile->GetTimeout(), FALSE);

	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_INITDIR, m_currentProfile->GetInitialDir());


	AuthenticationMethods methods = m_currentProfile->GetAcceptedMethods();
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_PASSWORD), (methods&Method_Password)?TRUE:FALSE);
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_KEY), (methods&Method_Key)?TRUE:FALSE);
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_INTERACTIVE), (methods&Method_Interactive)?TRUE:FALSE);
	::SetDlgItemText(m_hPageAuthentication, IDC_EDIT_KEYFILE, m_currentProfile->GetKeyFile());
	::SetDlgItemTextA(m_hPageAuthentication, IDC_EDIT_PASSPHRASE, m_currentProfile->GetPassphrase());
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_AGENT), m_currentProfile->GetUseAgent()?TRUE:FALSE);

	HWND hCombobox = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY);
	ComboBox_SetCurSel(hCombobox, (int)m_currentProfile->GetSecurityMode());

	bool isActive = m_currentProfile->GetConnectionMode() == Mode_Active;
	bool isAscii = m_currentProfile->GetTransferMode() == Mode_ASCII;

	CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, isActive?IDC_RADIO_ACTIVE:IDC_RADIO_PASSIVE);
	CheckRadioButton(m_hPageTransfer, IDC_RADIO_ASCII, IDC_RADIO_BINARY, isAscii?IDC_RADIO_ASCII:IDC_RADIO_BINARY);

	LoadFiletypes();
	LoadCacheMaps();

	return 0;
}

int ProfilesDialog::Clear() {
	::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_RENAME), FALSE);
	::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_DELETE), FALSE);

	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_HOSTNAME, "");
	::SetDlgItemInt(m_hPageConnection, IDC_EDIT_PORT, 0, FALSE);
	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_USERNAME, "");
	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_PASSWORD, "");

	::SetDlgItemInt(m_hPageConnection, IDC_EDIT_TIMEOUT, 0, FALSE);

	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_INITDIR, "");

	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_PASSWORD), FALSE);
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_KEY), FALSE);
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_INTERACTIVE), FALSE);
	::SetDlgItemText(m_hPageAuthentication, IDC_EDIT_KEYFILE, TEXT(""));
	::SetDlgItemTextA(m_hPageAuthentication, IDC_EDIT_PASSPHRASE, "");
	//Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_AGENT), FALSE);

	HWND hCombobox = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY);
	ComboBox_SetCurSel(hCombobox, 0);

	CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, IDC_RADIO_ACTIVE);
	CheckRadioButton(m_hPageTransfer, IDC_RADIO_ASCII, IDC_RADIO_BINARY, IDC_RADIO_ASCII);

	HWND hListASCII = ::GetDlgItem(m_hPageTransfer, IDC_LIST_ASCII);
	HWND hListBinary = ::GetDlgItem(m_hPageTransfer, IDC_LIST_BINARY);
	ListBox_ResetContent(hListASCII);
	ListBox_ResetContent(hListBinary);

	HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	ListView_DeleteAllItems(hListview);

	::SetDlgItemText(m_hPageCache, IDC_EDIT_CACHELOCAL, TEXT(""));
	::SetDlgItemTextA(m_hPageCache, IDC_EDIT_CACHEEXTERNAL, "");

	::EnableWindow(::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE), FALSE);

	::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_ADD), FALSE);
	::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_EDIT), FALSE);
	::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_DELETE), FALSE);

	return 0;
}

int ProfilesDialog::LoadFiletypes() {
	HWND hListASCII = ::GetDlgItem(m_hPageTransfer, IDC_LIST_ASCII);
	HWND hListBinary = ::GetDlgItem(m_hPageTransfer, IDC_LIST_BINARY);

	ListBox_ResetContent(hListASCII);
	ListBox_ResetContent(hListBinary);

	for(int i = 0; i < m_currentProfile->GetAsciiCount(); i++) {
		ListBox_AddString(hListASCII, m_currentProfile->GetAsciiType(i));
	}
	for(int i = 0; i < m_currentProfile->GetBinaryCount(); i++) {
		ListBox_AddString(hListBinary, m_currentProfile->GetBinaryType(i));
	}

	return 0;
}

int ProfilesDialog::LoadCacheMaps() {
	HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	ListView_DeleteAllItems(hListview);

	FTPCache * cache = m_currentProfile->GetCache();
	if (!cache)
		return -1;

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = LVIS_SELECTED;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	for(int i = 0; i < cache->GetPathMapCount(); i++) {
		const PathMap & pathmap = cache->GetPathMap(i);
		lvi.state = (i == 0)?LVIS_SELECTED:0;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = (TCHAR*)pathmap.localpath;
		int index = ListView_InsertItem(hListview,  &lvi);
		if (index == -1)
			return -1;
		TCHAR * external = SU::Utf8ToTChar(pathmap.externalpath);
		ListView_SetItemText(hListview, index, 1, external);
		SU::FreeTChar(external);
	}

	OnCacheMapSelect();
	EnableCacheMapUI();

	return 0;
}

int ProfilesDialog::EnableCacheMapUI() {
	HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);

	HWND hLocalEdit = GetDlgItem(m_hPageCache, IDC_EDIT_CACHELOCAL);
	HWND hExternalEdit = GetDlgItem(m_hPageCache, IDC_EDIT_CACHEEXTERNAL);
	bool valid = GetWindowTextLength(hLocalEdit) > 0 && GetWindowTextLength(hExternalEdit) > 0;

	::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_ADD), valid?TRUE:FALSE);


	if (selectedindex == -1) {
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_EDIT), FALSE);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_DELETE), FALSE);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE), FALSE);
	} else {
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_EDIT), valid?TRUE:FALSE);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHE_DELETE), valid?TRUE:FALSE);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE), TRUE);
	}

	return 0;
}

int ProfilesDialog::OnCacheMapSelect() {
	HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);
	if (selectedindex == -1) {
		::SetDlgItemText(m_hPageCache, IDC_EDIT_CACHELOCAL, TEXT(""));
		::SetDlgItemTextA(m_hPageCache, IDC_EDIT_CACHEEXTERNAL, "");
	} else {
		FTPCache * cache = m_currentProfile->GetCache();
		const PathMap & pathmap = cache->GetPathMap(selectedindex);
		::SetDlgItemText(m_hPageCache, IDC_EDIT_CACHELOCAL, pathmap.localpath);
		::SetDlgItemTextA(m_hPageCache, IDC_EDIT_CACHEEXTERNAL, pathmap.externalpath);
	}

	return 0;
}
