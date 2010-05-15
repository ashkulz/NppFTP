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
#include "FTPWindow.h"

#include "FTPSession.h"
#include "resource.h"
#include "InputDialog.h"
#include "MessageDialog.h"
#include "Npp/Notepad_plus_msgs.h"

#include "Commands.h"
#include <Commctrl.h>
#include <Windowsx.h>

const TCHAR * FTPWindow::FTPWINDOWCLASS = TEXT("NPPFTPMAIN");

FTPWindow::FTPWindow() :
	DockableWindow(FTPWINDOWCLASS),
	m_treeimagelist(m_hInstance),
	m_outputShown(false),
	m_currentSelection(NULL),
	m_localFileExists(false),
	m_ftpSession(NULL),
	m_vProfiles(NULL),
	m_globalCache(NULL),
	m_connecting(false),
	m_busy(false),
	m_cancelOperation(NULL)
{
	m_exStyle = WS_EX_NOACTIVATE;
	m_style = 0;

	//Create background brush
	//DWORD colorBkGnd = GetSysColor(COLOR_3DFACE);
	m_backgroundBrush = GetSysColorBrush(COLOR_3DFACE);//CreateSolidBrush(colorBkGnd);
}

FTPWindow::~FTPWindow() {
	//DeleteObject(m_backgroundBrush);
}

int FTPWindow::Create(HWND hParent, HWND hNpp, int MenuID, int MenuCommand) {
	SetTitle(TEXT("NppFTP"));
	SetInfo(TEXT("Disconnected"));
	SetLocation(DWS_DF_CONT_RIGHT);
	HICON icon = ::LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_ICON_FOLDERS));
	SetIcon(icon);

	int res = DockableWindow::Create(hParent, hNpp, MenuID, MenuCommand);
	if (res != 0)
		return -1;

	res = m_rebar.Create(m_hwnd);
	if (res != 0) {
		Destroy();
		return -1;
	}

	res = m_toolbar.Create(m_rebar.GetHWND());
	if (res != 0) {
		Destroy();
		return -1;
	}

	res = m_treeview.Create(m_hwnd);
	if (res != 0) {
		Destroy();
		return -1;
	}

	res = m_queueWindow.Create(m_hwnd);
	if (res != 0) {
		Destroy();
		return -1;
	}

	res = m_outputWindow.Create(hNpp, hNpp, 99, -1);	//99, pretty sure to be out of bounds
	if (res != 0) {
		Destroy();
		return -1;
	}

	m_treeimagelist.SetTreeview(&m_treeview);
	m_treeimagelist.SetFancyIcon(true);
	m_toolbar.AddToRebar(&m_rebar);

	CreateMenus();

	m_toolbar.SetMenu(IDB_BUTTON_TOOLBAR_CONNECT, m_popupProfile);
	m_toolbar.SetMenu(IDB_BUTTON_TOOLBAR_SETTINGS, m_popupSettings);

	m_toolbar.Show(true);
	m_rebar.Show(true);
	m_treeview.Show(true);
	m_queueWindow.Show(true);

	TCHAR source[MAX_PATH];
	res = ::SendMessage(m_hNpp, NPPM_GETFULLCURRENTPATH, (WPARAM)MAX_PATH, (LPARAM)source);
	if (res == TRUE) {
		OnActivateLocalFile(source);
	}
	SetToolbarState();

	return 0;
}

int FTPWindow::Destroy() {
	m_treeview.Destroy();
	m_toolbar.Destroy();
	m_rebar.Destroy();

	DestroyMenu(m_popupProfile);
	DestroyMenu(m_popupFile);
	DestroyMenu(m_popupDir);
	DestroyMenu(m_popupLink);

	return Window::Destroy();
}

int FTPWindow::Show(bool show) {
	int ret = DockableWindow::Show(show);
	if (ret != 0)
		return -1;

	if (show) {
		if (m_outputShown) {
			m_outputWindow.Show(true);
		}
	} else {
		m_outputWindow.Show(false);
	}

	return 0;
}

int FTPWindow::SetSession(FTPSession * ftpSession) {
	m_ftpSession = ftpSession;
	return 0;
}

