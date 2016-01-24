//********************************************************
//                                                            
//	FILENAME	: CBmpMenu.h
//	CONTENTS	: CBmpMenu class header file.
//
//  Copyright(c), 1999-2000.					  
//	All rights reserved									  
//                                                        
//  Author		:  Dipti Deogade
//                                                        
//***************************************************************************************

//** REVISION LIST **********************************************************************
//				 										  
// Number	Date		Author		Description
//
//
//***************************************************************************************

#if !defined(AFX_CBMPMENU_H__53F51970_5150_11D3_AB49_0004AC25CC15__INCLUDED_)
#define AFX_CBMPMENU_H__53F51970_5150_11D3_AB49_0004AC25CC15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//Internal user defined messages
#define WM_LBUTTONDOWNAFTER		WM_USER + 101
#define WM_RESETDATA			WM_USER + 102
#define WM_POPUPSUBMENU			WM_USER + 103

//Global contants
#define MENU_SELECTFIRSTITEM	0x0100
#define OBM_CHECK				32760			//OEM bitmap id for checkmark...taken from winuser.h

//***************************************************************************************
// The CBmpMenu class provides a way to create popup menus with vertical bitmap
//(like windows start bitmap). Also an user can show a child controls by the
//side of menu items. CBmpMenu window provides a placeholder window for child controls.
//The menu behavior is implemented using a toolbar window, of type MenuToolBar class. 
//
//In order to use CBmpMenu class, user needs to use the functions implemented by CMenu for 
//showing a popup menu. All the Microsoft documentation for popup menus applies to CBmpMenu class.
//Only the deviations from that documentation are described below:-
//
// 1) An additional constructor has been implemented to pass on the vertical
//		bitmap handle, bitmap width etc.
//
//     CBmpMenu(int nBitmapW=0, BOOL bShowBmp4SubMenu=FALSE, HBITMAP hBitmap=0, BOOL bStretchBmp=TRUE);
//	   nBitmapW = Width of the blank space to be shown to the left side of menu items in a popup menu.
//	   bShowBmp4SubMenu = If TRUE, then blank space is shown for all submenus of a popup menu
//						  If FALSE, then blank space is shown only for main menu.
//	   hBitmap = If a valid handle is passed then this bitmap is drawn on the blank space
//	   bStretchBmp = If TRUE, then bitmap is drawn using StretchBlt.
//					 If False, then the blank space is filled with pattern brush created from the bitmap
//		
// 2) The parameters passed in the message WM_ENTERMENULOOP, to the owner window of the 
//    menu has been changed.
//
//	  WM_ENTERMENULOOP:
//    wParam = (HMENU)hMenu;  //handle of menu which is entering the modal loop
//    lParam = (HEND)hWindow; //handle of menu window(menu window is of type CBmpMenu)
//
//	  Remarks:
//    If user wants to show a child control e.g. slider control to the left side of the control,
//	  then he needs to handle WM_ENTERMENULOOP in the owner window of the menu. Create child
//	  control with hWindow as parent and place them in the menu window using MoveWindow. User should
//	  take care that control is placed only in the blank space. Otherwise, it may not draw properly.
//
//***************************************************************************************

/////////////////////////////////////////////////////////////////////////////
// MenuToolBar window

class MenuToolBar : public CToolBar
{
private:		
	int		m_nSelectedItem;		//Index of the hot item
	int		m_nLastLBDownIndex;		//Index of the button for which left mouse click event was last processed
	CFont	m_oMenuFont;			//Stores the menu font
	CPoint	m_oHoverPt;				//Stores the last point in the client area of toolbar window, for which WM_MOSEHOVER was created
	int		m_nLastHoverIndex;		//Index of the button for which WM_MOUSEHOVE message was last created

// Construction
public:
	MenuToolBar();					//default constructor

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MenuToolBar)
	protected:
	virtual void OnCustomDrawNotify(LPARAM lParam, LRESULT* pResult );	//NM_CUSTOMDRAW handler
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~MenuToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(MenuToolBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) ;
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	
	void	OnPostLbuttonMsg(UINT nFlags, LPARAM point);			//Called from OnLbuttonDown using PostMessage
	BOOL	KeyboardFilter(UINT nChar, UINT nRepCnt, UINT nFlags) ; //Helper function used by OnKeyDown
	BOOL	Draw3DCheckmark(CDC& dc, const CRect rc, BOOL bSelected, HBITMAP hbmCheck,
				BOOL bDrawSunkenBdr, BOOL bGrayImage);				//Helper function used by OnCustomDrawNotify
																	//to draw image in checked menu item

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CBmpMenu window

class CBmpMenu : public CWnd, public CMenu
{
	DECLARE_DYNAMIC(CBmpMenu);

// Construction
public:
	//Constructor
	CBmpMenu(int nBitmapW=0, BOOL bShowBmp4SubMenu=FALSE, HBITMAP hBitmap=0, BOOL bStretchBmp=TRUE);

	//CMenu overridables
	BOOL TrackPopupMenu( UINT nFlags, int x, int y, CWnd* pWnd, CRect* pMenuItemRect=NULL);
	void operator delete( void* p ){ delete(p);};

// Attributes
public:
	CWnd*		m_pOwnerWnd;	//pointer to owner window

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBmpMenu)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
// Implementation
	virtual ~CBmpMenu();
	BOOL	Attach( HMENU hMenu );
	HMENU	Detach();

	void	DestroyRootMenu();		//Destroys all the menu windows starting from root menu window
	void	DestroySubMenus();		//Destroys all submenu windows of this menu window

	// Generated message map functions
protected:

	void	InitToolBarData(CToolBar* pToolBar, CPoint pt, CRect* pRect); //Used to initialize toolbar and for menu window placement
	void	Cleanup();				//deletes allocated memory 

	//{{AFX_MSG(CBmpMenu)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnPaint();
	//}}AFX_MSG

	void PopupSubMenu(WPARAM wp, BOOL bSelectFirstItem);	
			//pops up submenu at point CPoint(LOWORD(wp), HIWORD(wp)).
			//If bSelectFirstItem is TRUE then the first item in the submenu is shown as a hot item.												

	DECLARE_MESSAGE_MAP()

private:

	void PositionMenuWindow(CPoint pt, CRect* pItemRect, CRect rect);		//Main menu window placement procedure
	BOOL PositionSubMenu(CPoint pt, CRect menuRect, BOOL bRtAlign, BOOL bDnAlign); //submenu window placement procedure

	CBmpMenu*		m_pSubMenuWnd;	//pointer to submenu window
	HBITMAP			m_hBitmap;		//handle of vertical bitmap
	BOOL			m_bShowBmpAll;	//Flag which indicates whether leave a vertical blank space for all submenus or not
	BOOL			m_bStretchBmp;  //Flag indicating whether to use StretchBlt or PatBlt for filling up blank space
	int				m_nTBOffSet;    //Width of blank space to be shown to left side of menu items
	BOOL			m_bSubMenu;		//Flag indicating whether this menu is a submenu or not
	MenuToolBar*	m_pToolbar;		//pointer to the toolbar window
};

/////////////////////////////////////////////////////////////////////////////
HBITMAP GetSysColorBitmap(HDC hDC, HBITMAP hSourceBitmap, BOOL bMono, BOOL bSelected);
		//returns a bitmap with changed background colours of the image passed in hSourceBitmap

/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CBMPMENU_H__53F51970_5150_11D3_AB49_0004AC25CC15__INCLUDED_)
