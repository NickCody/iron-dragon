//=----------------------------------------------------------------------------
//
// class CBitArray
//
// Nomenclature: 
//    bitPos      0-31  index of a particular bit in a DWORD
//    bitVirtual,
//    bitV     0-n   where n is our listbox array count, virtual index
//                   of a bit within our entire array
//    elem     0-m   where m is the number of DWORD's needed to store
//                   the number of listbox elements, usually n / 32
//
//
class CBitArray
{
public:
   //=------------------------------------------
   // Method CBitArray
   //
   CBitArray( int bitv = 0 )
      {
         m_rgBitArray = NULL;
         m_bitvs   = bitv;
         m_elems   = 0;
         m_nNumSet    = 0;
         m_fRecalcNumset  = false;

         EnsureCapacity( bitv );
      }

   //=------------------------------------------
   // Method ~ CBitArray
   //
   ~ CBitArray()
      {
         if (m_rgBitArray)
            delete [] m_rgBitArray;
      }

   //=------------------------------------------
   // Method GetSize
   //
   int GetSize() const
   {
      return m_bitvs;
   }

   //=------------------------------------------
   // Method SetSize
   //
   void SetSize( int bitv )
   {
      EnsureCapacity( bitv );
      m_bitvs = bitv;
   }

   //=------------------------------------------
   // Method GetValue
   //
   bool GetValue( int bitv ) const
      {
         return get_at_bitv( bitv );
      }

   //=------------------------------------------
   // Method SetValue
   //
   bool SetValue( int bitv, bool fValue )
      {
         return set_at_bitv( bitv, fValue );
      }

   //=------------------------------------------
   // Method GetFirstSet
   //
   int GetFirstSet( ) const
      {
         int nRet = 0;
         int elemStop = elem_from_bitv(m_bitvs)+1;
         for( int i=0; (i < elemStop) && (nRet < m_bitvs); i++, nRet+=32 )
         {
            if( m_rgBitArray[i] != 0 )
            {
               for( int j=0; (j < 32) && (nRet < m_bitvs); j++, nRet++ )
               {
                  if( (1 << j) & m_rgBitArray[i] )
                     return nRet;
               }

                  // something is wrong, we should have found a bit
               return -1;
            }
         }

         return -1;
      }

   //=------------------------------------------
   // Method GetNumSet
   //
   int GetNumSet( )
      {

         if (m_fRecalcNumset)
         {
            int m_nNumSet = 0;
            int nRet = 0;
            int elemStop = elem_from_bitv(m_bitvs)+1;
            for( int i=0; (i < elemStop) && (nRet < m_bitvs); i++, nRet+=32 )
            {
               if( m_rgBitArray[i] != 0 )
               {
                  for( int j=0; (j < 32) && (nRet < m_bitvs); j++, nRet++ )
                  {
                     if( (1 << j) & m_rgBitArray[i] )
                        m_nNumSet++;
                  }
               }
            }
            m_fRecalcNumset  = false;
         }

         return m_nNumSet;
      }

   //=------------------------------------------
   // Method SetValueRange
   //
   bool SetValueRange( int bitvStart, int bitvEnd, bool fValue )
      {
         //
         // bugbug : find the reason why the optimal code below does not work...
         //
         for( int i=bitvStart; i <= bitvEnd; i++ )
            set_at_bitv( i, fValue );

         return true;
         //
         // validate parameters
         //
         if( bitvStart < 0 || bitvStart >= m_bitvs )
            return false;

         if( bitvEnd < bitvStart || bitvEnd >= m_bitvs )
            return false;
         
         //          +------------------ bitvStart
         //          |        
         //          |           +-- bitvEnd
         //          |   *inner* |
         //          V  Vvvvvvvvv   V
         //     |DDDD|DDDD|DDDD|DDDD|DDDD|
         // bitv   0123 4567 8901 2345 6789
         // elem  0   1   2    3 4
         //               ^    ^
         //               |    |
         //               |    +-------------innerEnd
         //               |
         //               +----------------- innerStart
         //    
         // We optimally set bits in *inner* DWORDs, but call 
         //   set_at_bitv for DWORDS that start and end bitv's are located in
         //

         int innerStart = elem_from_bitv( bitvStart ) + 1;
         int innerEnd   = elem_from_bitv( bitvEnd   ) - 1;
         if( innerStart <= innerEnd )
            memset( &m_rgBitArray[innerStart], fValue ? 0xff : 0, (innerEnd-innerStart+1)*(sizeof DWORD) );

            //
            // set bits in DWORD that bitvStart is located in
            //
         int bitvMarker = min( (innerStart-1)*32-1, bitvEnd);
         for( i=bitvStart; i <= bitvMarker; i++ )
            set_at_bitv( i, fValue );

            //
            // set bits in DWORD that bitvEnd is located in, ignore if -1
            //

         if( innerEnd+1 > 0 )
         {
            bitvMarker = (innerEnd+1)*32;
            for( i=bitvMarker; i <= bitvEnd; i++ )
               set_at_bitv( i, fValue );
         }

         m_fRecalcNumset  = true;

         return true;
      }

