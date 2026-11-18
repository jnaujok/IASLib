/*
 * CSortedArray Class
 *
 *  An array kept in order.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "SortedArray.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSortedArray, CArray );

#ifdef IASLIB_MULTI_THREADED__
    #define IASLIB_ARRAY_LOCK__     CMutexLock oLock__( m_mutex )
#else
    #define IASLIB_ARRAY_LOCK__     do { } while ( 0 )
#endif

    CSortedArray::CSortedArray( int (*fnCompare)(const CObject *, const CObject *), size_t nScale )
        : CArray( nScale )
    {
        m_fnCompare = fnCompare;
    }

    CSortedArray::~CSortedArray( void )
    {
    }

    size_t CSortedArray::LowerBound( const CObject *pSearch ) const
    {
        IASLIB_ARRAY_LOCK__;
        size_t nLow = 0, nHigh = m_aElements.Length();
        while ( nLow < nHigh )
        {
            size_t nMiddle = nLow + ( nHigh - nLow ) / 2;
            if ( m_fnCompare( m_aElements[ nMiddle ], pSearch ) < 0 )
                nLow = nMiddle + 1;
            else
                nHigh = nMiddle;
        }
        return nLow;
    }

    size_t CSortedArray::UpperBound( const CObject *pSearch ) const
    {
        size_t nLow = 0, nHigh = m_aElements.Length();
        while ( nLow < nHigh )
        {
            size_t nMiddle = nLow + ( nHigh - nLow ) / 2;
            if ( m_fnCompare( m_aElements[ nMiddle ], pSearch ) <= 0 )
                nLow = nMiddle + 1;
            else
                nHigh = nMiddle;
        }
        return nLow;
    }

    size_t CSortedArray::Push( CObject *pNew )
    {
        IASLIB_ARRAY_LOCK__;
        return m_aElements.Insert( UpperBound( pNew ), pNew );
    }

    size_t CSortedArray::Set( size_t nIndex, CObject *pSet )
    {
        {
            IASLIB_ARRAY_LOCK__;
            if ( nIndex < m_aElements.Length() )
                m_aElements.Remove( nIndex );
        }
        return Push( pSet );
    }

    size_t CSortedArray::Find( const CObject *pSearch ) const
    {
        size_t nIndex = LowerBound( pSearch );
        IASLIB_ARRAY_LOCK__;
        if ( nIndex < m_aElements.Length() && m_fnCompare( m_aElements[ nIndex ], pSearch ) == 0 )
            return nIndex;
        return NOT_FOUND;
    }

    CIterator *CSortedArray::Enumerate( void )
    {
        return new CArrayIterator( this );
    }
} // End of Namespace IASLib
