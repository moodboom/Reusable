// ---------------------------------------------------------------
// EMDatabase implementation file
//
// Perform the follwing when adding new objects:
//		1)	Add an #include line.
//		2) Add an entry to the OBJECT_TYPE/ClassID map.
//		3) Add a case statement to the NewObject() function.
// ---------------------------------------------------------------

#include "stdafx.h"
#include "..\Earthmover.h"				// For theApp.PurgeMessages();

#include <direct.h>						// For _tmkdir
#include <sys/stat.h>					// For _tchmod
#include <io.h>							//  "    "

#include "..\RandomHelpers.h"

#include "EMDatabase.h"

#include "..\BlowFish\Blowfish.h"

#include "..\LogFrame.h"

#include "..\RefScanData.h"			// For CreateReferenceLog(), and used for a fix during publishing.

#include "TypedObjectReference.h"	// Nice OBJECT_TYPE - ObjectReference combo class


// ---------------------------------------------------------------
// We have to include EVERY SINGLE PERSISTENT OBJECT in this file
// that is potentially included in an Earthmover database.
// ---------------------------------------------------------------
#include "..\Manufacturer\Manufacturer.h"			   			//   0
#include "..\Source\Source.h"							   			//   1

#ifndef MINIMAL_OBJECT_DEFINITIONS

	#include "..\Tread\Tread.h"										//	  2
	#include "..\Compound\Compound.h"				   			//   3
	#include "..\Tire\Tire.h"							   			//   4
	#include "..\RimSize\RimSize.h"					   			//   5
	#include "..\LoadCurve\LoadCurve.h"				   			//   6
	#include "..\Material\Material.h"				   			//   7
	#include "..\Tire\TireDeflection.h"	  							//   8		**UNUSED**
	#include "..\RoadCross\RoadCross.h"								//   9
	#include "..\TireCondition\TireCondition.h"					//  10
	#include "..\Weather\Weather.h"									//  11
	#include "..\TireTest\TireTest.h"								//  12
	#include "..\Media\Media.h"										//  13
	#include "..\DBTreeCache.h"										//  14
	#include "DBAssociations.h"										//  15
	#include "..\TireTest\StandardTest.h"							//  16
	#include "..\Rim\Rim.h"												//  17
	#include "..\Axle\Axle.h"											//  18
	#include "..\Engine\Engine.h"										//  19
	#include "..\Rigid\Rigid.h"										//  20
	#include "..\Artic\Artic.h"										//  21
	#include "..\Differential\Differential.h"						//  22
	#include "..\FinalDrive\FinalDrive.h"							//  23
	#include "..\Brakes\Brakes.h"										//  24
	#include "..\Equipment\Equipment.h"								//  25
	#include "..\Transmission\Transmission.h"						//  26
	#include "..\TorqueConverter\TorqueConverter.h"				//  27
	#include "..\Hydraulics\HydraulicFunction.h"					//  28
	#include "..\Hydraulics\HydraulicCylinder.h"					//  29
	#include "..\Hydraulics\HydraulicMotor.h"						//  30
	#include "..\Hydraulics\HydraulicPump.h"						//  31
	#include "..\Power\Power.h"										//  32
	#include "..\Power\MechanicalPower.h"							//  33
	#include "..\RubberTired\RubberTired.h"						//  34
	#include "..\Hauler\Hauler.h"										//  35
	#include "..\Hydraulics\HydraulicCooling.h"					//  36
	#include "..\Transmission\TransmissionControl.h"			//  37
	#include "..\Hauler\HaulerBody.h"								//  38
	#include "..\Hauler\HaulerBodyLiner.h"							//  39
	#include "..\Hauler\HaulerTailgate.h"							//  40
	#include "..\Fan\Fan.h"												//  41
	#include "..\Fan\FanDrive.h"										//  42
	#include "..\Hydraulics\Hydraulics.h"							//  43
	#include "..\ElectricalSystem\ElectricalSystem.h"			//  44
	#include "..\Alternator\Alternator.h"							//  45
	#include "..\Batteries\Batteries.h"								//	 46
	#include "..\AirCompressor\AirCompressor.h"					//	 47
	#include "..\Chassis\RubberTiredChassis.h"					//	 48
	#include "..\Chassis\HaulerChassis.h"							//	 49
	#include "..\Cab\Cab.h"												//	 50
	#include "..\Dropbox\Dropbox.h"									//	 51
	#include "..\MonitoringSystem\MonitoringSystem.h"			//	 52
	#include "..\Hydraulics\HydraulicTank.h"						//	 53
	#include "..\CoolingSystem\CoolingSystem.h"					//	 54
	#include "..\CoolingSystem\Radiator.h"							//	 55
	#include "..\CoolingSystem\OilCooler.h"						//	 56
	#include "..\Option\Option.h"										//	 57
	#include "..\Loader\Loader.h"										//	 58
	#include "..\Loader\LoaderBoom.h"								//	 59
	#include "..\Chassis\LoaderChassis.h"							//	 60
	#include "..\Bucket\Bucket.h"										//	 61
	#include "..\Counterweight\Counterweight.h"					//	 62
	#include "..\Hydraulics\HydraulicHeatingLoad.h"				//	 63
	#include "..\Loader\LoaderMeasurements.h"						//	 64
	#include "..\Site\ProjectSite.h"									//	 65
	#include "..\Site\RoadSegmentStraight.h"						//	 66
	#include "..\Site\RoadSegmentNode.h"							//	 67
	#include "..\Site\LoadNode.h"										//	 68
	#include "..\Site\DumpNode.h"										//	 69
	#include "..\Site\Operator.h"										//	 70
	#include "..\Site\CarryObject.h"									//	 71
	#include "..\Site\LoadingObject.h"								//	 72
	#include "..\Manufacturer\MiningCompany.h"			   	//  73
	#include "..\ElectricDrive\ElectricPower.h"					//  74
	#include "..\ElectricDrive\WheelMotor.h"						//  75
	#include "..\ElectricDrive\DriveAlternator.h"				//  76
	#include "..\ElectricDrive\ElectricDrive.h"					//  77
	#include "..\ComponentCost\ComponentCost.h"					//  78

#endif // MINIMAL_OBJECT_DEFINITIONS
// ---------------------------------------------------------------

#include "..\DBTreeControl.h"		// For FillTrees()

#include "..\DBTreeCache.h"		// For FillTree()

#include "..\Dongle\Dongle.h"		// Helps with selection of publish group

#include "..\DisplayMessage.h"

#include "EMDBArray.h"				// For DB dir

#include "DBPropertiesDlg.h"		// To pop up properties.


//-------------------------------------------------------------------//
// Initialize statics.
//-------------------------------------------------------------------//
//

//////////////////////////////////////////////////////////////////////////////////////
// PROGRAM VERSION
//////////////////////////////////////////////////////////////////////////////////////
// See ObjectStream.txt for versioning details.
//
// Each EMComponent derived class has its own local version number stored within
// its EMComponentInit constructor.  That is the best place to do local "object
// type" specific updates.  Global updates can be done by modifying the EMComponent
//	GetLowestSafeVersion() return value.
//																Major,	Minor
const uHuge	uhCurrentEMFileVersion = Version	(	1,			34		);
const COleDateTime dtDemoExpirationDate = COleDateTime( 
	2000,		// Year
	12,		// Month
	31,		// Day
	0,			// Hour
	0,			// Minute
	0			// Second
);
//
// VERSION HISTORY
//
// [1.35]	MDM, 9/07/00	35 will be used for the actual subobject creation.
//									This is so John can "get" version 34 and compile now,
//									before I have the versioning code in place.
//									Bumped Demo expiration from 12/1 to 12/31.
// [1.34]	MDM, 9/06/00	Component costing is in place.  While rev eng estimates
//									still need improvement, all subobjects are now required.
//									I am setting up versioning to create and initialize 
//									subobjects in any pre-1.34 database.
// [1.33]	MDM, 8/24/00	Previously, DataGrid::[Read|Write]Object() used OBJECT_TYPE
//									values.  This was updated to use ClassID values, which are
//									not subject to change.  We updated all objects, since MANY
//									contain DataGrids.
//									Bumped Demo expiration from 10/1 to 12/1.
// [1.32]	MDM, 7/10/00	New demo mode expiration feature.  Not too hard to hack out,
//									but why not just get the latest demo version from the web?
//									Instant-access button is included on expiration dlg.
// [1.31]	MDM, 7/07/00	New DEMO MODE!  Now the demo user gets one personal 
//									working database, automatically created, that allows them 
//									to create new objects, and modify and save them.
//									Simulation is also greatly improved, and now includes
//									acceleration limits on straight segments and turns,
//									including adjustment for embankment.
// [1.30]	MDM, 6/08/00	Version incremented after sending to J. D. Wientjes.
//									Includes first full single simulation reporting.
//									Still need to debug problems with Komatsu 530M in
//									simulation (530M was removed for the build).
// [1.29]	JWC, 5/12/00	Version incremented after successful post to web site.
//									Updates include new machines, bug fixes, and new rev-eng code.
// [1.28]	MDM, 4/28/00	Updated LoaderChassis object format, see Read().
// [1.27]	MDM, 4/7/00		Cleaned up db mgr menus, added CopyTo, GatherTo, and 
//									CopyAndGatherTo items and functionality.
//									V 26 was posted to web on 4/5/2000.
// [1.26]	JWC, 3/20/00	Added versioning to DataSpecial to handle DataGrid column
//									additions within ProjectSite.
// [1.25]	MDM, 3/17/00	Version incremented after successful post to web site.  Demo
//									on web site now includes SiteWizard first release!!  Also updated
//									simulation code with new operator braking considerations and
//									acceleration enabled across downhill-to-uphill grade changes.
// [1.24]	MDM, 3/13/00	Version incremented after successful post to web site.  Decided
//									to let the simulation charts stay in this time.  Demo now includes
//									all current objects including all tires from the Master database.
//									Project site wizard was removed for the post.  Now added back in.
// [1.23]	MDM, 2/28/00	Version incremented after successful post to web site.
//									Simulation speed and total time, as well as project site
//									wizard, were removed for the post.  Now added back in.
// [1.22]	JWC, 2/15/00	Saving road editor data for ProjectSite Wizard.
// [1.21]	JWC, 1/24/00	Saving zoom magnification in ProjectSite.
// [1.20]	MDM, 11/19/99	Corrected the totally lame-o numeric tree sorting, which in turn
//									required invalidation of previous caches.
// [1.19]	JWC, 8/27/99	Added MORE measurement objects to ProjectSite.
// [1.18]	MDM, 7/08/99	Added flag for Rev Eng completeness to all objects.
//									This required an update to ALL objects, so we
//									checked for versions less than 18 in EMC::GetLowestSafeVersion()
//									and added versioning code to EMC::Read().
// [1.17]	MDM, 7/08/99	Latest changes, shipped to JD Winches at Komatsu.
// [1.16]	JWC, 6/16/99	Added measurement objects to ProjectSite.
// [1.15]	MDM, 6/16/99	Updated Loader and Site without careful versioning in place.  
//									WARNING: We bumped up OldestHandledEMFileVersion to match.
//									All old databases will no longer be accessible.
//									In the near future, we will "draw the line in the sand" for the
//									first official version.  After that point in time, ALL CHANGES
//									TO OBJECT FORMATS SHOULD BE VERSIONED, and uhOldestHandledEMFileVersion
//									should remain constant for the foreseeable future.
// [1.14]	MDM, 4/14/99	Updated ObjectReference structure (Major change!!).  Included addition
//									of UpdateDatabaseFormat(), which provides a place to add versioning
//									to handle changes to the database header (or other database structure 
//									if you're feeling ambitious).
// [1.13]	MDM, 4/2/99		Removed 2 DataItems from Bucket.
// [1.12]	MDM, 3/5/99		Removed dgInOut grid from Loader Chassis.
// [1.11]	MDM, 3/3/99		Moved the data of Tire Deflection (the only internal object) into Tire.
//									Then, killed ALL code to special-handle internal objects.  You will have
//									to manually remove all Deflection objects from updated databases, so we
//									should probably up the OldestHandledEMFV to 1.11 once everyone has had
//									a chance to update their databases.
// [1.10]	JWC, 1/25/99	Removed "Unused" DataItems, DataEnums and DataStrings.
// [1.09]	JWC, 1/22/99	Moved Hydraulics from Equipment to Power.
// [1.08]	JWC, 1/21/99	Removed OilCooler subobjects from Brakes.
//									Moved Hydraulics subobjects from Equipment to Power.
//									Removed all "Unused" subobject indexes.
// [1.07]	JWC, 12/16/98	Removed OilCooler subobject from Transmission.
// [1.06]	MDM, 12/3/98	Added Machine:Unspecified row to dgSensors grid in MonitoringSystems.  Also
//									removed RTC's RTC_Weight DataItem.  Also added Spillguard2 point to 
//									HaulerBody dimensions grid.  Also, John removed HydraulicCircuits from
//									Equipment ( see Equipment::Read() for details ).
// [1.05]   MDM, 12/2/98	Major version upgrade.  
//									All objects updated to use the new versioning mechanism.  All
//									use of bVersion bools was removed.  Olly's database was updated and 
//									verified.
//									Once all in-house databases are updated, we will remove all versioning code 
//									and put in permanent code to fail if we ever encounter a pre-1.05 database.
// [1.04]	MDM, 11/6/98	New versioning mechanism
//									Update of objects to use new numbered versioning mechanism.
//									Addition of many object changes using old bool versioning mechanism.
// [1.03]	MDM, 8/98		First official use of the version number.


const uHuge	uhOldestHandledEMFileVersion = Version( 1, 15 );
//////////////////////////////////////////////////////////////////////////////////////


//-------------------------------------------------------------------//
// ClassID Map																			//
//-------------------------------------------------------------------//
// OBJECT_TYPE and ClassID are mapped to each other with this array.
// New types should be added LAST with a new ClassID one above the
// current highest.
// Conversions are available via the EMDatabase::GetObjectType() and
// GetClassID() functions.

struct OBJECT_TYPE_MAP
{
	OBJECT_TYPE	ObType;
	ClassID		CID;
};

