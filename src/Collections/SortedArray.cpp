/*
 * Sorted Array Class
 *
 *  This class provides an extension to the array class that
 * supports an array that is always in a sorted state. This
 * allows for somewhat slower insertions, but rapid retrieval
 * of an element from the array.
 *  A sorted array requires a sorting function be provided when
 * the Sorted Array object is instantiated. Note that no attempt
 * is made to assure that the correct type is passed to this
 * sorting function. That is left up to the user. To use a type
 * safe version of this class, you should use the TSortedArray
 * template class instead of this class.
 *  Note that, although this class supports all the interfaces
 * that CArray does, all of the insertion interfaces map to Push,
 * which inserts the object in the correct location in the array.
 * This assures that no matter what insertion method is used, the
 * element is always inserted in the correct location.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/24/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SortedArray.h"

namespace IASLib
{
	IMPLEMENT_OBJECT( CSortedArray, CArray );

	CSortedArray::CSortedArray( int (*fnCompare)(const CObject *, const CObject *), size_t nScale )
		: CArray( nScale )
	{
		m_fnCompare = fnCompare;
	}

	CSortedArray::~CSortedArray( void )
	{
		DeleteAll();
	}


	size_t CSortedArray::Push( CObject *pNew )
	{
		size_t nInsertPoint = Find( pNew );

      	if ( nInsertPoint >= ( GetCount() + 1 ) )
		    CArray::Push( pNew );
			return GetCount();

		return CArray::Insert( nInsertPoint, pNew );
	}

	size_t CSortedArray::FindIndex( CObject *pSearch, size_t nLow, size_t nHigh ) const
	{
		if ( nLow == nHigh )
		{
			return (size_t)(( -(int)nLow ) - 1);
		}

		size_t nMiddle = (nLow + nHigh) / 2;
		int    nResult;

		CObject *pCompare = m_apElements[ nMiddle ];

		nResult = (*m_fnCompare)( pCompare, pSearch );

		if ( nResult < 0 )
		{
         if ( ( nMiddle == nLow ) && ( nMiddle != nHigh ) )
         {
            nMiddle++;
         }
			return FindIndex( pSearch, nMiddle, nHigh );
		}

		if ( nResult > 0 )
		{
         if ( ( nMiddle == nHigh ) && ( nMiddle != nLow ) )
         {
            nMiddle--;
         }
			return FindIndex( pSearch, nLow, nMiddle );
		}

		return nMiddle;
	}

    CIterator *CSortedArray::Enumerate( void )
    {
        return new CArrayIterator( this );
    }

	bool CSortedArray::Swap( size_t swap1, size_t swap2 )
	{
		if ( ( swap1 != swap2 ) && ( swap1 < this->m_nElements ) && ( swap2 < m_nElements ) )
		{
			CObject *temp = m_apElements[swap1];
			m_apElements[swap1] = m_apElements[swap2];
			m_apElements[swap2] = temp;
			return true;
		}
		return false;
	}

} // End of Namespace IASLib

