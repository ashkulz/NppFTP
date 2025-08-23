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
#include "Dialog.h"
#include "Npp/PluginInterface.h"

#include "resource.h"

WNDPROC Dialog::_EditDefaultProc = NULL;

Dialog::Dialog(int dialogResource) :
	Window(NULL, NULL),
	m_title(NULL)
{
	if (dialogResource == 0) {
		m_dialogResource = IDD_DIALOG_GENERIC;
	} else {
		m_dialogResource = dialogResource;
	}
}

Dialog::~Dialog() {
	if (m_title) {
		SU::FreeTChar(m_title);
		m_title = NULL;
	}
}

int Dialog::Create(HWND hParent, bool isModal, const TCHAR * title) {
	m_hParent = hParent;

	if (m_title)
		SU::FreeTChar(m_title);
	if (title)
		m_title = SU::DupString(title);
	else
		m_title = NULL;

	if (isModal) {
		INT_PTR res = DialogBoxParam(m_hInstance, MAKEINTRESOURCE(m_dialogResource), m_hParent, &Dialog::DialogProc, (LPARAM)this);
		if (res == 0 || res == -1)
			return -1;
		return res;
	} else {
		HWND hRes = CreateDialogParam(m_hInstance, MAKEINTRESOURCE(m_dialogResource), m_hParent, &Dialog::DialogProc, (LPARAM)this);
		if (hRes == NULL)
			return -1;
		//m_hwnd = hRes;	//Done by DlgProc
		return 0;
	}
}

INT_PTR CALLBACK Dialog::DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Dialog * dlg = NULL;

	switch (uMsg) {
		case WM_INITDIALOG:
			dlg = (Dialog*)lParam;
			if (!dlg)
				return FALSE;
			dlg->m_hwnd = hwnd;
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dlg));
			return dlg->OnInitDialog();
		default :
			dlg = (Dialog*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (!dlg)
				return FALSE;
			return dlg->DlgMsgProc(uMsg, wParam, lParam);
	}

	return FALSE;
}


INT_PTR Dialog::DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	INT_PTR result = FALSE;

	switch(uMsg) {
		case WM_CLOSE: {
			EndDialog(m_hwnd, 99);
			result = TRUE;
			break; }
		case WM_COMMAND: {
			result = OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
			break; }
		case WM_NOTIFY: {
			result = OnNotify((NMHDR*)lParam);
			break; }
		default: {
			result = FALSE;
			break; }
	}

	return result;
}

INT_PTR Dialog::OnInitDialog() {
	if (m_title != NULL)
		::SetWindowText(m_hwnd, m_title);

	//Modified for darkmode support
	::SendMessage(_MainOutputWindow, NPPM_DARKMODESUBCLASSANDTHEME, static_cast<WPARAM>(NppDarkMode::dmfInit), reinterpret_cast<LPARAM>(m_hwnd));

	return TRUE;	//Allow focus to be set
}

INT_PTR Dialog::OnCommand(int ctrlId, int /*notifCode*/, HWND /*idHwnd*/) {
	if (ctrlId == IDCANCEL) {
		EndDialog(m_hwnd, 99);
		return TRUE;
	}
	return FALSE;
}

INT_PTR Dialog::OnNotify(NMHDR * /*pnmh*/) {
	return FALSE;
}

LRESULT Dialog::EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HWND hParent = ::GetParent(hwnd);

	switch(uMsg) {
		case WM_GETDLGCODE: {
			LRESULT lres = DLGC_WANTMESSAGE | DLGC_HASSETSEL;
			lres &= ~DLGC_WANTTAB;

			if (lParam && ((MSG *)lParam)->message == WM_KEYDOWN) {
				WPARAM key = ((MSG *)lParam)->wParam;

				switch(key) {
					case VK_RETURN: {
						LONG style = ::GetWindowLongPtr(hwnd, GWL_STYLE);
						if (style & ES_WANTRETURN)
							break;
						//fallthrough
						}
					case VK_TAB:
					case VK_ESCAPE: {
						lres &= ~DLGC_WANTMESSAGE;
						break; }
					default: {
						break; }
				}
			}
			return lres;
			break; }
		case WM_CHAR: {
			if (wParam == '\r' || wParam == '\n')
				return 0;
			break; }
		case WM_KEYUP: {
			WORD code = LOWORD(wParam);
			switch(code) {
				case VK_RETURN: {
					return 0;
					break; }
				case VK_DELETE: {
					if (hParent) {
						int id = (int)::GetWindowLongPtr(hwnd, GWLP_ID);
						::SendMessage(hParent, WM_COMMAND, MAKEWPARAM(id, EN_USERCHANGE), (LPARAM)hwnd);
					}
					break; }
			}
			break; }
		case WM_KEYDOWN: {
			WORD code = LOWORD(wParam);
			switch(code) {
				case VK_RETURN: {
					if (hParent) {
						int id = (int)::GetWindowLongPtr(hwnd, GWLP_ID);
						::SendMessage(hParent, WM_COMMAND, MAKEWPARAM(id, EN_KEYRETURN), (LPARAM) hwnd);
						return 0;
					}
					break; }
			}
			break; }
	}

	LRESULT res = CallWindowProc(Dialog::_EditDefaultProc, hwnd, uMsg, wParam, lParam);

	switch(uMsg) {
		case WM_CHAR: {
			if (hParent) {
				int id = (int)::GetWindowLongPtr(hwnd, GWLP_ID);
				::SendMessage(hParent, WM_COMMAND, MAKEWPARAM(id, EN_USERCHANGE), (LPARAM) hwnd);
			}
		break; }
		case WM_PASTE: {
			if (hParent) {
				int id = (int)::GetWindowLongPtr(hwnd, GWLP_ID);
				::SendMessage(hParent, WM_COMMAND, MAKEWPARAM(id, EN_USERCHANGE), (LPARAM) hwnd);
			}
			break; }
	}

	return res;
}
