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

#ifndef PROFILEOBJECT_H
#define PROFILEOBJECT_H

#include "FTPFile.h"
#include <unordered_map>
#include "FTPProfile.h"
#include "FTPSettings.h"

class ProfileObject;

typedef std::vector<ProfileObject*> POVector;

class ProfileObject : public FileObject {
public:
	ProfileObject(const char* path, bool isDir, bool isLink);
	ProfileObject(const char* path, const char* name, bool _isDir,FTPProfile* profile=NULL);
	ProfileObject(FTPFile* ftpfile);
	virtual					~ProfileObject();

	virtual int				GetChildCount() const;
	virtual ProfileObject*	GetChild(int index) const;
	virtual ProfileObject*	GetChildByName(const char* filename);
	virtual ProfileObject*  GetChildByPath(const char* fullpath);
	virtual int				AddChild(ProfileObject* child);
	virtual ProfileObject*	AddChild(const char* path, const char* name, FTPProfile* profile=NULL);
	virtual int				RemoveChild(ProfileObject* child, bool del = true);
	virtual int				RemoveAllChildren(bool del = true);
	virtual int				MoveTo(ProfileObject* newParent);
	virtual ProfileObject*	CopyTo(ProfileObject* parent, FTPSettings* ftpSettings,bool& bSameName);

	virtual int				SetParent(ProfileObject* parent);
	virtual ProfileObject*	GetParent();
	virtual ProfileObject*	GetFirstProfile();

	virtual int				SetDir(bool isDir);

	virtual bool			isLink() const;
	virtual bool			isDir() const;
	virtual bool			isRoot() const;
	virtual bool			containsProfile() const;

	virtual const char*		GetName() const;
	virtual int				SetName(const char* newName);
	virtual FTPProfile*		GetProfile() const;
	virtual const TCHAR*	GetLocalName() const;
	virtual const char*		GetPath() const;
	virtual const char*		GetMod() const;

	virtual bool			UpdatePath();
	virtual void			SetProfiles(vProfile* profiles);
	virtual int				FindProfileIndex(FTPProfile* profile);
	virtual int				SetRefresh(bool refresh);
	virtual bool			GetRefresh() const;

	virtual int				SetData(void* data);
	virtual void*			GetData() const;

	virtual long			GetSize() const;

	virtual FILETIME		GetCTime() const;
	virtual FILETIME		GetMTime() const;
	virtual FILETIME		GetATime() const;

	virtual int				Sort();

	static int				SortVector(POVector& foVect);
	static bool				m_debug;
	WORD					m_cutpaste;
protected:
	static bool				ComparePO(const ProfileObject* d1, const ProfileObject* d2);

	FTPProfile* m_profile;
	bool					m_isDir;
	bool					m_isLink;
	bool					m_isRoot;

	int						m_childCount;
	POVector				m_children;
	ProfileObject*			m_parent;

	char*					m_path;
	char*					m_name;
	TCHAR*					m_localName;

	bool					m_needRefresh;
	void* m_data;

	long					m_size;

	FILETIME				m_ctime;
	FILETIME				m_mtime;
	FILETIME				m_atime;

	ProfileObject*			CreateParentPath(const char* path);
	ProfileObject*			CreateParentPath(std::vector<std::string>& parts);

	static vProfile*		m_vProfiles;
private:
	using FileObject::SetParent; //avoid compiler warning about hidden method

	char*					MkPath(const char* first, const char* second) const;
	static std::unordered_map<std::string, ProfileObject*>	treeMap;

};

#endif //PROFILEOBJECT_H
