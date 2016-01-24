//-------------------------------------------------------------------//
// EMComponent.h
//
// As a base class of Earthmover component objects, EMComponent
// is an attempt to automate much of the functionality that is
// common across all Earthmover components.  This functionality
// includes arrays of DataItems, DataEnums, DataStrings, as well
// as lists of associated SubObjects, contained (member) SubObjects,
// and lists of lists of SubObjects.
//
// Many Earthmover modules (such as DBPropertySheet & EMCustomCtrl)
// manipulate EMComponent derived classes through EMComponent
// virtual functions without having any knowledge of what those
// classes are.  Hence there are a slough of virtual functions
// within EMComponent that may or may not need to be overrid in
// your derived class to handle specific member data stored within
// that class.
//
//-------------------------------------------------------------------//
#ifndef EM_COMPONENT_H
#define EM_COMPONENT_H

#include <bitset>							// For flags.

#include "EMArray.h"						// Custom CArray classes

#include "ObjectStream\ObjectStream.h"				// Persistent base class
#include "ObjectStream\TypedObjectReference.h"

#include "ReportSource.h"				// ReportSource base class
#include "EMSubObject.h"
#include "SubObjSlot.h"

#include "DataSource.h"					// Member data types
#include "DataItem.h"
#include "DataEnum.h"
#include "DataString.h"
#include "DataSpecial.h"

//-------------------------------------------------------------------//
// class pre-defines																	//
//-------------------------------------------------------------------//
// Pre-defined class to avoid nested include files...
// Most of these are used only as pointers in args
// to function calls, so the class defs are only needed
// in the cpp file.
//-------------------------------------------------------------------//

class EMComponentInit;
class EMComponent_Search;
class EMComponent;

class DBTreeControl;
class DBPropertySheet;
class ConfiguratorTreeControl;
class TypedRef;
class	OleDragData;
class CSpecTable;
class C2DChart;
class RefScanData;
class EMDatabase;
class CSiteController;
class RSD_TreeItemData;


//-------------------------------------------------------------------//
// DEFINES																				//
//-------------------------------------------------------------------//

const int NotAvail = -1;

// CArray typedefs
typedef CTypedPtrArray	< CPtrArray			, EMComponent*			> EMCArray;
typedef CArray				< ObjectReference	, ObjectReference&	> ObRefArray;

// bAcceptSubObj() argument defs
const int cnSearchAllSubObjs = -1;

const CString strRevEngCandidateMarker = _T("*");


//-------------------------------------------------------------------//
// class EMComponent																	//
//-------------------------------------------------------------------//
class EMComponent : public Persistent, public ReportSource
{

public:

	// Construction
	EMComponent(
		const ObjectReference	*pRef				= NULL,
		ObjectStream						*pNewDatabase	= NULL,
		EMComponentInit			*pInit			= NULL
	);

	// Destruction
	~EMComponent();

public:
	// Public data

	///////////////////////////////////////////////////////////////////////
	// EMComponentInit																	//
	///////////////////////////////////////////////////////////////////////
	// pEMInit is the Main initializer pointer.  It can be used to get
	// much we need to know about an object without instantiating that
	// object and reading it from the database.  A separate
	// EMComponentInit exists for every OBJECT_TYPE.
	//
	
	EMComponentInit *pEMInit;

	// Allocates all member arrays that are dependant upon pEMInit.
	void GetNewDataArrays();

	// Deallocates all member arrays that are dependant upon pEMInit.
	void DeleteDataArrays();
	
	//
	///////////////////////////////////////////////////////////////////////

	// Default DataSource:
	// This is the data source that gets applied to data fields when
	// "Apply To All" or "Apply To Changed Fields" gets pressed
	// on the Source Property Page.
	DataSource dsDefaultDataSource;
	

	///////////////////////////////////////////////////////////////////////
	// Persistent Flags																	//
	///////////////////////////////////////////////////////////////////////
	//
	
	// NOTE
	// You may add flags as needed, but be aware that a new flag will be set to 
	// false for objects that did not previously know of the flag.
	// You may adjust this by using versioning code in Read().
	// If you change the persistent size of the bitset, then you must add 
	// versioning code to update all previous objects.  However, the minimum
	// size of a bitset is 4 bytes, so you should be able to pump this up to 
	// 32 bits without changing the persistent size of the bitset.
	
protected:

	enum
	{
		PF_COMPLETENESS,
		PF_PRIVATE,
		PF_OFFICIAL,
		PF_REV_ENG_COMPLETENESS,
		PF_COUNT,
	};

	bitset<PF_COUNT> PersistentFlags;

public:

	// Completeness Flag.
	// This flag is set when OK or Apply is clicked on the
	// property sheet, as reported from IsDataComplete.  Other
	// processes can then rely on this flag, instead of having
	// to call IsDataComplete every time to check it.
	void SetCompleteness( bool bComplete = true		)	{ PersistentFlags.set( PF_COMPLETENESS, bComplete					);	}
	bool bIsComplete()											{ return PersistentFlags.at( PF_COMPLETENESS							);	}

	// Private flag.
	// For SubObjects and SubObject List objects.  This flag tells us
	// that this object was created solely for use by its parent
	// object, and it should be deleted if its parent is deleted.
	void SetPrivate( bool bPrivate = true				)	{ PersistentFlags.set( PF_PRIVATE, bPrivate							);	}
	bool bIsPrivate()												{ return PersistentFlags.at( PF_PRIVATE								);	}

	// Official flag.
	// This flag is set to true for all Accelerator-supplied objects.  Then, if the user
	// makes any non-configuration change to the object, it is set to false.
	void SetOfficial( bool bOfficial = true			)	{ PersistentFlags.set( PF_OFFICIAL, bOfficial						);	}
	bool bIsOfficial()											{ return PersistentFlags.at( PF_OFFICIAL								);	}

	// This flag determines if DoRevEng() found the object to be complete.  
	// DoRevEng() may find values out-of-range, for example.
	// We determine the status of this flag every time we save the object.
	// However, the overall completeness of an object is checked more
	// frequently, via IsDataComplete().  This flag allows us to maintain
	// the Rev Eng completeness status in between saves.
	// See IsDataComplete() for the call to bRevEngComplete().
	// See ValidateAndSave() for the call to SetRevEngComplete().
	void SetRevEngComplete( bool bComplete = true	)	{ PersistentFlags.set( PF_REV_ENG_COMPLETENESS, bComplete		);	}
	bool bRevEngComplete()										{ return PersistentFlags.at( PF_REV_ENG_COMPLETENESS				);	}

	//
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	// Non-persistent Flags																//
	///////////////////////////////////////////////////////////////////////
	//
	
protected:

	// See note for PersistentFlags, above.
	// This flag type now defined in EMSubObject.h, because so much of
	// the influence of these flags is coded there.
	NonPersistentFlagsBitSet NonPersistentFlags;

public:

	// Refresh Flag.
	// Turning off this flag tells all the sub-object processing
	// that it is ok to keep sub objects around until this object is
	// destroyed, as in this object is a "snapshot" object whose innards
	// do not need to be refreshed all the time.  Think of Spec Sheets.
	void SetRefresh( bool bRefresh = true			)	{ NonPersistentFlags.set( NPF_REFRESH, bRefresh					);	}
	bool bRefresh()											{ return NonPersistentFlags.at( NPF_REFRESH						);	}

	// Property Sheet flag.
	// This flag is used to track when this object is being displayed
	// on a propertysheet.  It is set within the SetPropSheetObject()
	// function, and should not be set or cleared anywhere else.
	void SetIsPropSheet( bool bPropSheet = true	)	{ NonPersistentFlags.set( NPF_PROP_SHEET_OBJECT, bPropSheet );	}
	bool bIsPropSheet()										{ return NonPersistentFlags.at( NPF_PROP_SHEET_OBJECT			);	}

	// This flag determines if DoRevEng() has been called, for the status page's benefit.
	void SetCalledDoRevEng( bool bCalled = true	)	{ NonPersistentFlags.set( NPF_CALLED_DO_REV_ENG, bCalled		);	}
	bool bCalledDoRevEng()									{ return NonPersistentFlags.at( NPF_CALLED_DO_REV_ENG			);	}

	// This indicates that we want to use the following subobject methodology:
	// If a subobject is requested, and there is a corresponding std subobject
	// that matches, then return the std.  This reduces the instances of an
	// object, thus reducing chances of lost changes. 
	void SetOptimizeStandard( bool bOpt = true )		{ NonPersistentFlags.set( NPF_OPTIMIZE_STANDARD, bOpt			);	}
	bool bOptimizeStandard()								{ return NonPersistentFlags.at( NPF_OPTIMIZE_STANDARD			);	}

protected:

	// This indicates that our state has been initialized.  This also indicates that subobjects 
	// should be initialized when loaded.  It also helps us make sure that the object is 
	// not accidentally re-initialized.
	void SetStateInitialized( bool bInited = true)	{ NonPersistentFlags.set( NPF_STATE_INITIALIZED, bInited		);	}
	bool bIsStateInitialized()								{ return NonPersistentFlags.at( NPF_STATE_INITIALIZED			);	}

	// This is saved during state mode so we can reset it when done.
	void SetPreviousRefresh( bool bPrev = true	)	{ NonPersistentFlags.set( NPF_PREVIOUS_REFRESH, bPrev			);	}
	bool bPreviousRefresh()									{ return NonPersistentFlags.at( NPF_PREVIOUS_REFRESH			);	}

	//
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	// VERSIONING
	///////////////////////////////////////////////////////////////////////
	//
	//
protected:
	// This just makes things look nicer, reduces typing a bit, in
	// derived class Read() functions.
	bool BeforeVersion( uLong Major, uLong Minor )
	{
		return ( bReadOK && pDatabase->GetDBVersion() < Version( Major, Minor ) );
	}
	//
public:
	// This function returns the lowest version number for which this
	// object does not need to be updated.  Persistent derived classes 
	// override this and provide the version number during which the 
	// last change to the database structure of the object occurred.  
	// EMComponent does this through EMComponentInit, therefore this 
	// function is not virtual here.
	uHuge GetLowestSafeVersion();
	//
	///////////////////////////////////////////////////////////////////////


public:

	// On the fly type identification.
	OBJECT_TYPE GetObjectType();

	// Gets the typed object reference for this object.
	TypedRef GetTypedRef();

	// We override the traversal functions so we can handle subobject
	// cleanup before traversing.
	virtual Persistent& FindObject(PdyKey *key);
	virtual Persistent& CurrentObject(PdyKey *key = 0);
	virtual Persistent& FirstObject(PdyKey *key = 0);
	virtual Persistent& LastObject(PdyKey *key = 0);
	virtual Persistent& NextObject(PdyKey *key = 0);
	virtual Persistent& PreviousObject(PdyKey *key = 0);

	///////////////////////////////////////////////////////////////////////
	// EMComponent Standard Overrides												//
	///////////////////////////////////////////////////////////////////////
	// If you have an object that has no object specific ("proprietary")
	// data, i.e. all the data in it are DataItem types or SubObjects,
	// then you won't need to override any of the virtual functions
	// below.  However, if you do override these functions, make sure
	// that you call the EMComponent version first to handle all the
	// base class data.
	///////////////////////////////////////////////////////////////////////

protected:

	// Database read and write functions.  These functions call ReadObject()
	// and WriteObject() (from Persistent.h) for each item of member data.
	//
	// These functions handle all reading and writing of
	// DataItems, DataEnums, DataStrings, and SubObject references.
	//
	// Derived versions need to read and write any "proprietary" data
	// contained within your derived class.  They should call
	// EMComponent::Read() and EMComponent::Write() first, then
	// handle proprietary data.
	virtual void Read();
	virtual void Write();

public:

	// Clears all data and intializes it to its default values.
	//
	// The EMComponent version handles all clearing of
	// DataItems, DataEnums, DataStrings, and SubObject references.
	//
	// Derived versions need to clear any "proprietary" data contained
	// within your derived class.
	virtual void Clear();

	// This function resets the values of all "state" variables, which
	// are defined as member variables that are not persistent.
	// After calling this function for the parent, SubObjects are
	// automatically initialized when loaded.
	virtual void InitState(
		bool bResetAsNeeded = false
	);

	void EndInitState();

	// Sets the DataSource of every item of data within
	// the object to be equal to the selected default source.
	// This function is called from the "Source" tab on
	// the property sheet when the "Apply to &All Fields"
	// or the "Apply to &Changed Fields" buttons are pressed.
	// If the bUserEnteredOnly flag is set, then the default
	// source should only be applied to sources that are user entered.
	//
	// The EMComponent version handles all SetSourcing of
	// DataItems, DataEnums, DataStrings, and SubObject references.
	//
	// Derived versions need to set the DataSources of any "proprietary"
	// data contained within you derived class.
	virtual void SetSource( bool bUserEnteredOnly = false );

	// This pseudo-abstract function allows derived classes to
	// fill in members and subobject members before IsDataComplete()
	// is called.  It is allowed to change data, as it is
	// called within ValidateAndSave().  Subobject changes should
	// be followed by a call to pSub->ChangeObject().  The changes
	// will be saved and propogated up to the parent through
	// the UpdateRevEngSubObjects() function.
	//
	// This function should use the m_ar_strDoRevEngMsgs array, 
	// stuffing any appropriate messages based on data validation
	// results (e.g., out-of-range values).  It returns true if all 
	// reverse engineering was completed, and false if there was
	// a problem that was noted in m_ar_strDoRevEngMsgs.
	virtual bool DoReverseEngineering() { return true; }

