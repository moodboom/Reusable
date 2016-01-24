//-------------------------------------------------------------------//
// AppInstaller.h interface															//
//-------------------------------------------------------------------//

#ifndef APP_INSTALLER_H
#define APP_INSTALLER_H

class AppInstaller
{

public:

	// Our constructor.
	AppInstaller();

	// Clean up here.
	~AppInstaller();

	bool VerifyInstallation(
		AppRegistryData*		pRegData,
		CCommandLineInfo*		pNewCommandLine,
		CPropertySheet*		pInstallWizard,
		bool						bForceOneInstance	= true
	);

protected:

	void CleanUp();

	// Override in derived class.
	virtual	int	nSharedMemorySize()	= 0;
	virtual	void	SetSharedMemory()		= 0;

	void RunInstalledVersion(
		bool bUpdate = false
	);

	CString	m_strExeName;
	CString	m_strCurrentPath;
	CString	m_strRegistryPath;
	
	CCommandLineInfo*		pCommandLine;

	// For our "IPC" needs, allows one instance, with communication between.	
	CSharedMemory* m_pMem;
	
	// Again for IPC, this thread detects messages from another process.
	OtherInstanceNotificationThread*	m_pOtherInstancesThread;

};

/////////////////////////////////////////////////////////////////////////////

#endif	// APP_INSTALLER_H
