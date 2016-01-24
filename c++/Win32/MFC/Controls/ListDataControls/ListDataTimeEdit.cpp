// ListDataTimeEdit.cpp : implementation file
//

#include "stdafx.h"

#include "..\..\BaseDateTime.h"			// This does our formatting for us.

#include "ListDataTimeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataTimeEdit


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataTimeEdit, ListDataLongEdit)
	//{{AFX_MSG_MAP(ListDataTimeEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// GetControlValue()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataTimeEdit::ExtractControlValue()
{
	// This class should ONLY be used for data display, not
	// data input.  The format in the edit box does not facilitate
	// data entry.
	ASSERT( false );
}


//-------------------------------------------------------------------//
// strOutput()																			//
//-------------------------------------------------------------------//
// This is the helper that does the formatting.
//-------------------------------------------------------------------//
CString ListDataTimeEdit::strOutput( Long lValue )
{
	if ( lValue > 0 )
	{
		BaseDateTimeSpan tsTime( 0, 0, 0, lValue );
		return tsTime.strConciseOutput();
	} else
		return CString( _T("-") );
}