	// DoRevEng() feedback should be stuffed here.
	CStringArray m_ar_strDoRevEngMsgs;
	void AddRevEngMsg( CString strNew ) { m_ar_strDoRevEngMsgs.Add( strNew ); }
	void AddRevEngMsg( UINT nNew ) { m_ar_strDoRevEngMsgs.Add( CString( (LPSTR) nNew ) ); }
	
	// Many EMC-derived classes will have weights associated with them.
	// We provide a base-class virtual function to faciliate calculating 
	// the total weight of an object and its components.
	// This function looks up the weight specified in EMCInit.
	// If you do not specify a weight in EMCInit, this returns
	// a weight of 0.
	// You may also override this function in derived classes
	// to directly provide the weight, as needed.  See Options for
	// an exanple.
	virtual DataItem diWeight();

	// Determines if the object is complete and ready for use
	// in tests and calculations.
	//
	// The EMComponent version handles checking the completeness of:
	//
	//		1) DataItems based on their m_eReqStatus value
	//		2) DataEnums and DataStrings based upon their bComplete value
	//		3) Subobjects based on return value from CheckSubObjCompleteness()
	//		4) DataGrids based on 
	//
	// Derived versions need to check the completeness of any
	// member data proprietary to your derived class, such as
	// grid data.
	virtual bool IsDataComplete(
		CStringArray*	p_ar_strErr				= 0,	// Array for completeness error strings.
		bool*				pbFoundRevEngItems	= 0,	// true if we found items that may be rev-eng'ed
		int				nIndent					= 0	// # of tabs to add before error string.
	);

	// These functions return errors formatted for display.
	CString GetDataCompleteErrors( 
		CStringArray*	p_ar_strErr			,		// Array for completeness error strings.
		bool				bFoundRevEngItems			// true if we found items that may be rev-eng'ed
	);
	CString EMComponent::GetDoRevEngErrors();

	// This function scans through the Refs of the object in any
	// desired fashion, using the RefScanData object.  See the
	// function description in the implementation file for details.
	bool GetNextRef( RefScanData& Data );

	// This function allows the caller to loop through all EMSubObjects
	// (both single and in subobject lists).
	// Provide a pointer, and it will get updated during the call.
	// Returns false when we're all out of subobjects.
	bool GetNextSubObject( EMSubObject*& pSO );
	
	// Call this before starting your GetNextSubObject() loop.
	void ResetSubObjectLoop()
		{ nSO = nSOList = nSOIndex = 0; }

protected:
	
	// This function updates any subobjects that were changed during
	// DoReverseEngineering().  Remember to call ChangeObject() on 
	// them when you make changes.
	void UpdateRevEngSubObjects();

	// GetNextSubObject() index vars.
	int nSO;
	int nSOList;
	int nSOIndex;

	// This function allows derived classes to provide access to any
	// Refs buried in their proprietary data.  Refs returned by this
	// function will be processed within GetNextRef().
	// The overridden function should set Data.pRef and increment 
	// Data.nOtherRef as long as there are additional Refs to provide.  
	// See Tire::GetOtherRef() for an example.
	virtual void GetOtherRef( RefScanData& Data ) {}

public:

	///////////////////////////////////////////////////////////////////////
	// COPY																					//
	///////////////////////////////////////////////////////////////////////
	// We found that in Earthmover there are various situations that
	// require different kinds of "Copy" functions.  Originally, it was
	// all handled through an operator= function, which quickly got
	// hacked up with ambiguous flags that directed the program flow.
	// Now we try to clean that up using regular functions that take
	// arguments to direct the action, instead of static or member
	// flags that control the action behind the scenes.
	void Copy(
		EMComponent		*pSrcEMC,						// Source EMComponent pointer.
		bool				bDupePriv			= false,	// Duplicate private objects in the DB?
		bool				bDupeBackups		= false,	// Copy SubObj backup arrays too?
		SwapRefArray	*parSwap				= NULL	// Used internally by EMComponent to update linked refs.
	);

	// This virtual function, where overridden, copies any data in the
	// EMComponent derived classes that is not handled by EMComponent.
	virtual void DerivedCopy( EMComponent *pSrcEMC ) {}

	// This virtual function was created so that we can include any
	// special handling for new objects created as copies of other
	// objects.  It gets called immediately after the copy funcions
	// are used to copy the source object to our new object.
	virtual void OnDuplicateOnNew() {}

	// This virtual function, where overridden, copies any data in the
	// EMComponent derived classes that is not handled by EMComponent.
	// It is specific to copies of objects created for reports, which
	// we commonly refer to as "snapshot" objects.
	// The EMComponent version moves over all subobject pointers from
	// the src EMComponent to this EMComponent.
	virtual void ReportCopy( EMComponent *pSrcEMC );

	//	END COPY
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	// ReportSource overrides
	///////////////////////////////////////////////////////////////////////

	// Allow ReportSource derived classes to clean themselves up,
	// since we use multiple inheritance in some ReportSource
	// derived classes.  Made this function virtual abstract to
	// ensure that this gets handled where needed, because it can
	// be a bitch to debug and find where it is NOT handled.
	virtual void SelfDestruct() { delete this; }

	// Chart creation function.
	// See comments in EMComponent.cpp for this function.
	virtual ChartState eCreateChart(
		int			nChartIdx,		// Chart # to create
		int			nNumObjs,		// # of EMComponent derived objects in chart.
		C2DChart		*pChart,			// Chart to modify for creation.
		int			*pnTitleRes,	// Title of chart.
		int			*pnBranchRes,	// Chart type string.
		EMIntArray	*parNesting,	// Used to track "nested" subobject charts.  Used by 2DChart_Tree only.
		OBJECT_TYPE	eType				// The type of the topmost object, may be used to determine which charts to show.
	);

	// Returns the total number of charts available.
	virtual int nChartCount();

	// Graphic creation function.
	// See comments in ReportSource.cpp for this function.
	// This overrid version handles the default graphics (bmp/jpg)
	// automatically for those objects that have them.  It assumes
	// that if there is a default graphic that it will appear
	// at nGraphicIdx = 0.
	virtual int nDrawGraphic(
		int		nGraphicIdx,					// Graphic # to create.
		int		nNumObjs,						// # of objects in graphic.
		int		*pnTitleRes		= NULL,		// Title of graphic.
		CDC		*pDC				= NULL,		// Device context to draw within.
		CRect		rect				= CRect(),	// Rectangle to draw within.
		int		nAnimStep		= 0,			// Animation step to draw (mostly unused).
		GraphicContext eContext	= GC_Null	// Where this graphic is being used.
	);

	// Returns an index to the default graphic.
	virtual int nDefaultGraphicIdx();

