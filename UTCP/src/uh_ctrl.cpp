//=================================================================
//  class: CUH_Control
//  file:  uh_ctrl.cpp
//
//  Purpose:
//
//   Ultimate History - Output Window and Logging Control
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
NppFTP:
Modification made April 2010:
-remove pragma statements
May 2010:
-Add time_t parameters for stamped line
-Add clipboard functionality
*/

#include "stdafx.h"

#include "uh_ctrl.h"
#include "ut_strop.h"


/**********************************
Constructor
***********************************/
CUH_Control::CUH_Control(){

    //setup the critical section variable
    #ifdef UH_THREADSAFE
    InitializeCriticalSection(&m_criticalSection);
    #endif

    //logging variables
    m_logName       = NULL;
    m_enableLog     = FALSE;
    m_timeStampedLog= FALSE;
    m_logDay        = 0;
    m_logMonth      = 0;
    m_logYear       = 0;
    m_fileHandle    = NULL;

    //window handle
    m_hWnd          = NULL;

    //display properties
    m_textColor     = GetSysColor(COLOR_WINDOWTEXT);
    m_backColor     = GetSysColor(COLOR_WINDOW);
    m_alignment     = TA_LEFT;
    m_margin        = 2;

    //setup the font
    m_font = NULL;
    SetFont(NULL);

    //history list variables
    m_historyList       = NULL;
    m_historyListMaxLen = 0;
    m_HLCurrentPosPtr   = NULL;
    m_HLCurrentPos      = -1;
    m_HLEndPosPtr       = NULL;
    m_HLEndPos          = -1;

    //scrollbar variables
    m_vScrollRange      = 0;
    m_vScrollPos        = 0;
    m_hScrollRange      = 0;
    m_hScrollPos        = 0;
    m_vScrollPageSize   = 0;
    m_hScrollPageSize   = 0;

    //max line width
    m_maxLineWidth = 0;

    //datestamp to string format
    m_DateFormatString = NULL;
    SetTimeStampFormat(_T("%X"));
}
/**********************************
Destructor
***********************************/
CUH_Control::~CUH_Control(){

    ClearHistory();

    if(m_font)
        DeleteObject(m_font);

    if(m_hWnd != NULL)
        DestroyWindow(m_hWnd);

    if(m_DateFormatString != NULL)
        delete[] m_DateFormatString;
    if(m_logName != NULL)
        delete[] m_logName;


    CloseLog();

    //release the critical section
    #ifdef UH_THREADSAFE
    DeleteCriticalSection(&m_criticalSection);
    #endif
}

/**********************************
CreateHistoryWindow
    Creates a window to be associated
    with the history control
Params
    parent - parent window handle
    style - window style
    rect - inital size and position
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::CreateHistoryWindow(HWND parent,DWORD style,RECT &rect){

    if(parent == NULL)
        return UH_ERROR;

    #ifdef WIN32
        HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(parent,GWLP_HINSTANCE);
    #else
        HINSTANCE hInstance = (HINSTANCE)GetWindowWord(parent,GWW_HINSTANCE);
    #endif

    WNDCLASS wndclass;

    //if the window class is not yet registered then register it
    if(GetClassInfo(hInstance,_T("CUH_Control"),&wndclass)==FALSE)
        RegisterWindowClass(hInstance);

    //create the window
    m_hWnd = CreateWindow(_T("CUH_Control"),_T(""),style,
                rect.left,rect.top,
                rect.right - rect.left,
                rect.bottom - rect.top,
                parent,
                NULL,
                hInstance,
                NULL);

    if(m_hWnd == NULL)
        return UH_ERROR;

    SendMessage(m_hWnd,UH_SET_THIS_PTR,0,(LPARAM)this);

    return UH_SUCCESS;
}

/**********************************
AttachHistoryWindow
    Attaches the CUH_Control class to an existing
    window (of "CUH_Control" type). This function
    is usually used to attach this class to a
    child control from a dialog
Params
    parent - parent window handle
    ID - id number of the child control to attach to
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::AttachHistoryWindow(HWND parent,UINT ID){

    if(parent == NULL)
        return UH_ERROR;

    m_hWnd = GetDlgItem(parent,ID);

    if(m_hWnd == NULL)
        return UH_ERROR;

    SendMessage(m_hWnd,UH_SET_THIS_PTR,0,(LPARAM)this);

    //get the number of screen lines
    RECT rect;
    GetClientRect(m_hWnd,&rect);
    m_vScrollPageSize = (rect.bottom / m_fontHeight);
    m_hScrollPageSize = (rect.right / m_fontWidth);
    SetHistoryLength(m_historyListMaxLen);

    return UH_SUCCESS;
}

/**********************************
RegisterWindowClass
    Registers a window class of type 'CUH_Control'
    so that it can be used in dialog boxes
Params
    hInstance - program instance handle
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::RegisterWindowClass(HINSTANCE hInstance){


    WNDCLASS wndclass;

    //register this new window class
    wndclass.style          = CS_HREDRAW | CS_VREDRAW |CS_DBLCLKS;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0 ;
    wndclass.cbWndExtra     = 20;
    wndclass.hInstance      = hInstance ;
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = NULL;
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = _T("CUH_Control");

    RegisterClass(&wndclass);

    if(GetClassInfo(hInstance,_T("CUH_Control"),&wndclass))
        return UH_SUCCESS;
    return UH_ERROR;
}

/**********************************
WndProc
    Internal Function
    Window procedure that distributes
    all window messages for windows of
    type 'CUH_Control' it then dispatches
    the messages to the class that is attached
    to the window
Params
    n/a
Return
    n/a
***********************************/
LRESULT CALLBACK CUH_Control::WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){

    switch(message){
        case WM_NCCREATE:{
            SetWindowLong(hwnd,0,(LPARAM)NULL);
            return 1;
        }
        case UH_SET_THIS_PTR:{
            //store the pointer to the calling class
            // CUH_Control *_this = (CUH_Control*)lParam;
            SetWindowLong(hwnd,0,(LONG)lParam);
            return 1;
        }
    }

    //get the pointer to the calling class
    CUH_Control *_this = (CUH_Control*)GetWindowLongPtr(hwnd,0);

    //call the functions that match in incoming message
    if(_this != NULL){
        switch(message){
            case WM_KEYDOWN:
                if(_this->OnKeyDown((int)wParam) == TRUE)
                    return 0;
                break;
            case WM_PAINT:
                _this->OnPaint();
                return 0;
            case WM_SIZE:
                _this->OnSize();
                return 0;
            case WM_NCDESTROY:
                _this->m_hWnd = NULL;
                return 0;
            case WM_HSCROLL:
                #ifdef WIN32
                _this->OnHScroll(LOWORD(wParam),HIWORD(wParam));
                #else
                _this->OnHScroll(wParam,LOWORD(lParam));
                #endif
                return 0;
            case WM_VSCROLL:
                #ifdef WIN32
                _this->OnVScroll(LOWORD(wParam),HIWORD(wParam));
                #else
                _this->OnVScroll(wParam,LOWORD(lParam));
                #endif
                return 0;
            case WM_MOUSEACTIVATE:
                SetFocus(_this->m_hWnd);
                break;
            case WM_ERASEBKGND:
                return 1;
        }
    }

    return (long)DefWindowProc(hwnd,message,wParam,lParam);
}

