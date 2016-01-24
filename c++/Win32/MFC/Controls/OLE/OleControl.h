//-------------------------------------------------------------------//
// OleControl.h : header file
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//
#ifndef OLE_CONTROL_H
   #define OLE_CONTROL_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "OleControlDropTarget.h"		// Contains one, captures d/d function calls


//-------------------------------------------------------------------//
// class OleControl																//
//-------------------------------------------------------------------//
// OleControl window
//-------------------------------------------------------------------//
class OleControl
{

// Construction
public:

   OleControl() {}

   virtual ~OleControl() {}

// Attributes
public:

// Operations
public:

// Implementation
public:

	////////////////////////////////////////////////////////
	// Drag/drop functionality.
	//
	// This function processes a drag over request.
	// Override in derived class based on control type.
	virtual DROPEFFECT OnDragOver(
		CWnd* pWnd,
		COleDataObject* pDataObject,
		DWORD dwKeyState,
		CPoint point
	) = 0;

	// This function processes a drop request.
	// Override in derived class based on control type.
	virtual BOOL OnDrop(
		CWnd* pWnd,
		COleDataObject* pDataObject,
		DROPEFFECT dropEffect,
		CPoint point
	) = 0;

protected:

	//-------------------------------------------------------------------//
	// Register()																			//
	//-------------------------------------------------------------------//
	// This function registers the drop target and ties it to
	// us.  Call this in derived classes at the appropriate time,
	// e.g., in PreSubclassWindow() override.
	//-------------------------------------------------------------------//
	void Register( CWnd* pWnd )
	{

		// Register drop target.
		DropTarget.Register( pWnd );

		// Let the drop target object know about us.
		DropTarget.SetControl( this );

	}

	// This adds the functionality, and passes notifications to us.
	OleControlDropTarget		DropTarget;

	// This specifies our drag/copy clipboard format.
	// Typically, you set this in your derived class's constructor.
	CLIPFORMAT					ClipFormat;

};

#endif	// OLE_CONTROL_H
