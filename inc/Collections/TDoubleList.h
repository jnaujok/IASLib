/**
 * TDoubleList Template
 * 
 * This template provides a means of storing any object in a doubly linked 
 * list. 
 * 
 *  Author: Jeffrey R. Naujok
 * Created: August 27, 2019
 * 
 * Copyright (c) 2019, Irene Adler Software. All Rights Reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DOUBLELIST_H__
#define IASLIB_DOUBLELIST_H__

#include "String_.h"
#include "TCollection.h"

namespace IASLib
{
    class CListItem : public CObject
    {
        public:
            DECLARE_OBJECT( CListItem, CObject )

            virtual int compare( const CListItem &oCompare ) const = 0;
    };

    template <class T> 
    class TDoubleList : public TCollection<T>
    {
        static_assert(std::is_base_of<T, CListItem>::value, "T must derive from CListItem");

        private:
            class TListElement
            {
                public:
                    T            *data;
                    TListElement *pNext;
                    TListElement *pPrev;
            };

        public:
            // Adds an element to the List at the end of the list.
            virtual bool    add( T *element );
            // Inserts an element into the list at the given position in the list. 
            virtual bool	add(int index, T *element);

            // Appends all of the elements in the specified collection to the end of this list, in the order that they are returned by the specified collection's iterator (optional operation).
            virtual bool	addAll( TCollection<T> list);
            // Inserts all of the elements in the specified collection into this list at the specified position (optional operation).
            virtual bool	addAll(int index, TCollection<T> c)

            // Removes all of the elements from this list (optional operation).
            virtual void	clear( void );

            // Returns true if this list contains the specified element.
            virtual bool	contains(T* element);
            // Returns true if this list contains all of the elements of the specified collection.
            virtual bool	containsAll(TCollection<T> c);

            // Compares the specified object with this list for equality.
            virtual bool	equals(TCollection<T> o);

            // Returns the element at the specified position in this list.
            virtual T*	    get(int index)

            // Returns the hash code value for this list.
            virtual int	hashCode( void );

            // Returns the index of the first occurrence of the specified element in this list, or -1 if this list does not contain the element.
            virtual int	indexOf(T* o);

            // Returns true if this list contains no elements.
            virtual bool	isEmpty( void );

            // Returns an iterator over the elements in this list in proper sequence.
            virtual TIterator<T>	iterator()

            // Returns the index of the last occurrence of the specified element in this list, or -1 if this list does not contain the element.
            virtual int	lastIndexOf(T* o);
            // Returns a list iterator over the elements in this list (in proper sequence).
            virtual TListIterator<T>	listIterator( void );

            // Returns a list iterator over the elements in this list (in proper sequence), starting at the specified position in the list.
            virtual TListIterator<T>	listIterator(int index);

            // Removes the element at the specified position in this list (optional operation).
            virtual T*   remove(int index);

            // Removes the first occurrence of the specified element from this list, if it is present (optional operation).
            virtual bool	remove(T* o);

            // Removes from this list all of its elements that are contained in the specified collection (optional operation).
            virtual bool	removeAll(TCollection<T*> c);

            // Retains only the elements in this list that are contained in the specified collection (optional operation).
            virtual bool	retainAll(TCollection<T*> c);

            // Replaces the element at the specified position in this list with the specified element (optional operation).
            virtual T*	set(int index, T* element);

            // Returns the number of elements in this list.
            virtual int	size( void );

            // Returns a view of the portion of this list between the specified fromIndex, inclusive, and toIndex, exclusive.
            virtual TList<T>	subList(int fromIndex, int toIndex);

            // Returns an array containing all of the elements in this list in proper sequence (from first to last element).
            virtual T** toArray();
    };

    template <class T>
    bool TDoubleList::put( T *value )
    {


    }
}

#endif // IASLIB_DOUBLELIST_H__       