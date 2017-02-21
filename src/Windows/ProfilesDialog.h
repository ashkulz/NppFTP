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

#ifndef PROFILESDIALOG_H
#define PROFILESDIALOG_H

#include "Dialog.h"
#include "ChildDialog.h"
#include "FTPProfile.h"

class FTPWindow;

class ProfilesDialog : public Dialog {
public:
							ProfilesDialog();
	virtual					~ProfilesDialog();

	virtual int				Create(HWND hParent, FTPWindow * ftpWindow, vProfile * profileVect, FTPCache * globalCache);
protected:
	virtual INT_PTR			DlgMsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR			OnInitDialog();	//DialogProc filters this one out, therefore calback

	INT_PTR					OnCommand(int ctrlId, int notifCode, HWND idHwnd);
	INT_PTR					OnNotify(NMHDR * pnmh);

	int						Clear();
	int						LoadProfiles();
	int						OnSelectProfile(FTPProfile * profile, BOOL force=FALSE);
	BOOL					LockZOS();

	int						LoadFiletypes();
	int						LoadCacheMaps();
	int						EnableCacheMapUI();
	int						OnCacheMapSelect();
	int						UpdateCacheMapSpinner();

	vProfile*				m_profiles;
	FTPProfile*				m_currentProfile;
	FTPCache*				m_globalCache;
	FTPWindow*				m_ftpWindow;

	ChildDialog				m_pageConnection;
	ChildDialog				m_pageAuthentication;
	ChildDialog				m_pageTransfer;
	ChildDialog				m_pageFTP;
	ChildDialog				m_pageCache;
	HWND					m_hPageConnection;
	HWND					m_hPageAuthentication;
	HWND					m_hPageTransfer;
	HWND					m_hPageFTP;
	HWND					m_hPageCache;
};

#endif //PROFILESDIALOG_H
