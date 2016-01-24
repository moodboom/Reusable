

class StreamObject
{

public:

	StreamObject( Stream* pStream = 0 )
	:
		// Init vars.
		m_pStream( pStream )
	{
	}

	virtual ~Stream()
	{}

	virtual bool Connect( Stream* pStream )
	{
		m_pStream = pStream;
		return true;
	}
	
	virtual bool Read()
	{ 
		// You DID connect this object to a stream first, right?
		ASSERT( m_pStream );

		return Read( m_pStream );
	}

	virtual bool Write()
	{ 
		// You DID connect this object to a stream first, right?
		ASSERT( m_pStream );

		return Write( m_pStream );
	}

	virtual bool Read( Stream* pStream ) = 0;
	virtual bool Write( Stream* pStream ) = 0;

	bool ReadData( void *buf, unsigned int length )
	{
		// Read requested data from stream.
		return m_pStream->GetData( buf, length );
	}

	bool WriteData( const void *buf, unsigned int length )
	{
		// Write requested data to stream.
		return m_pStream->PutData( buf, length );
	}


protected:
	Stream* m_pStream;

}

