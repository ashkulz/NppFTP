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
#include "FTPProfile.h"

#include "Encryption.h"
#include "InputDialog.h"
#include <algorithm>

const char * FTPProfile::ProfilesElement = "Profiles";

FTPProfile::FTPProfile() :
	m_name(NULL),
	m_parent(NULL),
	m_cache(NULL),
	m_hostname(NULL),
	m_port(0),
	m_username(NULL),
	m_password(NULL),
	m_askPassword(false),
	m_askPassphrase(false),
	m_timeout(30),
	m_securityMode(Mode_FTP),
	m_transferMode(Mode_Binary),
	m_connectionMode(Mode_Passive),
	m_dataPortMin(10000),
	m_dataPortMax(32000),
	m_ftpListParams(NULL),
	m_initialDir(NULL),
	m_keyFile(NULL),
	m_passphrase(NULL),
	m_useAgent(false),
	m_acceptedMethods(Method_Password)
{
}

FTPProfile::FTPProfile(const TCHAR * name) :
	m_parent(TEXT("")),
	m_port(21),
	m_askPassword(false),
	m_askPassphrase(false),
	m_timeout(30),
	m_securityMode(Mode_FTP),
	m_transferMode(Mode_Binary),
	m_connectionMode(Mode_Passive),
	m_dataPortMin(10000),
	m_dataPortMax(32000),
	m_useAgent(false),
	m_acceptedMethods(Method_Password)
{
	m_cache = new FTPCache();

	m_name = SU::DupString(name);
	m_parent = SU::DupString(L"");
	m_hostname = SU::strdup("");
	m_username = SU::strdup("");
	m_password = SU::strdup("");
	m_initialDir = SU::strdup("");

	m_ftpListParams = SU::strdup("");

	m_keyFile = SU::DupString(TEXT(""));
	m_passphrase = SU::strdup("");

	m_cache->SetEnvironment(m_hostname, m_username);
}

FTPProfile::FTPProfile(const TCHAR * name, const FTPProfile* other) :
	m_parent(TEXT("")),
	m_port(other->m_port),
	m_askPassword(other->m_askPassword),
	m_askPassphrase(other->m_askPassphrase),
	m_timeout(other->m_timeout),
	m_securityMode(other->m_securityMode),
	m_transferMode(other->m_transferMode),
	m_connectionMode(other->m_connectionMode),
	m_dataPortMin(other->m_dataPortMin),
	m_dataPortMax(other->m_dataPortMax),
	m_useAgent(other->m_useAgent),
	m_acceptedMethods(other->m_acceptedMethods)
{
	m_cache = new FTPCache();

	m_name = SU::DupString(name);
	m_hostname = SU::strdup(other->m_hostname);
	m_username = SU::strdup(other->m_username);
	m_password = SU::strdup(other->m_password);
	m_initialDir = SU::strdup(other->m_initialDir);
	m_parent = SU::DupString(other->m_parent);

	m_ftpListParams = SU::strdup(other->m_ftpListParams);

	m_keyFile = SU::DupString(other->m_keyFile);
	m_passphrase = SU::strdup(other->m_passphrase);

	m_cache->SetEnvironment(m_hostname, m_username);

	for(int i = 0; i < other->m_cache->GetPathMapCount(); i++) {
		PathMap map;
		const PathMap & othermap = other->m_cache->GetPathMap(i);
		map.localpath = SU::DupString(othermap.localpath);
		map.externalpath = SU::strdup(othermap.externalpath);
		m_cache->AddPathMap(map);
	}
}

