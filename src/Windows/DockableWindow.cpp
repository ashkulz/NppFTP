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
#include "DockableWindow.h"

#include "Docking.h"
#include "dockingResource.h"
#include "Notepad_plus_msgs.h"

const int DockableWindow::_titleSize = 128;
const int DockableWindow::_infoSize = 128;

DockableWindow::DockableWindow(LPCTSTR classname) :
	Window(NULL, classname),
	m_titleSize(0),
	m_title(NULL),
	m_infoSize(0),
	m_info(NULL),
	m_icon(NULL),
	m_docklocation(DWS_DF_CONT_LEFT),
	m_moduleName(NULL),
	m_registered(false),
	m_displayed(false),
	m_hNpp(NULL),
	m_menuID(-1),
	m_menuCmd(-1)
{
	TCHAR module[MAX_PATH];
	int ret = GetModuleFileName(m_hInstance, module, MAX_PATH);
	if (ret == 0) {
		return;
	}

	TCHAR* fname = PathFindFileName(module);
	m_moduleName = new TCHAR[lstrlen(fname)+1];
	lstrcpy(m_moduleName, fname);

	m_titleSize = DockableWindow::_titleSize;
	m_title = new TCHAR[m_titleSize];
	m_title[0] = 0;

	m_infoSize = DockableWindow::_infoSize;
	m_info = new TCHAR[m_infoSize];
	m_info[0] = 0;

	ZeroMemory(&m_tbd, sizeof(m_tbd));
}

DockableWindow::~DockableWindow() {
	if (m_moduleName)
		delete [] m_moduleName;

	if (m_title)
		delete [] m_title;
	if (m_info)
		delete [] m_info;

	if (m_icon)
		DestroyIcon(m_icon);
}

int DockableWindow::Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand) {
	int ret = Window::Create(hParent);
	if (ret != 0)
		return -1;

	m_hNpp = hNpp;
	m_menuID = MenuID;
	m_menuCmd = MenuCommand;

	return 0;
}

int DockableWindow::Destroy() {
	//Unfortunately, N++ does not support Unregistering.....
	return Window::Destroy();
}

int DockableWindow::Show(bool show) {
	if (m_displayed == show)
		return 0;

	if (show && !m_registered) {
		int ret = RegisterDockableDialog();
		if (ret == -1)
			return -1;
	}

	if (show) {
		SendMessage(m_hNpp, NPPM_DMMSHOW, 0, (LPARAM)m_hwnd);							//Show my window as requested
		if (m_menuCmd != -1)
			SendMessage(m_hNpp, NPPM_SETMENUITEMCHECK, (WPARAM)m_menuCmd, (LPARAM)TRUE);	//Check the menu item

		//Redraw window for clean sheet
		InvalidateRect(m_hwnd, NULL, TRUE);
	} else {
		SendMessage(m_hNpp, NPPM_DMMHIDE, 0, (LPARAM)m_hwnd);							//Hide my window as requested
		if (m_menuCmd != -1)
			SendMessage(m_hNpp, NPPM_SETMENUITEMCHECK, (WPARAM)m_menuCmd, (LPARAM)FALSE);	//Uncheck the menu item
	}

	m_displayed = show;

	return 0;
}

bool DockableWindow::IsVisible() {
	return m_displayed;
}

LRESULT DockableWindow::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch(uMsg) {
		case WM_NOTIFY: {
			NMHDR nmh = (NMHDR) *((NMHDR*)lParam);
			if (nmh.hwndFrom == m_hNpp) {
				if (nmh.code == DMN_CLOSE) {
					Show(false);
					result = TRUE;
				} else if (nmh.code & DMN_DOCK || nmh.code & DMN_FLOAT) {
					::SendMessage(m_hwnd, WM_SIZE, 0, 0);
					Redraw();
					result = TRUE;
				}
			} else {
				result = Window::MessageProc(uMsg, wParam, lParam);
			}
			break; }
		default:
			result = Window::MessageProc(uMsg, wParam, lParam);
	}

	return result;
}

int DockableWindow::SetIcon(HICON icon) {
	if (m_registered)
		return -1;

	m_icon = icon;

	return 0;
}

int DockableWindow::SetLocation(int location) {
	if (m_registered)
		return -1;

	m_docklocation = location;

	return 0;
}

int DockableWindow::SetTitle(const TCHAR* title) {
	if (lstrlen(title) > (m_titleSize-1))
		return -1;

	lstrcpy(m_title, title);

	if (m_registered)
		UpdateDockInfo();

	return 0;
}

int DockableWindow::SetInfo(const TCHAR* info) {
	if (lstrlen(info) > (m_infoSize-1))
		return -1;

	lstrcpy(m_info, info);

	if (m_registered)
		UpdateDockInfo();

	return 0;
}

int DockableWindow::RegisterDockableDialog() {
	if (m_registered)	//it is an error to register twice
		return -1;

	m_registered = true;

	Window::Show(false);

	int flag = DWS_ADDINFO | m_docklocation;
	if (m_icon)
		flag |= DWS_ICONTAB;

	m_tbd.dlgID = m_menuID;												//Nr of menu item to assign (!= _cmdID, beware)
	m_tbd.hIconTab = m_icon;												//icon to use
	m_tbd.pszAddInfo = m_info;											//Titlebar info pointer
	m_tbd.pszModuleName = m_moduleName;									//name of the dll
	m_tbd.pszName = m_title;												//Name for titlebar
	m_tbd.uMask = flag;													//Flags to use (see docking.h)
	m_tbd.hClient = m_hwnd;												//HWND Handle of window this dock belongs to
	m_tbd.iPrevCont = -1;

	//Since setting rcFloat values is a bit buggy, resizing the window manually so floating windows stay in proper size
	Resize(200, 400);

	SendMessage(m_hNpp, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&m_tbd);		//Register it

	//Registering breaks window styles, reset them
	::SetWindowLongPtr(m_hwnd, GWL_STYLE, m_style|WS_CHILD);
	::SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, m_exStyle);

	return 0;
}

int DockableWindow::UpdateDockInfo() {
	SendMessage(m_hNpp, NPPM_DMMUPDATEDISPINFO, 0, (LPARAM)m_hwnd);

	return 0;
}