/**********************************
OnPaint
    Internal function
    This routine paints the window.
    It displays the current history
    items
Params
    none
Return
    none
***********************************/
void CUH_Control::OnPaint(){

    if(m_hWnd ==  NULL)
        return;

    //start from the bottom of the rect and work up
    RECT rect,rect2;
    GetClientRect(m_hWnd,&rect);
    CopyRect(&rect2,&rect);
    UINT    nFormat;

    //get hte device context
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(m_hWnd, &ps);

    HFONT oldFont = NULL;
    if(m_font != NULL)
        oldFont = (HFONT)SelectObject(dc,m_font);

    if(m_historyList != NULL){

        // Enter into a critical section
        #ifdef UH_THREADSAFE
        EnterCriticalSection(&m_criticalSection);
        #endif

        //if the scrollbar range is zero then draw from the top
        if(m_vScrollRange ==0){
            rect2.top =0;
            rect2.bottom =0;
            UH_HistoryList *next = m_HLEndPosPtr;
            int margin = m_margin-(m_hScrollPos * m_fontWidth);
            for(int t = m_HLEndPos; t >= 0  && next != NULL; t--){
                rect2.bottom += m_fontHeight;

                ::SetTextColor(dc, (next->m_textColor == 0xFFFFFFFF) ? m_textColor : next->m_textColor);
                SetBkColor(dc, (next->m_backColor == 0xFFFFFFFF) ? m_backColor : next->m_backColor);

                ExtTextOut(dc,0,0,ETO_CLIPPED|ETO_OPAQUE, &rect2, NULL, 0,NULL);
                RECT    rect3;
                CopyRect(&rect3, &rect2);

                if(m_alignment == TA_CENTER) {
                    rect3.left += margin/2;
                    nFormat = DT_CENTER;
                    }
                else if(m_alignment == TA_RIGHT) {
                    rect3.right -= margin;
                    nFormat = DT_RIGHT;
                    }
                else {
                    rect3.left += margin;
                    nFormat = DT_LEFT;
                    }

                DrawText(dc, next->m_string, next->m_len, &rect3, DT_EXPANDTABS | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP | nFormat);


                next = next->m_prev;
                rect2.top = rect2.bottom;
            }
            rect2.bottom = rect.bottom;
            SetBkColor(dc, m_backColor);
            ExtTextOut(dc,rect2.left,rect2.top,ETO_OPAQUE,&rect2,_T(""),0,NULL);
        }
        else{
            rect2.top = rect.bottom;
            rect2.bottom = rect.bottom;

            int margin = m_margin-(m_hScrollPos * m_fontWidth);
            int pos = m_vScrollRange - m_vScrollPos;
            if(m_HLCurrentPos != pos)
                SetCurrentPosPtr(pos);
            UH_HistoryList *next = m_HLCurrentPosPtr;
            for(int t = pos ;t <= m_HLEndPos && next != NULL; t++) {
                rect2.top -= m_fontHeight;

                ::SetTextColor(dc, (next->m_textColor == 0xFFFFFFFF) ? m_textColor : next->m_textColor);
                SetBkColor(dc, (next->m_backColor == 0xFFFFFFFF) ? m_backColor : next->m_backColor);

                ExtTextOut(dc,0,0,ETO_CLIPPED|ETO_OPAQUE, &rect2, NULL, 0,NULL);
                RECT    rect3;
                CopyRect(&rect3, &rect2);

                if(m_alignment == TA_CENTER) {
                    rect3.left += margin/2;
                    rect3.right -= margin/2;
                    nFormat = DT_CENTER;
                    }
                else if(m_alignment == TA_RIGHT) {
                    rect3.right -= margin;
                    nFormat = DT_RIGHT;
                    }
                else {
                    rect3.left += margin;
                    nFormat = DT_LEFT;
                    }

                DrawText(dc, next->m_string, next->m_len, &rect3, DT_EXPANDTABS | DT_SINGLELINE | DT_VCENTER | DT_NOCLIP | nFormat);


                next = next->m_next;
                rect2.bottom = rect2.top;
                if(rect2.bottom <0)
                    break;
            }
            rect2.top = rect.top;
            SetBkColor(dc,m_backColor);
            ExtTextOut(dc,rect2.left,rect2.top,ETO_OPAQUE,&rect2,_T(""),0,NULL);
        }

    //exit the critical section
    #ifdef UH_THREADSAFE
    LeaveCriticalSection(&m_criticalSection);
    #endif

    }
    else{
        SetBkColor(dc,m_backColor);
        ExtTextOut(dc,0,0,ETO_OPAQUE,&rect,_T(""),0,NULL);
    }
    //clean -up
    if(m_font != NULL)
        SelectObject(dc,oldFont);

    EndPaint(m_hWnd,&ps);
}