const OBJECT_TYPE_MAP ObjectTypeMap[ OBJECT_LIST_LEN ] =
{
//	  OBJECT_TYPE							ClassID
	{ OBJECT_MFR,							0	},
	{ OBJECT_SOURCE,						1	},

#ifndef MINIMAL_OBJECT_DEFINITIONS

	{ OBJECT_TIRE_TREAD,					2	},
	{ OBJECT_TIRE_COMPOUND,				3	},
	{ OBJECT_LOAD_CURVE,					4	},
	{ OBJECT_TIRE,							5	},
	{ OBJECT_TIRE_RIM_SIZE,				6	},
	{ OBJECT_MATERIAL,					7	},
	{ OBJECT_TIRE_DEFLECTION,			8	},		// **UNUSED**
	{ OBJECT_ROAD_CROSS_SECTION,		9	},
	{ OBJECT_TIRE_TEST_CONDITION,		10	},
	{ OBJECT_WEATHER,						11	},
	{ OBJECT_TIRE_TEST,					12	},
	{ OBJECT_MEDIA,						13	},
	{ OBJECT_TREE_CACHE,					14	},
	{ OBJECT_DB_ASSOCIATIONS,			15	},
	{ OBJECT_STD_TEST,					16	},
	{ OBJECT_TIRE_RIM,					17	},
	{ OBJECT_AXLE,							18	},
	{ OBJECT_ENGINE,						19	},
	{ OBJECT_RIGID,						20	},
	{ OBJECT_ARTIC,						21	},
   { OBJECT_DIFFERENTIAL,				22	},
   { OBJECT_FINAL_DRIVE,				23	},
   { OBJECT_BRAKES,						24	},
	{ OBJECT_EQUIPMENT,					25	},
	{ OBJECT_TRANSMISSION,				26	},
	{ OBJECT_TORQUE_CONVERTER,			27	},
	{ OBJECT_HYDRAULIC_FUNCTION,		28	},
	{ OBJECT_HYDRAULIC_CYLINDER,		29	},
	{ OBJECT_HYDRAULIC_MOTOR,			30 },
	{ OBJECT_HYDRAULIC_PUMP,			31	},
	{ OBJECT_POWER,						32	},
	{ OBJECT_MECH_POWER,					33	},
	{ OBJECT_RUBBER_TIRED,				34	},
	{ OBJECT_HAULER,						35	},
	{ OBJECT_HYDRAULIC_COOLING,		36 },
	{ OBJECT_TRANSMISSION_CONTROL,	37	},
	{ OBJECT_HAULER_BODY,				38	},
	{ OBJECT_HAULER_BODY_LINER,		39	},
	{ OBJECT_HAULER_TAILGATE,			40	},
	{ OBJECT_FAN,							41 },
	{ OBJECT_FAN_DRIVE,					42 },
	{ OBJECT_HYDRAULICS,					43	},
	{ OBJECT_ELECTRICAL_SYSTEM,		44	},
	{ OBJECT_ALTERNATOR,					45	},
	{ OBJECT_BATTERIES,					46	},
	{ OBJECT_AIR_COMPRESSOR,			47	},
	{ OBJECT_RUBBER_TIRED_CHASSIS,	48 },
	{ OBJECT_HAULER_CHASSIS,			49 },
	{ OBJECT_CAB,							50 },
	{ OBJECT_DROPBOX,						51 },
	{ OBJECT_MONITORING_SYSTEM,		52 },
	{ OBJECT_HYDRAULIC_TANK,			53 },
	{ OBJECT_COOLING_SYSTEM,			54	},
	{ OBJECT_RADIATOR,					55	},
	{ OBJECT_OIL_COOLER,					56	},
	{ OBJECT_OPTION,						57	},
	{ OBJECT_LOADER,						58	},
	{ OBJECT_LOADER_BOOM,				59	},
	{ OBJECT_LOADER_CHASSIS,			60	},
	{ OBJECT_BUCKET,						61	},
	{ OBJECT_COUNTERWEIGHT,				62	},
	{ OBJECT_HYDRAULIC_HEAT_LOAD,		63	},
	{ OBJECT_LOADER_MEASUREMENTS,		64	},
	{ OBJECT_PROJECT_SITE,				65	},
	{ OBJECT_ROAD_SEGMENT_STRAIGHT,	66	},
	{ OBJECT_ROAD_SEGMENT_NODE,		67	},
	{ OBJECT_LOAD_NODE,					68	},
	{ OBJECT_DUMP_NODE,					69	},
	{ OBJECT_OPERATOR,					70 },
	{ OBJECT_CARRY_OBJ,					71 },
	{ OBJECT_LOADING_OBJ,				72 },
	{ OBJECT_MINE_CO,						73 },
	{ OBJECT_ELEC_POWER,					74	},
	{ OBJECT_WHEEL_MOTOR,				75	},
	{ OBJECT_DRIVE_ALTERNATOR,			76 },
	{ OBJECT_ELEC_DRIVE,					77 },
	{ OBJECT_COMPONENT_COST,			78	}

	// ** Add new database object types here. See ObjectTypes.txt.  **

#endif // MINIMAL_OBJECT_DEFINITIONS

};

// Re-Mappings of the above for quicker lookups.  See InitializeStatics().
OBJECT_TYPE EMDatabase::arObType [ OBJECT_LIST_LEN ];
ClassID EMDatabase::arCID [ OBJECT_LIST_LEN ];


// We use this var to specify the expected size of the plain header 
// for this version of Earthmover.
const uShort uExpectedEMPHSize = 
		sizeof uShort								// for uEMPlainHeaderSize
	+	sizeof uShort								// for HeaderSeed
	+	nUserNameLength * sizeof wchar_t		// for UserName
	+	nCompanyLength * sizeof wchar_t		// for Company
	+	nContactLength * sizeof wchar_t;		// for Contact

// We use this var to specify the expected size of the header 
// for this version of Earthmover.
const uShort uExpectedEMEHSize = 
		sizeof uShort				// for uEMEncryptedHeaderSize
	+	sizeof EMDBHeader;		// for EMHeader
//
//-------------------------------------------------------------------//



//-------------------------------------------------------------------//
// NewObject()																			//
//-------------------------------------------------------------------//
// This function is given the classid and returns a temp object
// of that classid type that is attached to this database.
// It is called by RegisterClasses() and RebuildIndexFile().
//-------------------------------------------------------------------//
Persistent* EMDatabase::NewObject(
	ClassID				classid,
	ObjectReference*	pRef
) {

	Persistent* pTemp = NULL;

	switch ( classid ) {
		case 0	: pTemp = new Manufacturer				( pRef, this );	break;
		case 1	: pTemp = new Source						( pRef, this );	break;

#ifndef MINIMAL_OBJECT_DEFINITIONS

		case 2	: pTemp = new Tread						( pRef, this );	break;
		case 3	: pTemp = new Compound					( pRef, this );	break;
		case 4	: pTemp = new LoadCurve					( pRef, this );	break;
		case 5	: pTemp = new Tire						( pRef, this );	break;
		case 6	: pTemp = new RimSize					( pRef, this );	break;
		case 7	: pTemp = new Material					( pRef, this );	break;
		case 8	: // *OLD* 
					  // Do not reuse this ClassID.  You MAY reuse the 
					  // corresponding OBJECT_TYPE.
					  // TO DO
					  // Clean up handling of old classes so this can be removed.
					  pTemp = new TireDeflection			( pRef, this );	break;
		case 9	: pTemp = new RoadCross					( pRef, this );	break;
		case 10	: pTemp = new TireCondition			( pRef, this );	break;
		case 11	: pTemp = new Weather					( pRef, this );	break;
		case 12	: pTemp = new TireTest					( pRef, this );	break;
		case 13	: pTemp = new Media						( pRef, this );	break;
		case 14	: pTemp = new DBTreeCache				( (OBJECT_TYPE)-1, this );	break;	// We want a temp object
		case 15	: pTemp = new DBAssociations			( this, true );	break;				// We want a temp object
		case 16	: pTemp = new StandardTest				( pRef, this );	break;
		case 17	: pTemp = new Rim							( pRef, this );	break;
		case 18	: pTemp = new Axle						( pRef, this );	break;
		case 19	: pTemp = new Engine						( pRef, this );	break;
		case 20	: pTemp = new Rigid						( pRef, this );	break;
		case 21	: pTemp = new Artic						( pRef, this );	break;
		case 22	: pTemp = new Differential				( pRef, this );	break;
		case 23	: pTemp = new FinalDrive				( pRef, this );	break;
		case 24	: pTemp = new Brakes						( pRef, this );	break;
		case 25	: pTemp = new Equipment					( pRef, this );	break;
		case 26	: pTemp = new Transmission				( pRef, this );	break;
		case 27	: pTemp = new TorqueConverter			( pRef, this );	break;
		case 28	: pTemp = new HydraulicFunction		( pRef, this );	break;
		case 29	: pTemp = new HydraulicCylinder		( pRef, this );	break;
		case 30	: pTemp = new HydraulicMotor			( pRef, this );	break;
		case 31	: pTemp = new HydraulicPump			( pRef, this );	break;
		case 32	: pTemp = new Power						( pRef, this );	break;
		case 33	: pTemp = new MechanicalPower			( pRef, this );	break;
		case 34	: pTemp = new RubberTired				( pRef, this );	break;
		case 35	: pTemp = new Hauler						( pRef, this );	break;
		case 36	: pTemp = new HydraulicCooling		( pRef, this );	break;
		case 37	: pTemp = new TransmissionControl	( pRef, this );	break;
		case 38	: pTemp = new HaulerBody				( pRef, this );	break;
		case 39	: pTemp = new HaulerBodyLiner			( pRef, this );	break;
		case 40	: pTemp = new HaulerTailgate			( pRef, this );	break;
		case 41	: pTemp = new Fan							( pRef, this );	break;
		case 42	: pTemp = new FanDrive					( pRef, this );	break;
		case 43	: pTemp = new Hydraulics				( pRef, this );	break;
		case 44	: pTemp = new ElectricalSystem		( pRef, this );	break;
		case 45	: pTemp = new Alternator				( pRef, this );	break;
		case 46	: pTemp = new Batteries					( pRef, this );	break;
		case 47	: pTemp = new AirCompressor			( pRef, this );	break;
		case 48	: pTemp = new RubberTiredChassis		( pRef, this );	break;
		case 49	: pTemp = new HaulerChassis			( pRef, this );	break;
		case 50	: pTemp = new Cab							( pRef, this );	break;
		case 51	: pTemp = new Dropbox					( pRef, this );	break;
		case 52	: pTemp = new MonitoringSystem		( pRef, this );	break;
		case 53	: pTemp = new HydraulicTank			( pRef, this );	break;
		case 54	: pTemp = new CoolingSystem			( pRef, this );	break;
		case 55	: pTemp = new Radiator					( pRef, this );	break;
		case 56	: pTemp = new OilCooler					( pRef, this );	break;
		case 57	: pTemp = new Option						( pRef, this );	break;
		case 58	: pTemp = new Loader						( pRef, this );	break;
		case 59	: pTemp = new LoaderBoom				( pRef, this );	break;
		case 60	: pTemp = new LoaderChassis			( pRef, this );	break;
		case 61	: pTemp = new Bucket						( pRef, this );	break;
		case 62	: pTemp = new Counterweight			( pRef, this );	break;
		case 63	: pTemp = new HydraulicHeatingLoad	( pRef, this );	break;
		case 64	: pTemp = new LoaderMeasurements		( pRef, this );	break;
		case 65	: pTemp = new ProjectSite				( pRef, this );	break;
		case 66	: pTemp = new RoadSegmentStraight	( pRef, this );	break;
		case 67	: pTemp = new RoadSegmentNode			( pRef, this );	break;
		case 68	: pTemp = new LoadNode					( pRef, this );	break;
		case 69	: pTemp = new DumpNode					( pRef, this );	break;
		case 70	: pTemp = new Operator					( pRef, this );	break;
		case 71	: pTemp = new CarryObject				( pRef, this );	break;
		case 72	: pTemp = new LoadingObject			( pRef, this );	break;
		case 73	: pTemp = new MiningCompany			( pRef, this );	break;
		case 74	: pTemp = new ElectricPower			( pRef, this );	break;
		case 75	: pTemp = new WheelMotor				( pRef, this );	break;
		case 76	: pTemp = new DriveAlternator			( pRef, this );	break;
		case 77	: pTemp = new ElectricDrive			( pRef, this );	break;
		case 78	: pTemp = new ComponentCost			( pRef, this );	break;

	// WARNING!  You may add to this list, but don't delete or
	// reorder items in it.

#endif // MINIMAL_OBJECT_DEFINITIONS

		// Unknown type found.
		default	:	ASSERT( false );
	}

	return pTemp;

}


//-------------------------------------------------------------------//
// EMDatabase()																		//
//-------------------------------------------------------------------//
// constructor
//-------------------------------------------------------------------//
EMDatabase::EMDatabase(
	CString* pstrFileName
) :

	// Base class.
	ObjectStream(
		pstrFileName,												// FileName,

		#ifdef MINIMAL_OBJECT_DEFINITIONS
			2
		#else
			OBJECT_LIST_LEN
		#endif

	),

	// Init vars.
	
	// Set these to the current size.
	uEMPlainHeaderSize( uExpectedEMPHSize ),
	uEMEncryptedHeaderSize( uExpectedEMEHSize )


{
}


//-------------------------------------------------------------------//
// ~EMDatabase()																		//
//-------------------------------------------------------------------//
EMDatabase::~EMDatabase()
{
}


