//-------------------------------------------------------------------//
//	Window Helpers
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef WINDOW_HELPERS_H
	#define WINDOW_HELPERS_H

// This updates the given rect, in screen coordinates, so that it
// is completely visible under the current resolution.  If it is
// already on-screen, it is not adjusted.
CRect GetOnScreenRect( CRect rectWnd );

CSize GetDialogSizeFromTemplate( UINT nIDTemplate, CWnd* pParent);

CWnd* GetAnyAvailableTopmostWindow();

#endif	// WINDOW_HELPERS_H
