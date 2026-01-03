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
#include "FTPSettings.h"

#include "Encryption.h"
#include "InputDialog.h"

bool	FTPSettings::m_debugMode = false;

FTPSettings::FTPSettings() :
	m_clearCache(false),
	m_clearCachePermanent(false),
	m_showOutput(false),
	m_splitRatio(0.5)
{
	m_globalCachePath = SU::DupString(TEXT("%CONFIGDIR%\\Cache\\%USERNAME%@%HOSTNAME%"));

	PathMap globalPathmap{};
	globalPathmap.localpath = SU::DupString(m_globalCachePath);
	globalPathmap.externalpath = SU::strdup("/");
	m_globalCache.Clear();
	//m_globalCache takes ownership of globalPathmap
	m_globalCache.AddPathMap(globalPathmap);
}

FTPSettings::~FTPSettings() {
	SU::FreeTChar(m_globalCachePath);
}

const TCHAR* FTPSettings::GetGlobalCachePath() const {
	return m_globalCachePath;
}

int FTPSettings::SetGlobalCachePath(const TCHAR * path) {
	if (!path || lstrlen(path) < 4) {	//must at least have a drive designator or network name
		path = TEXT("%CONFIGDIR%\\Cache\\%USERNAME%@%HOSTNAME%\\%PORT%");
	}

	PathMap globalPathmap{};
	globalPathmap.localpath = SU::DupString(path);
	globalPathmap.externalpath = SU::strdup("/");
	m_globalCache.Clear();
	m_globalCache.AddPathMap(globalPathmap);

	SU::FreeTChar(m_globalCachePath);
	m_globalCachePath = SU::DupString(path);

	return 0;
}

FTPCache* FTPSettings::GetGlobalCache() {
	return &m_globalCache;
}

const char* FTPSettings::GetEncryptionKey() const {
	return Encryption::GetDefaultKey();
}

int FTPSettings::SetEncryptionKey(const char * key) {
	Encryption::SetDefaultKey(key, -1);
	return 0;
}

bool FTPSettings::GetDebugMode() {
	return m_debugMode;
}

int FTPSettings::SetDebugMode(bool debugMode) {
	m_debugMode = debugMode;
	return 0;
}

bool FTPSettings::GetClearCache() const {
	return m_clearCache;
}

int FTPSettings::SetClearCache(bool clearCache) {
	m_clearCache = clearCache;
	return 0;
}

bool FTPSettings::GetClearCachePermanent() const {
	return m_clearCachePermanent;
}

int FTPSettings::SetClearCachePermanent(bool clearCachePermanent) {
	m_clearCachePermanent = clearCachePermanent;
	return 0;
}

bool FTPSettings::GetOutputShown() const {
	return m_showOutput;
}

int FTPSettings::SetOutputShown(bool showOutput) {
	m_showOutput = showOutput;
	return 0;
}

double FTPSettings::GetSplitRatio() const {
	return m_splitRatio;
}
int FTPSettings::SetSplitRatio(double splitRatio) {
	m_splitRatio = splitRatio;
	return 0;
}

int FTPSettings::LoadSettings(const TiXmlElement * settingsElem) {
	int outState = 0;
	const char * outstr = settingsElem->Attribute("outputShown", &outState);
	if (!outstr) {
		outState = 0;
	}
	m_showOutput = (outState != 0);

	double ratio = 0.5;
	const char * ratiostr = settingsElem->Attribute("windowRatio", &ratio);
	if (!ratiostr) {
		ratio = 0.5;
	}
	m_splitRatio = ratio;

	//Imperative that masterpassword be set before loading all profiles
	const char * passstr = settingsElem->Attribute("MasterPass");
	if (passstr) {
		InputDialog inputPass;
		const TCHAR * query = TEXT("Please enter master password");
		bool success = false;
		while(!success) {
			int res = inputPass.Create(_MainOutputWindow, TEXT("NppFTP: Master password required"), query, TEXT(""));
			if (res == 1) {
				char * localPass = SU::TCharToCP(inputPass.GetValue(), CP_ACP);
				char * challenge = Encryption::Decrypt(localPass, -1, passstr, true);

				if (strcmp(challenge, "NppFTP")) {
					query = TEXT("Wrong password.\r\nPlease enter master password");
				} else {
					//Encryption::SetDefaultKey(localPass, -1);
					SetEncryptionKey(localPass);
					success = true;
				}

				Encryption::FreeData(challenge);
				SU::FreeChar(localPass);
			} else {
				break;
			}
		}
		if (!success) {
			MessageBox(_MainOutputWindow,
							TEXT("Incorrect password entered.\r\n")
							TEXT("The passwords will most likely be corrupted and you have to reenter them"),
							TEXT("NppFTP: Password manager error"), MB_OK);
		}

	}

	const char * defaultCacheutf8 = settingsElem->Attribute("defaultCache");
	TCHAR * defaultCache;
	if (defaultCacheutf8) {
		defaultCache = SU::Utf8ToTChar(defaultCacheutf8);
		SetGlobalCachePath(defaultCache);
		SU::FreeTChar(defaultCache);
	}

	int clearState = 0;
	const char * clearstr = settingsElem->Attribute("clearCache", &clearState);
	if (!clearstr) {
		clearState = 0;
	}
	m_clearCache = (clearState != 0);

	int debugModeState = 0;
	const char * debugModeStr = settingsElem->Attribute("debugMode", &debugModeState);
	if (!debugModeStr) {
		debugModeState = 0;
	}
	m_debugMode = (debugModeState != 0);


	clearState = 0;
	clearstr = settingsElem->Attribute("clearCachePermanent", &clearState);
	if (!clearstr) {
		clearState = 0;
	}
	m_clearCachePermanent = (clearState != 0);

	return 0;
}

int FTPSettings::SaveSettings(TiXmlElement * settingsElem) const {
	char * defaultCacheutf8 = SU::TCharToUtf8(m_globalCachePath);
	settingsElem->SetAttribute("defaultCache", defaultCacheutf8);
	SU::FreeChar(defaultCacheutf8);

	settingsElem->SetAttribute("outputShown", m_showOutput?1:0);
	settingsElem->SetDoubleAttribute("windowRatio", m_splitRatio);

	if (!Encryption::IsDefaultKey()) {
		char * challenge = Encryption::Encrypt(NULL, -1, "NppFTP", -1);
		settingsElem->SetAttribute("MasterPass", challenge);
		Encryption::FreeData(challenge);
	}
	settingsElem->SetAttribute("debugMode", m_debugMode?1:0);
	settingsElem->SetAttribute("clearCache", m_clearCache?1:0);
	settingsElem->SetAttribute("clearCachePermanent", m_clearCachePermanent?1:0);

	return 0;
}
