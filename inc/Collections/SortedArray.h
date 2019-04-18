/**
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

#ifndef IASLIB_SORTEDARRAY_H__
#define IASLIB_SORTEDARRAY_H__

#include "Array.h"

namespace IASLib
{
    class CSortedArray : public CArray
    {
        protected:
            int                 (*m_fnCompare)(const CObject *, const CObject *);
        public:
                                CSortedArray( int (*fnCompare)(const CObject *, const CObject *),size_t nScale = 4 );
            virtual            ~CSortedArray( void );

                                DEFINE_OBJECT( CSortedArray )

            virtual size_t      Push( CObject *pNew );
                                virtual size_t      Insert( size_t nCount, CObject *pNew ) { nCount=nCount; return CSortedArray::Push(pNew); }
            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Prepend( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Find( CObject *pSearch ) const { return (GetLength()) ? FindIndex( pSearch, 0, m_nElements ) : IASLib::NOT_FOUND; }

			virtual bool		Swap( size_t nSwap1, size_t nSwap2 ) { nSwap1=nSwap2; return false; }

            virtual CIterator  *Enumerate( void );

        protected:
            virtual size_t      FindIndex( CObject *pSearch, size_t nLow, size_t nHigh ) const;
    }; // End of Class CSortedArray
} // End of Namespace IASLib

#endif
