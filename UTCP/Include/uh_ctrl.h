// =================================================================
//  class: CUH_Control
//  File:  uh_ctrl.h
//
//  Purpose:
//
//      Provide output to custom control and file logging 
//      for status messages.
//
// =================================================================
// Ultimate TCP/IP v4.2
// Copyright (C) The Ultimate Toolbox 1995-2007, all rights reserverd
// =================================================================
 
#ifndef _CUH_CONTROL
#define _CUH_CONTROL

#include <stdio.h>
#include <time.h>
#include "utstrlst.h"
#include "ut_strop.h"


// thread safe define, rem out for single threaded environments
// or include for this class to be safe in multi-threaded programs
#ifndef UH_THREADSAFE
#define UH_THREADSAFE
#endif

#define UH_SUCCESS 0
#define UH_ERROR   1
#define UH_SET_THIS_PTR     WM_USER+135


//history linked list
typedef struct UH_HistoryListTag{
    LPTSTR               m_string;
    COLORREF            m_textColor;
    COLORREF            m_backColor;
    int                 m_len;
    UH_HistoryListTag * m_next;
    UH_HistoryListTag * m_prev;
}UH_HistoryList;


//history-logging control class
class CUH_Control{

private:

    //log info
    LPTSTR       m_logName;
    BOOLEAN     m_enableLog;
    BOOLEAN     m_timeStampedLog;
    int         m_logDay,m_logMonth,m_logYear;
    FILE*       m_fileHandle;

    //current settings
    COLORREF    m_textColor;
    COLORREF    m_backColor;
    int         m_alignment;
    int         m_margin;
    HFONT       m_font;
    int         m_fontHeight;
    int         m_fontWidth;
    
    //history list info
    UH_HistoryList *m_historyList;
    int m_historyListMaxLen;

    UH_HistoryList *m_HLCurrentPosPtr;
    int             m_HLCurrentPos;
    UH_HistoryList *m_HLEndPosPtr;
    int             m_HLEndPos;
    
    //scrollbar values
    int m_vScrollRange;
    int m_vScrollPos;
    int m_hScrollRange;
    int m_hScrollPos;
    int m_vScrollPageSize;
    int m_hScrollPageSize;

    //max line width
    int m_maxLineWidth;

    _TCHAR * m_DateFormatString;

    //critical section variables
    #ifdef UH_THREADSAFE
    CRITICAL_SECTION m_criticalSection;
    #endif



public:
    HWND        m_hWnd;

protected:

    //intern

    void OnPaint();
    void OnSize();

    static long CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

    void OnHScroll(int code,int pos);
    void OnVScroll(int code,int pos);
    void SetCurrentPosPtr(int pos);
    void UpdateScrollRange();
    BOOL OnKeyDown(int nKeyCode);

    int  GetTimeDateStamp(LPTSTR string,int len);

    void WriteToLog(LPCTSTR string,int newline);
    int  OpenLog();
    int  CloseLog();

public:

    //contructor - destructor
    
    CUH_Control();
    virtual ~CUH_Control();

    //window creation
    
    int CreateHistoryWindow(HWND parent,DWORD style,RECT &rect);
    int AttachHistoryWindow(HWND parent,UINT ID);
    static int RegisterWindowClass(HINSTANCE hInstance);

    //setup
    
    int     SetTextColor(COLORREF color);
    COLORREF GetTextColor() const;

    int     SetBackColor(COLORREF color);
    COLORREF GetBackColor() const;

    int     SetMargin(int margin);
    int     GetMargin() const;

    int     SetAlignment(int alignment);
    int     GetAlignment() const;

    int     SetLogName(LPCTSTR logName);
    LPCTSTR  GetLogName() const;

    int     EnableLog(BOOLEAN yesNo);
    BOOL    IsLogEnabled() const;

    int     EnableTimeStampedLog(BOOLEAN yesNo);
    BOOL    IsTimeStampedLogEnabled() const;

    int     SetHistoryLength(int len);
    int     GetHistoryLength() const;

    int     SetFont(HFONT font);

    int     SetTimeStampFormat(LPCTSTR formatString);
    LPCTSTR  GetTimeStampFormat() const;

    //operations
	int     ClearHistory();

    int     AddLine(LPCTSTR string, COLORREF TextColor = 0xFFFFFFFF, COLORREF BackColor = 0xFFFFFFFF, BOOLEAN addToLog = TRUE);
    int     AddStampedLine(LPCTSTR string, COLORREF TextColor = 0xFFFFFFFF, COLORREF BackColor = 0xFFFFFFFF, BOOLEAN addToLog = TRUE);
    int     AppendToLine(LPCTSTR string,BOOLEAN addToLog = TRUE);

#if defined _UNICODE
	// some char based protocol classes will send server responses etc. as LPSTRs in a _UNICODE build. These
	// overloads can convert params and call the wide char versions.
    int     AddLine(LPCSTR string, COLORREF TextColor = 0xFFFFFFFF, COLORREF BackColor = 0xFFFFFFFF, BOOLEAN addToLog = TRUE);
    int     AddStampedLine(LPCSTR string, COLORREF TextColor = 0xFFFFFFFF, COLORREF BackColor = 0xFFFFFFFF, BOOLEAN addToLog = TRUE);
    int     AppendToLine(LPCSTR string,BOOLEAN addToLog = TRUE);
#endif
  
};

/**********************************
Static method that takes the same 
parameters as sprintf function and 
returning a pointer to the internal
buffer.

This method simplifies the use of the 
methods like AddLine if you need to
create the string first.

  Ex: AddLine(S("Number of errors: %d", nErrors));
***********************************/    
// v4.3 this generates too many warning 4505s.
/*
static LPCTSTR S(LPCTSTR lpszFormat, ...) 
{
    va_list         marker;
    static _TCHAR   szSBuffer[1024];

    // Initialize variable arguments
    va_start( marker, lpszFormat);  

    // Write formatted output using a pointer to a list of arguments
	CUT_Str::sntprintf(szSBuffer,sizeof(szSBuffer)-1, sizeof(szSBuffer)-1, lpszFormat, marker); 
    // Reset variable arguments
    va_end( marker );    

    // Return the pointer to the static buffer
    return szSBuffer;
}
*/
#endif
