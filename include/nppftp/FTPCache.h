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

#ifndef FTPCACHE_H
#define FTPCACHE_H

struct PathMap {
	TCHAR*					localpath;
	TCHAR*					localpathExpanded;	//internal variable, do not use
	char*					externalpath;
};

typedef std::vector<PathMap> vPathMap;

class FTPCache {
public:
	static const char *		CacheElem;
public:
							FTPCache();
	virtual					~FTPCache();

	virtual int				SetCacheParent(FTPCache * m_cacheParent);
	virtual int				SetEnvironment(const char * host, const char * user);

	virtual const PathMap &	GetPathMap(int i) const;
	virtual int				GetPathMapCount() const;
	virtual int				DeletePathMap(int i);
	virtual int				AddPathMap(PathMap pathmap);
	virtual int				SetPathMap(PathMap pathmap, int index);
	virtual int				SwapPathMap(int indexa, int indexb);
	virtual int				Clear();

							//return -1 on error, 0 on success, 1 if no cache match found
	virtual int				GetExternalPathFromLocal(const TCHAR * localpath, char * extbuf, int extsize) const;
	virtual int				GetLocalPathFromExternal(const char * externalpath, TCHAR * localbuf, int localsize) const;

	static TiXmlElement*	SaveCache(const FTPCache * cache);
	static FTPCache*		LoadCache(const TiXmlElement * cacheElem);
private:
	int						AddPathMap(const TCHAR * localpath, const char * externalpath);	//localpath in UTF-8

	virtual bool			IsPathPrefixLocal(const TCHAR * localpath, const TCHAR * prefix, const TCHAR ** postfix) const;
	virtual bool			IsPathPrefixExternal(const char * localpath, const char * prefix, const char ** postfix) const;

	int						ExpandPaths();
	TCHAR*					ExpandPath(const TCHAR * path);

	vPathMap				m_vCachePaths;
	FTPCache*				m_cacheParent;

	TCHAR*					m_activeHost;
	TCHAR*					m_activeUser;
};

#endif //FTPCACHE_H
