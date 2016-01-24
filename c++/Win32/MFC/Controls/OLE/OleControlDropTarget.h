//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef OLE_CONTROL_DROP_TARGET_H
   #define OLE_CONTROL_DROP_TARGET_H


#include <afxole.h>			// For COleDropTarget

class OleControl;				// Our parent
									// WARNING: Need to avoid cyclical include.


class OleControlDropTarget : public COleDropTarget
{
protected:

   OleControl* m_pControl;

// Construction
public:

// Attributes
public:

// Operations
public:

	// We need to associate the OLE control so we can access it
	// during drag-drop operations.  This function is
   // called by the OleControl that owns us, during
	// its creation.
   void SetControl( OleControl* pControl )
	{	
		m_pControl = pControl;	
	}

// Overrides
	DROPEFFECT OnDragOver(
		CWnd* pWnd,
		COleDataObject* pDataObject,
		DWORD dwKeyState,
		CPoint point
	);

	BOOL OnDrop(
		CWnd* pWnd,
		COleDataObject* pDataObject,
		DROPEFFECT dropEffect,
		CPoint point
	);

};



#endif // OLE_CONTROL_DROP_TARGET_H
