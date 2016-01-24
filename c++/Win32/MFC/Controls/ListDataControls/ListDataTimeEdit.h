#if !defined(LIST_DATA_TIME_EDIT_H)
#define LIST_DATA_TIME_EDIT_H


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// ListDataTimeEdit.h : header file
/////////////////////////////////////////////////////////////////////////////
//
// This class behaves just like the base class, except that the display of
// the Long value is formatted as follows:
//
//		"3:43"
//		"32:33"
//		"1:25:26"
//
/////////////////////////////////////////////////////////////////////////////


#include "ListDataLongEdit.h"		// Base class


/////////////////////////////////////////////////////////////////////////////
// ListDataTimeEdit window

class ListDataTimeEdit : public ListDataLongEdit
{

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataTimeEdit)
	//}}AFX_VIRTUAL

// Implementation
public:

	void ExtractControlValue();

protected:

	// This is the helper that does the formatting.
	// Some derived classes override for special display formatting.
	virtual CString strOutput( Long lValue );

	// Generated message map functions
	//{{AFX_MSG(ListDataTimeEdit)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_TIME_EDIT_H)
