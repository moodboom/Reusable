Steps to use sqlite in your project:
------------------------------------
1) Open sqlite\SQLite_Static_Library\SQLite_Static_Library.sln
2) You may need to change the runtime libraries of the project to match those of the executable that will include the libraries, in order to avoid linker errors.
3) Build the library.
4) Include it in your application (Project->Properties->Config->Linker->Input->Addl Dependencies->[sqlite\{build}\SQLite_Static_Library.lib]
5) Add the C++ wrapper to your project.  It's in the sqlite root dir:

      CppSQLite3.h
      CppSQLite3.cpp
      
   You will also need sqlite\sqlite-source\sqlite3.h
   but it is included in CppSQLite3.cpp, so no need to mess with it.

6) You are ready to write some code!  See the wrapper docs at http://www.codeproject.com/database/CppSQLite.asp.


NOTES: 

Last updated for sqlite-source-3_3_13.
Other files in the sqlite directory are...

   sqlite-source           the Windows-preprocessed "Source Code" from here: 
                                 http://sqlite.org/sqlite-source-3_3_13.zip
                                 
   SQLite_Static_Library   the actual static lib project, takes the source and makes a lib
   
