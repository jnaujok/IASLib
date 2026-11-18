/**
 * @file List.h
 * @brief CList, an owning doubly linked list of CObject pointers.
 *
 * CList Class
 *
 *  A doubly linked list of CObject pointers (owned, as in CArray). The
 * Get and Remove methods give NULL when there is no such element. For a
 * type safe list, or one of values, use TList.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_LIST_H__
#define IASLIB_LIST_H__

#include "Collection.h"
#include "../Templates/TList.h"

namespace IASLib
{
    /**
     * A doubly linked list of CObject pointers (owned, as in CArray).
     *
     * The Get and Remove methods give NULL when there is no such element.
     * Not thread-safe, and not copyable. For a type safe list, or one of
     * values, use TList.
     *
     * @code
     * CList lst;
     * CString *pB = new CString( "b" );
     * lst.AddLast( pB );
     * lst.AddBefore( pB, new CString( "a" ) );     // "a", "b"
     * delete lst.RemoveFirst();                    // removed: ours to delete
     * @endcode
     */
    class CList : public CCollection
    {
        protected:
            TList<CObject *>    m_lstElements;  ///< The elements.
        public:
                                /// An empty list.
                                CList( void );
                                /// Deletes every element.
            virtual            ~CList( void );

                                DEFINE_OBJECT( CList )

                /// The number of elements.
            virtual size_t      GetLength( void ) const { return m_lstElements.Length(); }

                /// Adds pNew at the front, returning its index (0).
                ///
                /// Each of the Add methods returns the index the object went to
                /// (NOT_FOUND, for AddBefore/AddAfter, if pValue is not in the
                /// list). The list owns the object.
            virtual size_t      AddFirst( CObject *pNew );
                /// Takes the first element out (the caller owns it), or NULL.
            virtual CObject    *RemoveFirst( void );
                /// Adds pNew at the back, returning its index.
            virtual size_t      AddLast( CObject *pNew );
                /// Takes the last element out (the caller owns it), or NULL.
            virtual CObject    *RemoveLast( void );
                /// Inserts pNew before nIndex, returning its index.
            virtual size_t      AddAt( size_t nIndex, CObject *pNew );
                /// Inserts pNew before pValue, returning its index (NOT_FOUND,
                /// adding nothing, if pValue is not in the list).
            virtual size_t      AddBefore( CObject *pValue, CObject *pNew );
                /// Inserts pNew after pValue, returning its index (NOT_FOUND,
                /// adding nothing, if pValue is not in the list).
            virtual size_t      AddAfter( CObject *pValue, CObject *pNew );
                /// Takes the element at nIndex out (the caller owns it), or NULL.
            virtual CObject    *RemoveAt( size_t nIndex );

                /// The element at nIndex (still owned by the list), or NULL.
            virtual CObject    *GetAt( size_t nIndex ) const;
                /// The first element, or NULL.
            virtual CObject    *GetFirst( void ) const;
                /// The last element, or NULL.
            virtual CObject    *GetLast( void ) const;
                /// The index of pValue (the same pointer), or -1.
            virtual int         Find( CObject *pValue ) const;

                /// Removes and deletes every element.
            virtual void        DeleteAll( void );
                /// Removes every element without deleting it.
            virtual void        EmptyAll( void );

                /// A new iterator, front to back (the caller deletes it).
            virtual CIterator  *Enumerate( void );

                /// The underlying template list.
            const TList<CObject *> &GetList( void ) const { return m_lstElements; }

        private:
                                /// Not copyable (the list owns its elements).
                                CList( const CList & );
                                /// Not assignable.
            CList              &operator =( const CList & );
    }; // End of class CList
} // End of Namespace IASLib

#endif // IASLIB_LIST_H__
