// DaoBaseSet.cpp : implementation file
//

#include "stdafx.h"
#include "HangTheDJ.h"
#include "DaoBaseSet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DaoBaseSet

IMPLEMENT_DYNAMIC(DaoBaseSet, CDaoRecordset)

//-------------------------------------------------------------------//
// DaoBaseSet()																			//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
DaoBaseSet::DaoBaseSet(CDaoDatabase* pdb)
:

	// Call base class.
	CDaoRecordset(pdb),

	// Init vars.
	pAttributes( 0 ),
	nAttributesCount( 0 )

{
	// FIELD DESCRIPTIONS
	/*										Description
	long		m_ID;						Song ID, used as database index
	CString	m_SongName;				Song name			(initialized from tag)
	CString	m_Artist;				Artist					"
	CString	m_Album;					Album						"
	CString	m_Year;					Year						"
	CString	m_Comment;				Comment, any length	"
	BYTE		m_Genre;					Genre						"
	BYTE		m_Rating;				Parental rating, same as for the movies
	CString	m_Notes;					Comments, any length
	CString	m_Filename;				Name of file (no path)
	CString	m_Path;					Path to file (may change, e.g. on insert into CD #2)
	BYTE		m_Found;					Specifies the number of scans since this file was found
	BYTE		m_Ignore;				Specifies the number of times to skip playing this song
	BYTE		m_PlaysSkipped;		Specifies the number of times this song has currently been skipped
	CString	m_Source;				
	long		m_TimeInSecs;			Song length in seconds, converted for display to h:min:sec
	BYTE		m_Frequency;			Attribute for overall score
	BYTE		m_Age;					Attribute for how long we have known about this song
	*/

	//{{AFX_FIELD_INIT(DaoBaseSet)
	m_SongName = _T("");
	m_Artist = _T("");
	m_Album = _T("");
	m_Year = _T("");
	m_Comment = _T("");
	m_Rating = 0;
	m_Frequency = 0;
	m_FreqDecay = 0;
	m_FreqDecayPlays = 0;
	m_Notes = _T("");
	m_Filename = _T("");
	m_Path = _T("");
	m_ID = 0;
	m_Found = 1;
	m_PlaysSkipped = 0;
	m_Source = _T("");
	m_TimeInSecs = 0;
	m_Genre = 0;
	m_nFields = 18;
	//}}AFX_FIELD_INIT
	m_nDefaultType = dbOpenTable;

}


//-------------------------------------------------------------------//
// ~DaoBaseSet()																			//
//-------------------------------------------------------------------//
// Clean up.
//-------------------------------------------------------------------//
DaoBaseSet::~DaoBaseSet()
{
	// Note that on a call to Update(), the class gets destroyed!
	// This is bizarre.
	// Anyway, we need to clean up our data that was allocated in
	// Open().  Will it ever get reallocated???
	CleanupAttributeData();

}


CString DaoBaseSet::GetDefaultDBName()
{
	return _T("Database.m3j");
}

CString DaoBaseSet::GetDefaultSQL()
{
	// return _T("[Songs]");
	// return _T("PARAMETERS [AttributesCount] LONG; SELECT * FROM Songs;" );
	return _T("PARAMETERS [SongTableVersion] LONG; SELECT * FROM Songs;" );

}


//-------------------------------------------------------------------//
// Open()																				//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool DaoBaseSet::Open( CDaoTableDef* pSongTable )
{
	int i;

	// The attribute count is determined from the total number
	// of fields minus the hardcoded DAO fields.
	nAttributesCount = pSongTable->GetFieldCount() - m_nFields;

	// DEBUG
	// Peek at the fields.
	/*
	for ( i = 0; i < m_nFields + nAttributesCount; i++ )
	{
		// Get the field info.
		CDaoFieldInfo Info;
		pSongTable->GetFieldInfo( i, Info );

	}
	*/

	// Apparently, multiple calls to Open may be made by the base
	// classes without calls to Close.  Be careful to clean up first.
	CleanupAttributeData();

	// Set up the attributes array and labels.
	pAttributes = new BYTE[ nAttributesCount ];
	for ( i = 0; i < nAttributesCount; i++ )
	{
		// Get the field info.
		CDaoFieldInfo Info;
		pSongTable->GetFieldInfo( m_nFields + i, Info );

		// Stuff the attribute labels.
		ASSERT( Info.m_nType == dbByte );
		astrAttributes.Add( new CString( Info.m_strName ) );

	}

	// Call the CDaoRecordset Open() function.
	// We overrode DoFieldExchange() to use our attributes.
	CDaoRecordset::Open( pSongTable, dbOpenTable );

	// Set the default index.
	SetCurrentIndex( _T("ID") );

	// TO DO
	return true;

}


