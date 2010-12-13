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

	if (m_hwnd == 0) {
		int err = GetLastError();
		OutErr("CreateWindow failed: %d", err);
		return -1;
	}

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
	//commented check: only usefull for rebars
	//if (::IsWindowVisible(m_hwnd) == TRUE)
		return (rc.bottom - rc.top);
	//return 0;
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

int Window::Focus() {
	HWND hPrev = ::SetFocus(m_hwnd);
	return (hPrev!=NULL?0:-1);
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
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

			return TRUE;
		default :
			window = (Window*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
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

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////


WindowSplitter::WindowSplitter(Window * windowParent, Window * window1, Window * window2) :
	m_windowParent(windowParent),
	m_window1(window1),
	m_window2(window2),
	m_splitRatio(0.5),
	m_dragging(false),
	m_splitterSize(5),
	m_dragStartOffset(0),
	m_splitterMargin(20)
{
	::ZeroMemory(&m_splitterRect, sizeof(m_splitterRect));
	::ZeroMemory(&m_windowRect, sizeof(m_windowRect));

	/*
	HCURSOR cursorDefault = LoadCursor(NULL, IDC_ARROW);
	HCURSOR cursorSplitterHorizontal = LoadCursor(NULL, IDC_SIZENS);
	HCURSOR cursorSplitterVertical = LoadCursor(NULL, IDC_SIZEWE);
	*/
}

WindowSplitter::~WindowSplitter() {
}

int WindowSplitter::SetRatio(double ratio) {
	if (ratio >= 0.0 && ratio <= 1.0) {
		m_splitRatio = ratio;
		m_splitterRect.top = m_windowRect.top + (m_windowRect.bottom*m_splitRatio);
		m_splitterRect.bottom = m_splitterRect.top + m_splitterSize;
		PerformLayout();
	}
	return 0;
}

double WindowSplitter::GetRatio() {
	return m_splitRatio;
}

int WindowSplitter::OnSize(int x, int y, int width, int height) {
	m_windowRect.left = x;
	m_windowRect.right = width;
	m_windowRect.top = y;
	m_windowRect.bottom = height;

	m_splitterRect.left = x;
	m_splitterRect.right = width+x;

	int splitterTop = y + (height*m_splitRatio);
	if ((splitterTop - y) > (height-m_splitterMargin))
		splitterTop = y + height - m_splitterMargin;
	if ((splitterTop - y) < m_splitterMargin)
		splitterTop = y + m_splitterMargin;

	m_splitterRect.top = splitterTop;
	m_splitterRect.bottom = m_splitterRect.top + m_splitterSize;

	//keep the ratio intact
	//m_splitRatio = (double)(splitterTop-y)/(double)height;

	return PerformLayout();
}

int WindowSplitter::OnCaptureChanged(HWND /*hNewCapture*/) {
	m_dragging = false;
	return 0;
}

bool WindowSplitter::OnSetCursor() {
	DWORD dPos = GetMessagePos();			//get current mouse pos
	POINTS pts = MAKEPOINTS(dPos);
	POINT pos = {pts.x, pts.y};
	ScreenToClient(m_windowParent->GetHWND(), &pos);

	if (PointOnSplitter(pos)) {
		::SetCursor(::LoadCursor(NULL, IDC_SIZENS));
		return true;
	}

	return false;
}

bool WindowSplitter::OnButtonDown() {
	//m_dragging = true;
	DWORD dPos = GetMessagePos();			//get current mouse pos
	POINTS pts = MAKEPOINTS(dPos);
	POINT pos = {pts.x, pts.y};
	ScreenToClient(m_windowParent->GetHWND(), &pos);

	if (PointOnSplitter(pos)) {
		m_dragging = true;
		m_dragStartOffset = pos.y - m_splitterRect.top;
		::SetCapture(m_windowParent->GetHWND());
		return true;
	}

	return false;
}

bool WindowSplitter::OnMouseMove() {
	if (!m_dragging)
		return false;

	DWORD dPos = GetMessagePos();			//get current mouse pos
	POINTS pts = MAKEPOINTS(dPos);
	POINT pos = {pts.x, pts.y};
	ScreenToClient(m_windowParent->GetHWND(), &pos);

	int newSplitterTop = pos.y-m_dragStartOffset;

	if (newSplitterTop < (m_windowRect.top + m_splitterMargin) || (newSplitterTop + m_splitterSize) > (m_windowRect.bottom + m_windowRect.top - m_splitterMargin))
		return true;

	m_splitRatio = (double)(newSplitterTop - m_windowRect.top)/(double)m_windowRect.bottom;

	m_splitterRect.top = newSplitterTop;
	m_splitterRect.bottom = m_splitterRect.top + m_splitterSize;

	PerformLayout();

	return false;
}

bool WindowSplitter::OnButtonUp() {
	if (m_dragging) {
		m_dragging = false;
		::ReleaseCapture();
		return true;
	}

	return false;
}

bool WindowSplitter::PointOnSplitter(POINT pt) {
	if (pt.x < m_splitterRect.left || pt.x > m_splitterRect.right)
		return false;

	if (pt.y < m_splitterRect.top || pt.y > m_splitterRect.bottom)
		return false;

	return true;
}

int WindowSplitter::PerformLayout() {
	int win1height = m_splitterRect.top - m_windowRect.top;
	int win2height = (m_windowRect.bottom - (m_splitterRect.bottom-m_windowRect.top));
	int win1top = m_windowRect.top;
	int win2top = m_splitterRect.bottom;

	m_window1->Move(m_windowRect.left, win1top, m_windowRect.right, win1height);
	m_window2->Move(m_windowRect.left, win2top, m_windowRect.right, win2height);

	return 0;
}