/**********************************
OnSize
    Internal function
    This routine is called when the
    window sizes. It will also check to
    see if the scrollbars should be
    displayed
Params
    none
Return
    none
***********************************/
void CUH_Control::OnSize(){

    if(m_hWnd ==  NULL)
        return;

    //get the number of screen lines
    RECT rect;
    GetClientRect(m_hWnd,&rect);
    m_vScrollPageSize = (rect.bottom / m_fontHeight);
    m_hScrollPageSize = (rect.right / m_fontWidth);

    SetHistoryLength(m_historyListMaxLen);
}

/**********************************
OnKeyDown
    Process scrolling through keyboard
Params
    nKeyCode    - key scan code
Return
    TRUE        - if we process this key
***********************************/
BOOL CUH_Control::OnKeyDown(int nKeyCode) {

    switch(nKeyCode) {
        case(VK_DOWN):
            OnVScroll(SB_LINEDOWN, 0);
            break;
        case(VK_UP):
            OnVScroll(SB_LINEUP, 0);
            break;
        case(VK_NEXT):
            OnVScroll(SB_PAGEDOWN, 0);
            break;
        case(VK_PRIOR):
            OnVScroll(SB_PAGEUP, 0);
            break;
        case(VK_LEFT):
            OnHScroll(SB_LINEUP, 0);
            break;
        case(VK_RIGHT):
            OnHScroll(SB_LINEDOWN, 0);
            break;
        default:
            return FALSE;
        }

    return TRUE;
}

/**********************************
UpdateScrollRange
    Internal function
    This function calcs the scrollbar
    ranges
Params
    none
Return
    none
***********************************/
void CUH_Control::UpdateScrollRange(){

    if(m_hWnd == NULL)
        return;

    //update the vertical scrollbar
    int offset = m_vScrollRange - m_vScrollPos;

    m_vScrollRange = m_HLEndPos - m_vScrollPageSize +1;
    if(m_vScrollRange <0)
        m_vScrollRange = 0;

    m_vScrollPos = m_vScrollRange - offset;
    if(m_vScrollPos <0)
        m_vScrollPos = 0;

    SetScrollRange(m_hWnd,SB_VERT,0,m_vScrollRange,FALSE);
    SetScrollPos(m_hWnd,SB_VERT,m_vScrollPos,TRUE);

    //update the horizontal scrollbar
    m_hScrollRange = m_maxLineWidth - m_hScrollPageSize+1;
    if(m_hScrollPos > m_hScrollRange)
        m_hScrollPos = m_hScrollRange;
    if(m_hScrollPos <0)
        m_hScrollPos = 0;
    SetScrollRange(m_hWnd,SB_HORZ,0,m_hScrollRange,FALSE);
    SetScrollPos(m_hWnd,SB_HORZ,m_hScrollPos,TRUE);

}

