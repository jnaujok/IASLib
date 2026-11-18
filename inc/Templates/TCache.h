/**
 * @file
 * @brief TCache Template.
 *
 *      A cache of at most MaxEntries values by key, which forgets the least
 * recently used entry to make room for a new one. Get() counts as a use;
 * Peek() does not. Entries may also expire: each can be given a time to
 * live (in milliseconds; 0 for never), after which Get() and Peek() no
 * longer find it, and ClearExpired() removes it. An eviction handler, if
 * set, is called for every entry the cache drops on its own (to make room,
 * or expired), so a cache of pointers can delete them; Remove() and
 * Clear() do not call it.
 *
 *      Iterating gives the entries from least to most recently used: each
 * has the Key and an Item, whose Value is the cached value (and nHits how
 * often Get() found it).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TCACHE_H__
#define IASLIB_TCACHE_H__

#include "THash.h"

#include <chrono>
#include <functional>

namespace IASLib
{
    /// A least-recently-used cache with optional expiry, built on a THash.
    ///
    /// The cache holds at most GetMaxEntries() values. Adding a new key to a
    /// full cache drops the least recently used entry. Get() counts as a use;
    /// Peek() does not. An entry given a time to live stops being found once
    /// it expires; looking it up (with Get(), Peek() or Contains()) or calling
    /// ClearExpired() drops it.
    ///
    /// The eviction handler, if set, is called for every entry the cache drops
    /// on its own (to make room, or expired), so a cache of pointers can delete
    /// them. Remove() and Clear() do not call it, and neither does Put() when
    /// it replaces an existing key's value. Not thread-safe.
    ///
    /// @tparam K The key type.
    /// @tparam V The value type.
    /// @tparam Hash Hash function object; the default is THashOf.
    /// @tparam Equal Key equality function object; the default is operator ==.
    ///
    /// @code
    /// TCache<CString, CObject *> oCache( 100 );
    /// oCache.SetEvictionHandler( []( const CString &, CObject *&pObject ) { delete pObject; } );
    /// oCache.Put( "logo", pLogo, 60000 );        // expires after a minute
    /// CObject **ppLogo = oCache.Get( "logo" );    // NULL if gone
    /// @endcode
    template <class K, class V, class Hash = THashOf<K>, class Equal = TEqualTo<K> >
    class TCache : public CObject
    {
        public:
            /// A cached value with its expiry time and hit count.
            struct Item
            {
                V           Value;   ///< The cached value.
                int64_t     llExpires;      ///< steady-clock milliseconds, 0 for never
                uint64_t    nHits;   ///< How many times Get() found this entry.

                Item( void ) : Value(), llExpires( 0 ), nHits( 0 ) {}   ///< A default value that never expires.
                Item( const V &value, int64_t llExpiresAt ) : Value( value ), llExpires( llExpiresAt ), nHits( 0 ) {}   ///< A value expiring at llExpiresAt (0 for never).
            };

            typedef THash<K, Item, Hash, Equal>             Table;   ///< The table that holds the entries.
            typedef typename Table::iterator                iterator;   ///< Iterator, least recently used first.
            typedef typename Table::const_iterator          const_iterator;   ///< Const iterator, least recently used first.
            typedef std::function<void ( const K &, V & )>  EvictionHandler;   ///< Called with the key and value of each entry the cache drops.
            typedef std::function<int64_t ( void )>         Clock;   ///< A clock giving milliseconds.

        protected:
            Table           m_hashItems;   ///< The entries, least recently used first.
            size_t          m_nMaxEntries;   ///< The most entries kept (at least 1).
            EvictionHandler m_fnEvicted;   ///< The eviction handler, or empty.
            Clock           m_fnClock;   ///< The clock set by SetClock(), or empty for the steady clock.
            uint64_t        m_nHits;   ///< Get() calls that found a value.
            uint64_t        m_nMisses;   ///< Get() calls that did not.

        public:
            DECLARE_OBJECT( TCache, CObject )

            /// Makes an empty cache.
            /// @param nMaxEntries The most entries to keep (0 is taken as 1).
            explicit TCache( size_t nMaxEntries = 1024 )
                : m_hashItems( 0 ), m_nMaxEntries( ( nMaxEntries > 0 ) ? nMaxEntries : 1 ), m_nHits( 0 ), m_nMisses( 0 )
            {
            }

            size_t      Length( void ) const { return m_hashItems.Length(); }   ///< Number of entries (expired ones not yet dropped included).
            size_t      GetLength( void ) const { return m_hashItems.Length(); }   ///< Same as Length().
            size_t      Count( void ) const { return m_hashItems.Length(); }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_hashItems.Length(); }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_hashItems.IsEmpty(); }   ///< True if there are no entries.
            size_t      GetMaxEntries( void ) const { return m_nMaxEntries; }   ///< The most entries kept.
            uint64_t    GetHits( void ) const { return m_nHits; }   ///< How many Get() calls found a value.
            uint64_t    GetMisses( void ) const { return m_nMisses; }   ///< How many Get() calls did not.

                /// Sets the most entries kept (0 is taken as 1).
            /// Lowering the limit drops the least recently used entries (through the eviction handler).
            void SetMaxEntries( size_t nMaxEntries )
            {
                m_nMaxEntries = ( nMaxEntries > 0 ) ? nMaxEntries : 1;
                while ( m_hashItems.Length() > m_nMaxEntries )
                    EvictOldest();
            }

            void SetEvictionHandler( EvictionHandler fnEvicted ) { m_fnEvicted = fnEvicted; }   ///< Sets the function called for each entry the cache drops on its own.
                /// For tests: a clock in milliseconds to use instead of the
                /// steady clock.
            void SetClock( Clock fnClock ) { m_fnClock = fnClock; }

                /// Adds or replaces key's value, as the most recently used
                /// entry, making room if the cache is full.
            /// @param key The key.
            /// @param value The value (copied).
            /// @param llTimeToLiveMs Milliseconds until it expires; 0 (or less) for never.
            void Put( const K &key, const V &value, int64_t llTimeToLiveMs = 0 )
            {
                int64_t llExpires = ( llTimeToLiveMs > 0 ) ? Now() + llTimeToLiveMs : 0;
                Item *pItem = m_hashItems.Find( key );
                if ( pItem != NULL )
                {
                    pItem->Value = value;
                    pItem->llExpires = llExpires;
                    m_hashItems.MoveToEnd( key );
                    return;
                }
                while ( m_hashItems.Length() >= m_nMaxEntries )
                    EvictOldest();
                m_hashItems.Put( key, Item( value, llExpires ) );
            }

                /// key's value (making it the most recently used), or NULL if
                /// it is not there or has expired. The pointer is valid until the entry is dropped.
            V *Get( const K &key )
            {
                Item *pItem = Live( key );
                if ( pItem == NULL )
                {
                    m_nMisses++;
                    return NULL;
                }
                m_nHits++;
                pItem->nHits++;
                m_hashItems.MoveToEnd( key );
                return &pItem->Value;
            }

            /// Copies key's value to oValue (making it the most recently used).
            /// @return False (and oValue unchanged) if it is not there or has expired.
            bool Get( const K &key, V &oValue )
            {
                V *pValue = Get( key );
                if ( pValue == NULL )
                    return false;
                oValue = *pValue;
                return true;
            }

                /// key's value without counting as a use, or NULL if it is not there or has expired.
            V *Peek( const K &key )
            {
                Item *pItem = Live( key );
                return ( pItem != NULL ) ? &pItem->Value : NULL;
            }

            bool Contains( const K &key ) { return Live( key ) != NULL; }   ///< True if key has a live entry (an expired one is dropped).

                /// How many times Get() has found key (0 if it is not there).
            uint64_t GetHitCount( const K &key ) const
            {
                const Item *pItem = m_hashItems.Find( key );
                return ( pItem != NULL ) ? pItem->nHits : 0;
            }

                /// Removes key without the eviction handler.
                /// @return False if the key was not there.
            bool Remove( const K &key )
            {
                return m_hashItems.Remove( key );
            }

            /// Removes key without the eviction handler (the value is in
            /// oValue, for the caller to dispose of).
            /// @return False if the key was not there.
            bool Remove( const K &key, V &oValue )
            {
                Item oItem;
                if ( ! m_hashItems.Remove( key, oItem ) )
                    return false;
                oValue = oItem.Value;
                return true;
            }

                /// Drops every expired entry (through the eviction handler),
                /// returning how many.
            size_t ClearExpired( void )
            {
                int64_t llNow = Now();
                TArray<K> aExpired;
                for ( const_iterator it = m_hashItems.begin(); it != m_hashItems.end(); ++it )
                    if ( Expired( it->Value, llNow ) )
                        aExpired.Push( it->Key );
                for ( size_t n = 0; n < aExpired.Length(); n++ )
                    Evict( aExpired[n] );
                return aExpired.Length();
            }

            void Clear( void ) { m_hashItems.Clear(); }   ///< Removes every entry without the eviction handler.

            iterator        begin( void ) { return m_hashItems.begin(); }   ///< The least recently used entry.
            iterator        end( void ) { return m_hashItems.end(); }   ///< Past the most recently used entry.
            const_iterator  begin( void ) const { return m_hashItems.begin(); }   ///< The least recently used entry.
            const_iterator  end( void ) const { return m_hashItems.end(); }   ///< Past the most recently used entry.

        protected:
            /// The current time in milliseconds (from SetClock()'s clock, or the steady clock).
            int64_t Now( void ) const
            {
                if ( m_fnClock )
                    return m_fnClock();
                return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
            }

            /// True if oItem has an expiry time and llNow has reached it.
            static bool Expired( const Item &oItem, int64_t llNow )
            {
                return oItem.llExpires != 0 && llNow >= oItem.llExpires;
            }

                /// The item for key, dropping it if it has expired.
            Item *Live( const K &key )
            {
                Item *pItem = m_hashItems.Find( key );
                if ( pItem != NULL && Expired( *pItem, Now() ) )
                {
                    Evict( key );
                    return NULL;
                }
                return pItem;
            }

            /// Drops the least recently used entry (through the eviction handler).
            void EvictOldest( void )
            {
                typename Table::Entry *pOldest = m_hashItems.FirstEntry();
                if ( pOldest != NULL )
                {
                    K key = pOldest->Key;
                    Evict( key );
                }
            }

            /// Removes key and passes its value to the eviction handler, if one is set.
            void Evict( const K &key )
            {
                Item oItem;
                if ( m_hashItems.Remove( key, oItem ) && m_fnEvicted )
                    m_fnEvicted( key, oItem.Value );
            }
    };
} // namespace IASLib

#endif // IASLIB_TCACHE_H__