//-------------------------------------------------------------------//
// InitializeStatics()																//
//-------------------------------------------------------------------//
// Fill out arObType[] and arCID[] from ObjectTypeMap[] so 
// that they can be used by GetObjectType() and GetClassID()
// for quick lookups.
//
// Currently both the OBJECT_TYPEs and ClassIDs both are
// zero based, so this little short cut will work.  In the
// future, though, this constraint might change in which
// case this quick lookup code will have to be zapped.
//
// This function (currently) only called from the EMDBArray
// constructor.
//-------------------------------------------------------------------//
void EMDatabase::InitializeStatics()
{
	int nA, nB;

	// Loop through the ClassIDs and OBJECT_TYPEs and make the
	// quick loopkup arrays.  Both ClassIDs and OBJECT_TYPEs 
	// run from 0 to OBJECT_LIST_LEN.  If this ever changes,
	// the ASSERTs below will trip and we will have to change 
	// these loop bounds and revise our strategy.
	for ( nA = 0; nA < OBJECT_LIST_LEN; nA++ ) 
	{
		// Go find the matching ClassID for OBJECT_TYPE nA.
		for ( nB = 0; nB < OBJECT_LIST_LEN; nB++ )
		{
			if ( ObjectTypeMap[ nB ].ObType == (OBJECT_TYPE) nA )
			{
				arCID[ nA ] = ObjectTypeMap[ nB ].CID;
				break;
			}
		}

		// Could not find (OBJECT_TYPE) nA in ObjectTypeMap[].
		// Is something wrong, or are ClassID and OBJECT_TYPEs
		// not zero based anymore?
		ASSERT( nB < OBJECT_LIST_LEN );

		// Go find the matching OBJECT_TYPE for ClassID nA.
		for ( nB = 0; nB < OBJECT_LIST_LEN; nB++ )
		{
			if ( ObjectTypeMap[ nB ].CID == (ClassID) nA )
			{
				arObType[ nA ] = ObjectTypeMap[ nB ].ObType;
				break;
			}
		}

		// Could not find (ClassID) nA in ObjectTypeMap[].
		// Is something wrong, or are ClassID and OBJECT_TYPEs
		// not zero based anymore?
		ASSERT( nB < OBJECT_LIST_LEN );
	}
}

//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// This version adds the pObjectSeed param.  We need to handle 
// specification of the object seed during node-level copying; see below.
//-------------------------------------------------------------------//
DB_INIT_RESULT EMDatabase::Initialize(
	bool				bFileShouldExist,
	const wstring*	pwsDatabaseName,
	DatabaseID*		pDBID,
	uLong				Group,
	uLong				DBVersion,
	bool				bPublished,
	uShort*			pObjectSeed,
	bool				bForceReindex,
	short int		nDataGranularity,
	short int		nIndexGranularity
) {

	if ( !bFileShouldExist ) 
	{
		// The object seed is supplied when we are making a copy of an existing 
		// database and then doing a low-level copy of the nodes (e.g., during
		// Defragmentation).  In most cases, this param is not passed.
		if ( pObjectSeed ) 
		{
			EMHeader.ObjectSeed = *pObjectSeed;

		} else 
		{
			// We want to get a random number for the ObjectSeed.  This should
			// only be done once, at the creation of the database.  From that
			// point on, the seed must be constant.
			// Zero has special meaning, avoid it.  See GetHeaderKey().
			do 
			{ 
				EMHeader.ObjectSeed = get_random_uShort();
			} while ( EMHeader.ObjectSeed == 0 );

		}

		// MDM	7/6/2000 11:52:30 AM
		// Improving access in demo mode.
		// We now need to handle the user being able to create new demo 
		// databases on-the-fly (although this happens "automatically"
		// behind their backs).
		if ( Group == DemoGroup )
		{
			wcsncpy( (wchar_t*) UserName	,	L"Demo User"	, nUserNameLength	);
			wcsncpy( (wchar_t*) Company	,	L""				, nCompanyLength	);
			wcsncpy( (wchar_t*) Contact   ,	L""				, nContactLength	);

		} else
		{
			// Put the current user's data in the header.
			MainDongle.GetUserData(
				(wchar_t*) UserName	,
				(wchar_t*) Company	,
				(wchar_t*) Contact
			);
		}
	}

	return ObjectStream::Initialize(
		bFileShouldExist,
		pwsDatabaseName, 
		pDBID,           
		Group,
		DBVersion,
		bPublished,
		bForceReindex,   
		nDataGranularity,
		nIndexGranularity
	);

}


//-------------------------------------------------------------------//
// ReadPlainHeader()																	//
//-------------------------------------------------------------------//
// This function reads the unencrypted portion of the header for
// EMDatabase's.
// Update the static uExpectedEMPHSize as needed when adding a new
// header version.
//-------------------------------------------------------------------//
void EMDatabase::ReadPlainHeader()
{

	// Read the base class header, which comes first.
	// This places us at the correct file offset.
	ObjectStream::ReadPlainHeader();

	// Read the size of the EM plain header.
	datafile.ReadData(
		&uEMPlainHeaderSize,
		sizeof uEMPlainHeaderSize
	);
	
	if ( uEMPlainHeaderSize == uExpectedEMPHSize ) 
	{
		// Read the header seed.  The return code for this
		// seed will be the header key.
		datafile.ReadData(
			&HeaderSeed,			// buffer
			sizeof HeaderSeed		// bytes to read
		);

		// Read the user data.
		datafile.ReadData(	UserName,	sizeof UserName	);
		datafile.ReadData(	Company,		sizeof Company		);
		datafile.ReadData(	Contact,		sizeof Contact		);


	// Handle old versions here.
	
	// --------------------------------------------------------------------
	// This is the original way things were set up, with the user data
	// in the encrypted section.  We moved it out (before the first ship)
	// so that it would not be encrypted, to help the end user figure out
	// why they couldnt access the db.
	// ( We did this before the first ship, so there is no reason to include
	//   it in the code anymore.  Besides, it is not finished - see ReadEncHeader().)
	/*
	} else if ( 
		uEMPlainHeaderSize ==
			sizeof uShort								// for uEMPlainHeaderSize
		+	sizeof uShort								// for HeaderSeed
	) {

		datafile.ReadData(
			&HeaderSeed,			// buffer
			sizeof HeaderSeed		// bytes to read
		);
	*/
	// --------------------------------------------------------------------
	

	} else 
	{
		// This database has an unknown format!
		DisplayMessage( IDM_UNKNOWN_HEADER );
	
	}

}


//-------------------------------------------------------------------//
// GetHeaderKey()																		//
//-------------------------------------------------------------------//
// For EMDatabases, the header key is the return code for the header
// seed.  The header seed was randomly generated during database
// creation.
//-------------------------------------------------------------------//
bool EMDatabase::GetHeaderKey( 
	uByte** ppKey, 
	uShort* pKeyBytes 
) {

	// Allocate the key buffer.
	uHuge* pKey = new uHuge;
	
	// Point the param to our buffer.
	*ppKey = (uByte*) pKey;

	// Assign the size.
	*pKeyBytes = sizeof uHuge;

	// When decrypting the header, we do not have access to the DBID,
	// so we can't determine if we have a demo database from it.  
	// Instead, we check for a header seed value of zero.
	// Note that this verification is only used during writing and
	// reading of the header.  The DBID should be used to test for
	// demo databases everywhere else.
	if ( HeaderSeed == 0 ) 
	{
		*pKey = 0x325bdb59799ef6b7;

	// We should have access to the dongle.
	} else 
	{
		// Get the HASP return code for HeaderSeed.  This
		// is the header key.
		// Just for fun, we create another dongle object.
		Dongle D;
		*pKey = D.GetReturnCode( HeaderSeed );

	}

	return true;

}


//-------------------------------------------------------------------//
// ReadEncryptedHeader()															//
//-------------------------------------------------------------------//
// This function reads the encrypted portion of the header.
// Update the static uExpectedEMEHSize as needed when adding a new
// header version.
//-------------------------------------------------------------------//
bool EMDatabase::ReadEncryptedHeader()
{

	// Read the base class header, which comes first.
	bool bReturn = ObjectStream::ReadEncryptedHeader();

	if ( bReturn ) 
	{
		// Read the size of the encrypted header.
		datafile.ReadData(
			&uEMEncryptedHeaderSize,
			sizeof uEMEncryptedHeaderSize
		);
		
		// We'll hit this if we are reading a database that was not created
		// with this version of Earthmover.  It's OK if the size we read is
		// smaller than expected; it should not be bigger.
		if ( uEMEncryptedHeaderSize == uExpectedEMEHSize ) 
		{		
			// Read the encrypted header block.  We are already at the
			// correct file offset.
			datafile.ReadData(
				&EMHeader,			// buffer
				sizeof EMHeader	// bytes to read
			);

			// Decrypt the header.
			VERIFY( 
				pHeaderEncryptor->StreamDecrypt(
					&EMHeader, 
					sizeof EMHeader
				)
			);


		// Handle old versions here.

		// --------------------------------------------------------------------
		// This is the original way things were set up, with the user data
		// in the encrypted section.  We moved it out so that it would not be 
		// encrypted, to help the end user figure out
		// why they couldnt access the db.
		//
		// Here's the size (needs checking) - this goes at the top of the function.
		/*
			int nOldEMEHSize1 = 266;
				sizeof uShort								// for uEMEncryptedHeaderSize
			+	sizeof uShort								// for ObjectSeed		|
			+	nUserNameLength * sizeof wchar_t		// for UserName		|- all part of previous EMDBHeader
			+	nCompanyLength * sizeof wchar_t		// for Company			|
			+	nContactLength * sizeof wchar_t;		// for Contact			|
			+	6;												// for fill				|
		*/
		// ( We did this before the first ship, so there is no reason to include
		//   it in the code anymore.  Besides, it is not finished - see the TODO.)
		/*
		} else if ( uEMEncryptedHeaderSize == nOldEMEHSize1 )	{

			// Set up the buffer.
			int nBufferSize = nOldEMEHSize1 - sizeof uShort;
			uByte* pBuffer = new uByte[ nBufferSize ];
			uByte* pBufPos;
			
			// Read it.
			datafile.ReadData(
				pBuffer,				// buffer
				nBufferSize			// bytes to read
			);

			// Decrypt the header.
			VERIFY( 
				pHeaderEncryptor->StreamDecrypt(
					pBuffer, 
					nBufferSize
				)
			);

			// Copy out the contents into the new locations.
			memcpy( &EMHeader.ObjectSeed, pBufPos, sizeof uShort );
			pBufPos += sizeof uShort;
			memcpy( UserName, pBufPos, nUserNameLength * sizeof wchar_t );
			pBufPos += nUserNameLength * sizeof wchar_t;
			memcpy( Company, pBufPos, nCompanyLength * sizeof wchar_t );
			pBufPos += nCompanyLength * sizeof wchar_t;
			memcpy( Contact, pBufPos, nContactLength * sizeof wchar_t );

			// Clean up.
			delete pBuffer;

			// Now we need to rewrite the header.  Note that if the total
			// header size had changed (which in this case it HASN'T), we
			// would have to rewrite the entire database file!!!
			// TO DO
			// This has not been tested.
			Close( false );
			Open( true );
			WriteHeader();
			Close( true );
			Open( false );
		
		*/	
		// --------------------------------------------------------------------

		
		} else 
		{
			// This database has an unknown format!
			bReturn = false;
		
		}

	}

	return bReturn;

}


//-------------------------------------------------------------------//
// WritePlainHeader()																//
//-------------------------------------------------------------------//
// This function writes the unencrypted portion of the EMDatabase 
// header.  We generate a new header seed every time we save the header.  
// Then, the encrypted section will be subsequently encrypted with 
// the new header key.
//-------------------------------------------------------------------//
void EMDatabase::WritePlainHeader()
{

	// Write the base class header, which comes first.
	ObjectStream::WritePlainHeader();

	// Write the size of the header.
	datafile.WriteData(
		&uExpectedEMPHSize,			// buffer
		sizeof uExpectedEMPHSize	// bytes to write
	);

	// Generate a non-zero header seed.
	do 
	{ 
		HeaderSeed = get_random_uShort(); 
	} while ( HeaderSeed == 0 );
	
	// MDM	7/6/2000 11:52:30 AM
	// Improving access in demo mode.
	// We now need to write out personal demo databases.

	// Note that end users should NEVER be able to write out demo databases.
	//	#ifdef ACCELERATOR_INHOUSE
	
		// If this is a demo database, we want to zero out 
		// the HeaderSeed.
		if ( DBHeader.Group == DemoGroup )
			HeaderSeed = 0;
	
	// #endif
	
	// Write it out.
	datafile.WriteData(
		&HeaderSeed,			// buffer
		sizeof HeaderSeed		// bytes to write
	);

	// Write out the user data.
	datafile.WriteData(	UserName,	sizeof UserName	);
	datafile.WriteData(	Company,		sizeof Company		);
	datafile.WriteData(	Contact,		sizeof Contact		);

}


//-------------------------------------------------------------------//
// WriteEncryptedHeader()															//
//-------------------------------------------------------------------//
// This function writes the encrypted portion of the EMDatabase header.
//-------------------------------------------------------------------//
void EMDatabase::WriteEncryptedHeader()
{

	// Write the base class header, which comes first.
	ObjectStream::WriteEncryptedHeader();

	// Write the size of the header.
	datafile.WriteData(
		&uExpectedEMEHSize,			// buffer
		sizeof uExpectedEMEHSize	// bytes to write
	);

	// Make a copy of the header; we don't want to trash the original.
	EMDBHeader EncryptedHeader;
	memcpy(
		&EncryptedHeader,
		&EMHeader,
		sizeof EMHeader
	);
	
	// Encrypt the header.
	VERIFY( 
		pHeaderEncryptor->StreamEncrypt(
			&EncryptedHeader, 
			sizeof EncryptedHeader
		)
	);

	// Write the encrypted header block.  We are already at the
	// correct file offset.
	datafile.WriteData(
		&EncryptedHeader,			// buffer
		sizeof EncryptedHeader	// bytes to read
	);

}


//-------------------------------------------------------------------//
// GetObjectKey()																		//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool EMDatabase::GetObjectKey( 
	uByte**	ppKey, 
	uShort*	pKeyBytes 
) {

	bool bReturn = true;
	
	// Allocate the key buffer.
	uHuge* pKey = new uHuge[2];
	
	// Point the param to our buffer.
	*ppKey = (uByte*) pKey;

	// Assign the size.
	*pKeyBytes = sizeof uHuge * 2;


	// Get the key.
	uHuge DBKey[2];

	// The demo databases always use the same key.  It is hardcoded
	// within the program, as there will not be a dongle available to
	// store it.
	const uHuge DefaultKey1	= 0xf3b0e5606a2e4b9a;
	const uHuge DefaultKey2	= 0x9336cb83304bca60;
	
	// If we are dealing with a Demo db, use the hardcoded key.
	if ( DBHeader.Group == DemoGroup ) 
	{
		DBKey[0] = DefaultKey1;
		DBKey[1] = DefaultKey2;
	
	} else 
	{
		// Look up group key.
		bReturn = MainDongle.GetKey( 
			DBHeader.Group,
			(uByte*) &DBKey
		);
		
		if ( bReturn ) 
		{
			// Always add the GROUP_IS_PUBLISHABLE bit when
			// en/decrypting objects, so we use the same key, regardless
			// of whether the user has read or write access.
			DBKey[0] |= GROUP_IS_PUBLISHABLE;
			
			// Get the return code for ObjectSeed.  This is the object mask.
			uHuge ObjectMask = MainDongle.GetReturnCode( EMHeader.ObjectSeed );

			// Mask the group key with the object mask to get the
			// final database key.
			DBKey[0] ^= ObjectMask;
			DBKey[1] ^= ObjectMask;

		}

	}

	// Copy out the results.
	*pKey = DBKey[0];
	*( pKey + 1 ) = DBKey[1];

	return bReturn;

}