/**********************************
OnVScroll
    Internal function
    This function processes vertical
    scrollbar messages and scrolls
    the history window contents
Params
    n/a
Return
    none
***********************************/
void CUH_Control::OnVScroll(int code,int pos){

    int oldPos = m_vScrollPos;

    switch(code){
        case SB_LINEDOWN:
            m_vScrollPos++;
            break;
        case SB_LINEUP:
            m_vScrollPos--;
            break;
        case SB_PAGEDOWN:
            m_vScrollPos += m_vScrollPageSize;
            break;
        case SB_PAGEUP:
            m_vScrollPos -= m_vScrollPageSize;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            m_vScrollPos = pos;
            break;
    }

    //check to see if the position has changed
    if(oldPos != m_vScrollPos){
        //range checking
        if(m_vScrollPos > m_vScrollRange)
            m_vScrollPos = m_vScrollRange;
        if(m_vScrollPos <0)
            m_vScrollPos = 0;
        SetScrollPos(m_hWnd,SB_VERT,m_vScrollPos,TRUE);

        //redraw the screen
        SetCurrentPosPtr(m_vScrollRange - m_vScrollPos);
        InvalidateRect(m_hWnd,NULL,TRUE);
    }
}

/**********************************
OnVScroll
    Internal function
    This function processes horizontal
    scrollbar messages and scrolls
    the history window contents
Params
    n/a
Return
    none
***********************************/
void CUH_Control::OnHScroll(int code,int pos){

    int oldPos = m_hScrollPos;

    switch(code){
        case SB_LINEDOWN:
            m_hScrollPos++;
            break;
        case SB_LINEUP:
            m_hScrollPos--;
            break;
        case SB_PAGEDOWN :
            m_hScrollPos += m_hScrollPageSize;
            break;
        case SB_PAGEUP:
            m_hScrollPos -= m_hScrollPageSize;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            m_hScrollPos = pos;
            break;
    }

    //check to see if the position has changed
    if(oldPos != m_hScrollPos){
        //range checking
        if(m_hScrollPos > m_hScrollRange)
            m_hScrollPos = m_hScrollRange;
        if(m_hScrollPos <0)
            m_hScrollPos = 0;
        SetScrollPos(m_hWnd,SB_HORZ,m_hScrollPos,TRUE);
        //redraw the screen
        InvalidateRect(m_hWnd,NULL,TRUE);
    }
}

/**********************************
SetFont
    Sets the font to use when new
    lines are added to the history list
Params
    hont - handle to a font
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::SetFont(HFONT font){

    SIZE size;
    HFONT   oldFont = NULL;

    if(m_font != NULL && font != NULL)
        DeleteObject(m_font);

    m_font = font;

    //get the font height
    if(m_hWnd != NULL){
        HDC dc = GetDC(m_hWnd);
        if(m_font != NULL)
            oldFont = (HFONT)SelectObject(dc,m_font);
        GetTextExtentPoint(dc,_T("X"),1,&size);
        if(m_font != NULL)
            SelectObject(dc,oldFont);
        ReleaseDC(m_hWnd,dc);
    }
    else{
        HDC dc = GetDC(NULL);
        if(m_font != NULL)
            oldFont = (HFONT)SelectObject(dc,m_font);
        GetTextExtentPoint(dc,_T("X"),1,&size);
        if(m_font != NULL)
            SelectObject(dc,oldFont);
        ReleaseDC(NULL,dc);
    }

    //store the font height
    m_fontHeight = size.cy;
    m_fontWidth = size.cx;

    //get the number of screen lines
    if(m_hWnd != NULL){
        RECT rect;
        GetClientRect(m_hWnd,&rect);
        m_vScrollPageSize = (rect.bottom / m_fontHeight);
        m_hScrollPageSize = (rect.right / m_fontWidth);
        SetHistoryLength(m_historyListMaxLen);
    }

    return UH_SUCCESS;
}

/**********************************
SetCurrentPosPtr
    Internal function
    Sets the current position within
    the history linked list
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
void CUH_Control::SetCurrentPosPtr(int pos){

    if(m_HLCurrentPosPtr == NULL)
        return;

    while(m_HLCurrentPos <pos){
        if(m_HLCurrentPosPtr->m_next == NULL)
            break;
        m_HLCurrentPosPtr = m_HLCurrentPosPtr->m_next;
        m_HLCurrentPos++;
    }
    while(m_HLCurrentPos > pos){
        if(m_HLCurrentPosPtr->m_prev == NULL)
            break;
        m_HLCurrentPosPtr = m_HLCurrentPosPtr->m_prev;
        m_HLCurrentPos--;
    }
}

/**********************************
GetHistoryLength
    Gets the minimum number of items
    that the history list can hold
Params
    none
Return
    mimimum number of items
***********************************/
int CUH_Control::GetHistoryLength() const
{
    return m_historyListMaxLen;
}
/**********************************
SetHistoryLength
    Sets the minimum number of items
    that the history list can hold
Params
    len - mimimum number of items
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::SetHistoryLength(int len){

    //make sure the min history length is the
    //number of items visible on the screen
    if(m_hWnd != NULL){
        RECT rect;
        GetClientRect(m_hWnd,&rect);
        //get the number of screen lines
        int screenLen = (rect.bottom / m_fontHeight);
        if(len < screenLen)
            len = screenLen;
    }

    //if the list length is zero then just clear it
    if(len == 0){
        ClearHistory();
        m_historyListMaxLen = 0;
    }
    else{

        m_historyListMaxLen = len;

        //if the current position is now out of range, re-adjust
        while(m_HLCurrentPos >= m_historyListMaxLen){
            if(m_HLCurrentPosPtr->m_prev != NULL){
                m_HLCurrentPosPtr = m_HLCurrentPosPtr->m_prev;
                m_HLCurrentPos --;
            }
        }

        //delete any items out of range
        UH_HistoryList *prev = NULL;
        while(m_HLEndPos >= m_historyListMaxLen){
            if(m_HLEndPosPtr != NULL){
                if(m_HLEndPosPtr->m_string != NULL)
                    delete[] m_HLEndPosPtr->m_string;
                prev = m_HLEndPosPtr->m_prev;
                prev = m_HLEndPosPtr->m_next = NULL;
                delete m_HLEndPosPtr;
            }

            m_HLEndPosPtr = prev;
        }
        UpdateScrollRange();
    }

    return UH_SUCCESS;
}

/**********************************
SetAligment
    Sets text alignment
Params
    alignment - new alignment
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::SetAlignment(int alignment){
    m_alignment = alignment;
    return UH_SUCCESS;
}
/**********************************
GetAligment
    Gets text alignment
Params
    none
Return
    alignment
***********************************/
int CUH_Control::GetAlignment() const
{
    return m_alignment;
}