FTPProfile::~FTPProfile() {
	if (m_name) {
		SU::FreeTChar(m_name);
		m_name = NULL;
	}
	if (m_parent) {
		SU::FreeTChar(m_parent);
		m_parent = NULL;
	}
	if (m_hostname) {
		SU::free(m_hostname);
		m_hostname = NULL;
	}
	if (m_username) {
		SU::free(m_username);
		m_username = NULL;
	}
	if (m_password) {
		SU::free(m_password);
		m_password = NULL;
	}

	if (m_cache) {
		delete m_cache;
		m_cache = NULL;
	}

	if (m_ftpListParams) {
		SU::free(m_ftpListParams);
		m_ftpListParams = NULL;
	}

	if (m_initialDir) {
		SU::free(m_initialDir);
		m_initialDir = NULL;
	}

	for(size_t i = 0; i < m_asciiTypes.size(); i++) {
		SU::FreeTChar(m_asciiTypes[i]);
	}
	m_asciiTypes.clear();

	for(size_t i = 0; i < m_binTypes.size(); i++) {
		SU::FreeTChar(m_binTypes[i]);
	}
	m_binTypes.clear();
}

FTPClientWrapper* FTPProfile::CreateWrapper() {
	FTPClientWrapper * wrapper = NULL;

	char * password = NULL, * passphrase = NULL;
	if (m_askPassword) {
		InputDialog passDialog;
		int ret = passDialog.Create(_MainOutputWindow, TEXT("Enter password"), TEXT("Please enter the password to connect to the server"), TEXT(""), true);
		if (ret == 1) {
			password = SU::TCharToCP(passDialog.GetValue(), CP_ACP);
		} else {
			return NULL;
		}
	} else {
		password = SU::strdup(m_password);
	}

	if (m_securityMode == Mode_SFTP && m_askPassphrase) {
		InputDialog passDialog;
		int ret = passDialog.Create(_MainOutputWindow, TEXT("Enter passphrase"), TEXT("Please enter the passphrase to decrypt private key:"), TEXT(""), true);
		if (ret == 1) {
			passphrase = SU::TCharToCP(passDialog.GetValue(), CP_ACP);
		} else {
			return NULL;
		}
	} else {
		passphrase = m_passphrase;
	}

	switch(m_securityMode) {
		case Mode_SFTP: {
			FTPClientWrapperSSH * SSHwrapper = new FTPClientWrapperSSH(m_hostname, m_port, m_username, password);
			wrapper = SSHwrapper;
			SSHwrapper->SetKeyFile(m_keyFile);
			SSHwrapper->SetPassphrase(passphrase);
			SSHwrapper->SetUseAgent(m_useAgent);
			SSHwrapper->SetAcceptedMethods(m_acceptedMethods);
			break; }
		case Mode_FTP:
		case Mode_FTPS:
		case Mode_FTPES: {
			FTPClientWrapperSSL * SSLwrapper = new FTPClientWrapperSSL(m_hostname, m_port, m_username, password);
			wrapper = SSLwrapper;
			if (m_securityMode == Mode_FTP)
				SSLwrapper->SetMode(CUT_FTPClient::FTP);
			else if (m_securityMode == Mode_FTPS)
				SSLwrapper->SetMode(CUT_FTPClient::FTPS);
			else if (m_securityMode == Mode_FTPES)
				SSLwrapper->SetMode(CUT_FTPClient::FTPES);

			SSLwrapper->SetConnectionMode(m_connectionMode);
			SSLwrapper->SetPortRange(m_dataPortMin, m_dataPortMax);
			SSLwrapper->SetListParams(m_ftpListParams);
			break; }
		case Mode_SecurityMax:
		default:
			SU::free(password);
			return NULL;
			break;

	}

	wrapper->SetTimeout(m_timeout);
	SU::free(password);

	return wrapper;
}

const TCHAR* FTPProfile::GetName() const {
	return m_name;
}

int FTPProfile::SetName(const TCHAR * name) {
	SU::FreeTChar(m_name);
	m_name = SU::DupString(name);
	return 0;
}

const TCHAR* FTPProfile::GetParent() const {
	return m_parent;
}


const char* FTPProfile::GetHostname() const {
	return m_hostname;
}

int FTPProfile::SetParent(const TCHAR * parent) {
	SU::FreeTChar(m_parent);
	m_parent = SU::DupString(parent);
	return 0;
}

