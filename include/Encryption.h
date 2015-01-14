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

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

class Encryption {
public:
	static int				Init();
	static int				Deinit();

	//The buffers passed as argument will be modified to contain the encrypted or decrypted content
	static char*			Encrypt(const char * key, int keysize, const char * data, int size);	//results in zero terminated string
	static char*			Decrypt(const char * key, int keysize, const char * data, bool addZero);	//requires Encrypt'ed string

	static int				FreeData(char * data);

	static int				SetDefaultKey(const char * defKey, int size = -1);
	static const char*		GetDefaultKey();	//size of 8, not zero terminated neccessarily
	static bool				IsDefaultKey();

	static const int		KeySize = 8;
private:
	static char*			DES_encrypt(const char * key, int keysize, const char * data, int size, bool addZero, int type);

	static char*			_DefaultKey;
	static bool				_IsDefaultKey;
};

#endif //ENCRYPTION_H
