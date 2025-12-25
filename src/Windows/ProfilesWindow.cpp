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
#include "ProfilesWindow.h"
#include "Commands.h"
#include "FTPWindow.h"
#include "MessageDialog.h"
#include "FTPSession.h"



ProfilesWindow::ProfilesWindow(FTPWindow* ftpwindow):
m_ftpwindow(ftpwindow),
m_profilesDialogSingle(IDD_DIALOG_PROFILES_SINGLE),
m_lastUsedProfile(NULL)
{

}
ProfilesWindow::~ProfilesWindow() {

}

bool ProfilesWindow::EditProfile(FileObject* item)
{
	if (!item) return false;

	ProfileObject* profileObject = dynamic_cast<ProfileObject*>(item);
	if (profileObject == nullptr) return false;

	if (profileObject->isDir())	return false;

	m_profilesDialogSingle.Create(m_ftpwindow->m_hwnd, m_ftpwindow, m_ftpwindow->m_vProfiles, m_ftpwindow->m_ftpSettings->GetGlobalCache(),
						profileObject->GetProfile());
	return true;
}

bool ProfilesWindow::CreateProfile(FileObject* parent)
{
	
	ProfileObject* parentFolder = dynamic_cast<ProfileObject*>(parent);
	if (parentFolder == nullptr) return false;

	FTPProfile* newProfile = new FTPProfile(TEXT("New profile"));
	newProfile->SetCacheParent(m_ftpwindow->m_ftpSettings->GetGlobalCache());
	m_ftpwindow->m_vProfiles->push_back(newProfile);
	newProfile->AddRef();
	newProfile->SetParent(SU::Utf8ToTChar(parentFolder->GetPath()));
	m_ftpwindow->m_treeview.ExpandDirectory(parentFolder);
	ProfileObject* newProfileObject = parentFolder->AddChild("", "New profile", newProfile);
	m_ftpwindow->m_treeview.FillTreeDirectory(parentFolder);
	m_ftpwindow->m_treeview.ExpandDirectory(parentFolder);
	m_ftpwindow->m_treeview.Focus();
	HTREEITEM htiNewProfile = (HTREEITEM)newProfileObject->GetData();
	TreeView_SelectItem(m_ftpwindow->m_treeview.GetHWND(), htiNewProfile);

	m_profilesDialogSingle.Create(m_ftpwindow->m_hwnd, m_ftpwindow, m_ftpwindow->m_vProfiles, m_ftpwindow->m_ftpSettings->GetGlobalCache(), newProfile);

	m_ftpwindow->m_treeview.Focus();
	TreeView_SelectItem(m_ftpwindow->m_treeview.GetHWND(), htiNewProfile);
	TreeView_EditLabel(m_ftpwindow->m_treeview.GetHWND(), htiNewProfile);
	return TRUE;
}

bool ProfilesWindow::DeleteProfile(FileObject* profileObject)
{
	ProfileObject* profile= dynamic_cast<ProfileObject*>(profileObject);
	if (profile == nullptr) return false;

	MessageDialog md;

	if (profile->isRoot()) return -1;

	int	res = -1;
	if (profile->isDir()) {
		res = md.Create(m_ftpwindow->m_hwnd, TEXT("Deleting Folder and all Profiles"), TEXT("Are you sure you want to delete this Folder and all subitems?"));
	}
	else {
		res = md.Create(m_ftpwindow->m_hwnd, TEXT("Deleting Profile"), TEXT("Are you sure you want to delete Profile?"));
	}
	if (res != 1)
		return -1;

	HTREEITEM hti = (HTREEITEM)(profile->GetData());
	if (profile->GetParent()->RemoveChild(profile, true) >= 0) {
		TreeView_DeleteItem(m_ftpwindow->m_treeview.GetHWND(), hti);
		m_ftpwindow->m_currentSelection = NULL;
	}

	return true;
}

bool ProfilesWindow::CreateProfileFolder(FileObject* parent)
{
	ProfileObject* parentFolder = dynamic_cast<ProfileObject*>(parent);
	if (parentFolder == nullptr) return false;

	ProfileObject* newFolder = parentFolder->AddChild("New Folder", "");
	m_ftpwindow->m_treeview.FillTreeDirectory(parentFolder);
	m_ftpwindow->m_treeview.ExpandDirectory(parentFolder);
	m_ftpwindow->m_treeview.Focus();
	HTREEITEM htiNewFolder = (HTREEITEM)newFolder->GetData();
	TreeView_SelectItem(m_ftpwindow->m_treeview.GetHWND(), htiNewFolder);
	TreeView_EditLabel(m_ftpwindow->m_treeview.GetHWND(), htiNewFolder);
	m_ftpwindow->m_currentSelection = newFolder;

	return true;
}

