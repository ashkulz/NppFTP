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

#ifndef FTPPROFILE_H
#define FTPPROFILE_H

#include "FTPClientWrapper.h"
#include "FTPCache.h"

class FTPProfile;
typedef std::vector<FTPProfile*> vProfile;
typedef std::vector<TCHAR*> vString;

class FTPProfile : public RefObject {
public:
	static const char*		ProfilesElement;
private:
	int 					m_refcounter;
							FTPProfile();
public:
							FTPProfile(const TCHAR * name);	//create new profile
							FTPProfile(const TCHAR * name, const FTPProfile* other);	//clone profile
							~FTPProfile();

	virtual FTPClientWrapper*	CreateWrapper();

	//getters/setters
	const TCHAR*			GetName() const;
	int						SetName(const TCHAR * name);

	const TCHAR*			GetParent() const;
	int						SetParent(const TCHAR * parent);
	const char*				GetHostname() const;
	int						SetHostname(const char * hostname);
	int						GetPort() const;
	int						SetPort(int port);
	const char*				GetUsername() const;
	int						SetUsername(const char * username);
	const char*				GetPassword() const;
	int						SetPassword(const char * password);
	bool					GetAskPassword() const;
	int						SetAskPassword(bool askPassword);

	int						GetTimeout() const;
	int						SetTimeout(int timeout);

	Security_Mode			GetSecurityMode() const;
	int						SetSecurityMode(Security_Mode mode);
	Transfer_Mode			GetTransferMode() const;
	int						SetTransferMode(Transfer_Mode mode);
	Connection_Mode			GetConnectionMode() const;
	int						SetConnectionMode(Connection_Mode mode);

	int						GetDataPortRange(int * min, int * max) const;
	int						SetDataPortRange(int min, int max);

	const char*				GetListParams() const;
	int						SetListParams(const char * listParams);

	const char*				GetInitialDir() const;
	int						SetInitialDir(const char * dir);

	const TCHAR*			GetKeyFile() const;
	int						SetKeyFile(const TCHAR * keyFile);
	const char*				GetPassphrase() const;
	int						SetPassphrase(const char * passphrase);
	bool					GetAskPassphrase() const;
	int						SetAskPassphrase(bool askPassphrase);
	bool					GetUseAgent() const;
	int						SetUseAgent(bool useAgent);
	AuthenticationMethods	GetAcceptedMethods() const;
	int						SetAcceptedMethods(AuthenticationMethods acceptedMethods);

	int						SetCacheParent(FTPCache * parentCache);

	//other functions
	//filetypes
	int						AddAsciiType(const TCHAR * type);	//type must start with period, cannot contain pipe
	int						AddBinaryType(const TCHAR * type);
	int						RemoveAsciiType(const TCHAR * type);
	int						RemoveBinaryType(const TCHAR * type);
	int						GetAsciiCount();
	int						GetBinaryCount();
	const TCHAR*			GetAsciiType(int i);
	const TCHAR*			GetBinaryType(int i);

	Transfer_Mode			GetFileTransferMode(const TCHAR* file) const;	//filename only, no paths (or path must be win32 compatiable?)
	int						GetCacheExternal(const TCHAR* localfile, char* extbuffer, int extbuffersize) const;
	int						GetCacheLocal(const char * externalfile, TCHAR* localbuffer, int localbuffersize) const;

	FTPCache*				GetCache() const;

	static vProfile			LoadProfiles(const TiXmlElement * profilesElem);
	static TiXmlElement*	SaveProfiles(const vProfile profiles);

	static int				SortVector(vProfile & pVect);
	bool					operator==(const FTPProfile& other) const;

private:
	static FTPProfile*		LoadProfile(const TiXmlElement * profileElem);
	TiXmlElement*			SaveProfile() const;	//return value only valid as long as profile object exists
	int						Sanitize();
	bool					ValidType(const TCHAR * type) const;
	int						ExpandTypeVector(tstring types, bool isAscii);
	tstring					CompactTypeVector(vString vect) const;

	static bool				CompareProfile(const FTPProfile * prof1, const FTPProfile * prof2);

	TCHAR*					m_name;

	TCHAR*					m_parent;

	FTPCache*				m_cache;

	char*					m_hostname;
	int						m_port;
	char*					m_username;
	char*					m_password;
	bool					m_askPassword;
	bool					m_askPassphrase;

	int						m_timeout;

	Security_Mode			m_securityMode;
	Transfer_Mode			m_transferMode;
	Connection_Mode			m_connectionMode;

	int						m_dataPortMin;
	int						m_dataPortMax;

	char*					m_ftpListParams;

	char*					m_initialDir;

	vString					m_asciiTypes;
	vString					m_binTypes;

	TCHAR*					m_keyFile;
	char*					m_passphrase;
	bool					m_useAgent;
	AuthenticationMethods	m_acceptedMethods;
};

#endif //FTPPROFILE_H
