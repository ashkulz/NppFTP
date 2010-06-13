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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "Dialog.h"
#include "FTPCache.h"

class SettingsDialog : public Dialog {
public:
							SettingsDialog();
	virtual					~SettingsDialog();

	virtual int				Create(HWND hParent, FTPCache * globalCache);
protected:
	virtual INT_PTR			DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR			OnInitDialog();

	virtual INT_PTR			OnCommand(int ctrlId, int notifCode, HWND idHwnd);
	virtual INT_PTR			OnNotify(NMHDR * pnmh);

	int						SaveGlobalPath();
	int						SaveMasterPassword();

	FTPCache*				m_globalCache;
};

#endif //SETTINGSDIALOG_H
