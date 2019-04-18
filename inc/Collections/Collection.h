/**
 * Collection Class
 *
 *  This abstract base class is common to all of the collection classes
 * and all of the derived classes must implement the base interface.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_COLLECTION_H__
#define IASLIB_COLLECTION_H__

#include "../BaseTypes/Object.h"
#include "Iterator.h"

namespace IASLib
{
    class CCollection : public CObject
    {
        protected:
            size_t              m_nElements;

        public:
                                CCollection( void );
            virtual            ~CCollection( void );
            
                                DEFINE_OBJECT( CCollection )

            virtual size_t      GetLength( void ) const { return m_nElements; }
            virtual size_t      Length( void ) const { return m_nElements; }
            virtual size_t      GetCount( void ) const { return m_nElements; }
            virtual size_t      Count( void ) const { return m_nElements; }

            virtual CIterator  *Enumerate( void ) = 0;

            virtual void        DeleteAll( void ) = 0;
            virtual void        EmptyAll( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_COLLECTION_H__
