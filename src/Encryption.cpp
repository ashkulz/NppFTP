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
#include "Encryption.h"

#include <openssl/des.h>

//The default key is initialzed to this. It is pretty much the same as saving passwords plaintext,
//though 8 year olds can't read it then
char * Encryption::_DefaultKey = NULL;
bool Encryption::_IsDefaultKey = true;
const char * defaultString = "NppFTP00";	//must be 8 in length
//const size_t Encryption::KeySize = 8;

int Encryption::Init() {
	_DefaultKey = new char[KeySize];
	strncpy(_DefaultKey, defaultString, KeySize);
	return 0;
}

int Encryption::Deinit() {
	if (_DefaultKey)
		delete [] _DefaultKey;
	_DefaultKey = NULL;
	return 0;
}

char* Encryption::Encrypt(const char * key, int keysize, const char * data, int size) {
	if (size == -1)
		size = strlen(data);

	char * encdata = DES_encrypt(key, keysize, data, size, false, DES_ENCRYPT);
	if (!encdata)
		return NULL;
	char * hexData = SU::DataToHex(encdata, size);
	delete [] encdata;
	return hexData;
}

char* Encryption::Decrypt(const char * key, int keysize, const char * data, bool addZero) {
	int size = strlen(data);
	char * encrdata = SU::HexToData(data, size, false);
	if (!encrdata)
		return NULL;

	size = size/2;
	char * decdata = DES_encrypt(key, keysize, encrdata, size, addZero, DES_DECRYPT);
	SU::FreeChar(encrdata);

	return decdata;
}

int Encryption::FreeData(char * data) {
	return SU::FreeChar(data);
}

int Encryption::SetDefaultKey(const char * defKey, int size) {
	if (size == -1)
		size = strlen(defKey);
	if (size == 0) {
		_IsDefaultKey = true;
		defKey = defaultString;	//cannot allow empty strings
		size = 8;
	} else {
		_IsDefaultKey = false;
	}
	if ((size_t)size > KeySize)
		size = KeySize;

	memcpy(_DefaultKey, defKey, size);
	for(size_t i = size; i < KeySize; i++) {
		_DefaultKey[i] = 0;	//fill rest with zeroes
	}
	return 0;
}

const char* Encryption::GetDefaultKey() {
	return _DefaultKey;
}

bool Encryption::IsDefaultKey() {
	return _IsDefaultKey;
}

char* Encryption::DES_encrypt(const char * key, int keysize, const char * data, int size, bool addZero, int type) {
	char keybuf[KeySize];
	if (key == NULL) {
		memcpy(keybuf, _DefaultKey, KeySize);
		keysize = KeySize;
	} else {
		if (keysize == -1)
			keysize = strlen(key);	//zero terminator NOT included
		if (keysize > KeySize)
			keysize = KeySize;
		memcpy(keybuf, key, keysize);
		for(size_t i = keysize; i < KeySize; i++)
			keybuf[i] = 0;
	}

	if (size == -1)
		size = strlen(data);

	char * decrypted = new char[size+(addZero?1:0)];
	if (!decrypted)
		return NULL;

	if (addZero)
		decrypted[size] = 0;

	// Prepare the key for use with DES_cfb64_encrypt
	DES_cblock key2;
	DES_key_schedule schedule;
	memcpy(key2, keybuf, KeySize);
	DES_set_odd_parity(&key2);
	DES_set_key_checked(&key2, &schedule);

	// Decryption occurs here
	int n = 0;
	DES_cfb64_encrypt((unsigned char*) data, (unsigned char *)decrypted, size, &schedule, &key2, &n, type);

	return decrypted;
}
