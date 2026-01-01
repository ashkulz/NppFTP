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

#ifndef FTPSETTINGS_H
#define FTPSETTINGS_H

#include "FTPCache.h"

//Container for various (global) settings

class FTPSettings {
public:
							FTPSettings();
	virtual					~FTPSettings();

	const TCHAR*			GetGlobalCachePath() const;
	int						SetGlobalCachePath(const TCHAR * path);

	FTPCache*				GetGlobalCache();

	const char*				GetEncryptionKey() const;	//array of size 8
	int						SetEncryptionKey(const char * key);	//key must array of size 8

	bool					GetDebugMode() const;
	int						SetDebugMode(bool debugMode);

	bool					GetClearCache() const;
	int						SetClearCache(bool clearCache);

	bool					GetClearCachePermanent() const;
	int						SetClearCachePermanent(bool clearCachePermanent);

	bool					GetOutputShown() const;
	int						SetOutputShown(bool showOutput);

	double					GetSplitRatio() const;
	int						SetSplitRatio(double splitRatio);

	int						LoadSettings(const TiXmlElement * settingsElem);
	int						SaveSettings(TiXmlElement * settingsElem);
private:
	TCHAR*					m_globalCachePath;
	FTPCache				m_globalCache;
	bool					m_clearCache;
	bool					m_clearCachePermanent;
	bool					m_showOutput;
	double					m_splitRatio;
	bool					m_debugMode;
};

#endif //FTPSETTINGS_H
