/*
 * CCollection Class
 *
 *  This abstract base class is common to all of the collection classes
 * and all of the derived classes must implement the base interface.
 *
 *  The classic collections hold CObject pointers and own what they
 * hold: DeleteAll() (and the destructor) delete the objects, while
 * EmptyAll() lets go of them without deleting. Because they own their
 * objects, the ones that hold CObject pointers cannot be copied. For
 * collections of values, or with type checking, use the templates
 * (Templates/TCollections.h), which these are built on.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
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
        public:
                                CCollection( void ) {}
            virtual            ~CCollection( void ) {}

                                DEFINE_OBJECT( CCollection )

            virtual size_t      GetLength( void ) const = 0;
            size_t              Length( void ) const { return GetLength(); }
            size_t              GetCount( void ) const { return GetLength(); }
            size_t              Count( void ) const { return GetLength(); }
            bool                IsEmpty( void ) const { return GetLength() == 0; }

                // A new iterator (the caller deletes it). Changing the
                // collection while iterating gives undefined results.
            virtual CIterator  *Enumerate( void ) = 0;

            virtual void        DeleteAll( void ) = 0;
            virtual void        EmptyAll( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_COLLECTION_H__
