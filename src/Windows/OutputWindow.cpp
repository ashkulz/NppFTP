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
#include "Commands.h"
#include <time.h>
#include <windowsx.h>
#include "Npp/Scintilla.h"

Output* _MainOutput = NULL;

const TCHAR * OutputWindow::OUTWINDOWCLASS = TEXT("NPPFTPOUTPUT");

const int WM_OUT_ADDLINE_SYST = WM_USER + 1;
const int WM_OUT_ADDLINE_CLNT = WM_USER + 2;
const int WM_OUT_ADDLINE_ERR = WM_USER + 3;

const int STYLE_CLIENT = STYLE_LASTPREDEFINED + 1;
const int STYLE_SYSTEM = STYLE_LASTPREDEFINED + 2;
const int STYLE_ERROR  = STYLE_LASTPREDEFINED + 3;

const int STYLE_MARGIN_CLIENT = STYLE_LASTPREDEFINED + 4;
const int STYLE_MARGIN_SYSTEM = STYLE_LASTPREDEFINED + 5;
const int STYLE_MARGIN_ERROR  = STYLE_LASTPREDEFINED + 6;

const int STYLE_MARGINOFFSET = 3;

OutputWindow::OutputWindow() :
	DockableWindow(OUTWINDOWCLASS),
	m_winThread(0),
	m_hContextMenu(NULL),
	m_hScintilla(NULL),
	m_maxLines(500),
	m_hNotify(NULL)
{
	m_style = 0;
	m_exStyle = 0;

	_MainOutput = this;
}

OutputWindow::~OutputWindow() {
}

int OutputWindow::Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand, HWND hNotify) {
	SetTitle(TEXT("NppFTP"));
	SetInfo(TEXT("Output"));
	SetLocation(DWS_DF_CONT_BOTTOM);
	HICON icon = ::LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON_MESSAGES));
	SetIcon(icon);

	int res = DockableWindow::Create(hParent, hNpp, MenuID, MenuCommand);
	if (res == -1)
		return -1;

	m_hNotify = hNotify;

	m_hScintilla = ::CreateWindowEx(WS_EX_CLIENTEDGE,
									TEXT("Scintilla"), TEXT(""),
									WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
									0, 0, 10, 10,
									m_hwnd, NULL, m_hInstance, NULL);
	if (m_hScintilla == NULL) {
		DockableWindow::Destroy();
		return -1;
	}

	m_winThread = ::GetCurrentThreadId();	//GetWindowThreadProcessId(m_histControl.m_hWnd, NULL);
	m_hContextMenu = ::CreatePopupMenu();
	::AppendMenu(m_hContextMenu,MF_STRING,IDM_POPUP_OUTPUT_COPY,TEXT("&Copy"));
	::AppendMenu(m_hContextMenu,MF_STRING,IDM_POPUP_OUTPUT_CLEAR,TEXT("Clea&r contents"));

	SetScintillaParameters();

	::SendMessage(m_hwnd, WM_SIZE, 0, 0);
	::ShowWindow(m_hScintilla, SW_SHOW);

	return 0;
}

int OutputWindow::Destroy() {
	DestroyWindow(m_hScintilla);
	return DockableWindow::Destroy();
}

int OutputWindow::Show(bool show) {
	int res = DockableWindow::Show(show);

	if (m_hNotify != NULL)
		::SendMessage(m_hNotify, WM_OUTPUTSHOWN, (WPARAM)show?TRUE:FALSE, 0);

	return res;
}

