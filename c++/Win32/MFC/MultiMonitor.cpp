//-------------------------------------------------------------------//
// MultiMonitor.h   : Multi-monitor helper functions
//
// Based on code from CodeProject.com with this original notice:
// Author: Donald Kackman
// Email:  don@itsEngineering.com
// Copyright 2002, Donald Kackman
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
//	Copyright © 2005 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"
#include "MultiMonitor.h"

// CMonitor

// constucts a monitor class not attached to any handle
CMonitor::CMonitor() : m_hMonitor( NULL )
{	
}


// copy constructor
CMonitor::CMonitor( const CMonitor& monitor  )
{
	m_hMonitor = (HMONITOR)monitor;
}

CMonitor::~CMonitor()
{
}



void CMonitor::Attach( const HMONITOR hMonitor )
{
	ASSERT( CMonitors::IsMonitor( hMonitor ) );

	m_hMonitor = hMonitor;
}

HMONITOR CMonitor::Detach() 
{
	HMONITOR hMonitor = m_hMonitor;
	m_hMonitor = NULL;
	return hMonitor;
}

// creates an HDC for the monitor
// it is up to the client to call DeleteDC
//
// for normal multimonitor drawing it is not necessary to get a
// dc for each monitor. Windows takes care of drawing correctly
// on all monitors
//
// Only very exacting applications would need a DC for each monitor
HDC CMonitor::CreateDC() const
{
	ASSERT( IsMonitor() );

	CString name;
	GetName( name );

	//create a dc for this display
	HDC hdc = ::CreateDC( name, name, NULL, NULL );
	ASSERT( hdc != NULL );

	//set the viewport based on the monitor rect's relation to the primary monitor
	CRect rect;
	GetMonitorRect( &rect );

	::SetViewportOrgEx( hdc, -rect.left, -rect.top, NULL );
	::SetViewportExtEx( hdc, rect.Width(), rect.Height(), NULL );

	return hdc;
}

int CMonitor::GetBitsPerPixel() const
{
	HDC hdc = CreateDC();
	int ret = ::GetDeviceCaps( hdc, BITSPIXEL ) * ::GetDeviceCaps( hdc, PLANES );
	VERIFY( ::DeleteDC( hdc ) );

	return ret;
}

void CMonitor::GetName( CString& string ) const
{
	ASSERT( IsMonitor() );

	MONITORINFOEX mi;
	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( m_hMonitor, &mi );

	string = mi.szDevice;
}

//
// these methods return true if any part of the item intersects the monitor rect
BOOL CMonitor::IsOnMonitor( const POINT pt ) const
{
	CRect rect;
	GetMonitorRect( rect );

	return rect.PtInRect( pt );
}

BOOL CMonitor::IsOnMonitor( const CWnd* pWnd ) const
{
	CRect rect;
	GetMonitorRect( rect );

	ASSERT( ::IsWindow( pWnd->GetSafeHwnd() ) );
	CRect wndRect;
	pWnd->GetWindowRect( &wndRect );

	return rect.IntersectRect( rect, wndRect );
}

BOOL CMonitor::IsOnMonitor( const LPRECT lprc ) const
{
	CRect rect;
	GetMonitorRect( rect );

	return rect.IntersectRect( rect, lprc );
}


void CMonitor::GetMonitorRect( LPRECT lprc ) const
{
	ASSERT( IsMonitor() );
	
	MONITORINFO mi;
    RECT        rc;

	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( m_hMonitor, &mi );
	rc = mi.rcMonitor;

	::SetRect( lprc, rc.left, rc.top, rc.right, rc.bottom );
}

//
// the work area does not include the start bar
void CMonitor::GetWorkAreaRect( LPRECT lprc ) const
{
	ASSERT( IsMonitor() );

	MONITORINFO mi;
    RECT        rc;

	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( m_hMonitor, &mi );
	rc = mi.rcWork;

	::SetRect( lprc, rc.left, rc.top, rc.right, rc.bottom );
}

