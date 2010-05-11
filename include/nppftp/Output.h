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

#ifndef OUTPUT_H
#define OUTPUT_H

enum Output_Type { Output_System, Output_Client, Output_Error };

class Output {
public:
							Output() {};
	virtual					~Output() {};

	virtual int				OutVA(Output_Type type, const TCHAR * message, va_list vaList) = 0;
};

extern Output* _MainOutput;
extern HWND _MainOutputWindow;

int OutputMsg(const TCHAR * msg, ...);
int OutputClnt(const TCHAR * msg, ...);
int OutputErr(const TCHAR * msg, ...);
int MessageBoxOutput(const TCHAR * msg);

#ifdef UNICODE
#define OutMsg(x,...) OutputMsg(L##x , ##__VA_ARGS__)
#define OutClnt(x,...) OutputClnt(L##x , ##__VA_ARGS__)
#define OutErr(x,...) OutputErr(L##x , ##__VA_ARGS__)
#else
#define OutMsg(x,...) OutputMsg(x , ##__VA_ARGS__)
#define OutClnt(x,...) OutputClnt(x , ##__VA_ARGS__)
#define OutErr(x,...) OutputErr(x , ##__VA_ARGS__)
#endif

#endif //OUTPUT_H
