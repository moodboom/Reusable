//-------------------------------------------------------------------//
// MenuHelpers.cpp
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include "stdafx.h"

#include "MenuHelpers.h"


//-------------------------------------------------------------------//
// FindMenuItemPosition()															//
//-------------------------------------------------------------------//
// This searches the given menu for the given string, returning
// the index if found, -1 if not found.
//-------------------------------------------------------------------//
int FindMenuItemPosition(CMenu *pMenu, LPCTSTR menuString)
{
	int position = -1;

	try
	{
		CString		itemText;
		int			i, numItems;

		numItems = pMenu->GetMenuItemCount();

		if (numItems == -1)
			return -1;

		for (i = 0; i < numItems; i++)
		{
			pMenu->GetMenuString(i, itemText, MF_BYPOSITION);
			
			if (itemText.Compare(menuString) == 0)
			{
				position = i;
				break;
			}
		}
	}
	catch (CMemoryException *ex)
	{
		ex->Delete();
	}

	return position;
}


//-------------------------------------------------------------------//
// FindMenuPos()													//
//-------------------------------------------------------------------//
// This searches the whole menu hierarchy to find the first 
// instance of a menu item that corresponds to the given command.  
// Since the search is hierarchical, we also return the child
// menu pointer that actually contains the menu item.
//-------------------------------------------------------------------//
bool FindMenuPos(CMenu *pBaseMenu, UINT myID, CMenu * & pMenu, int & mpos)
{
	// REMARK: pMenu is a pointer to a Cmenu-Pointer
	int myPos;
	if( pBaseMenu == NULL )
	{
		// Sorry, Wrong Number
		pMenu = NULL;
		mpos = -1;
		return true;
	}
	for( myPos = pBaseMenu->GetMenuItemCount() -1; myPos >= 0; myPos-- )
	{
		int Status = pBaseMenu->GetMenuState( myPos, MF_BYPOSITION);
		CMenu* mNewMenu;
		
		if( Status == 0xFFFFFFFF )
		{
			// That was not an legal Menu/Position-Combination
			pMenu = NULL;
			mpos = -1;
			return true;
		}

		// --------------------------------------------
		// MDM	3/25/2003 11:02:43 AM
		// New code (from CodeGuru comment)
		int nMenuItem = pBaseMenu->GetMenuItemID(myPos);
		// Is this the real one?
		if( pBaseMenu->GetMenuItemID(myPos) == myID )
		{
			// Yep!
			pMenu = pBaseMenu;
			mpos = myPos;
			return true;
		}
		// Maybe a subMenu?
		else if ( nMenuItem == -1 ) //-1 means its a SubMenu
		{
			mNewMenu = pBaseMenu->GetSubMenu(myPos);
		// --------------------------------------------
		// MDM	3/25/2003 11:02:31 AM
		// Original code
		/*
		// Is this the real one?
		if( pBaseMenu->GetMenuItemID(myPos) == myID )
		{
			// Yep!
			pMenu = pBaseMenu;
			mpos = myPos;
			return true;
		}
		// Maybe a subMenu?
		mNewMenu = pBaseMenu->GetSubMenu(myPos);
		// This function will return NULL if ther is NO SubMenu
		if( mNewMenu != NULL )
		{
		*/
		// --------------------------------------------

			// rekursive!
			bool found = FindMenuPos( mNewMenu, myID, pMenu, mpos);
			if(found)
				return true;	// return this loop
		}
		// I have to check the next in my loop
	}
	return false; // iterate in the upper stackframe
}


//-------------------------------------------------------------------//
// InsertMenu()														//
//-------------------------------------------------------------------//
// This enhanced version of InsertMenu() will ENABLE an item
// instead of inserting a duplicate if it exists already 
// anywhere in the menu.
//-------------------------------------------------------------------//
void InsertMenu(CMenu* pMenu, UINT oldID, int Flags, UINT newID, const TCHAR * MenuText)
{
	if( pMenu == NULL )     
		return;

	if(pMenu->GetMenuState( newID, Flags) == 0xFFFFFFFF )
		pMenu->InsertMenu(oldID, Flags, newID, MenuText );
	else
		pMenu->EnableMenuItem( newID, MF_ENABLED );
}