	// Derived classes are responsible for providing a display
	// name if they are going to be displayed to the user in
	// standard form, for example in a tree control and property
	// sheet.  The display name is accessed through the
	// GetDisplayName function.
	virtual void GetDisplayName( CString* pstrName );

	// Get an appropriate title string for the top left side "object type".
	virtual CString strSubjectTypeName( int nCnt );

	///////////////////////////////////////////////////////////////////////

	// Fills in any data that needs to be calculated.  Is called from the
	// propsheet when OK or Apply is hit, to update the calculated data
	// with any changes that were made.
	// Only enters this routine when IsDataComplete is true.
	// Derived versions need to call the base class version in order to
	// handle internal data.
	virtual void DoCalcOtherData();

	// Does any special linked data calcs when a given control
	// has been updated on the PropertySheet for this object.
	// Replaces all the special handling that would have to be
	// done in the OnCommand within the individual PropertyPages.
	virtual void DoObjectRefresh( UINT CtrlID ) {}

	// Silly function created to remove some duplicate code.  Changes
	// to an object are usually followed by various calls to
	// SubObj_Validate(), IsDataComplete(), DoCalcOtherData(),
	// BuildDisplayName(), AddObject(), ChangeObject(), SaveObject(),
	// etc.  This function wraps all those, and then updates the
	// tree with the correct function calls.
	//
	// Notice the various options available through the arguments.
	// The default args will do the standard save, calc and update tree.
	ObjectReference ValidateAndSave(
		bool					bAdd					= false,		// Add it as a new object?
		bool					bDiffRef				= false,		// Copy over an existing object?
		ObjectReference	*pSaveAs				= NULL,		// The object to copy over.
		bool					bUpdateTree			= true,		// Update tree sort & name?
		bool					bErrorUpdateTree	= false,		// Update tree if errors found?
		CStringArray*		p_ar_strErr			= 0,			// Array for completeness error strings.
		bool*					pbFoundRevEngItems= 0,			// true if we found items that may be rev-eng'ed
		int					nIndent				= 0			// # of tabs to add before error string.
	);


	///////////////////////////////////////////////////////////////////////
	// INITIALIZING / ENABLING SUBOBJECTS
	///////////////////////////////////////////////////////////////////////
	//

	// Performs any initialization by the parent of a new subobject.
	// Currently only called for creation of private subobjects via
	// parents' subobject lists.
	virtual void InitializeSubObject( EMComponent* pSubObject )	{}

	// This allows EMC to enable/disable the "new"/"create new"/"create copy"
	// commands in subobject DBListControls.  
	// bPropSheetAdd specifies if a prop sheet is requesting the status, 
	// as opposed to a list control.
	// Only override as needed.
	// See EMInit's bAllowAdd function to enable/disable the frame's "add" 
	// button on prop sheets.  
	virtual bool bAllowNewSubObj( 
		OBJECT_TYPE		eType
	) {
		return true;
	}

	//
	///////////////////////////////////////////////////////////////////////


	///////////////////////////////////////////////////////////////////////
	//																							//
	// BASE DATA TYPE HANDLING:														//
	//																							//
	///////////////////////////////////////////////////////////////////////
	// Moved basic data types like DataItem, DataEnum, DataString to
	// CArrays here in the base class.  This allows us to automate a
	// HUGE amount of the functionality found within EMComponent
	// derived objects and their data.
	//
	//	Note that these arrays are always indexed through the use of
	// enumerated types.  These types will always be defined at the top
	// of the derived class's header files.
	///////////////////////////////////////////////////////////////////////

public:

	DataItem		*di;			// Data arrays
	DataEnum		*de;
	DataString	*dstr;

protected:

	// Clears all BaseData values, setting them to their defaults.
	void	BaseData_Clear();

	// Reads BaseData from the database.
	bool	BaseData_Read();

	// Writes BaseData to the database.
	void	BaseData_Write();

	// Copies the BaseData from the passed EMComponent to this one.
	void	BaseData_Copy( EMComponent *pSrcEMC );

	// This function sets all the sources in an object.  If bUserEnteredOnly
	// is false, ALL will be set to the default source.  If true, then only
	// the user entered sources will be defaulted.
	void	BaseData_SetSource( bool bUserEnteredOnly = false );

	// For those base data items that are flagged for completeness
	// checks, check to make sure that they have values.
	bool	BaseData_IsDataComplete( 
		CStringArray*	p_ar_strErr				= 0, 
		bool*				pbFoundRevEngItems	= 0,	// true if we found items that may be rev-eng'ed
		int nIndent									= 0 
	);

	// This function is NOT a completeness check - it only ensures that
	// the units are still at what they were initilized at.  Basically
	// checks to see if anything has been galloping through memory and
	// trashing things, since this should NEVER return false.
	//
	// NOTE: This function has been modified to fix trashed values by
	// initializing them to their defaults, since we may get hits here
	// due to changes within versions that we want to recover from.
	bool	BaseData_IsValid();

protected:

	///////////////////////////////////////////////////////////////////////
	// DataSpecial																			//
	///////////////////////////////////////////////////////////////////////
	// Abstract base class for EMComponent data types that are important
	// enough to need automation but not important enough to have their
	// own arrays within EMComponent.  For this option, EMComponent keeps
	// a CPtrArray of DataSpecials within itself and calls the DataSpecial
	// virtual methods at the appropriate times.
	///////////////////////////////////////////////////////////////////////

	// A CPtrArray of DataSpecials.
	DataSpecialArray arpDataSpecials;

	// ObjectStream read and write functions for the above.  All other DataSpecial
	// calls have been included within their EMComponent equivalents.  These
	// two had to be specific because of versioning in the databases.
	void ReadDataSpecials();
	void WriteDataSpecials();

public:

	// The major disadvantage of using DataSpecial is that you have to
	// remember to pass Add the DataSpecial ptr to arpDataSpecials in
	// the component's constructor.  Do so through this function.  The
	// bUseAutoReadWrite boolean is for backwards compatability in EM
	// to before the DataSpecial automation was added to EMComponent.
	void AddDataSpecial( DataSpecial *pDataSpecial, bool bUseAutoReadWrite = true );

	// Gets the number of DataSpecial ptrs in the CArray.
	int nGetDataSpecialSize();

	// Returns a given DataSpecial ptr from the CArray.
	DataSpecial *pGetDataSpecialPtr( int nIdx );


	///////////////////////////////////////////////////////////////////////
	//																							//
	// SUB-OBJECT HANDLING:																//
	//																							//
	///////////////////////////////////////////////////////////////////////
	// Objects referred to by other objects in a child/parent
	// relationship are referred to in Earthmover as SubObjects.  These
	// SubObjects are handled in the base class, EMComponent, in much
	// the same way as the BaseData classes.  They are stored in arrays
	// of objects, separated by their OBJECT_TYPE.
	//
	//	Note that these arrays are always indexed through the use of
	// enumerated types.  These types will always be defined at the top
	// of the derived class's header files.
	//
	// Subobjects are typically managed outside of the parent object.
	// The parent object just keeps a Ref to the object, and only
	// loads it temporarily under specific conditions.
	///////////////////////////////////////////////////////////////////////

public:

