// ListDataFileSizeEdit.cpp : implementation file
//

#include "stdafx.h"

#include "..\..\CFileHelpers.h"		// strFileSizeFormat() does our formatting for us.

#include "ListDataFileSizeEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ListDataFileSizeEdit


//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(ListDataFileSizeEdit, ListDataLongEdit)
	//{{AFX_MSG_MAP(ListDataFileSizeEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//-------------------------------------------------------------------//
// GetControlValue()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void ListDataFileSizeEdit::ExtractControlValue()
{
	// This class should ONLY be used for data display, not
	// data input.  The format in the edit box does not facilitate
	// data entry at this point.  Make your edit box read-only.
	ASSERT( false );
}


//-------------------------------------------------------------------//
// strOutput()																			//
//-------------------------------------------------------------------//
// This is the helper that does the formatting.
//-------------------------------------------------------------------//
CString ListDataFileSizeEdit::strOutput( Long lValue )
{
	if ( lValue > 0 )
		return cstrFileSizeFormat( lValue );
	else
		return CString( _T("-") );
}


