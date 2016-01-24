// ----------- node.cpp

// ===============================================
// BaseDatabase Node and NodeFile class member functions
// ===============================================

#include "stdafx.h"

#include <io.h>		// For access()

#include "BaseDatabase.h"


//-------------------------------------------------------------------//
// NodeFile()																			//
//-------------------------------------------------------------------//
// The constructor is just used to initialize data.
// Initialization occurs in a separate Initialize() function.  
// This is done for two MAJOR reasons that apply to constructors
// in general: 
//
//		1) The first is simply because the constructor cannot return 
//			a result value.  We would be forced to add exception 
//			handling to provide a result value.  Exception handling adds
//			extra overhead.  There are few instances where the overhead
//			is justified, perhaps when a large number of types of errors 
//			must be passed upstream to be handled.  Even then, 
//			returning an appropriate result seems more concise.
//		2) The second is because functions overridden in derived 
//			classes cannot be resolved within the constructor in C++.  
//			Any call to an overridden function here would only
//			execute the base version.
//
// See Initialize().
//-------------------------------------------------------------------//
NodeFile::NodeFile(
	const CString* pstrNewFilename
) :

	// Init vars.
	nOpenCount( 0 )

{

	// Store the filename.
	strFilename = *pstrNewFilename;

}


NodeFile::~NodeFile()
{

	// Make sure you matched Opens with Closes.
	ASSERT( nOpenCount == 0 );

}


//-------------------------------------------------------------------//
// Initialize()																		//
//-------------------------------------------------------------------//
// This function is called to initialize the file.  This consists
// of making sure the file exists, and getting the nodelength and 
// verifying it.
// MS translates the original code's open flags to "r+b", 
// which requires the file to exist for open to succeed.  If we 
// add the trunc flag, the sum of flags is translated to "w+b", 
// which will create the file if needed, but kill its contents.
// So we need to test for existance of the file first, and use
// the appropriate flags.
//
// The pDB param is passed only if we need to write extended
// header data.  This is true for data files, not index files.
//
// The bTruncate flag is used when rebuilding the index file.  It
// guarantees that we start with a new file.
//
// We return:
//		NFI_SUCCESS					If everything's OK
//		NFI_ENCRYPTION_FAILURE	If ReadEncryptedHeader() fails
//		NFI_FILE_NOT_FOUND		If the file is not found and 
//											creation was not requested
//
//-------------------------------------------------------------------//
int NodeFile::Initialize( 
	int		nGranularity,
	bool		bCreateAsNeeded,
	BaseDatabase*	pDB,
	bool		bTruncate
) {

	int nReturn = NFI_SUCCESS;
	
	// Truncate requests should always specify bCreateAsNeeded as true.
	ASSERT( !bTruncate || bCreateAsNeeded );
	
	// See if the file is 'new', meaning it does not exist, or
	// the caller requested it be truncated.
	newfile = ( _access( LPCTSTR( strFilename ), 0 ) != 0 ) || bTruncate;

	// If we have a file, then we can just open it.
	if ( !newfile ) {

		nfile.open(
			LPCTSTR( strFilename ), 
			ios::in | ios::binary
		);

		// Read in the nodelength.
		ReadNodeLength();
		
		// We specified the default granularity in the constructor.
		// Here, we put a debug message in the output window if the 
		// granularity specified within the file itself differs from the default.
		// Note that this is not a problem, we just wanted to
		// be notified that we are dealing with a non-standard
		// granularity.
		#ifdef _DEBUG
			if ( header.nodelength != nGranularity ) {
				CString strMsg;
				strMsg.Format(
					_T("Note: %s has non-standard granularity of %d.\n"),
					*GetFilename(),
					header.nodelength
				);
				TRACE0( LPCTSTR( strMsg ) );
			}
		#endif

		// Read in any extended header required by the database.
		if ( pDB )
			if ( !pDB->ReadHeader() )
				nReturn = NFI_ENCRYPTION_FAILURE;

		// Note: the following code addresses a problem
		// with BaseDatabase::FindClass().  That function assumes that
		// we can search the index file for the class by reading
		// node 1 and continuing from there, as long as the file
		// is not new.  However, node 1 will not exist if a new 
		// database is created and nothing is done to it, then 
		// it is closed and reopened.  Here, we made sure that
		// files that did not contain node 1 were marked as new.
		// However, since we now require classes to be registered
		// first thing in new databases, we can assume safely that
		// there will always be a node 1 when calling FindClass().
		// Therefore, the code has been commented out.
	
		/*
		// Here, we want to check to see if we have added any
		// nodes to this file yet.  If not, we want to keep the newfile
		// status set to true, so we don't go looking for nodes
		// when there aren't any.
		// We can tell by the size of the file; if all it is is a 
		// header, then we know we need to keep the newfile status.
		
		// Get the diff between here and the EOF.
		streampos start = nfile.tellg();
		nfile.seekg( 0, ios::end );
		streampos end = nfile.tellg();
		
		// If we don't have a node's worth of file, set to new.
		if ( (int)( end - start ) < header.nodelength ) {
			newfile = true;
		}
		*/

		nfile.close();
		
	// If the file is new, we will only create if requested.
	} else if ( bCreateAsNeeded ) {

		// Create or truncate the file.
		nfile.open(
			LPCTSTR( strFilename ), 
			ios::in | ios::out | ios::binary | ios_base::trunc 
		);
		
		// We need to create and save a new header.
		// Make sure we clear out the header values.  If we
		// are truncating, they will have old values in them.
		memset( &header, 0, sizeof header );

		// Use the given granularity.
		ASSERT( nGranularity != 0 );
		header.nodelength = nGranularity;

		// Force a header save.
		WriteHeader( true );

		// Write the database's extended header as needed.
		if ( pDB ) 
			pDB->WriteHeader();

		// Now close it.
		nfile.close();

	} else {
	
		// We were supposed to find a file, and didn't.
		// We'll save the requested granularity and return.
		header.nodelength = nGranularity;
		nReturn = NFI_FILE_NOT_FOUND;

	}

	// Initialize node data length.
	// The data takes up the entire node, with the exception
	// of the next node number, located at the beginning of
	// the node.
	nodedatalength = header.nodelength - sizeof(NodeNbr);

	// Keep track of the header size for calculating offsets.
	// We need to make sure we call this AFTER we have taken
	// care of the file header.
	nExtendedHeaderSize = pDB? pDB->GetHeaderSize() : 0;
	
	return nReturn;

}