//-------------------------------------------------------------------//
// UpdateDatabaseFormat()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool EMDatabase::UpdateDatabaseFormat()
{

	if ( GetDBVersion() < Version( 1, 14 ) )
	{
	
		// We need to unmangle the db header.
		// Here's the old format of our DBID:
		struct{
		
				// DBID
				unsigned int	bPublished:1;	
				unsigned int	DBNumber:31;	
			
					// UserID
					uLong			UserNumber;		
					uLong			Group;			
			
				uLong				DBVersion;				

		} OldID;

		ASSERT( sizeof OldID == 16 );
		ASSERT( sizeof DBHeader.DBID == 8 );

		// Copy what we read in to the header to the old structure.
		memcpy( &OldID, &DBHeader.DBID, sizeof OldID );

		// Now convert.
		DBHeader.DBID.DBNumber		= OldID.DBNumber;
		DBHeader.DBID.UserNumber	= OldID.UserNumber;
		DBHeader.bPublished			= OldID.bPublished;
		DBHeader.DBVersion			= OldID.DBVersion;
		DBHeader.Group					= OldID.Group;
	
	}	

	return ObjectStream::UpdateDatabaseFormat();

}


//-------------------------------------------------------------------//
// UpdateOldObjects()																//
//-------------------------------------------------------------------//
// We need to go through the database and update all old objects.
// We first determine which classes need updating.  Then we
// load and save every object of every class that needs updating.  
// If it works, we return true.
// If after the load of any object, bReadOK is false, the update
// failed, and we immediately return false.  The database will
// not be usable with the current version of the program.
//-------------------------------------------------------------------//
bool EMDatabase::UpdateOldObjects()
{

	bool bResult = true;
	
	// We do not handle databases earlier than this.
	if ( FileVersionLastModified < uhOldestHandledEMFileVersion )
	{
	
		CString strMessage;
		strMessage.Format(
			IDS_DATABASE_TOO_OLD,
			CString( GetDatabaseName() ),
			GetMajorVersion( FileVersionLastModified ),
			GetMinorVersion( FileVersionLastModified )
		);
		DisplayMessage( strMessage );
		return false;
	
	}

	// Check for major database version updates.
	
	// OLD
	// TO DO
	// Take this out once this ASSERT() has had some
	// time to validate things.  Purge bReadOldStyle
	// everywhere it is used.
	ASSERT( GetDBVersion() > Version( 1, 14 ) );
	/*
	// In version 1.14, we updated the structure of ObRefs.
	// We use a global static to specify that we will be
	// reading old style refs in ObjectRead( ObRef ).
	if ( GetDBVersion() < Version( 1, 14 ) )
	{
		bReadOldStyle = true;
	}
	*/
	
	// We may be having to update a published database.  Published databases are 
	// typically only read from, not written to.  Make sure that the file is not
	// read-only, if we plan to update it successfully.
	if ( !VerifyFilesAreWritable() )
		return false;

	CWaitCursor LordIAmSOTiredHowLongCanThisGoON;

	// Set up a progress dialog.
	BaseProgressDlg ProgressDlg;
	
	// Loop through all component objects in our database.  
	// Note that this indirectly handles copying of internal objects, 
	// as they are handled by their parents.
	// Also note that we drop out as soon as we hit a bad result.
	EMComponent* pObject;
	CString strProgress;
	int nCurrentProgress = 0;
	for ( int i = 0; i <= TYPE_TREE_LAST && bResult; i++ ) 
	{
		EMComponentInit *pEMInit = EMComponentInit::ByType( i );

		// Update progress text.
		strProgress.Format( IDS_UPDATING_OLD_OBJECTS, pEMInit->strDescPlural() );
		ProgressDlg.SetText(	strProgress );

		// This object is a base class for other objects?
		// Then don't bother with the update - it will be handled
		// when we get to the final derived class.
		if ( pEMInit->bIsBaseClass() )
			continue;

		// Get an object.
		pObject = NewComponent( (OBJECT_TYPE) i );

		#ifdef ACCELERATOR_INHOUSE
		
			// Verify that the lowest safe version is greater than the current.
			// If you hit this, then the current version is incorrect, and
			// needs to be properly incremented (or something is trashed...).
			if ( pObject->GetLowestSafeVersion() > GetCurrentFileVersion() )
			{
				DisplayMessage( CString( _T("You need to update the current version to incorporate object changes.") ) );
			
				// Drop into debugger if in debug mode.
				ASSERT( false );

			}
		
		#endif

		// Determine if we need to update this class.  If so,
		// we will load every object of this class type next.
		if ( 
				bReadOldStyle																	// Major 1, 14 db change - update ALL objects.
			||	( GetDBVersion() < Version( 1, 33 ) )									// Major db change at 1.33 - update ALL objects.
			||	pObject->GetLowestSafeVersion() > FileVersionLastModified 
		) {

			// Loop through all the objects of this type.
			// Just reading them in will do the version update.
			// Stop if we have a problem.
			pObject->FirstObject();

			while ( pObject->ObjectExists() && bResult ) 
			{

				if ( pObject->bReadOK ) 
				{

					// Save the changes we made when loading.
					pObject->ChangeObject();
					pObject->SaveObject();

					// Load up the next (which does the update).
					pObject->NextObject();

					// Update progress.
					// Even though we don't know how many objects there will be, we can add one
					// to the progress for each object, until we hit our max.
					nCurrentProgress = min( 
						nCurrentProgress + 1, 
						100 * ( i + 1 ) / ( TYPE_TREE_LAST + 1 )
					);
					ProgressDlg.UpdateProgress( nCurrentProgress );

				}
				else
				{

					// The update has failed, because this object was not able to be
					// read successfully.
					bResult = false;

				}
					
			}

			// Update the progress.
			nCurrentProgress = 100 * ( i + 1 ) / ( TYPE_TREE_LAST + 1 );
			ProgressDlg.UpdateProgress( nCurrentProgress );

		}

		delete pObject;
	}

	// For Ver 1, 14
	bReadOldStyle = false;
	
	return bResult;

}


//-------------------------------------------------------------------//
// InitGatherSession()																//
//-------------------------------------------------------------------//
// This clears the data about all previously-gathered objects.
// It clears the GatheredRefs array within this class, and the 
// gather flags maintained in the tree controls.
//
// This function sets the gather data about this database back to
// the state it was in when we first connected to the database.
// We may not want to do this.  We want the gather data
// to be available as long as possible, to reduce duplication
// after multiple gather requests.
// However, if the objects of a database change, the gather data
// will not be updated to reflect the changes.  Initializing a
// new gather session is the only way to ensure that the latest
// gather information is used.
//
//		Init Then Gather					Gather Without Init
//		-------------------------		-----------------------
//		All objects gathered				New SO's of previously gathered
//													objects not gathered
//		Previously gathered		
//			objects duplicated			No duplication of objects
//-------------------------------------------------------------------//
void EMDatabase::InitGatherSession()
{
	int i;

	// Empty the copied objects array.
	for ( i = 0; i < TYPE_TREE_LAST; i ++ )
		GatheredRefs[i].clear();

	// Clear every object's gather checked flag in every tree.
	// This may take a minute or two.
	CWaitCursor Yaaaaaaaaaawwwwwwwwwwwnnnn;
	for ( i = 0; i < OBJECT_LIST_LEN; i++ )
	{
		DBTreeControl* pDBTree = DBTreeControl::GetTreePtr( (OBJECT_TYPE) i );
		POSITION Pos = NULL;
		DBTreeItemData *pData = pDBTree->GetFirstItemListDBData( Pos );

		while ( pData ) 
		{
			// Clear the status for objects of this database.
			if ( pData->ObRef.DBID == GetDBID() )
				pData->GatherClearChecked();

			pData = pDBTree->GetNextItemListDBData( Pos );
		}

	}

}


//-------------------------------------------------------------------//
// FindAndReplaceGatheredRef()													//
//-------------------------------------------------------------------//
// This function determines if the given object has already been
// gathered.  If so, we overwrite the old ref and return true.
// If not, we return false.
//
// NOTE that we MUST have direct access to the actual Ref, so we
// can update it as needed.
//-------------------------------------------------------------------//
bool EMDatabase::FindAndReplaceGatheredRef( 
	ObjectReference&	OldRef,
	OBJECT_TYPE			eType
) {

	// Get the appropriate gathered-object array.
	GatherArray* paRefs = &( GatheredRefs[ eType ] );

	// Loop through the ID array looking for a match.
	GatherArray::iterator theIterator;
	for (
		theIterator = paRefs->begin();
		theIterator != paRefs->end();
		theIterator++
	)
		if ( (*theIterator).OldRef == OldRef )
			break;

	// If we found the match...
	if ( theIterator != paRefs->end() )
	{
		// Stuff the resulting ref.
		OldRef.ObID = (*theIterator).NewID;
		OldRef.DBID = GetDBID();

		return true;

	} else 
		return false;

}


//-------------------------------------------------------------------//
// AddGatheredObject()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
void EMDatabase::AddGatheredObject( 
	TypedRef&	trOld,
	ObjectID		NewID
) {
	GatherPair NewPair;
	NewPair.OldRef = trOld.Ref;
	NewPair.NewID	= NewID;
	GatheredRefs[ trOld.eType ].push_back( NewPair );
}



//-------------------------------------------------------------------//
// DisplayProperties()																//
//-------------------------------------------------------------------//
// This displays a dlg showing read-only properties of the database
// and allowing writable properties to be edited.
//-------------------------------------------------------------------//
void EMDatabase::DisplayProperties( CWnd* pParent )
{

	DBPropertiesDlg PropDlg(
		this,
		pParent
	);

	PropDlg.DoModal();

}


//-------------------------------------------------------------------//
// GetObjectType()																	//
//-------------------------------------------------------------------//
OBJECT_TYPE EMDatabase::GetObjectType( ClassID CID )
{
	ASSERT( (int)CID < OBJECT_LIST_LEN );
	return arObType[ (int) CID ];
}

//-------------------------------------------------------------------//
// GetClassID()																		//
//-------------------------------------------------------------------//
ClassID EMDatabase::GetClassID( OBJECT_TYPE eType )
{
	ASSERT( eType < OBJECT_LIST_LEN );
	return arCID[ (int) eType ];
}

//-------------------------------------------------------------------//
// FillTree()																			//
//-------------------------------------------------------------------//
// This function fills the appropriate Earthmover tree control
// with information about the objects in the database.
//
// This function is called from EMDBArray::Initialize().
// This is also used by the db array to refresh the trees after
// db changes; in that case, bRefresh is true.
//
// This function fills the trees using a cache, if available.  If
// the cache is not available and we are dealing with a working
// database, it rips through every object of the given type,
// filling the tree and then saving a cache.  It returns true
// under either of these circumstances.  It returns false if
// no cache was found within a published database (which cannot
// be modified, and should always have an up-to-date cache).
//-------------------------------------------------------------------//
FillTreeResult EMDatabase::FillTree(
	OBJECT_TYPE			eWhich,
	SplashScreenDlg*	pSplash,
	bool					bCreate,
	bool					bRefresh,
	bool					bSort
) {

	
	FillTreeResult Result = FT_SUCCESS;

	// Make sure we are dealing with an EMComponent object
	// that has an associated tree.
	ASSERT( eWhich >=0 && eWhich <= TYPE_TREE_LAST );

	// Note that bRefresh and bCreate are mutually exclusive.
	ASSERT( ! ( bRefresh && bCreate ) );

	// Create if requested.
	if ( bCreate ) 
		DBTreeControl::CreateTree( eWhich );

	// Now we can get the tree pointer.
	DBTreeControl* pTree = DBTreeControl::GetTreePtr( eWhich );
	ASSERT( pTree );
	
	// When creating, we assume that
	// the caller wants us to turn off painting, and they
	// will handle turning it back on.
	// See EMDBArray::FillTrees() for details.
	if ( bCreate ) 
		pTree->BeginObjectInsertions();					

	// bSort = false means that this was done upstream.
	if ( bSort )
		pTree->BeginObjectInsertions();
	
	// Empty tree if refresh requested.
	if ( bRefresh )
		pTree->DeleteAllItemsAndData();


	///////////////////////////////////////
	// Fill the tree from the database.
	//

	// Open the database for reading.
	Open( false );

	// This will fill the tree based on the cache, if available.
	DBTreeCache TreeFiller(
		eWhich,
		this
	);

	if ( TreeFiller.ObjectExists() && !TreeFiller.bReadOK ) {

		// Major database problem.
		Result = FT_BAD_DATABASE;
		Close( false );
	
	} else if ( TreeFiller.ObjectExists() && TreeFiller.bUpToDate ) {

		// We did it, now close the db.
		Close( false );

	// Out-of-date (or no) cache, fill the tree object-by-object,
	// and then update the cache.
	} else {

		// The cache is out of date!  Bummer.
		// Warn the user that they may be in for a few minutes wait.
		if ( pSplash ) 
		{
			CString strMessage;
			strMessage.Format( 
				IDS_FILL_TREE_WITH_NO_CACHE, 				
				EMComponentInit::ByType( eWhich )->strDescPlural()
				// CString( TreeFiller.GetDatabasePtr()->GetDatabaseName() )
			);
			pSplash->AddString( strMessage );
		}

		// We should only have to do this for working databases.
		// If this is published and we had to do it, warn in the
		// return code.
		if ( IsPublished() )
			Result = FT_PUBLISHED_CACHE_OLD;

		// Since we'll need to write a cache, change from read to write open.
		Close( false );
		Open( true );

		EMComponent* pObject = NewComponent( eWhich );
		ASSERT( pObject != NULL );

		// Find the first object.
		pObject->FirstObject();

		// Add the items.
		while ( pObject->ObjectExists() ) {

			pTree->AddObject( 
				pObject, 
				false,				//	bForceHighlight
				false					// bSortTree
			);

			// Save the object here in case any errors were found (and corrected)
			// within EMComponent::Read_BaseData().  If this is true then the
			// Persistent::changed flag will be set and this call below will handle it.
			pObject->SaveObject();

			// See if we can get another.
			pObject->NextObject();

			// Process any other messages while we're working away.
			theApp.PurgeMessages();

		}

		// Delete our load object.
		delete pObject;

		// Now ensure that the cache gets saved.
		if ( TreeFiller.ObjectExists() )
			TreeFiller.ChangeObject();
		else
			TreeFiller.AddObject();

		// Now save the object.  This will use the tree
		// to create a new cache.
		// We force the save here so we can close the database
		// before this object goes out of scope.
		if ( !TreeFiller.SaveObject() )
			Result = FT_BAD_DATABASE;

		// Close up.
		Close( true );

	}

	// Sort and repaint.
	if ( bSort )
		pTree->EndObjectInsertions();

	return Result;

}


