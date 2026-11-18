/**
 * @file
 * @brief TList Template.
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
    /// A doubly linked list.
    ///
    /// Adding or removing at either end, or at an iterator, takes constant
    /// time, and elements never move: pointers and iterators to an element
    /// stay valid until it is removed. Access by index or value walks the
    /// list. Each element is a separate heap node owned by the list. Not
    /// thread-safe.
    ///
    /// @tparam T The element type.
    ///
    /// @code
    /// TList<int> oList = { 1, 2, 3 };
    /// oList.AddFirst( 0 );
    /// for ( auto it = oList.begin(); it != oList.end(); )
    /// {
    ///     if ( *it % 2 )
    ///         it = oList.Remove( it );    // returns the next element
    ///     else
    ///         ++it;
    /// }
    /// @endcode
    template <class T>
    class TList : public CObject
    {
        protected:
            /// One element of the list.
            struct Node
            {
                T       Value;   ///< The element.
                Node   *pPrev;   ///< The node before, or NULL.
                Node   *pNext;   ///< The node after, or NULL.

                Node( const T &value ) : Value( value ), pPrev( NULL ), pNext( NULL ) {}   ///< An unlinked node with a copied value.
                Node( T &&value ) : Value( std::move( value ) ), pPrev( NULL ), pNext( NULL ) {}   ///< An unlinked node with a moved value.
            };

            Node       *m_pFirst;   ///< The first node, or NULL.
            Node       *m_pLast;   ///< The last node, or NULL.
            size_t      m_nLength;   ///< Number of elements.

        public:
            /// Bidirectional iterator over a list.
            /// @tparam V The element type (const for a const_iterator).
            /// @tparam N The node type (const for a const_iterator).
            template <class V, class N>
            class TListIterator
            {
                    friend class TList;
                    N          *m_pNode;   ///< The node, or NULL at end().
                    const TList *m_pList;   ///< The list (so -- from end() can find the last node).
                public:
                    typedef std::bidirectional_iterator_tag iterator_category;   ///< A bidirectional iterator.
                    typedef V                               value_type;   ///< The element type.
                    typedef ptrdiff_t                       difference_type;   ///< Distance type.
                    typedef V                              *pointer;   ///< Pointer to an element.
                    typedef V                              &reference;   ///< Reference to an element.

                    TListIterator( void ) : m_pNode( NULL ), m_pList( NULL ) {}   ///< An iterator into no list.
                    TListIterator( N *pNode, const TList *pList ) : m_pNode( pNode ), m_pList( pList ) {}   ///< Points at pNode (NULL for end()) of pList.
                        /// An iterator converts to a const_iterator.
                    operator TListIterator<const V, const N>( void ) const { return TListIterator<const V, const N>( m_pNode, m_pList ); }

                    V &operator *( void ) const { return m_pNode->Value; }   ///< The element.
                    V *operator ->( void ) const { return &m_pNode->Value; }   ///< The element.
                    TListIterator &operator ++( void ) { m_pNode = m_pNode->pNext; return *this; }   ///< Moves to the next element.
                    TListIterator operator ++( int ) { TListIterator old( *this ); ++*this; return old; }   ///< Moves to the next element, returning the old position.
                        /// Moves to the previous element. Back from end() is the last element.
                    TListIterator &operator --( void ) { m_pNode = ( m_pNode != NULL ) ? m_pNode->pPrev : m_pList->m_pLast; return *this; }
                    TListIterator operator --( int ) { TListIterator old( *this ); --*this; return old; }   ///< Moves back, returning the old position.
                    bool operator ==( const TListIterator &o ) const { return m_pNode == o.m_pNode; }   ///< Same element.
                    bool operator !=( const TListIterator &o ) const { return m_pNode != o.m_pNode; }   ///< Different elements.
                    N *GetNode( void ) const { return m_pNode; }   ///< The node (NULL at end()).
            };

            typedef T                                   value_type;   ///< The element type.
            typedef TListIterator<T, Node>              iterator;   ///< Iterator, first to last.
            typedef TListIterator<const T, const Node>  const_iterator;   ///< Const iterator, first to last.

            DECLARE_OBJECT( TList, CObject )

            TList( void ) : m_pFirst( NULL ), m_pLast( NULL ), m_nLength( 0 ) {}   ///< Makes an empty list.

            /// Makes a list holding copies of the listed values.
            TList( std::initializer_list<T> aValues ) : m_pFirst( NULL ), m_pLast( NULL ), m_nLength( 0 )
            {
                for ( const T &value : aValues )
                    AddLast( value );
            }

            /// Copies another list.
            TList( const TList &oSource ) : CObject( oSource ), m_pFirst( NULL ), m_pLast( NULL ), m_nLength( 0 )
            {
                for ( const Node *p = oSource.m_pFirst; p != NULL; p = p->pNext )
                    AddLast( p->Value );
            }

            /// Takes over another list's nodes, leaving it empty.
            TList( TList &&oSource ) : CObject( oSource ), m_pFirst( oSource.m_pFirst ), m_pLast( oSource.m_pLast ), m_nLength( oSource.m_nLength )
            {
                oSource.m_pFirst = oSource.m_pLast = NULL;
                oSource.m_nLength = 0;
            }

            virtual ~TList( void ) { Clear(); }   ///< Deletes every node. Pointed-to objects are not deleted.

            /// Replaces the contents with copies of another list's.
            TList &operator =( const TList &oSource )
            {
                if ( this != &oSource )
                {
                    TList oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            /// Replaces the contents by taking over another list's nodes, leaving it empty.
            TList &operator =( TList &&oSource )
            {
                if ( this != &oSource )
                {
                    TList oTaken( std::move( oSource ) );
                    Swap( oTaken );
                }
                return *this;
            }

            /// Exchanges the contents of two lists (no copying).
            void Swap( TList &oOther )
            {
                std::swap( m_pFirst, oOther.m_pFirst );
                std::swap( m_pLast, oOther.m_pLast );
                std::swap( m_nLength, oOther.m_nLength );
            }

            size_t      Length( void ) const { return m_nLength; }   ///< Number of elements.
            size_t      GetLength( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      Count( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_nLength; }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_nLength == 0; }   ///< True if the list is empty.

                /// @name Adding
            /// The index forms return where the value went.
            ///@{
            size_t AddFirst( const T &value ) { Link( new Node( value ), m_pFirst ); return 0; }   ///< Adds a copy at the front. @return 0.
            size_t AddFirst( T &&value ) { Link( new Node( std::move( value ) ), m_pFirst ); return 0; }   ///< Moves a value to the front. @return 0.
            size_t AddLast( const T &value ) { Link( new Node( value ), NULL ); return m_nLength - 1; }   ///< Adds a copy at the back. @return Its index.
            size_t AddLast( T &&value ) { Link( new Node( std::move( value ) ), NULL ); return m_nLength - 1; }   ///< Moves a value to the back. @return Its index.
            size_t Push( const T &value ) { return AddLast( value ); }   ///< Same as AddLast().

                /// Inserts before position nIndex (at the end, if past it).
            /// @return The index the value went to.
            size_t AddAt( size_t nIndex, const T &value )
            {
                if ( nIndex >= m_nLength )
                    return AddLast( value );
                Link( new Node( value ), NodeAt( nIndex ) );
                return nIndex;
            }

                /// Inserts before the first element equal to
                /// existing; false (and nothing added) if there is none.
            bool AddBefore( const T &existing, const T &value )
            {
                Node *pAt = FindNode( existing );
                if ( pAt == NULL )
                    return false;
                Link( new Node( value ), pAt );
                return true;
            }

            /// Inserts after the first element equal to existing; false (and
            /// nothing added) if there is none.
            bool AddAfter( const T &existing, const T &value )
            {
                Node *pAt = FindNode( existing );
                if ( pAt == NULL )
                    return false;
                Link( new Node( value ), pAt->pNext );
                return true;
            }

                /// Inserts before an iterator's element (at the end, for end()),
                // returning an iterator to the new element.
            iterator Insert( const_iterator itBefore, const T &value )
            {
                Node *pNew = new Node( value );
                Link( pNew, const_cast<Node *>( itBefore.GetNode() ) );
                return iterator( pNew, this );
            }

                ///@}
                /// @name Removing
                ///@{

            /// Removes and returns the first element.
            /// @throws CCollectionException if the list is empty.
            T RemoveFirst( void )
            {
                if ( m_pFirst == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return Take( m_pFirst );
            }

            /// Removes and returns the last element.
            /// @throws CCollectionException if the list is empty.
            T RemoveLast( void )
            {
                if ( m_pLast == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return Take( m_pLast );
            }

            /// Removes and returns the element at nIndex.
            /// @throws CCollectionException if nIndex is past the end.
            T RemoveAt( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TList index out of range" );
                return Take( NodeAt( nIndex ) );
            }

            /// Removes the first element equal to value. @return False if there is none.
            bool RemoveValue( const T &value )
            {
                Node *pNode = FindNode( value );
                if ( pNode == NULL )
                    return false;
                Take( pNode );
                return true;
            }

                /// Removes an iterator's element, returning an iterator to the
                /// one after it. Removing at end() does nothing and returns end().
            iterator Remove( const_iterator itAt )
            {
                Node *pNode = const_cast<Node *>( itAt.GetNode() );
                if ( pNode == NULL )
                    return end();
                Node *pNext = pNode->pNext;
                Take( pNode );
                return iterator( pNext, this );
            }

                ///@}
                /// @name Access
                ///@{

            /// The first element.
            /// @throws CCollectionException if the list is empty.
            T &GetFirst( void )
            {
                if ( m_pFirst == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return m_pFirst->Value;
            }

            const T &GetFirst( void ) const { return const_cast<TList *>( this )->GetFirst(); }   ///< The first element. @throws CCollectionException if the list is empty.

            /// The last element.
            /// @throws CCollectionException if the list is empty.
            T &GetLast( void )
            {
                if ( m_pLast == NULL )
                    TemplateDetail::ThrowOutOfRange( "TList is empty" );
                return m_pLast->Value;
            }

            const T &GetLast( void ) const { return const_cast<TList *>( this )->GetLast(); }   ///< The last element. @throws CCollectionException if the list is empty.

            /// The element at nIndex (walking from the nearer end).
            /// @throws CCollectionException if nIndex is past the end.
            T &GetAt( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TList index out of range" );
                return NodeAt( nIndex )->Value;
            }

            const T &GetAt( size_t nIndex ) const { return const_cast<TList *>( this )->GetAt( nIndex ); }   ///< The element at nIndex. @throws CCollectionException if nIndex is past the end.

                /// A copy of the value at nIndex, or oDefault.
            T Get( size_t nIndex, const T &oDefault = T() ) const
            {
                return ( nIndex < m_nLength ) ? const_cast<TList *>( this )->NodeAt( nIndex )->Value : oDefault;
            }

                /// The index of the first element equal to value, or NOT_FOUND.
            size_t Find( const T &value ) const
            {
                size_t nIndex = 0;
                for ( const Node *p = m_pFirst; p != NULL; p = p->pNext, nIndex++ )
                    if ( p->Value == value )
                        return nIndex;
                return NOT_FOUND;
            }

            bool Contains( const T &value ) const { return Find( value ) != NOT_FOUND; }   ///< True if some element equals value.

            ///@}
            /// Deletes every node. Pointed-to objects are not deleted.
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

            /// For a list of pointers: deletes what each points to, then empties the list.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for lists of pointers" );
                for ( Node *p = m_pFirst; p != NULL; p = p->pNext )
                    TemplateDetail::DeletePointer( p->Value );
                Clear();
            }

            /// Equal if the same length with equal elements in the same order.
            bool operator ==( const TList &oOther ) const
            {
                if ( m_nLength != oOther.m_nLength )
                    return false;
                for ( const Node *a = m_pFirst, *b = oOther.m_pFirst; a != NULL; a = a->pNext, b = b->pNext )
                    if ( ! ( a->Value == b->Value ) )
                        return false;
                return true;
            }
            bool operator !=( const TList &oOther ) const { return ! ( *this == oOther ); }   ///< Not equal.

            iterator        begin( void ) { return iterator( m_pFirst, this ); }   ///< The first element.
            iterator        end( void ) { return iterator( NULL, this ); }   ///< Past the last element.
            const_iterator  begin( void ) const { return const_iterator( m_pFirst, this ); }   ///< The first element.
            const_iterator  end( void ) const { return const_iterator( NULL, this ); }   ///< Past the last element.

        protected:
                /// Links pNew in before pBefore (at the end if NULL).
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

            /// Unlinks and deletes a node, returning its value.
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

            /// The node at nIndex (unchecked), walking from whichever end is nearer.
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

            /// The first node whose value equals value, or NULL.
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
