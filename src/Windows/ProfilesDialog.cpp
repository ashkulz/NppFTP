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
#include <windowsx.h>

ProfilesDialog::ProfilesDialog() :
	Dialog(IDD_DIALOG_PROFILES),
	m_profiles(NULL),
	m_currentProfile(NULL),
	m_globalCache(NULL),
	m_ftpWindow(NULL),
	m_pageConnection(IDD_DIALOG_PROFILESCONNECTION),
	m_pageAuthentication(IDD_DIALOG_PROFILESAUTHENTICATION),
	m_pageTransfer(IDD_DIALOG_PROFILESTRANSFERS),
	m_pageFTP(IDD_DIALOG_PROFILESFTP),
	m_pageCache(IDD_DIALOG_PROFILESCACHE),
	m_hPageConnection(NULL),
	m_hPageTransfer(NULL),
	m_hPageFTP(NULL),
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
				FTPProfile::SortVector(*m_profiles);
				newProfile->AddRef();
				m_ftpWindow->OnProfileChange();
				OnSelectProfile(newProfile);
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

	if (!m_currentProfile) {
		return Dialog::OnCommand(ctrlId, notifCode, idHwnd);
	}

	//things that require a profile
	switch(ctrlId) {
		case IDC_BUTTON_PROFILE_RENAME: {
			InputDialog id;
			int res = id.Create(m_hwnd, TEXT("Renaming profile"), TEXT("Please enter the new name of the profile"), m_currentProfile->GetName());
			if (res == 1) {
				m_currentProfile->SetName(id.GetValue());
				FTPProfile::SortVector(*m_profiles);
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
			FTPProfile::SortVector(*m_profiles);
			m_ftpWindow->OnProfileChange();
			if (m_profiles->size() > 0) {
				OnSelectProfile(m_profiles->at(0));
			} else {
				OnSelectProfile(NULL);
			}
			LoadProfiles();
			break; }
		case IDC_BUTTON_PROFILE_COPY: {
			InputDialog id;
			TCHAR * newname = SU::TSprintfNB(TEXT("Copy of %T"), m_currentProfile->GetName());
			int res = id.Create(m_hwnd, TEXT("Copying profile"), TEXT("Please enter the name of the new profile"), newname);
			SU::FreeTChar(newname);
			if (res == 1) {
				FTPProfile * newProfile = new FTPProfile(id.GetValue(), m_currentProfile);
				newProfile->SetCacheParent(m_globalCache);
				m_profiles->push_back(newProfile);
				FTPProfile::SortVector(*m_profiles);
				newProfile->AddRef();
				m_ftpWindow->OnProfileChange();
				OnSelectProfile(newProfile);
				LoadProfiles();
			}
			break; }
		case IDC_EDIT_HOSTNAME: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetHostname(aTextBuffer);
			}
			break; }
		case IDC_EDIT_PORT: {
			if (notifCode == EN_USERCHANGE) {
				BOOL success = FALSE;
				int port = GetDlgItemInt(m_hPageConnection, ctrlId, &success, FALSE);
				if (success)
					m_currentProfile->SetPort(port);
			}
			break; }
		case IDC_EDIT_USERNAME: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetUsername(aTextBuffer);
			}
			break; }
		case IDC_EDIT_PASSWORD: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetPassword(aTextBuffer);
			}
			break; }
		case IDC_CHECK_ASKPASSWORD: {
			if (notifCode == BN_CLICKED) {
				LRESULT checked = Button_GetCheck(::GetDlgItem(m_hPageConnection, IDC_CHECK_ASKPASSWORD));
				::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_PASSWORD), !(checked == BST_CHECKED));
				m_currentProfile->SetAskPassword(checked == BST_CHECKED);
			}
			break; }
		case IDC_EDIT_TIMEOUT: {
			if (notifCode == EN_USERCHANGE) {
				BOOL success = FALSE;
				int timeout = GetDlgItemInt(m_hPageConnection, ctrlId, &success, FALSE);
				if (success)
					m_currentProfile->SetTimeout(timeout);
			}
			break; }
		case IDC_EDIT_INITDIR: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetInitialDir(aTextBuffer);
			}
			break; }
		case IDC_COMBO_SECURITY: {
			if (notifCode == CBN_SELCHANGE) {
				int sel = ComboBox_GetCurSel(idHwnd);

				if (LockZOS())
					break;

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
							default: break;
						}
						::SetDlgItemInt(m_hPageConnection, IDC_EDIT_PORT,  m_currentProfile->GetPort(), TRUE);
					}
				}
			}
			break; }
		case IDC_COMBO_SERVER: {
			if (notifCode == CBN_SELCHANGE) {
				int sel = ComboBox_GetCurSel(idHwnd);
				if (sel != CB_ERR) {
					m_currentProfile->SetServerType((Server_Type)sel);

					if ((Server_Type)sel == Server_ZOS) {
						// ZOS only works with these attributes
						m_currentProfile->SetConnectionMode(Mode_Passive);
						m_currentProfile->SetTransferMode(Mode_ASCII);
						m_currentProfile->SetSecurityMode(Mode_FTP);
						
						// Set port if not custom
						int port = m_currentProfile->GetPort();
						bool setport = (port == 21 || port == 22 || port == 990);
						if (setport) {
							m_currentProfile->SetPort(21);
						}

						OnSelectProfile(m_currentProfile, TRUE);
					}
				}
			}
			break; }
		case IDC_EDIT_KEYFILE: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowText(idHwnd, TTextBuffer, MAX_PATH);
				m_currentProfile->SetKeyFile(TTextBuffer);
			}
			break; }
		case IDC_EDIT_PASSPHRASE: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetPassphrase(aTextBuffer);
			}
			break; }
		case IDC_CHECK_ASKPASSPHRASE: {
			if (notifCode == BN_CLICKED) {
				LRESULT checked = Button_GetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_ASKPASSPHRASE));
				::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_PASSPHRASE), !(checked == BST_CHECKED));
				m_currentProfile->SetAskPassphrase(checked == BST_CHECKED);
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
				if (LockZOS())
					break;

				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, IDC_RADIO_ACTIVE);
				m_currentProfile->SetConnectionMode(Mode_Active);
			}
			break; }
		case IDC_RADIO_PASSIVE: {
			if (notifCode == BN_CLICKED) {
				if (LockZOS())
					break;

				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, IDC_RADIO_PASSIVE);
				m_currentProfile->SetConnectionMode(Mode_Passive);
			}
			break; }
		case IDC_RADIO_ASCII: {
			if (notifCode == BN_CLICKED) {
				if (LockZOS())
					break;

				CheckRadioButton(m_hPageTransfer, IDC_RADIO_ASCII, IDC_RADIO_BINARY, IDC_RADIO_ASCII);
				m_currentProfile->SetTransferMode(Mode_ASCII);
			}
			break; }
		case IDC_RADIO_BINARY: {
			if (notifCode == BN_CLICKED) {
				if (LockZOS())
					break;

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
		case IDC_EDIT_PORT_MIN:
		case IDC_EDIT_PORT_MAX: {
			if (notifCode == EN_USERCHANGE) {
				BOOL success = FALSE;
				int min = GetDlgItemInt(m_hPageTransfer, IDC_EDIT_PORT_MIN, &success, FALSE);
				if (!success)
					break;
				int max = GetDlgItemInt(m_hPageTransfer, IDC_EDIT_PORT_MAX, &success, FALSE);
				if (!success)
					break;
				m_currentProfile->SetDataPortRange(min, max);
			}
			break; }

		case IDC_EDIT_LISTPARAMS: {
			if (notifCode == EN_USERCHANGE) {
				GetWindowTextA(idHwnd, aTextBuffer, MAX_PATH);
				m_currentProfile->SetListParams(aTextBuffer);
			}
			break; }

		case IDC_EDIT_CACHELOCAL:
		case IDC_EDIT_CACHEEXTERNAL: {
			if (notifCode == EN_USERCHANGE) {
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
				LoadCacheMaps();
			} else if (ctrlId == IDC_BUTTON_CACHE_EDIT) {
				HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
				int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);
				m_currentProfile->GetCache()->SetPathMap(pathmap, selectedindex);
				LoadCacheMaps();
			}
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
		default: {
			return Dialog::OnCommand(ctrlId, notifCode, idHwnd);
			break; }
	}

	return TRUE;
}