/**********************************
SetTextColor
    Sets the text color to use when
    new lines are added to the history
    list
Params
    color - new text color to use
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::SetTextColor(COLORREF color){

    m_textColor = color;

    return UH_SUCCESS;
}

/**********************************
SetBackColor
    Sets the backgorund color to use when
    new lines are added to the history
    list
Params
    color - new back color to use
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)

***********************************/
int CUH_Control::SetBackColor(COLORREF color){

    m_backColor = color;
    return UH_SUCCESS;
}

/**********************************
GetTextColor
    Gets the text color to use when
    new lines are added to the history
    list
Params
    color - new text color to use
Return
    text color
***********************************/
COLORREF CUH_Control::GetTextColor() const
{
    return m_textColor;
}

/**********************************
GetBackColor
    Gets the backgorund color to use when
    new lines are added to the history
    list
Params
    color - new back color to use
Return
    back color
***********************************/
COLORREF CUH_Control::GetBackColor() const
{
    return m_backColor;
}
/**********************************
SetMargin
    Sets the new margin to use when
    new lines are added to the history
    list
Params
    margin - margin in pixels
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::SetMargin(int margin){

    m_margin = margin;
    return UH_SUCCESS;
}

/**********************************
GetMargin
    Gets the new margin to use when
    new lines are added to the history
    list
Params
    none
Return
    margin - margin in pixels
***********************************/
int CUH_Control::GetMargin() const
{
    return m_margin;
}
/**********************************
SetLogName
    Sets the filename of the log file.
    If date stamping is enabled then
    then date stamp will be instered just
    be for the extenion in the filename
Params
    logName - file name (plus optional
        path)
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::SetLogName(LPCTSTR logName){

    int rt = UH_SUCCESS;

    int len = (int)_tcslen(logName);

    if(len <1 || len > 255){
        rt  =  UH_ERROR;
    }
    else{
        if(m_logName != NULL)
            delete[] m_logName;

        m_logName = new _TCHAR[len+1];
        _tcscpy(m_logName,logName);
    }

    return rt;
}
/**********************************
SetLogName
    Sets the filename of the log file.
    If date stamping is enabled then
    then date stamp will be instered just
    be for the extenion in the filename
Params
    none
Return
    log file name
***********************************/
LPCTSTR CUH_Control::GetLogName() const
{
    return m_logName;
}

/**********************************
EnableLog
    Turn file logging on and off
Params
    yesNo - TRUE to enable FALSE to disable
        Disable is default
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::EnableLog(BOOLEAN yesNo){

    if(yesNo){
        m_enableLog = TRUE;
        OpenLog();
    }
    else{
        m_enableLog = FALSE;
        CloseLog();
    }

    return UH_SUCCESS;
}

/**********************************
IsLogEnabled
    Turn file logging on and off
Params
    none
Return
    TRUE/FALSE
***********************************/
BOOL CUH_Control::IsLogEnabled() const
{
     return m_enableLog;
}


