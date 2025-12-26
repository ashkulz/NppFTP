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
#include "ProfileObject.h"
#include <sstream>

#include <algorithm>

std::unordered_map<std::string, ProfileObject*> ProfileObject::treeMap = {};
vProfile*  ProfileObject::m_vProfiles = NULL;
bool ProfileObject::m_debug = FALSE;

ProfileObject::ProfileObject(const char* path, bool _isDir, bool _isLink) :
	m_profile(NULL),
	m_isDir(_isDir),
	m_isLink(_isLink),
	m_isRoot(false),
	m_childCount(0),
	m_children(),
	m_parent(NULL),
	m_needRefresh(!_isDir),	//refresh only required for dirs
	m_data(NULL),
	m_size(-1)
{
	size_t len = strlen(path) + 1;
	m_path = new char[len];
	strcpy(m_path, path);

	m_name = strrchr(m_path, '/');
	if (m_name[1] != 0)		//root directory case
	{
		m_name++;			//skip slash
	}

	m_localName = SU::Utf8ToTChar(m_name);

	FILETIME ft;
	SYSTEMTIME st;

	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &ft);

	m_ctime = ft;
	m_mtime = ft;
	m_atime = ft;
}

ProfileObject::ProfileObject(const char* path, const char* name, bool _isDir, FTPProfile* profile) :
	m_profile(profile),
	m_isDir(_isDir),
	m_isLink(false),
	m_isRoot(false),
	m_childCount(0),
	m_children(),
	m_parent(NULL),
	m_needRefresh(!_isDir),	//refresh only required for dirs
	m_data(NULL),
	m_size(-1)
{


	if (strcmp(path, "/") == 0) {
		ProfileObject::treeMap.clear();
		ProfileObject::treeMap.insert({ "",this });
		m_path = SU::strdup("");
		m_isRoot = TRUE;
	}
	else {
		m_path = (char *)MkPath(path, name);
	}
	m_name = SU::strdup(name);

	m_localName = SU::Utf8ToTChar(m_name);

	FILETIME ft;
	SYSTEMTIME st;

	::GetSystemTime(&st);
	::SystemTimeToFileTime(&st, &ft);

	m_ctime = ft;
	m_mtime = ft;
	m_atime = ft;
}

ProfileObject::ProfileObject(FTPFile* ftpfile) :
	m_profile(NULL),
	m_childCount(0),
	m_children(),
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

ProfileObject::~ProfileObject() {
	RemoveAllChildren(false);
	SU::free(m_path);
	SU::FreeTChar(m_localName);
}

int ProfileObject::GetChildCount() const {
	return m_childCount;
}

ProfileObject* ProfileObject::GetChild(int index) const {
	return m_children.at(index);
}

int ProfileObject::AddChild(ProfileObject* child) {
	m_children.push_back(child);
	child->SetParent(this);
	m_childCount = (int)m_children.size();

	return 0;
}

ProfileObject* ProfileObject::CreateParentPath(const char* path) {
	std::stringstream sstream(path);
	std::string s;
	std::vector<std::string> parts;
	while (std::getline(sstream, s, '/')) {
		if (s.size() > 0)
			parts.push_back(s);
	}
	if (parts.size() == 0) return NULL;

	return CreateParentPath(parts);
}

ProfileObject* ProfileObject::CreateParentPath(std::vector<std::string>& parts)
{
	if (parts.size() == 0) return NULL;
	std::string name = parts.front();
	parts.erase(parts.begin());

	ProfileObject* current = GetChildByName(name.c_str());
	if (current == NULL) {
		current = new ProfileObject(m_path, name.c_str(), true);
		AddChild(current);
	}

	if (parts.size() == 0)
		return current;
	else
		return current->CreateParentPath(parts);
}

char* ProfileObject::MkPath(const char* first, const char* second) const
{
	size_t lgFirst = strlen(first);
	size_t lgSecond = strlen(second);

	if (lgFirst == 0) return SU::strdup(second);
	if (lgSecond == 0) return SU::strdup(first);

	char* ret = new char[lgFirst + lgSecond + strlen("/") + 1];
	strcpy(ret, first);
	strcat(ret, "/");
	strcat(ret, second);
	return ret;

}