INT_PTR ProfilesDialog::OnNotify(NMHDR * pnmh) {
	INT_PTR result = 0;

	if (pnmh->idFrom == IDC_LIST_CACHE) {
		NMLISTVIEW * pnml = (NMLISTVIEW*)pnmh;
		if (pnml->hdr.code == LVN_ITEMCHANGED) {
			if (pnml->uChanged & LVIF_STATE && pnml-> uNewState & LVIS_SELECTED) {
				OnCacheMapSelect();
			}
		}
	} else if (pnmh->idFrom == IDC_TAB_PROFILEPAGE) {
		if (pnmh->code == TCN_SELCHANGE) {
			int index = TabCtrl_GetCurSel(pnmh->hwndFrom);
			m_pageConnection.Show(index == 0);
			m_pageAuthentication.Show(index == 1);
			m_pageTransfer.Show(index == 2);
			m_pageFTP.Show(index == 3);
			m_pageCache.Show(index == 4);
		}
	} else if (pnmh->idFrom == IDC_SPIN_CACHE) {
		NMUPDOWN * pnmud = (NMUPDOWN*)pnmh;
		if (pnmh->code == UDN_DELTAPOS) {
			if (!m_currentProfile)
				return TRUE;

			HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);

			FTPCache * cache = m_currentProfile->GetCache();

			int count = cache->GetPathMapCount();
			int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);
			int newindex = selectedindex+(pnmud->iDelta<0?-1:1);
			if (newindex < 0 || newindex > count-1)
				return TRUE;	//cannot accept move

			cache->SwapPathMap(selectedindex, newindex);

			LoadCacheMaps();

			ListView_SetItemState(hListview, newindex, LVIS_FOCUSED | LVIS_SELECTED, 0x000F);

			return FALSE;
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

	tci.pszText = (TCHAR*)TEXT("FTP Misc.");
	TabCtrl_InsertItem(hTab, 4, &tci);

	tci.pszText = (TCHAR*)TEXT("Cache");
	TabCtrl_InsertItem(hTab, 5, &tci);

	TabCtrl_SetCurSel(hTab, 0);

	m_pageConnection.Create(m_hwnd, m_hwnd, TEXT(""));
	m_pageAuthentication.Create(m_hwnd, m_hwnd, TEXT(""));
	m_pageTransfer.Create(m_hwnd, m_hwnd, TEXT(""));
	m_pageFTP.Create(m_hwnd, m_hwnd, TEXT(""));
	m_pageCache.Create(m_hwnd, m_hwnd, TEXT(""));

	PF::EnableThemeDialogTexture(m_pageConnection.GetHWND(), ETDT_ENABLETAB);
	PF::EnableThemeDialogTexture(m_pageAuthentication.GetHWND(), ETDT_ENABLETAB);
	PF::EnableThemeDialogTexture(m_pageTransfer.GetHWND(), ETDT_ENABLETAB);
	PF::EnableThemeDialogTexture(m_pageFTP.GetHWND(), ETDT_ENABLETAB);
	PF::EnableThemeDialogTexture(m_pageCache.GetHWND(), ETDT_ENABLETAB);
	//PF::EnableThemeDialogTexture(m_hwnd, ETDT_ENABLETAB);

	RECT tabRect;
	::GetClientRect(hTab, (LPRECT)&tabRect);
	::MapWindowPoints(hTab, m_hwnd, (LPPOINT)&tabRect, 2);
	TabCtrl_AdjustRect(hTab, FALSE, &tabRect);

	m_pageConnection.Move(tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top);
	m_pageAuthentication.Move(tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top);
	m_pageTransfer.Move(tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top);
	m_pageFTP.Move(tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top);
	m_pageCache.Move(tabRect.left, tabRect.top, tabRect.right-tabRect.left, tabRect.bottom-tabRect.top);

	m_hPageConnection = m_pageConnection.GetHWND();
	m_hPageAuthentication = m_pageAuthentication.GetHWND();
	m_hPageTransfer = m_pageTransfer.GetHWND();
	m_hPageFTP = m_pageFTP.GetHWND();
	m_hPageCache = m_pageCache.GetHWND();

	m_pageConnection.Show(true);

	_EditDefaultProc = (WNDPROC)::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_HOSTNAME), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_PORT), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_USERNAME), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_PASSWORD), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_TIMEOUT), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageConnection, IDC_EDIT_INITDIR), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_KEYFILE), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_PASSPHRASE), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageTransfer, IDC_EDIT_ASCII), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageTransfer, IDC_EDIT_BINARY), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageTransfer, IDC_EDIT_PORT_MIN), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageTransfer, IDC_EDIT_PORT_MAX), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageFTP, IDC_EDIT_LISTPARAMS), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);

	::SetWindowLongPtr(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHELOCAL), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);
	::SetWindowLongPtr(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHEEXTERNAL), GWLP_WNDPROC, (LONG_PTR)&Dialog::EditProc);


	HWND hListCache = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	ListView_SetExtendedListViewStyle(hListCache, LVS_EX_FULLROWSELECT);

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;

	lvc.cx = 110;
	TCHAR strLocalPath[] = TEXT("Local path");
	lvc.pszText = strLocalPath;
	ListView_InsertColumn(hListCache, 0, &lvc);

	lvc.cx = 110;
	TCHAR strExternalPath[] = TEXT("External path");
	lvc.pszText = strExternalPath;
	ListView_InsertColumn(hListCache, 1, &lvc);

	HWND hCombobox = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY);
	ComboBox_AddString(hCombobox, TEXT("FTP"));
	ComboBox_AddString(hCombobox, TEXT("FTPES (Explicit)"));
	ComboBox_AddString(hCombobox, TEXT("FTPS (Implicit)"));
	ComboBox_AddString(hCombobox, TEXT("SFTP"));

	HWND hCombobox2 = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SERVER);
	ComboBox_AddString(hCombobox2, TEXT("Default"));
	ComboBox_AddString(hCombobox2, TEXT("MVS, OS/390, z/OS"));

	int ret = LoadProfiles();
	if (ret == -1) {
		EndDialog(m_hwnd, -1);
		return FALSE;
	}

	if (m_profiles->size() > 0) {
		HWND hListProfile = ::GetDlgItem(m_hwnd, IDC_LIST_PROFILES);
		ListBox_SetCurSel(hListProfile, 0);
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
		if (m_currentProfile == m_profiles->at(i)) {
			ListBox_SetCurSel(hListProfile, index);
		}
	}

	return 0;
}