	// These are the master SubObject arrays.  All access to them
	// should be done through the functions below, to save some
	// maintenance headaches in the future.

	EMSubObjArray				SubObj;			// CArray of EMSubObjects.
	EMSubObjListArray			SubObjList;		// CArray of EMSubObjArrays.

	// Dynamic array of SubObject CArrays, used to backup the
	// information stored above for objects being displayed
	// in property sheets.
	EMSubObjListArray			SubObjBackup;	// CArray of EMSubObjArrays.


	// Returns the length of the requested SubObject List.
	int GetListSize( int nListIdx );

	// Very useful for looping through all the SubObjects within an
	// EMComponent object.  Returns true if a valid slot was found.
	bool GetNextSlot( SubObjSlot &Slot );

	// Get a pointer to a SubObject, loading from the DB if necessary.
	// These always return an EMComponent derived object of the specified
	//	OBJECT_TYPE, regardless if one exists in the DB or not.  If there
	// error or the SubObj ObjectReference is blank, will return a blank
	// temp object of the correct class.

	// SubObj version
	// MDM	7/7/99 1:40:40 PM
	// If a subobject is requested, and there is a corresponding standard object 
	// with the same Ref, this returns the object in the standard slot in place 
	// of the current.
	// This reduces the instances of an object, thus reducing chances of lost changes.
	// It is used in DoReverseEngineering().
	EMComponent* GetObjPtr( int nArrayIdx );

	// SubObj lists version
	EMComponent *GetObjPtr( int nListIdx, int nArrayIdx ) {
		return GetSubObj( nListIdx, nArrayIdx ).GetObjPtr( NonPersistentFlags );
	}
	// SubObjSlot version
	EMComponent *GetObjPtr( SubObjSlot Slot ) {
		return Slot.bIsList() ? GetObjPtr( Slot.m_nListIdx, Slot.m_nArrayIdx ) : GetObjPtr( Slot.m_nArrayIdx );
	}

	// Get the ObjectReference of a SubObj.

	// SubObj version
	ObjectReference *GetSubObjRef( int nArrayIdx ) {
		return &SubObj[ nArrayIdx ].ObRef();
	}
	// SubObj lists version
	ObjectReference *GetSubObjRef( int nListIdx, int nArrayIdx ) {
		return &((SubObjList[ nListIdx ])[ nArrayIdx ].ObRef());
	}
	// SubObjSlot version
	ObjectReference *GetSubObjRef( SubObjSlot Slot ) {
		return Slot.bIsList() ? GetSubObjRef( Slot.m_nListIdx, Slot.m_nArrayIdx ) : GetSubObjRef( Slot.m_nArrayIdx );
	}

	// Get the TypedRef of a SubObj.

	// SubObj version
	TypedRef GetSubObjTypedRef( int nArrayIdx );
	// SubObj lists version
	TypedRef GetSubObjTypedRef( int nListIdx, int nArrayIdx );
	// Returns all the refs in this subobject list.
	TypedRefArray GetSubObjTypedRefArray( int nListIdx );
	// SubObjSlot version
	TypedRef GetSubObjTypedRef( SubObjSlot Slot ) {
		return Slot.bIsList() ? GetSubObjTypedRef( Slot.m_nListIdx, Slot.m_nArrayIdx ) : GetSubObjTypedRef( Slot.m_nArrayIdx );
	}

	// Get EMSubObject class from the EMSubObject CArrays.  These should
	// be used primarily to get access to the flag Get and Set
	// functions of EMSubObject.

	// SubObj version
	EMSubObject &GetSubObj( int nArrayIdx ) {
		return SubObj[ nArrayIdx ];
	}
	// SubObj lists version
	EMSubObject &GetSubObj( int nListIdx, int nArrayIdx ) {
		return SubObjList[ nListIdx ][ nArrayIdx ];
	}
	// SubObjSlot version
	EMSubObject &GetSubObj( SubObjSlot Slot ) {
		return Slot.bIsList() ? GetSubObj( Slot.m_nListIdx, Slot.m_nArrayIdx ) : GetSubObj( Slot.m_nArrayIdx );
	}


	// Determine if SubObjs are present (selected) in this object.
	
	// SubObj version
	bool bHasSubObj( int nArrayIdx ) {
		return ( ! SubObj[ nArrayIdx ].ObRef().IsBlank() );
	}
	// SubObj lists version
	bool bHasSubObj( int nListIdx, int nArrayIdx ) {
		return ( 
				GetListSize( nListIdx ) > nArrayIdx									// Make sure the list is long enough.
			&&	!SubObjList[ nListIdx ][ nArrayIdx ].ObRef().IsBlank()		// Then check for blank.
		);
	}
	// SubObjSlot version
	bool bHasSubObj( SubObjSlot Slot ) {
		return Slot.bIsList() ? bHasSubObj( Slot.m_nListIdx, Slot.m_nArrayIdx ) : bHasSubObj( Slot.m_nArrayIdx );
	}

	// Detaches the pEMC ptr.  Call GetObjPtr() first to retrieve the
	// pointer, then this function to detach it.  The calling process is
	// in charge of deleting the pointer.

	// SubObj version
	void Detach( int nArrayIdx );

	// SubObj lists version
	void Detach( int nListIdx, int nArrayIdx ) {
		GetSubObj( nListIdx, nArrayIdx ).Detach();
	}
	// SubObjSlot version
	void Detach( SubObjSlot Slot ) {
		Slot.bIsList() ? Detach( Slot.m_nListIdx, Slot.m_nArrayIdx ) : Detach( Slot.m_nArrayIdx );
	}


	// Set a SubObject reference.
	// Returns true if the passed ref is not the same as the backup ref.
	bool SetSubObjRef(
		int					nArrayIdx,						// Which sub object to set.
		ObjectReference	*pRef,							// The ref of the new sub object.
		bool					bIsPrivate		= false,		// Is this a private object?
		bool					bIsConfigured	= false		// Is this a temp mem object?
	);

	// Called from the above function.  Allows derived classes to do any
	// special handling that results from the setting of a subobject ref.
	virtual void OnSetSubObjRef( int nArrayIdx, bool bIsBlank ) {}

	// Add a SubObject List reference.
	// Returns the index of that subobject in the list.
	int SubObjList_AddRef(
		int					nListIdx,						// Which sub object list to add to.
		ObjectReference	*pRef,							// The ref of the new sub object.
		bool					bIsPrivate		= false,		// Is this a private object?
		bool					bIsConfigured	= false		// Is this a configured object?
	);

