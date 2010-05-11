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
#include "SSLCertificates.h"

const char * SSLCertificates::DERsElem = "Certificates";

TiXmlElement* SSLCertificates::SaveDER(const vDER & derVect) {
	TiXmlElement * dersElem = new TiXmlElement(DERsElem);
	for(size_t i = 0; i < derVect.size(); i++) {
		TiXmlElement * derElem = new TiXmlElement("Certificate");
		char * derString = GetDERString(derVect[i]);
		derElem->SetAttribute("DER", derString);
		FreeDERString(derString);
		dersElem->LinkEndChild(derElem);
	}

	return dersElem;
}

vDER SSLCertificates::LoadDER(const TiXmlElement * dersElem) {
	vDER ders;

	if (!dersElem)
		return ders;

	if ( strcmp(DERsElem, dersElem->Value()) )
		return ders;

	const TiXmlElement* child = dersElem->FirstChildElement("Certificate");

	for( ; child; child = child->NextSiblingElement("Certificate") )
	{
		const char * derString = child->Attribute("DER");
		if (!derString)
			continue;
		DER der = GetDER(derString);
		ders.push_back(der);
	}

	return ders;
}

vX509 SSLCertificates::ConvertDERVector(const vDER & derVect) {
	vX509 x509Vect;
	for(size_t i = 0; i < derVect.size(); i++) {
		X509 * x509 = ConvertDER(derVect[i]);
		if (x509 == NULL)
			continue;

		x509Vect.push_back(x509);
	}

	return x509Vect;
}

vDER SSLCertificates::ConvertX509Vector(const vX509 & x509Vect) {
	vDER derVect;
	for(size_t i = 0; i < x509Vect.size(); i++) {
		DER der = ConvertX509(x509Vect[i]);
		if (der.len == 0)
			continue;

		derVect.push_back(der);
	}

	return derVect;
}

X509* SSLCertificates::ConvertDER(const DER & der) {
	//Assuming openssl 0.9.7 or higher
	X509 * x509 = NULL;

	const unsigned char ** buf = (const unsigned char **)&(der.data);
	x509 = d2i_X509(NULL, buf, der.len);

	return x509;
}

DER SSLCertificates::ConvertX509(const X509 * x509) {
	//Assuming openssl 0.9.7 or higher
	DER der;
	unsigned char * derBuffer = NULL;
	int len = 0;

	len = i2d_X509((X509*)x509, &derBuffer);

	der.len = len;
	der.data = derBuffer;

	return der;
}

int SSLCertificates::FreeDERVector(vDER & derVect) {
	for(size_t i = 0; i < derVect.size(); i++) {
		FreeDER(derVect[i]);
	}
	derVect.clear();

	return 0;
}

int SSLCertificates::FreeX509Vector(vX509 & x509Vect) {
	for(size_t i = 0; i < x509Vect.size(); i++) {
		FreeX509((X509*)x509Vect[i]);
	}
	x509Vect.clear();

	return 0;
}

int SSLCertificates::FreeDER(DER & der) {
	//delete [] der;
	OPENSSL_free(der.data);
	return 0;
}

int SSLCertificates::FreeX509(X509 * x509) {
	X509_free(x509);
	return 0;
}

char * SSLCertificates::GetDERString(const DER & der) {
	static const char * table = "0123456789ABCDEF";

	char * derString = new char[der.len*2+2+300];
	for(int i = 0; i < der.len; i++) {
		derString[i*2] = table[(unsigned int)der.data[i]/16];
		derString[i*2+1] = table[(unsigned int)der.data[i]%16];
	}
	derString[der.len*2] = 0;

	return derString;
}

DER SSLCertificates::GetDER(const char * derString) {
	DER der;
	size_t len = strlen(derString)/2;
	unsigned char * data = (unsigned char*)OPENSSL_malloc(len*sizeof(unsigned char));//new unsigned char[len];

	for(size_t i = 0; i < len; i++) {
		data[i] = 0;

		if (derString[i*2] <= '9')
			data[i] += (derString[i*2] - '0') * 16;
		else
			data[i] += ((derString[i*2] - 'A') + 10) * 16;

		if (derString[i*2+1] <= '9')
			data[i] += (derString[i*2+1] - '0');
		else
			data[i] += (derString[i*2+1] - 'A') + 10;
	}

	der.len = len;
	der.data = data;

	return der;
}

int SSLCertificates::FreeDERString(const char * derString) {
	delete [] derString;
	return 0;
}
