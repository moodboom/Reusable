//-------------------------------------------------------------------//
// fileHelpers
//-------------------------------------------------------------------//
//	Copyright © 2001 A better Software.
//-------------------------------------------------------------------//

#ifndef FILE_HELPERS_H
	#define FILE_HELPERS_H

#include <fstream>                  // For file streaming

#include "ustring.h"


bool MakeDir( ustring& strPath );
bool bFileExists( ustring& strFilename );


class LogRotator
{
public:
   
   LogRotator();

   bool Initialize(
      ustring  str_log_path                           ,
      ustring  str_log_filename                       ,
      ustring  str_log_extension                      = _T("log"),
      int      n_number_of_logs_to_archive            = 100
   );
   
   virtual ~LogRotator();

   void AddToLog( ustring& strMsg );
   
protected:

   ustring get_archive_name( int n_log_number );

   // -----
   // Settings

   ustring m_str_log_path;
   ustring m_str_log_filename;
   ustring m_str_log_extension;

   // -----


   // Internal
   std::ofstream* m_pFileStream;

};


#endif		// FILE_HELPERS_H