	// SubObjSlot version
	void SetSubObjRef(
		SubObjSlot			Slot,								// Which sub object to set (or add)
		ObjectReference	*pRef,							// The ref of the new sub object.
		bool					bIsPrivate		= false,		// Is this a private object?
		bool					bIsConfigured	= false		// Is this a configured object?
	){
		if ( Slot.bIsList() )	SubObjList_AddRef	( Slot.m_nListIdx, pRef, bIsPrivate, bIsConfigured );
		else							SetSubObjRef		( Slot.m_nArrayIdx, pRef, bIsPrivate, bIsConfigured );
	}

	// Add a SubObject List reference to the backup list held in
	// memory for property sheet objects.  Used to ensure that
	// duplicate private objects created from drag and drop
	// operations across prop sheets can clean up after delete
	// and cancel actions.
	// Returns the index of that subobject in the list.
	int SubObjList_AddBackupRef(
		int					nListIdx,						// Which sub object list to add to.
		ObjectReference	*pRef,							// The ref of the new sub object.
		bool					bIsPrivate		= false,		// Is this a private object?
		bool					bIsConfigured	= false		// Is this a configured object?
	);

	// Delete a SubObject List reference (reference version).
	void SubObjList_DeleteRef( int nListIdx, const ObjectReference &Ref );

	// Delete a SubObject List reference (indexed version).
	void SubObjList_DeleteRef( int nListIdx, int nArrayIdx ) {
		SubObjList[ nListIdx ].RemoveAt( nArrayIdx );
	}

	// Delete all SubObjects in the requested list.
	void SubObjList_RemoveAll( int nListIdx );

	// Finds a ref within the specified list.
	// Returns the list index if found, or -1 if not found.
	int SubObjList_FindRef( int nListIdx, const ObjectReference &Ref );

	// Sets the SubObj.pEMC pointer.  This function should only be used
	// in the rarest of circumstances, such as in Tread\Tread3dPage where
	// we are making a bogus tire and need to bypass the SubObj handling.
	void SetSubObjPtr( int nListIdx, EMComponent *pEMC );

	// Checks to see if the EMSubObject::Modified flag is set on any of
	// the SubObjects of this object.
	bool bHasModifiedSubObj();

	// Finds all the objects in the databases that could be possible
	// candidates for the SubObject specified by nSubObjIdx.  Returns
	// the results in arRefs.
	void FindAcceptableSubObjRefs(
		int					nSubObjIdx,						// SubObj index we are searching for.
		ObRefArray			&arRefs							// CArray of refs to return results through.
	);

	// Used to generate a subobject log file for this object.
	// Made this virtual so that derived classes can add their
	// own proprietary line items.
	virtual CString strSubObjectLog(
		bool		bRecursive,						// Recur down through subobjects as well?
		CString	strIndent	= CString()		// Indent level for line items - keep this default.
	);

protected:

	// Called from SaveObject() to handle configured private
	// sub objects.  Made virtual so that derived classes
	// can add in their own special property sheet sub object
	// handling.  See ProjectSite.
	virtual void SaveSubObjConfigChanges();

public:

	// Called from DBPropFrame::Cancel() to remove any SubObjList
	// objects that were added during that property sheet session.
	// (Since the last Apply, of course!)
	void CancelSubObjListChanges();

protected:

	// Releases all memory associated with the SubObj array and blanks
	// all object references.  Also handles SubObj lists.
	void SubObj_Clear();

	// Reads SubObjects from the database.
	bool SubObj_Read();

	// Writes SubObjects to the database.
	void SubObj_Write();

	// Copies the SubObjects from the passed EMComponent to this one.
	void SubObj_Copy(
		EMComponent		*pSrcEMC,						// Source EMComponent pointer.
		bool				bDupePriv,						// Duplicate private objects in the DB?
		bool				bDupeBackups,					// Copy SubObj backup arrays too?
		SwapRefArray	*parSwap				= NULL	// Used internally by EMComponent to update linked refs.
	);


	///////////////////////////////////////////////////////////////////////
	// GATHERING
	// See function comments for details.
	//

public:

	virtual bool Gather(
		bool					bIncludePublished		= false,
		EMDatabase*			pGatheringDB			= 0,
		EMDatabase*			pGatheringFromDB		= 0,
		BaseProgressDlg*	pProgressDlg			= 0,
		fstream*				pLogFile					= 0,
		int					nLogIndents				= 0
	);

	//
	///////////////////////////////////////////////////////////////////////


	// Checks the completeness of an objects SubObjects.
	// This function alse checks completeness of all
	// SubObjects, down through the hierarchy of SubObjects.
	bool SubObj_IsDataComplete(
		CStringArray*	p_ar_strErr				= 0,	// Array for completeness error strings.
		bool*				pbFoundRevEngItems	= 0,	// true if we found items that may be rev-eng'ed
		int				nIndent					= 0
	);

	// This function scans through all the ObjectReferences
	// contained within this object of type eSwapType and
	// swaps any instances of SeekRef with SwapRef.
	void SwapRefs( SwapRefArray &arSwap );

protected:

	// InsertSubObj() and RemoveSubObj() allow EMComponent derived
	// classes to insert and remove SubObjects prior to and after 
	// reading from the database, for versioning purposes.

	// They MUST be used in conjunction (i.e. Remove after Insert)
	// to repair any changes made to the static EMComponentInit classes.

	// Pass the class's pInit for pBaseInit and NOT the pEMInit pointer!!!!

	// See Widget.cpp for an example of their use.

	void InsertSubObj( int nInsertAt, SubObj_Init &Initializer, EMComponentInit *pBaseInit );
	void RemoveSubObj( int nRemoveAt, EMComponentInit *pBaseInit  );

	void InsertSubObjList( int nInsertAt, SubObj_Init &Initializer, EMComponentInit *pBaseInit  );
	void RemoveSubObjList( int nRemoveAt, EMComponentInit *pBaseInit  );

	void InsertDataItem( int nInsertAt, DI_Init &Initializer, EMComponentInit *pBaseInit );
	void RemoveDataItem( int nRemoveAt, EMComponentInit *pBaseInit  );

	void InsertDataEnum( int nInsertAt, DE_Init &Initializer, EMComponentInit *pBaseInit );
	void RemoveDataEnum( int nRemoveAt, EMComponentInit *pBaseInit  );

	void InsertDataString( int nInsertAt, DS_Init &Initializer, EMComponentInit *pBaseInit );
	void RemoveDataString( int nRemoveAt, EMComponentInit *pBaseInit  );

	void InsertDataSpecial( int nInsertAt, DataSpecial *pDataSpecial, bool bUseAutoReadWrite );
	void RemoveDataSpecial( int nRemoveAt );

	// This function called at the end of EMComponent::LoadObject()
	// to allow for any versioning that needs to take place *after*
	// the object has been completely loaded and all derived classes
	// have finished their nested Read() calls.  This is the correct
	// place to put any EMComponent::***DatabaseUpdate() calls.
	virtual void PostLoadVersionUpdate() {}

