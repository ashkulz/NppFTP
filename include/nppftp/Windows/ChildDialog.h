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

#ifndef CHILDDIALOG_H
#define CHILDDIALOG_H

#include "Dialog.h"

class ChildDialog : public Dialog {
public:
							ChildDialog(int dialogResource = 0);
	virtual					~ChildDialog();

	virtual int				Create(HWND hParent, HWND hOwner, const TCHAR * title);	//if modal, returns 99 on close
protected:
	virtual INT_PTR			DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND					m_hOwner;

};

#endif //CHILDDIALOG_H
