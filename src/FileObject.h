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

#ifndef FILEOBJECT_H
#define FILEOBJECT_H

#include "FTPFile.h"

class FileObject;

typedef std::vector<FileObject*> FOVector;

class FileObject {
public:
							FileObject(const char* path, bool isDir, bool isLink);
							FileObject(FTPFile * ftpfile);
							FileObject();
	virtual					~FileObject();

	virtual int				GetChildCount() const;
	virtual FileObject*		GetChild(int index) const;
	virtual FileObject*		GetChildByName(const char *filename);
	virtual int				AddChild(FileObject * child);
	virtual int				RemoveChild(FileObject * child, bool del = true);
	virtual int				RemoveAllChildren(bool del = true);

	virtual int				SetParent(FileObject * parent);
	virtual FileObject*		GetParent();

	virtual int				SetDir(bool isDir);

	virtual bool			isLink() const;
	virtual bool			isDir() const;
	virtual bool			isRoot() const;
	virtual bool			containsProfile() const;

	virtual const char*		GetName() const;
	virtual const TCHAR*	GetLocalName() const;
	virtual const char*		GetPath() const;
	virtual const char*		GetMod() const;

	virtual int				SetRefresh(bool refresh);
	virtual bool			GetRefresh() const;

	virtual int				SetData(void * data);
	virtual void *			GetData() const;

	virtual long			GetSize() const;

	virtual FILETIME		GetCTime() const;
	virtual FILETIME		GetMTime() const;
	virtual FILETIME		GetATime() const;

	virtual int				Sort();

	static int				SortVector(FOVector & foVect);
	WORD					m_cutpaste;
protected:
	static bool				CompareFO(const FileObject* d1, const FileObject* d2);

	bool					m_isDir;
	bool					m_isLink;

	int						m_childCount;
	FOVector				m_children;
	FileObject*				m_parent;

	char*					m_path;
	const char*				m_name;
	TCHAR*					m_localName;

	bool					m_needRefresh;
	void*					m_data;

	long					m_size;

	FILETIME				m_ctime;
	FILETIME				m_mtime;
	FILETIME				m_atime;

	char*					m_mod;
};

#endif //FILEOBJECT_H
