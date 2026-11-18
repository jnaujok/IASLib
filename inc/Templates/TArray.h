/**
 * @file
 * @brief TArray Template.
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
    /// A growable array of values, stored in order and by value.
    ///
    /// Elements live in one block of memory, so Data() can be passed to C
    /// functions, and the iterators are plain pointers. The array grows as
    /// described in TCommon.h. Indexing past the end with operator [] throws
    /// a CCollectionException; Get() returns a default instead.
    ///
    /// An array of pointers does not own what they point to; call
    /// DeleteAll() to delete them. Not thread-safe.
    ///
    /// @tparam T The element type: anything copyable or movable.
    ///
    /// @code
    /// TArray<CString> aNames = { "Ann", "Bob" };
    /// aNames.Push( "Cy" );
    /// aNames.Sort();
    /// for ( const CString &strName : aNames )
    ///     puts( strName );
    /// size_t nAt = aNames.Find( "Bob" );     // 1, or NOT_FOUND
    /// @endcode
    template <class T>
    class TArray : public CObject
    {
        protected:
            T              *m_pData;   ///< The elements (raw storage of m_nCapacity slots).
            size_t          m_nLength;   ///< Number of elements in use.
            size_t          m_nCapacity;   ///< Number of slots allocated.
            size_t          m_nScale;   ///< The least the array grows by.

        public:
            typedef T               value_type;   ///< The element type.
            typedef T              *iterator;   ///< Iterator (a plain pointer).
            typedef const T        *const_iterator;   ///< Const iterator (a plain pointer).

            DECLARE_OBJECT( TArray, CObject )

                /// Makes an empty array (no memory is allocated until needed).
            /// @param nScale The least the array grows by (0 is taken as 1).
            explicit TArray( size_t nScale = 16 )
                : m_pData( NULL ), m_nLength( 0 ), m_nCapacity( 0 ), m_nScale( ( nScale > 0 ) ? nScale : 1 )
            {
            }

            /// Makes an array holding copies of the listed values (scale 16).
            TArray( std::initializer_list<T> aValues )
                : m_pData( NULL ), m_nLength( 0 ), m_nCapacity( 0 ), m_nScale( 16 )
            {
                Reserve( aValues.size() );
                for ( const T &value : aValues )
                    Push( value );
            }

            /// Copies another array, element by element.
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

            /// Takes over another array's storage, leaving it empty.
            TArray( TArray &&oSource )
                : CObject( oSource ), m_pData( oSource.m_pData ), m_nLength( oSource.m_nLength ), m_nCapacity( oSource.m_nCapacity ), m_nScale( oSource.m_nScale )
            {
                oSource.m_pData = NULL;
                oSource.m_nLength = 0;
                oSource.m_nCapacity = 0;
            }

            /// Destroys the elements and frees the storage. Pointed-to objects are not deleted.
            virtual ~TArray( void )
            {
                Clear();
                Release();
            }

            /// Replaces the contents with copies of another array's (scale included).
            /// If a copy throws, this array is unchanged.
            TArray &operator =( const TArray &oSource )
            {
                if ( this != &oSource )
                {
                    TArray oCopy( oSource );
                    Swap( oCopy );
                }
                return *this;
            }

            /// Replaces the contents by taking over another array's storage, leaving it empty.
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

                /// Exchanges the contents of two arrays (no copying).
            void Swap( TArray &oOther )
            {
                std::swap( m_pData, oOther.m_pData );
                std::swap( m_nLength, oOther.m_nLength );
                std::swap( m_nCapacity, oOther.m_nCapacity );
                std::swap( m_nScale, oOther.m_nScale );
            }

            /// @name Size
            ///@{
            size_t      Length( void ) const { return m_nLength; }   ///< Number of elements.
            size_t      GetLength( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      Count( void ) const { return m_nLength; }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_nLength; }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_nLength == 0; }   ///< True if there are no elements.
            size_t      Capacity( void ) const { return m_nCapacity; }   ///< Number of elements that fit without growing.

            void        SetScale( size_t nScale ) { if ( nScale > 0 ) m_nScale = nScale; }   ///< Sets the least the array grows by (0 is ignored).
            size_t      GetScale( void ) const { return m_nScale; }   ///< The least the array grows by.

                /// Room for nCapacity elements without growing again. It never shrinks.
            void Reserve( size_t nCapacity )
            {
                if ( nCapacity > m_nCapacity )
                    Reallocate( nCapacity );
            }

                /// Gives back unused room (capacity becomes the length).
            void Compact( void )
            {
                if ( m_nCapacity > m_nLength )
                    Reallocate( m_nLength );
            }

                ///@}
            /// @name Access
            ///@{

            /// The element at nIndex.
            /// @throws CCollectionException if nIndex is past the end.
            T &operator []( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                return m_pData[ nIndex ];
            }

            /// The element at nIndex.
            /// @throws CCollectionException if nIndex is past the end.
            const T &operator []( size_t nIndex ) const
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                return m_pData[ nIndex ];
            }

                /// A copy of the element at nIndex, or oDefault past the end.
            T Get( size_t nIndex, const T &oDefault = T() ) const
            {
                return ( nIndex < m_nLength ) ? m_pData[ nIndex ] : oDefault;
            }

            T          &First( void ) { return (*this)[ 0 ]; }   ///< The first element. @throws CCollectionException if empty.
            const T    &First( void ) const { return (*this)[ 0 ]; }   ///< The first element. @throws CCollectionException if empty.
            T          &Last( void ) { return (*this)[ m_nLength - 1 ]; }   ///< The last element. @throws CCollectionException if empty.
            const T    &Last( void ) const { return (*this)[ m_nLength - 1 ]; }   ///< The last element. @throws CCollectionException if empty.

            T          *Data( void ) { return m_pData; }   ///< The elements in one block (NULL if nothing was allocated). Valid until the length changes.
            const T    *Data( void ) const { return m_pData; }   ///< The elements in one block (NULL if nothing was allocated). Valid until the length changes.

                ///@}
                /// @name Adding
            /// Each returns the index the value went to. A value may be one of the
            /// array's own elements.
            ///@{
            size_t Push( const T &value ) { return Insert( m_nLength, value ); }   ///< Adds a copy at the end.
            size_t Push( T &&value ) { return Insert( m_nLength, std::move( value ) ); }   ///< Moves a value to the end.
            size_t Append( const T &value ) { return Insert( m_nLength, value ); }   ///< Same as Push().
            size_t Append( T &&value ) { return Insert( m_nLength, std::move( value ) ); }   ///< Same as Push().
            size_t Prepend( const T &value ) { return Insert( 0, value ); }   ///< Adds a copy at the start.
            size_t Prepend( T &&value ) { return Insert( 0, std::move( value ) ); }   ///< Moves a value to the start.

                /// Inserts before nIndex (at the end, if nIndex is past it).
            size_t Insert( size_t nIndex, const T &value )
            {
                T copy( value );        // value may be one of our own elements
                return Insert( nIndex, std::move( copy ) );
            }

            /// Same, moving the value in.
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

        ///@}
        protected:
                /// Inserts, with room for one more already there.
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
                /// Replaces the element at nIndex; past the end, appends.
                /// @return The index the value went to.
            size_t Set( size_t nIndex, const T &value )
            {
                if ( nIndex >= m_nLength )
                    return Push( value );
                m_pData[ nIndex ] = value;
                return nIndex;
            }

                /// @name Removing
                ///@{

            /// Removes the element at nIndex and returns it.
            /// @throws CCollectionException if nIndex is past the end.
            T Remove( size_t nIndex )
            {
                if ( nIndex >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                T value( std::move( m_pData[ nIndex ] ) );
                Erase( nIndex );
                return value;
            }

                /// Removes the first element equal to value. @return False if there is none.
            bool RemoveValue( const T &value )
            {
                size_t nIndex = Find( value );
                if ( nIndex == NOT_FOUND )
                    return false;
                Erase( nIndex );
                return true;
            }

                /// Removes and returns the last element.
            /// @throws CCollectionException if the array is empty.
            T Pop( void )
            {
                if ( m_nLength == 0 )
                    TemplateDetail::ThrowOutOfRange( "TArray is empty" );
                return Remove( m_nLength - 1 );
            }

            /// Destroys every element. The storage is kept (see Compact()).
            void Clear( void )
            {
                for ( size_t n = 0; n < m_nLength; n++ )
                    m_pData[n].~T();
                m_nLength = 0;
            }

                /// For an array of pointers: deletes what each points to, then
                /// empties the array. It does not compile for other element types.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for arrays of pointers" );
                for ( size_t n = 0; n < m_nLength; n++ )
                    TemplateDetail::DeletePointer( m_pData[n] );
                Clear();
            }

                ///@}
                /// @name Searching (with operator ==)
            ///@{

            /// The index of the first element equal to value, from nStart on.
            /// @return The index, or NOT_FOUND.
            size_t Find( const T &value, size_t nStart = 0 ) const
            {
                for ( size_t n = nStart; n < m_nLength; n++ )
                    if ( m_pData[n] == value )
                        return n;
                return NOT_FOUND;
            }

            bool Contains( const T &value ) const { return Find( value ) != NOT_FOUND; }   ///< True if some element equals value.

                ///@}
                /// @name Ordering
                /// Sort() uses operator <, or a comparison giving whether its first argument goes before its second.
            ///@{
            void Sort( void ) { std::sort( begin(), end(), TLess<T>() ); }   ///< Sorts with operator < (not stable).
            template <class Compare> void Sort( Compare fnLess ) { std::sort( begin(), end(), fnLess ); }   ///< Sorts with fnLess( a, b ), true if a goes first (not stable).
            void StableSort( void ) { std::stable_sort( begin(), end(), TLess<T>() ); }   ///< Sorts with operator <, keeping equal elements in order.
            template <class Compare> void StableSort( Compare fnLess ) { std::stable_sort( begin(), end(), fnLess ); }   ///< Sorts with fnLess, keeping equal elements in order.
            void Reverse( void ) { std::reverse( begin(), end() ); }   ///< Reverses the order.

            /// Exchanges two elements.
            /// @return False if the indexes are the same (nothing to do).
            /// @throws CCollectionException if either index is past the end.
            bool Swap( size_t nIndex1, size_t nIndex2 )
            {
                if ( nIndex1 >= m_nLength || nIndex2 >= m_nLength )
                    TemplateDetail::ThrowOutOfRange( "TArray index out of range" );
                if ( nIndex1 == nIndex2 )
                    return false;
                std::swap( m_pData[ nIndex1 ], m_pData[ nIndex2 ] );
                return true;
            }

                ///@}
                /// Equal if the same length with equal elements.
            bool operator ==( const TArray &oOther ) const
            {
                if ( m_nLength != oOther.m_nLength )
                    return false;
                for ( size_t n = 0; n < m_nLength; n++ )
                    if ( ! ( m_pData[n] == oOther.m_pData[n] ) )
                        return false;
                return true;
            }
            bool operator !=( const TArray &oOther ) const { return ! ( *this == oOther ); }   ///< Not equal.

                /// @name Iteration
                /// First to last. Changing the length invalidates iterators.
                ///@{
            iterator        begin( void ) { return m_pData; }   ///< The first element.
            iterator        end( void ) { return m_pData + m_nLength; }   ///< Past the last element.
            const_iterator  begin( void ) const { return m_pData; }   ///< The first element.
            const_iterator  end( void ) const { return m_pData + m_nLength; }   ///< Past the last element.

        ///@}
        protected:
                /// Removes the element at nIndex (already checked), shrinking the storage if it is mostly unused.
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

            /// Moves the elements to new storage of nCapacity slots (at least the length).
            /// @throws std::bad_alloc if the memory cannot be had; the array is then unchanged.
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

            /// Frees the storage (the elements must already be destroyed).
            void Release( void )
            {
                ::operator delete( m_pData );
                m_pData = NULL;
                m_nCapacity = 0;
            }
    };
} // namespace IASLib

#endif // IASLIB_TARRAY_H__
