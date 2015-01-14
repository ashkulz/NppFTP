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
#include "Treeview.h"

/* Tree view extended styles */
#if 1
    #define TVS_EX_MULTISELECT          0x00000002L
    #define TVS_EX_DOUBLEBUFFER         0x00000004L
    #define TVS_EX_NOINDENTSTATE        0x00000008L
    #define TVS_EX_RICHTOOLTIP          0x00000010L
    #define TVS_EX_AUTOHSCROLL          0x00000020L
    #define TVS_EX_FADEINOUTEXPANDOS    0x00000040L
    #define TVS_EX_PARTIALCHECKBOXES    0x00000080L
    #define TVS_EX_EXCLUSIONCHECKBOXES  0x00000100L
    #define TVS_EX_DIMMEDCHECKBOXES     0x00000200L
    #define TVS_EX_DRAWIMAGEASYNC       0x00000400L

    #define TVM_SETEXTENDEDSTYLE		(TV_FIRST + 44)

#endif


#include "resource.h"

Treeview::Treeview() :
	Window(NULL, WC_TREEVIEW),
	m_treeImagelist(NULL)
{
	m_exStyle = WS_EX_CLIENTEDGE;
	m_style = WS_CHILD|/*WS_VISIBLE|*/WS_BORDER|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_LINESATROOT|TVS_HASLINES;

	INITCOMMONCONTROLSEX icx;
	icx.dwSize = sizeof(icx);
	icx.dwICC = ICC_TREEVIEW_CLASSES;
	InitCommonControlsEx(&icx);

    curSelectedItem = NULL;
}

Treeview::~Treeview() {
}

int Treeview::Create(HWND hParent) {
	int res = Window::Create(hParent);
	if (res == -1)
		return -1;

	HRESULT hres = PF::SetWindowTheme(m_hwnd, L"explorer", NULL);
	if (hres != E_NOTIMPL) {
		::SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_CHILD|/*WS_VISIBLE|*/WS_BORDER|TVS_HASBUTTONS|TVS_SHOWSELALWAYS|TVS_LINESATROOT|TVS_TRACKSELECT);
		SendMessage(m_hwnd, TVM_SETEXTENDEDSTYLE, 0, TVS_EX_FADEINOUTEXPANDOS|TVS_EX_DOUBLEBUFFER|TVS_EX_AUTOHSCROLL);
	}

	return 0;
}

HTREEITEM Treeview::AddRoot(FileObject * rootDir) {
	TV_INSERTSTRUCT tvinsert;
	tvinsert.hParent = TVI_ROOT;
	tvinsert.hInsertAfter = TVI_LAST;
	tvinsert.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|TVIF_CHILDREN;
	tvinsert.item.iImage = I_IMAGECALLBACK;
	tvinsert.item.iSelectedImage = I_IMAGECALLBACK;
	tvinsert.item.cChildren = I_CHILDRENCALLBACK;
	tvinsert.item.lParam = (LPARAM)rootDir;
	tvinsert.item.pszText = (TCHAR*)rootDir->GetLocalName();

	HTREEITEM hti = TreeView_InsertItem(m_hwnd, &tvinsert);

	rootDir->SetData(hti);

	return hti;
}

HTREEITEM Treeview::AddFileObject(HTREEITEM root, FileObject * file) {
	TV_INSERTSTRUCT tvinsert;

	tvinsert.hParent = root;
	tvinsert.hInsertAfter = TVI_LAST;
	tvinsert.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM|(file->isDir()?TVIF_CHILDREN:0);
	tvinsert.item.iImage = I_IMAGECALLBACK;
	tvinsert.item.iSelectedImage = I_IMAGECALLBACK;
	tvinsert.item.cChildren = I_CHILDRENCALLBACK;
	tvinsert.item.lParam = (LPARAM)file;
	tvinsert.item.pszText = (TCHAR*)file->GetLocalName();

	HTREEITEM hti = TreeView_InsertItem(m_hwnd, &tvinsert);

	file->SetData(hti);

	return hti;
}

int Treeview::FillTreeDirectory(FileObject * dir) {
	HTREEITEM hti = (HTREEITEM)dir->GetData();
	if (hti == NULL)
		return -1;

	int childcount = dir->GetChildCount();

	RemoveAllChildItems(hti);

	for(int i = 0; i < childcount; i++) {
		if (dir->GetChild(i)->isDir())
			AddFileObject(hti, dir->GetChild(i));
	}
	for(int i = 0; i < childcount; i++) {
		if (!dir->GetChild(i)->isDir())
			AddFileObject(hti, dir->GetChild(i));
	}

	UpdateFileObject(dir);

	return childcount;
}

