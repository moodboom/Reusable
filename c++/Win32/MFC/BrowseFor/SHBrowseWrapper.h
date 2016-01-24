//-------------------------------------------------------------------//
//	SHBrowseWrapper
//
// This class provides a wrapper around SHBrowseForFolder().  That
// base in turn is used in derived classes specialized for browsing
// folders, machines, etc.  See BrowseForHelpers.*, which contains
// functions that you can drop right into your code.
//
// Originally based on CodeGuru sample code:
// ShellBrowser.h: interface for the CShellBrowser class.
// Copyright 1998 Scott D. Killen
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef BROWSE_FOR_FOLDER_H
#define BROWSE_FOR_FOLDER_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <memory>
#include <shlobj.h>


class SHBrowseWrapper
{
public:
	SHBrowseWrapper(
		const CWnd*				pParent		= NULL, 
		const LPITEMIDLIST	pidl			= NULL, 
		const CString*			pstrTitle	= 0
	);
	virtual ~SHBrowseWrapper() = 0;

	void SetInitialSelection(const CString& strPath);

	//
	// Set the handle of the owner window for the dialog box.
	//
	void SetOwner(const HWND hwndOwner);

	//
	// Set the root of the heirarchy that will be browsed.  Get pidl from SHGetSpecialFolderLocation.
	// This can be set to NULL to use the Virtual Folder that represents the Windows Desktop.
	//
	void SetRoot(const LPITEMIDLIST pidl);

	// Access a string that is displayed above the tree view control in the dialog box. This
	// string can be used to specify instructions to the user.
	CString GetTitle() const;
	void SetTitle( const CString* pstrTitle );

	//
	// ulFlags = Value specifying the types of folders to be listed in the dialog box as well as
	//           other options. This member can include zero or more of the following values:
	//
	//          BIF_BROWSEFORCOMPUTER    Only returns computers. If the user selects anything
	//                                   other than a computer, the OK button is grayed.
	//
	//          BIF_BROWSEFORPRINTER     Only returns printers. If the user selects anything 
	//                                   other than a printer, the OK button is grayed.
	//
	//          BIF_DONTGOBELOWDOMAIN    Does not include network folders below the domain level
	//                                   in the tree view control.
	//
	//          BIF_RETURNFSANCESTORS    Only returns file system ancestors. If the user selects
	//                                   anything other than a file system ancestor, the OK
	//                                   button is grayed.
    //
    //          BIF_RETURNONLYFSDIRS     Only returns file system directories. If the user
	//                                   selects folders that are not part of the file system,
	//                                   the OK button is grayed.
	//
	//          BIF_STATUSTEXT           Includes a status area in the dialog box. The callback
	//                                   function can set the status text by sending messages to
	//                                   the dialog box. 
	//
	UINT GetFlags() const;
	void SetFlags(const UINT ulFlags);

	//
	// Call GetSelectedFolder to retrieve the folder selected by the user.
	//
	CString GetSelectedFolder() const;

	//
	// Function to retreive the image associated with the selected folder. The image is specified
	// as an index to the system image list. 
	//
	int GetImage() const;

	//
	// Call SelectFolder to display the dialog and get a selection from the user.  Use
	// GetSelectedFolder and GetImage to get the results of the dialog.
	//
	bool SelectFolder();

protected:
	//
	// OnInit is called before the dialog is displayed on the screen.
	//
	virtual void OnInit();

	//
	// OnSelChanged is called whenever the user selects a different directory.  pidl is the
	// LPITEMIDLIST of the new selection.  Use SHGetPathFromIDList to retrieve the path of the
	// selection.
	//
	virtual void OnSelChanged(const LPITEMIDLIST pidl) const;

	//
	// Call EnableOK to enable the OK button on the active dialog.  If bEnable is true then the
	// button is enabled, otherwise it is disabled.
	// NOTE -- This function should only be called within overrides of OnInit and OnSelChanged.
	//
	void EnableOK(const bool bEnable) const;

	//
	// Call SetSelection to set the selection in the active dialog.  pidl is the LPITEMIDLIST
	// of the path to be selected.  strPath is a CString containing the path to be selected.
	// NOTE -- This function should only be called within overrides of OnInit and OnSelChanged.
	//
	void SetSelection(const LPITEMIDLIST pidl) const;
	void SetSelection(const CString& strPath) const;

	//
	// Call SetStatusText to set the text in the Status area in the active dialog.  strText is
	// the text to be displayed.
	// NOTE -- This function should only be called within overrides of OnInit and OnSelChanged.
	//
	void SetStatusText(const CString& strText) const;

	CString m_strInitialSelection;

private:
	static int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

	typedef std::auto_ptr<char> AUTO_STR;
	AUTO_STR m_pchTitle;

	BROWSEINFO m_bi;
	char m_szSelected[MAX_PATH];
	CString m_strPath;
	HWND m_hwnd;
};

inline UINT SHBrowseWrapper::GetFlags() const
{
	return m_bi.ulFlags;
}

inline int SHBrowseWrapper::GetImage() const
{
	return m_bi.iImage;
}

#endif // BROWSE_FOR_FOLDER_H
