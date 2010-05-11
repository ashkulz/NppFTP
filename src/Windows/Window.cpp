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
#include "Window.h"

HINSTANCE Window::_hDefaultInst = NULL;

Window::Window(HINSTANCE hInst, LPCTSTR classname) :
	//m_hInstance(hInst),
	m_hwnd(NULL),
	m_hParent(NULL),
	m_className(classname),
	m_created(false),
	m_exStyle(0),
	m_style(0)
{
	if (hInst == 0)
		m_hInstance = _hDefaultInst;
	else
		m_hInstance = hInst;
}

int Window::Create(HWND hParent) {
	if (m_created)
		return -1;

//	if (hParent == 0)
//		return -1;

	m_hParent = hParent;
	m_hwnd = ::CreateWindowEx(
	             m_exStyle,
	             m_className,
	             TEXT(""),
	             m_style,
	             CW_USEDEFAULT, CW_USEDEFAULT,
	             CW_USEDEFAULT, CW_USEDEFAULT,
	             m_hParent,
	             NULL,
	             m_hInstance,
	             (LPVOID)this
	         );

	if (m_hwnd == 0)
		return -1;

	m_created = true;

	return 0;
}

int Window::Destroy() {
	if (!m_created)
		return 0;

	BOOL res = ::DestroyWindow(m_hwnd);
	m_created = false;
	m_hwnd = NULL;

	if (res == FALSE)
		return -1;
	return 0;
}

HWND Window::GetHWND() {
	return m_hwnd;
}

int Window::GetWidth() {
	RECT rc;
	::GetClientRect(m_hwnd, &rc);
	return (rc.right - rc.left);

}

int Window::GetHeight() {
	RECT rc;
	::GetClientRect(m_hwnd, &rc);
	if (::IsWindowVisible(m_hwnd) == TRUE)
		return (rc.bottom - rc.top);
	return 0;
}

int Window::OnSize(int /*newWidth*/, int /*newHeight*/) {
	return 1;
}

int Window::Resize(int width, int height) {
	BOOL res = SetWindowPos(m_hwnd, 0, 0, 0, width, height, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	return (res)?0:-1;
}

int Window::Move(int x1, int y1, int width, int height) {
	BOOL res = SetWindowPos(m_hwnd, 0, x1, y1, width, height, SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	return (res)?0:-1;
}

int Window::Show(bool show) {
	BOOL res = ::ShowWindow(m_hwnd, show?SW_SHOW:SW_HIDE);
	return (res)?0:-1;
}

bool Window::IsVisible() {
	BOOL shown = IsWindowVisible(m_hwnd);
	return (shown != FALSE);
}

int Window::Redraw() {
	BOOL res = RedrawWindow(m_hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW);
	return (res==TRUE?0:-1);
}

LRESULT Window::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;

	switch(uMsg) {
		case WM_SIZE: {
			RECT clientRect;
			GetClientRect(m_hwnd, &clientRect);
			result = OnSize(clientRect.right, clientRect.bottom);
			break; }
		case WM_PAINT: {
			result = ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
			break; }
		case WM_QUIT: {
			Destroy();
			return TRUE;
			break; }
		case WM_DESTROY: {
			PostQuitMessage(0);
			return TRUE;
			break; }
		default:
			result = ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}

	return result;
}

int Window::SetDefaultInstance(HINSTANCE hInst) {
	Window::_hDefaultInst = hInst;
	return 0;
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	Window * window = NULL;

	switch (uMsg) {
		case WM_NCCREATE:
			window = (Window*)(((LPCREATESTRUCT)lParam)->lpCreateParams);
			//window.setHWND(hwnd);
			::SetWindowLongPtr(hwnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(window));

			return TRUE;
		default :
			window = (Window*)::GetWindowLongPtr(hwnd, GWL_USERDATA);
			if (!window)
				return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
			return window->MessageProc(uMsg, wParam, lParam);
	}
}
/*
int Window::setClassname(LPCTSTR classname) {
	m_className = classname;
	return 0;
}

int Window::setHWND(HWND hwnd) {
	m_hwnd = hwnd;
	return 0;
}


*/
int Window::RegisterClass(LPCTSTR classname, WNDCLASSEX wclass) {
	if (_hDefaultInst == NULL)
		return -1;

	wclass.lpfnWndProc = Window::WindowProc;
	wclass.hInstance = _hDefaultInst;
	wclass.lpszClassName = classname;

	if (!::RegisterClassEx(&wclass))
		return -1;

	return 0;
}
