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

#ifndef MESSAGEDIALOG_H
#define MESSAGEDIALOG_H

#include "Dialog.h"

class MessageDialog : public Dialog {
public:
							MessageDialog();
	virtual					~MessageDialog();

							//Returns: 1 on Yes, 2 on No
	virtual int				Create(HWND hParent, const TCHAR * title, const TCHAR * message);
protected:
	virtual INT_PTR			DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR			OnInitDialog();	//DialogProc filters this one out, therefore calback

	TCHAR*					m_message;

	HWND					m_messageCtrl;
};

#endif //MESSAGEDIALOG_H
