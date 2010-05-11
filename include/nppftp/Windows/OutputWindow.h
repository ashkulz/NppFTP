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
#include "uh_ctrl.h"

class OutputWindow : public DockableWindow, Output {
public:

public:
							OutputWindow();
	virtual					~OutputWindow();

	virtual int				Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand);
	virtual int				Destroy();

	virtual int				OnSize(int newWidth, int newHeight) ;

	static int				RegisterClass();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

							//message: %T is tchar (%s or %S), %s is char, %S is wchar_t
	virtual int				OutVA(Output_Type type, const TCHAR * message, va_list vaList);
protected:
	virtual int				AddMessage(const TCHAR * message, COLORREF color);

	CUH_Control				m_histControl;

	static const TCHAR * OUTWINDOWCLASS;
};

#endif //OUTPUTWINDOW_H
