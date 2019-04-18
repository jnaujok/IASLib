/*
 * Int Array Class
 *
 *  This class provides a dynamically re-sizable array for use in
 * storing integers.
 *  As more elements are added to the array, the array is
 * dynamically increased in size to hold the additional elements.
 * As elements are removed, the array is also made smaller to
 * more efficently use memory.
 *  The rate at which an array grows can be adjusted by setting
 * the "Scale" value.
 *  The array supports sorting via a comparison function that
 * must be passed to the Sort function. (c.f. CSortedArray)
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_INTARRAY_H__
#define IASLIB_INTARRAY_H__

#include "Collection.h"

namespace IASLib
{
    class CIntArray : public CCollection
    {
        protected:
            int                *m_apElements;
            size_t              m_nSize;
            size_t              m_nScale;
        public:
                                CIntArray( size_t nScale = 4 );
            virtual            ~CIntArray( void );

                                DEFINE_OBJECT( CIntArray )

            virtual size_t      Push( int );
            virtual int         Get( size_t nCount );
            virtual int        &operator []( size_t nCount );
            virtual size_t      Insert( size_t nCount, int );
            virtual bool        Delete( size_t nCount );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );
        private:
            void                Resize( bool bIncrease = true );
    };
} // namespace IASLib

#endif
