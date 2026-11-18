/**
 * @file Collection.h
 * @brief CCollection, the base class of the classic (CObject pointer)
 * collections.
 *
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
    /**
     * The abstract base class of the classic collections.
     *
     * The classic collections hold CObject pointers and own what they hold:
     * DeleteAll() (and the destructor) delete the objects, while EmptyAll()
     * lets go of them without deleting. Because they own their objects, the
     * ones that hold CObject pointers cannot be copied. For collections of
     * values, or with type checking, use the templates
     * (Templates/TCollections.h), which these are built on.
     *
     * @code
     * CArray aItems;
     * aItems.Push( new CString( "one" ) );     // the array owns it now
     * CIterator *pIter = aItems.Enumerate();
     * while ( pIter->HasMore() )
     *     printf( "%s\n", ( (CString *)pIter->Next() )->c_str() );
     * delete pIter;                            // ~CArray deletes the string
     * @endcode
     */
    class CCollection : public CObject
    {
        public:
                                /// An empty collection.
                                CCollection( void ) {}
            virtual            ~CCollection( void ) {}

                                DEFINE_OBJECT( CCollection )

                /// The number of elements.
            virtual size_t      GetLength( void ) const = 0;
                /// The number of elements; same as GetLength().
            size_t              Length( void ) const { return GetLength(); }
                /// The number of elements; same as GetLength().
            size_t              GetCount( void ) const { return GetLength(); }
                /// The number of elements; same as GetLength().
            size_t              Count( void ) const { return GetLength(); }
                /// Whether there are no elements.
            bool                IsEmpty( void ) const { return GetLength() == 0; }

                /// A new iterator (the caller deletes it). Changing the
                /// collection while iterating gives undefined results.
            virtual CIterator  *Enumerate( void ) = 0;

                /// Removes every element and deletes it.
            virtual void        DeleteAll( void ) = 0;
                /// Removes every element without deleting it (the caller
                /// keeps them).
            virtual void        EmptyAll( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_COLLECTION_H__