int FTPWindow::SetProfilesVector(vProfile * vProfiles) {
	m_vProfiles = vProfiles;

	OnProfileChange();

	return 0;
}

int FTPWindow::SetGlobalCache(FTPCache * globalCache) {
	m_globalCache = globalCache;
	return 0;
}

int FTPWindow::OnSize(int newWidth, int newHeight) {
	int toolbarHeight = m_rebar.GetHeight();

	m_rebar.Resize(newWidth, toolbarHeight);

	int clientheight = newHeight - toolbarHeight;
	int treeheight = clientheight/2;
	int queueheight = clientheight/2 - 6;

	m_treeview.Move(0, toolbarHeight, newWidth, treeheight);

	m_queueWindow.Move(0, toolbarHeight+treeheight+5, newWidth-1, queueheight);	//listview appears to be 1px wider than treeview

	return 0;
}

int FTPWindow::OnProfileChange() {
	if (!m_vProfiles)
		return -1;

	::DestroyMenu(m_popupProfile);
	m_popupProfile = ::CreatePopupMenu();
	for(size_t i = 0; i < m_vProfiles->size(); i++) {
		::AppendMenu(m_popupProfile, MF_STRING, IDM_POPUP_PROFILE_FIRST + i, m_vProfiles->at(i)->GetName());
	}

	m_toolbar.SetMenu(IDB_BUTTON_TOOLBAR_CONNECT, m_popupProfile);

	SetToolbarState();

	return 0;
}

int FTPWindow::OnActivateLocalFile(const TCHAR* filename) {
	m_localFileExists = false;
	if (!filename) {
		SetToolbarState();
		return -1;
	}

	BOOL exist = PathFileExists(filename);
	if (exist == TRUE) {
		BOOL isdir = PathIsDirectory(filename);	//Cannot upload directories
		m_localFileExists = (isdir == FALSE);
	}

	SetToolbarState();

	return 0;
}

int FTPWindow::RegisterClass() {
	WNDCLASSEX FTPWindowClass;
	FTPWindowClass.cbSize = sizeof(WNDCLASSEX);
	FTPWindowClass.style = CS_DBLCLKS;//|CS_NOCLOSE;
	FTPWindowClass.cbClsExtra = 0;
	FTPWindowClass.cbWndExtra = 0;
	FTPWindowClass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	FTPWindowClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	FTPWindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW+1;
	FTPWindowClass.lpszMenuName = NULL;
	FTPWindowClass.hIconSm = NULL;

	FTPWindowClass.lpfnWndProc = NULL;
	FTPWindowClass.hInstance = NULL;
	FTPWindowClass.lpszClassName = NULL;

	//register the class
	int ret = Window::RegisterClass(FTPWINDOWCLASS, FTPWindowClass);
	if (ret != 0)
		return -1;

	return 0;
}