	// This utility function is useful for backwards compatability
	// database maintenance.  Use it to move a sub object ref from
	// one place to another, either within the same object or
	// possibly across a subobject relationship.  It checks first to
	// see if this operation has been completed previously.  Ask
	// JWC about this before using it.  The return value specifies
	// whether the change was made.
	static bool SubObjDatabaseUpdate(
		EMComponent *pFromObj,
		int			nFromIdx,
		EMComponent *pToObj,
		int			nToIdx
	);

	// The subobject LIST version of the above function.
	static bool SubObjListDatabaseUpdate(
		EMComponent *pFromObj,
		int			nFromIdx,
		EMComponent *pToObj,
		int			nToIdx
	);

public:

	// This function takes every SubObject in this object and checks it
	// to make sure that it is still valid through the bAcceptSubObj()
	// function.  This is needed because a lot of SubObjects are
	// dependant upon the contents of other SubObjects.  For example, if
	// the manufacturer of a tire is changed, then we need to check the
	// compound and tread of that tire to make sure that they have the
	// same manufacturer.
	//
	// For SubObjects that are found to be invalid, this function blanks
	// the ObjectReference of that SubObject.
	bool SubObj_Validate();

	// This function gets called from ValidateAndSave() after the
	// SubObjects have been validated.  Like its title sez, its only
	// purpose is to sort the SubObject lists.  The base class version
	// sorts Options into their tree hierarchy order.  This function
	// is virtual and can be overrid to implement any special sorting
	// for derived classes.  For example, you may decide to sort objects
	// for a cleaner visual representation in propsheet list controls.
	virtual void SortSubObjectLists();

	// Releases all memory associated with the SubObj array.
	// Also handles SubObj lists.
	void SubObj_Release();

	// Deletes any private SubObjects of this object.
	void DeletePrivateSubObjects();

	// This function switches this object from normal operation to
	// property sheet mode.  This mode is needed so that the Cancel
	// button can back out of private SubObject changes on the prop
	// sheet.  All "configured" SubObject changes are kept in memory
	// until Apply or OK or Cancel is pressed.
	virtual void SetPropSheetObject();

	// Cleans up the SubObject backup information allocated by
	// EMComponent::SetPropSheetObject().
	void DeleteSubObjBackup();

	// Scans through this object's sub-objects for a match with class
	// type Type.  If ID is non-zero, then the scan will be for the
	// specific object Type( ID ).  If bRecursive is set then this f'n
	// will search all sub-objects of this object's sub-objects, too.
	bool SubObj_Search(
		OBJECT_TYPE			eType,
		EMCArray				*pResultList	= NULL,
		ObjectReference	*pRef				= NULL,
		bool					bRecursive		= false
	);

	// Searches through DB for objects that contain this one.  If
	// bShowDlg is true, then a dialog is put up with a list of all
	// such DB objects.  Returns true if any found.  Caution:  This
	// function can run for like 20 minutes on a big DB for objects
	// that are used everywhere as SubObjects, like Manufacturers.
	static bool bIsSubObject(
		OBJECT_TYPE				eType,				// OBJECT_TYPE of search object.
		ObjectReference		&ObRef,				// ObjectReference of search object.
		bool						bShowDlg,			// Show a results dialog?
		CString					*pstrDlgTitle,		// Results dialog title.
		EMComponent_Search	*pSearch				// Search results object.
	);

	// This function determines whether an object can accept another
	// object as a SubObject.  Note that there are several comments
	// pertaining to this function in the "SubObj struct types"
	// section at the top of EMComponentInit.h.  If pnWhereToPut
	// points to an integer with a value of cnSearchAllSubObjs, then
	// all SubObjs and SubObjLists are searched for an appropriate
	// place to put the SubObj.  If not, then the position specified
	// by pnWhereToPut and pbListOnly will be the only one checked.
	virtual bool bAcceptSubObj(
		OBJECT_TYPE			eSubObjType,					// Drop OBJECT_TYPE.
		ObjectReference	*pSubObjRef,					// Drop ObRef.
		bool					*pbAcceptInList	= NULL,	// Returns whether drop accepted in SubObj or SubObjList.
		int					*pnWhereToPut		= NULL,	// Where we would drop the SubObj if we could.
		CString				*pstrWarning		= NULL,	// Warning message to pass back to caller.
		bool					*pbListOnly			= NULL,	// Passed if we know whether to check list or non-list only.
		bool					bVerifying			= true	// Indicates whether we are verifying an existing item or adding a 
	);																// new.  This is for derived classes' benefit.

	// Loops through all the objects in the passed OleDragData.
	// Calls bAcceptSubObj, and goes ahead and stuffs the SubObj into
	// the target object if it is accepted.
	virtual bool bProcessDragArray(
		OleDragData	*pDragData,		// Items to add to SubObj.
		CString		*pstrWarning,	// Warning message to pass back to caller.
		bool			bSaveToDB,		// Save to DB if changed?
		bool			bDeleteIfBad,	// Delete items from pDragData that were unacceptable.
		SubObjSlot	SlotToPut,		// Where we would drop the SubObj if we could.
		BYTE			eFindFlags		// Flag fields to determine search behavior.
	);

	// This utility function handles a sub obj link to a sub obj list.
	// Note that this list can reside in any of the sub objects (recusively).
	bool ResolveSubObjLink(
		EMComponent		**ppLinkObj,			// Ptr to return found object through.
		int				*pnLinkIdx,				// Ptr to return list index through.
		int				*pnDepth,				// Ptr to return new list array depth.
		int				nWhere					// Which sub object we are trying to match.
	);

	// Gets the tree for this object.  Just a shortcut.
	DBTreeControl *GetTreePtr();

	// This function adds this object and it's sub-objects to the
	// Component Tree passed through the pointer.  We allow each
	// object to add itself so that any special cases can be
	// handled by the object itself.  The default EMComponent version
	// adds the object and all of it's sub objects according to the
	// booleans in the sub object initializer arrays, so overriding
	// this function will be the exception and not the rule.
	virtual HTREEITEM FillComponentTree(
		ConfiguratorTreeControl	*pTree,											// Tree we are filling.
		HTREEITEM					hParent,											// HTREEITEM of the parent node.
		bool							bAddSelfFirst,									// Add 'this' object first?
		OBJECT_TYPE					eRootType			= OBJECT_LIST_LEN,	// Root object type.
		OBJECT_TYPE					eParentType			= OBJECT_LIST_LEN		// Immediate parent object type.
	);

	// Called by FillComponentTree to allow any special enabling
	// on the component tree.  Note that the return value of this function
	// is ignored if the SubObj_Init ShowPos value for the
	// requested SubObject is set to NoShow.
	virtual bool EnableComponentItem(
		OBJECT_TYPE		eRootType,		// Root object type.
		OBJECT_TYPE		eParentType,	// Immediate parent object type.
		int				nWhichList,		// The SubObj List (or NotList for SubObj).
		int				nIdx				// The SubObj (List) Idx
	);

