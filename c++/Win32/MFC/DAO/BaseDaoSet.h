/////////////////////////////////////////////////////////////////////////////
// BaseDAOSet class
//
// This class is derived from CDaoRecordset and extends it to provide
// additional base functionality.  Includes:
//
//		> ability to set/get parameters to/from the set's table
//
/////////////////////////////////////////////////////////////////////////////

// BaseDAOSet.h : header file
//
#if !defined(BASE_DAO_SET_H)
	#define BASE_DAO_SET_H

#include <afx.h>						// For CString
#include <afxdao.h>					// For CDaoRecordset (base class)


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class BaseDAOSet :	public CDaoRecordset
{
	typedef CDaoRecordset inherited;

// Construction
public:
   
	BaseDAOSet();

   ~BaseDAOSet();

	// Override to create a new table containing the fields from this set.
	virtual CDaoTableDef* CreateNewTable( 
		LPCTSTR	pszTableName
	) = 0;

protected:

	// This function uses SQL to add a param to a table, since
	// there are no MFC DAO functions to do so.  Once the param
	// is added, MFC DAO is set up to handle it.
	// See "Parameterizing the Filter in DaoEnrol" for a good
	// example of getting and setting your parameters with DDX.
	// Also, see CDaoRecordSet::Get/SetParamValue() for direct access.
	void AddParam( 
		CDaoTableDef*	pTable,
		COleVariant		vData 
	);

	// We must be able to create ourselves in new databases.
	virtual void Create() = 0;

};


#endif // !defined(BASE_DAO_SET_H)
