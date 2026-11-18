/**
 * @file IntArray.h
 * @brief CIntArray, an array of ints with the classic collection interface.
 *
 * CIntArray Class
 *
 *  An array of ints (a TArray<int> with the classic interface). Get()
 * gives 0 past the end, and operator [] a scratch value that is not part
 * of the array. Copies are independent.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_INTARRAY_H__
#define IASLIB_INTARRAY_H__

#include "Collection.h"
#include "../Templates/TArray.h"

namespace IASLib
{
    /**
     * An array of ints (a TArray<int> with the classic interface).
     *
     * Get() gives 0 past the end, and the non-const operator [] a scratch
     * value that is not part of the array. Copies are independent. Not
     * thread-safe. Its iterator gives the values through
     * CIterator::NextInt() (Next() only steps and returns NULL).
     *
     * @code
     * CIntArray aNumbers;
     * aNumbers.Push( 3 );
     * aNumbers.Push( 1 );
     * aNumbers.Sort();                 // 1, 3
     * int nFirst = aNumbers[ 0 ];      // 1
     * @endcode
     */
    class CIntArray : public CCollection
    {
        protected:
            TArray<int>         m_aElements;    ///< The values.
            int                 m_nScratch;     ///< What operator [] gives past the end (-1 each time).
        public:
                                /// An empty array.
                                /// @param nScale The least it grows by (0 means 4).
                                CIntArray( size_t nScale = 4 );
            virtual            ~CIntArray( void );

                                DEFINE_OBJECT( CIntArray )

                /// The number of values.
            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }

                /// Adds a value at the end, returning its index.
            virtual size_t      Push( int nValue );
                /// The value at nIndex, or 0 past the end.
            virtual int         Get( size_t nIndex ) const;
                /// The value at nIndex, to change; past the end, a scratch
                /// value (set to -1) that is not part of the array.
            virtual int        &operator []( size_t nIndex );
                /// Same as Get().
            virtual int         operator []( size_t nIndex ) const { return Get( nIndex ); }
                /// Inserts before nIndex (at the end if past it), returning the
                /// index it went to.
            virtual size_t      Insert( size_t nIndex, int nValue );
                /// Removes the value at nIndex; false past the end.
            virtual bool        Delete( size_t nIndex );
                /// The index of the first nValue, or NOT_FOUND.
            virtual size_t      Find( int nValue ) const { return m_aElements.Find( nValue ); }
                /// Sorts the values, ascending or descending.
            virtual void        Sort( bool bAscending = true );

                /// Removes every value.
            virtual void        DeleteAll( void );
                /// Removes every value (same as DeleteAll()).
            virtual void        EmptyAll( void );

                /// A new iterator (the caller deletes it); read it with NextInt().
            virtual CIterator  *Enumerate( void );
    };
} // namespace IASLib

#endif
