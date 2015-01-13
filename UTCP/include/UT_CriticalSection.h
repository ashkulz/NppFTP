// =================================================================
//  class: CUT_InitCriticalSection, CUT_CriticalSection
//  File:  UT_CriticalSection.h
//  
//  Purpose:
//
//	  Critical section helper classes
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

#ifndef CUT_CRITICAL_SECTION
#define CUT_CRITICAL_SECTION

// ===================================================================
// CUT_InitCriticalSection class 
// ===================================================================
class CUT_InitCriticalSection
{
public:
	CUT_InitCriticalSection()
	{
		InitializeCriticalSection(&m_CriticalSection);
	}
	
	~CUT_InitCriticalSection()
	{
		DeleteCriticalSection(&m_CriticalSection);
	}

	operator LPCRITICAL_SECTION()
	{
		return &m_CriticalSection;
	}

private:

	CRITICAL_SECTION m_CriticalSection;
};


// ===================================================================
// CUT_CriticalSection class 
// ===================================================================
class CUT_CriticalSection
{
public:
	CUT_CriticalSection(LPCRITICAL_SECTION lpCriticalSection) : m_lpCriticalSection(lpCriticalSection) 
	{
		EnterCriticalSection(lpCriticalSection);
	}
	
	~CUT_CriticalSection()
	{
		Leave();
	}

	void Leave()
	{
		if(m_lpCriticalSection != NULL)
		{
			LeaveCriticalSection(m_lpCriticalSection);
			m_lpCriticalSection = NULL;
		}
	}

private:

	LPCRITICAL_SECTION m_lpCriticalSection;

};

#endif
