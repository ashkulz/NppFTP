// =================================================================
//  class: CUT_StringList
//  File:  utstrlst.h
//  
//  Purpose:
//
//  Internal string manipulation for use with Ultimate TCP/IP  
//  classes.
//
//  declares:   CUT_StrMethods
//              CUT_StringList
//              CUT_TStringList
//
// =================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
// =================================================================

#ifndef UTSTLST_H
#define UTSTLST_H

#include <stdlib.h>
#include "uterr.h"


// =================================================================
//  CUT_StrMethods class
//
//  Helper class with several static string manupulation functions
// =================================================================

class CUT_StrMethods {

// Private ctor & dtor
private:
        CUT_StrMethods() {};
        ~CUT_StrMethods() {};

public:
        // String functions
        static void RemoveCRLF(LPSTR buf);
        static void RemoveCRLF(LPWSTR buf);

        // Does the buffer end with carriage return 
        static BOOL IsWithCRLF(LPCSTR buf);
        static BOOL IsWithCRLF(LPCWSTR buf);

        // Parses the given string into its components based on the 
        // separation character(s)
        static int  ParseString(LPCSTR string, LPCSTR sepChars, int index, LPSTR buf, int buflen, char chStringQualifier = 0);
        static int  ParseString(LPCWSTR string, LPCWSTR sepChars, int index, LPWSTR buf, int buflen, WCHAR chStringQualifier = 0);
        
        // Extracts a component from the given string at the given index
        // based on the separation character(s) 
        static int  ParseString(LPCSTR string, LPCSTR sepChars, int index, long *value);
        static int  ParseString(LPCWSTR string, LPCWSTR sepChars, int index, long *value);

        // Returns the number of components found in the given 
        // string when using the given list of separation characters
        static int  GetParseStringPieces(LPCSTR string, LPCSTR sepChars);
        static int  GetParseStringPieces(LPCWSTR string, LPCWSTR sepChars);

        // Removes spaces from the beginning and end of the string
        static void RemoveSpaces(LPSTR szString); 
        static void RemoveSpaces(LPWSTR szString); 

};


// =================================================================
//  CUT_StringList class
//
//  Double linked string list (Ascii)
// =================================================================
class CUT_StringList {

private:
    
    typedef struct UT_StringListTag{
        LPSTR           lpszString;
        UT_StringListTag *pNext;
        UT_StringListTag *pPrev;
    }UT_StringList;

    UT_StringList *m_list;

public:
    CUT_StringList();
    CUT_StringList(const CUT_StringList& strlist);
    virtual ~CUT_StringList();

    // Assigment operator
    CUT_StringList &operator=(const CUT_StringList strlist);
    
    // add a new string to the doubly linked list of strings
    BOOL AddString(LPCSTR lpszString);

    // clears the list
    BOOL ClearList();

    // get the number of items in the linked list
    long GetCount() const;

    // delete a string by index
    BOOL DeleteString(long lIndex);
    
    // retrive the string associated with the index specified
    LPCSTR GetString(long lIndex) const;

    // retrive the first string containing substring
    LPCSTR GetString(LPCSTR szSubString, int *nPos = NULL) const;

    BOOL Exists(LPCSTR szfindString);
};

// =================================================================
//  CUT_TStringList class
//
//  Double linked string list (TCHAR)
// =================================================================
class CUT_TStringList {

private:
    
    typedef struct UT_TStringListTag{
        LPTSTR          lpszString;
        UT_TStringListTag *pNext;
        UT_TStringListTag *pPrev;
    }UT_TStringList;

    UT_TStringList *m_list;

public:
    CUT_TStringList();
    CUT_TStringList(const CUT_TStringList& strlist);
    virtual ~CUT_TStringList();

    // Assigment operator
    CUT_TStringList &operator=(const CUT_TStringList strlist);
    
    // add a new string to the doubly linked list of strings
    BOOL AddString(LPCTSTR lpszString);

    // clears the list
    BOOL ClearList();

    // get the number of items in the linked list
    long GetCount() const;

    // delete a string by index
    BOOL DeleteString(long lIndex);
    
    // retrive the string associated with the index specified
    LPCTSTR GetString(long lIndex) const;

    // retrive the first string containing substring
    LPCTSTR GetString(LPCTSTR szSubString, int *nPos = NULL) const;

    BOOL Exists(LPCTSTR szfindString);
};


#endif
