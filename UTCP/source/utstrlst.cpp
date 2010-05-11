//=================================================================
//  class: CUT_StringList
//  File:  utstrlst.cpp
//
//  Purpose:
//
//  CUT_StringList class
//
//  Internal string manipulation for use with Ultimate TCP/IP
//  classes
//
//	defines:	CUT_StrMethods
//				CUT_StringList
//				CUT_TStringList
//
//=================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
//=================================================================

/*
NppFTP:
Modification made April 2010:
-remove pragma statements
*/

#include "stdafx.h"

#include "utstrlst.h"

#include "ut_strop.h"


// =================================================================
//	CUT_StrMethods class
//
//	Helper class with several static string manupulation functions
// =================================================================

/***************************************************
RemoveCRLF
    Removes the '\r\n' pair from the given string
    if found
Params
    buf - string to remove the '\r\n' from
Return
    none
****************************************************/
void CUT_StrMethods::RemoveCRLF(LPSTR buf)
{
	// v4.2 changed to size_t
	size_t		len, indx = 1;
	if(buf != NULL){
		len = strlen(buf);
		while((len - indx) >= 0 && indx <= 2) {
			if(buf[len - indx] == '\r' || buf[len - indx] == '\n')
				buf[len - indx] = 0;
			++indx;
		}
	}
}
/***************************************************
RemoveCRLF
	Overload for wide char
****************************************************/
void CUT_StrMethods::RemoveCRLF(LPWSTR buf)
{
	// v4.2 changed to size_t
	size_t		len, indx = 1;
	if(buf != NULL){
		len = wcslen(buf);
		while((len - indx) >= 0 && indx <= 2) {
			if(buf[len - indx] == _T('\r') || buf[len - indx] == _T('\n'))
				buf[len - indx] = 0;
			++indx;
		}
	}
}
/***************************************************
IsWithCRLF
    checks to see if the given string ends with a
    CRLF
Params
    none
Return
    TRUE - if a CRLF appears at the end of the string
    FALSE - if no CRLF appears
****************************************************/
BOOL CUT_StrMethods::IsWithCRLF(LPCSTR buf)
{
	BOOL retval = FALSE;
	if(buf != NULL) {
		// v4.2 changed to size_t
		size_t len = strlen(buf);
		if(buf[len-1] == '\r' || buf[len-1] == '\n')
			retval =  TRUE;
	}
	return retval;
}
/***************************************************
IsWithCRLF
	Overload for wide char
****************************************************/
BOOL CUT_StrMethods::IsWithCRLF(LPCWSTR buf)
{
	BOOL retval = FALSE;
	if(buf != NULL) {
		// v4.2 changed to size_t
		size_t len = wcslen(buf);
		if(buf[len-1] == _T('\r') || buf[len-1] == _T('\n'))
			retval =  TRUE;
	}
	return retval;
}
/***************************************************
ParseString
    Parses the given string into its components.
    A component is defined as the data found between
    the given separation characters in the sepChars
    string.
Params
    string				- string to parse
    sepChars			- characters to use as the separations
							between each of the string components.
    index				- the zero based index of the component that
							is to be returned
    buf					- the buffer to return the component in
    buflen				- the length of the return buffer
	chStringQualifier	- a character to indicate that the start and end of a string (ex: quote)
Return
    UTE_SUCCESS	- success
    UTE_ERROR	- error
****************************************************/
int CUT_StrMethods::ParseString(LPCSTR string,LPCSTR sepChars,int index,
    LPSTR buf,int buflen, char chStringQualifier){

    int t,x;
    int strLen ;
    int sepLen ;
    int piece = FALSE;
    int startPos =0;
    int endPos = 0;
    int currentIndex = 0;
    int found = FALSE;

    if(buflen >0)
        buf[0]=0;
    if (string == NULL)
        return UTE_ERROR;

    if (sepChars == NULL)
        return UTE_ERROR;

	bool bInsideString = false; // Indicates if we are inside a string or not.
								// When we are inside string we do not look for
								// separator characters

    strLen= (int)strlen(string);
    sepLen  = (int)strlen(sepChars);
    //go through the string to find the param at the given index
    for(t=0;t<=strLen;t++)
	{
		if (chStringQualifier != 0 && string[t] == chStringQualifier)
			bInsideString = !bInsideString; // toggle inside string

		//check for separation chars
		for(x=0;x<sepLen;x++)
		{
			if((string[t] == sepChars[x] && !bInsideString) || string[t] == 0)
			{
				if(piece)
				{
					endPos = t -1;
					if(currentIndex == index)
					{
						found = TRUE;
						break;
					}
					currentIndex++;
				}
				piece = FALSE;
				break;
			}
		}

        if(found == TRUE)
            break;

        //if none is found then it must be a piece
        if(x == sepLen)
		{
            if(!piece)
			{
                startPos = t;
            }
            piece = TRUE;
        }
    }
    if(found)
	{
        if((endPos - startPos) >= buflen)
            return UTE_ERROR;
        x=0;
        for(t=startPos;t<=endPos;t++)
		{
            buf[x] = string[t];
            x++;
        }
        buf[x]=0;
        return UTE_SUCCESS;
    }

    return UTE_ERROR;
}
/***************************************************
ParseString
    Overload for wide char
****************************************************/
// v4.2 this code should be tightened up.
int CUT_StrMethods::ParseString(LPCWSTR string,LPCWSTR sepChars,int index,
    LPWSTR buf,int buflen, WCHAR chStringQualifier){

    int t,x;
    int strLen ;
    int sepLen ;
    int piece = FALSE;
    int startPos =0;
    int endPos = 0;
    int currentIndex = 0;
    int found = FALSE;

    if(buflen >0)
        buf[0]=0;
    if (string == NULL)
        return UTE_ERROR;

    if (sepChars == NULL)
        return UTE_ERROR;

	bool bInsideString = false; // Indicates if we are inside a string or not.
								// When we are inside string we do not look for
								// separator characters

    strLen= (int)wcslen(string);
    sepLen  = (int)wcslen(sepChars);
    //go through the string to find the param at the given index
    for(t=0;t<=strLen;t++)
	{
		if (chStringQualifier != 0 && string[t] == chStringQualifier)
			bInsideString = !bInsideString; // toggle inside string

		//check for separation chars
		for(x=0;x<sepLen;x++)
		{
			if((string[t] == sepChars[x] && !bInsideString) || string[t] == 0)
			{
				if(piece)
				{
					endPos = t -1;
					if(currentIndex == index)
					{
						found = TRUE;
						break;
					}
					currentIndex++;
				}
				piece = FALSE;
				break;
			}
		}

        if(found == TRUE)
            break;

        //if none is found then it must be a piece
        if(x == sepLen)
		{
            if(!piece)
			{
                startPos = t;
            }
            piece = TRUE;
        }
    }
    if(found)
	{
        if((endPos - startPos) >= buflen)
            return UTE_ERROR;
        x=0;
        for(t=startPos;t<=endPos;t++)
		{
            buf[x] = string[t];
            x++;
        }
        buf[x]=0;
        return UTE_SUCCESS;
    }

    return UTE_ERROR;
}

