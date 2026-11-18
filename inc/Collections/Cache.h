/**
 * @file Cache.h
 * @brief CCache, a least-recently-used cache of CCacheItem objects.
 *
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
    /**
     * An LRU cache of objects derived from CCacheItem, by their keys
     * (CCacheItem::GetKey()), holding at most a fixed number.
     *
     * The cache owns its items: adding a new one when it is full deletes the
     * least recently used, and adding one whose key is already there replaces
     * (and deletes) the old one. Get() and Touch() count as uses; Discard()
     * makes an item the next to go. With bUseExpiration, Get() drops (and
     * deletes) expired items; ClearExpired() does so for all of them. It is
     * thread-safe: each call locks the cache (but a pointer Get() returns can
     * be deleted by a later call on another thread). It cannot be copied.
     * For a cache of any type, with expiry in milliseconds, use TCache.
     *
     * @code
     * CCache cache( 100, true );
     * cache.AddItem( new CUserEntry( "alice" ) );     // the cache owns it
     * CCacheItem *pItem = cache.Get( "alice" );       // a use; NULL if gone
     * @endcode
     */
    class CCache : public CCollection
    {
        friend class CCacheItem;

        public:
                /// The items by key, oldest (least recently used) first.
            typedef THash<CString, CCacheItem *> Table;

        protected:
            Table                   m_hashEntries;      ///< The items, least recently used first.
            size_t                  m_nQueueMax;        ///< The most items held.
            bool                    m_bUseExpiration;   ///< Whether Get() drops expired items.
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex          cacheMutex;         ///< Locked by every call.
#endif

        public:
                                    /// An empty cache.
                                    /// @param nMaxEntries The most items held (0 means 1).
                                    /// @param bUseExpiration Whether Get() drops expired items.
                                    CCache( size_t nMaxEntries, bool bUseExpiration = false );
                                    /// Deletes every item.
            virtual                ~CCache( void );

                                    DEFINE_OBJECT(CCache);

                /// The number of items.
            virtual size_t          GetLength( void ) const;
                /// The most items held.
            size_t                  GetMaxEntries( void ) const { return m_nQueueMax; }

                /// Adds an item as the most recently used (the cache owns it).
                /// An item with the same key is replaced and deleted; when the
                /// cache is full, the least recently used is deleted.
                /// @return false for NULL.
            virtual bool            AddItem( CCacheItem *pItem );
                /// Removes and deletes the item with pKey's key.
                /// @return false if there was none (or pKey is NULL).
            virtual bool            RemoveItem( const CCacheItem *pKey );
                /// The item with pKey's key (a use), or NULL. The cache still
                /// owns it.
            virtual CCacheItem     *Get( const CCacheItem *pKey );
                /// The item with this key (a use), or NULL.
            virtual CCacheItem     *Get( const char *strKey );
                /// Counts a use of the item with pKey's key (making it the most
                /// recently used), without the expiry check Get() makes.
                /// @return false if there is no such item.
            virtual bool            Touch( const CCacheItem *pKey );
                /// Makes the item the least recently used.
                /// @return false if there is no such item.
            virtual bool            Discard( const CCacheItem *pKey );

                /// Removes and deletes every item.
            virtual void            DeleteAll( void );
                /// Removes every item without deleting it.
            virtual void            EmptyAll( void );

                /// A new iterator (the caller deletes it), from least to most
                /// recently used. It cannot go back: Prev() returns NULL.
            virtual CIterator      *Enumerate( void );

                /// Removes and deletes items expired as of now, returning how many.
            virtual size_t          ClearExpired( void );
                /// Removes and deletes items expired as of dttCompare,
                /// returning how many.
            virtual size_t          ClearExpired( const CDate &dttCompare );

        private:
                /// Makes the item the most recently used (CCacheItem::Hit()
                /// calls it, with the cache locked).
            void                    MoveToHead( CCacheItem *pItem );
                /// The item with this key, dropping it if it has expired (with
                /// bUseExpiration); called with the cache locked.
            CCacheItem             *Lookup( const CString &strKey );

                                    /// Not copyable.
                                    CCache( const CCache & );
                                    /// Not assignable.
            CCache                 &operator =( const CCache & );
    };
} // namespace IASLib

#endif // IASLIB_CACHE_H__
