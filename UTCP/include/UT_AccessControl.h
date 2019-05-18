// =================================================================
//  class: CUT_AccessControl
//  File:  UT_AccessControl.h
//
//  Purpose:
//
//    Server access control class
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

/*
NppFTP modifications:
Removed pragma statements
*/

#ifndef CUT_ACCESS_CONTROL
#define CUT_ACCESS_CONTROL

#include "ut_clnt.h"
#include <stdio.h>
#include <time.h>

#include <list>

#include "UT_CriticalSection.h"

using namespace std;

// ===================================================================
// CUT_IPAddressRange structure.
// ===================================================================
typedef struct CUT_IPAddressRange
{
    CUT_IPAddressRange()
        { m_ipStartAddress.s_addr = 0; m_ipEndAddress.s_addr = 0; }

    in_addr m_ipStartAddress;   // First address of the range
    in_addr m_ipEndAddress;     // Last address of the range
} CUT_IPAddressRange;

// ===================================================================
// CUT_BlockedIPAddr structure.
// ===================================================================
typedef struct CUT_BlockedIPAddr
{
    CUT_BlockedIPAddr()
        { m_ipAddress.s_addr = 0; m_lBlockCount = 0; m_timeBlockExpire = 0; }

    in_addr m_ipAddress;        // IP Address
    long    m_lBlockCount;      // Blocks counter
    time_t  m_timeBlockExpire;  // Block expiration date and time
} CUT_BlockedIPAddr;


// ===================================================================
// enumAccessType Access type enumeration.
// ===================================================================
typedef enum enumAccessType
{
    ACCESS_ALLOWED,
    ACCESS_BLOCKED,
    ACCESS_TEMP_BLOCKED
} enumAccessType;

// ===================================================================
// STL lists definition
// ===================================================================
typedef list<CUT_IPAddressRange>    LIST_IP_ADDRESS_RANGE;
typedef list<CUT_BlockedIPAddr> LIST_BLOCKED_IP_ADDR;


// ===================================================================
// CUT_AccessControl class
// ===================================================================

// v4.2 Methods changed to take _TCHARs
class CUT_AccessControl
{
public:
    // Constructors/Destructor
    CUT_AccessControl();
    virtual ~CUT_AccessControl();

    // Checks if specified IP address is allowed to connect
    virtual BOOL IsAddressAllowed(in_addr &ipAddress);
    // v4.2 split this into char and wide char
    virtual BOOL IsAddressAllowed(LPCSTR lpszAddress);
#if defined _UNICODE
    virtual BOOL IsAddressAllowed(LPCWSTR lpszAddress);
#endif

    // Add address or addresses range to the allowed/blocked lists
    virtual void AddAddress(enumAccessType Type, in_addr &ipStartAddress, in_addr *ipEndAddress = NULL);
    // v4.2 split this into char and wide char
    virtual void AddAddress(enumAccessType Type, LPCSTR lpszStartAddress, LPCSTR lpszEndAddress = NULL);
#if defined _UNICODE
    virtual void AddAddress(enumAccessType Type, LPCWSTR lpszStartAddress, LPCWSTR lpszEndAddress = NULL);
#endif
    // Get address or addresses range from the allowed/blocked list by index
    virtual BOOL GetAddress(enumAccessType Type, long lIndex, in_addr &ipStartAddress, in_addr &ipEndAddress);
    // Get the allowed/blocked/temp.blocked list size
    virtual long GetAddressListSize(enumAccessType Type);
    // Delete address or addresses range from the allowed/blocked/temp.blocked list by index
    virtual BOOL DeleteAddress(enumAccessType Type, long lIndex);
    // Clear all addresses from the allowed/blocked/temp.blocked list
    virtual void ClearAll(enumAccessType Type);


    // Add address to the temporary blocked list
    virtual void AddTempBlockedAddress(in_addr &ipAddress);
    // v4.2 split this into char and wide char
    virtual void AddTempBlockedAddress(LPCSTR lpszAddress);
#if defined _UNICODE
    virtual void AddTempBlockedAddress(LPCWSTR lpszAddress);
#endif
    // Get address  from the temp.blocked list by index
    virtual BOOL GetTempBlockedAddress(long lIndex, in_addr &ipAddress, long &lBlockCounter, time_t &ExpiryTime);
    // Delete address from the temporary blocked list
    virtual BOOL DeleteTempBlockedAddress(in_addr &ipAddress);
    // v4.2 split this into char and wide char
    virtual BOOL DeleteTempBlockedAddress(LPCSTR lpszAddress);
#if defined _UNICODE
    virtual BOOL DeleteTempBlockedAddress(LPCWSTR lpszAddress);
#endif

    // Convert IP address string to the IP adress structure
    // v4.2 split this into char and wide char
    virtual in_addr StringToIP(LPCSTR lpszAddress);
#if defined _UNICODE
    virtual in_addr StringToIP(LPCWSTR lpszAddress);
#endif
    // Convert IP address structure to the IP adress string
    virtual char *IPToString(in_addr &ipAddress);

    // v4.2 split this into char and wide char
    int IPToString(LPSTR string, size_t max_len, in_addr &ipAddress, size_t *size);
#if defined _UNICODE
    int IPToString(LPWSTR string, size_t max_len, in_addr &ipAddress,  size_t *size);
#endif

// Protected methods
protected:

    // Called in the AddTempBlockedAddress function to calculate the expiration time of the block
    virtual time_t  OnCalcTempBlockTime(in_addr &ipAddress, long lBlockCounter, time_t timeBlockOldExpiry);

    // Thread entry function
    static  void    CheckExpired(void * _this);

// Protected data members
protected:

    LIST_IP_ADDRESS_RANGE   m_listAllowed;      // IP addresses which are allowed to connect
    LIST_IP_ADDRESS_RANGE   m_listBlocked;      // IP addresses which are not allowed to connect
    LIST_BLOCKED_IP_ADDR    m_listTempBlocked;  // IP addresses which are temp. not allowed to connect

    CUT_InitCriticalSection m_CriticalSection;  // Critical section used to access the list from different threads
    DWORD                   m_dwCheckExpiredThread; // Chgeck expired blocks thread ID
    BOOL                    m_bShutDown;        // Class shut down flag
};

#endif
