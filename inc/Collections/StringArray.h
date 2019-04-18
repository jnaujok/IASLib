/*
 * String Array Class
 *
 *  This class provides a dynamically re-sizable array for use in
 * storing strings, specifically CStrings.
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

#ifndef IASLIB_STRINGARRAY_H__
#define IASLIB_STRINGARRAY_H__

#include "../BaseTypes/String_.h"
#include "Collection.h"

namespace IASLib
{
    class CStringArray : public CCollection
    {
        protected:
            CString           **m_apElements;
            size_t              m_nSize;
            size_t              m_nScale;
        public:
                                CStringArray( size_t nScale = 4 );
            virtual            ~CStringArray( void );

                                DEFINE_OBJECT( CStringArray )

            virtual size_t      Push( CString pNew );
            virtual CString     Get( size_t nCount ) const;
            virtual CString    &operator []( size_t nCount ) const;
            virtual size_t      Insert( size_t nCount, CString pNew );
            virtual bool        Delete( size_t nCount );

            virtual bool        Sort( bool bAscending );
            virtual void        QuickSort( size_t nLo, size_t nHi, bool bAscending );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );
        private:
            void                Resize( bool bIncrease = true );
    };
} // namespace IASLib

#endif // IASLIB_STRINGARRAY_H__

