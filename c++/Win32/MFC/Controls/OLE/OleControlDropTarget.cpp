//-------------------------------------------------------------------//
// OleListDropTarget implementation
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "OleControl.h"					// Contained by this, we send it d/d calls

#include "OleControlDropTarget.h"


//-------------------------------------------------------------------//
// OnDragOver()																		//
//-------------------------------------------------------------------//
// The following overrides catch drag and drop occurences
// and pass them back to the control for processing.
// Thanks, Microsoft, for having to un-re-design your crappy class
// structure.
//-------------------------------------------------------------------//
DROPEFFECT OleControlDropTarget::OnDragOver(
	CWnd* pWnd,
	COleDataObject* pDataObject,
	DWORD dwKeyState,
	CPoint point
) {
	return m_pControl->OnDragOver(
		pWnd,
		pDataObject,
		dwKeyState,
		point
	);
}
BOOL OleControlDropTarget::OnDrop(
	CWnd* pWnd,
	COleDataObject* pDataObject,
	DROPEFFECT dropEffect,
	CPoint point
) {
	return m_pControl->OnDrop(
		pWnd,
		pDataObject,
		dropEffect,
		point
	);
}
