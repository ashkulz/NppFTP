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

#ifndef NPPFTP_H
#define NPPFTP_H

#include "FTPWindow.h"
#include "FTPSession.h"
#include "SSLCertificates.h"

#include "Npp/PluginInterface.h"

class NppFTP {
public:
							NppFTP();
	virtual					~NppFTP();

	int						Start(NppData nppData, TCHAR * nppConfigStore, int id, FuncItem funcItem);
	int						Stop();

	int						ShowFTPWindow();
	int						FocusFTPWindow();
	int						ShowAboutDialog();

	int						OnSave(const TCHAR* path);
	int						OnActivateLocalFile(const TCHAR* path);

	static int				InitAll(HINSTANCE hInst);
private:
	int						LoadSettings();		//-1 error, 0 success, 1 unable to load
	int						SaveSettings();
	void					AttemptToAutoConnect(const TCHAR* path);

	FTPSettings*			m_ftpSettings;
	FTPSession*				m_ftpSession;
	FTPWindow*				m_ftpWindow;

	vProfile				m_profiles;
	bool					m_activeSession;

	TCHAR*					m_configStore;


	NppData					m_nppData;

	vX509					m_certificates;
};

#endif //NPPFTP_H