LRESULT FTPWindow::MessageProc(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	bool doDefaultProc = false;
	LRESULT result = 0;

	switch(uMsg) {
		case WM_ERASEBKGND: {
			HDC hDC = (HDC) wParam;
			RECT rectClient;
			GetClientRect(m_hwnd, &rectClient);
			FillRect(hDC, &rectClient, m_backgroundBrush);
			result = TRUE;
			break; }
		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
				case IDM_POPUP_QUEUE_ABORT: {
					if (m_cancelOperation && m_cancelOperation->GetRunning()) {
						m_ftpSession->AbortTransfer();
					}
					m_cancelOperation = NULL;
					result = TRUE;
					break; }
				case IDM_POPUP_QUEUE_CANCEL: {
					if (m_cancelOperation && !m_cancelOperation->GetRunning()) {
						m_ftpSession->CancelOperation(m_cancelOperation);
					}
					m_cancelOperation = NULL;
					result = TRUE;
					break; }
				case IDM_POPUP_LINKTYPE: {
					m_currentSelection->SetDir(!m_currentSelection->isDir());
					m_currentSelection->GetParent()->Sort();
					m_treeview.UpdateFileObject(m_currentSelection);
					result = TRUE;
					break; }
				case IDB_BUTTON_TOOLBAR_CONNECT: {
					//Called by chevron popup: disconnect (ie no popup)
					//disconnect();
					result = TRUE;
					break;}
				case IDM_POPUP_DOWNLOADFILE:
				case IDB_BUTTON_TOOLBAR_DOWNLOAD: {
					m_ftpSession->DownloadFileCache(m_currentSelection->GetPath());
					result = TRUE;
					break; }
				case IDM_POPUP_DLDTOLOCATION: {
					TCHAR target[MAX_PATH];
					lstrcpy(target, m_currentSelection->GetLocalName());
					int res = PU::GetSaveFilename(target, MAX_PATH, m_hwnd);
					if (res == 0) {
						m_ftpSession->DownloadFile(m_currentSelection->GetPath(), target, false);
					}
					result = TRUE;
					break; }
				case IDM_POPUP_UPLOADFILE:
				case IDB_BUTTON_TOOLBAR_UPLOAD: {
					//upload(TRUE, TRUE);		//upload to cached folder is present, else upload to last selected folder
					//m_ftpSession->UploadFile();
					TCHAR source[MAX_PATH];
					BOOL res = ::SendMessage(m_hNpp, NPPM_GETFULLCURRENTPATH, (WPARAM)MAX_PATH, (LPARAM)source);
					if (res == TRUE) {
						if (m_currentSelection->isDir()) {
							m_ftpSession->UploadFile(source, m_currentSelection->GetPath(), true);
						} else {
							m_ftpSession->UploadFile(source, m_currentSelection->GetParent()->GetPath(), true);
						}
					}
					result = TRUE;
					break;}
				case IDM_POPUP_UPLOADOTHERFILE: {
					TCHAR source[MAX_PATH];
					source[0] = 0;
					int res = PU::GetOpenFilename(source, MAX_PATH, m_hwnd);
					if (res == 0) {
						m_ftpSession->UploadFile(source, m_currentSelection->GetPath(), true);
					}
					result = TRUE;
					break; }
				case IDM_POPUP_REFRESHDIR:
				case IDB_BUTTON_TOOLBAR_REFRESH: {
					m_ftpSession->GetDirectory(m_currentSelection->GetPath());
					result = TRUE;
					break; }
				case IDB_BUTTON_TOOLBAR_ABORT: {
					m_ftpSession->AbortTransfer();
					result = TRUE;
					break; }
				case IDB_BUTTON_TOOLBAR_RAWCMD: {
					//rawCommand();
					result = TRUE;
					break; }
				case IDB_BUTTON_TOOLBAR_SETTINGS: {
					//Should be handled by dropdown
					result = TRUE;
					break; }
				case IDB_BUTTON_TOOLBAR_MESSAGES: {
					m_outputShown = !m_outputShown;
					m_outputWindow.Show(m_outputShown);
					result = TRUE;
					break; }
				case IDM_POPUP_NEWDIR: {
					this->CreateDirectory(m_currentSelection);
					result = TRUE;
					break; }
				case IDM_POPUP_DELETEDIR: {
					this->DeleteDirectory(m_currentSelection);
					result = TRUE;
					break; }
				case IDM_POPUP_NEWFILE: {
					this->CreateFile(m_currentSelection);
					result = TRUE;
					break; }
				case IDM_POPUP_DELETEFILE: {
					this->DeleteFile(m_currentSelection);
					result = TRUE;
					break; }
				case IDM_POPUP_RENAMEFILE:
				case IDM_POPUP_RENAMEDIR: {
					this->Rename(m_currentSelection);
					result = TRUE;
					break; }
				case IDM_POPUP_SETTINGSGENERAL: {
					m_settingsDialog.Create(m_hwnd, m_globalCache);
					result = TRUE;
					break; }
				case IDM_POPUP_SETTINGSPROFILE: {
					m_profilesDialog.Create(m_hwnd, this, m_vProfiles, m_globalCache);
					result = TRUE;
					break; }
				default: {
					unsigned int value = LOWORD(wParam);
					if (!m_busy && value >= IDM_POPUP_PROFILE_FIRST && value <= IDM_POPUP_PROFILE_MAX) {
						FTPProfile * profile = m_vProfiles->at(value - IDM_POPUP_PROFILE_FIRST);
						m_ftpSession->StartSession(profile);
						m_ftpSession->Connect();
						result = TRUE;
					} else {
						doDefaultProc = true;
					}
					break; }
			}
			break; }
		case WM_NOTIFY: {
			NMHDR nmh = (NMHDR) *((NMHDR*)lParam);
			if (nmh.hwndFrom == m_toolbar.GetHWND()) {
				switch(nmh.code) {
					case TBN_DROPDOWN: {
						NMTOOLBAR * pnmtb = (NMTOOLBAR*)lParam;
						switch(pnmtb->iItem) {
							case IDB_BUTTON_TOOLBAR_CONNECT: {
								if (m_ftpSession->IsConnected()) {	//only call disconnect routine to disconnect, else pick profile
									m_ftpSession->TerminateSession();
									return TBDDRET_DEFAULT;
								}
								result = m_toolbar.DoDropDown(IDB_BUTTON_TOOLBAR_CONNECT);
								break; }
							case IDB_BUTTON_TOOLBAR_SETTINGS: {
								result = m_toolbar.DoDropDown(IDB_BUTTON_TOOLBAR_SETTINGS);
								break; }
							default: {
								result = TBDDRET_NODEFAULT;
								break; }
						}
						break; }
					default: {
						doDefaultProc = true;
						break; }
				}
			} else if (nmh.hwndFrom == m_hNpp) {
				return DockableWindow::MessageProc(uMsg, wParam, lParam);
			} else if (nmh.hwndFrom == m_treeview.GetHWND()) {
				switch(nmh.code) {
					case TVN_SELCHANGED: {
						const NM_TREEVIEW & nmt = (NM_TREEVIEW) *(NM_TREEVIEW*)lParam;
						m_currentSelection = m_treeview.GetItemFileObject(nmt.itemNew.hItem);
						SetToolbarState();
						result = TRUE;
						break; }
					case TVN_ITEMEXPANDING: {
						const NM_TREEVIEW & nmt = (NM_TREEVIEW) *(NM_TREEVIEW*)lParam;
						int res = m_treeview.OnExpanding(&nmt);
						if (res == TRUE) {
							FileObject * fo = (FileObject*) nmt.itemNew.lParam;
							m_ftpSession->GetDirectory(fo->GetPath());
						}
						return res;
						break; }
					case TVN_GETDISPINFO: {
						TV_DISPINFO * ptvdi = (TV_DISPINFO*)lParam;
						m_treeview.GetDispInfo(ptvdi);
						result = TRUE;
						break; }
					case NM_RCLICK:
					case NM_DBLCLK:
					case NM_CLICK: {
						HTREEITEM res = m_treeview.OnClick();
						if (res) {
							result = TRUE;
							m_currentSelection = m_treeview.GetItemFileObject(res);
							SetToolbarState();
							if (nmh.code == (UINT)NM_RCLICK) {
								DWORD pos = GetMessagePos();
								SHORT state = GetKeyState(VK_SHIFT);
								if ((state & 0x8000) && m_currentSelection->isLink()) {
									TrackPopupMenu(m_popupLink, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, m_hwnd, NULL);
								} else if (m_currentSelection->isDir()) {
									TrackPopupMenu(m_popupDir, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, m_hwnd, NULL);
								} else {
									TrackPopupMenu(m_popupFile, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, m_hwnd, NULL);
								}
							} else if (nmh.code == (UINT)NM_DBLCLK) {
								OnItemActivation();
							}
						} else {
							result = FALSE;
						}
						break; }
					case NM_RETURN: {
						OnItemActivation();
						result = TRUE;	//handle message
						break; }
					case TVN_SELCHANGING: {
						result = FALSE;
						break; }
					default: {
						doDefaultProc = true;
						break; }
				}
			} else if (nmh.hwndFrom == m_rebar.GetHWND()) {
				switch(nmh.code) {
					case RBN_CHEVRONPUSHED: {
						NMREBARCHEVRON * lpnm = (NMREBARCHEVRON*) lParam;
						POINT pt;
						pt.x = lpnm->rc.left;//right;
						pt.y = lpnm->rc.bottom;
						ClientToScreen(m_rebar.GetHWND(), &pt);
						m_toolbar.DoPopop(pt);
						result = TRUE;
						break; }
					default: {
						doDefaultProc = true;
						break; }
				}
			} else if (nmh.hwndFrom == m_queueWindow.GetHWND()) {
				switch(nmh.code) {
					case NM_RCLICK: {
						QueueOperation * op = m_queueWindow.GetSelectedQueueOperation();
						if (op != NULL)  {
							m_cancelOperation = op;
							bool run = op->GetRunning();
							DWORD pos = GetMessagePos();
							if (run) {
								TrackPopupMenu(m_popupQueueActive, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, m_hwnd, NULL);
							} else {
								TrackPopupMenu(m_popupQueueHold, TPM_LEFTALIGN, GET_X_LPARAM(pos), GET_Y_LPARAM(pos), 0, m_hwnd, NULL);
							}
							//m_cancelOperation = NULL;
						}
						result = TRUE;
						break; }
					default: {
						doDefaultProc = true;
						break; }
				}
			} else {
				switch(nmh.code) {
					case TTN_GETDISPINFO: {
						LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT) lParam;
						lpttt->hinst = NULL;
						lpttt->lpszText = (TCHAR*)m_toolbar.GetTooltip(lpttt->hdr.idFrom);
						result = TRUE;
						break; }
					default: {
						doDefaultProc = true;
						break; }
				}
			}
			break; }
		case NotifyMessageStart:
		case NotifyMessageEnd: {
			bool isStart = (uMsg == (UINT)NotifyMessageStart);
			int code = (int)wParam;
			QueueOperation * queueOp = (QueueOperation*)lParam;
			void * notifyData = queueOp->GetNotifyData();
			int res = OnEvent(queueOp, code, notifyData, isStart);
			if (res != 1)	//if res == 1, then queueop becomes invalid
				queueOp->AckNotification();
			return TRUE;
			break;}
		case NotifyMessageAdd: {
			QueueOperation * queueOp = (QueueOperation*)lParam;
			m_queueWindow.PushQueueItem(queueOp);
			queueOp->AckNotification();
			return TRUE;
			break; }
		case NotifyMessageRemove: {
			QueueOperation * queueOp = (QueueOperation*)lParam;
			m_queueWindow.RemoveQueueItem(queueOp);
			queueOp->AckNotification();
			return TRUE;
			break; }
		case NotifyMessageProgress: {
			QueueOperation * queueOp = (QueueOperation*)lParam;
			m_queueWindow.ProgressQueueItem(queueOp);
			queueOp->AckNotification();
			break; }
		default:
			doDefaultProc = true;
			break;
	}

	if (doDefaultProc)
		result = DockableWindow::MessageProc(uMsg, wParam, lParam);

	return result;
}