//-------------------------------------------------------------------//
// Compact()																			//
//-------------------------------------------------------------------//
// This function compacts the database.  It does so by rewriting
// the database.  We scan for all nodes that contain valid object
// header data.  We then transfer these nodes to a new copy of the
// database.  Once we have scanned the whole file, we delete the old
// file and its index, and reindex the new.
// TO DO
// (If you're feeling ambitious) Keep an array of all written
// node numbers.  Then, when writing nodes after the first in the
// object node list, check against the array to look for "crossed
// objects".  If found, write the objects, but warn in the log/msg.
//-------------------------------------------------------------------//
bool EMDatabase::Compact(
	BaseProgressDlg*	pProgressDlg,
	int					nStartProgress,
	int					nEndProgress,
	bool					bLog
) {

	CWaitCursor HoldOn;

	// There will be two stages of compacting.  The first is where we actually
	// defrag the db, and the second is where we recreate the index file.  This
	// var determines how much progress we assign to the point between the two
	// stages.
	int nCompactEndProgress = nStartProgress + ( nEndProgress - nStartProgress ) / 2;

	// Give feedback here as we set up to defrag...
	SetProgressTitle(
		pProgressDlg,
		IDS_DB_CREATING_TEMP_DB
	);

	// Create a log file if requested.
	fstream logfile;
	CString LogBuffer;
	CString DisplayName;
	if ( bLog ) {

		LogBuffer = *GetFilename();
		LogBuffer = LogBuffer.SpanExcluding( _T(".") );
		LogBuffer += _T(".DefragLog");
		logfile.open(
			LPCSTR( LogBuffer ),
			ios::out | ios_base::trunc
		);

		// Start log.
		LogBuffer = "Defragmentation initialized.\n\n";
		logfile.write(
			LPCSTR( LogBuffer ),
			LogBuffer.GetLength()
		);

		// Write database ID.
		GetDatabaseHeader( &LogBuffer );
		logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );

		// Write header for objects.
		LogBuffer = "ClassID: OldNode1-NewNode1, OldNode2-OldNode2, etc.\n";
		logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
		LogBuffer = "-----------------\n";
		logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );

	}

	// Open the data file for reading.  We also need the header.
	datafile.Open( false );
	datafile.ReadHeader();

	// Create a new database that will eventually replace the original.
	// We want to preserve the seed, since we are doing a raw node copy.
	CString strTempDBName;
	pEMDBArray->GetTempDBName( &strTempDBName );
	EMDatabase NewDB( &strTempDBName );
	if (
		NewDB.Initialize(
			false,									//	bFileShouldExist,
			&wstring( GetDatabaseName() ),	//	pwsDatabaseName,
			&GetDBID(),								// pDBID,
			DBHeader.Group,						// Group,
			DBHeader.DBVersion,					// DBVersion,
			IsPublished(),							// bPublished,
			&EMHeader.ObjectSeed					// pObjectSeed
		) == DB_INIT_SUCCESS
	) {

		NewDB.Open( true );

		// Ready to scan.
		SetProgressTitle(
			pProgressDlg,
			IDS_DB_DEFRAGGING
		);

		// We need a place to cache the object headers.
		ObjectHeader objhdr;

		// ------- scan object nodes
		NodeNbr end = datafile.HighestNode();
		long lProgress;
		for ( NodeNbr nd = 1; nd <= end; nd++ ) {

			// --- read the object header for this node
			Node* pTemp = new Node( &datafile, nd );
			pTemp->GetFromBuffer( &objhdr, sizeof ObjectHeader );

			// Check for first node of object.  bFirstNode
			// will be true.  If we have a first node and
			// it has not been deleted, then we should
			// process the object.
			if ( objhdr.bFirstNode ) {

				if ( objhdr.classid == -1 ) {

					// Log the deleted node.
					if ( bLog ) {

						LogBuffer.Format(
							"Skipped deleted object at node %i.\n",
							nd
						);
						logfile.write(
							LPCTSTR( LogBuffer ),
							LogBuffer.GetLength()
						);

					}

				} else if ( objhdr.classid >= nClassCount ) {

					// Log the invalid node.
					if ( bLog ) {

						LogBuffer.Format(
							"Skipped invalid object, class %i.\n",
							objhdr.classid
						);
						logfile.write(
							LPCTSTR( LogBuffer ),
							LogBuffer.GetLength()
						);

					}

				} else {

					// Write the chain of nodes to the new data file.

					Node* pNewNode;
					Node* pNewNextNode;

					// Get the first one.
					pNewNode = new Node(
						&NewDB.datafile,
						0,					// NewNodeNbr, 0 means get a new one.
						NM_CREATE
					);

					// Copy over all the contents
					// (we'll need to update the next node number).
					memcpy( pNewNode->pBuffer, pTemp->pBuffer, datafile.GetNodeLength() );

					// Log object start.
					if ( bLog ) {

						LogBuffer.Format(
							"%3i: %3i>%-3i",
							objhdr.classid,
							nd,
							pNewNode->GetNodeNbr()
						);
						logfile.write(
							LPCTSTR( LogBuffer ),
							LogBuffer.GetLength()
						);

					}

					// Get the next node nbr in the source db, if available.
					NodeNbr nx = pTemp->NextNode();

					// Loop through the list of nodes.
					while ( nx ) {

						// Get the next source node.
						delete pTemp;
						pTemp = new Node(
							&datafile,
							nx
						);

						// Get a new next node in the target db.
						pNewNextNode = new Node(
							&NewDB.datafile,
							0,					// NewNodeNbr, 0 means get a new one.
							NM_CREATE
						);

						// Set the next node nbr of the previous target node.
						pNewNode->SetNextNode( pNewNextNode->GetNodeNbr() );

						// Delete (and save) the previous node.
						delete pNewNode;

						// Set the current node pointer to the next.
						pNewNode = pNewNextNode;

						// Copy the source node contents to the target node.
						memcpy( pNewNode->pBuffer, pTemp->pBuffer, datafile.GetNodeLength() );

						// Log this node.
						if ( bLog ) {

							LogBuffer.Format(
								",%3i>%-3i",
								nx,
								pNewNode->GetNodeNbr()
							);
							logfile.write(
								LPCTSTR( LogBuffer ),
								LogBuffer.GetLength()
							);

						}

						// Get the next node nbr of the source node.
						nx = pTemp->NextNode();

					}

					// Delete (and save) the last new node.
					pNewNode->SetNextNode( 0 );
					delete pNewNode;

					// Skip to the next log line.
					if ( bLog )
						logfile.write( _T( "\n" ), 1 );

				}	// object first node test

			} // first node test

			// Clean up the temp node.
			delete pTemp;

   		// Update progress meter.
			lProgress = nStartProgress + ( nCompactEndProgress - nStartProgress ) * ( nd ) / end;
			pProgressDlg->UpdateProgress( (int) lProgress );

		}

		// Close the database files.
		datafile.Close( false );
		NewDB.Close( true );

		// Close the log file.
		if ( bLog )
			logfile.close();

		// Delete the new index file.
		remove( LPCTSTR( *NewDB.indexfile.GetFilename() ) );

		// Back up the data file.
		Backup();

		// Replace the original data file with the new file.
		remove( LPCTSTR( *datafile.GetFilename() ) );
		rename(
			LPCTSTR( *NewDB.datafile.GetFilename() ),
			LPCTSTR( *datafile.GetFilename() )
		);

		// Update the progress text.
		SetProgressTitle(
			pProgressDlg,
			IDS_DB_REPAIRING
		);

		// Reindex the new file.
		RebuildIndexFile(
			&( pProgressDlg->ProgressMeter ),
			nCompactEndProgress,
			nEndProgress,
			bLog
		);

		return true;

	} else {

		datafile.Close( false );
		DisplayMessage( IDM_DB_COMPACT_FAILED );
		return false;

	}

}


//-------------------------------------------------------------------//
// ClearCache()																		//
//-------------------------------------------------------------------//
// This clears the database's tree cache data.  The next time the
// database is connected to, a full database read will take place,
// and the caches will be refreshed.
//
// This function is called during unpublishing.
//-------------------------------------------------------------------//
void EMDatabase::ClearCache()
{
	// We can only do this for non-published databases, since it
	// will require write access.  ClearCache() should have been removed
	// from the menu if the db is published, ASSERT here to make sure.
	ASSERT( !IsPublished() );

	// Loop through all the object types that have associated
	// db trees, outdating the cache status.
	Open( true );
	for ( int i = 0; i <= TYPE_TREE_LAST; i++ )
		DBTreeCache BadCache(
			(OBJECT_TYPE) i,
			this,
			false,			// bTestValidity
			true				// bInvalidate
		);

	Close( true );
}


//-------------------------------------------------------------------//
// UpdateCaches()																		//
//-------------------------------------------------------------------//
// This updates the caches for this db.  As long as we are currently 
// connected to this db, the tree should be up to date and available 
// for caching.  We typically call this function for each db just 
// before exiting the program.  The progress dlg is optional; if not 
// provided, it is not used.
//
// This should not be called for published databases, except in the
// process of creating a published db from a working.
//-------------------------------------------------------------------//
void EMDatabase::UpdateCaches(
	BaseProgressDlg*	pProgressDlg,
	int					nStartProgress,
	int					nEndProgress
) {
	CWaitCursor HoldOn;

	Open( true );

	if ( pProgressDlg )
		SetProgressTitle(
			pProgressDlg,
			IDS_DB_CACHING
		);

	// Update all out-of-date caches.
	bool bNeededUpdate = false;
	for ( int j = 0; j <= TYPE_TREE_LAST; j++ ) 
	{
		// We want to open the cache object without loading
		// the tree, just testing its validity.  Then, if
		// not valid, we want to save it.
		DBTreeCache Cache(
			(OBJECT_TYPE) j,
			this,
			true					// bTestValidity
		);

		// If the cache object exists, was read in properly, but
		// is not up to date, update it.  Don't touch caches 
		// that have bReadOK set to false, as the decryption failed.
		if ( Cache.ObjectExists() ) 
		{
			ASSERT( Cache.bReadOK );							// Funky database - see DBTreeCache::Read()
			if ( Cache.bReadOK && !Cache.bUpToDate ) 
			{
				Cache.ChangeObject();
				Cache.SaveObject();
				bNeededUpdate = true;
			}
		} else 
		{
			Cache.AddObject();
			Cache.SaveObject();
			bNeededUpdate = true;
		}

		// Update progress.
		if ( pProgressDlg )
			pProgressDlg->UpdateProgress(
				nStartProgress + nEndProgress * ( j + 1 ) / ( TYPE_TREE_LAST + 1 )
			);

	}

	// Update associations, too.
	if ( bNeededUpdate )
		UpdateAssociations( pProgressDlg );
	
	// Now close the db.
	Close( true );
}


//-------------------------------------------------------------------//
// UpdateAssociations()																//
//-------------------------------------------------------------------//
// This does what it says.
//
// It should only be called for working databases or temporary
// databases in the proces of being published.  Published
// databases have their associations finalized when they
// are published.
//-------------------------------------------------------------------//
int EMDatabase::UpdateAssociations(
	BaseProgressDlg*	pProgressDlg,
	int					nStartProgress,
	int					nEndProgress,
	bool					bVerifyCompleteness
) {

	CWaitCursor HoldOn;

	Open( true );

	// Get associated database data via an object.
	DBAssociations DBData( 
		this
	);

	int nAssocResult = DBData.Extract(
		pProgressDlg,
		nStartProgress,
		nEndProgress
	);

	Close( true );

	return nAssocResult;

}