//-------------------------------------------------------------------//
// Close()																				//
//-------------------------------------------------------------------//
void DaoBaseSet::Close()
{
	// Call the base class.
	CDaoRecordset::Close();

	CleanupAttributeData();

}


//-------------------------------------------------------------------//
// CleanupAttributeData()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DaoBaseSet::CleanupAttributeData()
{

	// Clean up the dynamic attribute data.
	if ( pAttributes )
	{
		delete pAttributes;
		pAttributes = 0;
	}
	nAttributesCount = 0;

	for ( int i = astrAttributes.GetUpperBound(); i >= 0; i-- )
		delete astrAttributes[i];
	astrAttributes.RemoveAll();

}


//-------------------------------------------------------------------//
// DoFieldExchange()																	//
//-------------------------------------------------------------------//
// This version exchanges all DYNAMIC fields.
// Derived classes should let ClassWizard code up this function
// for static fields, and then call the base class version.
//-------------------------------------------------------------------//
void DaoBaseSet::DoFieldExchange(CDaoFieldExchange* pFX)
{

	// TO DO
	// Parameters?  No, I'm not sure that they persist.
	// pFX->SetFieldType(CDaoFieldExchange::param);
	// DFX_Text(pFX, "Name", m_strNameParam);

	//{{AFX_FIELD_MAP(DaoBaseSet)
	pFX->SetFieldType(CDaoFieldExchange::outputColumn);
	//}}AFX_FIELD_MAP

	// DFX the attribute fields manually.
	for ( int i = 0; i < nAttributesCount; i++ )
		DFX_Byte( pFX, LPCTSTR( *( astrAttributes[i] ) ), *( pAttributes + i ) );

}


