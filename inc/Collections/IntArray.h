/*
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
    class CIntArray : public CCollection
    {
        protected:
            TArray<int>         m_aElements;
            int                 m_nScratch;
        public:
                                CIntArray( size_t nScale = 4 );
            virtual            ~CIntArray( void );

                                DEFINE_OBJECT( CIntArray )

            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }

            virtual size_t      Push( int nValue );
            virtual int         Get( size_t nIndex ) const;
            virtual int        &operator []( size_t nIndex );
            virtual int         operator []( size_t nIndex ) const { return Get( nIndex ); }
            virtual size_t      Insert( size_t nIndex, int nValue );
            virtual bool        Delete( size_t nIndex );
            virtual size_t      Find( int nValue ) const { return m_aElements.Find( nValue ); }
            virtual void        Sort( bool bAscending = true );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );
    };
} // namespace IASLib

#endif
