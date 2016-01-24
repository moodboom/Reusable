#include "stdafx.h"
#include "TabDlgCtrlPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Construction

TabDlgCtrlPage::TabDlgCtrlPage () {
#ifndef _AFX_NO_OCC_SUPPORT
	AfxEnableControlContainer ();
#endif // !_AFX_NO_OCC_SUPPORT
	m_bRouteCommand = false;
	m_bRouteCmdMsg = false;
	m_bRouteNotify = false;
}

TabDlgCtrlPage::TabDlgCtrlPage (UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: inherited(nIDTemplate, pParent) {
#ifndef _AFX_NO_OCC_SUPPORT
	AfxEnableControlContainer ();
#endif // !_AFX_NO_OCC_SUPPORT
	m_bRouteCommand = false;
	m_bRouteCmdMsg = false;
	m_bRouteNotify = false;
}

/////////////////////////////////////////////////////////////////////////////
// Destruction

TabDlgCtrlPage::~TabDlgCtrlPage () {
}

/////////////////////////////////////////////////////////////////////////////
// Message Handlers

void TabDlgCtrlPage::OnOK (void) {
	//
	// Prevent inherited::OnOK from calling EndDialog.
	//
}

void TabDlgCtrlPage::OnCancel (void) {
	//
	// Prevent inherited::OnCancel from calling EndDialog.
	//
}

BOOL TabDlgCtrlPage::OnCommand (WPARAM wParam, LPARAM lParam) {
	// Call base class OnCommand to allow message map processing
	BOOL bReturn = inherited::OnCommand (wParam, lParam);

	if (true == m_bRouteCommand)
	{
		//
		// Forward WM_COMMAND messages to the dialog's parent.
		//
		return GetParent ()->SendMessage (WM_COMMAND, wParam, lParam);
	}

	return bReturn;
}

BOOL TabDlgCtrlPage::OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult) {
	BOOL bReturn = inherited::OnNotify(wParam, lParam, pResult);

	if (true == m_bRouteNotify)
	{
		//
		// Forward WM_NOTIFY messages to the dialog's parent.
		//
		return GetParent ()->SendMessage (WM_NOTIFY, wParam, lParam);
	}

	return bReturn;
}

BOOL TabDlgCtrlPage::OnCmdMsg (UINT nID, int nCode, void* pExtra,
	AFX_CMDHANDLERINFO* pHandlerInfo) {
	BOOL bReturn = inherited::OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);

#ifndef _AFX_NO_OCC_SUPPORT
	if (true == m_bRouteCmdMsg)
	{
		//
		// Forward ActiveX control events to the dialog's parent.
		//
		if (nCode == CN_EVENT)
			return GetParent ()->OnCmdMsg (nID, nCode, pExtra, pHandlerInfo);
	}
#endif // !_AFX_NO_OCC_SUPPORT

	return bReturn;
}