//-------------------------------------------------------------------//
// GetNewTable()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
CDaoTableDef* DaoBaseSet::CreateNewTable(
	LPCTSTR		pszTableName
) {

	// Delete it first.  Catch exceptions (one is thrown if it doesn't exist).
	try {
		m_pDatabase->DeleteTableDef( pszTableName );
	}
	catch( CDaoException* e )
	{
		e->Delete( );
	}

	// Construct and create the new table object.
	CDaoTableDef* pNewTable = new CDaoTableDef( m_pDatabase );
	bool bNewTable = true;
	try {
		pNewTable->Create( pszTableName );
	}
	catch( CDaoException* e )
	{
		e->Delete( );
		bNewTable = false;
	}

	if ( bNewTable ) {

		// Create the fields.
		// First, set up a CDaoFieldInfo structure that we can use for all fields.
		CDaoFieldInfo Field;
		Field.m_nOrdinalPosition	= 0;
		Field.m_lCollatingOrder		= 0;
		Field.m_strForeignName		= _T("");
		Field.m_strSourceField		= _T("");
		Field.m_strSourceTable		= _T("");
		Field.m_strValidationRule	= _T("");
		Field.m_strValidationText	= _T("");
		Field.m_strDefaultValue		= _T("");

		// The ID field MUST not allow zero length, or CreateField() will fail.
		// It is also the primary index, so it is required.
		Field.m_bAllowZeroLength	= FALSE;
		Field.m_lAttributes			= dbAutoIncrField;
		Field.m_bRequired				= TRUE;
		Field.m_strName = _T("ID"					);	Field.m_nType = dbLong		; Field.m_lSize =  0; pNewTable->CreateField( Field );

		// The remaining fields have these in common.
		Field.m_lAttributes			= dbUpdatableField;
		Field.m_bRequired				= FALSE;

		// Make the text fields allow zero length.  This is the easiest way to handle
		// the situation when one of them is blank.
		Field.m_bAllowZeroLength	= TRUE;
		Field.m_strName = _T("SongName"			);	Field.m_nType = dbText		; Field.m_lSize =	30; pNewTable->CreateField( Field );
		Field.m_strName = _T("Artist"				);	Field.m_nType = dbText		; Field.m_lSize =	30; pNewTable->CreateField( Field );
		Field.m_strName = _T("Album"				);	Field.m_nType = dbText		; Field.m_lSize =	30; pNewTable->CreateField( Field );
		Field.m_strName = _T("Year"				);	Field.m_nType = dbText		; Field.m_lSize =	 4; pNewTable->CreateField( Field );
		Field.m_strName = _T("Comment"			);	Field.m_nType = dbText		; Field.m_lSize = 30; pNewTable->CreateField( Field );
		Field.m_strName = _T("Notes"				);	Field.m_nType = dbMemo		; Field.m_lSize =  0; pNewTable->CreateField( Field );

		// The remaining fields MUST not allow zero length, or CreateField() will fail.
		// The one text field is an indexed field that cannot be blank anyway.
		Field.m_bAllowZeroLength	= FALSE;

		Field.m_strName = _T("Genre"				);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("TimeInSecs"		);	Field.m_nType = dbLong		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Rating"				);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Frequency"			);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("FreqDecay"			);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("FreqDecayPlays"	);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("PlaysSkipped"		);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Found"				);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Source"				);	Field.m_nType = dbText		; Field.m_lSize = 30; pNewTable->CreateField( Field );
		Field.m_strName = _T("Filename"			);	Field.m_nType = dbText		; Field.m_lSize =255; pNewTable->CreateField( Field );
		Field.m_strName = _T("Path"				);	Field.m_nType = dbMemo		; Field.m_lSize =  0; pNewTable->CreateField( Field );

		// In milliseconds from the beginning/end.
		Field.m_strName = _T("StompInTime"		);	Field.m_nType = dbLong		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("StompOutTime"		);	Field.m_nType = dbLong		; Field.m_lSize =  0; pNewTable->CreateField( Field );

		// Default attributes.
		/*
		Field.m_strName = _T("Good Karma"		);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Sarcastic"			);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Eclectic"			);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		Field.m_strName = _T("Tempo"				);	Field.m_nType = dbByte		; Field.m_lSize =  0; pNewTable->CreateField( Field );
		*/

		// ID index.
		CDaoIndexInfo IDIndex;
		CDaoIndexFieldInfo IDIndexInfo;
		IDIndexInfo.m_strName = _T("ID");
		IDIndexInfo.m_bDescending = FALSE;
		IDIndex.m_strName = _T("ID");
		IDIndex.m_nFields = 1;
		IDIndex.m_pFieldInfos = &IDIndexInfo;
		IDIndex.m_bPrimary = TRUE;
		IDIndex.m_bUnique = TRUE;
		IDIndex.m_bRequired = TRUE;
		IDIndex.m_bIgnoreNulls = TRUE;
		pNewTable->CreateIndex( IDIndex );

		// Filename index.
		CDaoIndexInfo FilenameIndex;
		CDaoIndexFieldInfo FilenameIndexInfo;
		FilenameIndexInfo.m_strName = _T("Filename");
		FilenameIndexInfo.m_bDescending = FALSE;
		FilenameIndex.m_strName = _T("Filename");
		FilenameIndex.m_nFields = 1;
		FilenameIndex.m_pFieldInfos = &FilenameIndexInfo;
		FilenameIndex.m_bPrimary = FALSE;
		FilenameIndex.m_bUnique = FALSE;
		FilenameIndex.m_bRequired = TRUE;
		FilenameIndex.m_bIgnoreNulls = TRUE;
		pNewTable->CreateIndex( FilenameIndex );

		// Source index.
		CDaoIndexInfo SourceIndex;
		CDaoIndexFieldInfo SourceIndexInfo;
		SourceIndexInfo.m_strName = _T("Source");
		SourceIndexInfo.m_bDescending = FALSE;
		SourceIndex.m_strName = _T("Source");
		SourceIndex.m_nFields = 1;
		SourceIndex.m_pFieldInfos = &SourceIndexInfo;
		SourceIndex.m_bPrimary = FALSE;
		SourceIndex.m_bUnique = FALSE;
		SourceIndex.m_bRequired = TRUE;
		pNewTable->CreateIndex( SourceIndex );

		pNewTable->Append();

	} else {

		// We failed attempt to create the table.
		ASSERT( false );
		delete pNewTable;
		pNewTable = 0;

	}

	return pNewTable;

}


//-------------------------------------------------------------------//
// Copy()																				//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void DaoBaseSet::Copy(
	DaoBaseSet* pSource
) {
	m_SongName			= pSource->m_SongName;
	m_Artist				= pSource->m_Artist;
	m_Album				= pSource->m_Album;
	m_Year				= pSource->m_Year;
	m_Comment			= pSource->m_Comment;
	m_Notes				= pSource->m_Notes;
	m_Genre				= pSource->m_Genre;
	m_TimeInSecs		= pSource->m_TimeInSecs;
	m_Rating				= pSource->m_Rating;
	m_Frequency			= pSource->m_Frequency;
	m_FreqDecay			= pSource->m_FreqDecay;
	m_FreqDecayPlays	= pSource->m_FreqDecayPlays;
	m_PlaysSkipped		= pSource->m_PlaysSkipped;
	m_Found				= pSource->m_Found;
	m_Source				= pSource->m_Source;
	m_Filename			= pSource->m_Filename;
	m_Path				= pSource->m_Path;

	// Copy attributes.
	for ( int i = 0; i < nAttributesCount; i++ )
		*astrAttributes[i] = *pSource->astrAttributes[i];
	memcpy( pAttributes, pSource->pAttributes, sizeof BYTE * nAttributesCount );

}


/////////////////////////////////////////////////////////////////////////////
// DaoBaseSet diagnostics

#ifdef _DEBUG
void DaoBaseSet::AssertValid() const
{
	CDaoRecordset::AssertValid();
}

