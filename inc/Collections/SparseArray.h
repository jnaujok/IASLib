/**
 * Sparse Array Class
 *
 *  This class provides an array for storing sparse items in an array,
 * in other words, items that may be highly separated in array space. 
 * This class does this in a memory efficient means. There is a slight,
 * four byte, overhead per element for storage, but there is no penalty
 * no matter how large the array becomes. So, you could have ten elements
 * populated over a range of 10,000,000 elements, and this class would
 * only have an overhead of 40 bytes.
 *  Access to elements is O(log n) where n is the number of currently 
 * inserted elements.
 *  This class stores pointers to any object derived from CObject.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2/16/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SPARSEARRAY_H__
#define IASLIB_SPARSEARRAY_H__

#include "SortedArray.h"

namespace IASLib
{
    class CSparseArray;

    class CSparseArrayIterator : public CIterator
    {
        protected:
            CSparseArray       *m_pArray;
            size_t              m_nCurrentPos;
            size_t              m_nSparseNumber;
        public:
                                CSparseArrayIterator( CSparseArray *pArray ) { m_nSparseNumber = 0; m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CSparseArrayIterator, CIterator )
            virtual            ~CSparseArrayIterator( void ) {}
            virtual CObject    *Next( void );
            virtual CObject    *Prev( void );

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;

            virtual size_t      GetSparseNumber( void ) { return m_nSparseNumber; }
    };


    class CSparseArray : public CSortedArray
    {
        friend class CSparseArrayIterator;

        protected:
            static int Compare( const CObject *pLeft, const CObject *pRight );

        public:
                                CSparseArray( size_t nGrowBy = 16 );
            virtual            ~CSparseArray( void );

                                DECLARE_OBJECT(CSparseArray, CSortedArray);

            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Prepend( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Push( CObject *pNew );

            virtual size_t      Insert( size_t nCount, CObject *pNew );

            virtual CObject    *Get( size_t nCount ) const;
            virtual CObject    *operator []( size_t nCount ) const;

            virtual CObject    *Remove( size_t nCount );
            virtual bool        Delete( size_t nCount );

			virtual bool		Swap( size_t nSwap1, size_t nSwap2 );

            virtual bool        Exists( size_t nCount );

            virtual CIterator  *Enumerate( void );
    };



} // namespace IASLib

#endif // IASLIB_SPARSEARRAY_H__
