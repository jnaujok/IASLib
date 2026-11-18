/**
 * @file
 * @brief THash Template.
 *
 *      A hash table from keys to values. Lookups, additions and removals
 * take constant time on average; the table doubles its buckets when it is
 * three-quarters full (see Compact() and Reserve() for controlling the
 * room). Iterating gives the entries (Key and Value) in the order they
 * were added, which does not change when the table grows.
 *
 *      Hash and Equal default to THashOf (integers, pointers, CString,
 * std::string, or anything with hashCode()) and operator ==.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_THASH_H__
#define IASLIB_THASH_H__

#include "TArray.h"

#include <iterator>

namespace IASLib
{
    /// A hash table from keys to values that remembers the order entries were added.
    ///
    /// Lookups, additions and removals take constant time on average. Each
    /// entry is a separate heap node in a bucket chain and in a list kept in
    /// the order entries were added; iteration follows that list, and it does
    /// not change when the table grows. Changing an existing key's value does
    /// not move it. The bucket count is a power of two and doubles when the
    /// table is three-quarters full; removals never shrink it (see Compact()).
    ///
    /// Pointers from Find() and iterators stay valid until that entry is
    /// removed. Not thread-safe.
    ///
    /// @tparam K The key type.
    /// @tparam V The value type.
    /// @tparam Hash Hash function object; the default is THashOf.
    /// @tparam Equal Key equality function object; the default is operator ==.
    ///
    /// @code
    /// THash<CString, int> oAges;
    /// oAges.Put( "Ann", 31 );
    /// oAges[ "Bob" ] = 27;
    /// int nAge = oAges.Get( "Ann", -1 );          // 31
    /// for ( const auto &oEntry : oAges )          // Ann, then Bob
    ///     printf( "%s %d\n", (const char *)oEntry.Key, oEntry.Value );
    /// @endcode
    template <class K, class V, class Hash = THashOf<K>, class Equal = TEqualTo<K> >
    class THash : public CObject
    {
        public:
            /// A key and its value, as iteration gives them.
            struct Entry
            {
                const K     Key;   ///< The key (cannot be changed in place).
                V           Value;   ///< The value.

                Entry( const K &key, const V &value ) : Key( key ), Value( value ) {}   ///< A key with a copied value.
                Entry( const K &key, V &&value ) : Key( key ), Value( std::move( value ) ) {}   ///< A key with a moved value.
            };

        protected:
            /// One entry with its hash and links.
            struct Node
            {
                Entry       oEntry;   ///< The key and value.
                size_t      nHash;   ///< The key's hash.
                Node       *pBucketNext;    ///< the next node in this bucket
                Node       *pPrev;          ///< the order nodes were added in (the one before)
                Node       *pNext;   ///< The node added after this one.

                Node( const K &key, const V &value, size_t nHashValue ) : oEntry( key, value ), nHash( nHashValue ), pBucketNext( NULL ), pPrev( NULL ), pNext( NULL ) {}   ///< An unlinked node with a copied value.
                Node( const K &key, V &&value, size_t nHashValue ) : oEntry( key, std::move( value ) ), nHash( nHashValue ), pBucketNext( NULL ), pPrev( NULL ), pNext( NULL ) {}   ///< An unlinked node with a moved value.
            };

            Node      **m_apBuckets;   ///< The bucket chains (owned array of m_nBuckets heads).
            size_t      m_nBuckets;         ///< 0, or a power of two
            size_t      m_nLength;   ///< Number of entries.
            Node       *m_pFirst;   ///< The oldest node, or NULL.
            Node       *m_pLast;   ///< The newest node, or NULL.
            Hash        m_fnHash;   ///< The hash function.
            Equal       m_fnEqual;   ///< The key equality.

        public:
            /// Forward iterator over the entries, oldest first.
            /// @tparam E The entry type (const for a const_iterator).
            /// @tparam N The node type (const for a const_iterator).
            template <class E, class N>
            class THashIterator
            {
                    N          *m_pNode;   ///< The node, or NULL at end().
                public:
                    typedef std::forward_iterator_tag   iterator_category;   ///< A forward iterator.
                    typedef E                           value_type;   ///< The entry type.
                    typedef ptrdiff_t                   difference_type;   ///< Distance type.
                    typedef E                          *pointer;   ///< Pointer to an entry.
                    typedef E                          &reference;   ///< Reference to an entry.

                    explicit THashIterator( N *pNode = NULL ) : m_pNode( pNode ) {}   ///< Points at pNode (NULL for end()).
                    operator THashIterator<const E, const N>( void ) const { return THashIterator<const E, const N>( m_pNode ); }   ///< An iterator converts to a const_iterator.
                    E &operator *( void ) const { return m_pNode->oEntry; }   ///< The entry.
                    E *operator ->( void ) const { return &m_pNode->oEntry; }   ///< The entry.
                    THashIterator &operator ++( void ) { m_pNode = m_pNode->pNext; return *this; }   ///< Moves to the next entry.
                    THashIterator operator ++( int ) { THashIterator old( *this ); m_pNode = m_pNode->pNext; return old; }   ///< Moves to the next entry, returning the old position.
                    bool operator ==( const THashIterator &o ) const { return m_pNode == o.m_pNode; }   ///< Same entry.
                    bool operator !=( const THashIterator &o ) const { return m_pNode != o.m_pNode; }   ///< Different entries.
            };

            typedef THashIterator<Entry, Node>              iterator;   ///< Iterator, oldest entry first.
            typedef THashIterator<const Entry, const Node>  const_iterator;   ///< Const iterator, oldest entry first.

            DECLARE_OBJECT( THash, CObject )

                /// Makes an empty table.
            /// @param nExpected How many entries to make room for at first (0 allocates nothing).
            /// @param fnHash The hash function.
            /// @param fnEqual The key equality.
            explicit THash( size_t nExpected = 0, Hash fnHash = Hash(), Equal fnEqual = Equal() )
                : m_apBuckets( NULL ), m_nBuckets( 0 ), m_nLength( 0 ), m_pFirst( NULL ), m_pLast( NULL ), m_fnHash( fnHash ), m_fnEqual( fnEqual )
            {
                if ( nExpected > 0 )
                    Reserve( nExpected );
            }

            /// Copies another table, keeping its order.
            THash( const THash &oSource )
                : CObject( oSource ), m_apBuckets( NULL ), m_nBuckets( 0 ), m_nLength( 0 ), m_pFirst( NULL ), m_pLast( NULL ),
                  m_fnHash( oSource.m_fnHash ), m_fnEqual( oSource.m_fnEqual )
            {
                Reserve( oSource.m_nLength );
                for ( const Node *p = oSource.m_pFirst; p != NULL; p = p->pNext )
                    Put( p->oEntry.Key, p->oEntry.Value );
            }

            /// Takes over another table's entries, leaving it empty.
            THash( THash &&oSource )
                : CObject( oSource ), m_apBuckets( oSource.m_apBuckets ), m_nBuckets( oSource.m_nBuckets ), m_nLength( oSource.m_nLength ),
                  m_pFirst( oSource.m_pFirst ), m_pLast( oSource.m_pLast ), m_fnHash( oSource.m_fnHash ), m_fnEqual( oSource.m_fnEqual )
            {
                oSource.m_apBuckets = NULL;
                oSource.m_nBuckets = oSource.m_nLength = 0;
                oSource.m_pFirst = oSource.m_pLast = NULL;
            }

            /// Deletes every entry. Pointed-to values are not deleted.
            virtual ~THash( void )
            {
                Clear();
                delete [] m_apBuckets;
            }

            /// Replaces the contents with copies of another table's.
            THash &operator =( const THash &oSource )
            {
                if ( this != &oSource )
                {
                    THash oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            /// Replaces the contents by taking over another table's entries, leaving it empty.
            THash &operator =( THash &&oSource )
            {
                if ( this != &oSource )
                {
                    THash oTaken( std::move( oSource ) );
                    Swap( oTaken );
                }
                return *this;
            }

            /// Exchanges the contents of two tables (no copying).
            void Swap( THash &oOther )
            {
                std::swap( m_apBuckets, oOther.m_apBuckets );
                std::swap( m_nBuckets, oOther.m_nBuckets );
                std::swap( m_nLength, oOther.m_nLength );
                std::swap( m_pFirst, oOther.m_pFirst );
                std::swap( m_pLast, oOther.m_pLast );
                std::swap( m_fnHash, oOther.m_fnHash );
                std::swap( m_fnEqual, oOther.m_fnEqual );
            }

            size_t      Length( void ) const { return m_nLength; }   ///< Number of entries.
            size_t      GetLength( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      Count( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_nLength; }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_nLength == 0; }   ///< True if there are no entries.
            size_t      BucketCount( void ) const { return m_nBuckets; }   ///< Number of buckets (0 or a power of two).

                /// Room for nExpected entries without growing.
            void Reserve( size_t nExpected )
            {
                size_t nBuckets = BucketsFor( nExpected );
                if ( nBuckets > m_nBuckets )
                    Rehash( nBuckets );
            }

                /// Shrinks the buckets to what the entries need.
            void Compact( void )
            {
                size_t nBuckets = BucketsFor( m_nLength );
                if ( nBuckets < m_nBuckets )
                    Rehash( nBuckets );
            }

                /// Sets key's value, adding the key if it is new; returns true
                /// if it was new. An existing key keeps its place in the order.
            bool Put( const K &key, const V &value )
            {
                size_t nHash = m_fnHash( key );
                Node *pNode = FindNode( key, nHash );
                if ( pNode != NULL )
                {
                    pNode->oEntry.Value = value;
                    return false;
                }
                AddNode( new Node( key, value, nHash ) );
                return true;
            }

            /// Same, moving the value in.
            bool Put( const K &key, V &&value )
            {
                size_t nHash = m_fnHash( key );
                Node *pNode = FindNode( key, nHash );
                if ( pNode != NULL )
                {
                    pNode->oEntry.Value = std::move( value );
                    return false;
                }
                AddNode( new Node( key, std::move( value ), nHash ) );
                return true;
            }

                /// key's value, or NULL if it is not there. The pointer is valid until the key is removed.
            V *Find( const K &key )
            {
                Node *pNode = FindNode( key, m_fnHash( key ) );
                return ( pNode != NULL ) ? &pNode->oEntry.Value : NULL;
            }

            /// Same, for a const table.
            const V *Find( const K &key ) const
            {
                return const_cast<THash *>( this )->Find( key );
            }

                /// A copy of key's value, or oDefault.
            V Get( const K &key, const V &oDefault = V() ) const
            {
                const V *pValue = Find( key );
                return ( pValue != NULL ) ? *pValue : oDefault;
            }

                /// Copies key's value to oValue, if it is there.
            /// @return False (and oValue unchanged) if the key is not there.
            bool Get( const K &key, V &oValue ) const
            {
                const V *pValue = Find( key );
                if ( pValue == NULL )
                    return false;
                oValue = *pValue;
                return true;
            }

                /// key's value, added (as V()) if it is not there.
            V &operator []( const K &key )
            {
                size_t nHash = m_fnHash( key );
                Node *pNode = FindNode( key, nHash );
                if ( pNode == NULL )
                {
                    pNode = new Node( key, V(), nHash );
                    AddNode( pNode );
                }
                return pNode->oEntry.Value;
            }

            bool HasKey( const K &key ) const { return Find( key ) != NULL; }   ///< True if the key is there.
            bool Contains( const K &key ) const { return Find( key ) != NULL; }   ///< Same as HasKey().

                /// Removes key, returning whether it was there.
            bool Remove( const K &key )
            {
                Node *pNode = UnlinkNode( key );
                if ( pNode == NULL )
                    return false;
                delete pNode;
                return true;
            }

            /// Removes key, returning whether it was there (and its value, in oValue).
            bool Remove( const K &key, V &oValue )
            {
                Node *pNode = UnlinkNode( key );
                if ( pNode == NULL )
                    return false;
                oValue = std::move( pNode->oEntry.Value );
                delete pNode;
                return true;
            }

                /// The keys, in the order they were added.
            TArray<K> Keys( void ) const
            {
                TArray<K> aKeys;
                aKeys.Reserve( m_nLength );
                for ( const Node *p = m_pFirst; p != NULL; p = p->pNext )
                    aKeys.Push( p->oEntry.Key );
                return aKeys;
            }

            /// The values, in the order they were added.
            TArray<V> Values( void ) const
            {
                TArray<V> aValues;
                aValues.Reserve( m_nLength );
                for ( const Node *p = m_pFirst; p != NULL; p = p->pNext )
                    aValues.Push( p->oEntry.Value );
                return aValues;
            }

                /// The oldest entry (NULL if empty).
            Entry *FirstEntry( void ) { return ( m_pFirst != NULL ) ? &m_pFirst->oEntry : NULL; }
            /// The oldest entry (NULL if empty).
            const Entry *FirstEntry( void ) const { return ( m_pFirst != NULL ) ? &m_pFirst->oEntry : NULL; }
            /// Moves an entry to the newest place (as TCache does on each use).
            /// @return False if the key is not there.
            bool MoveToEnd( const K &key )
            {
                Node *pNode = FindNode( key, m_fnHash( key ) );
                if ( pNode == NULL )
                    return false;
                if ( pNode != m_pLast )
                {
                    UnlinkOrder( pNode );
                    LinkOrder( pNode );
                }
                return true;
            }

                /// Moving an entry to the oldest place.
            /// @return False if the key is not there.
            bool MoveToFront( const K &key )
            {
                Node *pNode = FindNode( key, m_fnHash( key ) );
                if ( pNode == NULL )
                    return false;
                if ( pNode != m_pFirst )
                {
                    UnlinkOrder( pNode );
                    pNode->pPrev = NULL;
                    pNode->pNext = m_pFirst;
                    m_pFirst->pPrev = pNode;
                    m_pFirst = pNode;
                }
                return true;
            }

            /// Deletes every entry. The buckets are kept.
            void Clear( void )
            {
                Node *p = m_pFirst;
                while ( p != NULL )
                {
                    Node *pNext = p->pNext;
                    delete p;
                    p = pNext;
                }
                for ( size_t n = 0; n < m_nBuckets; n++ )
                    m_apBuckets[n] = NULL;
                m_pFirst = m_pLast = NULL;
                m_nLength = 0;
            }

                /// For pointer values: deletes what each points to, then empties.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<V>::value, "DeleteAll() is for tables of pointers" );
                for ( Node *p = m_pFirst; p != NULL; p = p->pNext )
                    TemplateDetail::DeletePointer( p->oEntry.Value );
                Clear();
            }

            iterator        begin( void ) { return iterator( m_pFirst ); }   ///< The oldest entry.
            iterator        end( void ) { return iterator( NULL ); }   ///< Past the newest entry.
            const_iterator  begin( void ) const { return const_iterator( m_pFirst ); }   ///< The oldest entry.
            const_iterator  end( void ) const { return const_iterator( NULL ); }   ///< Past the newest entry.

        protected:
            /// The bucket count (0, or a power of two from 8) that holds nEntries at most three-quarters full.
            /// @throws std::bad_alloc if that would overflow.
            static size_t BucketsFor( size_t nEntries )
            {
                if ( nEntries == 0 )
                    return 0;
                size_t nBuckets = 8;
                while ( nBuckets - nBuckets / 4 < nEntries )
                {
                    if ( nBuckets > ( (size_t)-1 ) / 2 )
                        throw std::bad_alloc();
                    nBuckets *= 2;
                }
                return nBuckets;
            }

            /// The node for key (whose hash is nHash), or NULL.
            Node *FindNode( const K &key, size_t nHash ) const
            {
                if ( m_nBuckets == 0 )
                    return NULL;
                for ( Node *p = m_apBuckets[ nHash & ( m_nBuckets - 1 ) ]; p != NULL; p = p->pBucketNext )
                    if ( p->nHash == nHash && m_fnEqual( p->oEntry.Key, key ) )
                        return p;
                return NULL;
            }

            /// Links a new node in, growing the buckets if needed. If growing throws,
            /// the node is deleted and the exception passed on.
            void AddNode( Node *pNode )
            {
                if ( m_nLength + 1 > m_nBuckets - m_nBuckets / 4 )
                {
                    try
                    {
                        Rehash( BucketsFor( m_nLength + 1 ) );
                    }
                    catch ( ... )
                    {
                        delete pNode;
                        throw;
                    }
                }
                size_t nBucket = pNode->nHash & ( m_nBuckets - 1 );
                pNode->pBucketNext = m_apBuckets[ nBucket ];
                m_apBuckets[ nBucket ] = pNode;
                LinkOrder( pNode );
                m_nLength++;
            }

            /// Unlinks key's node without deleting it. @return The node, or NULL.
            Node *UnlinkNode( const K &key )
            {
                if ( m_nBuckets == 0 )
                    return NULL;
                size_t nHash = m_fnHash( key );
                Node **ppLink = &m_apBuckets[ nHash & ( m_nBuckets - 1 ) ];
                while ( *ppLink != NULL )
                {
                    Node *p = *ppLink;
                    if ( p->nHash == nHash && m_fnEqual( p->oEntry.Key, key ) )
                    {
                        *ppLink = p->pBucketNext;
                        UnlinkOrder( p );
                        m_nLength--;
                        return p;
                    }
                    ppLink = &p->pBucketNext;
                }
                return NULL;
            }

            /// Appends a node to the order list (as the newest).
            void LinkOrder( Node *pNode )
            {
                pNode->pNext = NULL;
                pNode->pPrev = m_pLast;
                if ( m_pLast != NULL )
                    m_pLast->pNext = pNode;
                else
                    m_pFirst = pNode;
                m_pLast = pNode;
            }

            /// Removes a node from the order list.
            void UnlinkOrder( Node *pNode )
            {
                if ( pNode->pPrev != NULL )
                    pNode->pPrev->pNext = pNode->pNext;
                else
                    m_pFirst = pNode->pNext;
                if ( pNode->pNext != NULL )
                    pNode->pNext->pPrev = pNode->pPrev;
                else
                    m_pLast = pNode->pPrev;
            }

            /// Rebuilds the buckets with nBuckets heads (0 frees them).
            void Rehash( size_t nBuckets )
            {
                Node **apNew = NULL;
                if ( nBuckets > 0 )
                {
                    apNew = new Node *[ nBuckets ];
                    for ( size_t n = 0; n < nBuckets; n++ )
                        apNew[n] = NULL;
                    for ( Node *p = m_pFirst; p != NULL; p = p->pNext )
                    {
                        size_t nBucket = p->nHash & ( nBuckets - 1 );
                        p->pBucketNext = apNew[ nBucket ];
                        apNew[ nBucket ] = p;
                    }
                }
                delete [] m_apBuckets;
                m_apBuckets = apNew;
                m_nBuckets = nBuckets;
            }
    };
} // namespace IASLib

#endif // IASLIB_THASH_H__
