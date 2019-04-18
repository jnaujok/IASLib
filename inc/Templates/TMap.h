/**
 * Map Template
 *
 *  This template creates a mapping of keys K, to values T. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/15/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TMAP_H__
#define IASLIB_TMAP_H__

#include "TCollection.h"
#include "TEntry.h"

namespace IASLib
{
    template<class K, class V>
    class TMap : public TCollection<V>
    {
        protected:


        public:
                                TCollection( void );
            virtual            ~TCollection( void );
            
                                DEFINE_OBJECT( TCollection )

            virtual size_t      GetLength( void ) const { return m_nElements; }
            virtual size_t      Length( void ) const { return m_nElements; }
            virtual size_t      GetCount( void ) const { return m_nElements; }
            virtual size_t      Count( void ) const { return m_nElements; }

            virtual TIterator<T>  *Enumerate( void ) = 0;

            virtual void        DeleteAll( void ) = 0;
            virtual void        EmptyAll( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_COLLECTION_H__