/***************************************************
ParseString
    Parses the given string into its components.
    A component is defined as the data found between
    the given separation characters in the sepChars
    string.
Params
    string - string to parse
    sepChars - characters to use as the separations
        between each of the string components.
    index - the zero based index of the component that
        is to be returned
    value - a pointer to a long value to return the
        value found in
Return
    UTE_SUCCESS	- success
    UTE_ERROR	- error
****************************************************/
int CUT_StrMethods::ParseString(LPCSTR string,LPCSTR sepChars,int index,long *value){

    char buf[60];
    if(ParseString(string,sepChars,index,buf,60) == UTE_SUCCESS){
        *value = atol(buf);
        return UTE_SUCCESS;
    }
    return UTE_ERROR;
}
/***************************************************
ParseString
    Overload for wide char
****************************************************/
int CUT_StrMethods::ParseString(LPCWSTR string,LPCWSTR sepChars,int index,long *value){

    WCHAR buf[60];
    if(ParseString(string,sepChars,index,buf,60) == UTE_SUCCESS){
        *value = _wtol(buf);
        return UTE_SUCCESS;
    }
    return UTE_ERROR;
}

/***************************************************
GetParseStringPieces
    Returns the number of components found in the given
    string when using the given list of separation
    characters
Params
    string - string to parse
    sepChars - the list of separation characters
Return
    the number of components found
****************************************************/
int CUT_StrMethods::GetParseStringPieces(LPCSTR string,LPCSTR sepChars){

    int t,x;
    int strLen = (int)strlen(string);
    int sepLen = (int)strlen(sepChars);
    int count = 0;
    int piece = FALSE;

    for(t=0;t<strLen;t++){
        for(x=0;x<sepLen;x++){
            if(string[t] == sepChars[x]){
				if(piece){
                    count++;
				}
                piece = FALSE;
                break;
            }
        }
		if(x == sepLen) {
            piece = TRUE;
		}
    }
	if(piece){
        count++;
	}
    return count;
}
/***************************************************
GetParseStringPieces
    Overload for wide char
****************************************************/
int CUT_StrMethods::GetParseStringPieces(LPCWSTR string,LPCWSTR sepChars){

    int t,x;
    int strLen = (int)wcslen(string);
    int sepLen = (int)wcslen(sepChars);
    int count = 0;
    int piece = FALSE;

    for(t=0;t<strLen;t++){
        for(x=0;x<sepLen;x++){
            if(string[t] == sepChars[x]){
				if(piece){
                    count++;
				}
                piece = FALSE;
                break;
            }
        }
		if(x == sepLen){
            piece = TRUE;
		}
    }
	if(piece){
        count++;
	}
    return count;
}