int FTPWindow::CreateMenus() {
	//Create menu for settings button on toolbar
	m_popupSettings = CreatePopupMenu();
	AppendMenu(m_popupSettings,MF_STRING,IDM_POPUP_SETTINGSGENERAL,TEXT("&General settings"));
	AppendMenu(m_popupSettings,MF_STRING,IDM_POPUP_SETTINGSPROFILE,TEXT("&Profile settings"));

	//Create context menu for files in folder window
	m_popupFile = CreatePopupMenu();
	AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_DOWNLOADFILE,TEXT("&Download file"));
	AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_DLDTOLOCATION,TEXT("&Save file as..."));
	AppendMenu(m_popupFile,MF_SEPARATOR,0,0);
	AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_RENAMEFILE,TEXT("&Rename File"));
	AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_DELETEFILE,TEXT("D&elete File"));
	//AppendMenu(m_popupFile,MF_SEPARATOR,0,0);
	//AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_PERMISSIONFILE,TEXT("Permissions"));
	//AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_PROPSFILE,TEXT("&Properties"));

	//Create context menu for directories in folder window
	m_popupDir = CreatePopupMenu();
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_NEWDIR,TEXT("Create new &directory"));
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_NEWFILE,TEXT("Create new &file"));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_RENAMEDIR,TEXT("&Rename Directory"));
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_DELETEDIR,TEXT("D&elete directory"));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
    AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_UPLOADFILE,TEXT("&Upload current file here"));
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_UPLOADOTHERFILE,TEXT("Upload &other file here..."));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_REFRESHDIR,TEXT("Re&fresh"));
	//AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
	//AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_PERMISSIONDIR,TEXT("Permissions"));
	//AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_PROPSDIR,TEXT("&Properties"));

	//Create special context menu for links
	m_popupLink = CreatePopupMenu();
	AppendMenu(m_popupLink,MF_STRING,IDM_POPUP_LINKTYPE,TEXT("&Switch link type"));

	//Create empty profile menu, current implementation requires this
	m_popupProfile = CreatePopupMenu();

	//Create menu for running queue items
	m_popupQueueActive = CreatePopupMenu();
	AppendMenu(m_popupQueueActive,MF_STRING,IDM_POPUP_QUEUE_ABORT,TEXT("&Abort operation"));

	//Create menu for queue items on hold
	m_popupQueueHold = CreatePopupMenu();
	AppendMenu(m_popupQueueHold,MF_STRING,IDM_POPUP_QUEUE_CANCEL,TEXT("&Remove operation from queue"));

	return 0;

}

