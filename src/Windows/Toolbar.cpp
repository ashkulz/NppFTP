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

#include "../StdInc.h"
#include "Toolbar.h"

#include "resource.h"
#include "Commands.h"

//NOTE: the bitmap IDs have to be translated by TB_ADDBITMAP
const TBBUTTON defaultButtons[] = {
	//iBitmap, idCommand, fsState, fsStyle, dwData, iString
	{   IDB_BITMAP_CONNECT,  IDB_BUTTON_TOOLBAR_CONNECT,               0, TBSTYLE_DROPDOWN, {0,0}, 0, 0},
	//IDB_BITMAP_DISCONNECT
	{                    0,                           0, TBSTATE_ENABLED,      TBSTYLE_SEP, {0,0}, 0, 0},
	{  IDB_BITMAP_OPENDIR, IDB_BUTTON_TOOLBAR_OPENDIR, TBSTATE_ENABLED,      BTNS_BUTTON, {0,0}, 0, 0},
	{  IDB_BITMAP_DOWNLOAD, IDB_BUTTON_TOOLBAR_DOWNLOAD,               0,      BTNS_BUTTON, {0,0}, 0, 0},
	{    IDB_BITMAP_UPLOAD,   IDB_BUTTON_TOOLBAR_UPLOAD,               0,      BTNS_BUTTON, {0,0}, 0, 0},
	{   IDB_BITMAP_REFRESH,  IDB_BUTTON_TOOLBAR_REFRESH,               0,      BTNS_BUTTON, {0,0}, 0, 0},
	{     IDB_BITMAP_ABORT,    IDB_BUTTON_TOOLBAR_ABORT,               0,      BTNS_BUTTON, {0,0}, 0, 0},
	//{                    0,                           0, TBSTATE_ENABLED,      TBSTYLE_SEP, {0,0}, 0, 0},
	//{IDB_BITMAP_RAWCOMMAND,   IDB_BUTTON_TOOLBAR_RAWCMD,               0,      BTNS_BUTTON, {0,0}, 0, 0},
	{                    0,                           0, TBSTATE_ENABLED,      TBSTYLE_SEP, {0,0}, 0, 0},
	{  IDB_BITMAP_SETTINGS, IDB_BUTTON_TOOLBAR_SETTINGS,               0, TBSTYLE_DROPDOWN, {0,0}, 0, 0},
	{  IDB_BITMAP_MESSAGES, IDB_BUTTON_TOOLBAR_MESSAGES, TBSTATE_ENABLED,      BTNS_BUTTON, {0,0}, 0, 0}
};

const TCHAR* tooltips[] = {
	TEXT("(Dis)Connect"),
	TEXT("-"),
	TEXT("Open Directory"),
	TEXT("Download file"),
	TEXT("Upload file"),
	TEXT("Refresh"),
	TEXT("Abort"),
	//TEXT("-"),
	//TEXT("Send quote"),
	TEXT("-"),
	TEXT("Settings"),
	TEXT("Show messages window")
};

const size_t nrDefaultButtons = sizeof(defaultButtons)/sizeof(defaultButtons[0]);

Toolbar::Toolbar() :
	Window(NULL, TOOLBARCLASSNAME),
	m_nrButtons(0),
	m_buttons(NULL),
	m_buttonMenus(NULL),
	m_connectBitmapIndex(-1),
	m_disconnectBitmapIndex(-1),
	m_rebar(NULL)
{
	m_exStyle = WS_EX_PALETTEWINDOW;
	m_style = WS_CHILD|/*WS_VISIBLE|*/WS_CLIPCHILDREN|WS_CLIPSIBLINGS|TBSTYLE_TOOLTIPS|CCS_TOP|TBSTYLE_FLAT|BTNS_AUTOSIZE|CCS_NOPARENTALIGN|CCS_NORESIZE|CCS_NODIVIDER;
}

Toolbar::~Toolbar() {
}

