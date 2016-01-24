#if !defined(DATE_TIME_SELECTOR_H)
#define DATE_TIME_SELECTOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////
// ** NOTICE **
/////////////////////////////////////////////////////////////////////////
// The following resources are required by this module:
//

#define IDD_DATE_TIME_SELECTOR 				8101
#define IDC_DATETIME_DATE_SELECTOR			8102
#define IDC_DATETIME_TIME_SELECTOR			8103
#define IDC_DATETIME_CALENDAR					8104
#define IDC_DATETIME_TODAY						8105
#define IDC_DATETIME_NOW						8106
															
// Just drop this baby into your resource file:
//
//		IDD_DATE_TIME_SELECTOR DIALOGEX 0, 0, 156, 164
//		STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION
//		EXSTYLE WS_EX_TOOLWINDOW
//		CAPTION "Select date and time..."
//		FONT 8, "MS Sans Serif"
//		BEGIN
//		    CONTROL         "MonthCalendar1",IDC_DATETIME_CALENDAR,"SysMonthCal32",
//		                    MCS_NOTODAY | WS_TABSTOP | 0x8,7,7,142,87
//		    LTEXT           "&Date:",IDC_STATIC,18,102,18,8
//		    CONTROL         "DateTimePicker1",IDC_DATETIME_DATE_SELECTOR,
//		                    "SysDateTimePick32",DTS_RIGHTALIGN | DTS_UPDOWN | 
//		                    WS_TABSTOP,40,100,67,15
//		    PUSHBUTTON      "Toda&y",IDC_DATETIME_TODAY,112,101,27,14
//		    LTEXT           "&Time:",IDC_STATIC,17,122,18,8
//		    CONTROL         "DateTimePicker1",IDC_DATETIME_TIME_SELECTOR,
//		                    "SysDateTimePick32",DTS_RIGHTALIGN | DTS_UPDOWN | 
//		                    WS_TABSTOP | 0x8,40,120,67,15
//		    PUSHBUTTON      "&Now",IDC_DATETIME_NOW,112,120,27,14
//		    DEFPUSHBUTTON   "&OK",IDOK,28,142,48,14
//		    PUSHBUTTON      "&Cancel",IDCANCEL,80,142,48,14
//		END
// 
// 
// Ideally, resources would be specified on a class-by-class basis.  Unfortunately,
// Visual Studio does not handle that well.  Different projects have different resource
// files that contain all of the project's resources in one place.  Make sure
// you provide the resources matching the above defines in your resource file.
// You must also include this file in the resource file's "Read-only symbol
// directives" ( see "Resource Includes" on the View menu ).
//
// We want to exclude the remainder of the include file when dealing with App Studio.
#ifndef RC_INVOKED
/////////////////////////////////////////////////////////////////////////


#include "..\FontDlg.h"													// Base class, allows custom font.


/////////////////////////////////////////////////////////////////////////////
// DateTimeSelector dialog

class DateTimeSelector : public FontDlg
{
	typedef FontDlg inherited;

// Construction
public:
	DateTimeSelector(CWnd* pParent = NULL);   // standard constructor

protected:
// Dialog Data
	//{{AFX_DATA(DateTimeSelector)
	enum { IDD = IDD_DATE_TIME_SELECTOR };
	COleDateTime	m_SelectedDate;
	CTime				m_SelectedTime;
	COleDateTime	m_CalendarDate;
	//}}AFX_DATA

public:
	
	// This should be set before DoModal() and gotten after for the
	// user result...
	COleDateTime	m_SelectedDateTime;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DateTimeSelector)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DateTimeSelector)
	afx_msg void OnSelchangeDatetimeCalendar(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeDatetimeDateSelector(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnOK();
	afx_msg void OnDatetimeToday();
	afx_msg void OnDatetimeNow();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectDatetimeCalendar(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif		// RC_INVOKED

#endif // !defined(DATE_TIME_SELECTOR_H)
