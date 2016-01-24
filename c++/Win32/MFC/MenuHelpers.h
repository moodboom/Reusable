//-------------------------------------------------------------------//
//	Menu Helpers
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef MENU_HELPERS_H
	#define MENU_HELPERS_H


// This searches the given menu for the given string, returning
// the index if found, -1 if not found.
Long FindMenuItemPosition(CMenu *pMenu, LPCTSTR menuString);

// This searches the whole menu hierarchy to find the first 
// instance of a menu item that corresponds to the given command.  
// Since the search is hierarchical, we also return the child
// menu pointer that actually contains the menu item.
bool FindMenuPos(CMenu *pBaseMenu, UINT myID, CMenu * & pMenu, int & mpos);

// This enhanced version of InsertMenu() will ENABLE an item
// instead of inserting a duplicate if it exists already 
// anywhere in the menu.
void InsertMenu(CMenu* pMenu, UINT oldID, int Flags, UINT newID, const char * MenuText);

#endif	// MENU_HELPERS_H