ProfileObject* ProfileObject::AddChild(const char* path, const char* name, FTPProfile* profile) {

	const char* absPath = MkPath(m_path, path);

	ProfileObject* parent = NULL;
	if (path == NULL || strlen(path) == 0) {
		parent = this;
	}
	else {
		auto  parentIterator = ProfileObject::treeMap.find(absPath);
		if (parentIterator == ProfileObject::treeMap.end())
		{
			parent = CreateParentPath(path);
			ProfileObject::treeMap.insert({ absPath,parent });
		}
		else
			parent = ProfileObject::treeMap[absPath];
	}

	if (strlen(name) == 0) {
		parent->SetRefresh(FALSE);
		if (profile != NULL) {
			parent->m_profile = profile;
		} else if (parent->m_profile == NULL) {
			FTPProfile* dummy = new FTPProfile(TEXT("")); 
			dummy->SetParent(SU::Utf8ToTChar(parent->GetPath()));
			dummy->AddRef();
			m_vProfiles->push_back(dummy);
			parent->m_profile = dummy;
		}		
		return parent;
	}

	ProfileObject* child = new ProfileObject(absPath, name, false, profile);

	if (parent->GetProfile() != NULL) {
		size_t i = (size_t)-1;
		if((i = FindProfileIndex(parent->GetProfile())) >= 0) {
			m_vProfiles->erase(m_vProfiles->begin() + i);
		}
		parent->m_profile->Release();
		parent->m_profile = NULL;
	}

	parent->AddChild(child);

	return child;
}

int ProfileObject::RemoveChild(ProfileObject* child, bool del) {
	POVector::iterator it;

	for (it = m_children.begin(); it < m_children.end(); ++it) {
		if (*it == child) {
			if (del && child->GetChildCount() > 0) child->RemoveAllChildren(TRUE);
			if (child->GetProfile() != NULL) {
				int i = -1;
				if((i=FindProfileIndex(child->GetProfile())) >= 0) {
					if (GetChildCount() == 1) { //add dummy profile to folder
						FTPProfile* dummy = new FTPProfile(TEXT(""));
						dummy->SetParent(SU::Utf8ToTChar(GetPath()));
						dummy->AddRef();
						if (del)
							m_vProfiles->at(i) = dummy;
						else
							m_vProfiles->push_back(dummy);
						m_profile = dummy;
					}
					else if (del) {
						m_vProfiles->erase(m_vProfiles->begin() + i);
					}
				}
				if (del) child->m_profile->Release();
			}
			m_children.erase(it);
			m_childCount =(int) m_children.size();
			return m_childCount;
		}
	}

	return -1;
}

int ProfileObject::RemoveAllChildren(bool del) {
	POVector::iterator it;

	POVector temp_children = POVector(m_children);

	for (it = temp_children.begin(); it < temp_children.end(); ++it) {
		RemoveChild(*it, true);
	}

	m_children.clear();
	m_childCount = 0;

	return 0;
}

int ProfileObject::MoveTo(ProfileObject* newParent)
{
	if (newParent == GetParent()) return 0;

	GetParent()->RemoveChild(this, false);
	newParent->AddChild(this); 
	if (isDir()) UpdatePath();
	return 0;
}

ProfileObject* ProfileObject::CopyTo(ProfileObject* parent, FTPSettings* ftpSettings, bool& bSameName)
{
	POVector::iterator it;
	ProfileObject* newProfileObject;

	char* newname = SU::strdup(m_name);
	bSameName = FALSE;
	while (parent->GetChildByName(newname) != NULL) {
		char* tempname = newname;
		size_t lg = strlen(newname) + strlen("Copy of ") + 1;
		newname = new char[lg];
		sprintf(newname,"Copy of %s", tempname);
		SU::FreeChar(tempname);
		bSameName = TRUE;
	}
	if (isDir()) { 
		newProfileObject = parent->AddChild(newname, "");
	}
	else {
		FTPProfile* newProfile = NULL;
		if (GetProfile() != NULL) {
			newProfile = new FTPProfile(SU::Utf8ToTChar(newname), GetProfile());
			newProfile->SetCacheParent(ftpSettings->GetGlobalCache());
			newProfile->SetParent(SU::Utf8ToTChar(parent->GetPath()));
			m_vProfiles->push_back(newProfile);
			newProfile->AddRef();
		}
		newProfileObject = parent->AddChild("", newname, newProfile);
	}

	bool bDummy = FALSE;
	for (it = m_children.begin(); it < m_children.end(); ++it) {
		(*it)->CopyTo(newProfileObject, ftpSettings,bDummy);
	}


	return newProfileObject;

}

int ProfileObject::SetParent(ProfileObject* parent) {
	m_parent = parent;
	m_path = (char*)MkPath(parent->GetPath(), m_name);
	if (m_profile != NULL) {
		m_profile->SetParent(SU::Utf8ToTChar(parent->GetPath()));
	}
	return 0;
}

ProfileObject* ProfileObject::GetParent() {
	return m_parent;
}

int ProfileObject::SetDir(bool bisDir) {
	if (isDir() == bisDir)
		return 0;

	if (!bisDir && GetChildCount() > 0)	//proven to be directory
		return -1;

	m_isDir = bisDir;

	return 0;
}

bool ProfileObject::isLink() const {
	return m_isLink;
}

bool ProfileObject::isDir() const {
	return m_isDir;
}

bool ProfileObject::isRoot() const
{
	return m_isRoot;
}