//-------------------------------------------------------------------//
// Publish()																			//
//-------------------------------------------------------------------//
// This function publishes and unpublishes the database.
// Databases that are published are placed in the Databases\Published 
// dir.  Databases that are unpublished are placed in the Databases dir.
//
// TO DO
// Note that Unpublishing is incomplete!!!  Don't bother with it, it
// should be removed, as it has been replaced by Copy, then Gather
// steps.
//-------------------------------------------------------------------//
void EMDatabase::Publish(
	CWnd*	pParent,
	bool	bUnpublishing
) {

	// If this fails, we want to delete the published files.
	bool bSuccessful = false;

	// Make sure this is the correct type of database.
	if ( 
			( !IsPublished() && !bUnpublishing )
		|| (  IsPublished() &&  bUnpublishing )
	) {
		// Only allow publishing if our current version can handle it.
		if (
				FileVersionLastModified <= GetCurrentFileVersion()
			&&	DBHeader.FileVersionCreated <= GetCurrentFileVersion()
		) {
			
			// Get the standard database directory.
			CString strPubDir;
			pEMDBArray->GetDatabasePath( &strPubDir );

			// Use the existing db filename to start.
			CString strFilename;
			indexfile.GetFilenameNoPath( &strFilename );
			StripExtension( &strFilename );

			uLong NewVersion = 0;
			if ( !bUnpublishing )
			{
				// Get a string for the pending version number.
				// Add a version number to the current filename.
				NewVersion = DBHeader.DBVersion + 1;
				CString strVersion;
				strVersion.Format(
					_T(" v%d"),
					NewVersion
				);
				strFilename += strVersion;

				// Use the Published directory.
				// Make sure it exists.
				strPubDir += _T("Published\\");
				_tmkdir( LPCTSTR( strPubDir ) );

			} else
			{
				// Strip any published version number from the filename.
				
				// Look for the " v###" suffix.
				int nLength = strFilename.GetLength();
				int nIndex = strFilename.ReverseFind( _T('v') );
				if ( nIndex != -1 )
				{
					nIndex++;
					while ( 
							nIndex < nLength
						&&	( strFilename.Right( nLength - nIndex ).FindOneOf( _T("0123456789") ) == 0 )
					)
						nIndex++;
				}

				// Remove if found.
				if ( nIndex == nLength )
				{
					nIndex = strFilename.ReverseFind( _T('v') );
					if ( nIndex > 0 && strFilename[ nIndex - 1 ] == _T(' ') )
						nIndex--;
					strFilename = strFilename.Left( nIndex );
				}

			}

			// If either db file already exists, warn the user and fail.
			{
				CString strTest = strPubDir + strFilename;
				strTest += tIndexExt;
				bool bPubExists = ( _taccess( LPCTSTR( strTest ), 0 ) != -1 );
				strTest = strPubDir + strFilename;
				strTest += tPublishedDBExt;
				bPubExists |= ( _taccess( LPCTSTR( strTest ), 0 ) != -1 );
				if ( bPubExists ) 
				{				
					CString strExistsMsg;
					strExistsMsg.Format(
						IDM_DB_PUBLISHED_FILE_EXISTS,
						strPubDir + strFilename
					);
					DisplayMessage( strExistsMsg );
					return;				
				}
			}

			bool bProceed = true;

			// Get the target group.
			uLong TargetGroup;
			if ( bUnpublishing )

				// Use the user's personal group, i.e., the user number.
				TargetGroup = MainDongle.GetUserNumber();

			else

				// Ask the dongle to display a dialog that lets the user select the target
				// group from all the groups to which they have publishing rights.
				bProceed = MainDongle.SelectPublishGroup(
					&TargetGroup,
					pParent
				);
			
			if ( bProceed && VerifyContents() )
			{
				CWaitCursor HoldYourHorses;
				
				// Create a progress dlg.
				BaseProgressDlg ProgressDlg(
					pParent,
					bUnpublishing? IDS_DB_UNPUBLISH_STARTED : IDS_DB_PUBLISH_STARTED,
					IDD_PROGRESS_PUBLISH
				);

				bool bSuccess = true;

				// Backup before we get started.
				Backup();

				// Create a new database.
				//				
				// For publishing, we use:
				//
				//		the group selected above,
				//		the current user's number,
				//		the same database number,
				//		the next version number,
				//
				// For unpublishing, we use:
				//
				//		the user's personal group,
				//		the current user's number,
				//		the NEXT database number,
				//		version number 0,
				//
				EMDatabase PublishedDB(
					&( 
							strPubDir 
						+	strFilename 
						+	( bUnpublishing? tWorkingDBExt : tPublishedDBExt )
					)
				);

				wstring wsCurrentName = GetDatabaseName();
				
				// Get the target DBID.
				DatabaseID TargetDBID;
				if ( bUnpublishing )

					// We want to make a new working database ID.
					TargetDBID = MainDongle.GetNewDBID();
				
				else
				{				
					// The published database ID should match the
					// working DBID.  The only difference will be
					// the DBVersion.
					// Make sure the publisher still matches the
					// original creator of the database.
					ASSERT( DBHeader.DBID.UserNumber == MainDongle.GetUserNumber() );
					TargetDBID.UserNumber = DBHeader.DBID.UserNumber;
					TargetDBID.DBNumber = DBHeader.DBID.DBNumber;
				}

				if ( 
					PublishedDB.Initialize(
						false,									//	bFileShouldExist,
						&wsCurrentName,						//	pwsDatabaseName,
						&TargetDBID,							// DBID
						TargetGroup,							// Group
						NewVersion,								// DBVersion
						!bUnpublishing							// bPublished
					) == DB_INIT_SUCCESS 
				) {

					////////////////////////////////////////////////////////////////////////
					// PUBLISH STEPS
					// Now that we have a new published db, we want to fill it!  We first
					// add caching data.  Then we loop through all the component objects in
					// the working database, determine db associations of each, check completeness,
					// and, if all tests pass, add each to the published database.  Errors
					// are reported after all objects are scanned.
					// Note that we will be refreshing the calcs of objects in the
					// working db,so it must be opened in writable mode.
					////////////////////////////////////////////////////////////////////////
					PublishedDB.Open( true );
					Open( true );

					// We want to update associations in the published db.  We do this through
					// an associations object.
					DBAssociations Associations( 
						&PublishedDB
					);
					if ( !bUnpublishing )
						Associations.Start();

					// DEBUG
					// Get the working and published db id's for later.
					// UserID WorkingID		= DBHeader.DBID.User;
					// UserID PublishedID	= Associations.GetRef().DBID.User;

					// Set up to compile completeness data as well.
					bool bDBComplete = true;
					CArray< OBJECT_TYPE, OBJECT_TYPE > TypesNotFound;

					// Loop through all component objects in our database.  Note
					// that this indirectly handles copying of internal objects, as they are
					// handled by their parents.  Internal objects will not be included in
					// the associations check; this is OK because internal component objects
					// cannot (currently) have subobjects in other databases.
					// TO DO
					// Update as needed if this situation changes.
					int nStartProgress = 4;
					int nEndProgress = 97;
					int nCurrentProgress = nStartProgress;
					CString strProgress1( (LPSTR) ( bUnpublishing? IDS_DB_UNPUBLISHING_OBJECT : IDS_DB_PUBLISHING_OBJECT ) );
					EMComponent* pObject;
					for ( int i = 0; i <= TYPE_TREE_LAST; i++ ) 
					{
						// Update progress text.
						ProgressDlg.SetText(	
								strProgress1 
							+	EMComponentInit::ByType( i )->strDescPlural() 
							+	_T("...")
						);

						// Loop through all the objects of this type in the current database.
						pObject = NewComponent( (OBJECT_TYPE) i );
						pObject->FirstObject();

						while ( pObject->ObjectExists() ) 
						{
							// Set the name of the object in the progress dialog.  We want
							// to know something is happening.
							CString strName;
							pObject->GetDisplayName( &strName );
							ProgressDlg.GetDlgItem( IDC_PUBLISH_OBJECT_NAME )->SetWindowText( strName );

							// Process associations for published db's.
							bool bSkipPrivate = false;
							if ( !bUnpublishing )
								bSuccess &= Associations.AddAssociations( pObject );

							// If this is an unpublish, we do not want to copy private objects
							// directly, but rather copy them over with their parents, to keep
							// them attached.
							else
								bSkipPrivate = pObject->bIsPrivate();
							
							// If there have not yet been any errors and we don't want to skip...
							if ( bSuccess && !bSkipPrivate ) 
							{
								// Get a new temp object in the published db.
								EMComponent* pPubObject = PublishedDB.NewComponent( (OBJECT_TYPE) i );
								
								// Copy the "guts" of the object.
								// For publishing:
								//		Note that we do not want to copy private objects.  We are looping
								//		through every object type, and they will get copied over individually.
								//		The connection between the parent and the private object is maintained
								//		because the ObjectID's are all maintained as is (see below).
								// For unpublishing:
								//		Note that we want to copy private objects.  They must remain tied
								//		to their parent.  We therefore skip direct copies of private objects
								//		during unpublishing.
								pPubObject->Copy(
									pObject,			// Source EMComponent pointer.         
									bUnpublishing, // Duplicate private objects in the DB?
									false 			// Copy SubObj backup arrays too?      
								);

								
								// DEBUG
								/*
								// The following code put in here so that we can publish
								// our little hacked up debug DBs.  The overhead for it
								// is nill since this condition will always be false in
								// the real world.
								if ( ! ( WorkingID == PublishedID ) )
								{
									
									RefScanData Data(
										false,				//	bSubobjectsOnly
										false,				//	bIncludeBlankRefs
										true					//	bIncludeUnavailableRefs
									);
										
									while ( pPubObject->GetNextRef( Data ) )
									{
										if ( Data.pRef->DBID.User == WorkingID )
											Data.pRef->DBID.User = PublishedID;
									}
								}
								*/

								
								// We want to maintain the same ObjectID, since other objects
								// in our database will refer to us by it.
								//
								// IMPORTANT NOTE:  By doing this, we are bypassing the standard method
								// of generating ObjectID's.  The HighestObjectID values in the published
								// database will not be valid.  This is OK, since we won't be adding
								// objects to it.  If we need to modify a published db in-house, the
								// database should be defragmented as a first step, so the HighestObjectID
								// values are updated correctly.
								//
								pPubObject->SetID( pObject->GetID() );
								
								// Add the object to the published database.
								pPubObject->AddObject();
								pPubObject->SaveObject();
								
								// TO DO
								// We CANNOT do this here because Gather() relies on the gathered database 
								// being connected so that the tree controls can be used to get
								// private status.  We will have to address this issue after we get the
								// whole "share/unshare" connection mechanism working, that automatically
								// switches the connection from/to published/working db's.
								//
								// Gather up any related subobjects in working databases.
								// During publishing, the DBID's on the source and dest db
								// match, so no gathering occurs from source to dest.
								// During unpublishing, the source db is published, so
								// no gathering occurs from source to dest there either.
								// pPubObject->Gather();
								
								delete pPubObject;

							}

							pObject->NextObject();

							// Update progress.
							// Even though we don't know how many objects there will be, we can add one
							// to the progress for each object, until we hit our max.
							nCurrentProgress = min( 
								nCurrentProgress + 1, 
								nStartProgress + ( nEndProgress - nStartProgress ) * ( i + 1 ) / ( TYPE_TREE_LAST + 1 )
							);
							ProgressDlg.UpdateProgress( nCurrentProgress );
							
						}

						// Update the progress.
						nCurrentProgress = 
								nStartProgress 
							+	( nEndProgress - nStartProgress ) * ( i + 1 ) / ( TYPE_TREE_LAST + 1 );
						ProgressDlg.UpdateProgress( nCurrentProgress );

						delete pObject;

					}

					// Wrap up associations and report any errors.
					if ( !bUnpublishing )
						Associations.Finish();

					// If we succeeded...
					if ( bSuccess ) 
					{
						PublishedDB.Close( true );

						if ( bUnpublishing )
						{
							// We now need to defragment the db, to patch up
							// HighestObjectID values.
							CWaitCursor GoDogGo;

							// Set up a progress dlg.
							BaseProgressDlg ProgressDlg;

							// Compact to reset HighestObjectID values.
							PublishedDB.Compact( &ProgressDlg, 0, 50 );

							// We need to refresh the cache the next time we
							// open this database.  The cache refresh below
							// only works for working->published, since they
							// maintain the same DBID.
							PublishedDB.ClearCache();
							
							// Clean up.
							ProgressDlg.DestroyWindow();

						} else
						{
							// Update the published db's cache.  Note that the trees will be
							// used to build the cache.  We will cache objects in the trees 
							// that match the published db's DBID.  The tree will contain all 
							// our objects, because it has been filled with the objects in 
							// the working database, and the pub and working DBID's match.
							PublishedDB.UpdateCaches(
								&ProgressDlg,
								97,
								100
							);
							
							// Update the working database's "last published" version.
							DBHeader.DBVersion = NewVersion;
							WriteHeader();
						}
					
					// Even if we failed, we still need to close the target.
					} else
						PublishedDB.Close( true );
					
					// Now close the source.
					Close( true );

				} else 
				{
					DisplayMessage( IDM_DB_PUBLISH_INIT_FAILED );
				}

				if ( !bSuccess ) 
				{
					// Kill the published files if we did not succeed.
					PublishedDB.DeleteFiles();
				}

				// Clean up.
				ProgressDlg.DestroyWindow();

			}

		} else 
		{
			DisplayMessage( IDM_DB_PUBLISH_INVALID_VERSION );
		}

	} else 
	{
		DisplayMessage( IDM_DB_ALREADY_PUBLISHED );
	}
}


//-------------------------------------------------------------------//
// Gather()																				//
//-------------------------------------------------------------------//
void EMDatabase::Gather( 
	CWnd*			pParent,
	bool			bIncludePublished,
	EMDatabase* pSourceDB 
) {
	CWaitCursor CircleTheWagons;
	
	// Create a progress dlg.
	BaseProgressDlg ProgressDlg(
		pParent,
		0,
		IDD_PROGRESS_PUBLISH
	);
	ProgressDlg.SetMaxPos( TYPE_TREE_LAST + 1 );

	// Loop through all objects and gather them as requested.
	CString strProgress1( (LPSTR) IDS_DB_GATHERING_OBJECT );
	EMComponent* pObject;
	for ( int i = 0; i <= TYPE_TREE_LAST; i++ ) 
	{
		// Update progress text.
		ProgressDlg.SetText(	
				strProgress1 
			+	EMComponentInit::ByType( i )->strDescPlural() 
			+	_T("...")
		);

		// Loop through all the objects of this type in the current database.
		pObject = NewComponent( (OBJECT_TYPE) i );
		pObject->FirstObject();

		while ( pObject->ObjectExists() ) 
		{
			// Set the name of the object in the progress dialog.  We want
			// to know something is happening.
			CString strName;
			pObject->GetDisplayName( &strName );
			ProgressDlg.GetDlgItem( IDC_PUBLISH_OBJECT_NAME )->SetWindowText( strName );

			pObject->Gather( bIncludePublished, 0, pSourceDB );

			pObject->NextObject();
		}

		delete pObject;

		// Update the progress meter.
		ProgressDlg.UpdateProgress( i + 1 );
	}
}


