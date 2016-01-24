#ifndef LIMIT_SINGLE_INSTANCE_H
#define LIMIT_SINGLE_INSTANCE_H

#include <windows.h> 

// NOTE: This code is from MSDN, Tech note Q243953.

class CLimitSingleInstance
{
protected:
  DWORD  m_dwLastError;
  HANDLE m_hMutex;

public:
  CLimitSingleInstance(TCHAR *strMutexName)
  {
    //be sure to use a name that is unique for this application otherwise
    //two apps may think they are the same if they are using same name for
    //3rd parm to CreateMutex
    m_hMutex = CreateMutex(NULL, FALSE, strMutexName); //do early
    m_dwLastError = GetLastError(); //save for use later...
  }
   
  ~CLimitSingleInstance() 
  {
    if (m_hMutex)  //don't forget to close handles...
    {
       CloseHandle(m_hMutex); //do as late as possible
       m_hMutex = NULL; //good habit to be in
    }
  }

  BOOL IsAnotherInstanceRunning() 
  {
    return (ERROR_ALREADY_EXISTS == m_dwLastError);
  }
};

#endif // LIMIT_SINGLE_INSTANCE_H