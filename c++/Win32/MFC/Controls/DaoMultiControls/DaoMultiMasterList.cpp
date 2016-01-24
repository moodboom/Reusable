// DaoMultiMasterList.cpp : implementation file
//

#include "stdafx.h"

#include "DaoMultiMasterList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DaoMultiMasterList


//-------------------------------------------------------------------//
// DaoMultiMasterList()																//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
DaoMultiMasterList::DaoMultiMasterList(
	CString	strUniqueRegistryName,
	bool	bUseExternalData			,
	UINT	NoItemMenuID				,
	UINT	OneItemMenuID				,
	UINT	MultiItemMenuID
) :

	// Call base class.
	inherited(
		strUniqueRegistryName	,
		bUseExternalData			,
		NoItemMenuID				,
		OneItemMenuID				,
		MultiItemMenuID	
	),

	// Init vars.
	m_pSet( 0 )

{
}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// This damned function is necessary for any operations where you
// need access to the control window before it is shown.
//-------------------------------------------------------------------//
void DaoMultiMasterList::Initialize()
{

	// Make sure you assign a pointer to the set before you start
	// using the list.  Typically, something like this goes in your 
	// window's constructor:
	/*
		// We keep the table open.
		// We also get and keep a set open.
		try
		{
			pSongTable	= new CDaoTableDef( &JukeboxDB );
			pSongTable->Open( _T("Songs" ) );
			pSong = new SongSet( &JukeboxDB );
			pSong->Open( pSongTable );
			pSong->SetCurrentIndex( _T("ID") );

			// We want to tie the master list to the set.
			SongList.m_pSet = pSong;

			// We also need to tie down the db and table, for the
			// ignorant sorting mechanism.
			SongList.pJukeboxDB = &JukeboxDB;
			SongList.pSongTable = pSongTable;

		}
		catch( CDaoException* e )
		{
			// bail out...
		}

	*/
	
	// DEBUG
	// At the moment, we have a "friend song list" that does not fit
	// this requirement - revisit as needed...
	// ASSERT( m_pSet != 0 );

	// Call base class.
	inherited::Initialize();
}


//-------------------------------------------------------------------//
// ~DaoMultiMasterList()															//
//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
DaoMultiMasterList::~DaoMultiMasterList()
{
}

//-------------------------------------------------------------------//
// BEGIN_MESSAGE_MAP()																//
//-------------------------------------------------------------------//
BEGIN_MESSAGE_MAP(DaoMultiMasterList, DaoMultiMasterList::inherited)
	//{{AFX_MSG_MAP(DaoMultiMasterList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