/*********************************************
RemoveSpaces
	Removes leading and trialing spaces from
	the given string
Params
	szString - the string to trim the spaces from
Return
	None
**********************************************/
void CUT_StrMethods::RemoveSpaces(LPSTR szString){
	if(szString != NULL){
		// v4.2 changed to size_t
		size_t loop, len = strlen(szString);
		// Remove the trailing spaces
		for(loop = (len-1); loop >= 0; loop--) {
			if(szString[loop] != ' ')
				break;
		}
		if(loop < (len-1)) {
			szString[loop + 1] = 0;
			len = loop + 1;
		}
		// Remove the leading spaces
		for(loop = 0; loop < len ; loop++){
			if(szString[loop] != ' ')
				break;
		}
		// If there were leading spaces then shift the chars over
		if(loop > 0){
			memmove(szString,&szString[loop],len-loop+1);
		}
	}
}
/*********************************************
RemoveSpaces
	Overload for wide char
**********************************************/
void CUT_StrMethods::RemoveSpaces(LPWSTR szString){
	if(szString != NULL){
		// v4.2 changed to size_t
		size_t loop, len = wcslen(szString);
		// Remove the trailing spaces
		for(loop = (len-1); loop >= 0; loop--) {
			if(szString[loop] != _T(' '))
				break;
		}
		if(loop < (len-1)) {
			szString[loop + 1] = 0;
			len = loop + 1;
		}
		// Remove the leading spaces
		for(loop = 0; loop < len ; loop++){
			if(szString[loop] != _T(' '))
				break;
		}
		// If there were leading spaces then shift the chars over
		if(loop > 0){
			memmove(szString,&szString[loop],(len-loop+1) * sizeof(wchar_t));
		}
	}
}


// =================================================================
//	CUT_StringList class
//
//	Double linked string list
// =================================================================

/****************************************
 Constructor
*****************************************/
CUT_StringList::CUT_StringList() : m_list(NULL)
{
}

/****************************************
 Copy constructor
*****************************************/
CUT_StringList::CUT_StringList(const CUT_StringList& strlist) : m_list(NULL)
{
	LPCSTR	ptrItem;
	for(int i=0; i < strlist.GetCount(); i++)
		if((ptrItem = strlist.GetString(i)) != NULL)
			AddString(ptrItem);
}

/****************************************
 Destructor
*****************************************/
CUT_StringList::~CUT_StringList()
{
    ClearList();
}

/****************************************
 Assigment operator
*****************************************/
CUT_StringList &CUT_StringList::operator=(const CUT_StringList strlist)
{
	if(this == &strlist)	return *this;

	ClearList();

	LPCSTR	ptrItem;
	for(int i=0; i < strlist.GetCount(); i++)
		if((ptrItem = strlist.GetString(i)) != NULL)
			AddString(ptrItem);

	return *this;
}

/****************************************
 add a new string to the doubly linked list of strings
 Return:
    TRUE
*****************************************/
BOOL CUT_StringList::AddString(LPCSTR lpszString)
{

    if(m_list == NULL) {						// If list is empty
        m_list = new UT_StringList;
        m_list->lpszString = new char[strlen(lpszString) + 1];
        m_list->pNext = m_list->pPrev = NULL;
        strcpy(m_list->lpszString, lpszString);	// Copy string
		}
	else {										// If list is not empty

	    UT_StringList	*pItem = m_list;

		while(pItem->pNext != NULL)				// Search for the last item in the list
			pItem = pItem->pNext;

		pItem->pNext = new UT_StringList;
		pItem->pNext->pPrev = pItem;
		pItem = pItem->pNext;
		pItem->lpszString = new char[strlen(lpszString) + 1];
		pItem->pNext = NULL;
		strcpy(pItem->lpszString, lpszString);	// Copy string
		}

    return TRUE;
}

