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

#ifndef DRAGDROPWINDOW_H
#define DRAGDROPWINDOW_H

#include "Window.h"
#include "DragDropSupport.h"

class DragDropWindow : public Window {
public:
							DragDropWindow(DropDataWindow* dataWindow);
	virtual 				~DragDropWindow();

	virtual int				Create(HWND hParent);
	virtual int				Destroy();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	static int				RegisterClass();
protected:
	static DWORD WINAPI		StaticDummyWindowThread(LPVOID param);
	virtual int				DummyWindowThread();

	virtual int				PerformDragDrop();

	HANDLE					m_createWindowEvent;
	HANDLE					m_hWindowThread;
	DropDataWindow*			m_dataWindow;
	static const TCHAR*		DNDWINDOWCLASS;
};

#endif //DRAGDROPWINDOW_H