int Toolbar::Create(HWND hParent) {
	int res = Window::Create(hParent);
	if (res == -1)
		return -1;

	SendMessage(m_hwnd, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
	SendMessage(m_hwnd, TB_SETBITMAPSIZE, 0, (LPARAM)MAKELONG(16, 16));
	SendMessage(m_hwnd, TB_SETBUTTONSIZE, 0, (LPARAM)MAKELONG(18, 18));
	SendMessage(m_hwnd, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_HIDECLIPPEDBUTTONS);

	TBADDBITMAP ab;
	ab.hInst = m_hInstance;

	int imgnr = 0;
	m_buttons = new TBBUTTON[nrDefaultButtons];
	m_buttonMenus = new HMENU[nrDefaultButtons];
	m_nrButtons = nrDefaultButtons;

	for(size_t i = 0; i < nrDefaultButtons; i++) {
		m_buttons[i] = defaultButtons[i];
		m_buttonMenus[i] = NULL;
		ab.nID = m_buttons[i].iBitmap;
		if (ab.nID != 0)
			imgnr = SendMessage(m_hwnd, TB_ADDBITMAP, (WPARAM) 1, (LPARAM) &ab);
		else
			imgnr = 0;
		m_buttons[i].iBitmap = imgnr;
	}

	m_connectBitmapIndex = m_buttons[0].iBitmap;
	ab.nID = IDB_BITMAP_DISCONNECT;
	m_disconnectBitmapIndex = ::SendMessage(m_hwnd, TB_ADDBITMAP, (WPARAM) 1, (LPARAM) &ab);

	SendMessage(m_hwnd, TB_ADDBUTTONS, (WPARAM)nrDefaultButtons, (LPARAM)m_buttons);
	SendMessage(m_hwnd, TB_AUTOSIZE, 0, 0);

	return 0;
}

int Toolbar::Destroy() {
	if (m_rebar) {
		m_rebar->RemoveBand(m_rbBand.wID);
		m_rebar = NULL;
	}

	int ret = Window::Destroy();

	delete [] m_buttons;

	return ret;
}

int Toolbar::Enable(int cmdID, bool doEnable) const {
	LRESULT res = ::SendMessage(m_hwnd, TB_ENABLEBUTTON, cmdID, (LPARAM)doEnable);

	if (res == FALSE)
		return -1;

	return 0;
}

int Toolbar::GetWidth() const {
	RECT btnRect;
	int totalWidth = 0;
	for(size_t i = 0; i < m_nrButtons; i++) {
		LRESULT ret = ::SendMessage(m_hwnd, TB_GETITEMRECT, i, (LPARAM)&btnRect);
		if (ret == FALSE)
			return -1;
		totalWidth += btnRect.right - btnRect.left;
	}
	return totalWidth;
}

int Toolbar::GetHeight() const {
	DWORD size = (DWORD)SendMessage(m_hwnd, TB_GETBUTTONSIZE, 0, 0);
    DWORD padding = (DWORD)SendMessage(m_hwnd, TB_GETPADDING, 0,0);
	int totalHeight = HIWORD(size) + HIWORD(padding);

	return totalHeight;
}

bool Toolbar::GetChecked(int cmdID) const {
	LRESULT state = ::SendMessage(m_hwnd, TB_GETSTATE, (WPARAM)cmdID, 0);
	if (state == -1)
		return false;

	bool checked = (state & TBSTATE_CHECKED) != 0;
	return checked;
}

int Toolbar::SetChecked(int cmdID, BOOL check) {
	LRESULT ret = SendMessage(m_hwnd, TB_CHECKBUTTON, (WPARAM)cmdID, (LPARAM)MAKELONG(check, 0));

	if (ret == FALSE)
		return -1;

	return 0;
}

int Toolbar::SetMenu(int cmdID, HMENU menu) {
	int index = ::SendMessage(m_hwnd, TB_COMMANDTOINDEX, cmdID, 0);
	if (index == -1) {
		return -1;
	}

	m_buttonMenus[index] = menu;
	return 0;
}

int Toolbar::DoPopop(POINT chevPoint) {
	//first find hidden buttons
	int width = Window::GetWidth();

	size_t start = 0;
	RECT btnRect = {0,0,0,0};
	while(start < m_nrButtons) {
		LRESULT res = ::SendMessage(m_hwnd, TB_GETITEMRECT, start, (LPARAM)&btnRect);
		if (res == FALSE)
			return -1;
		if(btnRect.right > width)
			break;
		start++;
	}

	if (start < m_nrButtons) {	//some buttons are hidden
		HMENU menu = ::CreatePopupMenu();
		size_t curIndex = start;
		while (curIndex < m_nrButtons) {
			if (m_buttons[curIndex].idCommand != 0) {
				const TCHAR* name = tooltips[curIndex];
				int id = m_buttons[curIndex].idCommand;
				int flags = MF_STRING;
				if (m_buttonMenus[curIndex] != NULL) {
					id = (UINT_PTR)m_buttonMenus[curIndex];
					flags |= MF_POPUP;
				}
				if (::SendMessage(m_hwnd, TB_ISBUTTONENABLED, m_buttons[curIndex].idCommand, 0) != 0) {
					flags |= MF_ENABLED;
				} else {
					flags |= MF_GRAYED;
				}
				AppendMenu(menu, flags, id, name);
			} else
				AppendMenu(menu, MF_SEPARATOR, 0, TEXT(""));
			curIndex++;
		}
		TrackPopupMenu(menu, 0, chevPoint.x, chevPoint.y, 0, m_hwnd, NULL);
		for(size_t i = start; i < curIndex; i++) {
			::RemoveMenu(menu, 0, MF_BYPOSITION);
		}
		::DestroyMenu(menu);
	}

	return 0;
}

const TCHAR* Toolbar::GetTooltip(int cmdID) const {
	int index = ::SendMessage(m_hwnd, TB_COMMANDTOINDEX, cmdID, 0);
	if (index == -1) {
		return NULL;
	}

	return tooltips[index];
}

int Toolbar::DoDropDown(int buttonID) {
	int index = ::SendMessage(m_hwnd, TB_COMMANDTOINDEX, buttonID, 0);
	if (index == -1) {
		return TBDDRET_NODEFAULT;
	}

	HMENU menu = m_buttonMenus[index];
	if (menu == NULL)
		return TBDDRET_NODEFAULT;

	POINT pos;
	RECT offsetRect;
	if (SendMessage(m_hwnd, TB_GETITEMRECT, index, (LPARAM)&offsetRect) == TRUE) {
		pos.x = offsetRect.left;
		pos.y = offsetRect.bottom + 1;
		if (GetWindowRect(m_hwnd, &offsetRect) == TRUE) {
			pos.x += offsetRect.left;
			pos.y += offsetRect.top;
		} else {
			GetCursorPos(&pos);
		}
	} else {
		GetCursorPos(&pos);
	}
	::SendMessage(m_hwnd, TB_CHECKBUTTON, (WPARAM)buttonID, (LPARAM)TRUE);
	TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON, pos.x, pos.y, 0, m_hParent, NULL);	//TODO: does this pass the wm_command msg correctly?
	::SendMessage(m_hwnd, TB_CHECKBUTTON, (WPARAM)buttonID, (LPARAM)FALSE);
	return TBDDRET_DEFAULT;
}

