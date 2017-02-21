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
#include "FTPCache.h"

#include <algorithm>

const char * FTPCache::CacheElem = "Cache";
const int PathCacheSize = MAX_PATH+10;	//paths generally do not exceed MAX_PATH, but if it happens often buffer reallocation may need some profiling (e.g. set ceiling instead of just allocating)

FTPCache::FTPCache() :
	m_cacheParent(NULL)
{
	m_activeHost = SU::DupString(TEXT(""));
	m_activeUser = SU::DupString(TEXT(""));
}

FTPCache::~FTPCache() {
	Clear();
	m_cacheParent = NULL;
}

int FTPCache::SetCacheParent(FTPCache * cacheParent) {
	m_cacheParent = cacheParent;

	return 0;
}

int FTPCache::SetEnvironment(const char * host, const char * user) {
	SU::FreeTChar(m_activeHost);
	SU::FreeTChar(m_activeUser);

	m_activeHost = SU::Utf8ToTChar(host);
	m_activeUser = SU::Utf8ToTChar(user);

	ExpandPaths();

	return 0;
}

const PathMap & FTPCache::GetPathMap(int i) const {
	return m_vCachePaths.at(i);
}

int FTPCache::GetPathMapCount() const {
	return (int)m_vCachePaths.size();
}

int FTPCache::DeletePathMap(int i) {
	if (i < 0 || (size_t)i >= m_vCachePaths.size())
		return -1;

	SU::free(m_vCachePaths[i].externalpath);	//strdup
	SU::FreeTChar(m_vCachePaths[i].localpath);	//StringConversion

	m_vCachePaths.erase(m_vCachePaths.begin()+i);
	return 0;
}

int FTPCache::AddPathMap(PathMap pathmap) {
	TCHAR * expPath = ExpandPath(pathmap.localpath);
	if (expPath != NULL) {
		pathmap.localpathExpanded = expPath;
	}
	m_vCachePaths.push_back(pathmap);
	return 0;
}

int FTPCache::SetPathMap(PathMap pathmap, int i) {
	if (i < 0 || (size_t)i >= m_vCachePaths.size())
		return -1;

	SU::free(m_vCachePaths[i].externalpath);	//strdup
	SU::FreeTChar(m_vCachePaths[i].localpath);	//StringCOnversion

	TCHAR * expPath = ExpandPath(pathmap.localpath);
	if (expPath != NULL) {
		pathmap.localpathExpanded = expPath;
	}
	m_vCachePaths[i] = pathmap;
	return 0;
}

int FTPCache::SwapPathMap(int indexa, int indexb) {
	if (indexa < 0 || indexb < 0)
		return -1;

	if ((size_t)indexa > m_vCachePaths.size() || (size_t)indexb > m_vCachePaths.size())
		return -1;

	if (indexa == indexb)
		return 0;

	std::swap(m_vCachePaths[indexa], m_vCachePaths[indexb]);

	return 0;
}

int FTPCache::Clear() {
	for(size_t i = 0; i < m_vCachePaths.size(); i++) {
		SU::free(m_vCachePaths[i].externalpath);	//strdup
		SU::FreeTChar(m_vCachePaths[i].localpath);	//StringCOnversion
	}
	m_vCachePaths.clear();
	return 0;
}

int FTPCache::GetExternalPathFromLocal(const TCHAR * localpath, char * extbuf, int extsize) const {
	TCHAR expanded[MAX_PATH];
	BOOL res = PathSearchAndQualify(localpath, expanded, MAX_PATH);
	if (res == FALSE) {
		return -1;
	}

	for (size_t i = 0; i < m_vCachePaths.size(); i++) {
		const TCHAR * postfix = NULL;
		if ( IsPathPrefixLocal(expanded, m_vCachePaths[i].localpathExpanded, &postfix) ) {
			//get actual path using postfix
			return PU::ConcatLocalToExternal(m_vCachePaths[i].externalpath, postfix, extbuf, extsize);
		}
	}

	if (!m_cacheParent)
		return 1;

	return m_cacheParent->GetExternalPathFromLocal(localpath, extbuf, extsize);
}

int FTPCache::GetLocalPathFromExternal(const char * externalpath, TCHAR * localbuf, int localsize) const {
	for (size_t i = 0; i < m_vCachePaths.size(); i++) {
		const char * postfix = NULL;
		if ( IsPathPrefixExternal(externalpath, m_vCachePaths[i].externalpath, &postfix) ) {
			//get actual path using postfix
			return PU::ConcatExternalToLocal(m_vCachePaths[i].localpathExpanded, postfix, localbuf, localsize);
		}
	}

	if (!m_cacheParent)
		return 1;

	return m_cacheParent->GetLocalPathFromExternal(externalpath, localbuf, localsize);
}

