/* =================================================================
//  class: CUT_CramMd5
//  File:  UT_CramMd5.cpp
//  
//  Purpose:
//
//  Challenge-Response Authentication Mechanism 
//  Based on Request for Comments: 2104 
//  
//  HMAC is a  mechanism for message authentication
//  using cryptographic hash functions. HMAC can be used with any
//  iterative cryptographic hash function, e.g., MD5, SHA-1, in
//  combination with a secret shared key
// ===================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
// 
// =================================================================*/
#include "stdafx.h"
#include "ut_cramMd5.h"	

// Suppress warnings for non-safe str fns. Transitional, for VC6 support.
#pragma warning (push)
#pragma warning (disable : 4996)

/********************************
	Constructor
*********************************/
CUT_CramMd5::CUT_CramMd5()
{
	m_szPassword = NULL;
	m_szUserName = NULL;
	m_ChallengeResponse = NULL;
	
}
/********************************
	Destructor
*********************************/
CUT_CramMd5::~CUT_CramMd5()
{	
	if (m_szPassword != NULL)
	{
		memset(m_szPassword,0,strlen(m_szPassword));
		delete m_szPassword;
	}
	if (m_szUserName != NULL)
	{
		memset(m_szUserName,0,strlen(m_szUserName));
		delete m_szUserName;
	}
	if (m_ChallengeResponse != NULL)
	{
		memset(m_ChallengeResponse,0,strlen(m_ChallengeResponse));
		delete m_ChallengeResponse;
	}
	
}
/*******************************************
	Set the user name to be used for the response 
PARAM:
	LPCSTR - a string describing the user name
RET:
	Void
	Not that if the user name is NULL or empty string 
	this function does not set the user name

********************************************/
void CUT_CramMd5::SetUserName(LPCSTR name)
{
	assert (name);
	if (name != NULL)
	{
		if (name[0] != '\0' ) // no empty string allowed
		{
			if (m_szUserName!= NULL) 
				delete m_szUserName;
			m_szUserName = new char [strlen(name)+1];
			strcpy(m_szUserName,name);		
		}
	}
}
/*******************************************
Set the password to be used for the response 
PARAM
	LPCSTR - a string describing the password
	Not that if the Password is NULL or empty string 
	this function does not set the user name
********************************************/
void CUT_CramMd5::SetPassword(LPCSTR pass)
{
	assert (pass != NULL);
	if ( pass != NULL)
	{
		// no empty string allowed
		if (pass[0] != '\0' )
		{
			if (m_szPassword!= NULL)
				delete m_szPassword;
			m_szPassword = new char [strlen(pass)+1];
			strcpy(m_szPassword,pass);		
		}
	}
	
}
/*******************************************
Returns the user name
RET 
	a pointer to the user 
********************************************/
const char * CUT_CramMd5::GetUserName()
{
	return m_szUserName;
	
}
/*******************************************
Returns the user Password
RET 
	a pointer to the Password 
********************************************/
const char *CUT_CramMd5::GetPassword()
{
	return m_szPassword;
	
}
/***********************************************************
	Based on code from
	"HMAC: Keyed-Hashing for Message Authentication"
	RFC 2104

	HMAC can be used in combination with any iterated cryptographic
	hash function. MD5 and SHA-1 are examples of such hash functions.
	HMAC also uses a secret key for calculation and verification of 
	the message authentication values. 
	The main goals behind this construction are:

	* To use, without modifications, available hash functions.
	In particular, hash functions that perform well in software,
	and for which code is freely and widely available.

	* To preserve the original performance of the hash function without
	incurring a significant degradation.

	* To use and handle keys in a simple way.

	* To have a well understood cryptographic analysis of the strength of
	the authentication mechanism based on reasonable assumptions on the
	underlying hash function.

	* To allow for easy replaceability of the underlying hash function in
	case that faster or more secure hash functions are found or
	required.

	Hence, the Keyed MD5 digest is produced by calculating 

	MD5((tanstaaftanstaaf XOR opad), 
	MD5((tanstaaftanstaaf XOR ipad), 
PARAM
	unsigned char* szChallenge - pointer to data stream
	int text_len			  - length of data stream
	unsigned char* key		  - pointer to authentication key 
	MD5 &testContext		  - Refrence to the MD5 context that will hold the 
								result digest
RET
	VOID for now may be we should return errors

***********************************************************/
void CUT_CramMd5::HmacMd5( unsigned char* szChallenge ,  int text_len,  unsigned char* key,  int key_len, MD5 &testContext )
{	
	unsigned  char digest[500];
	MD5 context; 
	unsigned char k_ipad[65];   
	
	// inner padding -
	// * key XORd with ipad
	//	
	unsigned char k_opad[65];
	// outer padding -
	// key XORd with opad
	//	
	BYTE * uszTempKey; // will be pointing to a 16 byte 
	int i;
	// if key is longer than 64 bytes reset it to key=MD5(key) 
	if (key_len > 64)
	{
		
        MD5      tctx;
		// the tctx  is initialized by the constructor
		// So we are not calling it here
		tctx.Update ( key, key_len); 
		tctx.Finalize (); 
		uszTempKey  =  (BYTE *) tctx.GetRawDigest ();
		key = uszTempKey; 
		key_len = 16; 
		delete uszTempKey;
		
	} 
	//
	//	 the HmacMd5 transform looks like:
	//	
	//	 MD5(K XOR opad, MD5(K XOR ipad, text))
	//	
	//	 where K is an n byte key
	//	 ipad is the byte 0x36 repeated 64 times
	//	 opad is the byte 0x5c repeated 64 times
	//	 and text is the data being protected
	//	
	memset( (void *)k_ipad, 0,sizeof k_ipad);
	memset( (void *) k_opad,0, sizeof k_opad);
	strncpy((char *) k_ipad,(const char *)key, key_len);
	strncpy((char *) k_opad, (const char *) key, key_len);
	
	// XOR key with ipad and opad values 
	for (i=0; i<64; i++) 
	{
		k_ipad[i] ^= 0x36;
		k_opad[i] ^= 0x5c;
	}
	//
	//	 perform inner MD5
	//	
	// the context  is initialized by the constructor
	// So we are not calling it here	
	//context.init ();                  
	
	context.Update ( k_ipad, 64);      /* start with inner pad */
	context.Update ( szChallenge, text_len); /* then text of datagram */
	context.Finalize ();          /* finish up 1st pass */
	// this function allocate memory
	// we need to release it when we are done
	uszTempKey = (BYTE *)context.GetRawDigest();
	strcpy((char *)digest ,(const char *)uszTempKey); 
	delete uszTempKey;
	
	
	//	 perform outer MD5
	// init context for 2nd  pass 
	// this is done by the constructor so 
	// we don't call it
	testContext.Update ( k_opad, 64); 

    // start with outer pad
	testContext.Update (  digest, 16);     

	// then results of 1st
	testContext.Finalize ();     
}
/**********************************************************
GetClientResponse()
	Calculates and returns the The client response to the servers challengs
	The authentication type associated with CRAM is "CRAM-MD5".

	The client makes note of the data and then responds with a string
    consisting of the user name, a space, and a 'digest'.  The latter is
    computed by applying the keyed MD5 algorithm from [KEYED-MD5] where
    the key is a shared secret and the digested text is the timestamp
    including angle-brackets).

	This shared secret is a string known only to the client and server.
	The `digest' parameter itself is a 16-octet value which is sent in
	hexadecimal format, using lower-case ASCII characters

    When the server receives this client response, it verifies the digest
    provided.  If the digest is correct, the server should consider the
    client authenticated and respond appropriately.

    Here is an example using IMAP4 client and server comunication
	
	 C: A0001 AUTHENTICATE CRAM-MD5
     S: + PDE4OTYuNjk3MTcwOTUyQHBvc3RvZmZpY2UucmVzdG9uLm1jaS5uZXQ+
     C: dGltIGI5MTNhNjAyYzdlZGE3YTQ5NWI0ZTZlNzMzNGQzODkw

     where the shared secret is in the bove example is  "tanstaaftanstaaf"
	 and the server challenge is in the above example is
	 PDE4OTYuNjk3MTcwOTUyQHBvc3RvZmZpY2UucmVzdG9uLm1jaS5uZXQ+ 
	 which is a base64 mime encoded version of the string
	 <1896.697170952@postoffice.reston.mci.net>

	 This function will first decode the server challenge to a string
	 Then it will use the HmacMd5 function to get the 
	 Keyed-Hashing for Message Authentication value.
	 The shared secret is the password set using SetPassword() function
	 so the result of the HmacMd5 will be 
	 b913a602c7eda7a495b4e6e7334d3890
	 The user name is the string set using SetUserName() function
	 in the above example the user name is "tim".
	 Then the [user name]<space>[HMAC result] will be base64 encoded
	 and returns the result.

	 and there for the result will be for the above example
	 dGltIGI5MTNhNjAyYzdlZGE3YTQ5NWI0ZTZlNzMzNGQzODkw
	 	 
PARAM:
	ServerChallenge - Base64 mime encode server challenge
RET:
	The client response 
	NULL if any of the parameters is missing
***********************************************************/
char *CUT_CramMd5::GetClientResponse(LPCSTR ServerChallenge)
{
	int encodedDataLen = 500;
	CBase64 B64;
	char DecodedData[500];
	BYTE *ptr;
	MD5 testContext;
	char UserPasswordSeed[500];	
	char clientResponse[500];


		// do we have one already?
	if (m_ChallengeResponse != NULL)
		delete m_ChallengeResponse;
	m_ChallengeResponse = NULL;

	// debuging check 
	assert (ServerChallenge != NULL );
	if (ServerChallenge != NULL)
	{
		// no empty string allowed
		if (ServerChallenge[0] !=  '\0' )
		{
			// debuging check 
			assert (m_szPassword != NULL );
			if (m_szPassword != NULL)
			{	
				// debuging check 
				assert (m_szPassword != '\0'); // no empty allowed 
				// no empty string allowed
				if (m_szPassword != '\0')
				{
					// debuging check 
					assert (m_szUserName != NULL );
					if (m_szUserName != NULL)
					{
						// debuging check 
						assert (m_szUserName != '\0'); // no empty allowed 
						// no empty string allowed
						if (m_szUserName != '\0')
						{
							// decode the data received from the server 
							B64.DecodeData ((LPCSTR)ServerChallenge,
								(unsigned char *)DecodedData,
								&encodedDataLen);

							// Null terminate the result
							DecodedData[encodedDataLen] = 0;

							// After we have decoded the challenge received from the server
							// we will construct a n HMAC string using MD5 which is the uthintication we are employing in the 
							// AUTH command
							HmacMd5 ((unsigned char *)DecodedData,encodedDataLen,(unsigned char *)m_szPassword,(int)strlen(m_szPassword),testContext);
							
							// concatinate the hex digest to the user name 
							// add a white space to the user name to delimit the name from the password seed
							// now add the hex degit to the seed
							strcpy(UserPasswordSeed, m_szUserName);
							strcat (UserPasswordSeed, " ");
							// this function will allocate 16 byte
							// we need to claim the memory when we are done with it
							ptr = (BYTE *)testContext.GetDigestInHex ();
							strcat(UserPasswordSeed , (const char *)ptr);
							delete ptr;
							
							//make sure the null terminates the userPassword Seed 
							UserPasswordSeed[499] = 0;

							// now Base64 encode the result to be sent to the server
							B64.EncodeData ((LPCBYTE)UserPasswordSeed,(int)strlen(UserPasswordSeed),clientResponse, 500);

							//make sure the null terminates are there
							clientResponse[499] = 0;
							
							// we need to rturn it so let's send it back
							m_ChallengeResponse = new char [strlen(clientResponse)+1];
							strcpy(m_ChallengeResponse ,clientResponse);	
						}
					}
				}
			}
		}
	}
	// return the challenge response .
	// note it could be NULL if any of the parameters is missing
	return m_ChallengeResponse;
}

#pragma warning ( pop )