int Treeview::RemoveAllChildItems(HTREEITEM parent) {
	HTREEITEM child;
	while( (child = TreeView_GetChild(m_hwnd, parent)) != NULL) {
		TreeView_DeleteItem(m_hwnd, child);
	}

	return 0;
}

int Treeview::GetDispInfo(TV_DISPINFO* ptvdi) {
	if (!ptvdi)
		return -1;

	FileObject * fo = (FileObject*) (ptvdi->item.lParam);
	if (!fo)
		return -1;

	if (ptvdi->item.mask & TVIF_CHILDREN) {
		if ( fo->isDir() && (fo->GetRefresh() || fo->GetChildCount() > 0) ) {
			ptvdi->item.cChildren = 1;
		} else {
			ptvdi->item.cChildren = 0;
		}
	}

	if (ptvdi->item.mask & TVIF_IMAGE) {	//manually give icon
		int index = m_treeImagelist->GetIconIndex(fo, false);
		if (index != -1)
			ptvdi->item.iImage = index;
		else	//error
			ptvdi->item.iImage = 0;
	}

	if (ptvdi->item.mask & TVIF_SELECTEDIMAGE) {	//manually give icon
		int index = m_treeImagelist->GetIconIndex(fo, true);
		if (index != -1)
			ptvdi->item.iSelectedImage = index;
		else	//error
			ptvdi->item.iSelectedImage = 0;
	}

	return TRUE;
}

HTREEITEM Treeview::OnClick() {
	HTREEITEM selected = NULL;
	DWORD dPos = GetMessagePos();			//get current mouse pos
	POINTS pts = MAKEPOINTS(dPos);
	POINT pos = {pts.x, pts.y};
	ScreenToClient(m_hwnd, &pos);

	TV_HITTESTINFO ht;
	ht.pt = pos;
	selected = TreeView_HitTest(m_hwnd, &ht);

	if (selected != NULL && (ht.flags & TVHT_ONITEM)) {
		TreeView_Select(m_hwnd, selected, TVGN_CARET);
		return selected;
	}

	return NULL;
}

int Treeview::OnExpanding(const NM_TREEVIEW* nmt) {
	const TV_ITEM & tvi = nmt->itemNew;
	FileObject * fo = (FileObject*) tvi.lParam;
	int result = 0;

	switch(nmt->action) {
		case TVE_EXPAND: {
			if (fo->GetRefresh()) {
				result = TRUE;		//refresh this directory
			} else {
				FillTreeDirectory(fo);
				result = FALSE;		//let the tree expand, it has been filled
			}
			break; }
		case TVE_COLLAPSE: {
			ClearObjectDataRecursive(fo, false);
			TreeView_Expand(m_hwnd, tvi.hItem, TVE_COLLAPSE | TVE_COLLAPSERESET);
			result = FALSE;
			break; }
		default: {
			result = FALSE;
			break; }
	}

	return result;
}

FileObject* Treeview::GetItemFileObject(HTREEITEM item) {
	TV_ITEM tvi;
	tvi.hItem = item;
	tvi.mask = TVIF_PARAM;
	tvi.lParam = 0;

	SendMessage(m_hwnd, TVM_GETITEM, 0, (LPARAM)&tvi);
	FileObject * fo = (FileObject*) tvi.lParam;

	return fo;
}

bool Treeview::GetObjectItemRect(FileObject * fo, RECT * pRect) {
	HTREEITEM hti = (HTREEITEM)(fo->GetData());
	if (hti == NULL)
		return false;

	BOOL res = TreeView_GetItemRect(m_hwnd, hti, pRect, TRUE);
	return (res == TRUE);
}

FileObject* Treeview::GetItemByPoint(POINTL pt) {
	TV_HITTESTINFO ht;
	ht.pt.x = pt.x;
	ht.pt.y = pt.y;
	::ScreenToClient(m_hwnd, &ht.pt);
	HTREEITEM result = TreeView_HitTest(m_hwnd, &ht);
	if (result && (ht.flags & TVHT_ONITEM)) {
		return GetItemFileObject(result);
	}

	return NULL;
}

