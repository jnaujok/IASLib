/*
 * TArray Template
 *
 *      A growable array of values of any copyable or movable type (see
 * TCommon.h for how it grows). Elements are stored in order, by value, so
 * TArray<int> and TArray<CString> hold the numbers and strings
 * themselves; an array of pointers holds the pointers (DeleteAll()
 * deletes what they point to).
 *
 *      Indexes past the end throw a CCollectionException from operator [],
 *      Remove() and Swap(); Get() gives a default instead. Iterators (and
 *      range-based for) run first to last; changing the array's length
 *      invalidates them.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/26/2020 (rewritten 11/18/2026)
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TARRAY_H__
#define IASLIB_TARRAY_H__

#include "TCommon.h"

#include <algorithm>
#include <initializer_list>

namespace IASLib
{
    template <class T>
    class TArray : public CObject
    {
        protected:
            T              *m_pData;
            size_t          m_nLength;
            size_t          m_nCapacity;
            size_t          m_nScale;

        public:
            typedef T               value_type;
            typedef T              *iterator;
            typedef const T        *const_iterator;

            DECLARE_OBJECT( TArray, CObject )

                // nScale: the least the array grows by.
            explicit TArray( size_t nScale = 16 )
                : m_pData( NULL ), m_nLength( 0 ), m_nCapacity( 0 ), m_nScale( ( nScale > 0 ) ? nScale : 1 )
            {
            }

            TArray( std::initializer_list<T> aValues )
                : m_pData( NULL ), m_nLength( 0 ), m_nCapacity( 0 ), m_nScale( 16 )
            {
                Reserve( aValues.size() );
                for ( const T &value : aValues )
                    Push( value );
            }

            TArray( const TArray &oSource )
                : CObject( oSource ), m_pData( NULL ), m_nLength( 0 ), m_nCapacity( 0 ), m_nScale( oSource.m_nScale )
            {
                Reserve( oSource.m_nLength );
                for ( size_t n = 0; n < oSource.m_nLength; n++ )
                {
                    ::new ( m_pData + n ) T( oSource.m_pData[n] );
                    m_nLength++;
                }
            }

            TArray( TArray &&oSource )
                : CObject( oSource ), m_pData( oSource.m_pData ), m_nLength( oSource.m_nLength ), m_nCapacity( oSource.m_nCapacity ), m_nScale( oSource.m_nScale )
            {
                oSource.m_pData = NULL;
                oSource.m_nLength = 0;
                oSource.m_nCapacity = 0;
            }

            virtual ~TArray( void )
            {
                Clear();
                Release();
            }

            TArray &operator =( const TArray &oSource )
            {
                if ( this != &oSource )
                {
                    TArray oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            TArray &operator =( TArray &&oSource )
            {
                if ( this != &oSource )
                {
                    Clear();
                    Release();
                    m_pData = oSource.m_pData;
                    m_nLength = oSource.m_nLength;
                    m_nCapacity = oSource.m_nCapacity;
                    m_nScale = oSource.m_nScale;
                    oSource.m_pData = NULL;
                    oSource.m_nLength = 0;
                    oSource.m_nCapacity = 0;
                }
                return *this;
            }

                // Exchanges the contents of two arrays (no copying).
            void Swap( TArray &oOther )
            {
                std::swap( m_pData, oOther.m_pData );
                std::swap( m_nLength, oOther.m_nLength );
                std::swap( m_nCapacity, oOther.m_nCapacity );
                std::swap( m_nScale, oOther.m_nScale );
            }

                // Size
            size_t      Length( void ) const { return m_nLength; }
            size_t      GetLength( void ) const { return m_nLength; }
            size_t      Count( void ) const { return m_nLength; }
            size_t      GetCount( void ) const { return m_nLength; }
            bool        IsEmpty( void ) const { return m_nLength == 0; }
            size_t      Capacity( void ) const { return m_nCapacity; }

            void        SetScale( size_t nScale ) { if ( nScale > 0 ) m_nScale = nScale; }
            size_t      GetScale( void ) const { return m_nScale; }

                // Room for nCapacity elements without growing again.
            void Reserve( size_t nCapacity )
            {
                if ( nCapacity > m_nCapacity )
                    Reallocate( nCapacity );
            }

                // Gives back unused room.
            void Compact( void )
            {
                if ( m_nCapacity > m_nLength )
                    Reallocate( m_nLength );
            }

                // Access
            T &operator []( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                return m_pData[ nIndex ];
            }

            const T &operator []( size_t nIndex ) const
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                return m_pData[ nIndex ];
            }

                // The element at nIndex, or oDefault past the end.
            T Get( size_t nIndex, const T &oDefault = T() ) const
            {
                return ( nIndex < m_nLength ) ? m_pData[ nIndex ] : oDefault;
            }

            T          &First( void ) { return (*this)[ 0 ]; }
            const T    &First( void ) const { return (*this)[ 0 ]; }
            T          &Last( void ) { return (*this)[ m_nLength - 1 ]; }
            const T    &Last( void ) const { return (*this)[ m_nLength - 1 ]; }

            T          *Data( void ) { return m_pData; }
            const T    *Data( void ) const { return m_pData; }

                // Adding. Each returns the index the value went to.
            size_t Push( const T &value ) { return Insert( m_nLength, value ); }
            size_t Push( T &&value ) { return Insert( m_nLength, std::move( value ) ); }
            size_t Append( const T &value ) { return Insert( m_nLength, value ); }
            size_t Append( T &&value ) { return Insert( m_nLength, std::move( value ) ); }
            size_t Prepend( const T &value ) { return Insert( 0, value ); }
            size_t Prepend( T &&value ) { return Insert( 0, std::move( value ) ); }

                // Inserts before nIndex (at the end, if nIndex is past it).
            size_t Insert( size_t nIndex, const T &value )
            {
                T copy( value );        // value may be one of our own elements
                return Insert( nIndex, std::move( copy ) );
            }

            size_t Insert( size_t nIndex, T &&value )
            {
                if ( nIndex > m_nLength )
                    nIndex = m_nLength;
                    // value may be one of our own elements, which are about
                    // to move.
                T moved( std::move( value ) );
                if ( m_nLength == m_nCapacity )
                    Reallocate( TemplateDetail::GrowCapacity( m_nCapacity, m_nLength + 1, m_nScale, sizeof( T ) ) );
                return InsertRoom( nIndex, std::move( moved ) );
            }

        protected:
                // Inserts, with room for one more already there.
            size_t InsertRoom( size_t nIndex, T &&value )
            {

                if ( nIndex == m_nLength )
                {
                    ::new ( m_pData + m_nLength ) T( std::move( value ) );
                }
                else
                {
                        // Shift up one: construct the new last element, then
                        // move the rest along.
                    ::new ( m_pData + m_nLength ) T( std::move( m_pData[ m_nLength - 1 ] ) );
                    for ( size_t n = m_nLength - 1; n > nIndex; n-- )
                        m_pData[n] = std::move( m_pData[n - 1] );
                    m_pData[ nIndex ] = std::move( value );
                }
                m_nLength++;
                return nIndex;
            }

        public:
                // Replaces the element at nIndex; past the end, appends.
                // Returns the index the value went to.
            size_t Set( size_t nIndex, const T &value )
            {
                if ( nIndex >= m_nLength )
                    return Push( value );
                m_pData[ nIndex ] = value;
                return nIndex;
            }

                // Removing.
            T Remove( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                T value( std::move( m_pData[ nIndex ] ) );
                Erase( nIndex );
                return value;
            }

                // Removes the first element equal to value.
            bool RemoveValue( const T &value )
            {
                size_t nIndex = Find( value );
                if ( nIndex == NOT_FOUND )
                    return false;
                Erase( nIndex );
                return true;
            }

                // Removes and returns the last element.
            T Pop( void )
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TArray is empty" );
                return Remove( m_nLength - 1 );
            }

            void Clear( void )
            {
                for ( size_t n = 0; n < m_nLength; n++ )
                    m_pData[n].~T();
                m_nLength = 0;
            }

                // For an array of pointers: deletes what each points to, then
                // empties the array.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for arrays of pointers" );
                for ( size_t n = 0; n < m_nLength; n++ )
                    TemplateDetail::DeletePointer( m_pData[n] );
                Clear();
            }

                // Searching (with operator ==).
            size_t Find( const T &value, size_t nStart = 0 ) const
            {
                for ( size_t n = nStart; n < m_nLength; n++ )
                    if ( m_pData[n] == value )
                        return n;
                return NOT_FOUND;
            }

            bool Contains( const T &value ) const { return Find( value ) != NOT_FOUND; }

                // Ordering. Sort() uses operator <, or a comparison giving
                // whether its first argument goes before its second.
            void Sort( void ) { std::sort( begin(), end(), TLess<T>() ); }
            template <class Compare> void Sort( Compare fnLess ) { std::sort( begin(), end(), fnLess ); }
            void StableSort( void ) { std::stable_sort( begin(), end(), TLess<T>() ); }
            template <class Compare> void StableSort( Compare fnLess ) { std::stable_sort( begin(), end(), fnLess ); }
            void Reverse( void ) { std::reverse( begin(), end() ); }

            bool Swap( size_t nIndex1, size_t nIndex2 )
            {
                if ( nIndex1 >= m_nLength || nIndex2 >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                if ( nIndex1 == nIndex2 )
                    return false;
                std::swap( m_pData[ nIndex1 ], m_pData[ nIndex2 ] );
                return true;
            }

                // Equal if the same length with equal elements.
            bool operator ==( const TArray &oOther ) const
            {
                if ( m_nLength != oOther.m_nLength )
                    return false;
                for ( size_t n = 0; n < m_nLength; n++ )
                    if ( ! ( m_pData[n] == oOther.m_pData[n] ) )
                        return false;
                return true;
            }
            bool operator !=( const TArray &oOther ) const { return ! ( *this == oOther ); }

                // Iteration
            iterator        begin( void ) { return m_pData; }
            iterator        end( void ) { return m_pData + m_nLength; }
            const_iterator  begin( void ) const { return m_pData; }
            const_iterator  end( void ) const { return m_pData + m_nLength; }

        protected:
                // Removes the element at nIndex (already checked).
            void Erase( size_t nIndex )
            {
                for ( size_t n = nIndex; n + 1 < m_nLength; n++ )
                    m_pData[n] = std::move( m_pData[n + 1] );
                m_pData[ m_nLength - 1 ].~T();
                m_nLength--;

                size_t nShrink = TemplateDetail::ShrinkCapacity( m_nCapacity, m_nLength, m_nScale );
                if ( nShrink < m_nCapacity )
                    Reallocate( nShrink );
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
                            ::new ( pNew + nMoved ) T( std::move_if_noexcept( m_pData[ nMoved ] ) );
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
                    m_pData[n].~T();
                ::operator delete( m_pData );
                m_pData = pNew;
                m_nCapacity = nCapacity;
            }

            void Release( void )
            {
                ::operator delete( m_pData );
                m_pData = NULL;
                m_nCapacity = 0;
            }
    };
} // namespace IASLib

#endif // IASLIB_TARRAY_H__
