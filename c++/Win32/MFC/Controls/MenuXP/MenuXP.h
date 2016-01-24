//-------------------------------------------------------------------//
// MenuXP : Owner-draw menu including a custom font.
//
// This class is made to be easy to use, despite the fact that the
// silly MFC/Win32 API combo requires the parent of the menu, not 
// the CMenu, to handle the majority of the menu drawing messages.
//
// To use this class, dump the following macros into the class
// that will handle the menu.  This is typically a CMDIFrameWnd
// for MDI apps.  Any CWnd that hosts popup menus may also be "macro'ed".
//
//		In host interface file:
//		
//			#include <Controls/MenuXP/MenuXP.h>
//			
//			In host class (in a protected section):
//			
//				DECLARE_MENUXP()
//
//		In host implementation file:
//
//			IMPLEMENT_MENUXP(CadMainFrame, CMDIFrameWnd, true);
//
//			In host message map (add to end, outside of ClassWizard block):
//
//				ON_MENUXP_MESSAGES()
//
//	That's it!  No CMenu's to replace or anything, since it is the
// parent of the CMenu that does the work (through CMenuXP static
// functions).  Sometimes it is better to break the rules.  :>
//
// Based on Jean-Michel LE FOL's CMenuXP work on codeProject.com.
//
//	Copyright © 2002 A better Software.
//-------------------------------------------------------------------//

#pragma once

#include <afxtempl.h>

#include "..\..\FontHelpers.h"		// For SimpleFont


///////////////////////////////////////////////////////////////////////////////
class CImgDesc
{
public:
    HIMAGELIST m_hImgList;
    int        m_nIndex;

    CImgDesc (HIMAGELIST hImgList = NULL, int nIndex = 0) : m_hImgList (hImgList), m_nIndex (nIndex)
    {
    }
};

///////////////////////////////////////////////////////////////////////////////
class CMenuXP : public CMenu
{
// Attributes
public:
    static CMap <int, int, CString, CString&> m_sCaptions;
    static CMap <int, int, CImgDesc, CImgDesc&> m_Images;

// Operations
public:
    static void SetWinXPStyle (CMenu* pMenu, CWnd* pWnd = NULL);
    static bool IsOwnerDrawn (HMENU hMenu);
    static void OnMeasureItem (MEASUREITEMSTRUCT* pMeasureItemStruct);
    static void OnDrawItem (DRAWITEMSTRUCT* pDrawItemStruct);
    static LRESULT OnMenuChar (HMENU hMenu, UINT nChar, UINT nFlags);

	static void SetCustomFont( SimpleFont& sf );
 	static void SetSystemFont()							{ m_psfCustom = 0;	}

protected:

	static bool bUseCustomFont() { return ( m_psfCustom != 0 ); }

	static SimpleFont*	m_psfCustom;
	static CFont			m_cfCustom;

};

#define DECLARE_MENUXP()                                                             \
    protected:                                                                       \
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);     \
    afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); \
    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);          \
    afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);

#define ON_MENUXP_MESSAGES() \
    ON_WM_INITMENUPOPUP()    \
    ON_WM_MEASUREITEM()      \
    ON_WM_DRAWITEM()         \
    ON_WM_MENUCHAR()

#define IMPLEMENT_MENUXP(theClass, baseClass, bFlag)                               \
    void theClass::OnInitMenuPopup (CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) \
    {                                                                              \
        baseClass::OnInitMenuPopup (pPopupMenu, nIndex, bSysMenu);                 \
                                                                                   \
        if ( bFlag && !bSysMenu )                                                  \
        {                                                                          \
            CMenuXP::SetWinXPStyle (pPopupMenu, this);                    \
        }                                                                          \
    }                                                                              \
    void theClass::OnMeasureItem (int, LPMEASUREITEMSTRUCT lpMeasureItemStruct)    \
    {                                                                              \
        CMenuXP::OnMeasureItem (lpMeasureItemStruct);                              \
    }                                                                              \
    void theClass::OnDrawItem (int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)      \
    {                                                                              \
        if ( lpDrawItemStruct->CtlType == ODT_MENU )										  \
				CMenuXP::OnDrawItem (lpDrawItemStruct);                                \
																											  \
			/* MDM We let the function pass on through.		*/								  \
			/* We have other Menu classes that need this.	*/								  \
			baseClass::OnDrawItem( nIDCtl, lpDrawItemStruct );								  \
    }                                                                              \
    LRESULT theClass::OnMenuChar (UINT nChar, UINT nFlags, CMenu* pMenu)           \
    {                                                                              \
        if ( CMenuXP::IsOwnerDrawn (pMenu->m_hMenu) )                              \
        {                                                                          \
            return CMenuXP::OnMenuChar (pMenu->m_hMenu, nChar, nFlags);            \
        }                                                                          \
        return baseClass::OnMenuChar (nChar, nFlags, pMenu);                       \
    }

///////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