/****************************************
    clears the list
*****************************************/
BOOL CUT_StringList::ClearList()
{

    if(m_list == NULL)							// List is already cleared
        return TRUE;

    UT_StringList	*pNextItem;

    while(m_list != NULL) {
        pNextItem = m_list->pNext;
        delete[] m_list->lpszString;
        delete m_list;
        m_list = pNextItem;
	    }

    return TRUE;
}

/****************************************
    get the number of items in the
    linked list
*****************************************/
long CUT_StringList::GetCount() const
{

    if(m_list == NULL)							// If list is empty
        return 0;

    UT_StringList	*pNextItem = m_list;
	long			lCount = 0;

    while(pNextItem != NULL){
        lCount ++;
        pNextItem = pNextItem->pNext;
    }

    return lCount;
}

/****************************************
    delete a string by index
*****************************************/
BOOL CUT_StringList::DeleteString(long lIndex)
{

    if(m_list == NULL)
        return FALSE;

    UT_StringList	*pItem = m_list;
    long			lCount = 0;

    while(pItem != NULL) {
        if(lCount == lIndex) {
            if(pItem->pPrev == NULL) {
                m_list = pItem->pNext;
				if(m_list != NULL)
					m_list->pPrev = NULL;
				}
            else {
				pItem->pPrev->pNext = pItem->pNext;
				if(pItem->pNext != NULL)
					pItem->pNext->pPrev = pItem->pPrev;
				}

            delete[] pItem->lpszString;
            delete pItem;

            return TRUE;
			}

        lCount ++;
        pItem = pItem->pNext;
		}

    return FALSE;
}

/****************************************
    retrieve the string associated with
    the index specified
*****************************************/
LPCSTR CUT_StringList::GetString(long lIndex) const
{

    if(m_list == NULL)
        return NULL;

	UT_StringList	*pItem = m_list;
    long			lCount = 0;


    while(pItem != NULL){
        if(lCount == lIndex)
            return pItem->lpszString;
        lCount ++;
        pItem = pItem->pNext;
    }

    return NULL;
}

/****************************************
    retrieve the first string that contains
	the specified substring
*****************************************/
LPCSTR CUT_StringList::GetString(LPCSTR szSubString, int *nPos) const
{
	UT_StringList	*pItem = m_list;
	LPSTR			ptrStr;

    if(m_list == NULL || szSubString == NULL)
        return NULL;

    while(pItem != NULL) {
        if((ptrStr = strstr(pItem->lpszString, szSubString)) != NULL) {
			if(nPos != NULL)
				*nPos = (int)strlen(pItem->lpszString) - (int)strlen(ptrStr);
            return pItem->lpszString;
			}
        pItem = pItem->pNext;
	    }

    return NULL;
}

/****************************************
checks to see if the given string exists
in the list
*****************************************/
BOOL CUT_StringList::Exists(LPCSTR szfindString){

	UT_StringList	*pItem = m_list;

    if(m_list == NULL || szfindString == NULL)
        return FALSE;

    while(pItem != NULL) {
        if(strcmp(pItem->lpszString, szfindString) == 0)
			return TRUE;
        pItem = pItem->pNext;
	}
    return FALSE;
}

// =================================================================
//	CUT_TStringList class
//
//	Double linked string list
// =================================================================

/****************************************
 Constructor
*****************************************/
CUT_TStringList::CUT_TStringList() : m_list(NULL)
{
}

/****************************************
 Copy constructor
*****************************************/
CUT_TStringList::CUT_TStringList(const CUT_TStringList& strlist) : m_list(NULL)
{
	LPCTSTR	ptrItem;
	for(int i=0; i < strlist.GetCount(); i++)
		if((ptrItem = strlist.GetString(i)) != NULL)
			AddString(ptrItem);
}

/****************************************
 Destructor
*****************************************/
CUT_TStringList::~CUT_TStringList()
{
    ClearList();
}

/****************************************
 Assigment operator
*****************************************/
CUT_TStringList &CUT_TStringList::operator=(const CUT_TStringList strlist)
{
	if(this == &strlist)	return *this;

	ClearList();

	LPCTSTR	ptrItem;
	for(int i=0; i < strlist.GetCount(); i++)
		if((ptrItem = strlist.GetString(i)) != NULL)
			AddString(ptrItem);

	return *this;
}