//these two center methods are adapted from David Campbell's
//MSJ article (see comment at the top of the header file)
void CMonitor::CenterRectToMonitor( LPRECT lprc, const BOOL UseWorkAreaRect ) const
{
    int  w = lprc->right - lprc->left;
    int  h = lprc->bottom - lprc->top;

    CRect rect;
    if ( UseWorkAreaRect )
        GetWorkAreaRect( &rect );
    else
        GetMonitorRect( &rect );

    lprc->left = rect.left + ( rect.Width() - w ) / 2;
    lprc->top = rect.top + ( rect.Height() - h ) / 2;
    lprc->right	= lprc->left + w;
    lprc->bottom = lprc->top + h;
}

void CMonitor::CenterWindowToMonitor( CWnd* const pWnd, const BOOL UseWorkAreaRect ) const
{
	ASSERT( IsMonitor() );
	ASSERT( pWnd );
	ASSERT( ::IsWindow( pWnd->m_hWnd ) );

	CRect rect;
    pWnd->GetWindowRect( &rect );
    CenterRectToMonitor( &rect, UseWorkAreaRect );
    pWnd->SetWindowPos( NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
}

void CMonitor::ClipRectToMonitor( LPRECT lprc, const BOOL UseWorkAreaRect ) const
{
    int w = lprc->right - lprc->left;
    int h = lprc->bottom - lprc->top;

    CRect rect;
    if ( UseWorkAreaRect )
        GetWorkAreaRect( &rect );
    else
        GetMonitorRect( &rect );

    lprc->left = max( rect.left, min( rect.right - (LONG)w, lprc->left ) );
    lprc->top = max( rect.top, min( rect.bottom - (LONG)h, lprc->top ) );
    lprc->right = lprc->left + w;
    lprc->bottom = lprc->top  + h;
}

//
// is the instance the primary monitor
BOOL CMonitor::IsPrimaryMonitor() const
{
	ASSERT( IsMonitor() );

	MONITORINFO mi;

	mi.cbSize = sizeof( mi );
	::GetMonitorInfo( m_hMonitor, &mi );
	
	return mi.dwFlags == MONITORINFOF_PRIMARY;
}

//
// is the instance currently attached to a valid monitor handle
BOOL CMonitor::IsMonitor() const 
{
	return CMonitors::IsMonitor( m_hMonitor );
}



// -----------------------------------------



// CMonitors

CMonitors::CMonitors()
{
	m_MonitorArray.SetSize( GetMonitorCount() );

	ADDMONITOR addMonitor;
	addMonitor.pMonitors = &m_MonitorArray;
	addMonitor.currentIndex = 0;

	::EnumDisplayMonitors( NULL, NULL, AddMonitorsCallBack, (LPARAM)&addMonitor );
}

CMonitors::~CMonitors()
{
	for ( int i = 0; i < m_MonitorArray.GetSize(); i++ )
		delete m_MonitorArray.GetAt( i );
}


// CMonitors member functions

BOOL CALLBACK CMonitors::AddMonitorsCallBack( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	LPADDMONITOR pAddMonitor = (LPADDMONITOR)dwData;

	CMonitor* pMonitor = new CMonitor;
	pMonitor->Attach( hMonitor );

	pAddMonitor->pMonitors->SetAt( pAddMonitor->currentIndex, pMonitor );
	pAddMonitor->currentIndex++;

	return TRUE;
}
//
// returns the primary monitor
CMonitor CMonitors::GetPrimaryMonitor()
{
	//the primary monitor always has its origin at 0,0
	HMONITOR hMonitor = ::MonitorFromPoint( CPoint( 0,0 ), MONITOR_DEFAULTTOPRIMARY );
	ASSERT( IsMonitor( hMonitor ) );

	CMonitor monitor;
	monitor.Attach( hMonitor );
	ASSERT( monitor.IsPrimaryMonitor() );

	return monitor;
}

//
// is the given handle a valid monitor handle
BOOL CMonitors::IsMonitor( const HMONITOR hMonitor )
{
	if ( hMonitor == NULL )
		return FALSE;

	MATCHMONITOR match;
	match.target = hMonitor;
	match.foundMatch = FALSE;

	::EnumDisplayMonitors( NULL, NULL, FindMatchingMonitorHandle, (LPARAM)&match );

	return match.foundMatch;
}



//this is the callback method that gets called via IsMontior
BOOL CALLBACK CMonitors::FindMatchingMonitorHandle( HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData )
{
	LPMATCHMONITOR pMatch = (LPMATCHMONITOR)dwData;

	if ( hMonitor == pMatch->target )
	{
		//found a monitor with the same handle we are looking for		
		pMatch->foundMatch = TRUE;
		return FALSE; //stop enumerating
	}

	//haven't found a match yet
	pMatch->foundMatch = FALSE;
	return TRUE;	//keep enumerating
}


BOOL CMonitors::AllMonitorsShareDisplayFormat()
{
	return ::GetSystemMetrics( SM_SAMEDISPLAYFORMAT );
}

//
// the number of monitors on the system
int CMonitors::GetMonitorCount()
{ 
	return ::GetSystemMetrics(SM_CMONITORS);
}


CMonitor CMonitors::GetMonitor( const int index ) const
{
#if _MFC_VER >= 0x0700
	ASSERT( index >= 0 && index < m_MonitorArray.GetCount() ); 
#else
	ASSERT( index >= 0 && index < m_MonitorArray.GetSize() );
#endif

	CMonitor* pMonitor = (CMonitor*)m_MonitorArray.GetAt( index );

	return *pMonitor;
}

//
// returns the rectangle that is the union of all active monitors
void CMonitors::GetVirtualDesktopRect( LPRECT lprc )
{
	::SetRect( lprc, 
				::GetSystemMetrics( SM_XVIRTUALSCREEN ),
				::GetSystemMetrics( SM_YVIRTUALSCREEN ),
				::GetSystemMetrics( SM_CXVIRTUALSCREEN ),
				::GetSystemMetrics( SM_CYVIRTUALSCREEN ) );
	
}

//
// these methods determine wheter the given item is
// visible on any monitor
BOOL CMonitors::IsOnScreen( const LPRECT lprc )
{
	return ::MonitorFromRect( lprc, MONITOR_DEFAULTTONULL ) != NULL;
}

BOOL CMonitors::IsOnScreen( const POINT pt )
{
	return ::MonitorFromPoint( pt, MONITOR_DEFAULTTONULL ) != NULL;
}

BOOL CMonitors::IsOnScreen( const CWnd* pWnd )
{
	return ::MonitorFromWindow( pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONULL ) != NULL;
}

CMonitor CMonitors::GetNearestMonitor( const LPRECT lprc )
{
	CMonitor monitor;
	monitor.Attach( ::MonitorFromRect( lprc, MONITOR_DEFAULTTONEAREST ) );

	return monitor;

}

CMonitor CMonitors::GetNearestMonitor( const POINT pt )
{
	CMonitor monitor;
	monitor.Attach( ::MonitorFromPoint( pt, MONITOR_DEFAULTTONEAREST ) );

	return monitor;
}

CMonitor CMonitors::GetNearestMonitor( const CWnd* pWnd )
{
	ASSERT( pWnd );
	ASSERT( ::IsWindow( pWnd->m_hWnd ) );

	CMonitor monitor;
	monitor.Attach( ::MonitorFromWindow( pWnd->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST ) );

	return monitor;
}



// -----------------------------------------



// CMonitorDC

CMonitorDC::CMonitorDC( CMonitor* pMonitor )
{
	ASSERT( pMonitor );
	ASSERT( pMonitor->IsMonitor() );

	VERIFY( Attach( pMonitor->CreateDC() ) );
}

CMonitorDC::~CMonitorDC()
{
	VERIFY( DeleteDC() );
}


