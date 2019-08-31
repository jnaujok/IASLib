/*
 * Cache Class
 *
 *  This class provides an LRU cache for storing objects derived 
 * from CObject. As new items are cached, those that were used 
 * last are dropped from the cache. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/2004
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Cache.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CCache, CCollection );

	CCache::CCache( size_t nMaxEntries, bool bUseExpiration )
		: m_hashEntries( CHash::LARGE )
	{
        m_pListFirst = NULL;
        m_pListLast = NULL;
		m_nQueueMax = nMaxEntries;
        m_bUseExpiration = bUseExpiration;
	}

  	CCache::~CCache( void )
	{
        DeleteAll();
	}

    bool CCache::AddItem( CCacheItem *pItem )
    {
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
            // Make sure there is room in the Cache
        while ( ( m_pListLast ) && ( m_hashEntries.GetLength() >= m_nQueueMax ) )
        {
            CCacheItem *pOldest = m_pListLast;

            m_pListLast = pOldest->m_pPrevItem;
            m_pListLast->m_pNextItem = NULL;

            pOldest->m_pPrevItem = NULL;

            m_hashEntries.Delete( pOldest->GetKey() );
        }

        m_hashEntries.Push( pItem->GetKey(), pItem );
        pItem->m_pNextItem = m_pListFirst;
        m_pListFirst = pItem;
        pItem->m_pPrevItem = NULL;
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif

        return true;
    }

    bool CCache::RemoveItem( CCacheItem *pKey )
    {
        bool        bRetVal = false;
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        CCacheItem *pRemove = (CCacheItem *)m_hashEntries.Get( pKey->GetKey() );

        if ( pRemove )
        {
            RemoveFromList( pRemove );
            m_hashEntries.Delete( pKey->GetKey() );

            bRetVal = true;
        }

#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
        return bRetVal;
    }

    CCacheItem *CCache::Get( CCacheItem *pKey )
    {
        CCacheItem *pRetVal = NULL;        

#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        pRetVal = (CCacheItem *)m_hashEntries.Get( pKey->GetKey() );

        if ( pRetVal )
        {
            if ( m_bUseExpiration )
            {
                if ( pRetVal->IsExpired() )
                {
                    RemoveFromList( pRetVal );
                    m_hashEntries.Delete( pKey->GetKey() );
                    pRetVal = NULL;
                }
            }

            // It wasn't expired            
            if ( pRetVal )
            {
                pRetVal->Hit( this );
            }
        }

#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
        return pRetVal;
    }

    bool CCache::Touch( CCacheItem *pKey )
    {
        bool        bRetVal = false;
        CCacheItem *pItem = NULL;        
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        pItem = (CCacheItem *)m_hashEntries.Get( pKey->GetKey() );

        if ( pItem )
        {
            pItem->Hit( this );

            bRetVal = true;
        }

#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
        return bRetVal;
    }

    bool CCache::Discard( CCacheItem *pKey )
    {
        bool        bRetVal = false;
        CCacheItem *pItem = NULL;        
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        pItem = (CCacheItem *)m_hashEntries.Get( pKey->GetKey() );

        if ( pItem )
        {
            MoveToTail( pItem );
            bRetVal = true;
        }
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
        return bRetVal;
    }

    void CCache::ClearExpired( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        CCacheItem *pItem = m_pListFirst;
        while ( pItem )
        {
            if ( pItem->IsExpired() )
            {
                RemoveFromList(pItem);
                m_hashEntries.Delete( pItem->GetKey() );
            }
            pItem = pItem->m_pNextItem;
        }

#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
    }

    void CCache::ClearExpired( CDate &dttCompare )
    {
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        CCacheItem *pItem = m_pListFirst;
        while ( pItem )
        {
            if ( pItem->IsExpired( dttCompare ) )
            {
                RemoveFromList(pItem);
                m_hashEntries.Delete( pItem->GetKey() );
            }
            pItem = pItem->m_pNextItem;
        }
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
    }

    void CCache::DeleteAll( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        DetatchList( );
		m_hashEntries.DeleteAll();
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
    }

    void CCache::EmptyAll( void )
    {
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Lock();
#endif
        DetatchList( );
		m_hashEntries.EmptyAll();
#ifdef IASLIB_MULTI_THREADED__
        cacheMutex.Unlock();
#endif
    }

    CIterator *CCache::Enumerate( void )
    {
        return m_hashEntries.Enumerate();
    }

    void CCache::MoveToHead( CCacheItem *pItem )
    {
        // Skip if already at the head of the list.
        if ( pItem->m_pPrevItem )
        {
            // Move to head of LRU list
            CCacheItem *pTemp = m_pListFirst;
            m_pListFirst = pItem;
            if ( pItem->m_pNextItem )
            {
                pItem->m_pNextItem->m_pPrevItem = pItem->m_pPrevItem;
            }
            else
            {
                // At the tail of the list, so set the tail to the prior item
                m_pListLast = pItem->m_pPrevItem;
            }
            
            pItem->m_pPrevItem->m_pNextItem = pItem->m_pNextItem;
            pItem->m_pPrevItem = NULL;
            pItem->m_pNextItem = pTemp;
        }
    }

    void CCache::MoveToTail( CCacheItem *pItem )
    {
        if ( pItem->m_pNextItem )
        {
            CCacheItem *pTemp = m_pListLast;
            m_pListLast = pItem;

            pItem->m_pNextItem->m_pPrevItem = pItem->m_pPrevItem;

            if ( pItem->m_pPrevItem )
            {
                pItem->m_pPrevItem->m_pNextItem = pItem->m_pNextItem;
            }
            else
            {
                m_pListFirst = pItem->m_pNextItem;
            }

            if ( pTemp )
            {
                pTemp->m_pNextItem = pItem;
            }
            pItem->m_pPrevItem = pTemp;
            pItem->m_pNextItem = NULL;
        }
    }

    void CCache::RemoveFromList( CCacheItem *pRemove )
    {
        // Remove the item from the LRU list.
        if ( pRemove->m_pPrevItem )
        {
            pRemove->m_pPrevItem->m_pNextItem = pRemove->m_pNextItem;
        }
        else
        {
            m_pListFirst = pRemove->m_pNextItem;
        }

        if ( pRemove->m_pNextItem )
        {
            pRemove->m_pNextItem->m_pPrevItem = pRemove->m_pPrevItem;
        }
        else
        {
            m_pListLast = pRemove->m_pPrevItem;
            if ( pRemove->m_pPrevItem )
            {
                pRemove->m_pPrevItem->m_pNextItem = NULL;
            }
        }
    }

    void CCache::DetatchList( void )
    {
        CCacheItem *pItem;

        pItem = m_pListFirst;

        while ( pItem )
        {
            m_pListFirst = pItem->m_pNextItem;
            pItem->m_pNextItem = NULL;
            pItem->m_pPrevItem = NULL;
            
            pItem = m_pListFirst;
        }

        m_pListFirst = NULL;
        m_pListLast = NULL;
    }
} // namespace IASLib