bool ProfilesWindow::OnProfileItemDrop(FileObject* item, FileObject* parent, bool bIsMove)
{
	if (item == NULL || parent == NULL) return false;

	ProfileObject* dropObject = dynamic_cast<ProfileObject*>(item);
	if (dropObject == nullptr) return false;
	ProfileObject* parentFolder = dynamic_cast<ProfileObject*>(parent);
	if (parentFolder == nullptr) return false;


	if (!bIsMove) {
		bool bSameName = FALSE;

		ProfileObject* newProfileObject = dropObject->CopyTo(parentFolder, m_ftpwindow->m_ftpSettings, bSameName);
		m_ftpwindow->m_treeview.FillTreeDirectory(parentFolder);
		m_ftpwindow->m_treeview.ExpandDirectory(parentFolder);
		HTREEITEM htiNewProfile = (HTREEITEM)newProfileObject->GetData();
		TreeView_SelectItem(m_ftpwindow->m_treeview.GetHWND(), htiNewProfile);

		if (bSameName)
			TreeView_EditLabel(m_ftpwindow->m_treeview.GetHWND(), htiNewProfile);

	}
	else {
		HTREEITEM hti = (HTREEITEM)(dropObject->GetData());
		dropObject->MoveTo(parentFolder);

		TVITEMEX tvi{};
		tvi.hItem = hti;
		tvi.mask = TVIF_HANDLE;
		if (TreeView_GetItem(m_ftpwindow->m_treeview.GetHWND(), &tvi)) //check if handle is valid
			TreeView_DeleteItem(m_ftpwindow->m_treeview.GetHWND(), hti);
		m_ftpwindow->m_treeview.FillTreeDirectory(parentFolder);
		m_ftpwindow->m_treeview.ExpandDirectory(parentFolder);
		hti = (HTREEITEM)(dropObject->GetData());
		TreeView_SelectItem(m_ftpwindow->m_treeview.GetHWND(), hti);
	}

	m_ftpwindow->m_currentDragObject = NULL;
	return true;

}

bool ProfilesWindow::RenameProfileObject(FileObject* fo, const TCHAR* newName)
{
	ProfileObject* profileObject = dynamic_cast<ProfileObject*>(fo);
	if (profileObject == nullptr) return false;

	char* lastPath = SU::strdup(profileObject->GetPath());
	if (profileObject->SetName(SU::TCharToUtf8(newName)) < 0)
		return true;
	if(strcmp(lastPath,profileObject->GetPath()) == 0)
		m_lastUsedProfile = profileObject->GetPath();

	return true;

}

bool ProfilesWindow::OnProfileitemActivation(FileObject* item)
{

	if (!item)	return true;

	ProfileObject* profileItem = dynamic_cast<ProfileObject*>(item);
	if (profileItem == nullptr) return false;

	if (profileItem->isDir()) {
		if (m_ftpwindow->m_treeview.CollapseDirectory(profileItem) != 0)
			m_ftpwindow->m_treeview.ExpandDirectory(profileItem);
		return true;
	}
	else {
		ConnectRemote(profileItem);
	}

	return true;
}

bool ProfilesWindow::ConnectRemote(FileObject* item)
{
	ProfileObject* profileItem = dynamic_cast<ProfileObject*>(item);
	if (profileItem == nullptr) return false;

	return ConnectRemote(profileItem);
}

bool ProfilesWindow::ConnectRemote(ProfileObject* profileItem)
{
	if (!profileItem || profileItem->isDir()) return false;
	FTPProfile* profile = profileItem->GetProfile();
	int ret = m_ftpwindow->m_ftpSession->StartSession(profile);
	if (ret == -1) {
		OutErr("[NppFTP] Cannot start FTP session");
		return false;
	}
	else {
		m_ftpwindow->m_ftpSession->Connect();
		m_lastUsedProfile = profileItem->GetPath();
	}

	return true;
}

HMENU ProfilesWindow::SetProfilePopupMenu(FileObject* item)
{

	if (!item)	return NULL;

	ProfileObject* profileItem = dynamic_cast<ProfileObject*>(item);
	if (profileItem == nullptr) return NULL;

	HMENU hContext = NULL;
	if (profileItem->isDir()) {
		if (!profileItem->isRoot())
			hContext = m_popupTreeProfileFolder;
		else
			hContext = m_popupTreeProfileRootFolder;
	}
	else {
		hContext = m_popupTreeProfile;
	}

	return hContext;
}

