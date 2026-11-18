/*
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
    class CList : public CCollection
    {
        protected:
            TList<CObject *>    m_lstElements;
        public:
                                CList( void );
            virtual            ~CList( void );

                                DEFINE_OBJECT( CList )

            virtual size_t      GetLength( void ) const { return m_lstElements.Length(); }

                // Each returns the index the object went to (NOT_FOUND, for
                // AddBefore/AddAfter, if pValue is not in the list).
            virtual size_t      AddFirst( CObject *pNew );
            virtual CObject    *RemoveFirst( void );
            virtual size_t      AddLast( CObject *pNew );
            virtual CObject    *RemoveLast( void );
            virtual size_t      AddAt( size_t nIndex, CObject *pNew );
            virtual size_t      AddBefore( CObject *pValue, CObject *pNew );
            virtual size_t      AddAfter( CObject *pValue, CObject *pNew );
                // Takes the element at nIndex out (the caller owns it).
            virtual CObject    *RemoveAt( size_t nIndex );

            virtual CObject    *GetAt( size_t nIndex ) const;
            virtual CObject    *GetFirst( void ) const;
            virtual CObject    *GetLast( void ) const;
                // The index of pValue (the same pointer), or -1.
            virtual int         Find( CObject *pValue ) const;

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );

            const TList<CObject *> &GetList( void ) const { return m_lstElements; }

        private:
                                CList( const CList & );
            CList              &operator =( const CList & );
    }; // End of class CList
} // End of Namespace IASLib

#endif // IASLIB_LIST_H__