   //=------------------------------------------
   // Method SetValueAll
   //
   bool SetValueAll( bool fValue )
      {
         if (fValue)
         {
            if (m_bitvs > 7)
               memset( m_rgBitArray, 0xFF, (m_bitvs / 8 ) );
            for (int i = (m_bitvs / 8) * 8; i < m_bitvs; i++)
               set_at_bitv( i, fValue );
            m_nNumSet = m_bitvs;
         }  
         else
         {
            memset( m_rgBitArray, 0, m_elems*(sizeof DWORD) );
            m_nNumSet = 0;
         }

         m_fRecalcNumset  = false;

         return true;
      }

   //=------------------------------------------
   // Method EnsureCapacity
   //
   // @param 
   // @return
   //
   bool EnsureCapacity( int bitv )
   {
         //
         // check if the size to ensure is already encapsulated
         //
      if( bitv > m_elems*32 )
      {
         int newElems = (elem_from_bitv(bitv)+ 1)*2; // *2 from +1 ! NickC 07/07/97
         DWORD* rgNewItems = new DWORD[newElems];
         if( rgNewItems == NULL )
            return false;

         if( m_rgBitArray && m_elems)
         {
            memcpy( rgNewItems, m_rgBitArray, m_elems*(sizeof DWORD) );
            delete [] m_rgBitArray;
            memset( &rgNewItems[m_elems], 0, (sizeof DWORD) * (newElems - m_elems) );
         }
         else
            memset( rgNewItems, 0, (sizeof DWORD) * newElems );

         m_rgBitArray = rgNewItems;
         m_elems   = newElems;
      }

      return true;
   }


   //=------------------------------------------
   // Method InsertAt
   //
   // @param 
   // @return
   //
   // @notes  
   //
   bool InsertAt( int bitv, bool fValue )
      {
         int elem   = elem_from_bitv( bitv );
         int bitpos = bitpos_from_bitv( bitv );

            //
            // check to make sure we insert in closed range [0...m_elems*32-1]
            //
         if( elem > m_elems || bitpos > 32 )
            return false;

         return insert_at( elem, bitpos, fValue );
      }

   //=------------------------------------------
   // Method RemoveAt
   //
   // @param 
   // @return
   //
   // @notes  
   //
   bool RemoveAt( int bitv )
      {
         int elem   = elem_from_bitv( bitv );
         int bitpos = bitpos_from_bitv( bitv );

         return remove_at( elem, bitpos );
      }

private:
   //=------------------------------------------
   // Method elem_from_bitv
   //
   // @param      virtual bit position
   // @return     dword index (elem) bitv resides in
   //
   int elem_from_bitv( int bitv ) const
      {
         return bitv / 32;
      }
   
   //=------------------------------------------
   // Method bitpos_from_elem
   //
   // @param      virtual bit position (bitv)
   // @return     relative bit position (bitpos)
   //
   int bitpos_from_bitv( int bitv ) const
      {
         return bitv % 32;
      }

   //=------------------------------------------
   // Method bit_from_pos
   //
   // @param      elem, dword to extract bit from
   //          bitpos, bit position of bit to test for true/falsehood
   // @return     the value of [elem:bitpos] 0 ? false : true
   //
   bool get_at_bitv( int bitv ) const
      {
         int elem   = elem_from_bitv  ( bitv );
         int bitpos = bitpos_from_bitv( bitv );

         if( elem >= m_elems || elem < 0 )
            return false;

         DWORD x    = m_rgBitArray[elem];
         DWORD mask = 1 << bitpos;
         DWORD temp = x & mask;
         temp = temp >> bitpos;
         return temp != 0;
      }

   //=------------------------------------------
   // Method bit_from_pos
   //
   // @param      
   // @return     
   //
   bool set_at_bitv( int bitv, bool fSet )
      {
         int elem   = elem_from_bitv  ( bitv );
         int bitpos = bitpos_from_bitv( bitv );

         return set_at_bitv( elem, bitpos, fSet );
      }
   //=------------------------------------------
   // Method bit_from_pos
   //
   // @param      
   // @return     
   //
   bool set_at_bitv( int elem, int bitpos, bool fSet )
      {

         if( elem >= m_elems || bitpos > 32 )
            return false;

         DWORD mask = fSet ? (1 << bitpos) : 0xffffffff - (1 << bitpos);
         DWORD dwNewBitArray = fSet
            ? m_rgBitArray[elem] | mask
            : m_rgBitArray[elem] & mask;

         if (dwNewBitArray != m_rgBitArray[elem])
         {
            m_rgBitArray[elem]   = dwNewBitArray;
            if (fSet)
               m_nNumSet++;
            else
               m_nNumSet--;
         }

         return true;
      }