	// This method is used to fill the passed report selection tree
	// with available reports for this ReportSource.
	// Returns true if report items were added.
	// This version adds the basic "Overview" reports as well as
	// any nested subobject reports (recursively).
	bool FillReportSelectionTree(
		MultiSelectTreeControl &rTree,
		TypedRefArray &rarAdded,
		HTREEITEM hParent = NULL
	);

	// Override this function to add specialized reports to objects.
	// Each of these specialized reports will need a unique ID, as
	// listed within ReportSourceDefines.h.
	virtual bool FillReportSelectionTree_AddReports(
		MultiSelectTreeControl &rTree,	// The tree we are building up.
		RSD_TreeItemData &rData,			// The TreeItemData for the current object.
		HTREEITEM hParent,					// The parent HTREEITEM to add from.
		OBJECT_TYPE eReportTreeExpand		// The lowest object type for expand report selection tree branches.
	);

	// Used to allow special cases when checking the completeness
	// of SubObjects.  The default behavior is to check SubObjects
	// for completeness automatically but to bypass all SubObject
	// Lists objects.
	//
	// Override this function in your derived class to change this
	// behavior from the default.
	virtual bool CheckSubObjCompleteness(
		int nWhichList,	// The SubObj List (or NotList for SubObj).
		int nIdx				// The SubObj (List) Idx
	);

	// This determines if a subobject is optional, e.g. "none" should
	// be an available selection.  Override if a subobject is optional.
	virtual bool IsSubObjOptional(
		int nWhichList,	// The SubObj List (or NotList for SubObj).
		int nIdx				// The SubObj (List) Idx
	);

	// Override to add custom handling while adding and deleting of 
	// subobjects from lists.
	// Note that currently, nList = -1 for multiple-derived-class
	// lists.  You might want to update this to provide eObType, and
	// use it to determine what to do, if you need this functionality
	// for multiple-derived-class lists.
	virtual void SubObjectWasAdded( 
		int				nList,				// The SubObj List
		int				nCount		=	1	// The number of subobjects inserted
													// Drag & drop inserts may be > 1
	) {}
	virtual void SubObjectWasDeleted( 
		int				nList,				// The SubObj List
		int				nIdx					// The SubObj List Idx
	) {}

	///////////////////////////////////////////////////////////////////////
	// Option subobject functions
	///////////////////////////////////////////////////////////////////////
	// These utility functions help with EMComponent derived classes
	// that have Options subobjects.
	///////////////////////////////////////////////////////////////////////

	// Returns the index of the Option that matches requested option 
	// type and attributes.  Returns NotAvail if no matching option found.
	//
	// If a branch OptType is passed instead of a leaf OptType, this
	// function will return the first eligible Option that falls under
	// that branch.  Returns the found leaf OptType through pnOptionType.
	//
	// Note that SearchFlags comes from EMSubObject::DB_Flags, to allow
	// screening by "Installed" or "Standard".  The default is to just
	// search for availability.
	//
	// Note also that an int is used instead of an OptType for nOptionType
	// so that OptionTypes.h did not have to be included here.
	int nFindOption(
		int *				pnOptionType,				// OptType to search for.
		uByte				SearchFlags		= 0,		// Attributes to match.
		int				nStartFrom		= 0,		// List index to start from.
		EMComponent **	ppOption			= NULL	// Ptr to pass Option back through.
	);

	///////////////////////////////////////////////////////////////////////
	// Virtual functions from Persistant.h.
	///////////////////////////////////////////////////////////////////////

	void LoadObject(ObjAddr nd = 0);
	bool SaveObject();

	///////////////////////////////////////////////////////////////////////
	// TESTING
	///////////////////////////////////////////////////////////////////////
	void FillWithTestData( int nTestNumber );
	virtual bool operator==( EMComponent& SrcEMC );
	///////////////////////////////////////////////////////////////////////

	// This function must be overridden in the derived class
	// to supply sort group data.  Dynamic categories should
	// supply pstrGroup, and statics should provide pnGroup.
	// Not needed for objects that don't have trees.
	virtual void GetObjectSortGroupData(
		unsigned int	uSortCategory,
		CString*			pstrGroup,
		int*				pnGroup
	) {
		AfxDebugBreak();
	}

	// This function must be overridden in the derived class
	// to supply DBPropertyPages to add to the DBPropertySheet.
	// Not needed for objects that don't have property sheets,
	// assuming that there ever will be such a thing.
	virtual void AddSheetPages( DBPropertySheet* pDBPropSheet )
	{
		ASSERT( false );
	}

	// This function should be overrid by any derived class that
	// contains its own internal dirty flag.  This was originally
	// created for ProjectSite, so that Objective Diagram and our
	// own internal classes could share the same dirty flag without
	// the two realms having to be made aware of each other.

	// Returns a pointer to a bool if the custom dirty flag exists,
	// otherwise returns NULL.  The NULL return value tells DBPropFrame
	// that it can go ahead and use its own internal dirty flag.
	// This function should only be used by DBPropFrame.
	virtual bool *pGetDirtyBool() { return NULL; }

	///////////////////////////////////////////////////////////////////////
	// EMComponent static functions
	///////////////////////////////////////////////////////////////////////
	// Placed here because all processes that need these functions already
	// know about EMComponent, and putting them here instead of somewhere
	// else allows us to kill some includes in some cpp files.
	///////////////////////////////////////////////////////////////////////

	// Display name access using ID.
	static bool GetDisplayNameFromTree(
		CString*				pName,
      OBJECT_TYPE			eWhich,
		ObjectReference*	pRef
	);

	// Display name access using ID.  This version returns
	// the string instead of using the pointer, for those
	// cases where the return bool is unimportant and no
	// speed savings is required.
	static CString GetDisplayNameFromTree(
      OBJECT_TYPE			eWhich,
		ObjectReference*	pRef
	);
	// TypedRef version
	static inline CString GetDisplayNameFromTree( TypedRef Ref )	{ return GetDisplayNameFromTree( Ref, &Ref.Ref ); }

	// Determines whether an object is complete or not based
	// upon its sort order information.  Use of this function
	// assumes that the object's sort information has been
	// updated, which in most cases will be true.
	static bool GetCompletenessFromTree(
      OBJECT_TYPE			eWhich,
		ObjectReference*	pRef
	);
	// TypedRef version
	static inline bool GetCompletenessFromTree( TypedRef Ref )	{ return GetCompletenessFromTree( Ref, &Ref.Ref ); }

	//
	///////////////////////////////////////////////////////////////////////



//-------------------------------------------------------------------//
};	// class EMComponent declaration
//-------------------------------------------------------------------//



#endif

