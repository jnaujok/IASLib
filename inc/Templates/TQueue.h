/*
 * TQueue Template
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
    template <class T>
    class TQueue : public CObject
    {
        protected:
            T              *m_pData;
            size_t          m_nCapacity;
            size_t          m_nFront;       // where the front element is
            size_t          m_nLength;
            size_t          m_nScale;

        public:
            template <class Q, class V>
            class TQueueIterator
            {
                    Q          *m_pQueue;
                    size_t      m_nPosition;
                public:
                    typedef std::forward_iterator_tag   iterator_category;
                    typedef V                           value_type;
                    typedef ptrdiff_t                   difference_type;
                    typedef V                          *pointer;
                    typedef V                          &reference;

                    TQueueIterator( Q *pQueue, size_t nPosition ) : m_pQueue( pQueue ), m_nPosition( nPosition ) {}
                    V &operator *( void ) const { return m_pQueue->At( m_nPosition ); }
                    V *operator ->( void ) const { return &m_pQueue->At( m_nPosition ); }
                    TQueueIterator &operator ++( void ) { m_nPosition++; return *this; }
                    TQueueIterator operator ++( int ) { TQueueIterator old( *this ); m_nPosition++; return old; }
                    bool operator ==( const TQueueIterator &o ) const { return m_pQueue == o.m_pQueue && m_nPosition == o.m_nPosition; }
                    bool operator !=( const TQueueIterator &o ) const { return ! ( *this == o ); }
            };

            typedef TQueueIterator<TQueue, T>               iterator;
            typedef TQueueIterator<const TQueue, const T>   const_iterator;

            DECLARE_OBJECT( TQueue, CObject )

            explicit TQueue( size_t nScale = 16 )
                : m_pData( NULL ), m_nCapacity( 0 ), m_nFront( 0 ), m_nLength( 0 ), m_nScale( ( nScale > 0 ) ? nScale : 1 )
            {
            }

            TQueue( const TQueue &oSource )
                : CObject( oSource ), m_pData( NULL ), m_nCapacity( 0 ), m_nFront( 0 ), m_nLength( 0 ), m_nScale( oSource.m_nScale )
            {
                Reallocate( oSource.m_nLength );
                for ( size_t n = 0; n < oSource.m_nLength; n++ )
                    Push( oSource.At( n ) );
            }

            TQueue( TQueue &&oSource )
                : CObject( oSource ), m_pData( oSource.m_pData ), m_nCapacity( oSource.m_nCapacity ), m_nFront( oSource.m_nFront ),
                  m_nLength( oSource.m_nLength ), m_nScale( oSource.m_nScale )
            {
                oSource.m_pData = NULL;
                oSource.m_nCapacity = oSource.m_nFront = oSource.m_nLength = 0;
            }

            virtual ~TQueue( void )
            {
                Clear();
                ::operator delete( m_pData );
            }

            TQueue &operator =( const TQueue &oSource )
            {
                if ( this != &oSource )
                {
                    TQueue oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            TQueue &operator =( TQueue &&oSource )
            {
                if ( this != &oSource )
                {
                    TQueue oTaken( std::move( oSource ) );
                    Swap( oTaken );
                }
                return *this;
            }

            void Swap( TQueue &oOther )
            {
                std::swap( m_pData, oOther.m_pData );
                std::swap( m_nCapacity, oOther.m_nCapacity );
                std::swap( m_nFront, oOther.m_nFront );
                std::swap( m_nLength, oOther.m_nLength );
                std::swap( m_nScale, oOther.m_nScale );
            }

            size_t      Length( void ) const { return m_nLength; }
            size_t      GetLength( void ) const { return m_nLength; }
            size_t      Count( void ) const { return m_nLength; }
            size_t      GetCount( void ) const { return m_nLength; }
            bool        IsEmpty( void ) const { return m_nLength == 0; }
            size_t      Capacity( void ) const { return m_nCapacity; }
            void        SetScale( size_t nScale ) { if ( nScale > 0 ) m_nScale = nScale; }
            void        Reserve( size_t nCapacity ) { if ( nCapacity > m_nCapacity ) Reallocate( nCapacity ); }
            void        Compact( void ) { if ( m_nCapacity > m_nLength ) Reallocate( m_nLength ); }

                // Adds at the back, returning the new length.
            size_t Push( const T &value )
            {
                T copy( value );
                return Push( std::move( copy ) );
            }

            size_t Push( T &&value )
            {
                T moved( std::move( value ) );
                if ( m_nLength == m_nCapacity )
                    Reallocate( TemplateDetail::GrowCapacity( m_nCapacity, m_nLength + 1, m_nScale, sizeof( T ) ) );
                ::new ( m_pData + Slot( m_nLength ) ) T( std::move( moved ) );
                m_nLength++;
                return m_nLength;
            }

                // Removes and returns the front.
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

            T &Peek( void )
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TQueue is empty" );
                return m_pData[ m_nFront ];
            }

            const T &Peek( void ) const
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TQueue is empty" );
                return m_pData[ m_nFront ];
            }

                // The element nPosition from the front, or oDefault.
            T Get( size_t nPosition, const T &oDefault = T() ) const
            {
                return ( nPosition < m_nLength ) ? At( nPosition ) : oDefault;
            }

            void Clear( void )
            {
                for ( size_t n = 0; n < m_nLength; n++ )
                    m_pData[ Slot( n ) ].~T();
                m_nLength = 0;
                m_nFront = 0;
            }

            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for queues of pointers" );
                for ( size_t n = 0; n < m_nLength; n++ )
                    TemplateDetail::DeletePointer( m_pData[ Slot( n ) ] );
                Clear();
            }

            iterator        begin( void ) { return iterator( this, 0 ); }
            iterator        end( void ) { return iterator( this, m_nLength ); }
            const_iterator  begin( void ) const { return const_iterator( this, 0 ); }
            const_iterator  end( void ) const { return const_iterator( this, m_nLength ); }

                // Unchecked access by position from the front.
            T          &At( size_t nPosition ) { return m_pData[ Slot( nPosition ) ]; }
            const T    &At( size_t nPosition ) const { return m_pData[ Slot( nPosition ) ]; }

        protected:
            size_t Slot( size_t nPosition ) const
            {
                size_t nSlot = m_nFront + nPosition;
                return ( nSlot >= m_nCapacity ) ? nSlot - m_nCapacity : nSlot;
            }

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
