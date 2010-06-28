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

#ifndef DIALOG_H
#define DIALOG_H

#include "Window.h"

const int EN_USERCHANGE = WM_USER + 600;
const int EN_KEYRETURN = WM_USER + 601;

class Dialog : public Window {
public:
							Dialog(int dialogResource = 0);
	virtual					~Dialog();

	virtual int				Create(HWND hParent, bool isModal, const TCHAR * title);	//if modal, returns 99 on close

	static INT_PTR CALLBACK	DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	virtual INT_PTR			DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR			OnInitDialog();	//DialogProc filters this one out, therefore calback

	virtual INT_PTR			OnCommand(int ctrlId, int notifCode, HWND idHwnd);
	virtual INT_PTR			OnNotify(NMHDR * pnmh);

	static LRESULT			EditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static WNDPROC			_EditDefaultProc;

	int						m_dialogResource;

	TCHAR*					m_title;
};

#endif //DIALOG_H
