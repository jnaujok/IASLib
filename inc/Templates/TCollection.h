/**
 * Collection Template
 *
 *  This abstract base class is common to all of the collection classes
 * and all of the derived classes must implement the base interface.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 4/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TCOLLECTION_H__
#define IASLIB_TCOLLECTION_H__

#include "../BaseTypes/Object.h"

namespace IASLib
{
    // Forward declaration of the the Iterator template class.
    template<class T>
    class TIterator;


    template<class T>
    class TCollection : public CObject
    {
        protected:
            size_t              m_nElements;

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