//-------------------------------------------------------------------//
// Open()																				//
//-------------------------------------------------------------------//
// This function opens the node file for subsequent reading or
// writing of an object.
// Initialize() should have been called previously to verify
// that the file exists.  Also, BaseDatabase::VerifyFilesAreWritable() 
// should have been called before write access is attempted.  Here, 
// we just ASSERT either case is true.
//-------------------------------------------------------------------//
bool NodeFile::Open( bool bWritable ) throw (BadFileOpen)
{

	// If the file is not already open...
	if ( nOpenCount == 0 ) {
		
		// Assert that we have the requested access to the file.
		// If a read-only file is attempted to be opened with
		// bWritable == true, or if a non-existant file is
		// attempted to be opened for reading, this ASSERTs.

		// TO DO
		// ASSERTing wont do us any good out in the
		// "real world".  We only used it because this
		// call should always come after Initialize() and
		// VerifyFilesAreWritable() have been called.
		// But this does not guarantee that the file was not 
		// deleted, etc., since we called Initialize() and/or
		// VerifyFilesAreWritable().  We could return an error 
		// instead, to be totally safe.  But we would also
		// need to add handling of error returns from this
		// function.
		ASSERT( 
			_access( 
				LPCTSTR( strFilename ), 
				bWritable? 6 : 4				// 6 = write, 4 = read
			) == 0 
		);
		
		// TO DO
		// We want to use the protection flags (filebuf::sh_none, etc.),
		// but they are only available in MS's implementation of the 
		// fstream class, not the Standard C++ Library that we are using.
		// Consider converting from <fstream> to <fstream.h>.
		nfile.open(
			LPCTSTR( strFilename ), 
			ios::in | ios::binary | ( bWritable? ios::out : 0 )
		);
		
		// After opening, we want to read the file header.  This header is
		// used to cache file-level changes until we are ready to write our
		// changes out.  Note that this file will be opened, the header read,
		// changes made, and then the file will be saved all within one
		// step.  We only cache the header while we have exclusive write
		// access to the file, to avoid multiple writes.
		// Note that the header only contains data we need if we are
		// writing, so we don't even bother to read it if we aren't
		// going to be writing.
		if ( bWritable )
			ReadHeader();

	}
	
	// Increment the open count.
	nOpenCount++;

	// Make sure you match Opens with Closes.
	ASSERT( nOpenCount > 0 );

	// With nOpenCount, we provide for nested open/closes.
	// However, you must not nest a writable open within
	// a top-level read-only open.  Otherwise, we ASSERT 
	// here.
	// Note that we want to enforce this rule.  LoadObject
	// should always be done read-only, since we may be
	// dealing with a read-only file.  And SaveObject should
	// always get full write access across all operations.
	// TO DO
	// Can't use openmode?  How do we determine file status?
	/*
	ASSERT( 
			( ( nfile.openmode ) & ios::out )	// Either we can write,
		|| !bWritable									// or we didn't request it.
	);
	*/

	// TO DO
	// Handle errors.
	return true;

}


