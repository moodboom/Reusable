#if !defined(TAB_DLG_CTRL_PAGE_H)
#define TAB_DLG_CTRL_PAGE_H

/////////////////////////////////////////////////////////////////////////////
// TabDlgCtrlPage declaration

#include <NestedDialog.h>												// Base class.


class TabDlgCtrlPage : public NestedDialog
{
   typedef NestedDialog inherited;

public:
// Construction
	TabDlgCtrlPage ();	// Default Constructor
	TabDlgCtrlPage (UINT nIDTemplate, CWnd* pParent = NULL);	// Standard Constructor
// Destruction
	~TabDlgCtrlPage ();

	// Enable/disable command routing to the parent.
	void EnableRouteCommand(bool bRoute = true) { m_bRouteCommand = bRoute; };
	bool IsRouteCommand() { return m_bRouteCommand; };
	// Enable CmdMsg routing to the parent.
	void EnableRouteCmdMsg(bool bRoute = true) { m_bRouteCmdMsg = bRoute; };
	bool IsRouteCmdMsg() { return m_bRouteCmdMsg; };
	// Enable/Disable Notify routing to the parent.
	void EnableRouteNotify(bool bRoute = true) { m_bRouteNotify = bRoute; };
	bool IsRouteNotify() { return m_bRouteNotify; };

protected:
// Message Handlers
	virtual BOOL OnCommand (WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnOK (void);
	virtual void OnCancel (void);
	virtual BOOL OnCmdMsg (UINT nID, int nCode, void* pExtra,
		AFX_CMDHANDLERINFO* pHandlerInfo);

// Routing flags
	bool m_bRouteCommand;
	bool m_bRouteCmdMsg;
	bool m_bRouteNotify;
};

#endif // !defined(TAB_DLG_CTRL_PAGE_H)
