/**
 * Array Class
 *
 *  This class provides a dynamically re-sizable array for use in
 * storing CObject derived pointers. Any object that has been
 * derived from the CObject type can be stored in this array.
 *  As more elements are added to the array, the array is
 * dynamically increased in size to hold the additional elements.
 * As elements are removed, the array is also made smaller to
 * more efficently use memory.
 *  The rate at which an array grows can be adjusted by setting
 * the "Scale" value.
 *  The array supports sorting via a comparison function that
 * must be passed to the Sort function. (c.f. CSortedArray)
 *  An array simply takes derived objects and makes no attempt
 * to correctly cast the returns to their appropriate sub-class
 * type.  That is left up to the user. To use a type safe version
 * of this class, you should use the TArray template class instead
 * of this class.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ARRAY_H__
#define IASLIB_ARRAY_H__

#include "Collection.h"

namespace IASLib
{
    class CArray : public CCollection
    {
        protected:
            CObject           **m_apElements;
            size_t              m_nSize;
            size_t              m_nScale;
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex      m_mutex;
#endif
        public:
                                CArray( size_t nScale = 4 );
                                CArray( const CArray &oSource );
            virtual            ~CArray( void );

                                DEFINE_OBJECT( CArray );

            virtual size_t      Append( CObject *pNew ) { return Push( pNew ); }
            virtual size_t      Prepend( CObject *pNew ) { return Insert( 0, pNew ); }

            virtual size_t      Insert( size_t nIndex, CObject *pNew );

            virtual size_t      Push( CObject *pNew );

            virtual size_t      Set( size_t nIndex, CObject *pSet );

            virtual CObject    *Get( size_t nIndex ) const;
            virtual CObject    *operator []( size_t nIndex ) const;

            virtual CObject    *Remove( size_t nIndex );

            virtual bool        Delete( size_t nIndex );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual void        Sort( int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallbackData );

            virtual void        SetScale( size_t nScale ) { if ( nScale > 0 ) m_nScale = nScale; }

			virtual bool		Swap( size_t nSwap1, size_t nSwap2 );

            virtual CIterator  *Enumerate( void );
        protected:
            void                Resize( bool bIncrease = true );

            void                QuickSort( size_t nLow, size_t nHigh, int (*fnCompare)(const CObject *, const CObject *, void *), void *pCallback );
    }; // End of class CArray

    class CArrayIterator : public CIterator
    {
        protected:
            CArray             *m_pArray;
            size_t              m_nCurrentPos;
        public:
                                CArrayIterator( CArray *pArray ) { m_pArray = pArray; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CArrayIterator, CIterator )
            virtual            ~CArrayIterator( void ) {}
            virtual CObject    *Next( void );
            virtual CObject    *Prev( void );

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;
    };

} // End of Namespace IASLib

#endif
