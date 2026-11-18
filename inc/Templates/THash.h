/*
 * THash Template
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
    template <class K, class V, class Hash = THashOf<K>, class Equal = TEqualTo<K> >
    class THash : public CObject
    {
        public:
            struct Entry
            {
                const K     Key;
                V           Value;

                Entry( const K &key, const V &value ) : Key( key ), Value( value ) {}
                Entry( const K &key, V &&value ) : Key( key ), Value( std::move( value ) ) {}
            };

        protected:
            struct Node
            {
                Entry       oEntry;
                size_t      nHash;
                Node       *pBucketNext;    // the next node in this bucket
                Node       *pPrev;          // the order nodes were added in
                Node       *pNext;

                Node( const K &key, const V &value, size_t nHashValue ) : oEntry( key, value ), nHash( nHashValue ), pBucketNext( NULL ), pPrev( NULL ), pNext( NULL ) {}
                Node( const K &key, V &&value, size_t nHashValue ) : oEntry( key, std::move( value ) ), nHash( nHashValue ), pBucketNext( NULL ), pPrev( NULL ), pNext( NULL ) {}
            };

            Node      **m_apBuckets;
            size_t      m_nBuckets;         // 0, or a power of two
            size_t      m_nLength;
            Node       *m_pFirst;
            Node       *m_pLast;
            Hash        m_fnHash;
            Equal       m_fnEqual;

        public:
            template <class E, class N>
            class THashIterator
            {
                    N          *m_pNode;
                public:
                    typedef std::forward_iterator_tag   iterator_category;
                    typedef E                           value_type;
                    typedef ptrdiff_t                   difference_type;
                    typedef E                          *pointer;
                    typedef E                          &reference;

                    explicit THashIterator( N *pNode = NULL ) : m_pNode( pNode ) {}
                    operator THashIterator<const E, const N>( void ) const { return THashIterator<const E, const N>( m_pNode ); }
                    E &operator *( void ) const { return m_pNode->oEntry; }
                    E *operator ->( void ) const { return &m_pNode->oEntry; }
                    THashIterator &operator ++( void ) { m_pNode = m_pNode->pNext; return *this; }
                    THashIterator operator ++( int ) { THashIterator old( *this ); m_pNode = m_pNode->pNext; return old; }
                    bool operator ==( const THashIterator &o ) const { return m_pNode == o.m_pNode; }
                    bool operator !=( const THashIterator &o ) const { return m_pNode != o.m_pNode; }
            };

            typedef THashIterator<Entry, Node>              iterator;
            typedef THashIterator<const Entry, const Node>  const_iterator;

            DECLARE_OBJECT( THash, CObject )

                // nExpected: how many entries to make room for at first.
            explicit THash( size_t nExpected = 0, Hash fnHash = Hash(), Equal fnEqual = Equal() )
                : m_apBuckets( NULL ), m_nBuckets( 0 ), m_nLength( 0 ), m_pFirst( NULL ), m_pLast( NULL ), m_fnHash( fnHash ), m_fnEqual( fnEqual )
            {
                if ( nExpected > 0 )
                    Reserve( nExpected );
            }

            THash( const THash &oSource )
                : CObject( oSource ), m_apBuckets( NULL ), m_nBuckets( 0 ), m_nLength( 0 ), m_pFirst( NULL ), m_pLast( NULL ),
                  m_fnHash( oSource.m_fnHash ), m_fnEqual( oSource.m_fnEqual )
            {
                Reserve( oSource.m_nLength );
                for ( const Node *p = oSource.m_pFirst; p != NULL; p = p->pNext )
                    Put( p->oEntry.Key, p->oEntry.Value );
            }

            THash( THash &&oSource )
                : CObject( oSource ), m_apBuckets( oSource.m_apBuckets ), m_nBuckets( oSource.m_nBuckets ), m_nLength( oSource.m_nLength ),
                  m_pFirst( oSource.m_pFirst ), m_pLast( oSource.m_pLast ), m_fnHash( oSource.m_fnHash ), m_fnEqual( oSource.m_fnEqual )
            {
                oSource.m_apBuckets = NULL;
                oSource.m_nBuckets = oSource.m_nLength = 0;
                oSource.m_pFirst = oSource.m_pLast = NULL;
            }

            virtual ~THash( void )
            {
                Clear();
                delete [] m_apBuckets;
            }

            THash &operator =( const THash &oSource )
            {
                if ( this != &oSource )
                {
                    THash oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            THash &operator =( THash &&oSource )
            {
                if ( this != &oSource )
                {
                    THash oTaken( std::move( oSource ) );
                    Swap( oTaken );
                }
                return *this;
            }

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

            size_t      Length( void ) const { return m_nLength; }
            size_t      GetLength( void ) const { return m_nLength; }
            size_t      Count( void ) const { return m_nLength; }
            size_t      GetCount( void ) const { return m_nLength; }
            bool        IsEmpty( void ) const { return m_nLength == 0; }
            size_t      BucketCount( void ) const { return m_nBuckets; }

                // Room for nExpected entries without growing.
            void Reserve( size_t nExpected )
            {
                size_t nBuckets = BucketsFor( nExpected );
                if ( nBuckets > m_nBuckets )
                    Rehash( nBuckets );
            }

                // Shrinks the buckets to what the entries need.
            void Compact( void )
            {
                size_t nBuckets = BucketsFor( m_nLength );
                if ( nBuckets < m_nBuckets )
                    Rehash( nBuckets );
            }

                // Sets key's value, adding the key if it is new; returns true
                // if it was new.
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

                // key's value, or NULL if it is not there.
            V *Find( const K &key )
            {
                Node *pNode = FindNode( key, m_fnHash( key ) );
                return ( pNode != NULL ) ? &pNode->oEntry.Value : NULL;
            }

            const V *Find( const K &key ) const
            {
                return const_cast<THash *>( this )->Find( key );
            }

                // key's value, or oDefault.
            V Get( const K &key, const V &oDefault = V() ) const
            {
                const V *pValue = Find( key );
                return ( pValue != NULL ) ? *pValue : oDefault;
            }

                // Copies key's value to oValue, if it is there.
            bool Get( const K &key, V &oValue ) const
            {
                const V *pValue = Find( key );
                if ( pValue == NULL )
                    return false;
                oValue = *pValue;
                return true;
            }

                // key's value, added (as V()) if it is not there.
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

            bool HasKey( const K &key ) const { return Find( key ) != NULL; }
            bool Contains( const K &key ) const { return Find( key ) != NULL; }

                // Removes key, returning whether it was there (and its value,
                // in oValue).
            bool Remove( const K &key )
            {
                Node *pNode = UnlinkNode( key );
                if ( pNode == NULL )
                    return false;
                delete pNode;
                return true;
            }

            bool Remove( const K &key, V &oValue )
            {
                Node *pNode = UnlinkNode( key );
                if ( pNode == NULL )
                    return false;
                oValue = std::move( pNode->oEntry.Value );
                delete pNode;
                return true;
            }

                // The keys and the values, in the order they were added.
            TArray<K> Keys( void ) const
            {
                TArray<K> aKeys;
                aKeys.Reserve( m_nLength );
                for ( const Node *p = m_pFirst; p != NULL; p = p->pNext )
                    aKeys.Push( p->oEntry.Key );
                return aKeys;
            }

            TArray<V> Values( void ) const
            {
                TArray<V> aValues;
                aValues.Reserve( m_nLength );
                for ( const Node *p = m_pFirst; p != NULL; p = p->pNext )
                    aValues.Push( p->oEntry.Value );
                return aValues;
            }

                // The oldest entry (NULL if empty), and moving an entry to the
                // newest place (as TCache does on each use).
            Entry *FirstEntry( void ) { return ( m_pFirst != NULL ) ? &m_pFirst->oEntry : NULL; }
            const Entry *FirstEntry( void ) const { return ( m_pFirst != NULL ) ? &m_pFirst->oEntry : NULL; }
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

                // Moving an entry to the oldest place.
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

                // For pointer values: deletes what each points to, then empties.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<V>::value, "DeleteAll() is for tables of pointers" );
                for ( Node *p = m_pFirst; p != NULL; p = p->pNext )
                    TemplateDetail::DeletePointer( p->oEntry.Value );
                Clear();
            }

            iterator        begin( void ) { return iterator( m_pFirst ); }
            iterator        end( void ) { return iterator( NULL ); }
            const_iterator  begin( void ) const { return const_iterator( m_pFirst ); }
            const_iterator  end( void ) const { return const_iterator( NULL ); }

        protected:
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

            Node *FindNode( const K &key, size_t nHash ) const
            {
                if ( m_nBuckets == 0 )
                    return NULL;
                for ( Node *p = m_apBuckets[ nHash & ( m_nBuckets - 1 ) ]; p != NULL; p = p->pBucketNext )
                    if ( p->nHash == nHash && m_fnEqual( p->oEntry.Key, key ) )
                        return p;
                return NULL;
            }

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
