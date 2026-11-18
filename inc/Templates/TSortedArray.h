/**
 * @file
 * @brief TSortedArray Template.
 *
 *      An array kept in order: Push() puts each value where it belongs
 * (after any equal ones, so equal values keep the order they came in),
 * and Find() is a binary search. The order comes from Less, a comparison
 * giving whether its first argument goes before its second (operator <
 * by default); a comparison object can be given to the constructor.
 * Elements can be read but not changed in place, which could break the
 * order.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TSORTEDARRAY_H__
#define IASLIB_TSORTEDARRAY_H__

#include "TArray.h"

namespace IASLib
{
    /// An array kept in sorted order, with binary search.
    ///
    /// Push() puts each value in its place, after any equal values, so
    /// equal values keep the order they were added in. Elements are read-only
    /// through this class (changing one could break the order). Not thread-safe.
    ///
    /// @tparam T The element type.
    /// @tparam Less The ordering: a function object where Less( a, b ) is true
    ///         if a goes before b. The default is operator <.
    ///
    /// @code
    /// TSortedArray<int> aSorted;
    /// aSorted.Push( 5 );
    /// aSorted.Push( 1 );
    /// aSorted.Push( 3 );              // 1, 3, 5
    /// bool bHas = aSorted.Contains( 3 );
    /// @endcode
    template <class T, class Less = TLess<T> >
    class TSortedArray : public CObject
    {
        protected:
            TArray<T>       m_aItems;   ///< The elements, in order.
            Less            m_fnLess;   ///< The ordering.

        public:
            typedef T                               value_type;   ///< The element type.
            typedef typename TArray<T>::const_iterator const_iterator;   ///< Const iterator (a pointer).
            typedef const_iterator                  iterator;   ///< Iterators are always const.

            DECLARE_OBJECT( TSortedArray, CObject )

            /// Makes an empty sorted array.
            /// @param fnLess The ordering.
            /// @param nScale The least the array grows by.
            explicit TSortedArray( Less fnLess = Less(), size_t nScale = 16 ) : m_aItems( nScale ), m_fnLess( fnLess ) {}

            size_t      Length( void ) const { return m_aItems.Length(); }   ///< Number of elements.
            size_t      GetLength( void ) const { return m_aItems.Length(); }   ///< Same as Length().
            size_t      Count( void ) const { return m_aItems.Length(); }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_aItems.Length(); }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_aItems.IsEmpty(); }   ///< True if there are no elements.
            void        Reserve( size_t nCapacity ) { m_aItems.Reserve( nCapacity ); }   ///< Room for nCapacity elements without growing.
            void        Compact( void ) { m_aItems.Compact(); }   ///< Gives back unused room.
            void        SetScale( size_t nScale ) { m_aItems.SetScale( nScale ); }   ///< Sets the least the array grows by.

                /// Inserts in order (after equal values), returning the index it went to.
            size_t Push( const T &value ) { return m_aItems.Insert( UpperBound( value ), value ); }
            /// Same, moving the value in.
            size_t Push( T &&value )
            {
                size_t nIndex = UpperBound( value );
                return m_aItems.Insert( nIndex, std::move( value ) );
            }

                /// Inserts only if no equal value is there; returns its index
                /// either way (the existing value's index if there was one).
            size_t PushUnique( const T &value )
            {
                size_t nIndex = LowerBound( value );
                if ( nIndex < m_aItems.Length() && ! m_fnLess( value, m_aItems[ nIndex ] ) )
                    return nIndex;
                return m_aItems.Insert( nIndex, value );
            }

            const T    &operator []( size_t nIndex ) const { return m_aItems[ nIndex ]; }   ///< The element at nIndex. @throws CCollectionException if nIndex is past the end.
            T           Get( size_t nIndex, const T &oDefault = T() ) const { return m_aItems.Get( nIndex, oDefault ); }   ///< A copy of the element at nIndex, or oDefault past the end.
            const T    &First( void ) const { return m_aItems.First(); }   ///< The smallest element. @throws CCollectionException if empty.
            const T    &Last( void ) const { return m_aItems.Last(); }   ///< The largest element. @throws CCollectionException if empty.

                /// The index of the first element equal to value, or NOT_FOUND (binary search).
            size_t Find( const T &value ) const
            {
                size_t nIndex = LowerBound( value );
                if ( nIndex < m_aItems.Length() && ! m_fnLess( value, m_aItems[ nIndex ] ) )
                    return nIndex;
                return NOT_FOUND;
            }

            bool Contains( const T &value ) const { return Find( value ) != NOT_FOUND; }   ///< True if an equal value is present.

                /// The first index whose element is not before value (Length() if none).
            size_t LowerBound( const T &value ) const
            {
                return (size_t)( std::lower_bound( m_aItems.begin(), m_aItems.end(), value, m_fnLess ) - m_aItems.begin() );
            }

            /// The first index whose element is after value (Length() if none).
            size_t UpperBound( const T &value ) const
            {
                return (size_t)( std::upper_bound( m_aItems.begin(), m_aItems.end(), value, m_fnLess ) - m_aItems.begin() );
            }

            T       Remove( size_t nIndex ) { return m_aItems.Remove( nIndex ); }   ///< Removes and returns the element at nIndex. @throws CCollectionException if nIndex is past the end.
            /// Removes the first element equal to value. @return False if there is none.
            bool    RemoveValue( const T &value )
            {
                size_t nIndex = Find( value );
                if ( nIndex == NOT_FOUND )
                    return false;
                m_aItems.Remove( nIndex );
                return true;
            }

            void    Clear( void ) { m_aItems.Clear(); }   ///< Removes every element.
            void    DeleteAll( void ) { m_aItems.DeleteAll(); }   ///< For pointers: deletes what each points to, then empties the array.

            const_iterator begin( void ) const { return m_aItems.begin(); }   ///< The smallest element.
            const_iterator end( void ) const { return m_aItems.end(); }   ///< Past the largest element.
    };
} // namespace IASLib

#endif // IASLIB_TSORTEDARRAY_H__
