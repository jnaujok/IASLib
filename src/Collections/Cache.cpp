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
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Cache.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CCache, CCollection );

	CCache::CCache( size_t nMaxEntries, bool bUseExpiration )
		: m_aEntries( CCacheItem::SortCompare, ( nMaxEntries < 1024 ) ? nMaxEntries : 1024 )
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
            // Make sure there is room in the Cache
        while ( ( m_pListLast ) && ( m_aEntries.GetLength() >= m_nQueueMax ) )
        {
            CCacheItem *pOldest = m_pListLast;

            m_pListLast = pOldest->m_pPrevItem;
            m_pListLast->m_pNextItem = NULL;

            pOldest->m_pPrevItem = NULL;

            m_aEntries.Delete( m_aEntries.Find( pOldest ) );
        }

        m_aEntries.Push( pItem );
        pItem->m_pNextItem = m_pListFirst;
        m_pListFirst = pItem;

        return true;
    }

    bool CCache::RemoveItem( CCacheItem *pKey )
    {
        bool        bRetVal = false;
        size_t      nIndex = m_aEntries.Find( pKey );

        if ( nIndex != IASLib::NOT_FOUND )
        {
            CCacheItem *pRemove = (CCacheItem *)m_aEntries.Get( nIndex );

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
            }

            m_aEntries.Delete( nIndex );

            bRetVal = true;
        }

        return bRetVal;
    }

    CCacheItem *CCache::Get( CCacheItem *pKey )
    {
        CCacheItem *pRetVal = NULL;        
        size_t      nIndex = m_aEntries.Find( pKey );

        if ( nIndex != IASLib::NOT_FOUND )
        {
            pRetVal = (CCacheItem *)m_aEntries.Get( nIndex );

            if ( m_bUseExpiration )
            {
                if ( pRetVal->IsExpired() )
                {
                    if ( pRetVal->m_pNextItem )
                    {
                        pRetVal->m_pNextItem->m_pPrevItem = pRetVal->m_pPrevItem;
                    }
                    else
                    {
                        m_pListLast = pRetVal->m_pPrevItem;
                    }

                    if ( pRetVal->m_pPrevItem )
                    {
                        pRetVal->m_pPrevItem->m_pNextItem = pRetVal->m_pNextItem;
                    }
                    else
                    {
                        m_pListFirst = pRetVal->m_pNextItem;
                    }

                    pRetVal = NULL;
                    m_aEntries.Delete( nIndex );
                }
            }
            if ( pRetVal )
            {
                pRetVal->Hit( this );
            }
        }

        return pRetVal;
    }

    bool CCache::Touch( CCacheItem *pKey )
    {
        bool        bRetVal = false;
        CCacheItem *pItem = NULL;        
        size_t      nIndex = m_aEntries.Find( pKey );

        if ( nIndex != IASLib::NOT_FOUND )
        {
            pItem = (CCacheItem *)m_aEntries.Get( nIndex );

            pItem->Hit( this );

            bRetVal = true;
        }

        return bRetVal;
    }

    bool CCache::Discard( CCacheItem *pKey )
    {
        bool        bRetVal = false;
        CCacheItem *pItem = NULL;        
        size_t      nIndex = m_aEntries.Find( pKey );

        if ( nIndex != IASLib::NOT_FOUND )
        {
            pItem = (CCacheItem *)m_aEntries.Get( nIndex );

            MoveToTail( pItem );
            bRetVal = true;
        }
        return bRetVal;
    }

    void CCache::ClearExpired( void )
    {
        size_t nIndex = 0;

        while ( nIndex < m_aEntries.GetCount() )
        {
            CCacheItem *pItem = (CCacheItem *)m_aEntries.Get( nIndex );

            if ( pItem->IsExpired() )
            {
                m_aEntries.Delete( nIndex );
            }
            else
            {
                nIndex++;
            }
        }
    }

    void CCache::ClearExpired( CDate &dttCompare )
    {
        size_t nIndex = 0;

        while ( nIndex < m_aEntries.GetCount() )
        {
            CCacheItem *pItem = (CCacheItem *)m_aEntries.Get( nIndex );

            if ( pItem->IsExpired( dttCompare ) )
            {
                m_aEntries.Delete( nIndex );
            }
            else
            {
                nIndex++;
            }
        }
    }

    void CCache::DeleteAll( void )
    {
        DetatchList( );
		m_aEntries.DeleteAll();
    }

    void CCache::EmptyAll( void )
    {
        DetatchList( );
		m_aEntries.EmptyAll();
    }

    CIterator *CCache::Enumerate( void )
    {
        return m_aEntries.Enumerate();
    }

    void CCache::MoveToHead( CCacheItem *pItem )
    {
        if ( m_pListFirst != pItem )
        {
            if ( pItem->m_pNextItem )
            {
                pItem->m_pNextItem->m_pPrevItem = pItem->m_pPrevItem;
            }
            else
            {
                m_pListLast = pItem->m_pPrevItem;
            }

            pItem->m_pPrevItem->m_pNextItem = pItem->m_pNextItem;

            if ( m_pListFirst )
            {
                m_pListFirst->m_pPrevItem = pItem;
            }
            pItem->m_pNextItem = m_pListFirst;
            pItem->m_pPrevItem = NULL;
            m_pListFirst = pItem;
        }
    }

    void CCache::MoveToTail( CCacheItem *pItem )
    {
        if ( m_pListLast != pItem )
        {
            pItem->m_pNextItem->m_pPrevItem = pItem->m_pPrevItem;

            if ( pItem->m_pPrevItem )
            {
                pItem->m_pPrevItem->m_pNextItem = pItem->m_pNextItem;
            }
            else
            {
                m_pListFirst = pItem->m_pNextItem;
            }

            if ( m_pListLast )
            {
                m_pListLast->m_pNextItem = pItem;
            }
            pItem->m_pPrevItem = m_pListLast;
            pItem->m_pNextItem = NULL;
            m_pListLast = pItem;
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

