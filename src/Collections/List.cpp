/*
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

#ifdef IASLIB_WIN32
#include "stdafx.h"
#endif

#include <memory>
#include "List.h"

namespace IASLib
{
    class CListIterator : public CIterator
    {
        protected:
            CList          *m_pList;
            size_t          m_nCurrentPos;
        public:
                                CListIterator( CList *pList ) { m_pList = pList; m_nCurrentPos = 0; }
                                DECLARE_OBJECT( CListIterator, CIterator )
            virtual            ~CListIterator( void ) {}
            virtual CObject    *Next( void );
            virtual CObject    *Prev( void );

            virtual void        Reset( void ) { m_nCurrentPos = 0; }
            virtual bool        HasMore( void ) const;
    };

    CObject *CListIterator::Next( void )
    {
        CObject *pRetVal = NULL;

        if ( m_nCurrentPos < m_pList->GetLength() )
        {
            pRetVal = m_pList->GetAt( m_nCurrentPos );
            m_nCurrentPos++;
        }

        return pRetVal;
    }

    CObject *CListIterator::Prev( void )
    {
        CObject *pRetVal = NULL;

        if ( ( m_nCurrentPos > 0 ) && ( m_nCurrentPos <= m_pList->GetLength() ) )
        {
            m_nCurrentPos--;
            pRetVal = m_pList->GetAt( m_nCurrentPos );
        }

        return pRetVal;
    }

    bool CListIterator::HasMore( void ) const
    {
        bool bRetVal = false;

        if ( m_nCurrentPos < m_pList->GetLength() )
        {
            bRetVal = true;
        }

        return bRetVal;
    }

        // Forward reference to internal object
    class CListNode : public CObject
    {
        protected:
            CListNode  *m_pNext;
            CObject    *m_pData;
        public:
                        CListNode( CObject *pObject, CListNode *pNext = NULL );
            virtual    ~CListNode( void );

                        DEFINE_OBJECT( CListNode )

            CListNode  *GetNext( void ) { return m_pNext; }
            void        SetNext( CListNode *pNext ) { m_pNext = pNext; }

            CObject    *GetData( void ) { return m_pData; }
            void        SetData( CObject *pData ) { m_pData = pData; }

            void        Empty( void );

            bool        IsLast( void ) { return (bool)(m_pNext == NULL); }
    };

    IMPLEMENT_OBJECT( CListNode, CObject );

    CListNode::CListNode( CObject *pData, CListNode *pNext )
    {
        m_pData = pData;
        m_pNext = pNext;
    }

    CListNode::~CListNode( void )
    {
        if ( m_pData )
            delete m_pData;
        m_pData = NULL;
        m_pNext = NULL;
    }

    void CListNode::Empty( void )
    {
        m_pData = NULL;
    }

    IMPLEMENT_OBJECT( CList, CObject );

    CList::CList( void )
    {
        m_pFirst = NULL;
        m_pLast = NULL;
        m_nElements = 0;
    }

    CList::~CList( void )
    {
        DeleteAll();
    }

    size_t CList::AddFirst( CObject *pNew )
    {
        CListNode   *pNewNode = new CListNode( pNew, m_pFirst );
        if ( m_pFirst == NULL )
            m_pLast = pNewNode;
        m_pFirst = pNewNode;
        m_nElements++;
        return 0;
    }

    CObject *CList::RemoveFirst( void )
    {
        CListNode *pRetVal = NULL;
        CObject   *pRet = NULL;

        if ( m_pFirst )
        {
            pRetVal = m_pFirst;
            m_pFirst = pRetVal->GetNext();
            if ( m_pFirst == NULL )
                m_pLast = NULL;
            if ( m_pFirst->GetNext() == NULL )
                m_pLast = m_pFirst;
        }

        if ( pRetVal )
        {
            pRet = pRetVal->GetData();
            pRetVal->Empty();
            delete pRetVal;
            m_nElements--;
        }

        return pRet;
    }

    size_t CList::AddLast( CObject *pNew )
    {
        CListNode  *pNewNode = new CListNode( pNew, NULL );

        if ( m_pLast )
        {
            m_pLast->SetNext( pNewNode );
            m_pLast = pNewNode;
        }
        else
        {
            m_pLast = pNewNode;
            m_pFirst = pNewNode;
        }

        m_nElements++;

        return m_nElements;
    }

    CObject *CList::RemoveLast( void )
    {
        CListNode  *pRetNode = NULL;
        CListNode  *pTrailer = NULL;
        CObject    *pRetVal = NULL;

        pRetNode = m_pFirst;
        while ( pRetNode != m_pLast )
        {
            pTrailer = pRetNode;
            pRetNode = pRetNode->GetNext();
        }

        if ( pRetNode )
        {
            if ( pTrailer )
            {
                pTrailer->SetNext( NULL );
            }
            else
            {
                m_pFirst = NULL;
            }
            m_pLast = pTrailer;

            pRetVal = pRetNode->GetData();
            pRetNode->Empty();
            delete pRetNode;
            m_nElements--;
        }

        return pRetVal;
    }

    size_t CList::AddAt( size_t nIndex, CObject *pNew )
    {
        size_t nCount = 0;
        CListNode  *pNextNode = m_pFirst;
        CListNode  *pLastNode = NULL;
        CListNode  *pNewNode = new CListNode( pNew, NULL );

        while ( ( pNextNode ) && ( nCount < nIndex ) )
        {
            pLastNode = pNextNode;
            pNextNode = pNextNode->GetNext();
            nCount++;
        }

        if ( pNextNode )
        {
            pNewNode->SetNext( pNextNode );
        }
        else
        {
            m_pLast = pNewNode;
        }

        if ( pLastNode )
        {
            pLastNode->SetNext( pNewNode );
        }
        else
        {
            m_pFirst = pNewNode;
        }

        m_nElements++;

        return nCount;
    }

    size_t CList::AddBefore( CObject *pValue, CObject *pNew )
    {
        CListNode  *pNextNode = m_pFirst;
        CListNode  *pTrailer = NULL;
        CListNode  *pNewNode = new CListNode( pNew, NULL );
        size_t      nCount = 0;

        while ( ( pNextNode ) && ( pNextNode->GetData() != pValue ) )
        {
            pTrailer = pNextNode;
            pNextNode = pNextNode->GetNext();
            nCount++;
        }

        if ( pNextNode )
        {
            pNewNode->SetNext( pNextNode );
        }
        else
        {
            m_pLast = pNewNode;
        }

        if ( pTrailer )
        {
            pTrailer->SetNext( pNewNode );
        }
        else
        {
            m_pFirst = pNewNode;
        }

        m_nElements++;

        return nCount;
    }

    size_t CList::AddAfter( CObject *pValue, CObject *pNew )
    {
        CListNode  *pNextNode = m_pFirst;
        CListNode  *pTrailer = NULL;
        CListNode  *pNewNode = new CListNode( pNew, NULL );
        size_t      nCount = 0;

        while ( ( pNextNode ) && ( pNextNode->GetData() != pValue ) )
        {
            pTrailer = pNextNode;
            pNextNode = pNextNode->GetNext();
            nCount++;
        }

        if ( pNextNode )
        {
            if ( pNextNode->GetData() == pValue )
            {
                pTrailer = pNextNode;
                pNextNode = pNextNode->GetNext();
                nCount++;
            }
        }

        if ( pNextNode )
        {
            pNewNode->SetNext( pNextNode );
        }
        else
        {
            m_pLast = pNewNode;
        }

        if ( pTrailer )
        {
            pTrailer->SetNext( pNewNode );
        }
        else
        {
            m_pFirst = pNewNode;
        }

        m_nElements++;

        return nCount;
    }

    CObject *CList::GetAt( size_t nIndex )
    {
        size_t nCount = 0;
        CListNode  *pNextNode = m_pFirst;

        while ( ( pNextNode ) && ( nCount < nIndex ) )
        {
            pNextNode = pNextNode->GetNext();
            nCount++;
        }

        if ( pNextNode )
        {
            return pNextNode->GetData();
        }

        return NULL;
    }

    CObject *CList::GetFirst( void )
    {
        if ( m_pFirst )
        {
            return m_pFirst->GetData();
        }

        return NULL;
    }

    CObject *CList::GetLast( void )
    {
        if ( m_pLast )
        {
            return m_pLast->GetData();
        }

        return NULL;
    }

    int CList::Find( CObject *pValue )
    {
        CListNode  *pNextNode = m_pFirst;
        int         nCount = 0;

        while ( ( pNextNode ) && ( pNextNode->GetData() != pValue ) )
        {
            pNextNode = pNextNode->GetNext();
            nCount++;
        }

        if ( pNextNode )
        {
            return nCount;
        }

        return -1;
    }

    void CList::DeleteAll( void )
    {
        CListNode *pNext = m_pFirst;
        CListNode *pLast = NULL;

        while ( pNext )
        {
            pLast = pNext;
            pNext = pNext->GetNext();

            delete pLast;
            pLast = NULL;
        }

        if ( pLast )
            delete pLast;

        m_pFirst = NULL;
        m_pLast = NULL;
        m_nElements = 0;
    }

    void CList::EmptyAll( void )
    {
        CListNode *pNext = m_pFirst;
        CListNode *pLast = NULL;

        while ( pNext )
        {
            pLast = pNext;
            pNext = pNext->GetNext();
            pLast->Empty();
            delete pLast;
            pLast = NULL;
        }

        if ( pLast )
        {
            pLast->Empty();
            delete pLast;
        }

        m_pFirst = NULL;
        m_pLast = NULL;
        m_nElements = 0;
    }

    CIterator *CList::Enumerate( void )
    {
        return new CListIterator( this );
    }

} // End of namespace (IASLib)