int Toolbar::AddToRebar(Rebar * rebar) {
	if (m_rebar)
		return 0;

	m_rebar = rebar;
	ZeroMemory(&m_rbBand, REBARBAND_SIZE);

	m_rbBand.cbSize  = REBARBAND_SIZE;

	m_rbBand.fMask   = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE |
					  RBBIM_SIZE | RBBIM_IDEALSIZE | RBBIM_ID;

	m_rbBand.fStyle     = RBBS_VARIABLEHEIGHT | RBBS_USECHEVRON;
	m_rbBand.hwndChild  = m_hwnd;
	m_rbBand.wID        = REBAR_BAR_TOOLBAR;	//ID REBAR_BAR_TOOLBAR for toolbar
	m_rbBand.cxMinChild = 0;
	m_rbBand.cyIntegral = 1;
	m_rbBand.cyMinChild = m_rbBand.cyMaxChild = GetHeight();
	m_rbBand.cxIdeal    = m_rbBand.cx         = GetWidth();

	int ret = m_rebar->AddBand(&m_rbBand, true);
	if (ret != 0)
		return -1;

	m_rebar->SetIDVisible(REBAR_BAR_TOOLBAR, true);

	return 0;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

Rebar::Rebar() :
	Window(NULL, REBARCLASSNAME)
{
	m_exStyle = WS_EX_TOOLWINDOW;
	m_style = WS_CHILD|/*WS_VISIBLE|*/WS_CLIPSIBLINGS|WS_CLIPCHILDREN|RBS_VARHEIGHT|RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN;
}

Rebar::~Rebar() {
}

int Rebar::Create(HWND hParent) {
	int ret = Window::Create(hParent);
	if (ret == -1)
		return -1;

	REBARINFO rbi;
	ZeroMemory(&rbi, sizeof(REBARINFO));
	rbi.cbSize = sizeof(REBARINFO);
	rbi.fMask  = 0;
	rbi.himl   = (HIMAGELIST)NULL;
	::SendMessage(m_hwnd, RB_SETBARINFO, 0, (LPARAM)&rbi);

	return 0;
}

int Rebar::Destroy() {
	int ret = Window::Destroy();
	m_usedIDs.clear();
	return ret;
};

int Rebar::AddBand(REBARBANDINFO * rBand, bool useID) {
	if (rBand->fMask & RBBIM_STYLE)
		;//rBand->fStyle |= RBBS_GRIPPERALWAYS;
	else
		rBand->fStyle = 0;//RBBS_GRIPPERALWAYS;
	rBand->fMask |= RBBIM_ID | RBBIM_STYLE;
	if (useID) {
		if (IsIDTaken(rBand->wID))
			return -1;

	} else {
		rBand->wID = GetNewID();
	}
	::SendMessage(m_hwnd, RB_INSERTBAND, (WPARAM)-1, (LPARAM)rBand);	//add to end of list

	return 0;
}

int Rebar::Update(int id, REBARBANDINFO * rBand) {
	int index = (int)SendMessage(m_hwnd, RB_IDTOINDEX, (WPARAM)id, 0);
	::SendMessage(m_hwnd, RB_SETBANDINFO, (WPARAM)index, (LPARAM)rBand);

	return 0;
}

int Rebar::RemoveBand(int id) {
	int index = (int)SendMessage(m_hwnd, RB_IDTOINDEX, (WPARAM)id, 0);
	if (id >= REBAR_BAR_EXTERNAL)
		ReleaseID(id);
	::SendMessage(m_hwnd, RB_DELETEBAND, (WPARAM)index, (LPARAM)0);

	return 0;
}

int Rebar::SetIDVisible(int id, bool show) {
	int index = (int)SendMessage(m_hwnd, RB_IDTOINDEX, (WPARAM)id, 0);
	if (index == -1 )
		return -1;	//error

	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;

	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(m_hwnd, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
	if (show)
		rbBand.fStyle &= (RBBS_HIDDEN ^ -1);
	else
		rbBand.fStyle |= RBBS_HIDDEN;
	::SendMessage(m_hwnd, RB_SETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);

	return 0;
}

int Rebar::GetIDVisible(int id) const {
	int index = (int)SendMessage(m_hwnd, RB_IDTOINDEX, (WPARAM)id, 0);
	if (index == -1 )
		return -1;	//error

	REBARBANDINFO rbBand;
	ZeroMemory(&rbBand, REBARBAND_SIZE);
	rbBand.cbSize  = REBARBAND_SIZE;

	rbBand.fMask = RBBIM_STYLE;
	::SendMessage(m_hwnd, RB_GETBANDINFO, (WPARAM)index, (LPARAM)&rbBand);
	return ((rbBand.fStyle & RBBS_HIDDEN) == 0);
}

int Rebar::GetNewID() {
	int idToUse = REBAR_BAR_EXTERNAL;
	int curVal = 0;
	size_t size = m_usedIDs.size();
	for(size_t i = 0; i < size; i++) {
		curVal = m_usedIDs.at(i);
		if (curVal < idToUse) {
			continue;
		} else if (curVal == idToUse) {
			idToUse++;
		} else {
			break;		//found gap
		}
	}

	m_usedIDs.push_back(idToUse);
	return idToUse;
}

int Rebar::ReleaseID(int id) {
	size_t size = m_usedIDs.size();
	for(size_t i = 0; i < size; i++) {
		if (m_usedIDs.at(i) == id) {
			m_usedIDs.erase(m_usedIDs.begin()+i);
			break;
		}
	}

	return 0;
}

bool Rebar::IsIDTaken(int id) {
	size_t size = m_usedIDs.size();
	for(size_t i = 0; i < size; i++) {
		if (m_usedIDs.at(i) == id) {
			return true;
		}
	}
	return false;
}
