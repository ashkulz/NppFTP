// ===================================================================
//      File: UTExtErr.h
// 
//   Purpose: Provide a way to add error constants and error string 
//              associations without changing the CUT_ERR class. You
//              can copy this file to your project and add only project
//              specific extended errors.                
//
// ===================================================================
// Ultimate TCP/IP v4.2
// This software along with its related components, documentation and files ("The Libraries")
// is © 1994-2007 The Code Project (1612916 Ontario Limited) and use of The Libraries is
// governed by a software license agreement ("Agreement").  Copies of the Agreement are
// available at The Code Project (www.codeproject.com), as part of the package you downloaded
// to obtain this file, or directly from our office.  For a copy of the license governing
// this software, you may contact us at legalaffairs@codeproject.com, or by calling 416-849-8900.
// ===================================================================

#ifndef _EXT_ERR_STRINGS
    // Add your error consatnts here. See example below:
    // UTE_DATABASE_CONNECT_ERROR,
    // UTE_NOT_SUPPORTED,
#else
    // Add your error string associations here. See example below:
    // pStrings[UTE_DATABASE_CONNECT_ERROR] =_T("Database connection error.");
    // pStrings[UTE_NOT_SUPPORTED]          =_T("Operation is not supported.");
#endif