int OutputWindow::OnSize(int newWidth, int newHeight) {
	BOOL res = SetWindowPos(m_hScintilla, NULL, 0, 0, newWidth, newHeight, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
	if (res == FALSE)
		return -1;

	return 0;
}

int OutputWindow::RegisterClass() {
	WNDCLASSEX OUTWindowClass{};
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
	LRESULT result = 0;

	switch(uMsg) {
		case WM_SETFOCUS: {
			//Why restore focus here? This window should never be able to get focus in the first place
			HWND hPrev = (HWND)wParam;
			if (hPrev != NULL)
				::SetFocus(hPrev);
			break; }
		case WM_ERASEBKGND: {
			HDC hDC = (HDC) wParam;
			RECT rectClient;
			GetClientRect(m_hwnd, &rectClient);
			FillRect(hDC, &rectClient, ::GetSysColorBrush(COLOR_3DFACE));
			result = TRUE;
			break; }
		case WM_OUT_ADDLINE_SYST: {
			TCHAR * msg = (TCHAR*)lParam;
			time_t time = (time_t)wParam;
			AddMessage(msg, Output_System, time);
			delete [] msg;
			result = TRUE;
			break; }
		case WM_OUT_ADDLINE_CLNT: {
			TCHAR * msg = (TCHAR*)lParam;
			time_t time = (time_t)wParam;
			AddMessage(msg, Output_Client, time);
			delete [] msg;
			result = TRUE;
			break; }
		case WM_OUT_ADDLINE_ERR: {
			TCHAR * msg = (TCHAR*)lParam;
			time_t time = (time_t)wParam;
			AddMessage(msg, Output_Error, time);
			delete [] msg;
			result = TRUE;
			break; }
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDM_POPUP_OUTPUT_COPY: {
					::SendMessage(m_hScintilla, SCI_COPYALLOWLINE, 0, 0);
					result = TRUE;
					break; }
				case IDM_POPUP_OUTPUT_CLEAR: {
					::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)false, 0);
					::SendMessage(m_hScintilla, SCI_CLEARALL, 0, 0);
					::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)true, 0);
					result = TRUE;
					break; }
				default: {
					result = FALSE;
					break; }
			}
			break; }
		case WM_CONTEXTMENU: {
			POINT menuPos{};
			menuPos.x = GET_X_LPARAM(lParam);
			menuPos.y = GET_Y_LPARAM(lParam);
			bool fromKeyboard = (menuPos.x == -1 && menuPos.y == -1);
			if (fromKeyboard) {	//keyboard activation
				RECT winRect;
				::GetWindowRect(m_hScintilla, &winRect);
				menuPos.x = winRect.left;
				menuPos.y = winRect.top;
				//use the top left location as the output window has no caret location
			}
			::TrackPopupMenu(m_hContextMenu, TPM_LEFTALIGN, menuPos.x, menuPos.y, 0, m_hwnd, NULL);
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
	if (m_hwnd == NULL || m_hScintilla == NULL)
		return -1;

	if (!message)
		return -1;

	TCHAR * msgBuffer = new TCHAR[1024];
	memset(msgBuffer, 0, (1024*sizeof(TCHAR)));

	/*int ret =*/SU::TSprintfV(msgBuffer, (1024-1), message, vaList);
    //if (ret == -1)	//-1 indicates truncation, not necessarily failure
	//	return -1;

	//Replace newline characters, otherwise screw up output
	TCHAR * curChar = msgBuffer;
	while(*curChar) {
		if (*curChar == TEXT('\r') || *curChar == TEXT('\n'))
			*curChar = ' ';
		curChar++;
	}

	time_t t = time(NULL);

	UINT msg = WM_OUT_ADDLINE_SYST;
	switch(type) {
		case Output_System:
			msg = WM_OUT_ADDLINE_SYST;
			break;
		case Output_Client:
			msg = WM_OUT_ADDLINE_CLNT;
			break;
		case Output_Error:
			msg = WM_OUT_ADDLINE_ERR;
			break;
		default:
			break;
	}

	DWORD curThread = GetCurrentThreadId();
	if (m_winThread == curThread) {
		::PostMessage(m_hwnd, msg, (WPARAM)t, (LPARAM)msgBuffer);
		return 0;
	} else {
		::PostMessage(m_hwnd, msg, (WPARAM)t, (LPARAM)msgBuffer);
	}


    return 0;
}

int OutputWindow::ScrollLastLine() {
	int nrLines = (int)::SendMessage(m_hScintilla, SCI_GETLINECOUNT, 0, 0);
	::SendMessage(m_hScintilla, SCI_LINESCROLL, 0, (LPARAM)nrLines);	//scrolling the amount of lines available ensures the last one becomes visible

	return 0;
}

int OutputWindow::SetScintillaParameters() {
	if (m_hScintilla == NULL)
		return -1;

	::SendMessage(m_hScintilla, SCI_USEPOPUP, (WPARAM)false, 0);

	::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)true, 0);
	::SendMessage(m_hScintilla, SCI_SETSCROLLWIDTH, (WPARAM)10, 0);
	::SendMessage(m_hScintilla, SCI_SETSCROLLWIDTHTRACKING, (WPARAM)true, 0);

	::SendMessage(m_hScintilla, SCI_SETUNDOCOLLECTION, (WPARAM)false, 0);
	::SendMessage(m_hScintilla, SCI_SETCODEPAGE, SC_CP_UTF8, 0);

	::SendMessage(m_hScintilla, SCI_STARTSTYLING, (WPARAM)0, (LPARAM)0xff);

	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_DEFAULT, RGB(0, 180, 180));
	::SendMessage(m_hScintilla, SCI_STYLECLEARALL, 0, 0);

	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_CLIENT, RGB(0, 180, 0));
	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_SYSTEM, RGB(0, 0, 180));
	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_ERROR, RGB(255, 0, 0));

	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_MARGIN_CLIENT, RGB(0, 180, 0));
	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_MARGIN_SYSTEM, RGB(0, 0, 180));
	::SendMessage(m_hScintilla, SCI_STYLESETFORE, STYLE_MARGIN_ERROR, RGB(255, 0, 0));
	::SendMessage(m_hScintilla, SCI_STYLESETBACK, STYLE_MARGIN_CLIENT, RGB(245, 245, 245));
	::SendMessage(m_hScintilla, SCI_STYLESETBACK, STYLE_MARGIN_SYSTEM, RGB(245, 245, 245));
	::SendMessage(m_hScintilla, SCI_STYLESETBACK, STYLE_MARGIN_ERROR, RGB(245, 245, 245));

	::SendMessage(m_hScintilla, SCI_STYLESETBACK, STYLE_LINENUMBER, RGB(245, 245, 245));

	::SendMessage(m_hScintilla, SCI_SETMARGINTYPEN, (WPARAM)0, (LPARAM)SC_MARGIN_TEXT);
	int width = ::SendMessage(m_hScintilla, SCI_TEXTWIDTH, (WPARAM)STYLE_SYSTEM, (LPARAM)"88:88:88  ");
	::SendMessage(m_hScintilla, SCI_SETMARGINWIDTHN, (WPARAM)0, (LPARAM)width);
	::SendMessage(m_hScintilla, SCI_SETMARGINWIDTHN, (WPARAM)1, (LPARAM)0);
	::SendMessage(m_hScintilla, SCI_SETMARGINWIDTHN, (WPARAM)2, (LPARAM)0);
	return 0;
}

