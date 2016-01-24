
template<class Archive>
inline void TestData::serialize(
 Archive & ar, 
 const unsigned int // file_version 
) {	
	ar & m_nTestBytes;
	ar & m_pTestData;
}