/**********************************
EnableTimeStampedLog
    Turns date stamped log file naming on and
    off. If on then a new file will be created
    whenever the system date changes. This way
    a new file will be created every 24 hours.
    The date stamp is interted into the log file
    name just before the filename extention.
Params
    yesNo - TRUE to enable , FALSE to disable
        Disable is default
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::EnableTimeStampedLog(BOOLEAN yesNo){

    if(yesNo)
        m_timeStampedLog = TRUE;
    else
        m_timeStampedLog = FALSE;

    return UH_SUCCESS;
}
/**********************************
EnableTimeStampedLog
    Return time stamp enabled flag
Params
    none
Return
    Log time stamp enabled flag
***********************************/
BOOL CUH_Control::IsTimeStampedLogEnabled() const
{
    return m_timeStampedLog;
}

/**********************************
AddLine
    Adds a new line to the control.
    If logging is on and the addToLog
    param is TRUE then this line is also
    added to the log file.
Params
    string - string to add
    addToLog - if TRUE then add to the file
        log, if it is enabled
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::AddLine(LPCTSTR string, COLORREF TextColor, COLORREF BackColor, BOOLEAN addToLog){

    //enter into a critical section
    #ifdef UH_THREADSAFE
    EnterCriticalSection(&m_criticalSection);
    #endif

    //update the log file
    if(addToLog && m_enableLog)
        WriteToLog(string,TRUE);

    //check to see if a window exists
    int updateScrollFlag = FALSE;
    if(m_hWnd != NULL){
        UH_HistoryList *old = m_historyList;

        m_historyList = new UH_HistoryList;
        m_historyList->m_next =old;
        m_historyList->m_prev = NULL;
        m_historyList->m_textColor = TextColor;
        m_historyList->m_backColor = BackColor;

        if(old != NULL)
            old->m_prev = m_historyList;

        int len = (int)_tcslen(string);
        if(len == 0) {
            m_historyList->m_string = new _TCHAR[1];
            m_historyList->m_string = 0;
            m_historyList->m_len =0;
        }
        else{
            m_historyList->m_string = new _TCHAR[len+1];
            _tcscpy(m_historyList->m_string,string);
// TD TODO          CUT_StrMethods::RemoveCRLF(m_historyList->m_string);
            m_historyList->m_len = (int)_tcslen(m_historyList->m_string);
        }

        //store the max line width
        if(len > m_maxLineWidth){
            m_maxLineWidth = len;
            updateScrollFlag = TRUE;
        }

        //adjust the current pos
        if(m_HLCurrentPosPtr != NULL)
            m_HLCurrentPosPtr = m_HLCurrentPosPtr->m_prev;
        else{
            m_HLCurrentPosPtr = m_historyList;
            m_HLCurrentPos = 0;
        }

        //cut items off the end - if reached
        if(m_HLEndPosPtr != NULL){
            if(m_HLEndPos == m_historyListMaxLen -1){
                UH_HistoryList *old = m_HLEndPosPtr;
                m_HLEndPosPtr = m_HLEndPosPtr->m_prev;
                m_HLEndPosPtr->m_next = NULL;

                //if this is the longest line
                //then find the new longest line
                if(old->m_len == m_maxLineWidth){
                    UH_HistoryList *check = old->m_prev;
                    m_maxLineWidth = 0;
                    while(check != NULL){
                        if(m_maxLineWidth < check->m_len)
                            m_maxLineWidth = check->m_len;
                        check = check->m_prev;
                    }
                    updateScrollFlag = TRUE;
                }
                if(old->m_string!= NULL)
                    delete[]old->m_string;
                delete old;
            }
            else{
                m_HLEndPos ++;
                updateScrollFlag = TRUE;
            }
        }
        else{
            m_HLEndPosPtr = m_historyList;
            m_HLEndPos = 0;
        }

    }

    //exit the critical section
    #ifdef UH_THREADSAFE
    LeaveCriticalSection(&m_criticalSection);
    #endif

    if(m_hWnd != NULL) {
        //update the scrollbar
        if(updateScrollFlag)
            UpdateScrollRange();

        //redraw the screen
        InvalidateRect(m_hWnd,NULL,TRUE);
        }

    return UH_SUCCESS;
}

/**********************************
AddStampedLine
    Adds a new line to the control with
    a time stamp added to the beginning
    of the line.
    If logging is on and the addToLog
    param is TRUE then this line is also
    added to the log file.
Params
    string - string to add
    addToLog - if TRUE then add to the file
        log, if it is enabled
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::AddStampedLine(LPCTSTR string, COLORREF TextColor, COLORREF BackColor, BOOLEAN addToLog){
    return AddStampedLineT(string, TextColor, BackColor, addToLog, 0);
}

int CUH_Control::AddStampedLineT(LPCTSTR string, COLORREF TextColor, COLORREF BackColor, BOOLEAN addToLog, time_t time){

    //enter into a critical section
    #ifdef UH_THREADSAFE
    EnterCriticalSection(&m_criticalSection);
    #endif

    int len = (int)_tcslen(string);
    _TCHAR *buf = new _TCHAR[len+25];

    GetTimeDateStamp(buf,len+25, time);
    _tcscat(buf,_T(" "));
    _tcscat(buf,string);
    int rt = AddLine(buf,TextColor, BackColor, addToLog);
    delete[] buf;

    //exit the critical section
    #ifdef UH_THREADSAFE
    LeaveCriticalSection(&m_criticalSection);
    #endif

    return rt;
}

/**********************************
AppendToLine
    Appends more information to the last
    line added.
    If logging is on and the addToLog
    param is TRUE then this line is also
    added to the log file.
Params
    string - string to add
    addToLog - if TRUE then add to the file
        log, if it is enabled
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::AppendToLine(LPCTSTR string,BOOLEAN addToLog){


    //enter into a critical section
    #ifdef UH_THREADSAFE
    EnterCriticalSection(&m_criticalSection);
    #endif

    int rt = UH_SUCCESS;

    if(m_historyList == NULL){
        rt = UH_ERROR;
    }
    else if(m_historyList->m_string == NULL){
        rt = UH_ERROR;
    }
    else{
        //update the log file
        if(addToLog && m_enableLog)
            WriteToLog(string,FALSE);

        //check to see if a window exists
        if(m_hWnd != NULL){

            int len = (int)_tcslen(string) + (int)_tcslen(m_historyList->m_string);
            _TCHAR *buf = new _TCHAR[len+1];
            _tcscpy(buf,m_historyList->m_string);
            _tcscat(buf,string);

            delete[] m_historyList->m_string;
            m_historyList->m_string = buf;
            m_historyList->m_len = len;

            //store the max line width
            if(len > m_maxLineWidth){
                m_maxLineWidth = len;
                UpdateScrollRange();
            }
            //redraw the screen
            InvalidateRect(m_hWnd,NULL,TRUE);
        }
    }

    //exit the critical section
    #ifdef UH_THREADSAFE
    LeaveCriticalSection(&m_criticalSection);
    #endif

    return rt;
}

/**********************************
ClearHistory
    Clears all items from the history
    list
Params
    none
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************/
int CUH_Control::ClearHistory(){

    //enter into a critical section
    #ifdef UH_THREADSAFE
    EnterCriticalSection(&m_criticalSection);
    #endif

    UH_HistoryList *current = m_historyList;
    UH_HistoryList *next;
    //delete the history list
    while(current != NULL){
        next= current->m_next;

        if(current->m_string != NULL)
            delete[] current->m_string;
        delete current;

        current = next;
    }
    m_historyList = NULL;
    m_HLCurrentPosPtr = NULL;
    m_HLCurrentPos = -1;
    m_HLEndPosPtr = NULL;
    m_HLEndPos = -1;

    m_maxLineWidth = 0;

    if(m_hWnd != NULL){
        UpdateScrollRange();
        InvalidateRect(m_hWnd,NULL,TRUE);
        SendMessage(m_hWnd,WM_PAINT,0,0);
    }

    //exit the critical section
    #ifdef UH_THREADSAFE
    LeaveCriticalSection(&m_criticalSection);
    #endif

    return UH_SUCCESS;
}