//-------------------------------------------------------------------//
// Close()																				//
//-------------------------------------------------------------------//
// Here, we close the file.  You must specify whether it had been
// opened for writing, so we know whether we should flush the
// cached header or not.
//-------------------------------------------------------------------//
void NodeFile::Close( bool bWritable )
{

	if ( nOpenCount == 1 ) {

		// Save the header as needed if we were doing a write.
		if ( bWritable )
			WriteHeader();
		
		// Close the file.
		nfile.close();

	}
	
	// Decrement the count.
	nOpenCount--;
	
	// Make sure you match Opens with Closes.
	ASSERT( nOpenCount >= 0 );

}


//-------------------------------------------------------------------//
// ReadNodeLength()																	//
//-------------------------------------------------------------------//
void NodeFile::ReadNodeLength()
{

	// The nodelength variable is stored right at the end of
	// the header, which is first in the file.
	ReadData(
		&header.nodelength, 
		sizeof header.nodelength, 
		
		// Get the offset of nodelength from the beginning of 
		// the header (the same both in memory and in the database).
		(long)&header.nodelength - (long)&header
	
	);
	origheader = header;
	
}



//-------------------------------------------------------------------//
// WriteHeader()																		//
//-------------------------------------------------------------------//
// This function writes out the header structure if needed.  It 
// is typically maintained in memory during a write, and then
// flushed at the end of the write, to minimize database access.
// Note that we have to force a write when we have a new file.
// Also, the header contains the nodelength, but its value does 
// not change, so it is written only if bForceWrite is true, 
// because we have a new file.
//-------------------------------------------------------------------//
void NodeFile::WriteHeader( bool bForceWrite )
{

	if (	bForceWrite ) {

		// We have a new file, write out the entire header.
		WriteData(&header, sizeof header, 0);
		origheader = header;

	} else if (
			header.deletednode != origheader.deletednode
		|| header.highestnode != origheader.highestnode 
	) {
	
		// The file header has changed, write 
		// out the changes.
		ASSERT( ( (long)&header.nodelength - (long)&header ) < USHRT_MAX );
		WriteData(
			&header, 
			
			// Don't bother writing past beginning of nodelength.
			(unsigned short)( (long)&header.nodelength - (long)&header ),
			
			0
		);
		origheader = header;
	
	}
	
}


//-------------------------------------------------------------------//
// ReadData()																			//
//-------------------------------------------------------------------//
void NodeFile::ReadData(void *buf, 
			unsigned short siz, long wh) throw (FileReadError)
{
	if (wh != -1)
		nfile.seekg(wh);
	nfile.read(reinterpret_cast<char*>(buf), siz);
	
	// TO DO
	// This is currently being hit if the database file is
	// read-only.  We need to handle the problem upstream, 
	// when the file is opened.
	if ( nfile.fail() || nfile.eof() ) {
		nfile.clear();
		throw FileReadError();
	}
	
	nfile.seekp(nfile.tellg());
}


//-------------------------------------------------------------------//
// WriteData()																			//
//-------------------------------------------------------------------//
void NodeFile::WriteData(const void *buf, 
			unsigned short siz, long wh) throw (FileWriteError)
{
	if (wh != -1)
		nfile.seekp(wh);
	nfile.write(reinterpret_cast<const char*>(buf), siz);
	
	// MDM Modified.
	// I believe we were not being notified of an incomplete
	// write here due to disk full.  The docs are sparse, at best.
	// if (nfile.fail()) {
	if ( !nfile.good() ) {
		nfile.clear();
		throw FileWriteError();
	}
	nfile.seekg(nfile.tellp());

}


//-------------------------------------------------------------------//
// GetFilenameNoPath()																//
//-------------------------------------------------------------------//
// This function strips the path from the filename and places it in
// the passed string.
// This is used during publishing.
//-------------------------------------------------------------------//
void NodeFile::GetFilenameNoPath(
	CString* pstrFilename
) {

	*pstrFilename = *GetFilename();

	int nTrunc;
	if (	
			( nTrunc = pstrFilename->ReverseFind( _T('\\') ) ) != -1 
		|| ( nTrunc = pstrFilename->ReverseFind( _T('/' ) ) ) != -1 
	) {

		// Strip it.
		*pstrFilename = pstrFilename->Right( pstrFilename->GetLength() - nTrunc - 1 );

	}

}

