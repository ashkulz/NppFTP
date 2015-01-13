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

#ifndef TREEVIEW_H
#define TREEVIEW_H

#include "Window.h"
#include <commctrl.h>

#ifndef RBN_CHEVRONPUSHED
#define RBN_CHEVRONPUSHED (RBN_FIRST - 10)
#endif

class Treeview;
class TreeImageList;
class TreeFile;

class Treeview : public Window {
	friend class TreeImageList;
public:
							Treeview();
	virtual					~Treeview();

	virtual int				Create(HWND hParent);

	virtual HTREEITEM		AddRoot(FileObject * rootDir);
	virtual HTREEITEM		AddFileObject(HTREEITEM root, FileObject * file);

	virtual int				FillTreeDirectory(FileObject * dir);
	virtual int				RemoveAllChildItems(HTREEITEM parent);

	virtual int				GetDispInfo(TV_DISPINFO* ptvdi);
	virtual HTREEITEM		OnClick();
	virtual int				OnExpanding(const NM_TREEVIEW* nmt);	//return true when refresh is required

	virtual FileObject*		GetItemFileObject(HTREEITEM item);
	virtual bool			GetObjectItemRect(FileObject * fo, RECT * pRect);
	virtual FileObject*		GetItemByPoint(POINTL pt);

	virtual int				UpdateFileObject(FileObject * fo);
	virtual int				UpdateDirectory(FileObject * dir);
	virtual int				ExpandDirectory(FileObject * dir);
	virtual int				EnsureObjectVisible(FileObject * fo);

	virtual int				ClearAll();
protected:
	virtual int				ClearObjectDataRecursive(FileObject * fo, bool includeTop);

	virtual int				RedrawItem(HTREEITEM item);

	TreeImageList*			m_treeImagelist;
	HTREEITEM               curSelectedItem;
};

class TreeImageList {
public:
							TreeImageList(HINSTANCE hInst);
	virtual					~TreeImageList();

	virtual int				SetTreeview(Treeview * tree);

	virtual int				SetFancyIcon(bool useFancy);

	virtual int				GetIconIndex(const FileObject* fo, bool isSelected);
protected:
	virtual int				Initialize();
	virtual int				Deinitialize();

	HINSTANCE				m_hInst;

	bool					m_initialized;
	bool					m_useFancy;

	Treeview*				m_treeview;

	HIMAGELIST				m_hImageListTreeviewSimple;
	HIMAGELIST				m_hImageListTreeviewFancy;
	HIMAGELIST				m_hImageListTreeviewCurrent;	//either of the two above

	int						m_iconDirClosedFancy;
	int						m_iconDirClosedLinkFancy;
	int						m_iconDirOpenFancy;
	int						m_iconDirOpenLinkFancy;
};

#endif //TREEVIEW_H