int FTPWindow::SetToolbarState() {
	if (m_vProfiles && m_vProfiles->size() > 0 && !m_connecting) {
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_CONNECT, true);
	} else {
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_CONNECT, false);
	}
	m_toolbar.Enable(IDB_BUTTON_TOOLBAR_ABORT, m_busy);
	m_toolbar.Enable(IDB_BUTTON_TOOLBAR_RAWCMD, false);
	m_toolbar.Enable(IDB_BUTTON_TOOLBAR_SETTINGS, true);
	m_toolbar.Enable(IDB_BUTTON_TOOLBAR_MESSAGES, true);

	if (!m_currentSelection || !m_ftpSession->IsConnected()) {
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_DOWNLOAD, false);
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_UPLOAD, false);
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_REFRESH, false);
	} else {
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_DOWNLOAD, !m_currentSelection->isDir());
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_UPLOAD, m_localFileExists);	//m_currentSelection->isDir());
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_REFRESH, m_currentSelection->isDir());
	}

	return 0;
}

int FTPWindow::OnEvent(QueueOperation * queueOp, int code, void * /*data*/, bool isStart) {
	int result = 0;
	void * queueData = queueOp->GetData();
	int queueResult = queueOp->GetResult();

	//Set busy parameter
	switch(queueOp->GetType()) {
		case QueueOperation::QueueTypeDownload:
		case QueueOperation::QueueTypeUpload: {
			m_busy = isStart;
			break; }
		case QueueOperation::QueueTypeConnect:
		case QueueOperation::QueueTypeDisconnect:
		case QueueOperation::QueueTypeDirectoryGet:
		case QueueOperation::QueueTypeDirectoryCreate:
		case QueueOperation::QueueTypeDirectoryRemove:
		case QueueOperation::QueueTypeFileCreate:
		case QueueOperation::QueueTypeFileDelete:
		case QueueOperation::QueueTypeFileRename:
		case QueueOperation::QueueTypeQuote:
		default: {
			//Other operations cannot be aborted
			break; }
	}

	//TODO: properly handle failures, like with getdir
	//Perform GUI operations
	switch(queueOp->GetType()) {
		case QueueOperation::QueueTypeConnect: {
			m_connecting = isStart;
			if (isStart) {
				SetInfo(TEXT("Connecting"));
				OutMsg("Connecting");
			} else {
				if (queueOp->GetResult() != -1) {
					OnConnect(code);
					SetInfo(TEXT("Connected"));
					OutMsg("Connected");
				} else {
					OutErr("Unable to connect");
					OnDisconnect(code);
					m_ftpSession->TerminateSession();
					result = 1;
				}
			}
			break; }
		case QueueOperation::QueueTypeDisconnect: {
			if (isStart) {
				break;
			}
			OutMsg("Disconnected");
			OnDisconnect(code);
			result = 1;
			break; }
		case QueueOperation::QueueTypeDirectoryGet: {
			QueueGetDir * dirop = (QueueGetDir*)queueOp;
			if (isStart) {
				break;
			}
			if (queueResult == -1) {
				OutErr("Failure retrieving contents of directory %s", dirop->GetDirPath());
				break;	//failure
			}
			FTPFile* files = (FTPFile*)queueData;
			int count = dirop->GetFileCount();
			FileObject* parent = m_ftpSession->FindPathObject(dirop->GetDirPath());
			if (parent)
				OnDirectoryRefresh(parent, files, count);
			break; }
		case QueueOperation::QueueTypeDownload: {
			QueueDownload * opdld = (QueueDownload*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Download of %s failed", opdld->GetExternalPath());
				break;	//failure
			}

			if (code == 0) {
				//Download to cache: Open file
				OutMsg("Download of %s succeeded, opening file.", opdld->GetExternalPath());
				::SendMessage(m_hNpp, NPPM_DOOPEN, (WPARAM)0, (LPARAM)opdld->GetLocalPath());
				::SendMessage(m_hNpp, NPPM_RELOADFILE, (WPARAM)0, (LPARAM)opdld->GetLocalPath());
			} else {
				//Download to other location: Ask
				int ret = ::MessageBox(m_hNpp, TEXT("The download is complete. Do you wish to open the file?"), TEXT("Download complete"), MB_YESNO);
				if (ret == IDYES) {
					::SendMessage(m_hNpp, NPPM_DOOPEN, (WPARAM)0, (LPARAM)opdld->GetLocalPath());
					::SendMessage(m_hNpp, NPPM_RELOADFILE, (WPARAM)0, (LPARAM)opdld->GetLocalPath());
				}
			}
			break; }
		case QueueOperation::QueueTypeUpload: {
			QueueUpload * opuld = (QueueUpload*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Upload of %S failed", opuld->GetLocalPath());
				break;	//failure
			}

			OutMsg("Upload of %s succeeded.", opuld->GetExternalPath());

			char path[MAX_PATH];
			strcpy(path, opuld->GetExternalPath());
			char * name = (char*)PU::FindExternalFilename(path);
			if (!name)
				break;

			*name = 0;	//truncate path

			m_ftpSession->GetDirectory(path);
			break; }
		case QueueOperation::QueueTypeDirectoryCreate: {
			QueueCreateDir * opmkdir = (QueueCreateDir*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to create directory %s", opmkdir->GetDirPath());
				break;	//failure
			}
			OutMsg("Created directory %s", opmkdir->GetDirPath());
			break; }
		case QueueOperation::QueueTypeDirectoryRemove: {
			QueueRemoveDir * oprmdir = (QueueRemoveDir*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to remove directory %s", oprmdir->GetDirPath());
				break;	//failure
			}
			OutMsg("Removed directory %s", oprmdir->GetDirPath());
			break; }
		case QueueOperation::QueueTypeFileCreate: {
			QueueCreateFile * opmkfile = (QueueCreateFile*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to create file %s", opmkfile->GetFilePath());
				break;	//failure
			}
			OutMsg("Created file %s", opmkfile->GetFilePath());
			break; }
		case QueueOperation::QueueTypeFileDelete: {
			QueueDeleteFile * opdelfile = (QueueDeleteFile*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to delete file %s", opdelfile->GetFilePath());
				break;	//failure
			}
			OutMsg("Deleted file %s", opdelfile->GetFilePath());
			break; }
		case QueueOperation::QueueTypeFileRename: {
			QueueRenameFile * oprename = (QueueRenameFile*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to rename file %s", oprename->GetFilePath());
				break;	//failure
			}
			OutMsg("Renamed %s to %s", oprename->GetFilePath(), oprename->GetNewPath());
			break; }
		case QueueOperation::QueueTypeQuote: {
			QueueQuote * opquote = (QueueQuote*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to perform quote operation %s", opquote->GetQuote());
				break;	//failure
			}
			break; }
		default: {
			//Other operations do not require change in GUI atm (update tree for delete/rename/create later on)
			break; }
	}

	SetToolbarState();

	return result;
}

