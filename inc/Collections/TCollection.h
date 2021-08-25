/**
 * TCollection Template
 * 
 * This template provides a base class for all iterable collections.
 * 
 *  Author: Jeffrey R. Naujok
 * Created: August 27, 2019
 * 
 * Copyright (c) 2019, Irene Adler Software. All Rights Reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TCOLLECTION_H__
#define IASLIB_TCOLLECTION_H__

#include "../BaseTypes/Object.h"

namespace IASLib
{
    template <class T> 
    class TCollection
    {
        // Ensures that this collection contains the specified element (optional operation).
        virtual bool	add(T *e);

        // Adds all of the elements in the specified collection to this collection (optional operation).
        virtual bool	addAll(TCollection<T> c);

        // Removes all of the elements from this collection (optional operation).
        virtual void	clear( void );

        // Returns true if this collection contains the specified element.
        virtual bool    contains( T *o );

        // Returns true if this collection contains all of the elements in the specified collection.
        virtual bool	containsAll(TCollection<T> c);

        // Compares the specified object with this collection for equality.
        virtual bool	equals(TCollection<T> c);

        // Returns the hash code value for this collection.
        virtual int	    hashCode();

        // Returns true if this collection contains no elements.
        virtual bool	isEmpty();

        // Returns an iterator over the elements in this collection.
        virtual TIterator<T>	iterator( void );

        // Removes a single instance of the specified element from this collection, if it is present (optional operation).
        virtual bool	remove(T *o);

        // Removes all of this collection's elements that are also contained in the specified collection (optional operation).
        virtual bool	removeAll(TCollection<T> c);

        // Retains only the elements in this collection that are contained in the specified collection (optional operation).
        virtual bool	retainAll(TCollection<T> c);

        // Returns the number of elements in this collection.
        virtual int	    size( void );

        // Returns an array containing all of the elements in this collection.
        virtual T**	    toArray();
    };
}

#endif // IASLIB_TCOLLECTION_H__