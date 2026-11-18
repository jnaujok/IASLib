/*
 * CSortedArray Class
 *
 *  An array of CObject pointers (owned, as in CArray) kept in the order
 * of a comparison function given to the constructor (negative, zero or
 * positive, as strcmp). Every way of adding (Push, Insert, Append,
 * Prepend, Set) puts the object where it belongs, after any equal ones,
 * and Find() is a binary search. Swap() and Sort() are refused, since
 * they would break the order.
 *
 *  For a type safe sorted array, use TSortedArray.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
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
                                CSortedArray( int (*fnCompare)(const CObject *, const CObject *), size_t nScale = 4 );
            virtual            ~CSortedArray( void );

                                DEFINE_OBJECT( CSortedArray )

                // Inserts in order, returning the index it went to.
            virtual size_t      Push( CObject *pNew );
            virtual size_t      Insert( size_t, CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Prepend( CObject *pNew ) { return Push( pNew ); }
                // Replaces the element at nIndex (without deleting it) and
                // moves the new one to its place; returns where it went.
            virtual size_t      Set( size_t nIndex, CObject *pSet );

                // The index of an element comparing equal to pSearch (the
                // first such), or NOT_FOUND.
            virtual size_t      Find( const CObject *pSearch ) const;
                // Where pSearch would go: the first index whose element is
                // not before it.
            virtual size_t      LowerBound( const CObject *pSearch ) const;

            virtual bool        Swap( size_t, size_t ) { return false; }
            virtual void        Sort( int (*)(const CObject *, const CObject *, void *), void * ) {}

            virtual CIterator  *Enumerate( void );

        protected:
            size_t              UpperBound( const CObject *pSearch ) const;
    }; // End of Class CSortedArray
} // End of Namespace IASLib

#endif