void DaoBaseSet::Dump(CDumpContext& dc) const
{
	CDaoRecordset::Dump(dc);
}
#endif //_DEBUG


//-------------------------------------------------------------------//
// AddAttribute()																		//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DaoBaseSet::AddAttribute( 
	BYTE		ubDefaultValue,
	CString& strLowerName,
	CString& strUpperName
) {
	// Add a table column for the new attribute.

	// Stuff the fields with the default value.

	// Add the attribute to our array.
	
	// Add an attribute control.

	// Increment the attributes count.
	nAttributesCount++;

}

//-------------------------------------------------------------------//
// DeleteAttribute()																	//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DaoBaseSet::DeleteAttribute(
	int nOffset
) {
	// Confirm deletion with a killable warning.
	
		// Delete the attribute control.

		// Delete the attribute from our array.

		// Delete the attribute column.
}

//-------------------------------------------------------------------//
// RenameAttribute()																	//
//-------------------------------------------------------------------//
// 
//-------------------------------------------------------------------//
void DaoBaseSet::RenameAttribute(
	int		nOffset,
	CString&	strLowerName,
	CString& strUpperName
) {
	// Rename the control.

	// Rename the table column.

	// Update our array.

}






CdbDBEngine      dbeng;
CdbWorkspace    wrkDefault;
CdbDatabase   db;
CdbProperty   prpUserDefined, prpEnum;
COleVariant   var("This is a user-defined property.", VT_BSTRT),
         varBstr;
int         i;

// Open the database.
db = dbeng.OpenDatabase(_T("Northwind.mdb"));

// Create user-defined property.
prpUserDefined = db.CreateProperty();
   
// Set properties of new property.
prpUserDefined.SetName("UserDefinedProperty");
prpUserDefined.SetType(dbText);
prpUserDefined.SetValue((LPVARIANT)var);

// Append property to current database.
db.Properties.Append(prpUserDefined);

// Enumerate all properties of current database.
printf("Properties of Database %s\n", db.GetName());
for (i = 0; i < db.Properties.GetCount(); i ++)
   {
   prpEnum = db.Properties[i];
   printf(" Properties(%d)\n", i); 
   printf("  Name: %s\n",  prpEnum.GetName());
   printf("  Type: %d\n", prpEnum.GetType());
   
   // Change type of variant to BSTR for printing.
   try
      {
      varBstr.ChangeType(VT_BSTR, var);
      printf("  Value: %s\n", (LPCSTR)varBstr.pbstrVal);
      }
   catch (CdbException e)
      {
      printf("  Value: Value could not be converted to a string\n");
      }
   
   printf("  Inherited: %d\n", prpEnum.GetInherited());
   }

This example shows how you can set an application-defined property (or any user-defined property that may not yet exist) without causing a run-time error. The example sets an arbitrary property of a Field object. The return value of the function is True if the value was properly set. The return value is False if an unexpected error occurs when the property is set. See the properties listed in the Property summary topic for additional examples.

LONG SetFieldProperty(
   CdbDBEngine &dbe,
   CdbField &fldPropVal, 
   LPCTSTR pstrName,
   int nType, 
   LPVARIANT pvarValue)
   {
#define ERR_PROPERTY_NONEXISTENT 3270
   CdbProperty prpUserDefined; 
   LONG      lErr;
   
   // Set the field property value.
   try 
      {
      fldPropVal.Properties[pstrName].SetValue(pvarValue);
      }
   
   catch (CdbException exSetPropValue)
      {
      lErr = dbe.Errors[(LONG)0].GetNumber();
      if (lErr != ERR_PROPERTY_NONEXISTENT)
         {
         return lErr;
         }
      else
         {
         // Create Property object, setting Name, Type, and Value properties.
         try
            {
            prpUserDefined = fldPropVal.CreateProperty(pstrName, nType, pvarValue);
            fldPropVal.Properties.Append(prpUserDefined);
            }
         catch (CdbException exUDP)
            {
            return dbe.Errors[(LONG)0].GetNumber();
            }
         }
      }
   return NOERROR;
   }

int main()
   {
   CdbDBEngine dbeng;
   CdbDatabase db;
   CdbTableDef td;
   CdbField   fld;
   COleVariant var("This is a user-defined property.", VT_BSTRT);
   LONG lErr;
   
   // Open the database.
   db = dbeng.OpenDatabase(_T("Northwind.mdb"));
   
   // Get a reference on a table.
   td = db.TableDefs[_T("Employees")];

   // Get a reference on the 'name' field.
   fld = td.Fields[_T("Last Name")];
   
   // Set the field property.
   lErr = SetFieldProperty(dbeng, fld, _T("foo"), dbText, (LPVARIANT)var);
   
   return (lErr == NOERROR) ? 0 : -1;
   }