bool ProfileObject::containsProfile() const
{
	return true;
}

const char* ProfileObject::GetName() const {
	size_t l = strlen(m_name) + strlen(".mlc") + 1;
	char* name = new char[l];
	strcpy(name, m_name);
	strcat(name, ".mlc");
	//strcpy(name, ".nfo");
	return name;

}

int ProfileObject::SetName(const char* newName)  {
	if (strcmp(m_path, "") == 0) return -1; //Root cannot be renamed

	m_name = SU::strdup(newName);
	m_path = (char*)MkPath(GetParent()->GetPath(), m_name);
	m_localName = SU::Utf8ToTChar(m_name);


	if (m_profile != NULL && !isDir()) {
		m_profile->SetName(SU::Utf8ToTChar(newName));
	}

	if (isDir()) {
		POVector::iterator it;

		if (m_profile != NULL) {
			m_profile->SetParent(SU::Utf8ToTChar(m_path));
			m_profile->SetName(TEXT(""));
		}
		for (it = m_children.begin(); it < m_children.end(); ++it) {
			(*it)->UpdatePath();
		}
	}
	
	return 0;
}

FTPProfile* ProfileObject::GetProfile() const
{
	return m_profile;
}

const TCHAR* ProfileObject::GetLocalName() const {
	return m_localName;
}

const char* ProfileObject::GetPath() const {
	return m_path;
}

const char* ProfileObject::GetMod() const
{
	return nullptr;
}

bool ProfileObject::UpdatePath()
{
	m_path = (char *)MkPath(GetParent()->GetPath(), m_name);

	if (m_profile != NULL) {
		if(isDir())
			m_profile->SetParent(SU::Utf8ToTChar(m_path));
		else
			m_profile->SetParent(SU::Utf8ToTChar(GetParent()->GetPath()));
	}

	if (isDir()) {
		POVector::iterator it;
		for (it = m_children.begin(); it < m_children.end(); ++it) {
			(*it)->UpdatePath();
		}
	}

	return TRUE;
}

void ProfileObject::SetProfiles(vProfile* profiles)
{
	m_vProfiles = profiles;
}

int ProfileObject::FindProfileIndex(FTPProfile* profile)
{
	if (m_vProfiles == NULL) return  -1;

	for (size_t i = 0; i < m_vProfiles->size(); i++) {
		if (m_vProfiles->at(i) == profile) {
			return (int)i;
		}
	}

	return -1; 
}

int ProfileObject::SetRefresh(bool refresh) {
	m_needRefresh = refresh;
	return 0;
}

bool ProfileObject::GetRefresh() const {
	return m_needRefresh;
}

int ProfileObject::SetData(void* data) {
	m_data = data;
	return 0;
}

void* ProfileObject::GetData() const {
	return m_data;
}

long ProfileObject::GetSize() const {
	return m_size;
}

ProfileObject* ProfileObject::GetFirstProfile() {
	if (GetChildCount() == 0) return NULL;

	if (GetChild(0)->isDir())
		return GetChild(0)->GetFirstProfile();
	else
		return GetChild(0);

}

FILETIME ProfileObject::GetCTime() const {
	return m_ctime;
}

FILETIME ProfileObject::GetMTime() const {
	return m_mtime;
}

FILETIME ProfileObject::GetATime() const {
	return m_atime;
}


int ProfileObject::Sort() {
	return ProfileObject::SortVector(m_children);
}

int ProfileObject::SortVector(POVector& foVect) {
	std::sort(foVect.begin(), foVect.end(), &ProfileObject::ComparePO);

	return 0;
}

bool ProfileObject::ComparePO(const ProfileObject* fo1, const ProfileObject* fo2) {
	int res = 0;
	if (fo1->isDir() != fo2->isDir()) {
		res = fo1->isDir() ? -1 : 1;
	}
	else {
		res = lstrcmpiA(fo1->GetPath(), fo2->GetPath());
	}
	return (res < 0);
}

ProfileObject* ProfileObject::GetChildByName(const char* filename) {

	int i;
	int count = GetChildCount();
	if (count == 0)
		return NULL;

	for (i = 0; i < count; i++) {
		if (strcmp(GetChild(i)->m_name, filename) == 0) {
			return GetChild(i);
		}
	}

	return NULL;
}

ProfileObject* ProfileObject::GetChildByPath(const char* fullpath) {
	int i;
	int count = GetChildCount();
	if (count == 0)	return NULL;

	ProfileObject* child = NULL;
	ProfileObject* subChild = NULL;
	for (i = 0; i < count; i++) {
		child = GetChild(i);
		if (!strcmp(child->GetPath(), fullpath))	return child;

		if (child->isDir()) {
			subChild = child->GetChildByPath(fullpath);
			if (subChild != NULL) return subChild;
		}
	}
	return NULL;


}

