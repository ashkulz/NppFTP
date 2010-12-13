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

#ifndef FTPWINDOW_H
#define FTPWINDOW_H

#include "DockableWindow.h"

#include "Toolbar.h"
#include "Treeview.h"
#include "QueueWindow.h"
#include "FTPProfile.h"
#include "FTPSettings.h"
#include "QueueOperation.h"
#include "OutputWindow.h"
#include "SettingsDialog.h"
#include "ProfilesDialog.h"
#include "DragDropSupport.h"
#include "DragDropWindow.h"

class FTPSession;

class FTPWindow : public DockableWindow, public DropTargetWindow, public DropDataWindow {
	friend class NppFTP;	//for output window/ratio
public:
							FTPWindow();
	virtual					~FTPWindow();

	////////////////////////
	//DockableWindow
	virtual int				Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand);
	virtual int				Destroy();

	virtual int				Show(bool show);
	virtual int				Focus();

	virtual int				Init(FTPSession * session, vProfile * vProfiles, FTPSettings * ftpSettings);

	virtual int				OnSize(int newWidth, int newHeight);
	virtual int				OnProfileChange();
	virtual int				OnActivateLocalFile(const TCHAR* filename);

	static int				RegisterClass();

	virtual LRESULT			MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	////////////////////////
	//DropTargetWindow
	virtual bool			AcceptType(LPDATAOBJECT pDataObj);
	virtual HRESULT			OnDragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	virtual HRESULT			OnDragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	virtual HRESULT			OnDragLeave();
	virtual HRESULT			OnDrop(LPDATAOBJECT pDataObj, DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);

	////////////////////////
	//DropDataWindow
	virtual int				GetNrFiles();
	virtual int				GetFileDescriptor(FILEDESCRIPTOR * fd, int index);
	virtual int				StreamData(CStreamData * stream, int index);
	virtual int				OnEndDnD();

protected:
	virtual int				CreateMenus();
	virtual int				SetToolbarState();

	virtual int				OnEvent(QueueOperation * queueOp, int code, void * data, bool isStart);
	virtual int				OnDirectoryRefresh(FileObject * parent, FTPFile * files, int count);
	virtual int				OnError(QueueOperation * queueOp, int code, void * data, bool isStart);

	virtual int				OnItemActivation();

	virtual int				OnConnect(int code);
	virtual int				OnDisconnect(int code);

	virtual int				CreateDirectory(FileObject * parent);
	virtual int				DeleteDirectory(FileObject * dir);

	virtual int				CreateFile(FileObject * parent);
	virtual int				DeleteFile(FileObject * file);

	virtual int				Rename(FileObject * fo);

	//virtual int				UploadCurrentFile(FileObject * parent);
	//virtual int				UploadOtherFile(FileObject * parent);

	Toolbar					m_toolbar;
	Rebar					m_rebar;
	Treeview				m_treeview;
	TreeImageList			m_treeimagelist;
	QueueWindow				m_queueWindow;
	SettingsDialog			m_settingsDialog;
	ProfilesDialog			m_profilesDialog;
	WindowSplitter			m_splitter;

	OutputWindow			m_outputWindow;
	bool					m_outputShown;

	HBRUSH					m_backgroundBrush;

	FileObject*				m_currentSelection;
	bool					m_localFileExists;

	HMENU					m_popupProfile;
	HMENU					m_popupSettings;
	HMENU					m_popupFile;
	HMENU					m_popupDir;
	HMENU					m_popupLink;
	HMENU					m_popupQueueActive;
	HMENU					m_popupQueueHold;

	FTPSession*				m_ftpSession;
	vProfile*				m_vProfiles;
	FTPSettings*			m_ftpSettings;

	bool					m_connecting;
	bool					m_busy;
	QueueOperation*			m_cancelOperation;

	DragDropWindow			m_dndWindow;
	FileObject*				m_currentDropObject;

	static const TCHAR * FTPWINDOWCLASS;
};

#endif //FTPWINDOW_H
