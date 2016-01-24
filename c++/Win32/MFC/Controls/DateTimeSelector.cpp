// DateTimeSelector.cpp : implementation file
//

#include "stdafx.h"
#include "DateTimeSelector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DateTimeSelector dialog


//-------------------------------------------------------------------//
// DateTimeSelector()																//
//-------------------------------------------------------------------//
DateTimeSelector::DateTimeSelector(CWnd* pParent /*=NULL*/)
	: inherited(DateTimeSelector::IDD, pParent)
{
	//{{AFX_DATA_INIT(DateTimeSelector)
	//}}AFX_DATA_INIT

}


//-------------------------------------------------------------------//
// OnInitDialog()																		//
//-------------------------------------------------------------------//
// Patch things up...
//-------------------------------------------------------------------//
BOOL DateTimeSelector::OnInitDialog() 
{
	// User provides the initial date/time through m_SelectedDateTime.
	// If you hit ASSERT()'s here, make sure u provided a valid value
	// as follows:
	//
	//		DateTimeSelector dtsDlg( this );           
	//		dtsDlg.m_SelectedDateTime = m_dtCreateTime;
	//		if ( dtsDlg.DoModal() == IDOK )            
	//			m_dtCreateTime = dtsDlg.m_SelectedDateTime;
	//

	// Use it to set our protected members.
	ASSERT( m_SelectedDateTime.GetStatus() == COleDateTime::valid );
	m_CalendarDate = m_SelectedDateTime;
	m_SelectedDate = m_SelectedDateTime;
	
	// These MS classes are pretty damned ignorant rrr...
	SYSTEMTIME stTemp;
	VERIFY( m_SelectedDateTime.GetAsSystemTime( stTemp ) );
	m_SelectedTime = CTime( stTemp );
		
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//-------------------------------------------------------------------//
// DoDataExchange()																	//
//-------------------------------------------------------------------//
void DateTimeSelector::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DateTimeSelector)
	DDX_DateTimeCtrl(pDX, IDC_DATETIME_DATE_SELECTOR, m_SelectedDate);
	DDX_DateTimeCtrl(pDX, IDC_DATETIME_TIME_SELECTOR, m_SelectedTime);
	DDX_MonthCalCtrl(pDX, IDC_DATETIME_CALENDAR, m_CalendarDate);
	//}}AFX_DATA_MAP
}


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(DateTimeSelector, CDialog)
	//{{AFX_MSG_MAP(DateTimeSelector)
	ON_NOTIFY(MCN_SELCHANGE, IDC_DATETIME_CALENDAR, OnSelchangeDatetimeCalendar)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIME_DATE_SELECTOR, OnDatetimechangeDatetimeDateSelector)
	ON_BN_CLICKED(IDC_DATETIME_TODAY, OnDatetimeToday)
	ON_BN_CLICKED(IDC_DATETIME_NOW, OnDatetimeNow)
	ON_NOTIFY(MCN_SELECT, IDC_DATETIME_CALENDAR, OnSelectDatetimeCalendar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// DateTimeSelector message handlers


//-------------------------------------------------------------------//
// OnSelectDatetimeCalendar()														//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DateTimeSelector::OnSelectDatetimeCalendar(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Not workin'...
	/*

	// Before we get the calendar control result, set a valid date
	// or we will fail when validating the time for the retrieved date.
	// FU, MS.
	m_CalendarDate = COleDateTime::GetCurrentTime();

	// Now extract from the calendar.
	UpdateData();

	m_SelectedDate = m_CalendarDate;

	// Redisplay.
	UpdateData( FALSE );
	*/

	// Not workin'...
	// So we removed calendar ctrl member var...
	/*
	UpdateData();

	m_SelectedDate = m_CalendarDate;

	// Redisplay.
	UpdateData( FALSE );
	*/

	// Not workin'...
	/*
	if ( m_CalendarCtrl.GetCurSel( m_CalendarDate ) )
	{
		m_SelectedDate = m_CalendarDate;

		// Redisplay.
		UpdateData( FALSE );
	}
	*/

	*pResult = 0;
}


//-------------------------------------------------------------------//
// OnSelchangeDatetimeCalendar()													//
//-------------------------------------------------------------------//
// Update from calendar to selector.
//
// NOTE: This notification message is similar to MCN_SELECT, but 
// it is sent in response to any selection change. MCN_SELECT is 
// sent only for an explicit date selection.  For example, the 
// control sends MCN_SELCHANGE when the user explicitly changes 
// his or her selection within the current month or when the 
// selection is implicitly changed in response to next/previous 
// month navigation.
//-------------------------------------------------------------------//
void DateTimeSelector::OnSelchangeDatetimeCalendar(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE)pNMHDR;

	// Update the date selector.
	m_SelectedDate = lpNMSelChange->stSelStart;

	// Redisplay JUST THE DATE SELECTOR or we have problems.
	// UpdateData( FALSE );
	CDataExchange DX( 
		this,				//	m_pDlgWnd
		FALSE				// m_bValidateAndSave
	);
	DDX_DateTimeCtrl( &DX, IDC_DATETIME_DATE_SELECTOR, m_SelectedDate);

	*pResult = 0;
}


//-------------------------------------------------------------------//
// OnDatetimechangeDatetimeDateSelector()										//
//-------------------------------------------------------------------//
// Update from selector to calendar.
//-------------------------------------------------------------------//
void DateTimeSelector::OnDatetimechangeDatetimeDateSelector(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Make sure we have the updated data.
	UpdateData();

	m_CalendarDate = m_SelectedDate;

	// Redisplay.
	UpdateData( FALSE );

	*pResult = 0;
}


//-------------------------------------------------------------------//
// OnDatetimeToday()																	//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DateTimeSelector::OnDatetimeToday() 
{
	COleDateTime dtNow( time(0) );
	m_SelectedDate = dtNow;
	m_CalendarDate = dtNow;

	// Redisplay.
	UpdateData( FALSE );
}


//-------------------------------------------------------------------//
// OnDatetimeNow()																	//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DateTimeSelector::OnDatetimeNow() 
{
	m_SelectedTime = CTime( time(0) );

	// Redisplay.
	UpdateData( FALSE );
}


//-------------------------------------------------------------------//
// OnOK()																				//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DateTimeSelector::OnOK() 
{
	// Get the latest shit.
	UpdateData();

	// Extract the final value.
	m_SelectedDateTime.SetDateTime(
		m_SelectedDate.GetYear(),			// int nYear
		m_SelectedDate.GetMonth(),			// int nMonth 
		m_SelectedDate.GetDay(),			// int nDay
		m_SelectedTime.GetHour(),			// int nHour
		m_SelectedTime.GetMinute(),		// int nMin 
		m_SelectedTime.GetSecond()			// int nSec 
	);
	
	CDialog::OnOK();
}