int OutputWindow::AddMessage(const TCHAR * message, Output_Type type, time_t ttime) {
	int style = STYLE_DEFAULT;

	switch(type) {
		case Output_System:
			style = STYLE_SYSTEM;
			break;
		case Output_Client:
			style = STYLE_CLIENT;
			break;
		case Output_Error:
			style = STYLE_ERROR;
			break;
		default:
			style = STYLE_DEFAULT;
			break;
	}

	char * utf8Buffer = SU::TCharToUtf8(message);
	char * timeString = new char[11];

	struct tm * systime = localtime(&ttime);
	if(strftime(timeString, 10, "%H:%M:%S", systime) == 0) {
		timeString[0] = 0;
	}

	int lastPos = ::SendMessage(m_hScintilla, SCI_GETTEXTLENGTH, 0, 0);
	int curPos = ::SendMessage(m_hScintilla, SCI_GETCURRENTPOS, 0, 0);

	::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)false, 0);
	::SendMessage(m_hScintilla, SCI_APPENDTEXT, (WPARAM)strlen(utf8Buffer), (LPARAM)utf8Buffer);
	::SendMessage(m_hScintilla, SCI_APPENDTEXT, (WPARAM)2, (LPARAM)"\r\n");
	::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)true, 0);

	::SendMessage(m_hScintilla, SCI_STARTSTYLING, (WPARAM)lastPos, (LPARAM)0xff);
	::SendMessage(m_hScintilla, SCI_SETSTYLING, strlen(utf8Buffer)+2, style);

	int lineCount = ::SendMessage(m_hScintilla, SCI_GETLINECOUNT, 0, 0);
	lineCount--;	//ignore final newline

	::SendMessage(m_hScintilla, SCI_MARGINSETTEXT, (WPARAM)lineCount-1, (LPARAM)timeString);
	::SendMessage(m_hScintilla, SCI_MARGINSETSTYLE, (WPARAM)lineCount-1, (LPARAM)style+STYLE_MARGINOFFSET);

	if (lineCount > m_maxLines) {
		::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)false, 0);
		int endPos = ::SendMessage(m_hScintilla, SCI_POSITIONFROMLINE, (WPARAM)1, 0);
		char buffer[13]{};
		::SendMessage(m_hScintilla, SCI_MARGINGETTEXT, (WPARAM)1, (LPARAM)buffer);

		::SendMessage(m_hScintilla, SCI_SETTARGETSTART, 0, 0);
		::SendMessage(m_hScintilla, SCI_SETTARGETEND, endPos, 0);
		::SendMessage(m_hScintilla, SCI_REPLACETARGET, 0, (LPARAM)"");

		::SendMessage(m_hScintilla, SCI_MARGINSETTEXT, (WPARAM)0, (LPARAM)buffer);

		::SendMessage(m_hScintilla, SCI_SETREADONLY, (WPARAM)true, 0);
	}

	delete [] timeString;

	if (curPos == lastPos || curPos == lastPos - 2) {	//if at last line, scroll the caret
		lastPos = ::SendMessage(m_hScintilla, SCI_GETTEXTLENGTH, 0, 0);
		::SendMessage(m_hScintilla, SCI_SETANCHOR, lastPos, 0);
		::SendMessage(m_hScintilla, SCI_SETCURRENTPOS, lastPos, 0);
		//::SendMessage(m_hScintilla, SCI_SCROLLCARET, 0, 0);
	}

	//if the last line is in view, scroll the view
	int visible = ::SendMessage(m_hScintilla, SCI_LINESONSCREEN, 0, 0);
	int firstvisible = ::SendMessage(m_hScintilla, SCI_GETFIRSTVISIBLELINE, 0, 0);
	int lastline = lineCount-1;	//ignore very last line
	if ((firstvisible+visible+1) >= lastline) {
		::SendMessage(m_hScintilla, SCI_LINESCROLL, 0, (LPARAM)1);	//scroll down one line, as one line was added
	}

	SU::FreeChar(utf8Buffer);

	return 0;
}
