/*
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
    class CArray : public CCollection
    {
        protected:
            TArray<CObject *>   m_aElements;
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex      m_mutex;
#endif
        public:
                                CArray( size_t nScale = 4 );
            virtual            ~CArray( void );

                                DEFINE_OBJECT( CArray );

            virtual size_t      GetLength( void ) const;

            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Prepend( CObject *pNew ) { return Insert( 0, pNew ); }

                // Inserts before nIndex (at the end if past it), returning
                // the index it went to.
            virtual size_t      Insert( size_t nIndex, CObject *pNew );
            virtual size_t      Push( CObject *pNew );

                // Replaces the element at nIndex without deleting the old one
                // (past the end, appends), returning the index used.
            virtual size_t      Set( size_t nIndex, CObject *pSet );

            virtual CObject    *Get( size_t nIndex ) const;
            virtual CObject    *operator []( size_t nIndex ) const;

                // The index of pFind (the same pointer), or NOT_FOUND.
            virtual size_t      Find( const CObject *pFind ) const;

                // Takes the element out (the caller owns it), or NULL.
            virtual CObject    *Remove( size_t nIndex );
                // Takes the element out and deletes it.
            virtual bool        Delete( size_t nIndex );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

                // Sorts with fnCompare (negative, zero or positive, as
                // strcmp), keeping equal elements in order.
            virtual void        Sort( int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallbackData );

            virtual void        SetScale( size_t nScale ) { m_aElements.SetScale( nScale ); }
            void                Reserve( size_t nCapacity );
            void                Compact( void );

            virtual bool        Swap( size_t nSwap1, size_t nSwap2 );

            virtual CIterator  *Enumerate( void );

        private:
                                CArray( const CArray & );
            CArray             &operator =( const CArray & );
    }; // End of class CArray

    class CArrayIterator : public CIterator
    {
        protected:
            const CArray       *m_pArray;
            size_t              m_nCurrentPos;
        public:
                                CArrayIterator( const CArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CArrayIterator, CIterator )
            virtual            ~CArrayIterator( void ) {}
            virtual CObject    *Next( void );
            virtual CObject    *Prev( void );

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;
    };

} // End of Namespace IASLib

#endif
