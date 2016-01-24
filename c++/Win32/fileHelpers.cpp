//-------------------------------------------------------------------//
// fileHelpers
//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#include <windows.h>						// For CreateDirectory()

#include <io.h>							// For _taccess()

#include "ustringHelpers.h"			// For GetPathFromFilename()

#include "fileHelpers.h"


//-------------------------------------------------------------------//
// MakeDir()																			//
//-------------------------------------------------------------------//
// This function makes the requested dir.
// It can handle requests that require creation of more than one
// directory.
//
// It only returns false if the dir does not exist after the call.
// More specifically, if CreateDirectory() fails because the dir
// already exists, we still return true.
//-------------------------------------------------------------------//
bool MakeDir( ustring& strPath )
{
	bool bReturn = true;

	// Copy the param so we don't damage it.
   // This isn't really any different than removing the ref marker on the param,
   // but this way some overzealous programmer (read "me") doesn't just re-add
   // the ref marker to the param after we remove it.
   ustring strPathCopy = strPath;
   
	// Recurse as needed.
	RemoveTrailingBackslash( strPathCopy );
	ustring strParent = GetPathFromFilename( strPathCopy );
	if ( 
			!( strParent == strPathCopy )
		&&	!bFileExists( strParent ) 
	)
		bReturn = MakeDir( strParent );
	
	if ( bReturn )
	{
		bReturn = ( 
			CreateDirectory( 
				strPathCopy.c_str(),	// pointer to directory path string
				0 						// pointer to security descriptor (NT only), NULL = use default
			) != FALSE
		);

		if ( !bReturn )
		{
			// If we failed because the dir exists, we want to return true.
			DWORD dwError = GetLastError();
			if ( dwError == ERROR_ALREADY_EXISTS )
				bReturn = true;
		}
	}

	return bReturn;

}


//-------------------------------------------------------------------//
// bFileExists()																		//
//-------------------------------------------------------------------//
// This function determines if the given file exists.
//-------------------------------------------------------------------//
bool bFileExists( ustring& strFilename )
{
	return ( _taccess( strFilename.c_str(), 0 ) != -1 );
}


LogRotator::LogRotator()
{
}


bool LogRotator::Initialize(
   ustring  str_log_path                  ,
   ustring  str_log_filename              ,
   ustring  str_log_extension             ,
   int      n_number_of_logs_to_archive
) {
   // Init vars
   m_str_log_path       = str_log_path       ;
   m_str_log_filename   = str_log_filename   ;
   m_str_log_extension  = str_log_extension  ;

   // Make sure parameters are set.
   if ( m_str_log_path.size() == 0 )
      return false;

   if ( m_str_log_filename.size() == 0 )
      return false;

   // Make sure the path ends in a backslash.
   if ( m_str_log_path[ m_str_log_path.size() - 1 ] != _T('\\') )
      m_str_log_path += _T('\\'); 
   
   // Make sure the log file exists.
   ustring str_full_filename = m_str_log_path + m_str_log_filename;
   str_full_filename += _T(".");
   str_full_filename += m_str_log_extension;
   if ( !bFileExists( str_full_filename ) )
   {
      // Create the directory.
      // The ofstream should then be able to create the file.
      if ( !MakeDir( m_str_log_path ) )
         return false;
   }

   // We rotate the last (n-1) log files, dropping the nth.
   int n_log_loop = n_number_of_logs_to_archive;
   while ( !bFileExists( get_archive_name( n_log_loop ) ) && n_log_loop > 0 )
   {
      --n_log_loop;
   }
   for ( ; n_log_loop > 0; n_log_loop-- )
   {
      if ( bFileExists( get_archive_name( n_log_loop ) ) )
      {
         if ( n_log_loop == n_number_of_logs_to_archive )
         {
            // nth log, kill it.
            _tremove( get_archive_name( n_log_loop ).c_str() );

         } else
         {
            // Rotate it.
            _trename(
               get_archive_name( n_log_loop     ).c_str(),
               get_archive_name( n_log_loop + 1 ).c_str()
            );
         }
      }
   }

   // Now rotate the last one.
   if ( bFileExists( str_full_filename ) )
   {
      _trename(
         str_full_filename.c_str(),
         get_archive_name( 1 ).c_str()
      );
   }

   // Open a new file for writing.
	m_pFileStream = new std::ofstream( str_full_filename.c_str(), 0 );

   return true;
}


ustring LogRotator::get_archive_name( int n_log_number )
{
   ustring str_archive_name;
   str_archive_name.printf( 
      m_str_log_path.size() + m_str_log_filename.size() + m_str_log_extension.size() + 5, 
      _T("%s%s%d%s%s"), 
      m_str_log_path.c_str(), 
      m_str_log_filename.c_str(), 
      n_log_number,
      _T("."),
      m_str_log_extension.c_str()
   );
   return str_archive_name;
}


void LogRotator::AddToLog( ustring& strMsg )
{
   std::ofstream& out = *m_pFileStream;

	out.write( strMsg.c_str(), strMsg.size() );
   out << std::endl;
   out.flush();

   /*




            // Get log file size.

            // Is current log file big enough to require rotation?
            if ( nLogFileSize > pref.MaxLogFileSize )
            {
               // Rotate log files.

               // Check total log space usage - do we need to remove the last file?
            }




   std::ofstream& out = *m_pFileStream;

	out.seekp( ua.m_ta.StartByte(), std::ios::beg );
	m_pFileStream->write( &ua.m_ta.FileBuffer()[0], ua.m_ta.ChunkSize() );




   // Open a new file for writing.
	m_pFileStream = new std::ofstream( m_fd.strCompleteFilename().c_str(), std::ios::binary );   



		// Close the stream.
		out.close();
		delete m_pFileStream;
		m_pFileStream = 0;

   */

}


LogRotator::~LogRotator()
{
	// Close the stream.
	m_pFileStream->close();
	delete m_pFileStream;
	m_pFileStream = 0;
}
