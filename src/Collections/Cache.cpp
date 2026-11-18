/**
 * Cache Class
 *
 *  An LRU cache of CCacheItems.
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Cache.h"

namespace IASLib
{
#ifdef IASLIB_MULTI_THREADED__
    #define IASLIB_CACHE_LOCK__     CMutexLock oLock__( cacheMutex )
#else
    #define IASLIB_CACHE_LOCK__     do { } while ( 0 )
#endif

    namespace
    {
        class CCacheIterator : public CIterator
        {
            protected:
                const CCache::Table        *m_pTable;
                CCache::Table::const_iterator m_itCurrent;
            public:
                                    CCacheIterator( const CCache::Table *pTable ) : m_pTable( pTable ), m_itCurrent( pTable->begin() ) {}
                                    DECLARE_OBJECT( CCacheIterator, CIterator )
                virtual            ~CCacheIterator( void ) {}
                virtual CObject    *Next( void )
                {
                    if ( m_itCurrent == m_pTable->end() )
                        return NULL;
                    CObject *pValue = m_itCurrent->Value;
                    ++m_itCurrent;
                    return pValue;
                }
                virtual CObject    *Prev( void ) { return NULL; }
                virtual void        Reset( void ) { m_itCurrent = m_pTable->begin(); }
                virtual bool        HasMore( void ) const { return m_itCurrent != m_pTable->end(); }
        };
    }

    IMPLEMENT_OBJECT( CCache, CCollection );

    CCache::CCache( size_t nMaxEntries, bool bUseExpiration )
    {
        m_nQueueMax = ( nMaxEntries > 0 ) ? nMaxEntries : 1;
        m_bUseExpiration = bUseExpiration;
    }

    CCache::~CCache( void )
    {
        DeleteAll();
    }

    size_t CCache::GetLength( void ) const
    {
        IASLIB_CACHE_LOCK__;
        return m_hashEntries.Length();
    }

    bool CCache::AddItem( CCacheItem *pItem )
    {
        if ( pItem == NULL )
            return false;

        CString strKey = pItem->GetKey();
        TArray<CCacheItem *> aDropped;
        {
            IASLIB_CACHE_LOCK__;
            CCacheItem *pOld = NULL;
            if ( m_hashEntries.Remove( strKey, pOld ) && pOld != pItem )
                aDropped.Push( pOld );
            while ( m_hashEntries.Length() >= m_nQueueMax )
            {
                Table::Entry *pOldest = m_hashEntries.FirstEntry();
                CString strOldest = pOldest->Key;
                CCacheItem *pEvicted = NULL;
                m_hashEntries.Remove( strOldest, pEvicted );
                aDropped.Push( pEvicted );
            }
            m_hashEntries.Put( strKey, pItem );
        }
        aDropped.DeleteAll();
        return true;
    }

    bool CCache::RemoveItem( const CCacheItem *pKey )
    {
        if ( pKey == NULL )
            return false;
        CCacheItem *pRemoved = NULL;
        {
            IASLIB_CACHE_LOCK__;
            if ( ! m_hashEntries.Remove( pKey->GetKey(), pRemoved ) )
                return false;
        }
        delete pRemoved;
        return true;
    }

        // Finds an item (locked by the caller), dropping it if it expired.
    CCacheItem *CCache::Lookup( const CString &strKey )
    {
        CCacheItem *pItem = m_hashEntries.Get( strKey, NULL );
        if ( pItem != NULL && m_bUseExpiration && pItem->IsExpired() )
        {
            m_hashEntries.Remove( strKey );
            delete pItem;
            pItem = NULL;
        }
        return pItem;
    }

    CCacheItem *CCache::Get( const CCacheItem *pKey )
    {
        return ( pKey != NULL ) ? Get( pKey->GetKey() ) : NULL;
    }

    CCacheItem *CCache::Get( const char *strKey )
    {
        IASLIB_CACHE_LOCK__;
        CCacheItem *pItem = Lookup( strKey );
        if ( pItem != NULL )
            pItem->Hit( this );
        return pItem;
    }

    bool CCache::Touch( const CCacheItem *pKey )
    {
        if ( pKey == NULL )
            return false;
        IASLIB_CACHE_LOCK__;
        CCacheItem *pItem = m_hashEntries.Get( pKey->GetKey(), NULL );
        if ( pItem == NULL )
            return false;
        pItem->Hit( this );
        return true;
    }

    bool CCache::Discard( const CCacheItem *pKey )
    {
        if ( pKey == NULL )
            return false;
        IASLIB_CACHE_LOCK__;
        return m_hashEntries.MoveToFront( pKey->GetKey() );
    }

        // Called by CCacheItem::Hit(), with the cache locked.
    void CCache::MoveToHead( CCacheItem *pItem )
    {
        m_hashEntries.MoveToEnd( pItem->GetKey() );
    }

    size_t CCache::ClearExpired( void )
    {
        CDate dttNow;
        dttNow.SetToCurrent();
        return ClearExpired( dttNow );
    }

    size_t CCache::ClearExpired( const CDate &dttCompare )
    {
        TArray<CCacheItem *> aExpired;
        {
            IASLIB_CACHE_LOCK__;
            TArray<CString> aKeys;
            for ( Table::const_iterator it = m_hashEntries.begin(); it != m_hashEntries.end(); ++it )
                if ( it->Value->IsExpired( dttCompare ) )
                    aKeys.Push( it->Key );
            for ( size_t n = 0; n < aKeys.Length(); n++ )
            {
                CCacheItem *pItem = NULL;
                if ( m_hashEntries.Remove( aKeys[n], pItem ) )
                    aExpired.Push( pItem );
            }
        }
        size_t nCount = aExpired.Length();
        aExpired.DeleteAll();
        return nCount;
    }

    void CCache::DeleteAll( void )
    {
        Table oTaken;
        {
            IASLIB_CACHE_LOCK__;
            oTaken.Swap( m_hashEntries );
        }
        oTaken.DeleteAll();
    }

    void CCache::EmptyAll( void )
    {
        IASLIB_CACHE_LOCK__;
        m_hashEntries.Clear();
    }

    CIterator *CCache::Enumerate( void )
    {
        return new CCacheIterator( &m_hashEntries );
    }
} // namespace IASLib
