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

#ifndef OUTPUTWINDOW_H
#define OUTPUTWINDOW_H

#include "DockableWindow.h"

class OutputWindow : public DockableWindow, Output {
public:

public:
							OutputWindow();
	virtual					~OutputWindow();

	virtual int				Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand, HWND hNotify);
	virtual int				Destroy();

	virtual int				Show(bool show);
	virtual int				OnSize(int newWidth, int newHeight);

	static int				RegisterClass();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

							//message: %T is tchar (%s or %S), %s is char, %S is wchar_t
	virtual int				OutVA(Output_Type type, const TCHAR * message, va_list vaList);

	virtual int				ScrollLastLine();
protected:
	virtual int				SetScintillaParameters();
	virtual int				AddMessage(const TCHAR * message, Output_Type type, time_t time);

	DWORD					m_winThread;
	HMENU					m_hContextMenu;
	HWND					m_hScintilla;
	int						m_maxLines;
	HWND					m_hNotify;

	static const TCHAR * OUTWINDOWCLASS;
};

#endif //OUTPUTWINDOW_H
