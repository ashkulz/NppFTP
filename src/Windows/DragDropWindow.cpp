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
#include "DragDropWindow.h"
#include "Commands.h"

const TCHAR* DragDropWindow::DNDWINDOWCLASS = TEXT("DummyWin");

DragDropWindow::DragDropWindow(DropDataWindow* dataWindow) :
	Window(NULL, DNDWINDOWCLASS),
	m_hWindowThread(NULL),
	m_dataWindow(dataWindow)
{
	m_createWindowEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

DragDropWindow::~DragDropWindow() {
	::CloseHandle(m_createWindowEvent);
}

int DragDropWindow::Create(HWND hParent) {
	if (m_hWindowThread != NULL)
		return -1;

	m_hParent = hParent;
	m_hwnd = NULL;
	m_hWindowThread = ::CreateThread(NULL, 0, &DragDropWindow::StaticDummyWindowThread, (LPVOID)this, 0, NULL);
	if (m_hWindowThread == NULL)
		return -1;

	::WaitForSingleObject(m_createWindowEvent, INFINITE);
	if (m_hwnd == NULL)
		return -1;

	return 0;
}

int DragDropWindow::Destroy() {
	if (m_hwnd == NULL)
		return -1;
	::DestroyWindow(m_hwnd);
	::WaitForSingleObject(m_hWindowThread, 5000);
	return 0;
}

LRESULT DragDropWindow::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch(uMsg) {
		case WM_DND:{
			PerformDragDrop();
			::DestroyWindow(m_hwnd);
			::PostQuitMessage(0);
			break;}
		default: {
			result = ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
			break; }
	}
	return result;
}

int DragDropWindow::RegisterClass() {
	WNDCLASSEX DnDWindowClass;
	DnDWindowClass.cbSize = sizeof(WNDCLASSEX);
	DnDWindowClass.style = 0;
	DnDWindowClass.cbClsExtra = 0;
	DnDWindowClass.cbWndExtra = 0;
	DnDWindowClass.hIcon = NULL;
	DnDWindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	DnDWindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
	DnDWindowClass.lpszMenuName = NULL;
	DnDWindowClass.hIconSm = NULL;

	DnDWindowClass.lpfnWndProc = NULL;
	DnDWindowClass.hInstance = NULL;
	DnDWindowClass.lpszClassName = NULL;

	//register the class
	int ret = Window::RegisterClass(DNDWINDOWCLASS, DnDWindowClass);
	if (ret != 0)
		return -1;

	return 0;
}

DWORD WINAPI DragDropWindow::StaticDummyWindowThread(LPVOID param) {
	DragDropWindow * ddWin = (DragDropWindow*)param;
	int result = ddWin->DummyWindowThread();
	return result;
}

int DragDropWindow::DummyWindowThread() {
	OutMsg("[DnD] Thread start");

	HRESULT res = OleInitialize(NULL);
	if (res != S_OK && res != S_FALSE) {
		OutErr("[DnD] Error initializing OLE (Thread): %d\n", res);
		return -1;
	}

	m_hwnd = ::CreateWindowEx(
	             0,
	             m_className,
	             TEXT(""),
	             0,
	             CW_USEDEFAULT, CW_USEDEFAULT,
	             CW_USEDEFAULT, CW_USEDEFAULT,
	            // m_hParent,
	             HWND_MESSAGE,
	             NULL,
	             m_hInstance,
	             (LPVOID)this
	         );

	::SetEvent(m_createWindowEvent);
	if (!m_hwnd) {
		OutErr("[DnD] Cannot create drag and drop container window! %d\n", GetLastError());
		return -1;
	}

	MSG msg;
	DWORD retval;
	while(true) {
		int result = ::GetMessage(&msg, NULL, 0, 0);
		if (result == -1) {
			::DestroyWindow(m_hwnd);
			retval = -1;	//error
		} else if (result == 0) {
			retval = 0;
			break;	//quit
		} else {
			::DispatchMessage(&msg);
		}
	}

	::OleUninitialize();

	OutMsg("[DnD] Thread end");

	m_hWindowThread = NULL;

	return retval;
}

int DragDropWindow::PerformDragDrop() {
	int result = 0;

	OutMsg("[DnD] PerformDragDrop start");
	CDropSource * src = new CDropSource();
	CDataObject * dat = new CDataObject(m_dataWindow);

	DWORD resEffect = 0;
	HRESULT res = DoDragDrop(dat, src, DROPEFFECT_COPY, &resEffect);

	src->Release();
	dat->Release();
	if (res == S_OK || res == DRAGDROP_S_DROP)
		result = 0;
	else
		result = -1;

	m_dataWindow->OnEndDnD();

	OutMsg("[DnD] PerformDragDrop end");

	return result;
}
