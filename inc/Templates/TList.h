/*
 * TList Template
 *
 *      A doubly linked list: adding or removing at either end, or next to
 * an element an iterator is at, takes constant time, and elements never
 * move (pointers and iterators to them stay valid until they are
 * removed). Finding by index or value walks the list. RemoveFirst(),
 * RemoveLast() and the GetFirst()/GetLast()/GetAt() references throw a
 * CCollectionException when there is no such element.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TLIST_H__
#define IASLIB_TLIST_H__

#include "TCommon.h"

#include <initializer_list>
#include <iterator>

namespace IASLib
{
    template <class T>
    class TList : public CObject
    {
        protected:
            struct Node
            {
                T       Value;
                Node   *pPrev;
                Node   *pNext;

                Node( const T &value ) : Value( value ), pPrev( NULL ), pNext( NULL ) {}
                Node( T &&value ) : Value( std::move( value ) ), pPrev( NULL ), pNext( NULL ) {}
            };

            Node       *m_pFirst;
            Node       *m_pLast;
            size_t      m_nLength;

        public:
            template <class V, class N>
            class TListIterator
            {
                    friend class TList;
                    N          *m_pNode;
                    const TList *m_pList;
                public:
                    typedef std::bidirectional_iterator_tag iterator_category;
                    typedef V                               value_type;
                    typedef ptrdiff_t                       difference_type;
                    typedef V                              *pointer;
                    typedef V                              &reference;

                    TListIterator( void ) : m_pNode( NULL ), m_pList( NULL ) {}
                    TListIterator( N *pNode, const TList *pList ) : m_pNode( pNode ), m_pList( pList ) {}
                        // An iterator converts to a const_iterator.
                    operator TListIterator<const V, const N>( void ) const { return TListIterator<const V, const N>( m_pNode, m_pList ); }

                    V &operator *( void ) const { return m_pNode->Value; }
                    V *operator ->( void ) const { return &m_pNode->Value; }
                    TListIterator &operator ++( void ) { m_pNode = m_pNode->pNext; return *this; }
                    TListIterator operator ++( int ) { TListIterator old( *this ); ++*this; return old; }
                        // Back from end() is the last element.
                    TListIterator &operator --( void ) { m_pNode = ( m_pNode != NULL ) ? m_pNode->pPrev : m_pList->m_pLast; return *this; }
                    TListIterator operator --( int ) { TListIterator old( *this ); --*this; return old; }
                    bool operator ==( const TListIterator &o ) const { return m_pNode == o.m_pNode; }
                    bool operator !=( const TListIterator &o ) const { return m_pNode != o.m_pNode; }
                    N *GetNode( void ) const { return m_pNode; }
            };

            typedef T                                   value_type;
            typedef TListIterator<T, Node>              iterator;
            typedef TListIterator<const T, const Node>  const_iterator;

            DECLARE_OBJECT( TList, CObject )

            TList( void ) : m_pFirst( NULL ), m_pLast( NULL ), m_nLength( 0 ) {}

            TList( std::initializer_list<T> aValues ) : m_pFirst( NULL ), m_pLast( NULL ), m_nLength( 0 )
            {
                for ( const T &value : aValues )
                    AddLast( value );
            }

            TList( const TList &oSource ) : CObject( oSource ), m_pFirst( NULL ), m_pLast( NULL ), m_nLength( 0 )
            {
                for ( const Node *p = oSource.m_pFirst; p != NULL; p = p->pNext )
                    AddLast( p->Value );
            }

            TList( TList &&oSource ) : CObject( oSource ), m_pFirst( oSource.m_pFirst ), m_pLast( oSource.m_pLast ), m_nLength( oSource.m_nLength )
            {
                oSource.m_pFirst = oSource.m_pLast = NULL;
                oSource.m_nLength = 0;
            }

            virtual ~TList( void ) { Clear(); }

            TList &operator =( const TList &oSource )
            {
                if ( this != &oSource )
                {
                    TList oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            TList &operator =( TList &&oSource )
            {
                if ( this != &oSource )
                {
                    TList oTaken( std::move( oSource ) );
                    Swap( oTaken );
                }
                return *this;
            }

            void Swap( TList &oOther )
            {
                std::swap( m_pFirst, oOther.m_pFirst );
                std::swap( m_pLast, oOther.m_pLast );
                std::swap( m_nLength, oOther.m_nLength );
            }

            size_t      Length( void ) const { return m_nLength; }
            size_t      GetLength( void ) const { return m_nLength; }
            size_t      Count( void ) const { return m_nLength; }
            size_t      GetCount( void ) const { return m_nLength; }
            bool        IsEmpty( void ) const { return m_nLength == 0; }

                // Adding. The index forms return where the value went.
            size_t AddFirst( const T &value ) { Link( new Node( value ), m_pFirst ); return 0; }
            size_t AddFirst( T &&value ) { Link( new Node( std::move( value ) ), m_pFirst ); return 0; }
            size_t AddLast( const T &value ) { Link( new Node( value ), NULL ); return m_nLength - 1; }
            size_t AddLast( T &&value ) { Link( new Node( std::move( value ) ), NULL ); return m_nLength - 1; }
            size_t Push( const T &value ) { return AddLast( value ); }

                // Inserts before position nIndex (at the end, if past it).
            size_t AddAt( size_t nIndex, const T &value )
            {
                if ( nIndex >= m_nLength )
                    return AddLast( value );
                Link( new Node( value ), NodeAt( nIndex ) );
                return nIndex;
            }

                // Inserts before or after the first element equal to
                // existing; false (and nothing added) if there is none.
            bool AddBefore( const T &existing, const T &value )
            {
                Node *pAt = FindNode( existing );
                if ( pAt == NULL )
                    return false;
                Link( new Node( value ), pAt );
                return true;
            }

            bool AddAfter( const T &existing, const T &value )
            {
                Node *pAt = FindNode( existing );
                if ( pAt == NULL )
                    return false;
                Link( new Node( value ), pAt->pNext );
                return true;
            }

                // Inserts before an iterator's element (at the end, for end()),
                // returning an iterator to the new element.
            iterator Insert( const_iterator itBefore, const T &value )
            {
                Node *pNew = new Node( value );
                Link( pNew, const_cast<Node *>( itBefore.GetNode() ) );
                return iterator( pNew, this );
            }

                // Removing.
            T RemoveFirst( void )
            {
                if ( m_pFirst == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return Take( m_pFirst );
            }

            T RemoveLast( void )
            {
                if ( m_pLast == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return Take( m_pLast );
            }

            T RemoveAt( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TList index out of range" );
                return Take( NodeAt( nIndex ) );
            }

            bool RemoveValue( const T &value )
            {
                Node *pNode = FindNode( value );
                if ( pNode == NULL )
                    return false;
                Take( pNode );
                return true;
            }

                // Removes an iterator's element, returning an iterator to the
                // one after it.
            iterator Remove( const_iterator itAt )
            {
                Node *pNode = const_cast<Node *>( itAt.GetNode() );
                if ( pNode == NULL )
                    return end();
                Node *pNext = pNode->pNext;
                Take( pNode );
                return iterator( pNext, this );
            }

                // Access.
            T &GetFirst( void )
            {
                if ( m_pFirst == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return m_pFirst->Value;
            }

            const T &GetFirst( void ) const { return const_cast<TList *>( this )->GetFirst(); }

            T &GetLast( void )
            {
                if ( m_pLast == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return m_pLast->Value;
            }

            const T &GetLast( void ) const { return const_cast<TList *>( this )->GetLast(); }

            T &GetAt( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TList index out of range" );
                return NodeAt( nIndex )->Value;
            }

            const T &GetAt( size_t nIndex ) const { return const_cast<TList *>( this )->GetAt( nIndex ); }

                // The value at nIndex, or oDefault.
            T Get( size_t nIndex, const T &oDefault = T() ) const
            {
                return ( nIndex < m_nLength ) ? const_cast<TList *>( this )->NodeAt( nIndex )->Value : oDefault;
            }

                // The index of the first element equal to value, or NOT_FOUND.
            size_t Find( const T &value ) const
            {
                size_t nIndex = 0;
                for ( const Node *p = m_pFirst; p != NULL; p = p->pNext, nIndex++ )
                    if ( p->Value == value )
                        return nIndex;
                return NOT_FOUND;
            }

            bool Contains( const T &value ) const { return Find( value ) != NOT_FOUND; }

            void Clear( void )
            {
                Node *p = m_pFirst;
                while ( p != NULL )
                {
                    Node *pNext = p->pNext;
                    delete p;
                    p = pNext;
                }
                m_pFirst = m_pLast = NULL;
                m_nLength = 0;
            }

            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for lists of pointers" );
                for ( Node *p = m_pFirst; p != NULL; p = p->pNext )
                    TemplateDetail::DeletePointer( p->Value );
                Clear();
            }

            bool operator ==( const TList &oOther ) const
            {
                if ( m_nLength != oOther.m_nLength )
                    return false;
                for ( const Node *a = m_pFirst, *b = oOther.m_pFirst; a != NULL; a = a->pNext, b = b->pNext )
                    if ( ! ( a->Value == b->Value ) )
                        return false;
                return true;
            }
            bool operator !=( const TList &oOther ) const { return ! ( *this == oOther ); }

            iterator        begin( void ) { return iterator( m_pFirst, this ); }
            iterator        end( void ) { return iterator( NULL, this ); }
            const_iterator  begin( void ) const { return const_iterator( m_pFirst, this ); }
            const_iterator  end( void ) const { return const_iterator( NULL, this ); }

        protected:
                // Links pNew in before pBefore (at the end if NULL).
            void Link( Node *pNew, Node *pBefore )
            {
                pNew->pNext = pBefore;
                pNew->pPrev = ( pBefore != NULL ) ? pBefore->pPrev : m_pLast;
                if ( pNew->pPrev != NULL )
                    pNew->pPrev->pNext = pNew;
                else
                    m_pFirst = pNew;
                if ( pBefore != NULL )
                    pBefore->pPrev = pNew;
                else
                    m_pLast = pNew;
                m_nLength++;
            }

            T Take( Node *pNode )
            {
                if ( pNode->pPrev != NULL )
                    pNode->pPrev->pNext = pNode->pNext;
                else
                    m_pFirst = pNode->pNext;
                if ( pNode->pNext != NULL )
                    pNode->pNext->pPrev = pNode->pPrev;
                else
                    m_pLast = pNode->pPrev;
                m_nLength--;
                T value( std::move( pNode->Value ) );
                delete pNode;
                return value;
            }

            Node *NodeAt( size_t nIndex )
            {
                    // Walk from whichever end is nearer.
                if ( nIndex < m_nLength / 2 )
                {
                    Node *p = m_pFirst;
                    while ( nIndex-- > 0 )
                        p = p->pNext;
                    return p;
                }
                Node *p = m_pLast;
                for ( size_t n = m_nLength - 1; n > nIndex; n-- )
                    p = p->pPrev;
                return p;
            }

            Node *FindNode( const T &value )
            {
                for ( Node *p = m_pFirst; p != NULL; p = p->pNext )
                    if ( p->Value == value )
                        return p;
                return NULL;
            }
    };
} // namespace IASLib

#endif // IASLIB_TLIST_H__
