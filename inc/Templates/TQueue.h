/**
 * @file
 * @brief TQueue Template.
 *
 *      First in, first out, in a ring buffer: Push() and Pop() take
 * constant time (a queue built on an array has to move every element on
 * each Pop()). Pop() and Peek() on an empty queue throw a
 * CCollectionException. Get( 0 ) and iterating start at the front (the
 * next to be popped).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TQUEUE_H__
#define IASLIB_TQUEUE_H__

#include "TCommon.h"

#include <iterator>

namespace IASLib
{
    /// A first in, first out queue, stored as a ring buffer.
    ///
    /// Push() adds at the back and Pop() takes from the front, both in
    /// constant time (apart from growing). The buffer grows and shrinks as
    /// described in TCommon.h. Pop() and Peek() on an empty queue throw a
    /// CCollectionException. Iteration runs front to back. Not thread-safe.
    ///
    /// @tparam T The element type.
    ///
    /// @code
    /// TQueue<CString> oJobs;
    /// oJobs.Push( "first" );
    /// oJobs.Push( "second" );
    /// while ( ! oJobs.IsEmpty() )
    ///     Run( oJobs.Pop() );         // "first", then "second"
    /// @endcode
    template <class T>
    class TQueue : public CObject
    {
        protected:
            T              *m_pData;   ///< The ring buffer (raw storage of m_nCapacity slots).
            size_t          m_nCapacity;   ///< Number of slots allocated.
            size_t          m_nFront;       ///< where the front element is
            size_t          m_nLength;   ///< Number of elements.
            size_t          m_nScale;   ///< The least the buffer grows by.

        public:
            /// Forward iterator over a queue, front to back.
            /// @tparam Q The queue type (const for a const_iterator).
            /// @tparam V The element type (const for a const_iterator).
            template <class Q, class V>
            class TQueueIterator
            {
                    Q          *m_pQueue;   ///< The queue.
                    size_t      m_nPosition;   ///< Position from the front.
                public:
                    typedef std::forward_iterator_tag   iterator_category;   ///< A forward iterator.
                    typedef V                           value_type;   ///< The element type.
                    typedef ptrdiff_t                   difference_type;   ///< Distance type.
                    typedef V                          *pointer;   ///< Pointer to an element.
                    typedef V                          &reference;   ///< Reference to an element.

                    TQueueIterator( Q *pQueue, size_t nPosition ) : m_pQueue( pQueue ), m_nPosition( nPosition ) {}   ///< Points at position nPosition of pQueue.
                    V &operator *( void ) const { return m_pQueue->At( m_nPosition ); }   ///< The element.
                    V *operator ->( void ) const { return &m_pQueue->At( m_nPosition ); }   ///< The element.
                    TQueueIterator &operator ++( void ) { m_nPosition++; return *this; }   ///< Moves to the next element.
                    TQueueIterator operator ++( int ) { TQueueIterator old( *this ); m_nPosition++; return old; }   ///< Moves to the next element, returning the old position.
                    bool operator ==( const TQueueIterator &o ) const { return m_pQueue == o.m_pQueue && m_nPosition == o.m_nPosition; }   ///< Same queue and position.
                    bool operator !=( const TQueueIterator &o ) const { return ! ( *this == o ); }   ///< Not equal.
            };

            typedef TQueueIterator<TQueue, T>               iterator;   ///< Iterator, front to back.
            typedef TQueueIterator<const TQueue, const T>   const_iterator;   ///< Const iterator, front to back.

            DECLARE_OBJECT( TQueue, CObject )

            /// Makes an empty queue (no memory is allocated until needed).
            /// @param nScale The least the buffer grows by (0 is taken as 1).
            explicit TQueue( size_t nScale = 16 )
                : m_pData( NULL ), m_nCapacity( 0 ), m_nFront( 0 ), m_nLength( 0 ), m_nScale( ( nScale > 0 ) ? nScale : 1 )
            {
            }

            /// Copies another queue.
            TQueue( const TQueue &oSource )
                : CObject( oSource ), m_pData( NULL ), m_nCapacity( 0 ), m_nFront( 0 ), m_nLength( 0 ), m_nScale( oSource.m_nScale )
            {
                Reallocate( oSource.m_nLength );
                for ( size_t n = 0; n < oSource.m_nLength; n++ )
                    Push( oSource.At( n ) );
            }

            /// Takes over another queue's buffer, leaving it empty.
            TQueue( TQueue &&oSource )
                : CObject( oSource ), m_pData( oSource.m_pData ), m_nCapacity( oSource.m_nCapacity ), m_nFront( oSource.m_nFront ),
                  m_nLength( oSource.m_nLength ), m_nScale( oSource.m_nScale )
            {
                oSource.m_pData = NULL;
                oSource.m_nCapacity = oSource.m_nFront = oSource.m_nLength = 0;
            }

            /// Destroys the elements and frees the buffer. Pointed-to objects are not deleted.
            virtual ~TQueue( void )
            {
                Clear();
                ::operator delete( m_pData );
            }

            /// Replaces the contents with copies of another queue's.
            TQueue &operator =( const TQueue &oSource )
            {
                if ( this != &oSource )
                {
                    TQueue oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            /// Replaces the contents by taking over another queue's buffer, leaving it empty.
            TQueue &operator =( TQueue &&oSource )
            {
                if ( this != &oSource )
                {
                    TQueue oTaken( std::move( oSource ) );
                    Swap( oTaken );
                }
                return *this;
            }

            /// Exchanges the contents of two queues (no copying).
            void Swap( TQueue &oOther )
            {
                std::swap( m_pData, oOther.m_pData );
                std::swap( m_nCapacity, oOther.m_nCapacity );
                std::swap( m_nFront, oOther.m_nFront );
                std::swap( m_nLength, oOther.m_nLength );
                std::swap( m_nScale, oOther.m_nScale );
            }

            size_t      Length( void ) const { return m_nLength; }   ///< Number of elements.
            size_t      GetLength( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      Count( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_nLength; }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_nLength == 0; }   ///< True if the queue is empty.
            size_t      Capacity( void ) const { return m_nCapacity; }   ///< Number of elements that fit without growing.
            void        SetScale( size_t nScale ) { if ( nScale > 0 ) m_nScale = nScale; }   ///< Sets the least the buffer grows by (0 is ignored).
            void        Reserve( size_t nCapacity ) { if ( nCapacity > m_nCapacity ) Reallocate( nCapacity ); }   ///< Room for nCapacity elements without growing.
            void        Compact( void ) { if ( m_nCapacity > m_nLength ) Reallocate( m_nLength ); }   ///< Gives back unused room.

                /// Adds a copy at the back, returning the new length.
            size_t Push( const T &value )
            {
                T copy( value );
                return Push( std::move( copy ) );
            }

            /// Same, moving the value in.
            size_t Push( T &&value )
            {
                T moved( std::move( value ) );
                if ( m_nLength == m_nCapacity )
                    Reallocate( TemplateDetail::GrowCapacity( m_nCapacity, m_nLength + 1, m_nScale, sizeof( T ) ) );
                ::new ( m_pData + Slot( m_nLength ) ) T( std::move( moved ) );
                m_nLength++;
                return m_nLength;
            }

                /// Removes and returns the front.
            /// @throws CCollectionException if the queue is empty.
            T Pop( void )
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TQueue is empty" );
                T value( std::move( m_pData[ m_nFront ] ) );
                m_pData[ m_nFront ].~T();
                m_nFront = ( m_nFront + 1 == m_nCapacity ) ? 0 : m_nFront + 1;
                m_nLength--;
                if ( m_nLength == 0 )
                    m_nFront = 0;

                size_t nShrink = TemplateDetail::ShrinkCapacity( m_nCapacity, m_nLength, m_nScale );
                if ( nShrink < m_nCapacity )
                    Reallocate( nShrink );
                return value;
            }

            /// The front element, left in place.
            /// @throws CCollectionException if the queue is empty.
            T &Peek( void )
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TQueue is empty" );
                return m_pData[ m_nFront ];
            }

            /// The front element, left in place.
            /// @throws CCollectionException if the queue is empty.
            const T &Peek( void ) const
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TQueue is empty" );
                return m_pData[ m_nFront ];
            }

                /// A copy of the element nPosition from the front, or oDefault.
            T Get( size_t nPosition, const T &oDefault = T() ) const
            {
                return ( nPosition < m_nLength ) ? At( nPosition ) : oDefault;
            }

            /// Destroys every element. The buffer is kept.
            void Clear( void )
            {
                for ( size_t n = 0; n < m_nLength; n++ )
                    m_pData[ Slot( n ) ].~T();
                m_nLength = 0;
                m_nFront = 0;
            }

            /// For a queue of pointers: deletes what each points to, then empties the queue.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for queues of pointers" );
                for ( size_t n = 0; n < m_nLength; n++ )
                    TemplateDetail::DeletePointer( m_pData[ Slot( n ) ] );
                Clear();
            }

            iterator        begin( void ) { return iterator( this, 0 ); }   ///< The front element.
            iterator        end( void ) { return iterator( this, m_nLength ); }   ///< Past the back element.
            const_iterator  begin( void ) const { return const_iterator( this, 0 ); }   ///< The front element.
            const_iterator  end( void ) const { return const_iterator( this, m_nLength ); }   ///< Past the back element.

                /// Unchecked access by position from the front (nPosition must be less than Length()).
            T          &At( size_t nPosition ) { return m_pData[ Slot( nPosition ) ]; }
            /// Same, for a const queue.
            const T    &At( size_t nPosition ) const { return m_pData[ Slot( nPosition ) ]; }

        protected:
            /// The buffer slot of the element nPosition from the front.
            size_t Slot( size_t nPosition ) const
            {
                size_t nSlot = m_nFront + nPosition;
                return ( nSlot >= m_nCapacity ) ? nSlot - m_nCapacity : nSlot;
            }

            /// Moves the elements to a new buffer of nCapacity slots (at least the length), front first.
            /// @throws std::bad_alloc if the memory cannot be had; the queue is then unchanged.
            void Reallocate( size_t nCapacity )
            {
                if ( nCapacity < m_nLength )
                    nCapacity = m_nLength;
                if ( nCapacity == m_nCapacity )
                    return;

                T *pNew = NULL;
                if ( nCapacity > 0 )
                {
                    if ( nCapacity > ( (size_t)-1 ) / sizeof( T ) )
                        throw std::bad_alloc();
                    pNew = (T *)::operator new( nCapacity * sizeof( T ) );
                    size_t nMoved = 0;
                    try
                    {
                        for ( ; nMoved < m_nLength; nMoved++ )
                            ::new ( pNew + nMoved ) T( std::move_if_noexcept( m_pData[ Slot( nMoved ) ] ) );
                    }
                    catch ( ... )
                    {
                        for ( size_t n = 0; n < nMoved; n++ )
                            pNew[n].~T();
                        ::operator delete( pNew );
                        throw;
                    }
                }
                for ( size_t n = 0; n < m_nLength; n++ )
                    m_pData[ Slot( n ) ].~T();
                ::operator delete( m_pData );
                m_pData = pNew;
                m_nCapacity = nCapacity;
                m_nFront = 0;
            }
    };
} // namespace IASLib

#endif // IASLIB_TQUEUE_H__
