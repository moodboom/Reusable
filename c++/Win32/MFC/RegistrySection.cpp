//-------------------------------------------------------------------//
// RegistrySection
//
// This class maintains a section in the registry.  A section is a group
// of strings under a common key.
//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//


#include "stdafx.h"

#include "RegistrySection.h"

#include "DisplayMessage.h"					// For debugging msgs.


//-------------------------------------------------------------------//
// RegistrySection()																		//
//-------------------------------------------------------------------//
// The constructor loads an entire section from the Registry.  The       
// constructor looks for the specified section using the following
// registry structure:                                                 
//                                                                     
// 		HKEY_CURRENT_USER\                                            
// 			Software\                                                  
// 				<company name>\                                         
// 					<application name>\                                  
// 						<section name>\                                   
// 							<value name>		<- These are all loaded      
//                                                                     
// This function assumes that the App has previously called			     
// SetRegistryKey() to establish the company and app sections.  It     
// uses the undocumented GetSectionKey() function, which looks up our  
// specified section for us.                                           
//
// This class has the ability to use default label names.  If the
// caller specifies bNewUseDefaultLabels as true, then labels
// are assigned automatically when the values are saved.  The
// labels are non-overlapping numeric values, starting with 1.
//
// If bNewUseDefaultLabels is false, then the caller is responsible
// for labels.  One should be supplied every time a value is
// added.
//
// Note the LPCTSTR param.  There was no need to use the more powerful 
// CString class, as the section will typically be identified by a     
// constant static string.                                             
//-------------------------------------------------------------------//
RegistrySection::RegistrySection(
	LPCTSTR	pszNewSectionName,
	bool		bNewUseDefaultLabels
) :

	// Init vars.
	pszSectionName		( pszNewSectionName		),
	bUseDefaultLabels	( bNewUseDefaultLabels	),
	bChanged				( false						)

{

	// Note that we used to get the section in the constructor.  But
	// if this is a static object, you will hit the constructor before
	// you can initialize the app's default section, and this will fail.
	// GetSection();

}


//-------------------------------------------------------------------//
// ~RegistrySection()																//
//-------------------------------------------------------------------//
RegistrySection::~RegistrySection()
{

	// Note that we used to save the section in the destructor.  But
	// if this is a static object, you will hit the destructor after
	// the app's resources have been closed, and this will fail.
	// if ( bChanged )
	// 	Write();
	
	// Clean up.
	ClearLabels();
	ClearValues();

}


//-------------------------------------------------------------------//
// Read()																				//
//-------------------------------------------------------------------//
// This gets the entire section.  You need to do this first.  :>
//                                                                     
// This function is based on the sample code provided in MSDN's        
// "Using the Registry".  It doesn't make too much sense.  There       
// are separate loops for the labels and the values, and they use      
// two different loop methodologies.  
// To quote Crusty: "Don't blame me, I didn't do it!"
//-------------------------------------------------------------------//
void RegistrySection::Read()
{
	// Clear any previous contents.
	ClearAll();

	CHAR     achClass[MAX_PATH] = "";  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // length of class string 
	DWORD    cSubKeys;                 // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;					     // number of values for key 
	DWORD    cchMaxValue;				  // longest value name 
	DWORD    cbMaxValueData;			  // longest value data 
	DWORD    cbSecurityDescriptor;	  // size of security descriptor 
	FILETIME ftLastWriteTime;			  // last write time      
	DWORD		j; 
	DWORD		retValue;      
	const DWORD MAX_VALUE_NAME = 200;
	const DWORD MAX_VALUE_DATA_NAME = 500;
	CHAR		achValue[MAX_VALUE_NAME]; 
	DWORD		cchValue = MAX_VALUE_NAME;     
	BYTE		abData[MAX_VALUE_DATA_NAME]; 
	DWORD		cbData = MAX_VALUE_DATA_NAME;     

	// Get the section key.
	HKEY hSectionKey = AfxGetApp()->GetSectionKey( pszSectionName );

	// Get the class name and the value count. 
	RegQueryInfoKey(
		hSectionKey,				 // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // length of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime	       // last write time  
	);
	
	// Get the actual "key" "value" "data". 
	if ( cValues )
	{
		for ( j = 0, retValue = ERROR_SUCCESS; j < cValues; j++ ) 
		{
			// MAJOR PROBLEMS if you do not reset these each time!
			// They are updated during each call to RegEnumValue().
			cchValue = MAX_VALUE_NAME;     
			cbData = MAX_VALUE_DATA_NAME;

			achValue[0] = '\0'; 
			retValue = RegEnumValue(
				hSectionKey, 
				j, 
				achValue, 
				&cchValue,                 
				NULL, 
				NULL,			// &dwType,                 
				abData,		// &bData, 
				&cbData		// &bcData  
			);

			if ( retValue == (DWORD) ERROR_SUCCESS )
			{ 				
				// We found a new value.  Add to array.
				Values.Add( new CString( abData ) );

				// TO DO
				// This has not been tested yet.
				if ( !bUseDefaultLabels )
					Labels.Add( new CString( achValue ) );

			#ifdef _DEBUG
			} else
			{
				// DEBUG
				CString strMsg;
				strMsg.Format( 
					_T("Error enumerating registry section!\n%s\nError %d\n(Common results: %d %d %d)"), 
					CString( abData ), 
					retValue, 
					ERROR_SUCCESS, 
					ERROR_INSUFFICIENT_BUFFER,
					ERROR_MORE_DATA
				);
				DisplayMessage( strMsg );

			#endif

			}
		}
	} 

	// Close the key.
	VERIFY( RegCloseKey( hSectionKey ) == ERROR_SUCCESS );

	bChanged = false;


}


