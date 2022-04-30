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
#include <commctrl.h>
#include <windowsx.h>
#include <typeinfo>

const TCHAR * FTPWindow::FTPWINDOWCLASS = TEXT("NPPFTPMAIN");

FTPWindow::FTPWindow() :
	DockableWindow(FTPWINDOWCLASS),
	DropTargetWindow(),
	ProfilesWindow(this),
	m_treeimagelist(m_hInstance),
	m_splitter(this, &m_treeview, &m_queueWindow),
	m_outputShown(false),
	m_currentSelection(NULL),
	m_localFileExists(false),
	m_ftpSession(NULL),
	m_vProfiles(NULL),
	m_ftpSettings(NULL),
	m_connecting(false),
	m_busy(false),
	m_cancelOperation(NULL),
	m_dndWindow(this),
	m_currentDropObject(NULL),
	m_currentDragObject(NULL)
{
	m_exStyle = 0;
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

	res = m_outputWindow.Create(hNpp, hNpp, 99, -1, m_hwnd);	//99, pretty sure to be out of bounds
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

	//Notepad++ still uses DragAcceptFiles. Dropping files still works, but the cursor wont be updaetd nicely
	//Uncomment this to disable the Notepad++ behaviour, for debugging purposes only.
	//DragAcceptFiles(m_hNpp, FALSE);
	m_dropHwnd = m_hwnd;
	DoRegisterDragDrop(m_treeview.GetHWND());

	return 0;
}

int FTPWindow::Destroy() {
	m_treeview.Destroy();
	m_toolbar.Destroy();
	m_rebar.Destroy();

	m_dndWindow.Destroy();

	DestroyMenu(m_popupProfile);
	DestroyMenu(m_popupFile);
	DestroyMenu(m_popupTreeProfile);
	DestroyMenu(m_popupTreeProfileRootFolder);
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
		//hiding output window causes m_outputShown to be reset, so set it back to true again
		if (m_outputShown) {
			m_outputWindow.Show(false);
			m_outputShown = true;
			m_ftpSettings->SetOutputShown(true);
		}
	}

	//Focus();

	return 0;
}

int FTPWindow::Focus() {
	return m_treeview.Focus();
}

int FTPWindow::Init(FTPSession * session, vProfile * vProfiles, FTPSettings * ftpSettings) {
	m_ftpSession = session;
	m_vProfiles = vProfiles;
	m_ftpSettings = ftpSettings;

	OnProfileChange();
	m_splitter.SetRatio(m_ftpSettings->GetSplitRatio());
	m_outputShown = m_ftpSettings->GetOutputShown();

	return 0;
}

int FTPWindow::OnSize(int newWidth, int newHeight) {
	int toolbarHeight = m_rebar.GetHeight();

	m_rebar.Resize(newWidth, toolbarHeight);

	int clientheight = newHeight - toolbarHeight;

	m_splitter.OnSize(0, toolbarHeight, newWidth, clientheight);
/*
	int treeheight = clientheight/2;
	int queueheight = clientheight - treeheight;

	m_treeview.Move(0, toolbarHeight, newWidth, treeheight);

	m_queueWindow.Move(0, toolbarHeight+treeheight+5, newWidth-1, queueheight);	//listview appears to be 1px wider than treeview
*/
	return 0;
}