int FTPProfile::SetHostname(const char * hostname) {
	SU::free(m_hostname);
	m_hostname = SU::strdup(hostname);
	m_cache->SetEnvironment(m_hostname, m_username);
	return 0;
}

int FTPProfile::GetPort() const {
	return m_port;
}

int FTPProfile::SetPort(int port) {
	if (port <= 0 || port >= 65536)
		return -1;
	m_port = port;
	return 0;
}

const char* FTPProfile::GetUsername() const {
	return m_username;
}

int FTPProfile::SetUsername(const char * username) {
	SU::free(m_username);
	m_username = SU::strdup(username);
	m_cache->SetEnvironment(m_hostname, m_username);
	return 0;
}

const char* FTPProfile::GetPassword() const {
	return m_password;
}

int FTPProfile::SetPassword(const char * password) {
	SU::free(m_password);
	m_password = SU::strdup(password);
	return 0;
}

bool FTPProfile::GetAskPassword() const {
	return m_askPassword;
}

int FTPProfile::SetAskPassword(bool askPassword) {
	m_askPassword = askPassword;
	return 0;
}

int FTPProfile::GetTimeout() const {
	return m_timeout;
}

int FTPProfile::SetTimeout(int timeout) {
	if (timeout < 0)
		return -1;

	m_timeout = timeout;
	return 0;
}

Security_Mode FTPProfile::GetSecurityMode() const {
	return m_securityMode;
}

int FTPProfile::SetSecurityMode(Security_Mode mode) {
	if (mode < 0 || mode > Mode_SecurityMax)
		return -1;

	m_securityMode = mode;
	return 0;
}

Transfer_Mode FTPProfile::GetTransferMode() const {
	return m_transferMode;
}

int FTPProfile::SetTransferMode(Transfer_Mode mode) {
	if (mode < 0 || mode > Mode_TransferMax)
		return -1;

	m_transferMode = mode;
	return 0;
}

Connection_Mode FTPProfile::GetConnectionMode() const {
	return m_connectionMode;
}

int FTPProfile::SetConnectionMode(Connection_Mode mode) {
	if (mode < 0 || mode > Mode_ConnectionMax)
		return -1;

	m_connectionMode = mode;
	return 0;
}

int FTPProfile::GetDataPortRange(int * min, int * max) const {
	if (!min || !max)
		return -1;

	*min = m_dataPortMin;
	*max = m_dataPortMax;

	return 0;
}

int FTPProfile::SetDataPortRange(int min, int max) {
	m_dataPortMin = min;
	m_dataPortMax = max;

	if (m_dataPortMin < 1000)
		m_dataPortMin = 1000;
	if (m_dataPortMin > 65000)
		m_dataPortMin = 65000;

	if (m_dataPortMax < m_dataPortMin)
		m_dataPortMax = m_dataPortMin+1;

	if (m_dataPortMax > 65001)
		m_dataPortMax = 65001;

	return 0;
}

const char * FTPProfile::GetListParams() const {
	return m_ftpListParams;
}

int FTPProfile::SetListParams(const char * listParams) {
	if (m_ftpListParams)
		SU::free(m_ftpListParams);
	m_ftpListParams = SU::strdup(listParams);
	return 0;
}

const char* FTPProfile::GetInitialDir() const {
	return m_initialDir;
}

int FTPProfile::SetInitialDir(const char * dir) {
	SU::free(m_initialDir);
	m_initialDir = SU::strdup(dir);
	return 0;
}

const TCHAR* FTPProfile::GetKeyFile() const {
	return m_keyFile;
}

int FTPProfile::SetKeyFile(const TCHAR * keyFile) {
	SU::FreeTChar(m_keyFile);
	m_keyFile = SU::DupString(keyFile);
	return 0;
}

const char* FTPProfile::GetPassphrase() const {
	return m_passphrase;
}

