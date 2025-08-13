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

#ifndef DOCKABLEWINDOW_H
#define DOCKABLEWINDOW_H

#include "Window.h"

#include "Npp/Docking.h"

class DockableWindow : public Window {
public:
							DockableWindow(LPCTSTR classname);
	virtual					~DockableWindow();

	virtual int				Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand);
	virtual int				Destroy();

	virtual int				Show(bool show);
	virtual bool			IsVisible();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	//only possible before registering
	virtual int				SetIcon(HICON icon);
	virtual int				SetLocation(int location);

	//Also allowed after registering
	virtual int				SetTitle(const TCHAR* title);
	virtual int				SetInfo(const TCHAR* info);
protected:
	using Window::Create; //avoid compiler warning about hidden method

	virtual int				RegisterDockableDialog();
	virtual int				UpdateDockInfo();

	int						m_titleSize;
	TCHAR*					m_title;
	int						m_infoSize;
	TCHAR*					m_info;
	HICON					m_icon;
	int						m_docklocation;

	TCHAR*					m_moduleName;

	bool					m_registered;
	bool					m_displayed;

	HWND					m_hNpp;
	int						m_menuID;
	int						m_menuCmd;

	tTbData					m_tbd;
private:
	static const int		_titleSize;
	static const int		_infoSize;
};

#endif //DOCKABLEWINDOW_H
