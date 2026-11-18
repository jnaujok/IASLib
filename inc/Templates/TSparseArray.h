/**
 * @file
 * @brief TSparseArray Template.
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
    /// An array where only some indexes have values.
    ///
    /// Entries are kept in a TArray sorted by index, so lookups are a binary
    /// search and memory is used only for indexes that are set. Iteration
    /// gives Entry objects in index order. Adding or removing entries
    /// invalidates pointers from Find() and iterators. Not thread-safe.
    ///
    /// @tparam T The value type.
    ///
    /// @code
    /// TSparseArray<CString> aNames;
    /// aNames.Set( 1000, "thousand" );
    /// aNames[ 7 ] = "seven";
    /// for ( const auto &oEntry : aNames )
    ///     printf( "%zu %s\n", oEntry.Index, (const char *)oEntry.Value );
    /// @endcode
    template <class T>
    class TSparseArray : public CObject
    {
        public:
            /// One index and its value.
            struct Entry
            {
                size_t  Index;   ///< The index.
                T       Value;   ///< The value at that index.

                Entry( void ) : Index( 0 ), Value() {}   ///< Index 0 with a default value.
                Entry( size_t nIndex, const T &value ) : Index( nIndex ), Value( value ) {}   ///< An index with a copied value.
                Entry( size_t nIndex, T &&value ) : Index( nIndex ), Value( std::move( value ) ) {}   ///< An index with a moved value.
                bool operator ==( const Entry &o ) const { return Index == o.Index && Value == o.Value; }   ///< Same index and equal values.
            };

            typedef typename TArray<Entry>::iterator        iterator;   ///< Iterator over entries, in index order.
            typedef typename TArray<Entry>::const_iterator  const_iterator;   ///< Const iterator over entries, in index order.

        protected:
            TArray<Entry>   m_aEntries;   ///< The entries, sorted by index.

        public:
            DECLARE_OBJECT( TSparseArray, CObject )

            /// Makes an empty sparse array.
            /// @param nScale The least the entry storage grows by.
            explicit TSparseArray( size_t nScale = 16 ) : m_aEntries( nScale ) {}

            size_t      Length( void ) const { return m_aEntries.Length(); }   ///< Number of indexes that have values (not the highest index).
            size_t      GetLength( void ) const { return m_aEntries.Length(); }   ///< Same as Length().
            size_t      Count( void ) const { return m_aEntries.Length(); }   ///< Same as Length().
            size_t      GetCount( void ) const { return m_aEntries.Length(); }   ///< Same as Length().
            bool        IsEmpty( void ) const { return m_aEntries.IsEmpty(); }   ///< True if no index has a value.
            void        Compact( void ) { m_aEntries.Compact(); }   ///< Gives back unused room.

                /// Sets the value at nIndex (adding or replacing it).
            void Set( size_t nIndex, const T &value )
            {
                size_t nAt = Position( nIndex );
                if ( nAt < m_aEntries.Length() && m_aEntries[ nAt ].Index == nIndex )
                    m_aEntries[ nAt ].Value = value;
                else
                    m_aEntries.Insert( nAt, Entry( nIndex, value ) );
            }

            /// Same, moving the value in.
            void Set( size_t nIndex, T &&value )
            {
                size_t nAt = Position( nIndex );
                if ( nAt < m_aEntries.Length() && m_aEntries[ nAt ].Index == nIndex )
                    m_aEntries[ nAt ].Value = std::move( value );
                else
                    m_aEntries.Insert( nAt, Entry( nIndex, std::move( value ) ) );
            }

            bool Exists( size_t nIndex ) const { return Lookup( nIndex ) != NOT_FOUND; }   ///< True if nIndex has a value.

                /// The value at nIndex, or NULL if there is none. The pointer is valid until entries are added or removed.
            T *Find( size_t nIndex )
            {
                size_t nAt = Lookup( nIndex );
                return ( nAt != NOT_FOUND ) ? &m_aEntries[ nAt ].Value : NULL;
            }

            /// Same, for a const array.
            const T *Find( size_t nIndex ) const
            {
                size_t nAt = Lookup( nIndex );
                return ( nAt != NOT_FOUND ) ? &m_aEntries[ nAt ].Value : NULL;
            }

                /// A copy of the value at nIndex, or oDefault if there is none.
            T Get( size_t nIndex, const T &oDefault = T() ) const
            {
                const T *pValue = Find( nIndex );
                return ( pValue != NULL ) ? *pValue : oDefault;
            }

                /// The value at nIndex, added (as T()) if there is none.
            T &operator []( size_t nIndex )
            {
                size_t nAt = Position( nIndex );
                if ( nAt >= m_aEntries.Length() || m_aEntries[ nAt ].Index != nIndex )
                    m_aEntries.Insert( nAt, Entry( nIndex, T() ) );
                return m_aEntries[ nAt ].Value;
            }

            /// Removes the value at nIndex. @return False if there was none.
            bool Remove( size_t nIndex )
            {
                size_t nAt = Lookup( nIndex );
                if ( nAt == NOT_FOUND )
                    return false;
                m_aEntries.Remove( nAt );
                return true;
            }

                /// The nth entry, in index order.
            /// @throws CCollectionException if nPosition >= Length().
            const Entry &GetEntry( size_t nPosition ) const { return m_aEntries[ nPosition ]; }

                /// The lowest index in use (0 if empty).
            size_t FirstIndex( void ) const { return m_aEntries.IsEmpty() ? 0 : m_aEntries.First().Index; }
            /// The highest index in use (0 if empty).
            size_t LastIndex( void ) const { return m_aEntries.IsEmpty() ? 0 : m_aEntries.Last().Index; }

            void Clear( void ) { m_aEntries.Clear(); }   ///< Removes every entry.

                /// For pointer values: deletes what each points to, then empties.
            void DeleteAll( void )
            {
                static_assert( std::is_pointer<T>::value, "DeleteAll() is for arrays of pointers" );
                for ( size_t n = 0; n < m_aEntries.Length(); n++ )
                    TemplateDetail::DeletePointer( m_aEntries[n].Value );
                Clear();
            }

            iterator        begin( void ) { return m_aEntries.begin(); }   ///< The entry with the lowest index.
            iterator        end( void ) { return m_aEntries.end(); }   ///< Past the last entry.
            const_iterator  begin( void ) const { return m_aEntries.begin(); }   ///< The entry with the lowest index.
            const_iterator  end( void ) const { return m_aEntries.end(); }   ///< Past the last entry.

        protected:
                /// Where an entry for nIndex is, or would go.
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

            /// The position of the entry for nIndex, or NOT_FOUND.
            size_t Lookup( size_t nIndex ) const
            {
                size_t nAt = Position( nIndex );
                return ( nAt < m_aEntries.Length() && m_aEntries[ nAt ].Index == nIndex ) ? nAt : NOT_FOUND;
            }
    };
} // namespace IASLib

#endif // IASLIB_TSPARSEARRAY_H__