int FTPProfile::SetPassphrase(const char * passphrase) {
	SU::free(m_passphrase);
	m_passphrase = SU::strdup(passphrase);
	return 0;
}

bool FTPProfile::GetAskPassphrase() const {
	return m_askPassphrase;
}

int FTPProfile::SetAskPassphrase(bool askPassphrase) {
	m_askPassphrase = askPassphrase;
	return 0;
}

bool FTPProfile::GetUseAgent() const {
	return m_useAgent;
}

int FTPProfile::SetUseAgent(bool useAgent) {
	m_useAgent = useAgent;
	return 0;
}

AuthenticationMethods FTPProfile::GetAcceptedMethods() const {
	return m_acceptedMethods;
}

int FTPProfile::SetAcceptedMethods(AuthenticationMethods acceptedMethods) {
	m_acceptedMethods = acceptedMethods;
	return 0;
}

int FTPProfile::SetCacheParent(FTPCache * parentCache) {
	if (!m_cache)
		return -1;

	m_cache->SetCacheParent(parentCache);
	return 0;
}


//other functions
int FTPProfile::AddAsciiType(const TCHAR * type) {
	if (!ValidType(type))
		return -1;

	for(size_t i = 0; i < m_asciiTypes.size(); i++) {
		if (!lstrcmp(type, m_asciiTypes[i])) {
			return 0;
		}
	}

	m_asciiTypes.push_back(SU::DupString(type));
	return 0;
}

int FTPProfile::AddBinaryType(const TCHAR * type) {
	if (!ValidType(type))
		return -1;

	for(size_t i = 0; i < m_binTypes.size(); i++) {
		if (!lstrcmp(type, m_binTypes[i])) {
			return 0;
		}
	}

	m_binTypes.push_back(SU::DupString(type));
	return 0;
}

int FTPProfile::RemoveAsciiType(const TCHAR * type) {
	for(size_t i = 0; i < m_asciiTypes.size(); i++) {
		if (!lstrcmp(type, m_asciiTypes[i])) {
			SU::FreeTChar(m_asciiTypes[i]);
			m_asciiTypes.erase(m_asciiTypes.begin()+i);
			return 0;
		}
	}
	return -1;
}

int FTPProfile::RemoveBinaryType(const TCHAR * type) {
	for(size_t i = 0; i < m_binTypes.size(); i++) {
		if (!lstrcmp(type, m_binTypes[i])) {
			SU::FreeTChar(m_binTypes[i]);
			m_binTypes.erase(m_binTypes.begin()+i);
			return 0;
		}
	}
	return -1;
}

int FTPProfile::GetAsciiCount() {
	return (int)m_asciiTypes.size();
}

int FTPProfile::GetBinaryCount() {
	return (int)m_binTypes.size();
}

const TCHAR* FTPProfile::GetAsciiType(int i) {
	return m_asciiTypes.at(i);
}

const TCHAR* FTPProfile::GetBinaryType(int i) {
	return m_binTypes.at(i);
}

Transfer_Mode FTPProfile::GetFileTransferMode(const TCHAR* file) const {
	LPCTSTR suffix = NULL;

	if (m_asciiTypes.size()) {
		suffix = PathFindSuffixArray(file, (const TCHAR**)(&m_asciiTypes[0]), static_cast<int>(m_asciiTypes.size()) );
		if (suffix) {
			return Mode_ASCII;
		}
	}

	if (m_binTypes.size()) {
		suffix = PathFindSuffixArray(file, (const TCHAR**)(&m_binTypes[0]), static_cast<int>(m_binTypes.size()) );
		if (suffix) {
			return Mode_Binary;
		}
	}

	return m_transferMode;
}

int FTPProfile::GetCacheExternal(const TCHAR* localfile, char* extbuffer, int extbuffersize) const {
	if (!extbuffer || extbuffersize == 0)
		return -1;

	return m_cache->GetExternalPathFromLocal(localfile, extbuffer, extbuffersize);
}

