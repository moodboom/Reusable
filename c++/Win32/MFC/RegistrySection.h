//-------------------------------------------------------------------//
// RegistrySection
//
// This class maintains a section in the registry.  A section is a group
// of strings under a common key.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//


#ifndef REGISTRY_SECTION_H
	#define REGISTRY_SECTION_H

#include <afxtempl.h>				// For CTypedPtrArray

class RegistrySection {

public:

	RegistrySection(
		LPCTSTR	pszNewSectionName,
		bool		bNewUseDefaultLabels = true
	);

	~RegistrySection();

	void Read();
	void Write();

	CTypedPtrArray <CPtrArray, CString*> Labels;
	CTypedPtrArray <CPtrArray, CString*> Values;

	int Add(
		CString*	pNewValue,
		CString* pNewLabel = NULL
	);

	bool Remove(
		CString*	pOldValue,
		CString* pOldLabel = NULL
	);

	bool Remove(
		int nIndex
	);

	// Finds the passed label in the section, returns the CArray index.
	int nFindLabel(
		CString &strSearch
	);

	// Sets the "changed" flag manually.
	// The section will be saved in destructor if true.
	// We want to be able to manually turn off the save, too, in case the
	// user wants to cancel pending changes.
	void SetChanged( bool bNewChanged = true )
	{
		bChanged = bNewChanged;
	}

	void ClearAll();

protected:

	bool bChanged;

	void ClearLabels();
	void ClearValues();

	LPCTSTR	pszSectionName;
	bool		bUseDefaultLabels;

};

#endif	// REGISTRY_SECTION_H
