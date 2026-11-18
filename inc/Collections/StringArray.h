/**
 * @file StringArray.h
 * @brief CStringArray, an array of strings with the classic collection
 * interface.
 *
 * CStringArray Class
 *
 *  An array of strings (a TArray<CString> with the classic interface).
 * Get() gives an empty string past the end, and the non-const operator []
 * a scratch string that is not part of the array. Copies are independent
 * (and cheap: the strings share their text until changed).
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STRINGARRAY_H__
#define IASLIB_STRINGARRAY_H__

#include "Collection.h"
#include "../BaseTypes/String_.h"
#include "../Templates/TArray.h"

namespace IASLib
{
    /**
     * An array of strings (a TArray<CString> with the classic interface).
     *
     * Get() gives an empty string past the end, and the non-const
     * operator [] a scratch string that is not part of the array. Copies
     * are independent (and cheap: the strings share their text until
     * changed). Not thread-safe. Its iterator gives the strings through
     * CIterator::NextString() (Next() only steps and returns NULL).
     *
     * @code
     * CStringArray aWords;
     * aWords.Push( "pear" );
     * aWords.Push( "apple" );
     * aWords.Sort( true );
     * CString strAll = aWords.Join( ", " );    // "apple, pear"
     * @endcode
     */
    class CStringArray : public CCollection
    {
        protected:
            TArray<CString>     m_aElements;    ///< The strings.
            CString             m_strScratch;   ///< What the non-const operator [] gives past the end.
        public:
                                /// An empty array.
                                /// @param nScale The least it grows by (0 means 4).
                                CStringArray( size_t nScale = 4 );
                                /// An independent copy.
                                CStringArray( const CStringArray &oSource );
            virtual            ~CStringArray( void );

                                DEFINE_OBJECT( CStringArray )

                /// Makes this an independent copy of oSource.
            virtual CStringArray &operator =( const CStringArray &oSource );

                /// The number of strings.
            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }

                /// Adds a string at the end, returning its index.
            virtual size_t      Push( const CString &strNew );
                /// The string at nIndex, or "" past the end.
            virtual CString     Get( size_t nIndex ) const;
                /// The string at nIndex, to change; past the end, a scratch
                /// string (emptied each time) that is not part of the array.
            virtual CString    &operator []( size_t nIndex );
                /// The string at nIndex, or "" past the end.
            virtual const CString &operator []( size_t nIndex ) const;

                /// Inserts before nIndex (at the end if past it), returning the
                /// index it went to.
            virtual size_t      Insert( size_t nIndex, const CString &strNew );
                /// Removes the string at nIndex; false past the end.
            virtual bool        Delete( size_t nIndex );

                /// The index of the first string equal to strFind, or NOT_FOUND.
            virtual size_t      Find( const char *strFind, bool bCaseSensitive = true ) const;
                /// Whether a string equal to strFind is in the array.
            bool                Contains( const char *strFind, bool bCaseSensitive = true ) const { return Find( strFind, bCaseSensitive ) != NOT_FOUND; }
                /// The strings joined, with strSeparator between them (NULL:
                /// nothing between).
            CString             Join( const char *strSeparator ) const;

                /// Sorts every string (case sensitively, equal ones kept in
                /// order). Always returns true.
            virtual bool        Sort( bool bAscending );
                /// Sorts the elements from nLo to nHi (inclusive; nHi past the
                /// end means to the end).
            virtual void        QuickSort( size_t nLo, size_t nHi, bool bAscending );

                /// Removes every string.
            virtual void        DeleteAll( void );
                /// Removes every string (same as DeleteAll()).
            virtual void        EmptyAll( void );

                /// A new iterator (the caller deletes it); read it with NextString().
            virtual CIterator  *Enumerate( void );

                /// Inserts a string at the start, returning 0.
            virtual size_t      Put( const CString &strNew ) { return Insert( 0, strNew ); }
                /// Same as Push().
            virtual size_t      Append( const CString &strNew ) { return Push( strNew ); }

                /// The underlying template array.
            const TArray<CString> &GetArray( void ) const { return m_aElements; }
    };
} // namespace IASLib

#endif // IASLIB_STRINGARRAY_H__
