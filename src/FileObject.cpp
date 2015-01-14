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
//#include "FileObject.h"

#include <algorithm>


FileObject::FileObject(const char* path, bool _isDir, bool _isLink) :
	m_isDir(_isDir),
	m_isLink(_isLink),
	m_childCount(0),
	m_parent(NULL),
	m_needRefresh(_isDir),	//refresh only required for dirs
	m_data(NULL),
	m_size(-1)
{
	size_t len = strlen(path)+1;
	m_path = new char[len];
	strcpy(m_path, path);

	m_name = strrchr(m_path, '/');
	if (m_name[1] != 0)		//root directory case
		m_name++;			//skip slash

	m_localName = SU::Utf8ToTChar(m_name);

	FILETIME ft;
	SYSTEMTIME st;

	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &ft);

	m_ctime = ft;
	m_mtime = ft;
	m_atime = ft;
}

FileObject::FileObject(FTPFile * ftpfile) :
	m_childCount(0),
	m_parent(NULL),
	m_data(NULL)
{
	m_isDir = (ftpfile->fileType != FTPTypeFile);
	m_isLink = (ftpfile->fileType == FTPTypeLink);

	m_needRefresh = m_isDir;	//refresh only required for dirs

	m_path = SU::strdup(ftpfile->filePath);

	m_name = strrchr(m_path, '/');
	if (m_name[1] != 0)		//root directory case
		m_name++;			//skip slash

	m_localName = SU::Utf8ToTChar(m_name);

	m_size = ftpfile->fileSize;

	m_ctime = ftpfile->ctime;
	m_mtime = ftpfile->mtime;
	m_atime = ftpfile->atime;
}

FileObject::~FileObject() {
	RemoveAllChildren();
	SU::free(m_path);
	SU::FreeTChar(m_localName);
}

int FileObject::GetChildCount() const {
	return m_childCount;
}

FileObject* FileObject::GetChild(int index) const {
	return m_children.at(index);
}

int FileObject::AddChild(FileObject * child) {
	m_children.push_back(child);
	child->SetParent(this);
	m_childCount = m_children.size();
	return 0;
}

int FileObject::RemoveChild(FileObject * child, bool del) {
	FOVector::iterator it;

	for(it = m_children.begin(); it < m_children.end(); it++) {
		if (*it == child) {
			if (del)
				delete child;
			m_children.erase(it);
			m_childCount = m_children.size();
			return 0;
		}
	}

	return -1;
}

int FileObject::RemoveAllChildren(bool del) {
	FOVector::iterator it;

	if (del) {
		for(it = m_children.begin(); it < m_children.end(); it++) {
			delete *it;
		}
	}

	m_children.clear();
	m_childCount = 0;

	return 0;
}

int FileObject::SetParent(FileObject * parent) {
	m_parent = parent;
	return 0;
}

FileObject* FileObject::GetParent() {
	return m_parent;
}

int FileObject::SetDir(bool bisDir) {
	if (isDir() == bisDir)
		return 0;

	if (!bisDir && GetChildCount() > 0)	//proven to be directory
		return -1;

	m_isDir = bisDir;

	return 0;
}

bool FileObject::isLink() const {
	return m_isLink;
}

bool FileObject::isDir() const {
	return m_isDir;
}

const char* FileObject::GetName() const {
	return m_name;
}

const TCHAR* FileObject::GetLocalName() const {
	return m_localName;
}

const char* FileObject::GetPath() const {
	return m_path;
}

int FileObject::SetRefresh(bool refresh) {
	m_needRefresh = refresh;
	return 0;
}

bool FileObject::GetRefresh() const {
	return m_needRefresh;
}

int FileObject::SetData(void * data) {
	m_data = data;
	return 0;
}

void * FileObject::GetData() const {
	return m_data;
}

long FileObject::GetSize() const {
	return m_size;
}

FILETIME FileObject::GetCTime() const {
	return m_ctime;
}

FILETIME FileObject::GetMTime() const {
	return m_mtime;
}

FILETIME FileObject::GetATime() const {
	return m_atime;
}


int FileObject::Sort() {
	return FileObject::SortVector(m_children);
}

int FileObject::SortVector(FOVector & foVect) {
	std::sort(foVect.begin(), foVect.end(), &FileObject::CompareFO);

	return 0;
}

bool FileObject::CompareFO(const FileObject * fo1, const FileObject * fo2) {
	int res = 0;
	if (fo1->isDir() != fo2->isDir()) {
		res = fo1->isDir()?false:true;
	} else {
		res = lstrcmpiA(fo1->GetPath(), fo2->GetPath());
	}
	return (res <= 0);
}

FileObject* FileObject::GetChildByName(const char *filename) {

    int i;
    int count = GetChildCount();
    if (count == 0)
        return NULL;

	for(i = 0; i < count; i++) {
		if ( !strcmp( GetChild(i)->GetName(), filename ) ) {
			return GetChild(i);
		}
	}

	return NULL;
}