int Treeview::UpdateFileObject(FileObject * fo) {
	void* dat = fo->GetData();
	if (!dat)
		return 0;

	HTREEITEM hti = (HTREEITEM)dat;

	TV_ITEM tvi;
	tvi.hItem = hti;
	tvi.mask = TVIF_IMAGE|TVIF_SELECTEDIMAGE|(fo->isDir()?TVIF_CHILDREN:0);
	tvi.iImage = I_IMAGECALLBACK;
	tvi.iSelectedImage = I_IMAGECALLBACK;
	tvi.cChildren = I_CHILDRENCALLBACK;

	SendMessage(m_hwnd, TVM_SETITEM, 0, (LPARAM)&tvi);
	RedrawItem(hti);

	return 0;
}

int Treeview::UpdateDirectory(FileObject * dir) {
	void* dat = dir->GetData();
	if (!dat)
		return 0;

	HTREEITEM hti = (HTREEITEM)dat;

	TV_ITEM tvi;
	tvi.hItem = hti;
	tvi.mask = TVIF_STATE;
	tvi.stateMask = TVIS_EXPANDED;
	BOOL res = TreeView_GetItem(m_hwnd, &tvi);
	if (res == FALSE)
		return -1;

	bool expanded = (tvi.state != 0);
	if (expanded) {
		RemoveAllChildItems(hti);
		FillTreeDirectory(dir);
		TreeView_Expand(m_hwnd, hti, TVE_EXPAND);
	}
	return 0;
}

int Treeview::ExpandDirectory(FileObject * dir) {
	void* dat = dir->GetData();
	if (!dat)
		return 0;

	HTREEITEM hti = (HTREEITEM)dat;
	RECT rc = {0,0,0,0};
	RECT *rcPtr = &rc;
	BOOL visible = TreeView_GetItemRect(m_hwnd, hti, rcPtr, FALSE);

	if (visible == TRUE) {
		TreeView_Expand(m_hwnd, hti, TVE_EXPAND);
	}

	if (curSelectedItem)
        TreeView_SetItemState(m_hwnd, curSelectedItem, 0, TVIS_SELECTED);

	TreeView_SetItemState(m_hwnd, hti, TVIS_SELECTED, TVIS_SELECTED);
    curSelectedItem = hti;
	return 0;
}

int Treeview::EnsureObjectVisible(FileObject * fo) {
	HTREEITEM hti = (HTREEITEM)(fo->GetData());

	if (hti == NULL && fo->GetParent() == fo)
		return -1;	//root invisble, error!

	if(hti == NULL) {
		int res = EnsureObjectVisible(fo->GetParent());
		if (res == -1)
			return -1;

		FillTreeDirectory(fo->GetParent());
	}

	hti = (HTREEITEM)(fo->GetData());

	TreeView_EnsureVisible(m_hwnd, hti);
	return 0;
}

int Treeview::ClearAll() {
	HTREEITEM root = TreeView_GetRoot(m_hwnd);
	if (root == NULL)
		return 0;

	FileObject * fo = GetItemFileObject(root);
	ClearObjectDataRecursive(fo, true);

	TreeView_DeleteItem(m_hwnd, root);

	return 0;
}

int Treeview::ClearObjectDataRecursive(FileObject * fo, bool includeTop) {
	if (!fo)
		return -1;

	if (includeTop) {
		fo->SetData(NULL);
	}
	size_t count = fo->GetChildCount();
	for(size_t i = 0; i < count; i++) {
		ClearObjectDataRecursive(fo->GetChild(i), true);
	}

	return 0;
}

