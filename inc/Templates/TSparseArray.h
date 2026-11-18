/*
 * TSparseArray Template
 *
 *      An array where only some indexes have values: element 5 and element
 * 5,000,000 take the same room. Entries are kept in index order; looking
 * one up is a binary search, and adding one moves the entries after it.
 * Length() is the number of entries, and iterating gives the entries
 * (Index and Value) in index order.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 11/18/2026
 *
 * Copyright (C) 2026, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_TSPARSEARRAY_H__
#define IASLIB_TSPARSEARRAY_H__

#include "TArray.h"

namespace IASLib
{
    template <class T>
    class TSparseArray : public CObject
    {
        public:
            struct Entry
            {
                size_t  Index;
                T       Value;

                Entry( void ) : Index( 0 ), Value() {}
                Entry( size_t nIndex, const T &value ) : Index( nIndex ), Value( value ) {}
                Entry( size_t nIndex, T &&value ) : Index( nIndex ), Value( std::move( value ) ) {}
                bool operator ==( const Entry &o ) const { return Index == o.Index && Value == o.Value; }
            };

            typedef typename TArray<Entry>::iterator        iterator;
            typedef typename TArray<Entry>::const_iterator  const_iterator;

        protected:
            TArray<Entry>   m_aEntries;

        public:
            DECLARE_OBJECT( TSparseArray, CObject )

            explicit TSparseArray( size_t nScale = 16 ) : m_aEntries( nScale ) {}

            size_t      Length( void ) const { return m_aEntries.Length(); }
            size_t      GetLength( void ) const { return m_aEntries.Length(); }
            size_t      Count( void ) const { return m_aEntries.Length(); }
            size_t      GetCount( void ) const { return m_aEntries.Length(); }
            bool        IsEmpty( void ) const { return m_aEntries.IsEmpty(); }
            void        Compact( void ) { m_aEntries.Compact(); }

                // Sets the value at nIndex (adding or replacing it).
            void Set( size_t nIndex, const T &value )
            {
                size_t nAt = Position( nIndex );
                if ( nAt < m_aEntries.Length() && m_aEntries[ nAt ].Index == nIndex )
                    m_aEntries[ nAt ].Value = value;
                else
                    m_aEntries.Insert( nAt, Entry( nIndex, value ) );
            }

            void Set( size_t nIndex, T &&value )
            {
                size_t nAt = Position( nIndex );
                if ( nAt < m_aEntries.Length() && m_aEntries[ nAt ].Index == nIndex )
                    m_aEntries[ nAt ].Value = std::move( value );
                else
                    m_aEntries.Insert( nAt, Entry( nIndex, std::move( value ) ) );
            }

            bool Exists( size_t nIndex ) const { return Lookup( nIndex ) != NOT_FOUND; }

                // The value at nIndex, or NULL if there is none.
            T *Find( size_t nIndex )
            {
                size_t nAt = Lookup( nIndex );
                return ( nAt != NOT_FOUND ) ? &m_aEntries[ nAt ].Value : NULL;
            }

            const T *Find( size_t nIndex ) const
            {
                size_t nAt = Lookup( nIndex );
                return ( nAt != NOT_FOUND ) ? &m_aEntries[ nAt ].Value : NULL;
            }

                // The value at nIndex, or oDefault if there is none.
            T Get( size_t nIndex, const T &oDefault = T() ) const
            {
                const T *pValue = Find( nIndex );
                return ( pValue != NULL ) ? *pValue : oDefault;
            }

                // The value at nIndex, added (as T()) if there is none.
            T &operator []( size_t nIndex )
            {
                size_t nAt = Position( nIndex );
                if ( nAt >= m_aEntries.Length() || m_aEntries[ nAt ].Index != nIndex )
                    m_aEntries.Insert( nAt, Entry( nIndex, T() ) );
                return m_aEntries[ nAt ].Value;
            }

            bool Remove( size_t nIndex )
            {
                size_t nAt = Lookup( nIndex );
                if ( nAt == NOT_FOUND )
                    return false;
                m_aEntries.Remove( nAt );
                return true;
            }

                // The nth entry, in index order.
            const Entry &GetEntry( size_t nPosition ) const { return m_aEntries[ nPosition ]; }

                // The lowest and highest indexes in use (0 if empty).
            size_t FirstIndex( void ) const { return m_aEntries.IsEmpty() ? 0 : m_aEntries.First().Index; }
            size_t LastIndex( void ) const { return m_aEntries.IsEmpty() ? 0 : m_aEntries.Last().Index; }

            void Clear( void ) { m_aEntries.Clear(); }

                // For pointer values: deletes what each points to, then empties.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for arrays of pointers" );
                for ( size_t n = 0; n < m_aEntries.Length(); n++ )
                    TemplateDetail::DeletePointer( m_aEntries[n].Value );
                Clear();
            }

            iterator        begin( void ) { return m_aEntries.begin(); }
            iterator        end( void ) { return m_aEntries.end(); }
            const_iterator  begin( void ) const { return m_aEntries.begin(); }
            const_iterator  end( void ) const { return m_aEntries.end(); }

        protected:
                // Where an entry for nIndex is, or would go.
            size_t Position( size_t nIndex ) const
            {
                size_t nLow = 0, nHigh = m_aEntries.Length();
                while ( nLow < nHigh )
                {
                    size_t nMiddle = nLow + ( nHigh - nLow ) / 2;
                    if ( m_aEntries[ nMiddle ].Index < nIndex )
                        nLow = nMiddle + 1;
                    else
                        nHigh = nMiddle;
                }
                return nLow;
            }

            size_t Lookup( size_t nIndex ) const
            {
                size_t nAt = Position( nIndex );
                return ( nAt < m_aEntries.Length() && m_aEntries[ nAt ].Index == nIndex ) ? nAt : NOT_FOUND;
            }
    };
} // namespace IASLib

#endif // IASLIB_TSPARSEARRAY_H__
