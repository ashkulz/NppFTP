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

#ifndef SSLCERTIFICATES_H
#define SSLCERTIFICATES_H

#include <openssl/x509.h>

struct DER {
	unsigned char * data;
	int len;
};

typedef std::vector<DER> vDER;
typedef std::vector<const X509 *> vX509;

class SSLCertificates {
public:
	static TiXmlElement*	SaveDER(const vDER & derVect);
	static vDER				LoadDER(const TiXmlElement * dersElem);

	static vX509			ConvertDERVector(const vDER & derVect);
	static vDER				ConvertX509Vector(const vX509 & x509Vect);

	static X509*			ConvertDER(const DER & der);
	static DER				ConvertX509(const X509 * x509);

	static int				FreeDERVector(vDER & derVect);
	static int				FreeX509Vector(vX509 & x509Vect);

	static int				FreeDER(DER & der);
	static int				FreeX509(X509 * x509);

	static const char*		DERsElem;
private:
public:
	static char *			GetDERString(const DER & der);
	static DER				GetDER(const char * derString);
	static int				FreeDERString(const char * derString);
};

#endif //SSLCERTIFICATES_H
