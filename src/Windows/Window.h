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

#ifndef WINDOW_H
#define WINDOW_H

//Based on Notepad++'s Window class

class Window {
public:
							Window(HINSTANCE hInst, LPCTSTR classname);
	virtual 				~Window() {};

	virtual int				Create(HWND hParent);
	virtual int				Destroy();

	virtual HWND			GetHWND();

	virtual int				GetWidth();
	virtual int				GetHeight();

	virtual int				OnSize(int newWidth, int newHeight);

	virtual int				Resize(int width, int height);
	virtual int				Move(int x1, int y1, int width, int height);

	virtual int				Show(bool show);
	virtual bool			IsVisible();
	virtual int				Redraw();
	virtual int				Focus();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	static int				SetDefaultInstance(HINSTANCE hInst);
	static LRESULT CALLBACK	WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	//virtual int				setHWND(HWND hwnd);
	//virtual int				setClassname(LPCTSTR classname);

	static int				RegisterClass(LPCTSTR classname, WNDCLASSEX wclass);

	HINSTANCE				m_hInstance;
	HWND					m_hwnd;
	HWND					m_hParent;
	LPCTSTR					m_className;
	bool					m_created;
	DWORD					m_exStyle;
	DWORD					m_style;

	static HINSTANCE		_hDefaultInst;
};

class WindowSplitter {
public:
							WindowSplitter(Window * windowParent, Window * window1, Window * window2);
							~WindowSplitter();

	int						SetRatio(double ratio);
	double					GetRatio();

	int						OnSize(int x, int y, int width, int height);
	int						OnCaptureChanged(HWND hNewCapture);

	bool					OnSetCursor();
	bool					OnButtonDown();
	bool					OnMouseMove();
	bool					OnButtonUp();
private:
	bool					PointOnSplitter(POINT pt);
	int						PerformLayout();

	Window*					m_windowParent;
	Window*					m_window1;
	Window*					m_window2;

	RECT					m_windowRect;
	RECT					m_splitterRect;
	double					m_splitRatio;
	bool					m_dragging;
	int						m_splitterSize;
	int						m_dragStartOffset;
	int						m_splitterMargin;
};

#endif //WINDOW_H