int FTPProfile::GetCacheLocal(const char * externalfile, TCHAR* localbuffer, int localbuffersize) const {
	if (!localbuffer || localbuffersize == 0)
		return -1;

	return m_cache->GetLocalPathFromExternal(externalfile, localbuffer, localbuffersize);
}

FTPCache* FTPProfile::GetCache() const {
	return m_cache;
}

vProfile FTPProfile::LoadProfiles(const TiXmlElement * profilesElem) {
	vProfile profiles;

	if (!profilesElem)
		return profiles;

	if ( strcmp(FTPProfile::ProfilesElement, profilesElem->Value()) )
		return profiles;

	const TiXmlElement* child = profilesElem->FirstChildElement("Profile");

	for( ; child; child = child->NextSiblingElement("Profile") )
	{
		FTPProfile * profile = FTPProfile::LoadProfile(child);
		if (profile) {
			profiles.push_back(profile);
		}
	}

	SortVector(profiles);

	return profiles;
}

TiXmlElement* FTPProfile::SaveProfiles(const vProfile profiles) {
	TiXmlElement * profilesElem = new TiXmlElement(FTPProfile::ProfilesElement);
	size_t count = profiles.size();
	for(size_t i = 0; i < count; i++) {
		TiXmlElement* profile = profiles[i]->SaveProfile();
		if (profile) {
			profilesElem->LinkEndChild(profile);
		}
	}

	return profilesElem;
}

FTPProfile* FTPProfile::LoadProfile(const TiXmlElement * profileElem) {
	if (!profileElem)
		return NULL;

	FTPProfile * profile = new FTPProfile();
	bool success = false;
	const char * attrstr = NULL;

	do {
		attrstr = profileElem->Attribute("name");
		if (!attrstr)
			break;
		profile->m_name = SU::Utf8ToTChar(attrstr);

		attrstr = profileElem->Attribute("parent");
		if (!attrstr)
			profile->m_parent = SU::Utf8ToTChar("");
		else
			profile->m_parent = SU::Utf8ToTChar(attrstr);


		attrstr = profileElem->Attribute("hostname");
		if (!attrstr)
			profile->m_hostname = SU::strdup("");
		else
			profile->m_hostname = SU::strdup(attrstr);

		profileElem->Attribute("port", &profile->m_port);

		attrstr = profileElem->Attribute("username");
		if (!attrstr)
			profile->m_username = SU::strdup("");
		else
			profile->m_username = SU::strdup(attrstr);

		attrstr = profileElem->Attribute("password");
		if (!attrstr) {
			profile->m_password = SU::strdup("");
		} else {
			char * decryptpass = Encryption::Decrypt(NULL, -1, attrstr, true);
			if (decryptpass)
				profile->m_password = SU::strdup(decryptpass);
			else
				profile->m_password = SU::strdup("");
			Encryption::FreeData(decryptpass);
		}

		profileElem->QueryBoolAttribute("askPassword", &profile->m_askPassword);

		profileElem->Attribute("timeout", &profile->m_timeout);

		//TODO: this is rather risky casting, check if the compiler accepts it
		profileElem->Attribute("securityMode", (int*)(&profile->m_securityMode));
		profileElem->Attribute("transferMode", (int*)(&profile->m_transferMode));
		profileElem->Attribute("connectionMode", (int*)(&profile->m_connectionMode));

		profileElem->Attribute("dataPortMin", (int*)(&profile->m_dataPortMin));
		profileElem->Attribute("dataPortMax", (int*)(&profile->m_dataPortMax));

		attrstr = profileElem->Attribute("listParams");
		if (!attrstr)
			profile->m_ftpListParams = SU::strdup("");
		else
			profile->m_ftpListParams = SU::strdup(attrstr);

		const TiXmlElement * cacheElem = profileElem->FirstChildElement(FTPCache::CacheElem);
		if (!cacheElem)
			break;

		profile->m_cache = FTPCache::LoadCache(cacheElem);
		if (!profile->m_cache)
			break;

		attrstr = profileElem->Attribute("initialDir");
		if (!attrstr)
			profile->m_initialDir = SU::strdup("");
		else
			profile->m_initialDir = SU::strdup(attrstr);

		attrstr = profileElem->Attribute("keyFile");
		if (!attrstr)
			profile->m_keyFile = SU::DupString(TEXT(""));
		else
			profile->m_keyFile = SU::Utf8ToTChar(attrstr);

		attrstr = profileElem->Attribute("passphrase");
		if (!attrstr) {
			profile->m_passphrase = SU::strdup("");
		} else {
			char * decryptphrase = Encryption::Decrypt(NULL, -1, attrstr, true);
			profile->m_passphrase = SU::strdup(decryptphrase);
			Encryption::FreeData(decryptphrase);
		}

		profileElem->QueryBoolAttribute("askPassphrase", &profile->m_askPassphrase);
		profileElem->QueryBoolAttribute("useAgent", &profile->m_useAgent);
		profileElem->Attribute("acceptedMethods", (int*)(&profile->m_acceptedMethods));

		const TiXmlElement * typesElem = profileElem->FirstChildElement("FileTypes");
		if (!typesElem)
			break;

		const char * asciistr = typesElem->Attribute("asciiTypes");
		const char * binstr = typesElem->Attribute("binaryTypes");
		if (!asciistr || !binstr)
			break;

		TCHAR * asciistrW = SU::Utf8ToTChar(asciistr);
		TCHAR * binstrW = SU::Utf8ToTChar(binstr);

		profile->ExpandTypeVector(asciistrW, true);
		profile->ExpandTypeVector(binstrW, false);

		SU::FreeTChar(asciistrW);
		SU::FreeTChar(binstrW);

		profile->Sanitize();

		success = true;
	} while(false);

	if (!success) {
		delete profile;
		return NULL;
	}

	return profile;
}

