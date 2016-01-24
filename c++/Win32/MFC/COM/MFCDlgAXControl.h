#if !defined(MFC_DLG_AX_CONTROL_H)
#define MFC_DLG_AX_CONTROL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MFCDlgAXControl.h : Declaration of the MFCDlgAXControl ActiveX Control class.

#include "MainDialog.h"

/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl : See MFCDlgAXControl.cpp for implementation.

class MFCDlgAXControl : public COleControl
{
    DECLARE_DYNCREATE(MFCDlgAXControl)

// Constructor
public:
    MFCDlgAXControl();

// Overrides
	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(MFCDlgAXControl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
    virtual ~MFCDlgAXControl();

	MainDialog	m_MainDialog;

    DECLARE_OLECREATE_EX(MFCDlgAXControl)    // Class factory and guid
    DECLARE_OLETYPELIB(MFCDlgAXControl)      // GetTypeInfo
    DECLARE_PROPPAGEIDS(MFCDlgAXControl)     // Property page IDs
    DECLARE_OLECTLTYPE(MFCDlgAXControl)       // Type name and misc status

// Message maps
    //{{AFX_MSG(MFCDlgAXControl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
    //{{AFX_DISPATCH(MFCDlgAXControl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
    //{{AFX_EVENT(MFCDlgAXControl)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
    //{{AFX_DISP_ID(MFCDlgAXControl)
		// NOTE: ClassWizard will add and remove enumeration elements here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DISP_ID
	};
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(MFC_DLG_AX_CONTROL_H)
