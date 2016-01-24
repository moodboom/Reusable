// MFCDlgAXControl.cpp : Implementation of the MFCDlgAXControl ActiveX Control class.

#include "stdafx.h"
#include "CXEdit.h"
#include "MFCDlgAXControl.h"
#include "CXEditPpg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(MFCDlgAXControl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(MFCDlgAXControl, COleControl)
    //{{AFX_MSG_MAP(MFCDlgAXControl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_EDIT, OnEdit)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(MFCDlgAXControl, COleControl)
    //{{AFX_DISPATCH_MAP(MFCDlgAXControl)
	// NOTE - ClassWizard will add and remove dispatch map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DISPATCH_MAP
    DISP_FUNCTION_ID(MFCDlgAXControl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(MFCDlgAXControl, COleControl)
    //{{AFX_EVENT_MAP(MFCDlgAXControl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(MFCDlgAXControl, 1)
    PROPPAGEID(CCXEditPropPage::guid)
END_PROPPAGEIDS(MFCDlgAXControl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(MFCDlgAXControl, "CXEdit.MFCDlgAXControl.1",
	0xdf1c8708, 0xd598, 0x49d4, 0x8a, 0x57, 0xab, 0x8, 0xe2, 0xa, 0x44, 0x43)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(MFCDlgAXControl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DCXEdit =
		{ 0xea9f10db, 0x2821, 0x4160, { 0xa6, 0x3, 0x45, 0xcc, 0xe0, 0xb0, 0x4f, 0x27 } };
const IID BASED_CODE IID_DCXEditEvents =
		{ 0xa849fd96, 0x939c, 0x4b23, { 0xbb, 0xd0, 0x39, 0x2, 0xfb, 0x8b, 0x3c, 0x83 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwCXEditOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(MFCDlgAXControl, IDS_CXEdit, _dwCXEditOleMisc)


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::MFCDlgAXControlFactory::UpdateRegistry -
// Adds or removes system registry entries for MFCDlgAXControl

BOOL MFCDlgAXControl::MFCDlgAXControlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegInsertable | afxRegApartmentThreading to afxRegInsertable.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
            IDS_CXEdit,
            IDB_CXEdit,
			afxRegInsertable | afxRegApartmentThreading,
            _dwCXEditOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::MFCDlgAXControl - Constructor

MFCDlgAXControl::MFCDlgAXControl()
{
    InitializeIIDs(&IID_DCXEdit, &IID_DCXEditEvents);

	// TODO: Initialize your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::~MFCDlgAXControl - Destructor

MFCDlgAXControl::~MFCDlgAXControl()
{
	// TODO: Cleanup your control's instance data here.
}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::OnDraw - Drawing function

void MFCDlgAXControl::OnDraw(
	CDC* pdc,
	const CRect& rcBounds,
	const CRect& rcInvalid
) {

	// MDM	11/15/2000 12:14:48 PM
	// There are TWO STATES of an ActiveX control:
	//		RUNTIME		Actually instantiated and running in a container.
	//		DESIGN		Dropped into a container in design mode, NOT instantiated
	//								- just draw a border or something.

	if ( m_MainDialog.GetSafeHwnd() )
	{
		// runtime

		CRect rcDlg( rcBounds );

		// First, get the minimum size allowed for our dialog.
		// The dialog should supply this info in standard fashion,
		// through OnGetMinMaxInfo().
		MINMAXINFO mmi;
		m_MainDialog.OnGetMinMaxInfo( &mmi );
		if ( mmi.ptMinTrackSize.x > rcDlg.Width() )
			rcDlg.right = rcDlg.left + mmi.ptMinTrackSize.x;
		if ( mmi.ptMinTrackSize.y > rcDlg.Height() )
			rcDlg.bottom = rcDlg.top + mmi.ptMinTrackSize.y;

		// Set the scroll dialog's "design size".
		m_MainDialog.SetClientRect( CRect( 0, 0, mmi.ptMinTrackSize.x, mmi.ptMinTrackSize.y ) );

		m_MainDialog.MoveWindow( rcDlg,TRUE );

		// MAY need this?
		// m_MainDialog.RedrawWindow (rcBounds,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_UPDATENOW);

	} else
	{
		// design time (do not use m_MainDialog)
		// because at design time the function
		//	OnCreate(LPCREATESTRUCT lpCreateStruct)
		// is not called

		// now draw something (userinformation etc.)
		//
		pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH) GetStockObject(WHITE_BRUSH)));
		pdc->Ellipse(rcBounds);

		RECT rect;
		int x,y;
		y = rcBounds.Height()/2;
		x= rcBounds.Width()/2;
		rect.left=rcBounds.left+x-80;
		rect.right=rcBounds.left+x+80;
		rect.top=rcBounds.top+y-20;
		rect.bottom=rcBounds.top+y+20;
		pdc->DrawText(
			"Design Time of Control",
			&rect,
			DT_CENTER|DT_SINGLELINE
		);
	}

}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::DoPropExchange - Persistence support

void MFCDlgAXControl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::OnResetState - Reset control to default state

void MFCDlgAXControl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl::AboutBox - Display an "About" box to the user

void MFCDlgAXControl::AboutBox()
{
    CDialog dlgAbout(IDD_ABOUTBOX_CXEdit);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// MFCDlgAXControl message handlers

BOOL MFCDlgAXControl::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.dwExStyle |= WS_EX_CONTROLPARENT;
	return COleControl::PreCreateWindow(cs);
}

int MFCDlgAXControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (COleControl::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_MainDialog.Create( IDD_MAIN_DIALOG, this );

	return 0;
}