int FTPWindow::OnDirectoryRefresh(FileObject * parent, FTPFile * files, int count) {
	parent->SetRefresh(false);
	parent->RemoveAllChildren(false);
	for(int i = 0; i < count; i++) {
		parent->AddChild(new FileObject(files+i));
	}
	parent->Sort();
	//The treeview is out of sync here, make sure no GUI calls go between this function and the next call
	m_treeview.FillTreeDirectory(parent);
	m_treeview.ExpandDirectory(parent);

	return 0;
}

int FTPWindow::OnItemActivation() {
	if (!m_currentSelection)
		return -1;

	if (m_currentSelection->isDir()) {
		m_ftpSession->GetDirectory(m_currentSelection->GetPath());
	} else {
		m_ftpSession->DownloadFileCache(m_currentSelection->GetPath());
	}
	return 0;
}

int FTPWindow::OnConnect(int code) {
	if (code != 0)	//automated connect
		return 0;

	FileObject * root = m_ftpSession->GetRootObject();
	m_treeview.AddRoot(root);

	FileObject * last = root;
	while(last->GetChildCount() > 0) {
		last = last->GetChild(0);
	}

	m_treeview.EnsureObjectVisible(last);
	TreeView_Select(m_treeview.GetHWND(), last->GetData(), TVGN_CARET);
	m_ftpSession->GetDirectory(last->GetPath());

	SetToolbarState();

	return 0;
}