int FTPWindow::OnProfileChange() {
	if (!m_vProfiles)
		return -1;
	
	std::vector<const TCHAR*> strSubmenus;
	std::vector<HMENU> hSubmenus;
	
	::DestroyMenu(m_popupProfile);
	m_popupProfile = ::CreatePopupMenu();
	for(size_t i = 0; i < m_vProfiles->size(); i++) {
		if(m_vProfiles->at(i)->GetParent() != NULL && _tcscmp(m_vProfiles->at(i)->GetParent(), _T("")) != 0 )
		{
			HMENU hSubMenu = NULL;
			
			for(size_t k = 0;k < strSubmenus.size(); k++) {
				if(_tcscmp(strSubmenus.at(k),m_vProfiles->at(i)->GetParent()) == 0 )
				{
					hSubMenu=hSubmenus.at(k);
					break;
				}
			}
			
			if(hSubMenu == NULL)
			{
				hSubMenu = CreatePopupMenu();
				strSubmenus.push_back(m_vProfiles->at(i)->GetParent());
				hSubmenus.push_back(hSubMenu);
				::AppendMenu(m_popupProfile, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, m_vProfiles->at(i)->GetParent());
			}
			
			::AppendMenu(hSubMenu, MF_STRING, IDM_POPUP_PROFILE_FIRST + i, m_vProfiles->at(i)->GetName());
		}
		else
			::AppendMenu(m_popupProfile, MF_STRING, IDM_POPUP_PROFILE_FIRST + i, m_vProfiles->at(i)->GetName());
	}

	m_toolbar.SetMenu(IDB_BUTTON_TOOLBAR_CONNECT, m_popupProfile);

	InitProfilesTree();
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
		case WM_SETFOCUS: {
			//Why restore focus here? This window should never be able to get focus in the first place
			HWND hPrev = (HWND)wParam;
			if (hPrev != NULL)
				::SetFocus(hPrev);
			break; }
		case WM_ERASEBKGND: {
			HDC hDC = (HDC) wParam;
			RECT rectClient;
			GetClientRect(m_hwnd, &rectClient);
			FillRect(hDC, &rectClient, m_backgroundBrush);
			result = TRUE;
			break; }
		case WM_SETCURSOR: {
			if (m_splitter.OnSetCursor()) {
				return TRUE;
			}
			return FALSE;
			break; }
		case WM_CAPTURECHANGED: {
			m_splitter.OnCaptureChanged((HWND)lParam);
			break; }
		case WM_LBUTTONDOWN: {
			m_splitter.OnButtonDown();
			break; }
		case WM_LBUTTONUP: {
			if(m_splitter.OnButtonUp())
				m_ftpSettings->SetSplitRatio(m_splitter.GetRatio());
			else if (m_currentDragObject != NULL )	{
				// Get destination item.
				HTREEITEM htiDest = TreeView_GetDropHilight(m_treeview.GetHWND());
				ImageList_EndDrag();
				TreeView_SelectDropTarget(m_treeview.GetHWND(), NULL);
				ReleaseCapture();
				ShowCursor(TRUE);
				if (htiDest != NULL) {
					m_treeview.Focus();
					TreeView_SelectItem(m_treeview.GetHWND(), htiDest);
					FileObject* parent = m_treeview.GetItemFileObject(htiDest);
					if (!parent->isDir()) parent = parent->GetParent();
					bool bIsMove = ((GetKeyState(VK_CONTROL) & 0x8000) == 0);
					if(!OnProfileItemDrop(m_currentDragObject, parent, bIsMove))
						OnFileItemDrop(m_currentDragObject, parent, bIsMove);
					m_currentDragObject = NULL;

				} 
				m_currentDragObject = NULL;
				result = TRUE;
			}
			break; }
		case WM_MOUSEMOVE: {
			if ((wParam & MK_LBUTTON) && m_splitter.OnMouseMove()) break;
			if((wParam & MK_LBUTTON) || (wParam & MK_RBUTTON)) {

				HTREEITEM htiTarget;  // Handle to target item. 
				TVHITTESTINFO tvht;   // Hit test information. 

				if (m_currentDragObject != NULL )
				{
					if (m_currentDragObject->isDir() &&
						dynamic_cast<ProfileObject*>(m_currentDragObject) == NULL &&
						(GetKeyState(VK_CONTROL) & 0x8000) != 0) { // directories can't be copied
						ImageList_EndDrag();
						ReleaseCapture();
						ShowCursor(TRUE);
						result = FALSE;
						m_currentDragObject = NULL;
						break;
					}
					POINT point;
					point.x = GET_X_LPARAM(lParam);
					point.y = GET_Y_LPARAM(lParam);
					ClientToScreen(m_hwnd, &point);
					ScreenToClient(m_treeview.GetHWND(), &point);
					ImageList_DragMove(point.x, point.y);
					// Turn off the dragged image so the background can be refreshed.
					ImageList_DragShowNolock(FALSE);
					tvht.pt.x = point.x;
					tvht.pt.y = point.y;
					if ((htiTarget = TreeView_HitTest(m_treeview.GetHWND(), &tvht)) != NULL)
					{
						TreeView_SelectDropTarget(m_treeview.GetHWND(), htiTarget);
					}
					VScrollTreeView(point.y);
					ImageList_DragShowNolock(TRUE);


					result = TRUE;
				}
			}
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

                case IDB_BUTTON_TOOLBAR_OPENDIR: {

                    // Show the dialog to get input directory name from the user.
                    InputDialog id;
                    int res = id.Create(m_hwnd, TEXT("Open Directory"), TEXT("Enter directory name:"), TEXT(""));
                    if (res != 1) {
                        return 0;
                    }

                    // Read the input directory name.
                    const TCHAR *dirName    = id.GetValue();
                    char *dirNameCP         = SU::TCharToCP(dirName, CP_ACP);

                    m_ftpSession->GetDirectoryHierarchy(dirNameCP);
                    break;
                }

				case IDM_POPUP_PROFILE_CONNECT: {
					if (!m_busy && ConnectRemote(m_currentSelection) ){
						result = TRUE;
					}
					else {
						doDefaultProc = true;
					}
					break;
				}

				case IDM_POPUP_DOWNLOADFILE:
				case IDB_BUTTON_TOOLBAR_DOWNLOAD: {
					SHORT state = GetKeyState(VK_CONTROL);
					if (!(state & 0x8000)) {
						m_ftpSession->DownloadFileCache(m_currentSelection->GetPath());
						result = TRUE;
						break;
					}
					//else fallthrough
					}
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
					BOOL doUpload = FALSE;
					SHORT state = GetKeyState(VK_CONTROL);
					if ((state & 0x8000) && LOWORD(wParam) == IDB_BUTTON_TOOLBAR_UPLOAD) {
						source[0] = 0;
						int res = PU::GetOpenFilename(source, MAX_PATH, m_hParent);
						if (res == 0)
							doUpload = TRUE;
					} else {
						doUpload = ::SendMessage(m_hNpp, NPPM_GETFULLCURRENTPATH, (WPARAM)MAX_PATH, (LPARAM)source);
					}
					if (doUpload == TRUE) {
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
					m_outputShown = !m_outputWindow.IsVisible();
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
					HTREEITEM hti = (HTREEITEM)(m_currentSelection->GetData());
					TreeView_EditLabel(m_treeview.GetHWND(), hti);
					//this->Rename(m_currentSelection);
					result = TRUE;
					break; }
				case IDM_POPUP_SETTINGSGENERAL: {
					m_settingsDialog.Create(m_hwnd, m_ftpSettings);
					result = TRUE;
					break; }
				case IDM_POPUP_PROFILE_EDIT:
				case IDM_POPUP_SETTINGSPROFILE: {
					if(!EditProfile(m_currentSelection))
						m_profilesDialog.Create(m_hwnd, this, m_vProfiles, m_ftpSettings->GetGlobalCache());
					result = TRUE;
					break; }
				case IDM_POPUP_PROFILE_CREATE: {
					result= CreateProfile(m_currentSelection);
					break;	}
				case IDM_POPUP_PROFILE_DELETE: {
					result=DeleteProfile(m_currentSelection);
					break;	}
				case IDM_POPUP_CUT:
				case IDM_POPUP_COPY: {
					m_currentDragObject = m_currentSelection;
					m_currentDragObject->m_cutpaste = LOWORD(wParam);
					if (m_currentDragObject->m_cutpaste == IDM_POPUP_CUT) {
						HTREEITEM hti = (HTREEITEM)(m_currentSelection->GetData());
						if (hti != NULL)
							TreeView_SetItemState(m_treeview.GetHWND(), hti, TVIS_CUT, TVIS_CUT);
					}
					result = TRUE;
					break;	}
				case IDM_POPUP_PASTE: {
					if (m_currentDragObject == NULL ) {
						result = FALSE;
						break;
					}
					if(!(result=OnProfileItemDrop(m_currentDragObject, m_currentSelection, m_currentDragObject->m_cutpaste == IDM_POPUP_CUT)))
						result=OnFileItemDrop(m_currentDragObject, m_currentSelection, m_currentDragObject->m_cutpaste == IDM_POPUP_CUT);
					break; 	}
				case IDM_POPUP_PROFILE_FOLDER_CREATE: {
					result = CreateProfileFolder(m_currentSelection);
					break;	}											 
				default: {
					unsigned int value = LOWORD(wParam);
					if (!m_busy && value >= IDM_POPUP_PROFILE_FIRST && value <= IDM_POPUP_PROFILE_MAX) {
						FTPProfile * profile = m_vProfiles->at(value - IDM_POPUP_PROFILE_FIRST);
						int ret = m_ftpSession->StartSession(profile);
						if (ret == -1) {
							OutErr("[NppFTP] Cannot start FTP session");
							result = TRUE;
							break;
						}
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
								else {
									if(!(result=ConnectRemote(m_currentSelection)))
										result = m_toolbar.DoDropDown(IDB_BUTTON_TOOLBAR_CONNECT);
								}
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
				case TVN_KEYDOWN: {
					LPNMTVKEYDOWN ptvkd = (LPNMTVKEYDOWN)lParam;
					result = FALSE;
					if ((GetKeyState(VK_ESCAPE) & 0x8000) != 0) {
						if (m_currentDragObject != NULL) {
							TreeView_SetItemState(nmh.hwndFrom, (HTREEITEM)m_currentDragObject->GetData(), ~TVIS_CUT, TVIS_CUT);
							m_currentDragObject = NULL;
							ImageList_EndDrag();
							ReleaseCapture();
							ShowCursor(TRUE);
						}
						break;
					}
					if (m_currentSelection == NULL) break;
					if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
						if ( !(m_currentSelection != NULL && m_currentSelection->isRoot())  &&
							((GetKeyState(0x43) & 0x8000) != 0 || (GetKeyState(VK_INSERT) & 0x8000))) { //CTRL-C CTRL-INS
							m_currentDragObject = m_currentSelection;
							m_currentDragObject->m_cutpaste = IDM_POPUP_COPY;
							result = TRUE;
							break;
						}
						else if (!(m_currentSelection != NULL && m_currentSelection->isRoot()) && (GetKeyState(0x58) & 0x8000) != 0) { //CTRL-X
							m_currentDragObject = m_currentSelection;
							m_currentDragObject->m_cutpaste = IDM_POPUP_CUT;
							result = TRUE;
							break;
						}
						else if ((GetKeyState(0x56) & 0x8000) != 0) { //CTRL-V
							if(!OnProfileItemDrop(m_currentDragObject, m_currentSelection, m_currentDragObject->m_cutpaste == IDM_POPUP_CUT))
								OnFileItemDrop(m_currentDragObject, m_currentSelection, m_currentDragObject->m_cutpaste == IDM_POPUP_CUT);
							m_currentDragObject = NULL;
							result = TRUE;
							break;
						}
					}
					else if ((GetKeyState(VK_SHIFT) & 0x8000) != 0) {
						if ( (GetKeyState(VK_INSERT) & 0x8000)) { //SHIFT-INS
							if (m_currentDragObject != NULL ) {
								if(!OnProfileItemDrop(m_currentDragObject, m_currentSelection, m_currentDragObject->m_cutpaste == IDM_POPUP_CUT))
									OnFileItemDrop(m_currentDragObject, m_currentSelection, m_currentDragObject->m_cutpaste == IDM_POPUP_CUT);
								m_currentDragObject = NULL;
								result = TRUE;
								break;
							}
							break;
						}
						else if (!(m_currentSelection != NULL && m_currentSelection->isRoot()) &&  (GetKeyState(VK_DELETE) & 0x8000) != 0) { //SHIFT-DELETE
							m_currentDragObject = m_currentSelection;
							m_currentDragObject->m_cutpaste = IDM_POPUP_CUT;
							result = TRUE;
							break;
						}

					}

					switch (ptvkd->wVKey) {
						case VK_DELETE: {
							if (!(result = DeleteProfile(m_currentSelection)))
							{
								if (m_currentSelection->isDir())
									result=DeleteDirectory(m_currentSelection);
								else
									result = DeleteFile(m_currentSelection);
							}
							if (result >= 0) m_currentSelection = NULL;
							break;	}
					}
					break;	} 
					case TVN_ENDLABELEDIT: {
						LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
						TV_ITEM tvitem = (TV_ITEM)ptvdi->item;
						FileObject* current = m_treeview.GetItemFileObject(tvitem.hItem);
						if (tvitem.pszText != NULL) {
							if(!RenameProfileObject(current, tvitem.pszText))
								Rename(current, tvitem.pszText);
							result = TRUE;
						}
						else
							result = FALSE;
						break;	}
					case TVN_BEGINLABELEDIT: {
						LPNMTVDISPINFO ptvdi = (LPNMTVDISPINFO)lParam;
						TV_ITEM tvitem = (TV_ITEM)ptvdi->item;
						FileObject* current = m_treeview.GetItemFileObject(tvitem.hItem);
						if (!current || current->isRoot()) result = TRUE;
						break;	}
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
							m_currentSelection = m_treeview.GetItemFileObject(res);
							SetToolbarState();
							if (nmh.code == (UINT)NM_DBLCLK) {
								if(!OnProfileitemActivation(m_currentSelection))
									OnItemActivation();
								result = TRUE;
							}
						}
						break; }
					case NM_RELEASEDCAPTURE: {
						if (m_currentDragObject != NULL) {
							TreeView_SetItemState(m_treeview.GetHWND(), (HTREEITEM)m_currentDragObject->GetData(), ~TVIS_CUT, TVIS_CUT);
							m_currentDragObject = NULL;
							ImageList_EndDrag();
							ShowCursor(TRUE);
						}
						break; }
					case NM_RETURN: {
						if (!OnProfileitemActivation(m_currentSelection))
							OnItemActivation();
						result = TRUE;	//handle message
						break; }
					case TVN_SELCHANGING: {
						result = FALSE;
						break; }
					case TVN_BEGINDRAG: {
						HWND hwndTV = nmh.hwndFrom;
						LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
						HIMAGELIST himl;    // handle to image list 

						m_currentSelection=m_treeview.GetItemFileObject(lpnmtv->itemNew.hItem);
						if(m_currentDragObject != NULL && m_currentDragObject->isRoot()) break;
						if (m_currentSelection->isRoot()) break;
						TreeView_SelectItem(hwndTV, lpnmtv->itemNew.hItem);
						himl = TreeView_CreateDragImage(hwndTV, lpnmtv->itemNew.hItem);
						ImageList_BeginDrag(himl, 0, 0, 0);
						ImageList_DragEnter(hwndTV, lpnmtv->ptDrag.x, lpnmtv->ptDrag.x);

						ShowCursor(FALSE);
						m_currentDragObject = m_currentSelection;
						SetCapture(GetParent(hwndTV));
						result = TRUE;

						if((GetKeyState(VK_CONTROL) & 0x8000) == 0)
							TreeView_SetItemState(nmh.hwndFrom, lpnmtv->itemNew.hItem, TVIS_CUT, TVIS_CUT);
/*
						if (m_currentDropObject != NULL) {	//currently only one queued DnD op is supported
							result = FALSE;
							break;
						}
						NMTREEVIEW * pnmtv = (NMTREEVIEW*)lParam;
						HTREEITEM hti = pnmtv->itemNew.hItem;
						FileObject * fo = m_treeview.GetItemFileObject(hti);
						if (fo != NULL) {
							m_currentDropObject = fo;
							m_dndWindow.Create(m_hwnd);
							::PostMessage(m_dndWindow.GetHWND(), WM_DND, 0, 0);
							result = TRUE;
						}
*/
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
		case WM_CONTEXTMENU: {
			HWND hWinContext = (HWND)wParam;
			HMENU hContext = NULL;

			POINT menuPos;
			menuPos.x = GET_X_LPARAM(lParam);
			menuPos.y = GET_Y_LPARAM(lParam);
			bool fromKeyboard = (menuPos.x == -1 && menuPos.y == -1);
			if (fromKeyboard) {	//keyboard activation
				DWORD pos = GetMessagePos();
				menuPos.x = GET_X_LPARAM(pos);
				menuPos.y = GET_Y_LPARAM(pos);

			}

			if (hWinContext == m_treeview.GetHWND()) {
				if (!m_currentSelection) {
					result = FALSE;
					break;
				}

				if (fromKeyboard) {
					RECT treerect;
					bool res = m_treeview.GetObjectItemRect(m_currentSelection, &treerect);
					if (res) {
						menuPos.x = treerect.left;
						menuPos.y = treerect.bottom;
						::ClientToScreen(m_treeview.GetHWND(), &menuPos);
					}
				}

				SHORT state = GetKeyState(VK_SHIFT);
				if ((state & 0x8000) && m_currentSelection->isLink() && !fromKeyboard) {
					hContext = m_popupLink;
				}
				else {

					if ((hContext = SetProfilePopupMenu(m_currentSelection)) == NULL) {
						if (m_currentSelection->isDir()) {
							if(m_currentSelection->isRoot())
								hContext = m_popupRootDir;
							else
								hContext = m_popupDir;
						}
						else {
							hContext = m_popupFile;
						}
					}
				}

			} else if (hWinContext == m_queueWindow.GetHWND()) {
				QueueOperation * op = m_queueWindow.GetSelectedQueueOperation();
				if (!op) {
					result = FALSE;
					break;
				}

				m_cancelOperation = op;
				bool run = op->GetRunning();

				if (fromKeyboard) {
					RECT queuerect;
					bool res = m_queueWindow.GetSelectedQueueRect(&queuerect);
					if (res) {
						menuPos.x = queuerect.left;
						menuPos.y = queuerect.bottom;
						::ClientToScreen(m_queueWindow.GetHWND(), &menuPos);
					}
				}

				if (run) {
					hContext = m_popupQueueActive;
				} else {
					hContext = m_popupQueueHold;
				}
			}

			if (!hContext) {
				result = FALSE;
				break;
			}

			::TrackPopupMenu(hContext, TPM_LEFTALIGN, menuPos.x, menuPos.y, 0, m_hwnd, NULL);
			result = TRUE;
			break; }
		case  WM_INITMENUPOPUP: {
			HMENU hMenu = (HMENU)wParam;
			if ( m_currentDragObject != NULL) 
				EnableMenuItem(hMenu, IDM_POPUP_PASTE, MF_BYCOMMAND| MF_ENABLED);
			else
				EnableMenuItem(hMenu, IDM_POPUP_PASTE, MF_BYCOMMAND | MF_GRAYED);
			result = TRUE;
			break;   }
		case WM_OUTPUTSHOWN: {
			if (wParam == TRUE) {
				m_outputShown = true;
				m_toolbar.SetChecked(IDB_BUTTON_TOOLBAR_MESSAGES, TRUE);
				m_ftpSettings->SetOutputShown(true);
			} else {
				m_outputShown = false;
				m_toolbar.SetChecked(IDB_BUTTON_TOOLBAR_MESSAGES, FALSE);
				m_ftpSettings->SetOutputShown(false);
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

int FTPWindow::OnFileItemDrop(FileObject* item, FileObject* parent, bool bIsMove)
{
	if (item == NULL || parent == NULL) return false;

	int ret = -1;
	if (parent->GetChildByName(item->GetName()) != NULL) {
		MessageDialog md;

		int res = md.Create(m_hwnd, TEXT("Target item exists"), TEXT("Are you sure you want to overwrite existing item?"));
		if (res != 1) {
			TreeView_SetItemState(m_treeview.GetHWND(), (HTREEITEM)m_currentDragObject->GetData(), ~TVIS_CUT, TVIS_CUT);
			m_currentDragObject = NULL;
			return FALSE;
		}

	}

	if (bIsMove)
		ret = Move(item, parent);
	else
		ret = Copy(item, parent);

	if(ret) m_currentDragObject = NULL;
	return ret;

}

bool FTPWindow::AcceptType(LPDATAOBJECT pDataObj) {
	FORMATETC fmtetc;

	fmtetc.ptd	    = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex   = -1;
	fmtetc.tymed	= TYMED_HGLOBAL;
	fmtetc.cfFormat = CF_HDROP;

	if (pDataObj->QueryGetData(&fmtetc) == NOERROR)
		return true;

	return false;
}

HRESULT FTPWindow::OnDragEnter(LPDATAOBJECT /*pDataObj*/, DWORD /*grfKeyState*/, POINTL /*pt*/, LPDWORD /*pdwEffect*/) {
	return S_OK;
}

HRESULT FTPWindow::OnDragOver(DWORD /*grfKeyState*/, POINTL pt, LPDWORD pdwEffect) {
	FileObject * fo = m_treeview.GetItemByPoint(pt);

	if (fo && fo->isDir()) {
		*pdwEffect = DROPEFFECT_COPY;
		TreeView_Select(m_treeview.GetHWND(), fo->GetData(), TVGN_DROPHILITE);
		m_currentDropObject = fo;
		return S_OK;
	} else {
		TreeView_Select(m_treeview.GetHWND(), NULL, TVGN_DROPHILITE);
		m_currentDropObject = NULL;
	}

	return S_OK;
}

HRESULT FTPWindow::OnDragLeave() {
	TreeView_Select(m_treeview.GetHWND(), NULL, TVGN_DROPHILITE);
	return S_OK;
}

HRESULT FTPWindow::OnDrop(LPDATAOBJECT pDataObj, DWORD /*grfKeyState*/, POINTL /*pt*/, LPDWORD pdwEffect) {
	TreeView_Select(m_treeview.GetHWND(), NULL, TVGN_DROPHILITE);

	STGMEDIUM medium;
	FORMATETC formatetc;
	formatetc.cfFormat = CF_HDROP;
	formatetc.tymed = TYMED_HGLOBAL;
	formatetc.dwAspect = 0;
	formatetc.lindex = -1;
	formatetc.ptd = NULL;;

	HRESULT dataRes = pDataObj->GetData(&formatetc, &medium);
	if (dataRes == S_OK) {
		*pdwEffect = DROPEFFECT_COPY;
		HDROP hdrop = (HDROP)GlobalLock(medium.hGlobal);

		TCHAR pathToFile[MAX_PATH];
		int filesDropped = DragQueryFile(hdrop, 0xffffffff, NULL, 0);
		for (int i = 0; i < filesDropped; ++i) {
			if (DragQueryFile(hdrop, i, pathToFile, MAX_PATH) != 0) {
				//pathToFile is not checked. If it doesnt exist or its a directory or link, CreateFile either allows a handle to be opened or not
				m_ftpSession->UploadFile(pathToFile, m_currentDropObject->GetPath(), true, 1);	//1: User specified location
			}
		}


		GlobalUnlock(medium.hGlobal);
		GlobalFree(medium.hGlobal);
	}

	return S_OK;
}

int FTPWindow::GetNrFiles() {
	if (!m_currentDropObject)
		return -1;

	return 1;
}

int FTPWindow::GetFileDescriptor(FILEDESCRIPTOR * fd, int index) {
	if (!m_currentDropObject)
		return -1;
	if (index != 0)
		return -1;

#define FD_PROGRESSUI 0x00004000

	fd->dwFlags = FD_ATTRIBUTES | FD_CREATETIME | FD_ACCESSTIME | FD_WRITESTIME | FD_FILESIZE | FD_PROGRESSUI;
	fd->nFileSizeLow = m_currentDropObject->GetSize();
	fd->nFileSizeHigh = 0;
	fd->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	fd->ftCreationTime = m_currentDropObject->GetCTime();
	fd->ftLastAccessTime = m_currentDropObject->GetATime();
	fd->ftLastWriteTime = m_currentDropObject->GetMTime();
	lstrcpyn(fd->cFileName, m_currentDropObject->GetLocalName(), MAX_PATH);

	return 0;
}

int FTPWindow::StreamData(CStreamData * stream, int index) {
	if (!m_currentDropObject)
		return -1;
	if (index != 0)
		return -1;

	HANDLE hWriteHandle = stream->GetWriteHandle();
	int dldRes = m_ftpSession->DownloadFileHandle(m_currentDropObject->GetPath(), hWriteHandle);
	if (dldRes == -1)
		return -1;

	return 0;
}

int FTPWindow::OnEndDnD() {
	if (!m_currentDropObject)
		return -1;

	m_currentDropObject = 0;

	return 0;
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
	AppendMenu(m_popupFile,MF_SEPARATOR,0,0);
	AppendMenu(m_popupFile, MF_STRING, IDM_POPUP_COPY, TEXT("&Copy File"));
	AppendMenu(m_popupFile, MF_STRING, IDM_POPUP_CUT, TEXT("&Cut File"));
	//AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_PERMISSIONFILE,TEXT("Permissions"));
	//AppendMenu(m_popupFile,MF_STRING,IDM_POPUP_PROPSFILE,TEXT("&Properties"));

	
	//Create context menu for directories in folder window
	m_popupDir = CreatePopupMenu();
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_NEWDIR,TEXT("Create new &directory"));
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_NEWFILE,TEXT("&Create new file"));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_RENAMEDIR,TEXT("&Rename Directory"));
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_DELETEDIR,TEXT("D&elete directory"));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
    AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_UPLOADFILE,TEXT("&Upload current file here"));
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_UPLOADOTHERFILE,TEXT("Upload &other file here..."));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
	AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_REFRESHDIR,TEXT("Re&fresh"));
	AppendMenu(m_popupDir,MF_SEPARATOR,0,0);
	AppendMenu(m_popupDir, MF_STRING, IDM_POPUP_CUT, TEXT("&Cut Folder"));
	AppendMenu(m_popupDir, MF_STRING | MF_GRAYED, IDM_POPUP_PASTE, TEXT("&Paste"));
	//AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_PERMISSIONDIR,TEXT("Permissions"));
	//AppendMenu(m_popupDir,MF_STRING,IDM_POPUP_PROPSDIR,TEXT("&Properties"));

	//Create context menu for directories in folder window
	m_popupRootDir = CreatePopupMenu();
	AppendMenu(m_popupRootDir, MF_STRING, IDM_POPUP_NEWDIR, TEXT("Create new &directory"));
	AppendMenu(m_popupRootDir, MF_STRING, IDM_POPUP_NEWFILE, TEXT("&Create new file"));
	AppendMenu(m_popupRootDir, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupRootDir, MF_STRING, IDM_POPUP_UPLOADFILE, TEXT("&Upload current file here"));
	AppendMenu(m_popupRootDir, MF_STRING, IDM_POPUP_UPLOADOTHERFILE, TEXT("Upload &other file here..."));
	AppendMenu(m_popupRootDir, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupRootDir, MF_STRING, IDM_POPUP_REFRESHDIR, TEXT("Re&fresh"));
	AppendMenu(m_popupRootDir, MF_SEPARATOR, 0, 0);
	AppendMenu(m_popupRootDir, MF_STRING | MF_GRAYED, IDM_POPUP_PASTE, TEXT("&Paste"));


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

	ProfilesWindow::CreateMenus();

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
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_OPENDIR, false);
	} else {
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_DOWNLOAD, !m_currentSelection->isDir());
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_UPLOAD, m_localFileExists);	//m_currentSelection->isDir());
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_REFRESH, m_currentSelection->isDir());
		m_toolbar.Enable(IDB_BUTTON_TOOLBAR_OPENDIR, true);
	}

	return 0;
}

