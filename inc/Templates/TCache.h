/*
 * TCache Template
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
    template <class K, class V, class Hash = THashOf<K>, class Equal = TEqualTo<K> >
    class TCache : public CObject
    {
        public:
            struct Item
            {
                V           Value;
                int64_t     llExpires;      // steady-clock milliseconds, 0 for never
                uint64_t    nHits;

                Item( void ) : Value(), llExpires( 0 ), nHits( 0 ) {}
                Item( const V &value, int64_t llExpiresAt ) : Value( value ), llExpires( llExpiresAt ), nHits( 0 ) {}
            };

            typedef THash<K, Item, Hash, Equal>             Table;
            typedef typename Table::iterator                iterator;
            typedef typename Table::const_iterator          const_iterator;
            typedef std::function<void ( const K &, V & )>  EvictionHandler;
            typedef std::function<int64_t ( void )>         Clock;

        protected:
            Table           m_hashItems;
            size_t          m_nMaxEntries;
            EvictionHandler m_fnEvicted;
            Clock           m_fnClock;
            uint64_t        m_nHits;
            uint64_t        m_nMisses;

        public:
            DECLARE_OBJECT( TCache, CObject )

            explicit TCache( size_t nMaxEntries = 1024 )
                : m_hashItems( 0 ), m_nMaxEntries( ( nMaxEntries > 0 ) ? nMaxEntries : 1 ), m_nHits( 0 ), m_nMisses( 0 )
            {
            }

            size_t      Length( void ) const { return m_hashItems.Length(); }
            size_t      GetLength( void ) const { return m_hashItems.Length(); }
            size_t      Count( void ) const { return m_hashItems.Length(); }
            size_t      GetCount( void ) const { return m_hashItems.Length(); }
            bool        IsEmpty( void ) const { return m_hashItems.IsEmpty(); }
            size_t      GetMaxEntries( void ) const { return m_nMaxEntries; }
            uint64_t    GetHits( void ) const { return m_nHits; }
            uint64_t    GetMisses( void ) const { return m_nMisses; }

                // Lowering the limit drops the least recently used entries.
            void SetMaxEntries( size_t nMaxEntries )
            {
                m_nMaxEntries = ( nMaxEntries > 0 ) ? nMaxEntries : 1;
                while ( m_hashItems.Length() > m_nMaxEntries )
                    EvictOldest();
            }

            void SetEvictionHandler( EvictionHandler fnEvicted ) { m_fnEvicted = fnEvicted; }
                // For tests: a clock in milliseconds to use instead of the
                // steady clock.
            void SetClock( Clock fnClock ) { m_fnClock = fnClock; }

                // Adds or replaces key's value, as the most recently used
                // entry, making room if the cache is full.
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

                // key's value (making it the most recently used), or NULL if
                // it is not there or has expired.
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

            bool Get( const K &key, V &oValue )
            {
                V *pValue = Get( key );
                if ( pValue == NULL )
                    return false;
                oValue = *pValue;
                return true;
            }

                // key's value without counting as a use.
            V *Peek( const K &key )
            {
                Item *pItem = Live( key );
                return ( pItem != NULL ) ? &pItem->Value : NULL;
            }

            bool Contains( const K &key ) { return Live( key ) != NULL; }

                // How many times Get() has found key.
            uint64_t GetHitCount( const K &key ) const
            {
                const Item *pItem = m_hashItems.Find( key );
                return ( pItem != NULL ) ? pItem->nHits : 0;
            }

                // Removes key without the eviction handler (the value is in
                // oValue, for the caller to dispose of).
            bool Remove( const K &key )
            {
                return m_hashItems.Remove( key );
            }

            bool Remove( const K &key, V &oValue )
            {
                Item oItem;
                if ( ! m_hashItems.Remove( key, oItem ) )
                    return false;
                oValue = oItem.Value;
                return true;
            }

                // Drops every expired entry (through the eviction handler),
                // returning how many.
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

            void Clear( void ) { m_hashItems.Clear(); }

            iterator        begin( void ) { return m_hashItems.begin(); }
            iterator        end( void ) { return m_hashItems.end(); }
            const_iterator  begin( void ) const { return m_hashItems.begin(); }
            const_iterator  end( void ) const { return m_hashItems.end(); }

        protected:
            int64_t Now( void ) const
            {
                if ( m_fnClock )
                    return m_fnClock();
                return (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
            }

            static bool Expired( const Item &oItem, int64_t llNow )
            {
                return oItem.llExpires != 0 && llNow >= oItem.llExpires;
            }

                // The item for key, dropping it if it has expired.
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

            void EvictOldest( void )
            {
                typename Table::Entry *pOldest = m_hashItems.FirstEntry();
                if ( pOldest != NULL )
                {
                    K key = pOldest->Key;
                    Evict( key );
                }
            }

            void Evict( const K &key )
            {
                Item oItem;
                if ( m_hashItems.Remove( key, oItem ) && m_fnEvicted )
                    m_fnEvicted( key, oItem.Value );
            }
    };
} // namespace IASLib

#endif // IASLIB_TCACHE_H__
