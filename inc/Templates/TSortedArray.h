/*
 * TSortedArray Template
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
    template <class T, class Less = TLess<T> >
    class TSortedArray : public CObject
    {
        protected:
            TArray<T>       m_aItems;
            Less            m_fnLess;

        public:
            typedef T                               value_type;
            typedef typename TArray<T>::const_iterator const_iterator;
            typedef const_iterator                  iterator;

            DECLARE_OBJECT( TSortedArray, CObject )

            explicit TSortedArray( Less fnLess = Less(), size_t nScale = 16 ) : m_aItems( nScale ), m_fnLess( fnLess ) {}

            size_t      Length( void ) const { return m_aItems.Length(); }
            size_t      GetLength( void ) const { return m_aItems.Length(); }
            size_t      Count( void ) const { return m_aItems.Length(); }
            size_t      GetCount( void ) const { return m_aItems.Length(); }
            bool        IsEmpty( void ) const { return m_aItems.IsEmpty(); }
            void        Reserve( size_t nCapacity ) { m_aItems.Reserve( nCapacity ); }
            void        Compact( void ) { m_aItems.Compact(); }
            void        SetScale( size_t nScale ) { m_aItems.SetScale( nScale ); }

                // Inserts in order, returning the index it went to.
            size_t Push( const T &value ) { return m_aItems.Insert( UpperBound( value ), value ); }
            size_t Push( T &&value )
            {
                size_t nIndex = UpperBound( value );
                return m_aItems.Insert( nIndex, std::move( value ) );
            }

                // Inserts only if no equal value is there; returns its index
                // either way.
            size_t PushUnique( const T &value )
            {
                size_t nIndex = LowerBound( value );
                if ( nIndex < m_aItems.Length() && ! m_fnLess( value, m_aItems[ nIndex ] ) )
                    return nIndex;
                return m_aItems.Insert( nIndex, value );
            }

            const T    &operator []( size_t nIndex ) const { return m_aItems[ nIndex ]; }
            T           Get( size_t nIndex, const T &oDefault = T() ) const { return m_aItems.Get( nIndex, oDefault ); }
            const T    &First( void ) const { return m_aItems.First(); }
            const T    &Last( void ) const { return m_aItems.Last(); }

                // The index of the first element equal to value, or NOT_FOUND.
            size_t Find( const T &value ) const
            {
                size_t nIndex = LowerBound( value );
                if ( nIndex < m_aItems.Length() && ! m_fnLess( value, m_aItems[ nIndex ] ) )
                    return nIndex;
                return NOT_FOUND;
            }

            bool Contains( const T &value ) const { return Find( value ) != NOT_FOUND; }

                // The first index whose element is not before value, and the
                // first whose element is after it.
            size_t LowerBound( const T &value ) const
            {
                return (size_t)( std::lower_bound( m_aItems.begin(), m_aItems.end(), value, m_fnLess ) - m_aItems.begin() );
            }

            size_t UpperBound( const T &value ) const
            {
                return (size_t)( std::upper_bound( m_aItems.begin(), m_aItems.end(), value, m_fnLess ) - m_aItems.begin() );
            }

            T       Remove( size_t nIndex ) { return m_aItems.Remove( nIndex ); }
            bool    RemoveValue( const T &value )
            {
                size_t nIndex = Find( value );
                if ( nIndex == NOT_FOUND )
                    return false;
                m_aItems.Remove( nIndex );
                return true;
            }

            void    Clear( void ) { m_aItems.Clear(); }
            void    DeleteAll( void ) { m_aItems.DeleteAll(); }

            const_iterator begin( void ) const { return m_aItems.begin(); }
            const_iterator end( void ) const { return m_aItems.end(); }
    };
} // namespace IASLib

#endif // IASLIB_TSORTEDARRAY_H__