int FTPCache::ClearCurrentCache(bool permanent) {
	SHFILEOPSTRUCT shfop;
	TCHAR * dirPath = new TCHAR[MAX_PATH+1];

	ZeroMemory(&shfop, sizeof(shfop));
	shfop.hwnd = _MainOutputWindow;
	shfop.wFunc = FO_DELETE;
	shfop.pFrom = dirPath;
	shfop.pTo = NULL;
	shfop.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;// | FOF_WANTNUKEWARNING;
	if (!permanent) {
		shfop.fFlags |= FOF_ALLOWUNDO;	//use recycle bin
	}
	shfop.lpszProgressTitle = NULL;



	for(size_t i = 0; i < m_vCachePaths.size(); i++) {
		OutMsg("[Cache] Clearing cache in '%T'", m_vCachePaths[i].localpathExpanded);
		lstrcpy(dirPath, m_vCachePaths[i].localpathExpanded);
		int len = lstrlen(dirPath);
		dirPath[len+1] = 0;
		int res = SHFileOperation(&shfop);
		if (res != 0) {
			//Error may also be triggered if cache is empty, so stay silent
			//OutErr("Failure clearing cache: %d", res);
		}
	}

	delete [] dirPath;

	if (m_cacheParent)
		return m_cacheParent->ClearCurrentCache(permanent);

	return 0;
}

TiXmlElement* FTPCache::SaveCache(const FTPCache * cache) {
	TiXmlElement * cacheElem = new TiXmlElement(FTPCache::CacheElem);

	for(size_t i = 0; i < cache->m_vCachePaths.size(); i++) {
		TiXmlElement * child = new TiXmlElement("Location");

		char * utf8local = SU::TCharToUtf8(cache->m_vCachePaths[i].localpath);
		child->SetAttribute("localpath", utf8local);
		SU::FreeChar(utf8local);
		child->SetAttribute("externalpath", cache->m_vCachePaths[i].externalpath);

		cacheElem->LinkEndChild(child);
	}

	return cacheElem;
}

FTPCache* FTPCache::LoadCache(const TiXmlElement * cacheElem) {
	if (!cacheElem)
		return NULL;

	if ( strcmp(FTPCache::CacheElem, cacheElem->Value()) )
		return NULL;

	FTPCache * cache = new FTPCache();

	const char * localpathstr = NULL;
	const char * externalpathstr = NULL;
	const TiXmlElement* child = cacheElem->FirstChildElement("Location");
	for( ; child; child = child->NextSiblingElement("Location") )
	{
		localpathstr = child->Attribute("localpath");
		if (!localpathstr)
			continue;
		TCHAR * localpath = SU::Utf8ToTChar(localpathstr);
		if (!localpath)
			continue;

		externalpathstr = child->Attribute("externalpath");
		if (!externalpathstr)
			continue;

		cache->AddPathMap(localpath, externalpathstr);
		SU::FreeTChar(localpath);
	}

	return cache;
}

int FTPCache::AddPathMap(const TCHAR * localpath, const char * externalpath) {
	if (localpath == 0 || externalpath == 0)
		return -1;

	PathMap map;
	map.externalpath = SU::strdup(externalpath);
	map.localpath = SU::DupString(localpath);

	TCHAR * expPath = ExpandPath(map.localpath);
	if (expPath == NULL) {
		SU::FreeTChar(map.localpath);
		return -1;
	}
	map.localpathExpanded = expPath;

	m_vCachePaths.push_back(map);
	return 0;
}

bool FTPCache::IsPathPrefixLocal(const TCHAR * localpath, const TCHAR * prefix, const TCHAR ** postfix) const {

	int commonsize = PathCommonPrefix(localpath, prefix, NULL);
	int prefixsize = lstrlen(prefix);

	if (commonsize != prefixsize)
		return false;

	*postfix = localpath+prefixsize;
	return true;

/*
	while(*prefix != 0) {
		if (*localpath == 0)
			return false;

		if (*localpath != *prefix)
			return false;

		localpath++;
		prefix++;
	}

	*postfix = localpath;

	return true;
*/
}

bool FTPCache::IsPathPrefixExternal(const char * localpath, const char * prefix, const char ** postfix) const {
	
	// Some system don't start with / (hello z/OS)
	if (localpath[0] != '/') {
		prefix++;
		*postfix = localpath;
		return true;
	}
	
	while(*prefix != 0) {
		if (*localpath == 0)
			return false;

		if (*localpath != *prefix)
			return false;

		localpath++;
		prefix++;
	}

	*postfix = localpath;

	return true;
}

int FTPCache::ExpandPaths() {
	for(size_t i = 0; i < m_vCachePaths.size(); i++) {
		TCHAR * expPath = ExpandPath(m_vCachePaths[i].localpath);
		if (expPath != NULL) {
			delete [] m_vCachePaths[i].localpathExpanded;
			m_vCachePaths[i].localpathExpanded = expPath;
		}
	}
	return 0;
}

TCHAR* FTPCache::ExpandPath(const TCHAR * path) {
	if (!path)
		return NULL;

	tstring replacestring(path);
	if (_ConfigPath) {
		replacestring = SU::ReplaceString(replacestring, TEXT("%CONFIGDIR%"), _ConfigPath);
	} else {
		replacestring = SU::ReplaceString(replacestring, TEXT("%CONFIGDIR%"), TEXT("."));
	}
	replacestring = SU::ReplaceString(replacestring, TEXT("%USERNAME%"), m_activeUser);
	replacestring = SU::ReplaceString(replacestring, TEXT("%HOSTNAME%"), m_activeHost);

	TCHAR * expanded = new TCHAR[MAX_PATH];
	BOOL res = PathSearchAndQualify(replacestring.c_str(), expanded, MAX_PATH);

	if (res == FALSE) {
		delete [] expanded;
		return NULL;
	}

	PathRemoveBackslash(expanded);

	//OutMsg("Expanded %T to %T", path, expanded);

	return expanded;
}