int FTPWindow::OnEvent(QueueOperation * queueOp, int code, void * data, bool isStart) {
	int result = 0;
	void * queueData = queueOp->GetData();
	int queueResult = queueOp->GetResult();

	//Set busy parameter
	switch(queueOp->GetType()) {
		case QueueOperation::QueueTypeDownload:
		case QueueOperation::QueueTypeDownloadHandle:
		case QueueOperation::QueueTypeCopyFile:
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

            std::vector<FTPDir*> parentDirObjs = dirop->GetParentDirObjs();
            size_t i;

            for (i=0; i<parentDirObjs.size(); i++) {
                FTPDir* curFTPDir = parentDirObjs[i];

                FileObject* parent;
                parent = m_ftpSession->FindPathObject(curFTPDir->dirPath);
                if (parent)
                    OnDirectoryRefresh(parent, curFTPDir->files, curFTPDir->count);
            }

			if (queueResult == -1) {
				OutErr("Failure retrieving contents of directory %s", dirop->GetDirPath());
				//break commented: even if failed, update the treeview etc., count should result in 0 anyway
				//break;	//failure
			}
			FTPFile* files = (FTPFile*)queueData;
			int count = dirop->GetFileCount();
			FileObject* parent = m_ftpSession->FindPathObject(dirop->GetDirPath());
			if (parent)
				OnDirectoryRefresh(parent, files, count);
			break; }
		case QueueOperation::QueueTypeDownloadHandle:
		case QueueOperation::QueueTypeDownload: {
			QueueDownload * opdld = (QueueDownload*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Download of %s failed", opdld->GetExternalPath());
				OnError(queueOp, code, data, isStart);
				break;	//failure
			}

			if (queueOp->GetType() == QueueOperation::QueueTypeDownload) {
				if (code == 0) {
					//Download to cache: Open file
					OutMsg("Download of %s succeeded, opening file.", opdld->GetExternalPath());
					::SendMessage(m_hNpp, NPPM_DOOPEN, (WPARAM)0, (LPARAM)opdld->GetLocalPath());
				} else {
					//Download to other location: Ask
					int ret = ::MessageBox(m_hNpp, TEXT("The download is complete. Do you wish to open the file?"), TEXT("Download complete"), MB_YESNO);
					if (ret == IDYES) {
						::SendMessage(m_hNpp, NPPM_DOOPEN, (WPARAM)0, (LPARAM)opdld->GetLocalPath());
					}
				}
			} else {
				OutMsg("Download of %s succeeded.", opdld->GetExternalPath());
			}
			break; }
		case QueueOperation::QueueTypeUpload: {
			QueueUpload * opuld = (QueueUpload*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Upload of %S failed", opuld->GetLocalPath());
				OnError(queueOp, code, data, isStart);
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
			QueueQuote* opquote = (QueueQuote*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to perform quote operation %s", opquote->GetQuote());
				break;	//failure
			}
			break; }
		case QueueOperation::QueueTypeCopyFile: {
			QueueCopyFile* opcopy = (QueueCopyFile*)queueOp;
			if (isStart)
				break;
			if (queueResult == -1) {
				OutErr("Unable to perform copy operation %s", opcopy->GetExternalPath());
				break;	//failure
			}
			m_ftpSession->GetDirectory(opcopy->GetExternalNewParent());
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
	m_treeview.UpdateFileObject(parent);
	m_treeview.FillTreeDirectory(parent);
	m_treeview.ExpandDirectory(parent);

	return 0;
}

int FTPWindow::OnError(QueueOperation * /*queueOp*/, int /*code*/, void * /*data*/, bool /*isStart*/) {
	::MessageBeep(MB_ICONERROR);
	if (!IsVisible())
		Show(true);
	if (!m_outputShown)
		m_outputWindow.Show(true);

	m_outputWindow.ScrollLastLine();

	return 0;
}

int FTPWindow::OnItemActivation() {
	if (!m_currentSelection)
		return -1;

	if (m_currentSelection->isDir()) {
		if (m_treeview.CollapseDirectory(m_currentSelection) != 0)
			m_treeview.ExpandDirectory(m_currentSelection);
	} else {
		m_ftpSession->DownloadFileCache(m_currentSelection->GetPath());
	}
	return 0;
}

int FTPWindow::OnConnect(int code) {
	m_currentSelection = NULL;
	m_currentDragObject = NULL;
	m_currentDropObject = NULL;

	m_treeview.ClearAll();

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

	TCHAR * info = SU::TSprintfNB(TEXT("Connected to %T"), m_ftpSession->GetCurrentProfile()->GetName());
	SetInfo(info);
	delete [] info;

	SetToolbarState();

	return 0;
}

int FTPWindow::OnDisconnect(int /*code*/) {
	m_currentSelection = NULL;
	m_currentDragObject = NULL;
	m_currentDropObject = NULL;

	m_treeview.ClearAll();

	if (m_ftpSettings->GetClearCache()) {
		m_ftpSession->GetCurrentProfile()->GetCache()->ClearCurrentCache( m_ftpSettings->GetClearCachePermanent() );
	}

	SetInfo(TEXT("Disconnected"));

	InitProfilesTree();
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

    // Check if there is already an existing file of the same name
    int childcount = parent->GetChildCount();
    char *newFileName_CP = SU::TCharToCP(newName, CP_ACP);

	for(int i = 0; i < childcount; i++) {

	    const char *currentFileName = parent->GetChild(i)->GetName();
		if (!strcmp(currentFileName, newFileName_CP)) {

            res = ::MessageBox(m_hwnd, TEXT("A file/directory by the same name already exists. Do you want to create a new blank file ?"), TEXT("Creating file"), MB_YESNO);
            if (res == IDNO) {
                return 0;
            }
            else {
                break; 
            }
        }
	}

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

int FTPWindow::Rename(FileObject* fo, const TCHAR* newName) {

	char path[MAX_PATH];
	int	res = PU::ConcatLocalToExternal(fo->GetParent()->GetPath(), newName, path, MAX_PATH);

	if (res == -1)
		return -1;

	res = m_ftpSession->Rename(fo->GetPath(), path);
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(fo->GetParent()->GetPath());

	return 0;
}

int FTPWindow::Move(FileObject* fo, FileObject* _newParent) {

	if (fo->isRoot()) {
		OutErr("Can't move root");
		return -1;
	}


	char path[MAX_PATH];
	FileObject* currentParent = fo->GetParent();

	int	res = PU::ConcatLocalToExternal(_newParent->GetPath(), SU::Utf8ToTChar(fo->GetName()), path, MAX_PATH);
	if (res == -1)
		return -1;

	res = m_ftpSession->Rename(fo->GetPath(), path);
	if (res == -1)
		return -1;

	m_ftpSession->GetDirectory(currentParent->GetPath());
	m_ftpSession->GetDirectory(_newParent->GetPath());

	return 0;
}

int FTPWindow::Copy(FileObject* fo, FileObject* _newParent) {
	if (fo->isDir()) {
		OutErr("Can't copy directories");
		return -1;
	}

	int	res = m_ftpSession->CopyFile(fo->GetPath(),_newParent->GetPath());

	if (res == -1)
		return -1;

	return 0;

}

int FTPWindow::VScrollTreeView(LONG yPos)
{
	static LONG scrollregion = TreeView_GetItemHeight(m_treeview.GetHWND()) * 4;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_POS | SIF_RANGE;
	GetScrollInfo(m_treeview.GetHWND(), SB_VERT, &si);
	if (si.nPos == 0 && si.nMax == 0 && si.nMin == 0) return -1;


	RECT rect;
	if (GetWindowRect(m_treeview.GetHWND(), &rect))
	{
		int height = rect.bottom - rect.top;
		if (height <= 2 * scrollregion) return -1;

		if (height - yPos <= scrollregion && si.nPos < si.nMax) 
				SendMessage(m_treeview.GetHWND(), WM_VSCROLL, 1, 0);
		else if(yPos <= scrollregion && si.nPos > si.nMin)
			SendMessage(m_treeview.GetHWND(), WM_VSCROLL, 0, 0);
	} 


	return 0;
}
