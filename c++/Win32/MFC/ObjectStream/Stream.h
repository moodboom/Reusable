






class Stream
{
public:
	Stream(
		int nOpenTimeoutMS		= 1000
		int nCloseTimeoutMS		= 1000
	)
	:
		// Init vars.
		m_nOpenCount		( 0 ),
		m_nOpenTimeoutMS	( nOpenTimeoutMS	),
		m_nCloseTimeoutMS	( nCloseTimeoutMS	)
	{
	}

	virtual ~Stream()
	{
		// If the user did not close up,
		// do so for her.
		if ( m_nOpenCount > 0 )
			Disconnect();
	}
	
	virtual bool Open()
	{
		ASSERT( m_nOpenCount >= 0 );

		if ( m_nOpenCount == 0 )
			Connect();

		m_nOpenCount++;
	}

	virtual bool Close()
	{
		ASSERT( m_nOpenCount > 0 );

		m_nOpenCount--;

		if ( m_nOpenCount == 0 )
			Disconnect();
	}

	bool bIsConnected() { return m_nOpenCount > 0; }

	// Workhorses
	virtual bool GetData(       void *buf, unsigned int length ) = 0;
	virtual bool PutData( const void *buf, unsigned int length ) = 0;

protected:

	// Only start writing after all these are released
	bool NotifyReadAccess()
	{ 
		if ( m_nReadAccesses == 0 )
			set read event

		m_nReadAccesses++; 
	}

	bool UnnotifyReadAccess()
	{ 
		ASSERT( m_nReadAccesses > 0 );
		m_nReadAccesses--; 

		if ( m_nReadAccesses == 0 )
			release read event
	}

	// Dont allow any reading while this is going on
	bool ReserveWriteAccess()
	{
		while ( m_nReadAccesses > 0 )
			;	
			
		block reading
	}

	bool UnreserveWriteAccess()
	{
		release block
	}

	virtual bool Connect() = 0;

	virtual bool Disconnect() = 0;

	int m_nOpenCount;

	int m_nOpenTimeoutMS;
	int m_nCloseTimeoutMS;

}



	this type of blocking will be required in derived classes:

	virtual bool SafeRead()
	{
		// We have to wait if a write is in progress.
		if write event unavailable
			wait

		set bReading event

		// Now we read.
		return Read();
	}

	virtual bool SafeWrite()
	{
		// Block reads and writes during write.
		GetEvent();

		// Write.
		bool bReturn = Write();

		// Release block.
		release event

		return bReturn;
	}

