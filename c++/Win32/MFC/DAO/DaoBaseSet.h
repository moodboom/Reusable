#if !defined(SONG_SET_H)
#define SONG_SET_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DaoBaseSet.h : header file
//

// #include "stdafx.h"
// OR...

#include <afxtempl.h>								// For CTypedPtrArray

#include <afxdao.h>									// For CDaoRecordset.




/////////////////////////////////////////////////////////////////////////////
// DaoBaseSet DAO recordset

class DaoBaseSet : public CDaoRecordset
{
	typedef CDaoRecordset inherited;

public:

	DaoBaseSet(CDaoDatabase* pDatabase = NULL);
	DECLARE_DYNAMIC(DaoBaseSet)

	~DaoBaseSet();

	// Override to create a new table containing the fields from this set.
	virtual CDaoTableDef* CreateNewTable( 
		LPCTSTR	pszTableName
	) = 0;

// Field/Param Data
	//{{AFX_FIELD(DaoBaseSet, CDaoRecordset)
	//}}AFX_FIELD

	// These are the actual attributes that will get updated as
	// we rip through records.
	BYTE*	pDynamicByteFields;

	// These are the dynamic field labels, read in on Open().
	CTypedPtrArray <CPtrArray, CString*> astrDynamicByteFields;
	
	// This count is kept around since we use it so much.
	// It is calced as the total number of fields - m_nFields.
	int m_nDynamicByteFieldCount;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DaoBaseSet)
	public:
	virtual CString GetDefaultDBName();		// Default database name
	virtual CString GetDefaultSQL();		// Default SQL for Recordset
	virtual void DoFieldExchange(CDaoFieldExchange* pFX);  // RFX support
	//}}AFX_VIRTUAL

// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	//////////////////////////
	// Dynamic field operations

	void AddByteField( 
		BYTE		ubDefaultValue,
		CString& strLowerName,
		CString& strUpperName		= CString()
	);

	void DeleteField(
		int nOffset
	);

	void RenameField(
		int		nOffset,
		CString&	strLowerName,
		CString& strUpperName		= CString()
	);

	//
	//////////////////////////

	
	void CleanUp();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(SONG_SET_H)
