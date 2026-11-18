/**
 * @file Array.h
 * @brief CArray, an owning array of CObject pointers, and its iterator.
 *
 * CArray Class
 *
 *  An array of CObject pointers, which the array owns (see CCollection).
 * It grows as elements are added, and gives back room as they are
 * removed (see Templates/TCommon.h); SetScale() sets the least it grows
 * by. Get() and operator [] give NULL past the end. It is thread-safe:
 * each call locks the array (but a sequence of calls is not atomic, and a
 * Sort() comparison must not call back into the array).
 *
 *  For a type safe array, or an array of values, use TArray.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ARRAY_H__
#define IASLIB_ARRAY_H__

#include "Collection.h"
#include "../Templates/TArray.h"
#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif

namespace IASLib
{
    /**
     * An array of CObject pointers, which the array owns (see CCollection).
     *
     * It grows as elements are added, and gives back room as they are
     * removed (see Templates/TCommon.h); SetScale() sets the least it grows
     * by. Get() and operator [] give NULL past the end. It is thread-safe:
     * each call locks the array (but a sequence of calls is not atomic, and
     * a Sort() comparison must not call back into the array). It cannot be
     * copied. For a type safe array, or an array of values, use TArray.
     *
     * @code
     * CArray aNames;
     * aNames.Push( new CString( "b" ) );
     * aNames.Prepend( new CString( "a" ) );
     * CString *pFirst = (CString *)aNames[ 0 ];    // "a"; still owned by the array
     * delete aNames.Remove( 1 );                   // taken out: now ours to delete
     * @endcode
     */
    class CArray : public CCollection
    {
        protected:
            TArray<CObject *>   m_aElements;    ///< The elements.
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex      m_mutex;        ///< Locked by every call.
#endif
        public:
                                /// An empty array.
                                /// @param nScale The least it grows by (0 means 4).
                                CArray( size_t nScale = 4 );
                                /// Deletes every element.
            virtual            ~CArray( void );

                                DEFINE_OBJECT( CArray );

                /// The number of elements.
            virtual size_t      GetLength( void ) const;

                /// Adds pNew at the end (same as Push()), returning its index.
            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
                /// Adds pNew at the start, returning its index (0).
            virtual size_t      Prepend( CObject *pNew ) { return Insert( 0, pNew ); }

                /// Inserts before nIndex (at the end if past it), returning
                /// the index it went to. The array owns pNew.
            virtual size_t      Insert( size_t nIndex, CObject *pNew );
                /// Adds pNew at the end, returning its index. The array owns pNew.
            virtual size_t      Push( CObject *pNew );

                /// Replaces the element at nIndex without deleting the old one
                /// (past the end, appends), returning the index used.
            virtual size_t      Set( size_t nIndex, CObject *pSet );

                /// The element at nIndex (still owned by the array), or NULL
                /// past the end.
            virtual CObject    *Get( size_t nIndex ) const;
                /// Same as Get().
            virtual CObject    *operator []( size_t nIndex ) const;

                /// The index of pFind (the same pointer), or NOT_FOUND.
            virtual size_t      Find( const CObject *pFind ) const;

                /// Takes the element out (the caller owns it), or NULL past
                /// the end.
            virtual CObject    *Remove( size_t nIndex );
                /// Takes the element out and deletes it; false past the end.
            virtual bool        Delete( size_t nIndex );

                /// Removes and deletes every element.
            virtual void        DeleteAll( void );
                /// Removes every element without deleting it.
            virtual void        EmptyAll( void );

                /// Sorts with fnCompare (negative, zero or positive, as
                /// strcmp), keeping equal elements in order. A NULL
                /// fnCompare does nothing.
                /// @param fnCompare The comparison; it gets pCallbackData as
                ///     its third argument and must not call into the array.
                /// @param pCallbackData Passed to fnCompare.
            virtual void        Sort( int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallbackData );

                /// Sets the least the array grows by.
            virtual void        SetScale( size_t nScale ) { m_aElements.SetScale( nScale ); }
                /// Makes room for nCapacity elements.
            void                Reserve( size_t nCapacity );
                /// Gives back unused room.
            void                Compact( void );

                /// Swaps two elements; false if either index is past the end,
                /// or they are the same.
            virtual bool        Swap( size_t nSwap1, size_t nSwap2 );

                /// A new CArrayIterator over the array (the caller deletes it).
            virtual CIterator  *Enumerate( void );

        private:
                                /// Not copyable (the array owns its elements).
                                CArray( const CArray & );
                                /// Not assignable.
            CArray             &operator =( const CArray & );
    }; // End of class CArray

    /// The iterator CArray::Enumerate() gives: the elements in index order.
    class CArrayIterator : public CIterator
    {
        protected:
            const CArray       *m_pArray;       ///< The array (not owned).
            size_t              m_nCurrentPos;  ///< The index of the next element.
        public:
                                /// An iterator at the start of pArray.
                                CArrayIterator( const CArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CArrayIterator, CIterator )
            virtual            ~CArrayIterator( void ) {}
                /// The next element, or NULL at the end.
            virtual CObject    *Next( void );
                /// Steps back and returns that element, or NULL at the start.
            virtual CObject    *Prev( void );

                /// Back to the start.
            virtual void        Reset( void ) { m_nCurrentPos = 0; }
                /// Whether there is a next element.
            virtual bool        HasMore( void ) const;
    };

} // End of Namespace IASLib

#endif
