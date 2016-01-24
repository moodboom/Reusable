// NodeFile.h

#ifndef NODE_FILE_H
	#define NODE_FILE_H

#include <fstream>			// For fstream
#include <string>				// For string

#include "..\StandardTypes.h"

class BaseDatabase;							// We use a pointer for extended header functions.


// MDM VC 8.0 does not yet implement Exception Specifications, which are 
// provided in BaseDatabase.  The code compiles fine, it just does not 
// restrict thrown exceptions to the specified type (which is fine for now).
// This pragma removes warnings about the missing implementation.
#ifdef WIN32
#pragma warning( disable : 4290 )
#endif


// NodeNbr's type, combined with the granularity, dictates how 
// large database files can get.
//
//		Type size	Granularity		Max File Size
//		---------	-----------		-------------
//		2 bytes		128 bytes		64k x 128 bytes = 8 MB 
//		4 bytes		128 bytes		4 billion x 128 bytes = ~600 GB
//
typedef uLong NodeNbr;

// ----- exceptions to be thrown
class BadFileOpen{};
class FileReadError{};
class FileWriteError{};


// ============================
// Node File Header Record
// ============================
class FileHeader	{
	
	// Member variables.
	// The variables are clustered in a manner that
	// is optimized for structure storage.  Structures
	// are aligned on 8-byte boundaries by the compiler,
	// by default.  Variables are on boundaries that are
	// multiples of their size.

	// Type		Name				  Size	Description
	//------------------------(bytes)--------------------------------------------
	NodeNbr		deletednode;	//	  4	first deleted node
	NodeNbr		highestnode;	//	  4	highest assigned node
	uShort		nodelength;		//	  2	For file-level granularity specification
	uShort		FileStatus;		//   2	For future flag use

	friend class NodeFile;
	
	// Clear data upon creation.
	// DatabaseID's constructor will clear itself.
	FileHeader() { 
		deletednode = highestnode = 0; nodelength = FileStatus = 0; 
	}

};


// NodeFile::Initialize() return values.
#define NFI_SUCCESS					0
#define NFI_ENCRYPTION_FAILURE	1
#define NFI_FILE_NOT_FOUND			2

// ================
// Node File Class
// ================
class NodeFile	{

	// Used for file-level granularity specification.
	short int nodedatalength;

	// The header of the file is kept in memory while we have
	// exclusive write access to the file.  It is then written
	// out when we are done.
	FileHeader header;
	FileHeader origheader;
	
	fstream nfile;
	bool newfile;		 // true if building new node file

public:
   
	NodeFile::NodeFile(
		const CString* pstrNewFilename
	);
	
	virtual ~NodeFile();

	int Initialize( 
		int		nGranularity		= 0,
		bool		bCreateAsNeeded	= true,
		BaseDatabase*	pDB					= 0,
		bool		bTruncate			= false
	);

	int nExtendedHeaderSize;

	// These are used to open/close the file, specifying whether
	// we are doing a read or write.
	bool Open( bool bWritable ) throw (BadFileOpen);
	void Close( bool bWritable );

	bool IsClosed() { return nOpenCount == 0; }
	
	// These functions manipulate the file header on disk.
	inline void ReadNodeLength();
	inline void ReadHeader();
	inline void WriteHeader( bool bForceWrite = false );

	// Allow access to header data.
	short int	GetNodeLength()						{ return header.nodelength;	}
	NodeNbr		DeletedNode() const					{ return header.deletednode;	}
	NodeNbr		HighestNode() const					{ return header.highestnode;	}
	short int	GetNodeDataLength()					{ return nodedatalength;		}
	void			SetDeletedNode( NodeNbr node )	{ header.deletednode = node;	}
	void			SetHighestNode(NodeNbr node)		{ header.highestnode = node;	}
	
	void ReadData(
		void *buf,
		unsigned short siz, 
		long wh = -1
	) throw (FileReadError);

	void WriteData(
		const void *buf,
		unsigned short siz, 
		long wh = -1
	) throw (FileWriteError);
	
	void Seek( 
		streampos offset, 
		ios::seek_dir dir = ios::beg 
	) {
		// MDM BaseDatabase REWRITE
		// nfile.seekg( offset, (enum ios_base::seekdir)dir ); 
		// nfile.seekp( offset, (enum ios_base::seekdir)dir ); 
		nfile.seekg( offset, (ios_base::seekdir)dir ); 
		nfile.seekp( offset, (ios_base::seekdir)dir ); 
	}

	streampos FilePosition()
		{ return nfile.tellg(); }
	bool IsNewFile() const
		{ return newfile; }
	void ResetNewFile()
		{ newfile = false; }

	// BaseDatabase::RebuildIndex() needs this.
	CString* GetFilename()
	{ return &strFilename; }

	void GetFilenameNoPath(
		CString* pstrFilename
	);

protected:
	CString			strFilename;
	unsigned int	nOpenCount;

	friend class BaseDatabase;			// For access to header data during Compact.

};



//-------------------------------------------------------------------//
// ReadHeader()																		//
//-------------------------------------------------------------------//
// This function reads the header structure from the database.  The
// header is maintained in memory during a write, and then
// flushed at the end of the write, to minimize database access.
//-------------------------------------------------------------------//
inline void NodeFile::ReadHeader()
{

	ReadData(
		&header, 
		sizeof header - sizeof header.nodelength,
		0
	);
	origheader = header;
	
}




#endif  // NODE_FILE_H