//-------------------------------------------------------------------//
// CopyAll()																			//
//-------------------------------------------------------------------//
// This function loops through all the objects in the specified
// source DB, copying each to this database.
//-------------------------------------------------------------------//
void EMDatabase::CopyAll( 
	CWnd*			pParent,
	EMDatabase* pSourceDB,
	bool			bGather
) {
	CWaitCursor HereComeDaJudge;
	
	// Create a log file.
	fstream logfile;
	CString strBuffer;

	strBuffer = *GetFilename();
	StripExtension( &strBuffer );
	strBuffer += _T(".GatherLog");
	logfile.open(
		LPCSTR( strBuffer ),
		ios::out | ios_base::trunc
	);

	// Create a progress dlg.
	BaseProgressDlg ProgressDlg(
		pParent,
		0,
		IDD_PROGRESS_PUBLISH
	);
	ProgressDlg.SetMaxPos( TYPE_TREE_LAST + 1 );

	// Loop through all objects and copy them as needed.
	// NOTE: for gathering to succeed, we must go from the 
	// highest to lowest object type.  This forces proper
	// gathering of public subobjects.
	CString strProgress1( (LPSTR) ( bGather? IDS_DB_COPYING_AND_GATHERING_OBJECT : IDS_DB_COPYING_OBJECT ) );
	EMComponent* pSourceObject;
	for ( int i = TYPE_TREE_LAST; i >= 0; i-- ) 
	{
		// Update progress text.
		ProgressDlg.SetText(	
				strProgress1 
			+	EMComponentInit::ByType( i )->strDescPlural() 
			+	_T("...")
		);

		// Loop through all the objects of this type in the source database.
		pSourceObject = pSourceDB->NewComponent( (OBJECT_TYPE) i );
		pSourceObject->FirstObject();

		while ( pSourceObject->ObjectExists() ) 
		{
			// We skip private objects, they will be copied by their parents.
			// This information is maintained by the tree.  Look up the tree data now.
			DBTreeItemData* pTreeData = 
				DBTreeControl::GetTreePtr( (OBJECT_TYPE) i )->
					GetItemDBData( 
						&pSourceObject->GetRef() 
					);
			if ( 
					pTreeData 
				&&	!pTreeData->bIsPrivate()
			) {

				// Set the name of the object in the progress dialog.  We want
				// to know something is happening.
				pSourceObject->GetDisplayName( &strBuffer );
				ProgressDlg.GetDlgItem( IDC_PUBLISH_OBJECT_NAME )->SetWindowText( strBuffer );
		
				// Do we want to maintain gather information?  This
				// allows us to replace Refs in subsequent objects gathered
				// to this database.  Use this flag to reduce duplication 
				// when desired.
				if ( bGather )
				{
					// We may or may not need to copy this object, depending on 
					// whether it is already in the gather array.  CopyAndGather()
					// does this work for us, copying and gathering only as needed.
					CopyAndGather(
						pSourceObject->GetTypedRef(),
						false,								// bIncludePublished,
						pSourceDB, 							// pGatheringFromDB, 
						&logfile
					);

				} else
				{
					// Copy it over!
					DuplicateDBObject(
						pSourceObject->GetTypedRef(),
						true									// bDestIsInConnectedDB
					);
				}

			} else if ( pTreeData )
			{
				// Log that we skipped this object because it is private.
				CString strName;
				pSourceObject->GetDisplayName( &strName );
				strBuffer.Format(
					_T("Skipping private %s [%s]...\n"),
					EMComponentInit::ByType( i )->strDescSingular(),
					strName
				);
				logfile.write( LPCSTR( strBuffer ), strBuffer.GetLength() );

			} else
			{
				// We didn't find it in the tree?
				CString strName;
				pSourceObject->GetDisplayName( &strName );
				strBuffer.Format(
					_T("WARNING: Not in tree!  Skipping %s [%s]...\n"),
					strName
				);
				logfile.write( LPCSTR( strBuffer ), strBuffer.GetLength() );
			
			}

			pSourceObject->NextObject();
		
		}

		delete pSourceObject;

		// Update the progress meter.
		ProgressDlg.UpdateProgress( ( TYPE_TREE_LAST - i ) + 1 );
	}
}


//-------------------------------------------------------------------//
// CopyAndGather()																	//
//-------------------------------------------------------------------//
// This function takes the source object, identified by Type and Ref,
// and determines if it has been previously gathered to this db.
// If so, we return.  If not, we make a new copy of the object 
// in our database, and then gather it up.
// 
// This is used by CopyAll().
//-------------------------------------------------------------------//
bool EMDatabase::CopyAndGather(
	TypedRef&			trSource,
	bool					bIncludePublished,
	EMDatabase*			pGatheringFromDB,
	fstream*				pLogFile,
	int					nLogIndents
) {
	bool bGatherSucceeded = true;

	// Look for the ref in the previously gathered array.
	// Note that we call this function for a "top-level"
	// object, not a subobject, so there is no Ref to
	// update.  We don't need to provide the ACTUAL ref here.
	if ( !FindAndReplaceGatheredRef( trSource.Ref, trSource.eType ) ) 
	{
		// We don't have this object yet, so copy it over!
		ObjectReference NewRef = DuplicateDBObject(
			trSource,
			true							// bDestIsInConnectedDB
		);

		// Add a new array entry.
		AddGatheredObject(
			trSource,
			NewRef.ObID
		);

		// Now get the object and gather it up.
		EMComponent* pNewObject = NewComponent( trSource.eType, &NewRef );
		bGatherSucceeded = pNewObject->Gather(
			bIncludePublished,
			0,
			pGatheringFromDB,
			0,
			pLogFile,
			nLogIndents
		);
	}

	return bGatherSucceeded;

}


//-------------------------------------------------------------------//
// DuplicateDBObject()																//
//-------------------------------------------------------------------//
// Given the type and reference of an object, this function will
// create a new one in the database that is an exact copy of that
// object.  The appropriate tree is updated with the new object's
// information, if requested.  
//
// Returns the ObjectReference of the new object.
//-------------------------------------------------------------------//
ObjectReference EMDatabase::DuplicateDBObject(
	TypedRef			&trSrc,
	bool				bUpdateTree,
	SwapRefArray	*parSwap			// Used internally by EMComponent to update linked refs.
) {
	ObjectReference NewRef;

	// If the source reference is blank then don't
	// add anything.  Just return a blank ref.
	if ( !trSrc.Ref.IsBlank() )
	{
		// Get the source object from the database.
		EMComponent *pSrcEMC = pEMDBArray->NewComponent( trSrc );

		// Get a brand new empty object in our database to mangle.
		EMComponent *pNewEMC = NewComponent( trSrc.eType );

		// Copy the source object.
		pNewEMC->Copy(
			pSrcEMC,		// Source EMComponent pointer.
			true, 		// Duplicate private objects in the DB?
			false, 		// Copy SubObj backup arrays too?
			parSwap		// Used internally by EMComponent to update linked refs.
		);

		// Change and save it out to the DB.
		NewRef = pNewEMC->ValidateAndSave( true, false, NULL, bUpdateTree );

		// Wipe our butts.
		delete pSrcEMC;
		delete pNewEMC;

	}

	return NewRef;
}


//-------------------------------------------------------------------//
// CreateReferenceLog()																//
//-------------------------------------------------------------------//
// This function creates a log of all the objects in the database,
// along with their subobjects and all associated references.
//-------------------------------------------------------------------//
bool EMDatabase::CreateReferenceLog(
	CProgressCtrl* pProgressMeter,
	CString strLogName
) {

	if ( strLogName == _T("") ) {
		strLogName = *GetFilename();
		StripExtension( &strLogName );
		strLogName += _T(".RefLog");
	}

	bool bReturn = true;
	Persistent* pTemp;
	fstream logfile;
	CString LogBuffer;
	CString DisplayName;

	logfile.open(
		strLogName,
		ios::out | ios_base::trunc
	);

	// Start log.
	LogBuffer = "Reference log initialized.\n\n";
	logfile.write(
		LPCSTR( LogBuffer ),
		LogBuffer.GetLength()
	);

	// Write database ID.
	GetDatabaseHeader( &LogBuffer );
	logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );

	// Write header for objects.	             OBJECT_TRANSMISSION_CONTROL
	LogBuffer = " Type                         Name                            ObjectID [Node]\n";												logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
	LogBuffer = "                                 SubobjectTypes                     ObjectID      [User#-Group-DB#-Ver]\n";			logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
	LogBuffer = "-------------------------------------------------------------------------------------------------------------\n";	logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );

	long lProgress;
	for ( int i = 0; i < OBJECT_LIST_LEN; i++ ) {

		// Get a temp object.
		ClassID Class = GetClassID( (OBJECT_TYPE) i );
		pTemp = NewObject( Class );

		// Loop through all objects.
		pTemp->FirstObject();
		while ( pTemp->ObjectExists() ) {

			pTemp->GetDisplayName( &DisplayName );

			// Only tree objects have EMInit pointers.
			CString strObjectType;
			if ( i <= TYPE_TREE_LAST )
				strObjectType = EMComponentInit::ByType( i )->strDescSingular();
			else
				strObjectType.Format( 
					_T("OBJECT_TYPE[ %i ]"),
					i	
				);
	
			// LogBuffer = " Type                         Name                            ObjectID [Node]\n";	logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
			LogBuffer.Format(
				" %-28s %-31s %8i [%i]\n",
				strObjectType,
				DisplayName,
				pTemp->GetID(),
				pTemp->ObjectAddress()
			);

			logfile.write(
				LPCTSTR( LogBuffer ),
				LogBuffer.GetLength()
			);

			// Get the number of subobjects for EMComponent objects.
			// Check for large numbers and truncate as needed.
			EMComponent* pEMC = dynamic_cast<EMComponent*>( pTemp );
			if ( pEMC ) {

				int nSubObjCount = pEMC->pEMInit->GetSubObjSize();
				const nSubObjMax = 100;
				if ( nSubObjCount > nSubObjMax ) {
					LogBuffer.Format(
						IDS_DB_BAD_SUBOBJ_COUNT,
						nSubObjCount,
						nSubObjMax
					);
					logfile.write(
						LPCTSTR( LogBuffer ),
						LogBuffer.GetLength()
					);
					nSubObjCount = nSubObjMax;
				}

				// Write out all the Refs within this object.
				// We use the default params to cause a scan of ALL refs.
				RefScanData Data;
				while ( pEMC->GetNextRef( Data ) )
				{


					// Write each on its own line.
					//	LogBuffer = "                                 SubobjectTypes                     ObjectID      [User#-Group-DB#-Ver]\n";	logfile.write( LPCSTR( LogBuffer ), LogBuffer.GetLength() );
					LogBuffer.Format(
						"                                 %-34s %8i      [%i-%i]\n",
						EMComponentInit::ByType( Data.Type )->strDescSingular(),
						Data.pRef->ObID,
						Data.pRef->DBID.UserNumber,
						Data.pRef->DBID.DBNumber
					);
					logfile.write(
						LPCTSTR( LogBuffer ),
						LogBuffer.GetLength()
					);

				}

			}

			// Get the next object from the source database.
			pTemp->NextObject();

		}

		delete pTemp;

		// Update progress meter.
		lProgress = 100 * ( i + 1 ) / nClassCount;
		pProgressMeter->SetPos( (int) lProgress );

	}

	logfile.close();

	return bReturn;

}


//-------------------------------------------------------------------//
// Stuff()																				//
//-------------------------------------------------------------------//
// This function stuffs test data into this database.  It stuffs the
// requested number of object "sets".  One set consists of one of
// every type of object in the database.  Each object set is
// initialized using the object count; see
// EMComponent::FillWithTestData().
//-------------------------------------------------------------------//
bool EMDatabase::StuffTestData(
	CProgressCtrl* pProgressMeter,
	int				nObjectCount
) {


	long lProgress;
	bool bReturn = true;

	// We need a loop object and an object that we can use
	// to store a copy of the object for comparison.
	EMComponent* pLoop;
	EMComponent* pOriginal;
	ObjectReference LoopRef;

	// We are going to stuff a complete object "set" the
	// specified number of times.
	for ( int h = 1; h <= nObjectCount && bReturn; h++ ) {

		// Loop through all object types.
		// We go in hierarchical order as specified in ObjectTypes.h.
		for ( int i = 0; i <= TYPE_TREE_LAST && bReturn; i++ ) {

			// Get new objects.  Provide the db directly.
			pLoop = NewComponent( (OBJECT_TYPE) i );
			pOriginal = NewComponent( (OBJECT_TYPE) i );

			// Fill in all the data fields of the object with a
			// representation of the loop count.
			pLoop->FillWithTestData( h );

			// Copy the "guts" of the object.
			pOriginal->Copy(
				pLoop,	// Source EMComponent pointer.         
				false, 	// Duplicate private objects in the DB?
				false 	// Copy SubObj backup arrays too?      
			);

			// Add the object.
			// We force a save so we can get at the ID
			// before deleting it.
			pLoop->AddObject();
			pLoop->SaveObject();

			// Save the ref, so we can reload it.
			LoopRef = pLoop->GetRef();

			// Now load it back up, using this database.
			delete pLoop;
			pLoop = NewComponent( (OBJECT_TYPE) i, &LoopRef );

			if ( !( *pLoop == *pOriginal ) ) {

				CString strMessage;
				strMessage.Format( _T("Failure on object type %i, number %i."), i, h );
				DisplayMessage( strMessage );
				bReturn = false;

			}

			delete pLoop;
			delete pOriginal;

			// Update progress meter.
			lProgress = ( 100 * ( h * ( TYPE_TREE_LAST + 1 ) + i + 1 ) ) / ( nObjectCount * ( TYPE_TREE_LAST + 1 ) );
			pProgressMeter->SetPos( (int) lProgress );

		}

	}

	return bReturn;

}