TiXmlElement* FTPProfile::SaveProfile() const {
	TiXmlElement * profileElem = new TiXmlElement("Profile");

	char * utf8name = SU::TCharToUtf8(m_name);
	profileElem->SetAttribute("name", utf8name);
	SU::FreeChar(utf8name);

	char * utf8parent = SU::TCharToUtf8(m_parent);
	profileElem->SetAttribute("parent", utf8parent);
	SU::FreeChar(utf8parent);


	profileElem->SetAttribute("hostname", m_hostname);
	profileElem->SetAttribute("port", m_port);
	profileElem->SetAttribute("username", m_username);

	char * encryptPass = Encryption::Encrypt(NULL, -1, m_askPassword?"":m_password, -1);	//when asking for password, do not store the password
	profileElem->SetAttribute("password", encryptPass);
	Encryption::FreeData(encryptPass);

	profileElem->SetAttribute("askPassword", m_askPassword);

	profileElem->SetAttribute("timeout", m_timeout);

	profileElem->SetAttribute("securityMode", m_securityMode);
	profileElem->SetAttribute("transferMode", m_transferMode);
	profileElem->SetAttribute("connectionMode", m_connectionMode);

	profileElem->SetAttribute("dataPortMin", m_dataPortMin);
	profileElem->SetAttribute("dataPortMax", m_dataPortMax);

	profileElem->SetAttribute("listParams", m_ftpListParams);

	profileElem->SetAttribute("initialDir", m_initialDir);

	char * utf8keyfile = SU::TCharToUtf8(m_keyFile);
	profileElem->SetAttribute("keyFile", utf8keyfile);
	SU::FreeChar(utf8keyfile);

	char * encryptPhrase = Encryption::Encrypt(NULL, -1, m_askPassphrase?"":m_passphrase, -1);
	profileElem->SetAttribute("passphrase", encryptPhrase);
	Encryption::FreeData(encryptPhrase);

	profileElem->SetAttribute("askPassphrase", m_askPassphrase);
	profileElem->SetAttribute("useAgent", m_useAgent?1:0);
	profileElem->SetAttribute("acceptedMethods", (int)m_acceptedMethods);

	TiXmlElement * cacheElem = FTPCache::SaveCache(m_cache);
	if (!cacheElem) {
		delete profileElem;
		return NULL;
	}
	profileElem->LinkEndChild(cacheElem);

	tstring asciiString = CompactTypeVector(m_asciiTypes);
	tstring binaryString = CompactTypeVector(m_binTypes);

	char * asciiUtf8 = SU::TCharToUtf8(asciiString.c_str());
	char * binUtf8 = SU::TCharToUtf8(binaryString.c_str());

	TiXmlElement * typesElem = new TiXmlElement("FileTypes");
	typesElem->SetAttribute("asciiTypes", asciiUtf8);
	typesElem->SetAttribute("binaryTypes", binUtf8);
	profileElem->LinkEndChild(typesElem);

	SU::FreeChar(asciiUtf8);
	SU::FreeChar(binUtf8);

	return profileElem;
}