   //=------------------------------------------
   // Method insert_at
   //
   // @param 
   // @return
   //
   // @notes  Sort of complex part... (example of inserting at 0-based bitpos=2, 
   //                            n = value to set)
   //
   //    x  newBit   prev  result   afterMask   x1    FINAL
   //
   //    b0   0    b0   b0       0     0      b0
   //    b1   0    b1   b1       0     0      b1
   //    b2   n    0    n        0     0      n      carry contains b31
   //    b3   0    0    0        1     b2     b2
   //    ...     ...  ...  ...      ...      ...    ...
   //    b31     0    0    0        1     b30    b30
   //
   //
   bool insert_at( int elem, int bitpos, bool fSet )
      {
         EnsureCapacity( m_bitvs+1 );

         if( elem >= m_elems || bitpos > 32 )
            return false;

         if (GetNumSet() < 1)
         {
            m_bitvs++;
            if (fSet)
               set_at_bitv( elem, bitpos, fSet );

            return true;
         }
            //
            // Sort of complex part
            //
         DWORD x      = m_rgBitArray[elem];
         DWORD newBit    = fSet ? 0x01 << bitpos : 0;
         DWORD prev      = bitpos 
                        ? (x << (32-(bitpos))) >> (32-(bitpos))
                        : 0;
         DWORD result    = newBit | prev;
         DWORD afterMask  = 0xffffffff << (bitpos+1);
         DWORD x1     = (x << 1) & afterMask;
         result |= x1;
         m_rgBitArray[elem] = result;

            //
            // for each DWORD remaining, quickly shift 1 up, and or with carry
            //
         DWORD carry = x >> 31;
         for( int i=elem+1; i < m_elems; i++ )
         {
            DWORD dx = m_rgBitArray[i];

               // optimize for case where all 32-bits are zero & carry is 0
               //
            if( dx == 0 && carry == 0 )
               continue;

            m_rgBitArray[i] = (m_rgBitArray[i] << 1) | carry;
            carry = (dx >> 31);
         }

            //
            // capacity is already increased, just update our total valid bit count
            //
         m_bitvs++;

         return 0; // return the end bit which we lost...
      }

   //=------------------------------------------
   // Method remove_at
   //
   // @param 
   // @return
   //
   // @notes  Sort of complex part... (example of inserting at 0-based bitpos=2, 
   //                            n = value to set)
   //
   //     x   downshift  lowerMask        upperMask           x1   x2    last     
   //          (x>>1)    ((0xffffffff>>30) ((0xffffffff>>2)<<2) (x&lm) (ds&um)
   //     ----------------------------------------------------------------------------
   //     b0   b1       1            0               b0    0    0
   //     b1   b2       1            0               b1    0    0 
   // del->b2  b3       0            1                0    b3   0   final = x1|x2|last 
   //     b3   b4       0            1                0    b4   0 
   //     ...  ...      ...             ...             ...   ... ...
   //     b31  0        0            1                0    0    b'0
   //
   //
   bool remove_at( int elem, int bitpos )
      {

            //
            // check to make sure we remove a valid bit
            //
         if( elem > m_elems || bitpos > 32 )
            return false;

         if (GetNumSet() < 1)
         {
            m_bitvs--;
            return true;
         }

         // JJD 7/28/97 first turn bit off
         //
         set_at_bitv( elem, bitpos, false );

         DWORD last = elem >= m_elems ? 0 : (m_rgBitArray[elem+1]>>31);

         if( bitpos )
         {
            DWORD x  = m_rgBitArray[elem];
            DWORD ds = x >> 1;
            DWORD lm = 0xffffffff >> (32-(bitpos+1));
            DWORD um = (0xffffffff >> (bitpos+1)) << (bitpos=1);
            DWORD x1 = x & lm;
            DWORD x2 = ds & um;

            // final
            m_rgBitArray[elem] =  x1 | x2 | last;
         }
         else
            m_rgBitArray[elem] =  (m_rgBitArray[elem] >> 1) | last;
         
         for( int i=elem+1; i < m_elems; i++ )
         {
            last  = (i+1) >= m_elems ? 0 : (m_rgBitArray[(i+1)]>>31);
            m_rgBitArray[i] = (m_rgBitArray[i] >> 1) | last;
         }

         m_bitvs--;

         return true;

      }

private:
   DWORD* m_rgBitArray;
   int    m_bitvs;         // number of bits in use
   int    m_elems;         // number of DWORD's in array
   int    m_nNumSet;       // number of bits set on
   BYTE   m_fRecalcNumset;    // need to recalc number of bits set

};
