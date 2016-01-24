#if !defined(LIST_DATA_FILE_SIZE_EDIT_H)
#define LIST_DATA_FILE_SIZE_EDIT_H


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// ListDataFileSizeEdit.h : header file
/////////////////////////////////////////////////////////////////////////////
//
// This class behaves just like the base class, except that the display of
// the Long value is formatted as follows (via FileHelpers's
// strFileSizeFormat() ):
//
//		"  3.5 TB   "
//		"  2.4 GB   "
//		" 12.3 MB   "
//		" 32.3 KB   "
//		"125 bytes"
//
/////////////////////////////////////////////////////////////////////////////


#include "ListDataLongEdit.h"		// Base class


/////////////////////////////////////////////////////////////////////////////
// ListDataFileSizeEdit window

class ListDataFileSizeEdit : public ListDataLongEdit
{

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataFileSizeEdit)
	//}}AFX_VIRTUAL

// Implementation
public:

	void ExtractControlValue();

protected:

	// This is the helper that does the formatting.
	// Some derived classes override for special display formatting.
	virtual CString strOutput( Long lValue );

	// Generated message map functions
	//{{AFX_MSG(ListDataFileSizeEdit)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_FILE_SIZE_EDIT_H)