int Treeview::RedrawItem(HTREEITEM item) {
	if (!item)
		return -1;

	RECT rc;
	RECT *rcPtr = &rc;
	BOOL visible = TreeView_GetItemRect(m_hwnd, item, rcPtr, TRUE);	//TRUE: get rect of entire row
	if (visible == TRUE) {
		//::RedrawWindow(m_hwnd, &rc, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW);
		::RedrawWindow(m_hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

TreeImageList::TreeImageList(HINSTANCE hInst) :
	m_hInst(hInst),
	m_initialized(false),
	m_treeview(NULL),
	m_hImageListTreeviewSimple(NULL),
	m_hImageListTreeviewFancy(NULL),
	m_hImageListTreeviewCurrent(NULL)
{
	Initialize();

	m_useFancy = true;
	SetFancyIcon(false);
}

TreeImageList::~TreeImageList() {
	Deinitialize();
}

int TreeImageList::SetTreeview(Treeview * tree) {
	if (m_treeview)
		return -1;

	m_treeview = tree;
	m_treeview->m_treeImagelist = this;

	//Set the imagelist
	HWND hTreeview = m_treeview->GetHWND();
	SendMessage(hTreeview, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)m_hImageListTreeviewCurrent);

	return 0;
}

int TreeImageList::Initialize() {
	if (m_initialized)
		return 0;

	//Load the simple imagelist
	HBITMAP hBitmap = LoadBitmap(m_hInst,(LPCTSTR)MAKEINTRESOURCE(IDB_BITMAP_TREEICONS));
	HBITMAP hBitmapMask = LoadBitmap(m_hInst,(LPCTSTR)MAKEINTRESOURCE(IDB_BITMAP_TREEICONS_MASK));

	m_hImageListTreeviewSimple = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 8, 2);
	if (m_hImageListTreeviewSimple == NULL)
		return -1;

	ImageList_Add(m_hImageListTreeviewSimple, hBitmap, hBitmapMask);

	DeleteObject(hBitmap);
	DeleteObject(hBitmapMask);

	//Load the fancy imagelist
	SHFILEINFO shfi;
	m_hImageListTreeviewFancy = (HIMAGELIST)SHGetFileInfo(TEXT(""), FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	ImageList_SetBkColor(m_hImageListTreeviewFancy, CLR_NONE);	//makes icons transparent, this may conflict with the imagelist of other plugins

	if (m_hImageListTreeviewFancy == NULL)
		return -1;

	//Cache fancy directory icons
	SHGetFileInfo(TEXT("C:\\Test"), FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);
	m_iconDirClosedFancy = shfi.iIcon;
	SHGetFileInfo(TEXT("C:\\Test"), FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_OPENICON);
	m_iconDirOpenFancy = shfi.iIcon;
	SHGetFileInfo(TEXT("C:\\Test"), FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_LINKOVERLAY);
	m_iconDirClosedLinkFancy = shfi.iIcon;
	SHGetFileInfo(TEXT("C:\\Test"), FILE_ATTRIBUTE_DIRECTORY, &shfi, sizeof(SHFILEINFO), SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_OPENICON|SHGFI_LINKOVERLAY);
	m_iconDirOpenLinkFancy = shfi.iIcon;

	m_initialized = true;

	return 0;
}

int TreeImageList::Deinitialize() {
	if (!m_initialized)
		return 0;

	m_initialized = false;

	ImageList_Destroy(m_hImageListTreeviewSimple);
	m_hImageListTreeviewSimple = NULL;

	return 0;
}

int TreeImageList::SetFancyIcon(bool useFancy) {
	if (useFancy == m_useFancy)
		return 0;

	m_useFancy = useFancy;

	if (useFancy) {
		m_hImageListTreeviewCurrent = m_hImageListTreeviewFancy;
	} else {
		m_hImageListTreeviewCurrent = m_hImageListTreeviewSimple;
	}

	//Set the imagelist
	if (m_treeview) {
		HWND hTreeview = m_treeview->GetHWND();
		SendMessage(hTreeview, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)m_hImageListTreeviewCurrent);
	}

	return 0;
}

int TreeImageList::GetIconIndex(const FileObject* fo, bool isSelected) {
	int index= -1;

	if (m_useFancy) {
		if (fo->isDir()) {
			if (fo->isLink()) {
				if (isSelected) {
					index = m_iconDirOpenLinkFancy;
				} else {
					index = m_iconDirClosedLinkFancy;
				}
			} else {
				if (isSelected) {
					index = m_iconDirOpenFancy;
				} else {
					index = m_iconDirClosedFancy;
				}
			}
		} else {
			SHFILEINFO shfi;
			ZeroMemory(&shfi, sizeof(SHFILEINFO));

			int flags = (SHGFI_SMALLICON|SHGFI_SYSICONINDEX|SHGFI_USEFILEATTRIBUTES);
			if (fo->isLink())
				flags |= SHGFI_LINKOVERLAY;
			if (isSelected)
				flags |= SHGFI_SELECTED;

			TCHAR * wName = SU::Utf8ToTChar(fo->GetName());
			SHGetFileInfo(wName, FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(SHFILEINFO), flags);
			SU::FreeTChar(wName);
			index = shfi.iIcon;
		}
	} else {
		if (fo->isDir()) {
			index = 0;
		} else {
			index = 2;
		}
		if (fo->isLink()) {
			index += 4;
		}
		if (isSelected) {
			index += 1;
		}
	}

	return index;
}