int ProfilesWindow::CreateMenus()
{
	//Create context menu for Profile in Treeview
	m_popupTreeProfile = CreatePopupMenu();
	AppendMenu(m_popupTreeProfile, MF_STRING, IDM_POPUP_PROFILE_CONNECT, TEXT("&Connect"));
	AppendMenu(m_popupTreeProfile, MF_STRING, IDM_POPUP_PROFILE_EDIT, TEXT("&Edit Profile"));
	AppendMenu(m_popupTreeProfile, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfile, MF_STRING, IDM_POPUP_RENAMEFILE, TEXT("&Rename Profile"));
	AppendMenu(m_popupTreeProfile, MF_STRING, IDM_POPUP_PROFILE_DELETE, TEXT("&Delete Profile"));
	AppendMenu(m_popupTreeProfile, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfile, MF_STRING, IDM_POPUP_COPY, TEXT("&Copy Profile"));
	AppendMenu(m_popupTreeProfile, MF_STRING, IDM_POPUP_CUT, TEXT("&Cut Profile"));

	SetMenuDefaultItem(m_popupTreeProfile, IDM_POPUP_PROFILE_CONNECT, FALSE);

	//Create context menu for Profile Folder in Treeview
	m_popupTreeProfileFolder = CreatePopupMenu();
	AppendMenu(m_popupTreeProfileFolder, MF_STRING, IDM_POPUP_PROFILE_CREATE, TEXT("&Create new Profile"));
	AppendMenu(m_popupTreeProfileFolder, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfileFolder, MF_STRING, IDM_POPUP_PROFILE_FOLDER_CREATE, TEXT("&Create new Folder"));
	AppendMenu(m_popupTreeProfileFolder, MF_STRING, IDM_POPUP_RENAMEFILE, TEXT("&Rename Folder"));
	AppendMenu(m_popupTreeProfileFolder, MF_STRING, IDM_POPUP_PROFILE_DELETE, TEXT("&Delete Folder"));
	AppendMenu(m_popupTreeProfileFolder, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfileFolder, MF_STRING, IDM_POPUP_COPY, TEXT("&Copy Folder"));
	AppendMenu(m_popupTreeProfileFolder, MF_STRING, IDM_POPUP_CUT, TEXT("&Cut Folder"));
	AppendMenu(m_popupTreeProfileFolder, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfileFolder, MF_STRING | MF_GRAYED, IDM_POPUP_PASTE, TEXT("&Paste"));

	SetMenuDefaultItem(m_popupTreeProfileFolder, IDM_POPUP_PROFILE_FOLDER_CREATE, FALSE);

	//Create context menu for Profile Root Folder in Treeview
	m_popupTreeProfileRootFolder = CreatePopupMenu();
	AppendMenu(m_popupTreeProfileRootFolder, MF_STRING, IDM_POPUP_PROFILE_CREATE, TEXT("&Create new Profile"));
	AppendMenu(m_popupTreeProfileRootFolder, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfileRootFolder, MF_STRING, IDM_POPUP_PROFILE_FOLDER_CREATE, TEXT("&Create new Folder"));
	AppendMenu(m_popupTreeProfileRootFolder, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupTreeProfileRootFolder, MF_STRING | MF_GRAYED, IDM_POPUP_PASTE, TEXT("&Paste"));

	SetMenuDefaultItem(m_popupTreeProfileRootFolder, IDM_POPUP_PROFILE_FOLDER_CREATE, FALSE);

	return 0;
}

int ProfilesWindow::InitProfilesTree()
{
	m_ftpwindow->m_currentSelection = NULL;
	m_ftpwindow->m_currentDragObject = NULL;
	m_ftpwindow->m_currentDropObject = NULL;
	m_ftpwindow->m_treeview.ClearAll();

	ProfileObject* tvRoot = new ProfileObject("/", "Profiles", true);
	tvRoot->SetProfiles(m_ftpwindow->m_vProfiles);
	for (size_t i = 0; i < m_ftpwindow->m_vProfiles->size(); i++) {
		FTPProfile* profile = m_ftpwindow->m_vProfiles->at(i);
		tvRoot->AddChild(SU::TCharToUtf8(profile->GetParent()), SU::TCharToUtf8(profile->GetName()), profile);
	}
	m_ftpwindow->m_treeview.AddRoot(tvRoot);

	ProfileObject* lastProfile = NULL;
	if (m_lastUsedProfile != NULL && (lastProfile = tvRoot->GetChildByPath(m_lastUsedProfile)) != NULL) {
		m_ftpwindow->m_treeview.EnsureObjectVisible(lastProfile->GetParent());
		m_ftpwindow->m_treeview.ExpandDirectory(lastProfile->GetParent(), lastProfile);
	}
	else {
		ProfileObject* firstProfile = tvRoot->GetFirstProfile();
		if (firstProfile != NULL)
			m_ftpwindow->m_treeview.EnsureObjectVisible(firstProfile);
		else
			m_ftpwindow->m_treeview.ExpandDirectory(tvRoot);
	}

	return 0;
}