int FTPProfile::Sanitize() {
	if (m_port <= 0 || m_port >= 65536)
		m_port = 21;

	if (m_timeout < 0)
		m_timeout = 0;

	if (m_securityMode < 0 || m_securityMode >= Mode_SecurityMax)
		m_securityMode = Mode_FTP;

	if (m_transferMode < 0 || m_transferMode >= Mode_TransferMax)
		m_transferMode = Mode_Binary;

	if (m_connectionMode < 0 || m_connectionMode >= Mode_ConnectionMax)
		m_connectionMode = Mode_Passive;

	if (m_dataPortMin < 1000)
		m_dataPortMin = 1000;
	if (m_dataPortMin > 65000)
		m_dataPortMin = 65000;
	if (m_dataPortMax < m_dataPortMin)
		m_dataPortMax = m_dataPortMin;
	if (m_dataPortMax > 65001)
		m_dataPortMax = 65001;

	m_acceptedMethods = (AuthenticationMethods)(m_acceptedMethods & Method_All);

	return 0;
}

bool FTPProfile::ValidType(const TCHAR * type) const {
	if (!type)
		return false;

	if (lstrlen(type) < 2)
		return false;

	if (type[0] != TEXT('.'))
		return false;

	return true;
}

int FTPProfile::ExpandTypeVector(tstring types, bool isAscii) {
	const tstring delimiter = TEXT("|");
    tstring::size_type lastPos = types.find_first_not_of(delimiter, 0);
    tstring::size_type pos     = types.find_first_of(delimiter, lastPos);

    while (tstring::npos != pos || tstring::npos != lastPos)
    {
        tstring type = types.substr(lastPos, pos - lastPos);
        if (isAscii)
			AddAsciiType(type.c_str());
		else
			AddBinaryType(type.c_str());

        lastPos = types.find_first_not_of(delimiter, pos);
        pos = types.find_first_of(delimiter, lastPos);
    }

	return 0;
}

tstring FTPProfile::CompactTypeVector(vString vect) const {
	tstring typeString;

	if (vect.size() > 0) {
		typeString = vect[0];
		for(size_t i = 1; i < vect.size(); i++) {
			typeString += TEXT("|");
			typeString += vect[i];
		}
	}

	return typeString;
}

int FTPProfile::SortVector(vProfile & pVect) {
	std::sort(pVect.begin(), pVect.end(), &FTPProfile::CompareProfile);

	return 0;
}

bool FTPProfile::operator==(const FTPProfile& other) const
{
	return (lstrcmpi(this->GetName(), other.GetName()) == 0 && lstrcmpi(this->GetParent(), other.GetParent()) == 0);
}

bool FTPProfile::CompareProfile(const FTPProfile * prof1, const FTPProfile * prof2) {
	int res = 0;
	res = lstrcmpi(prof1->GetName(), prof2->GetName());

	return (res < 0);
}
