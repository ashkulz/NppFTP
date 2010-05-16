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
#include "OutputWindow.h"

#include "resource.h"

Output* _MainOutput = NULL;

const TCHAR * OutputWindow::OUTWINDOWCLASS = TEXT("NPPFTPOUTPUT");

OutputWindow::OutputWindow() :
	DockableWindow(OUTWINDOWCLASS),
	m_histControl()
{
	m_style = 0;
	m_exStyle = WS_EX_NOACTIVATE;

	_MainOutput = this;
}

OutputWindow::~OutputWindow() {
}

int OutputWindow::Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand) {
	SetTitle(TEXT("NppFTP"));
	SetInfo(TEXT("Output"));
	SetLocation(DWS_DF_CONT_BOTTOM);
	HICON icon = ::LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON_MESSAGES));
	SetIcon(icon);

	int res = DockableWindow::Create(hParent, hNpp, MenuID, MenuCommand);
	if (res == -1)
		return -1;

	RECT rect = {0, 0, 10, 10};
	res = m_histControl.CreateHistoryWindow(m_hwnd, WS_CHILD, rect);
	if (res != UH_SUCCESS) {
		DockableWindow::Destroy();
		return -1;
	}

	m_histControl.SetHistoryLength(250);

	::SetWindowLongPtr(m_histControl.m_hWnd, GWL_EXSTYLE, (LONG_PTR)WS_EX_CLIENTEDGE);

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	BOOL ret = ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	if (ret != FALSE) {
		HFONT controlFont = ::CreateFontIndirect(&ncm.lfMessageFont);
		if (controlFont != NULL) {
			m_histControl.SetFont(controlFont);	//m_histControl takes ownership of font object
		}
	}

	::ShowWindow(m_histControl.m_hWnd, SW_SHOW);

	Show(true);	//kinda hacky, but otherwise input might be lost, TODO?
	Show(false);

	return 0;
}

int OutputWindow::Destroy() {
	DestroyWindow(m_histControl.m_hWnd);
	return DockableWindow::Destroy();
}

int OutputWindow::OnSize(int newWidth, int newHeight) {
	BOOL res = SetWindowPos(m_histControl.m_hWnd, 0, 0, 0, newWidth, newHeight, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	if (res == FALSE)
		return -1;

	return 0;
}

int OutputWindow::RegisterClass() {
	WNDCLASSEX OUTWindowClass;
	OUTWindowClass.cbSize = sizeof(WNDCLASSEX);
	OUTWindowClass.style = CS_DBLCLKS;//|CS_NOCLOSE;
	OUTWindowClass.cbClsExtra = 0;
	OUTWindowClass.cbWndExtra = 0;
	OUTWindowClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	OUTWindowClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	OUTWindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
	OUTWindowClass.lpszMenuName = NULL;
	OUTWindowClass.hIconSm = NULL;

	OUTWindowClass.lpfnWndProc = NULL;
	OUTWindowClass.hInstance = NULL;
	OUTWindowClass.lpszClassName = NULL;

	//register the class
	int ret = Window::RegisterClass(OUTWINDOWCLASS, OUTWindowClass);
	if (ret != 0)
		return -1;

	return 0;
}

LRESULT OutputWindow::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	bool doDefaultProc = false;
	LRESULT result;

	switch(uMsg) {
		case WM_ERASEBKGND: {
			HDC hDC = (HDC) wParam;
			RECT rectClient;
			GetClientRect(m_hwnd, &rectClient);
			FillRect(hDC, &rectClient, ::GetSysColorBrush(COLOR_3DFACE));
			result = TRUE;
			break; }
		default:
			doDefaultProc = true;
			break;
	}

	if (doDefaultProc)
		result = DockableWindow::MessageProc(uMsg, wParam, lParam);

	return result;
}

int OutputWindow::OutVA(Output_Type type, const TCHAR * message, va_list vaList) {
	static TCHAR msgBuffer[1024];
	if (m_hwnd == NULL || m_histControl.m_hWnd == NULL)
		return -1;

	if (!message)
		return -1;

	COLORREF color = 0xFFFFFFFF;

	switch(type) {
		case Output_System:
			color = RGB(0, 0, 180);
			break;
		case Output_Client:
			color = RGB(0, 180, 0);
			break;
		case Output_Error:
			color = RGB(255, 0, 0);
			break;
		default:
			color = 0xFFFFFFFF;
			break;
	}

	msgBuffer[0] = 0;

	/*int ret =*/SU::TSprintfV(msgBuffer, 1024, message, vaList);
    //if (ret == -1)	//-1 indicates truncation, not necessarily failure
	//	return -1;

	AddMessage(msgBuffer, color);

    return 0;
}

int OutputWindow::AddMessage(const TCHAR * message, COLORREF color) {
	int res = m_histControl.AddLine(message, color, 0xFFFFFFFF, FALSE);
	if (res != UH_SUCCESS)
		return -1;

	return 0;
}