/**********************************
CopyToClipboard
    Copies all items from the
    history to the clipboard in
    plaintext.
Params
    none
Return
    0 - (UH_SUCCESS) Places contents on clipboard
    1 - (UH_ERROR) No memory or clipboard failure
***********************************/
int CUH_Control::CopyToClipboard(){
    long size = 0;

    UH_HistoryList *current = m_historyList;
    UH_HistoryList *last = m_historyList;
    while(current != NULL){
        if(current->m_string != NULL)
            size += lstrlen(current->m_string) + 2; //+newline
        last = current;
        current = current->m_next;
    }
    size += 1;

    BOOL result = OpenClipboard(m_hWnd);
    if (result == FALSE) {
        return UH_ERROR;
    }
    result = EmptyClipboard();
    if (result == FALSE) {
        return UH_ERROR;
    }

    long totalBytesNeeded = size*sizeof(TCHAR);
    HGLOBAL hTXTBuffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, totalBytesNeeded);
    TCHAR * clipString = (TCHAR*)GlobalLock(hTXTBuffer);
    clipString[0] = 0;

    current = last;
    while(current != NULL){
        if(current->m_string != NULL) {
            lstrcat(clipString, current->m_string);
            lstrcat(clipString, TEXT("\r\n"));
        }
        current = current->m_prev;
    }

    GlobalUnlock(hTXTBuffer);

#if defined _UNICODE
    HANDLE clipHandle = SetClipboardData(CF_UNICODETEXT, hTXTBuffer);
#else
    HANDLE clipHandle = SetClipboardData(CF_TEXT, hTXTBuffer);
#endif
    if (!clipHandle) {
        GlobalFree(hTXTBuffer);
        return UH_ERROR;
    }

    result = CloseClipboard();
    if (result == FALSE) {
        //what to do?
        return UH_ERROR;
    }

    return UH_SUCCESS;
}

