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

#ifndef PROFILESWINDOW_H
#define PROFILESWINDOW_H

#include "ProfileObject.h"
#include "Treeview.h"
#include "resource.h"
#include "ProfilesDialog.h"

class FTPWindow;

class ProfilesWindow  {
	
public:
	ProfilesWindow(FTPWindow* ftpwindow);
	virtual					~ProfilesWindow();
protected:
	virtual int				CreateMenus();
	virtual int				InitProfilesTree();
	virtual bool			EditProfile(FileObject* item);
	virtual bool			CreateProfile(FileObject* parentFolder);
	virtual bool			DeleteProfile(FileObject* profileObject);
	virtual bool			CreateProfileFolder(FileObject* parentFolder);
	virtual bool			OnProfileItemDrop(FileObject* item, FileObject* parent, bool bIsMove);
	virtual bool			RenameProfileObject(FileObject* fo, const TCHAR* _newName);
	virtual bool			OnProfileitemActivation(FileObject* item);
	virtual bool			ConnectRemote(FileObject* item);
	virtual bool			ConnectRemote(ProfileObject* item);
	virtual HMENU			SetProfilePopupMenu(FileObject* item);
	HMENU					m_popupTreeProfile;
	HMENU					m_popupTreeProfileFolder;
	HMENU					m_popupTreeProfileRootFolder;
	FTPWindow*				m_ftpwindow;
	ProfilesDialog			m_profilesDialogSingle;
	const char*				m_lastUsedProfile;


private:
};

#endif //FTPWINDOW_H

