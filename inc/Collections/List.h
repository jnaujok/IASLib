/**
 * List Class
 *
 *  This class provides a dynamic linked list for storing pointers
 * to objects derived from CObject. The linked list is singly
 * linked and features O(1) insert and delete times. However, all
 * finds, insert at, and other such operations are O(n).
 *  Linked lists, more than any other data type, can grow to fill
 * all available memory space and are more efficient at using memory
 * since their elements do not need to be contiguous in memeory,
 * like an array.
 *  This class represents a singly linked list, to use a doubly linked
 * list, use the CDList class.
 *  To store objects other than CObject pointers, use the template
 * version of this class TList.
 *  Note: Unlike a typical singly linked list, we do store a "Last"
 * pointer to the last item in the array to make addition at either end
 * of the array an O(1) operation. That's really the only reason it's
 * there, honestly.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 10/26/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_LIST_H__
#define IASLIB_LIST_H__

#include "../BaseTypes/Object.h"
#include "Collection.h"

namespace IASLib
{
        // Forward reference to internal object
    class CListNode;

    class CList : public CCollection
    {
        protected:
            CListNode          *m_pFirst;
            CListNode          *m_pLast;
        public:
                                CList( void );
            virtual            ~CList( void );

                                DEFINE_OBJECT( CList )

            virtual size_t      AddFirst( CObject *pNew );
            virtual CObject    *RemoveFirst( void );
            virtual size_t      AddLast( CObject *pNew );
            virtual CObject    *RemoveLast( void );

            virtual size_t      AddAt( size_t nIndex, CObject *pNew );
            virtual size_t      AddBefore( CObject *pValue, CObject *pNew );
            virtual size_t      AddAfter( CObject *pValue, CObject *pNew );

            virtual CObject    *GetAt( size_t nIndex );
            virtual CObject    *GetFirst( void );
            virtual CObject    *GetLast( void );

            virtual int         Find( CObject *pValue );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );

    }; // End of class CList
} // End of Namespace IASLib

#endif // IASLIB_LIST_H__