int FTPWindow::OnDisconnect(int /*code*/) {
	m_currentSelection = NULL;
	m_treeview.ClearAll();

	SetInfo(TEXT("Disconnected"));

	SetToolbarState();

	return 0;
}

int FTPWindow::CreateDirectory(FileObject * parent) {
	InputDialog id;

	int res = id.Create(m_hwnd, TEXT("Creating directory"), TEXT("Please enter the name of the new directory:"), TEXT("New directory"));
	if (res != 1)
		return 0;

	const TCHAR * newName = id.GetValue();
	char path[MAX_PATH];
	res = PU::ConcatLocalToExternal(parent->GetPath(), newName, path, MAX_PATH);
	if (res == -1)
		return -1;

	res = m_ftpSession->MkDir(path);
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(parent->GetPath());

	return 0;
}

int FTPWindow::DeleteDirectory(FileObject * dir) {
	MessageDialog md;

	int res = md.Create(m_hwnd, TEXT("Deleting directory"), TEXT("Are you sure you want to delete this directory?"));
	if (res != 1)
		return 0;

	res = m_ftpSession->RmDir(dir->GetPath());
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(dir->GetParent()->GetPath());

	return 0;
}

int FTPWindow::CreateFile(FileObject * parent) {
	InputDialog id;

	int res = id.Create(m_hwnd, TEXT("Creating file"), TEXT("Please enter the name of the new file:"), TEXT("New file"));
	if (res != 1)
		return 0;

	const TCHAR * newName = id.GetValue();
	char path[MAX_PATH];
	res = PU::ConcatLocalToExternal(parent->GetPath(), newName, path, MAX_PATH);
	if (res == -1)
		return -1;

	res = m_ftpSession->MkFile(path);
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(parent->GetPath());

	return 0;
}

int FTPWindow::DeleteFile(FileObject * file) {
	MessageDialog md;

	int res = md.Create(m_hwnd, TEXT("Deleting file"), TEXT("Are you sure you want to delete this file?"));
	if (res != 1)
		return 0;

	res = m_ftpSession->DeleteFile(file->GetPath());
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(file->GetParent()->GetPath());

	return 0;
}

int FTPWindow::Rename(FileObject * fo) {
	InputDialog id;

	int res = id.Create(m_hwnd, TEXT("Renaming"), TEXT("Please enter the new name:"), fo->GetLocalName());
	if (res != 1)
		return 0;

	const TCHAR * newName = id.GetValue();
	char path[MAX_PATH];
	res = PU::ConcatLocalToExternal(fo->GetParent()->GetPath(), newName, path, MAX_PATH);
	if (res == -1)
		return -1;

	res = m_ftpSession->Rename(fo->GetPath(), path);
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(fo->GetParent()->GetPath());

	return 0;
}