/**********************************************
GetTimeDateStamp
    Internal function
    Returns a string with a time stamp that includes
    the date.
Params
    string - buffer for the stamp to be put in
    len - length of the buffer
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************************/
int CUH_Control::GetTimeDateStamp(LPTSTR string,int len, time_t ttime){

    if(len < 18)
        return UH_ERROR;

    //get the time/date
    time_t  t           = (ttime==0)?time(NULL):ttime;
    struct tm * systime = localtime(&t);

    //format the date into a string
    if(_tcsftime(string,len,m_DateFormatString,systime) == 0){
        string[0] =0;
        return UH_ERROR;
    }
    return UH_SUCCESS;
}

/**********************************************
SetTimeStampFormat
    Sets the format for stamped lines. See the
    information on the strftime function for
    for full information on format strings
Params
    formatString - format mask for the time stamp
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************************/
int CUH_Control::SetTimeStampFormat(LPCTSTR formatString){

    int len = (int)_tcslen(formatString);

    if(len <1){
        return UH_ERROR;
    }

    if(m_DateFormatString != NULL)
        delete[] m_DateFormatString;

    m_DateFormatString = new _TCHAR[len+1];
    _tcscpy(m_DateFormatString,formatString);

    return UH_SUCCESS;
}
/**********************************************
GetTimeStampFormat
    Gets the format for stamped lines. See the
    information on the strftime function for
    for full information on format strings
Params
    none
Return
    format mask for the time stamp
***********************************************/
LPCTSTR CUH_Control::GetTimeStampFormat() const
{
    return m_DateFormatString;
}

/**********************************************
OpenLog
    Internal function
    Opens up the history log file for writing.
Params
    none
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************************/
int CUH_Control::OpenLog(){

    CloseLog();

    if(m_timeStampedLog){
        //get the date string
        time_t  t           = time(NULL);
        struct tm * systime = localtime(&t);
        _TCHAR strDate[30];
        m_logDay    = systime->tm_mday;
        m_logMonth  = systime->tm_mon;
        m_logYear   = systime->tm_year;
        _sntprintf(strDate,sizeof(strDate)/sizeof(_TCHAR)-1,_T("%2.2d%2.2d%4.4d"),systime->tm_mday,systime->tm_mon+1,
            systime->tm_year+1900);

        //find the .
        int pos;
        int len = (int)_tcslen(m_logName);
        for(pos = 0; pos < len; pos++){
            if(m_logName[pos] == '.')
                break;
        }

        //add the date string just before the . in the filename
        _TCHAR * buf = new _TCHAR[len+10];
        _tcscpy(buf,m_logName);
        buf[pos] =0;
        _tcscat(buf,strDate);
        _tcscat(buf,&m_logName[pos]);

        m_fileHandle = _tfopen(buf,_T("a+"));

        delete[] buf;
    }
    else{
        m_fileHandle = _tfopen(m_logName,_T("a+"));
    }

    if(m_fileHandle == NULL)
        return UH_ERROR;

    return UH_SUCCESS;
}

/**********************************************
CloseLog
    Internal function
Params
    none
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************************/
int CUH_Control::CloseLog(){
    if(m_fileHandle != NULL)
        fclose(m_fileHandle);

    return UH_SUCCESS;
}

/**********************************************
WriteToLog
    Internal function
Params
    string - the string to write to the log
    newline - TRUE if a new line is to be added
        after the string
Return
    0 - (UH_SUCCESS)
    1 - (UH_ERROR)
***********************************************/
void CUH_Control::WriteToLog(LPCTSTR string,int newline){

    //check to see if the date has changed
    time_t  t  = time(NULL);
    struct tm * lt = localtime(&t);
    if(m_logDay != lt->tm_mday || m_logMonth != lt->tm_mon ||
      m_logYear != lt->tm_year){
        OpenLog();
    }

    if(m_fileHandle == NULL)
        return;

    if(newline)
        fwrite("\n",sizeof(char),1,m_fileHandle);
#if defined _UNICODE
    size_t size = _tcslen(string);
    char * buffA = (char*) alloca(size+1);
    *buffA = '\0';
    CUT_Str::cvtcpy(buffA, size, string);   // convert to ANSI
    fwrite(buffA,sizeof(char),_tcslen(string),m_fileHandle);
#else
    fwrite(string,sizeof(char),_tcslen(string),m_fileHandle);
#endif //_UNICODE
}

#if defined _UNICODE
int CUH_Control::AddLine(LPCSTR string, COLORREF TextColor, COLORREF BackColor, BOOLEAN addToLog)
{
    return AddLine(WC(string), TextColor, BackColor, addToLog);
}
int CUH_Control::AddStampedLine(LPCSTR string, COLORREF TextColor, COLORREF BackColor, BOOLEAN addToLog)
{
    return AddStampedLine(WC(string), TextColor, BackColor, addToLog);
}
int CUH_Control::AppendToLine(LPCSTR string, BOOLEAN addToLog)
{
    return AppendToLine(WC(string), addToLog);
}
#endif