/****************************************
 add a new string to the doubly linked list of strings
 Return:
    TRUE
*****************************************/
BOOL CUT_TStringList::AddString(LPCTSTR lpszString)
{

    if(m_list == NULL) {						// If list is empty
        m_list = new UT_TStringList;
        m_list->lpszString = new _TCHAR[_tcslen(lpszString) + 1];
        m_list->pNext = m_list->pPrev = NULL;
        _tcscpy(m_list->lpszString, lpszString);	// Copy string
		}
	else {										// If list is not empty

	    UT_TStringList	*pItem = m_list;

		while(pItem->pNext != NULL)				// Search for the last item in the list
			pItem = pItem->pNext;

		pItem->pNext = new UT_TStringList;
		pItem->pNext->pPrev = pItem;
		pItem = pItem->pNext;
		pItem->lpszString = new _TCHAR[_tcslen(lpszString) + 1];
		pItem->pNext = NULL;
		_tcscpy(pItem->lpszString, lpszString);	// Copy string
		}

    return TRUE;
}

/****************************************
    clears the list
*****************************************/
BOOL CUT_TStringList::ClearList()
{

    if(m_list == NULL)							// List is already cleared
        return TRUE;

    UT_TStringList	*pNextItem;

    while(m_list != NULL) {
        pNextItem = m_list->pNext;
        delete[] m_list->lpszString;
        delete m_list;
        m_list = pNextItem;
	    }

    return TRUE;
}

/****************************************
    get the number of items in the
    linked list
*****************************************/
long CUT_TStringList::GetCount() const
{

    if(m_list == NULL)							// If list is empty
        return 0;

    UT_TStringList	*pNextItem = m_list;
	long			lCount = 0;

    while(pNextItem != NULL){
        lCount ++;
        pNextItem = pNextItem->pNext;
    }

    return lCount;
}

/****************************************
    delete a string by index
*****************************************/
BOOL CUT_TStringList::DeleteString(long lIndex)
{

    if(m_list == NULL)
        return FALSE;

    UT_TStringList	*pItem = m_list;
    long			lCount = 0;

    while(pItem != NULL) {
        if(lCount == lIndex) {
            if(pItem->pPrev == NULL) {
                m_list = pItem->pNext;
				if(m_list != NULL)
					m_list->pPrev = NULL;
				}
            else {
				pItem->pPrev->pNext = pItem->pNext;
				if(pItem->pNext != NULL)
					pItem->pNext->pPrev = pItem->pPrev;
				}

            delete[] pItem->lpszString;
            delete pItem;

            return TRUE;
			}

        lCount ++;
        pItem = pItem->pNext;
		}

    return FALSE;
}

/****************************************
    retrieve the string associated with
    the index specified
*****************************************/
LPCTSTR CUT_TStringList::GetString(long lIndex) const
{

    if(m_list == NULL)
        return NULL;

	UT_TStringList	*pItem = m_list;
    long			lCount = 0;


    while(pItem != NULL){
        if(lCount == lIndex)
            return pItem->lpszString;
        lCount ++;
        pItem = pItem->pNext;
    }

    return NULL;
}

/****************************************
    retrieve the first string that contains
	the specified substring
*****************************************/
LPCTSTR CUT_TStringList::GetString(LPCTSTR szSubString, int *nPos) const
{
	UT_TStringList	*pItem = m_list;
	LPTSTR			ptrStr;

    if(m_list == NULL || szSubString == NULL)
        return NULL;

    while(pItem != NULL) {
        if((ptrStr = _tcsstr(pItem->lpszString, szSubString)) != NULL) {
			if(nPos != NULL)
				*nPos = (int)_tcslen(pItem->lpszString) - (int)_tcslen(ptrStr);
            return pItem->lpszString;
			}
        pItem = pItem->pNext;
	    }

    return NULL;
}

/****************************************
checks to see if the given string exists
in the list
*****************************************/
BOOL CUT_TStringList::Exists(LPCTSTR szfindString){

	UT_TStringList	*pItem = m_list;

    if(m_list == NULL || szfindString == NULL)
        return FALSE;

    while(pItem != NULL) {
        if(_tcscmp(pItem->lpszString, szfindString) == 0)
			return TRUE;
        pItem = pItem->pNext;
	}
    return FALSE;
}
