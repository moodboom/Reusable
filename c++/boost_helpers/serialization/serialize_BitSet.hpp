// NOTE that it is assumed that you have included boost serialization headers before this point...


// MDM In order to stream our BitSet class we need a serialize() function.
// Let's try putting this one outside the BitSet class.  That's called
// the "non-intrusive formulation".  The only problem is that you
// obviously need access to all the internal data - here, BitSet::GetBitData()
// does that for us, whee!
namespace boost {
namespace serialization {

template<class Archive>
inline void serialize(Archive & ar, BitSet & bs, const unsigned int version)
{
   unsigned int* pnData = 0;

   // Archive the number of bits, then the actual long values that hold them.
   // We'll do it this way:
   //    First archive an int for the BitSet size.
   //    Next, archive the required number of ints, based on the depth.
   
   int nCount = 0xAA;    // Dummy it up.
   int nDepth = 0xAA;

   // If we're saving, we need to extract the data first.
   if ( Archive::is_saving::value )
   {
      nCount = bs.nGetSize();
      pnData = bs.GetBitData();
	  nDepth = bs.GetBitDataDepth();
   }
	
   ar & nCount;

   // If we're loading, we need to allocate.
   if ( Archive::is_loading::value ) 
   {
      bs.SetSize( nCount );
      pnData = bs.GetBitData();
	  nDepth = bs.GetBitDataDepth();
   }

   // Now handle the values.
   int nA;
   for ( nA = 0; nA < nDepth; nA++ )
   {
	  ar & pnData[nA];
   }
}

} // namespace serialization
} // namespace boost