//-------------------------------------------------------------------//
// DeleteTestData()																	//
//-------------------------------------------------------------------//
// This function deletes a subset of objects in the database.
// Currently, it deletes all objects below the given count with
// even object ID's.
//-------------------------------------------------------------------//
bool EMDatabase::DeleteTestData(
	CProgressCtrl* pProgressMeter,
	int				nObjectCount
) {

	long lProgress;
	bool bReturn = true;

	// We need a loop object and an object that we can use
	// to store a copy of the object for comparison.
	EMComponent* pLoop;

	// We stuff this in a var so we can change it easily in debug mode.
	int nObjectTypeCount = TYPE_TREE_LAST;

	// Loop through all object types.
	// We go in hierarchical order as specified in ObjectTypes.h.
	for ( int i = 0; i < nObjectTypeCount && bReturn; i++ ) {

		// Get all even ID objects and delete 'em.
		for ( int ID = 0; ID <= nObjectCount; ID += 2 ) {
			pLoop = NewComponent( (OBJECT_TYPE) i, &ObjectReference( DatabaseID(), ID ) );
			if ( pLoop->ObjectExists() )
				pLoop->DeleteObject();
			delete pLoop;

			// Update progress meter.
			lProgress = ( 100 * ( i * nObjectCount + ID + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );

		}

	}

	// TO DO
	// Verify contents.

	return bReturn;

}


//-------------------------------------------------------------------//
// NewTestObject()																			//
//-------------------------------------------------------------------//
void EMDatabase::NewTestObject(
	OBJECT_TYPE	eType
) {

	// Get new objects.
	EMComponent *pData = NewComponent( eType );

	pData->AddObject();
	pData->SaveObject();

	ObjectReference Ref = pData->GetRef();

	// Fill in all the data fields of the object.
	pData->FillWithTestData( Ref.ObID - 1 );

	pData->ChangeObject();
	pData->SaveObject();

	delete pData;
}

//-------------------------------------------------------------------//
// DeleteObject()																		//
//-------------------------------------------------------------------//
void EMDatabase::DeleteObject(
	OBJECT_TYPE	eType,
	int			nObjectNum
) {

	// The object ID is nObjectNum.
	ObjectReference Ref;
	Ref.ObID = nObjectNum;

	// Get new objects.
	EMComponent *pData = NewComponent( eType, &Ref );

	pData->DeleteObject();

	delete pData;
}



//-------------------------------------------------------------------//
// SetObjectData()																	//
//-------------------------------------------------------------------//
void EMDatabase::SetObjectData(
	OBJECT_TYPE	eType,
	int			nObjectNum,
	int			nTestValue
) {

	// The object ID is nObjectNum.
	ObjectReference Ref;
	Ref.ObID = nObjectNum;

	// Get new objects.
	EMComponent *pData = NewComponent( eType, &Ref );

	// Fill in all the data fields of the object.
	pData->FillWithTestData( nTestValue );

	pData->ChangeObject();
	pData->SaveObject();

	delete pData;
}


//-------------------------------------------------------------------//
// VerifyObject()																		//
//-------------------------------------------------------------------//
bool EMDatabase::VerifyObject(
	OBJECT_TYPE	eType,
	int			nObjectNum,
	int			nTestValue
) {

	bool			bReturn = true;

	// The object ID is nObjectNum.
	ObjectReference Ref;
	Ref.ObID = nObjectNum;

	// Get new objects.  Provide the db directly.
	EMComponent *pData		= NewComponent( eType, &Ref );
	EMComponent *pCompare	= NewComponent( eType );

	// Fill in all the data fields of the object.
	pCompare->FillWithTestData( nTestValue );

	// Verify the two are the same.
	if ( !( *pData == *pCompare ) ) {

		CString strMessage, strObjName;
		strMessage.Format( 
			_T("Failure on object #%i, of type %s."), 
			nObjectNum, 
			EMComponentInit::ByType( eType )->strDescSingular()
		 );
		DisplayMessage( strMessage );
		bReturn = false;

	}

	delete pData;
	delete pCompare;

	return bReturn;

}


//-------------------------------------------------------------------//
// ChangeTestData()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool EMDatabase::ChangeTestData(
	CProgressCtrl* pProgressMeter,
	int				nObjectCount
) {


	nObjectCount = 20;

	long lProgress;
	bool bReturn = true;

	// We stuff this in a var so we can change it easily in debug mode.
	// We'll just do the one for now, should be all we need.
	// int nObjectTypeCount = TYPE_TREE_LAST;
	int nObjectTypeCount = 1;

	int h, i;
	CString strLog;

	// This is used to create the btree log.
	EMComponent *pMfr = NewComponent( OBJECT_MFR );

	// INITIALIZE - CREATE ALL OBJECTS

	for ( h = 0; h < nObjectCount; h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			NewTestObject( (OBJECT_TYPE) i );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}
	CreateReferenceLog( pProgressMeter, _T("1_AfterInitialAdd.log") );

	// CHANGE & VERIFY

	for ( h = 0; h < nObjectCount; h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			SetObjectData( (OBJECT_TYPE) i, h+1, nObjectCount - h - 1 );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}
	CreateReferenceLog( pProgressMeter, _T("2_AfterFirstChange.log") );
	for ( h = 0; h < nObjectCount; h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			VerifyObject( (OBJECT_TYPE) i, h+1, nObjectCount - h - 1 );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}

	// DELETE EVENS

	for ( h = 0; h < nObjectCount; h+=2 ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			DeleteObject( (OBJECT_TYPE) i, h+1 );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}
	CreateReferenceLog( pProgressMeter, _T("3_AfterDeleteEvens.log") );

	// CHANGE & VERIFY REMAINING OBJS

	for ( h = 1; h < nObjectCount; h+=2 ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			SetObjectData( (OBJECT_TYPE) i, h+1, h );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}
	CreateReferenceLog( pProgressMeter, _T("4_AfterSecondChange.log") );
	for ( h = 1; h < nObjectCount; h+=2 ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			VerifyObject( (OBJECT_TYPE) i, h+1, h );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}

	// ADD & VERIFY NEW OBJS

	for ( h = 0; h < nObjectCount; h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			NewTestObject( (OBJECT_TYPE) i );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
			pMfr->LogBTree();
		}
	}
	CreateReferenceLog( pProgressMeter, _T("5_AfterSecondAdd.log") );
	for ( h = nObjectCount; h < ( nObjectCount << 1 ); h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			VerifyObject( (OBJECT_TYPE) i, h+1, h );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}

	// DELETE ODDS

	// Note: Previously a btree bug occcurred on the "h=11" deletion.
	// It still occurs only if less than 4 keys can fit in a node.
	// We put in an ASSERT to check this condition.
	for ( h = 1; h < nObjectCount; h+=2 ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			DeleteObject( (OBJECT_TYPE) i, h+1 );

			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}

	}

	// Log the mfr btree.
	pMfr->LogBTree();

	CreateReferenceLog( pProgressMeter, _T("6_AfterDeleteOdds.log") );

	// CHANGE & VERIFY REMAINING OBJS

	for ( h = nObjectCount; h < ( nObjectCount << 1 ); h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			SetObjectData( (OBJECT_TYPE) i, h+1, ( nObjectCount << 1 ) - h - 1 );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}

	CreateReferenceLog( pProgressMeter, _T("7_AfterLastChange.log") );

	for ( h = nObjectCount; h < ( nObjectCount << 1 ); h++ ) {
		for ( i = 0; i < nObjectTypeCount; i++ ) {
			VerifyObject( (OBJECT_TYPE) i, h+1, ( nObjectCount << 1 ) - h - 1 );
			lProgress = ( 100 * ( h * nObjectTypeCount + i + 1 ) ) / ( nObjectCount * nObjectTypeCount );
			pProgressMeter->SetPos( (int) lProgress );
		}
	}

	// Delete mfr btree log helper object.
	delete pMfr;

	return bReturn;

}


//-------------------------------------------------------------------//
// VerifyTestData()																	//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
bool EMDatabase::VerifyTestData(
	BaseProgressDlg* pProgressDlg
) {

	bool bReturn = true;
	int i;

	// We stuff this in a var so we can change it easily in debug mode.
	int nObjectTypeCount = TYPE_TREE_LAST;

	CString strProgress;
	CString strObjType;
	int nObjectCount;

	// Loop through all object types.
	// We go in hierarchical order as specified in ObjectTypes.h.
	for ( i = 0; i <= nObjectTypeCount && bReturn; i++ ) {

		// Create a new object of this type in this database.
		EMComponent* pData = NewComponent( (OBJECT_TYPE) i );

		// Get the first.
		pData->FirstObject();
		nObjectCount = 1;

		// Keep the desc string around during the loop.
		strObjType.LoadString( EMComponentInit::ByType( i )->uiDescSingular() );

		while ( pData->ObjectExists() && bReturn ) {

			// Get a new temp object.
			EMComponent* pCompare	= NewComponent( (OBJECT_TYPE) i );

			// Fill in all the data fields of the object
			// using the data object's ID.
			pCompare->FillWithTestData( pData->GetID() );

			// Verify the two are the same.
			if ( !( *pData == *pCompare ) ) {

				strProgress.Format(
					_T("Failure on object %i of type %s."),
					nObjectCount,
					strObjType
				);
				DisplayMessage( strProgress );
				bReturn = false;

			} else {

				// Report progress.
				strProgress.Format(
					_T( "Processed object %i of type %s..." ),
					nObjectCount,
					strObjType
				);
				pProgressDlg->SetText( strProgress );

			}

			delete pCompare;

			pData->NextObject();
			nObjectCount++;

		}

		delete pData;

		// Update progress.
		strProgress.Format(
			_T("%i objects of type %s verified."),
			nObjectCount - 1,
			strObjType
		);
		DisplayMessage( strProgress );

	}

	return bReturn;

}


//-------------------------------------------------------------------//
// VerifyContents()																	//
//-------------------------------------------------------------------//
// Verifies that every object in the database is fully
// complete and calculated.  Puts up a detailed error
// log for the user, which he can save for reopening later.
// Passing a valid eSingleType will cause this function to
// check only that object type.
//-------------------------------------------------------------------//
bool EMDatabase::VerifyContents( int nStartType, bool bContinueAfter )
{
	bool bAllValid = true;

	CWaitCursor HopeYoButtFallsAsleepYaChump;

	/////////////////////////////////////////////////////////////////////
	// Create an error log window for displaying the results of our
	// database verification scan.

	CString strTitle = GetDatabaseName();
	strTitle += ResStr( IDS_VERIFICATION_LOG );

	CEdit &LogEdit = CLogFrame::AutoCreateLog( strTitle );

	CString strLog = strTitle + strCR + strCR;

	/////////////////////////////////////////////////////////////////////
	// Now loop through all the object types in turn.

	for ( int nA = nStartType; nA <= TYPE_TREE_LAST; nA++ ) 
	{
		OBJECT_TYPE eType = (OBJECT_TYPE) nA;
		bool bThisTypeError = false;
		CString strType = EMComponentInit::ByType( eType )->strDescPlural() + _T("...") + strCR;

		// Add object type header to log "temporarily".
		// Update the edit window and scroll to the bottom.
		LogEdit.SetWindowText( strLog + strType );
		LogEdit.LineScroll( LogEdit.GetLineCount() );

		/////////////////////////////////////////////////////////////////////
		// Loop through all the objects of this type.

		EMComponent* pObject = NewComponent( eType );
		pObject->FirstObject();

		while ( pObject->ObjectExists() ) 
		{
			bool bError = false;

			CString strName;

			// Use the tree control to get the display name as opposed to
			// requesting it from the object itself.  This is probably faster
			// but, most importantly, we can verify the tree cache by making
			// sure that every object in the db has a matching tree list entry.
			bool bResult = EMComponent::GetDisplayNameFromTree( 
				&strName,				// CString to stuff name into.
				eType, 					//	the object's type.
				&pObject->GetRef() 	//	ObjectReference to search for.   
			);

			// Error found - get the display name directly from the object and
			// add an error entry to the log.
			if ( !bResult )
			{
				// If this is the first error for this object type
				// then add the type string to the log "permanently".
				if ( !bThisTypeError )
				{
					strLog += strType;
					bThisTypeError = true;
				}

				pObject->GetDisplayName( &strName );
				bError = true;

				strLog += strTAB + strName + strCR;
				strLog += strTAB + strTAB + ResStr( IDM_NOT_FOUND_IN_CACHE ) + strCR;

				// Update the edit window and scroll to the bottom.
				LogEdit.SetWindowText( strLog );
				LogEdit.LineScroll( LogEdit.GetLineCount() );
			}

			// No error found - add the object's display string to the log
			// temporarily as a sort of progress bar.
			else
			{
				// Update the edit window and scroll to the bottom.
				if ( bThisTypeError )
					LogEdit.SetWindowText( strLog + strTAB + strName + strCR );
				else
					LogEdit.SetWindowText( strLog + strType + strTAB + strName + strCR );

				LogEdit.LineScroll( LogEdit.GetLineCount() );
			}

			// First save the object back to the database in
			// case any changes were made upon loading the
			// object due to versioning.

			// Arrays to pass error strings back through.
			CStringArray arstrErr;
			bool bFoundRevEngItems = false;

			// We will report on published db's, but not update them.
			if ( !IsPublished() )
			{
			
				// Validate everything, save it back to the db, update the
				// tree string and sorting, etc. etc. etc.
				pObject->ValidateAndSave(
					false,					// Add it as a new object?
					false,					// Copy over an existing object?
					NULL,						// The object to copy over.
					false,					// Update tree sort & name?
					true,						// Update tree if errors found?
					&arstrErr,				// Array to pass error strings back through.
					&bFoundRevEngItems,	// true if we found items that may be rev-eng'ed
					2							// # of tabs to add before error string.
				);

			}

			// If the object is not complete then add the "why" info to the log.
			// We skip private objects here because they will appear within the
			// error log section of their owners and the only way to get to them
			// is through their owners anyway.
			if ( !pObject->bIsComplete() && !pObject->bIsPrivate() ) 
			{
				if ( !bError )
				{
					// If this is the first error for this object type
					// then add the type string to the log "permanently".
					if ( !bThisTypeError )
					{
						strLog += strType;
						bThisTypeError = true;
					}

					strLog += strTAB + strName + strCR;
					bError = true;
				}

				strLog += pObject->GetDataCompleteErrors( 
					&arstrErr,				// Array to pass error strings back through.
					bFoundRevEngItems		// true if we found items that may be rev-eng'ed
				);

				if ( pObject->bCalledDoRevEng() && !pObject->bRevEngComplete() )
				{
					// Add a carriage return between sections as needed.
					if ( !strLog.IsEmpty() )
						strLog += strCR;

					strLog += pObject->GetDoRevEngErrors();

				}
			}

			if ( bError )
				bAllValid = false;

			// Get the next object for the next while loop pass.
			pObject->NextObject();

		}

		delete pObject;

		// Put a blank line between each object type section.
		if ( bThisTypeError )
			strLog += strCR;

		// Only wanted one pass?
		if ( !bContinueAfter )
			break;
	}

	// Update the edit window with the appropriate 
	// final results strings and scroll to the bottom.
	strLog += ResStr( IDS_DB_VERIFICATION_FINISHED ) + strCR;

	if ( bAllValid )
	{
		if ( bContinueAfter )
		{
			if ( nStartType == 0 )
				strLog += ResStr( IDS_ALL_OBJ_VERIFIED );
			else
				strLog += ResStr( IDS_SEL_OBJ_VERIFIED );
		}
		else
		{
			strLog += ResStr( IDS_ALL_SP )
				+ EMComponentInit::ByType( nStartType )->strDescPlural() 
				+ ResStr( IDS_SP_VERIFIED );
		}
	}
	else
	{
		strLog += ResStr( IDS_DB_OBJ_ERRORS );
	}

	LogEdit.SetWindowText( strLog );
	LogEdit.LineScroll( LogEdit.GetLineCount() );

	return bAllValid;
}

