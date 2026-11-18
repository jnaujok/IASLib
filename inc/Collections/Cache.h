/**
 * Cache Class
 *
 *  An LRU cache of objects derived from CCacheItem, by their keys
 * (GetKey()), holding at most nMaxEntries. The cache owns its items:
 * adding a new one when it is full deletes the least recently used, and
 * adding one whose key is already there replaces (and deletes) the old
 * one. Get() and Touch() count as uses; Discard() makes an item the next
 * to go. With bUseExpiration, Get() drops (and deletes) expired items;
 * ClearExpired() does so for all of them. It is thread-safe: each call
 * locks the cache.
 *
 *  For a cache of any type, with expiry in milliseconds, use TCache.
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 1/15/2004 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CACHE_H__
#define IASLIB_CACHE_H__

#include "../BaseTypes/String_.h"
#include "Collection.h"
#include "CacheItem.h"
#include "../Templates/THash.h"
#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif

namespace IASLib
{
    class CCache : public CCollection
    {
        friend class CCacheItem;

        public:
                // Oldest (least recently used) first.
            typedef THash<CString, CCacheItem *> Table;

        protected:
            Table                   m_hashEntries;
            size_t                  m_nQueueMax;
            bool                    m_bUseExpiration;
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex          cacheMutex;
#endif

        public:
                                    CCache( size_t nMaxEntries, bool bUseExpiration = false );
            virtual                ~CCache( void );

                                    DEFINE_OBJECT(CCache);

            virtual size_t          GetLength( void ) const;
            size_t                  GetMaxEntries( void ) const { return m_nQueueMax; }

                // Adds an item as the most recently used (the cache owns it).
            virtual bool            AddItem( CCacheItem *pItem );
                // Removes and deletes the item with pKey's key.
            virtual bool            RemoveItem( const CCacheItem *pKey );
                // The item with pKey's key (a use), or NULL.
            virtual CCacheItem     *Get( const CCacheItem *pKey );
            virtual CCacheItem     *Get( const char *strKey );
            virtual bool            Touch( const CCacheItem *pKey );
                // Makes the item the least recently used.
            virtual bool            Discard( const CCacheItem *pKey );

            virtual void            DeleteAll( void );
            virtual void            EmptyAll( void );

                // From least to most recently used.
            virtual CIterator      *Enumerate( void );

                // Removes and deletes expired items (as of now, or of
                // dttCompare), returning how many.
            virtual size_t          ClearExpired( void );
            virtual size_t          ClearExpired( const CDate &dttCompare );

        private:
            void                    MoveToHead( CCacheItem *pItem );
            CCacheItem             *Lookup( const CString &strKey );

                                    CCache( const CCache & );
            CCache                 &operator =( const CCache & );
    };
} // namespace IASLib

#endif // IASLIB_CACHE_H__