BOOL ProfilesDialog::LockZOS() {
	if (m_currentProfile->GetServerType() == Server_ZOS) {
		OnSelectProfile(m_currentProfile, TRUE);
		return TRUE;
	}

	return FALSE;
}

int ProfilesDialog::OnSelectProfile(FTPProfile * profile, BOOL force) {
	if (profile == m_currentProfile && profile != NULL && !force)
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
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_CHECK_ASKPASSWORD), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_TIMEOUT), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_INITDIR), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_PASSWORD), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_KEY), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_INTERACTIVE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_KEYFILE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_BUTTON_KEYBROWSE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_PASSPHRASE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_AGENT), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_ACTIVE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_PASSIVE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_ASCII), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_RADIO_BINARY), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_LIST_ASCII), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_LIST_BINARY), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_EDIT_ASCII), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_EDIT_BINARY), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_EDIT_PORT_MIN), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageTransfer, IDC_EDIT_PORT_MAX), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageFTP, IDC_EDIT_LISTPARAMS), enableSettings);

		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_LIST_CACHE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_EDIT_CACHELOCAL), enableSettings);
		::EnableWindow(::GetDlgItem(m_hPageCache, IDC_BUTTON_CACHEBROWSE), enableSettings);
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
	::EnableWindow(::GetDlgItem(m_hwnd, IDC_BUTTON_PROFILE_COPY), enableProfileBtn);

	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_HOSTNAME, m_currentProfile->GetHostname());
	::SetDlgItemInt(m_hPageConnection, IDC_EDIT_PORT, m_currentProfile->GetPort(), FALSE);
	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_USERNAME, m_currentProfile->GetUsername());
	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_PASSWORD, m_currentProfile->GetPassword());
	Button_SetCheck(::GetDlgItem(m_hPageConnection, IDC_CHECK_ASKPASSWORD), (m_currentProfile->GetAskPassword())?TRUE:FALSE);
	::EnableWindow(::GetDlgItem(m_hPageConnection, IDC_EDIT_PASSWORD), !(m_currentProfile->GetAskPassword()));

	::SetDlgItemInt(m_hPageConnection, IDC_EDIT_TIMEOUT, m_currentProfile->GetTimeout(), FALSE);

	::SetDlgItemTextA(m_hPageConnection, IDC_EDIT_INITDIR, m_currentProfile->GetInitialDir());


	AuthenticationMethods methods = m_currentProfile->GetAcceptedMethods();
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_PASSWORD), (methods&Method_Password)?TRUE:FALSE);
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_KEY), (methods&Method_Key)?TRUE:FALSE);
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_INTERACTIVE), (methods&Method_Interactive)?TRUE:FALSE);
	::SetDlgItemText(m_hPageAuthentication, IDC_EDIT_KEYFILE, m_currentProfile->GetKeyFile());
	::SetDlgItemTextA(m_hPageAuthentication, IDC_EDIT_PASSPHRASE, m_currentProfile->GetPassphrase());
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_ASKPASSPHRASE), (m_currentProfile->GetAskPassphrase())?TRUE:FALSE);
	::EnableWindow(::GetDlgItem(m_hPageAuthentication, IDC_EDIT_PASSPHRASE), !(m_currentProfile->GetAskPassphrase()));
	Button_SetCheck(::GetDlgItem(m_hPageAuthentication, IDC_CHECK_AGENT), m_currentProfile->GetUseAgent()?TRUE:FALSE);

	HWND hCombobox = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SECURITY);
	ComboBox_SetCurSel(hCombobox, (int)m_currentProfile->GetSecurityMode());

	HWND hCombobox2 = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SERVER);
	ComboBox_SetCurSel(hCombobox2, (int)m_currentProfile->GetServerType());

	bool isActive = m_currentProfile->GetConnectionMode() == Mode_Active;
	bool isAscii = m_currentProfile->GetTransferMode() == Mode_ASCII;

	CheckRadioButton(m_hPageTransfer, IDC_RADIO_ACTIVE, IDC_RADIO_PASSIVE, isActive?IDC_RADIO_ACTIVE:IDC_RADIO_PASSIVE);
	CheckRadioButton(m_hPageTransfer, IDC_RADIO_ASCII, IDC_RADIO_BINARY, isAscii?IDC_RADIO_ASCII:IDC_RADIO_BINARY);


	int min;
	int max;
	m_currentProfile->GetDataPortRange(&min, &max);
	::SetDlgItemInt(m_hPageTransfer, IDC_EDIT_PORT_MIN, min, FALSE);
	::SetDlgItemInt(m_hPageTransfer, IDC_EDIT_PORT_MAX, max, FALSE);

	::SetDlgItemTextA(m_hPageFTP, IDC_EDIT_LISTPARAMS, m_currentProfile->GetListParams());

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

	HWND hCombobox2 = ::GetDlgItem(m_hPageConnection, IDC_COMBO_SERVER);
	ComboBox_SetCurSel(hCombobox2, 0);

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
	if (!m_currentProfile) {
		ListView_DeleteAllItems(hListview);
		return 0;
	}

	FTPCache * cache = m_currentProfile->GetCache();

	int curSelection = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);

	if (curSelection >= cache->GetPathMapCount())
		curSelection = cache->GetPathMapCount()-1;
	if (curSelection == -1)
		curSelection = 0;

	ListView_DeleteAllItems(hListview);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_STATE;
	lvi.state = 0;
	lvi.stateMask = LVIS_SELECTED;
	lvi.iItem = 0;
	lvi.iSubItem = 0;

	for(int i = 0; i < cache->GetPathMapCount(); i++) {
		const PathMap & pathmap = cache->GetPathMap(i);
		lvi.state = (i == curSelection)?LVIS_SELECTED:0;
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
	UpdateCacheMapSpinner();

	return 0;
}

int ProfilesDialog::UpdateCacheMapSpinner() {
	HWND hListview = ::GetDlgItem(m_hPageCache, IDC_LIST_CACHE);
	HWND hSpin = ::GetDlgItem(m_hPageCache, IDC_SPIN_CACHE);
	FTPCache * cache = m_currentProfile->GetCache();
	int count = cache->GetPathMapCount();

	int selectedindex = ListView_GetNextItem(hListview, -1, LVNI_ALL|LVNI_SELECTED);
	if (selectedindex == -1) {
		::SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(0,0) );
		::SendMessage(hSpin, UDM_SETPOS, 0, 0);

	} else {
		::SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(0,count-1) );
		::SendMessage(hSpin, UDM_SETPOS, 0, selectedindex );
	}
	return 0;
}