//-------------------------------------------------------------------//
// Write()																		//
//-------------------------------------------------------------------//
// This writes the section to the registry, as it is currently 
// represented by the Labels and Values arrays.  It is called
// in the destructor.
//-------------------------------------------------------------------//
void RegistrySection::Write() 
{

	int i, nCount;
	
	// Make sure we either supply the labels or the caller did.
	ASSERT(
			bUseDefaultLabels
		||	( Labels.GetSize() == Values.GetSize() )
	);

	// Clear any labels if we are creating defaults.
	if ( bUseDefaultLabels )
		ClearLabels();
	
	// Clear out the previous section contents by
	// wiping out the entire section.  It will be
	// automatically recreated when we add values
	// to it, next.
	HKEY hAppKey = AfxGetApp()->GetAppRegistryKey();
	RegDeleteKey(
		hAppKey,
		pszSectionName
	);
	VERIFY( RegCloseKey( hAppKey ) == ERROR_SUCCESS );
	
	// Add a new key for all of our current values.
	nCount = Values.GetSize();
	for ( i = 0; i < nCount; i++ ) {

		if ( bUseDefaultLabels ) {

			// Make our default label.
			// All labels are given a numeric representation.
			CString* pNewLabel = new CString;
			pNewLabel->Format( "%d", i );

			Labels.Add( pNewLabel );

		}

		// Add it.
		VERIFY( 
			AfxGetApp()->WriteProfileString(
				pszSectionName, 
				LPCTSTR( *( Labels[i] ) ),
				LPCTSTR( *( Values[i] ) )
			) 
		);

	}

	bChanged = false;

}


//-------------------------------------------------------------------//
// ClearAll()																			//
//-------------------------------------------------------------------//
void RegistrySection::ClearAll()
{
	ClearLabels();
	ClearValues();
}


//-------------------------------------------------------------------//
// ClearLabels()																		//
//-------------------------------------------------------------------//
// Removes allocations and clears the list.
//-------------------------------------------------------------------//
void RegistrySection::ClearLabels()
{

	// Clean up.
	for ( int i = Labels.GetUpperBound(); i >= 0; i-- )
		delete Labels[i];
	Labels.RemoveAll();

	bChanged = true;

}


//-------------------------------------------------------------------//
// ClearValues()																		//
//-------------------------------------------------------------------//
// Removes allocations and clears the list.
//-------------------------------------------------------------------//
void RegistrySection::ClearValues()
{

	// Clean up.
	for ( int i = Values.GetUpperBound(); i >= 0; i-- )
		delete Values[i];
	Values.RemoveAll();

	bChanged = true;

}


//-------------------------------------------------------------------//
// Add()																					//
//-------------------------------------------------------------------//
// This adds strings to the section.  Note that the strings
// pointed to by the parameters are not modified in any
// way.  A new CString is created as a copy of the passed strings.
//-------------------------------------------------------------------//
int RegistrySection::Add(
	CString*	pNewValue,
	CString* pNewLabel
) {

	if ( pNewValue )
		Values.Add( new CString( *pNewValue ) );
	else
		Values.Add( new CString );

	if ( pNewLabel )
		Labels.Add( new CString( *pNewLabel ) );

	bChanged = true;

	return Values.GetUpperBound();
}


//-------------------------------------------------------------------//
// Remove()																				//
//-------------------------------------------------------------------//
// This removes strings from the section.  Note that the strings
// pointed to by the parameters are not modified in any way.
// If you are using default labels, you should not pass the
// pOldLabel param.  If you are not using default labels, you should
// pass pOldLabel; pOldValue will only be used as an additional check
// if you pass it.
//
// TO DO
// If this becomes popular, it should be rewritten into two functions,
// one for default labels and one for no default labels.
//-------------------------------------------------------------------//
bool RegistrySection::Remove(
	CString*	pOldValue,
	CString* pOldLabel
) {
	int i;
	bool bFound = false;

	// If the label was specified, it governs, and we remove 
	// it and its corresponding value.
	if ( pOldLabel ) 
	{
		i = Labels.GetUpperBound();
		while ( i >= 0 && bFound ) {
			if ( !( bFound = ( *( Labels[i] ) == *pOldLabel ) ) )
				i--;
		}

		if ( bFound ) 
		{			
			// If the value was specified, make sure it
			// matches.
			if( !pOldValue || ( *( Values[i] ) == *pOldValue ) ) {
			
				// Remove.
				delete Labels[i];
				Labels.RemoveAt( i );
				delete Values[i];
				Values.RemoveAt( i );

			} else 
			{			
				// Bad params.  Don't remove, and return false.
				bFound = false;
			
			}

		}
	
	// If the label was not specified, use the value.
	} else 
	{
		int nValuesCount = Values.GetSize();
		for ( i = 0; i < nValuesCount; i++ )
			if ( *( Values[i] ) == *pOldValue )
				break;

		// If we found it...
		if ( bFound = ( i < nValuesCount ) ) {
			
			// Remove it.
			delete Values[i];
			Values.RemoveAt( i );

		}

	}

	bChanged |= bFound;

	return bFound;

}


