#if !defined(LIST_DATA_BYTE_SLIDER_H)
#define LIST_DATA_BYTE_SLIDER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// ListDataByteSlider.h : header file
//

#include "..\ProgressSliderCtrl.h"
#include "ListDataControlData.h"


/////////////////////////////////////////////////////////////////////////////
// ListDataByteSlider window

// class ListDataByteSlider : public CSliderCtrl, public ListDataControlData< BYTE >
class ListDataByteSlider : public ProgressSliderCtrl, public ListDataControlData< BYTE >
{
   typedef ProgressSliderCtrl inherited;

// Construction
public:
	ListDataByteSlider();

	void Initialize(
		BYTE*						pNewDBVar,
		BYTE*						pNewAltDBVar,
		int						nNewListColumn
	);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ListDataByteSlider)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ListDataByteSlider();

	void ExtractControlValue();

	CString GetValueAsString();
	CString GetAltVarAsString();

protected:

	void DisplayControlValue();

	// Generated message map functions
	//{{AFX_MSG(ListDataByteSlider)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(LIST_DATA_BYTE_SLIDER_H)