//-------------------------------------------------------------------//
// Remove()																				//
//-------------------------------------------------------------------//
bool RegistrySection::Remove(
	int	nIndex
) {

	bool bValidIndex = ( nIndex < Values.GetSize() );

	if ( bValidIndex ) {

		delete Values[nIndex];
		Values.RemoveAt( nIndex );
		if ( !bUseDefaultLabels ) {
			delete Labels[nIndex];
			Labels.RemoveAt( nIndex );
		}
	
	}

	bChanged |= bValidIndex;

	return bValidIndex;

}


//-------------------------------------------------------------------//
// nFindLabel()																		//
//-------------------------------------------------------------------//
// Finds the passed label in the section, returns the CArray index.
// Returns -1 if none found.
//-------------------------------------------------------------------//
int RegistrySection::nFindLabel(
	CString &strSearch
) {

	// Search for the passed string.
	int nA;
	for ( nA = 0; nA < Labels.GetSize(); nA++ )
		if ( *Labels[ nA ] == strSearch )
			break;

	// Return loop counter if valid.
	if ( nA < Labels.GetSize() )
		return nA;

	// Else return the "not found" value.
	return -1;
}


// Here's an example of placing a structure in each of the strings.
// Prolly better off setting up some kind of binary section class?
/*

//-------------------------------------------------------------------//
// PutWindowPlacement()																//
//-------------------------------------------------------------------//
// Used to put and get WINDOWPLACEMENT structs to and from the
// registry.
//
//	typedef struct tagWINDOWPLACEMENT {
//		 UINT  length;
//		 UINT  flags;
//		 UINT  showCmd;
//		 POINT ptMinPosition;
//		 POINT ptMaxPosition;
//		 RECT  rcNormalPosition;
//	} WINDOWPLACEMENT;
//
//-------------------------------------------------------------------//
void RegistrySection::PutWindowPlacement( int nIdx, WINDOWPLACEMENT &WndPl )
{
	Values[ nIdx ]->Format(
		_T("%10u%10u%10u%10d%10d%10d%10d%10d%10d%10d%10d"),
		WndPl.length,
		WndPl.flags,
		WndPl.showCmd,
		WndPl.ptMinPosition.x,
		WndPl.ptMinPosition.y,
		WndPl.ptMaxPosition.x,
		WndPl.ptMaxPosition.y,
		WndPl.rcNormalPosition.left,
		WndPl.rcNormalPosition.top,
		WndPl.rcNormalPosition.right,
		WndPl.rcNormalPosition.bottom
	);

	SetChanged();
}

//-------------------------------------------------------------------//
// GetWindowPlacement()																//
//-------------------------------------------------------------------//
// See above comment for SetWindowPlacement()
//-------------------------------------------------------------------//
bool RegistrySection::GetWindowPlacement( int nIdx, WINDOWPLACEMENT &WndPl )
{
	// Versioning from replacement of stupid nGetIntRect function.
	if ( Values[ nIdx ]->GetLength() < 80 )
		return false;

	WndPl.length	=	_tcstoul( *Values[ nIdx ], NULL, 10 );
	WndPl.flags		=	_tcstoul( Values[ nIdx ]->Mid( 10, 10 ), NULL, 10 );
	WndPl.showCmd	=	_tcstoul( Values[ nIdx ]->Mid( 20, 10 ), NULL, 10 );

	WndPl.ptMinPosition.x	=	_tcstol( Values[ nIdx ]->Mid( 30, 10 ), NULL, 10 );
	WndPl.ptMinPosition.y	=	_tcstol( Values[ nIdx ]->Mid( 40, 10 ), NULL, 10 );
	WndPl.ptMaxPosition.x	=	_tcstol( Values[ nIdx ]->Mid( 50, 10 ), NULL, 10 );
	WndPl.ptMaxPosition.y	=	_tcstol( Values[ nIdx ]->Mid( 60, 10 ), NULL, 10 );

	WndPl.rcNormalPosition.left	=	_tcstol( Values[ nIdx ]->Mid( 70, 10 ), NULL, 10 );
	WndPl.rcNormalPosition.top		=	_tcstol( Values[ nIdx ]->Mid( 80, 10 ), NULL, 10 );
	WndPl.rcNormalPosition.right	=	_tcstol( Values[ nIdx ]->Mid( 90, 10 ), NULL, 10 );
	WndPl.rcNormalPosition.bottom	=	_tcstol( Values[ nIdx ]->Mid( 100, 10 ), NULL, 10 );

	return true;
}

*/