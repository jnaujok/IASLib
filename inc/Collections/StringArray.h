/*
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
    class CStringArray : public CCollection
    {
        protected:
            TArray<CString>     m_aElements;
            CString             m_strScratch;
        public:
                                CStringArray( size_t nScale = 4 );
                                CStringArray( const CStringArray &oSource );
            virtual            ~CStringArray( void );

                                DEFINE_OBJECT( CStringArray )

            virtual CStringArray &operator =( const CStringArray &oSource );

            virtual size_t      GetLength( void ) const { return m_aElements.Length(); }

            virtual size_t      Push( const CString &strNew );
            virtual CString     Get( size_t nIndex ) const;
            virtual CString    &operator []( size_t nIndex );
            virtual const CString &operator []( size_t nIndex ) const;

            virtual size_t      Insert( size_t nIndex, const CString &strNew );
            virtual bool        Delete( size_t nIndex );

                // The index of the first string equal to strFind, or NOT_FOUND.
            virtual size_t      Find( const char *strFind, bool bCaseSensitive = true ) const;
            bool                Contains( const char *strFind, bool bCaseSensitive = true ) const { return Find( strFind, bCaseSensitive ) != NOT_FOUND; }
                // The strings joined, with strSeparator between them.
            CString             Join( const char *strSeparator ) const;

            virtual bool        Sort( bool bAscending );
                // Sorts the elements from nLo to nHi (inclusive).
            virtual void        QuickSort( size_t nLo, size_t nHi, bool bAscending );

            virtual void        DeleteAll( void );
            virtual void        EmptyAll( void );

            virtual CIterator  *Enumerate( void );

            virtual size_t      Put( const CString &strNew ) { return Insert( 0, strNew ); }
            virtual size_t      Append( const CString &strNew ) { return Push( strNew ); }

            const TArray<CString> &GetArray( void ) const { return m_aElements; }
    };
} // namespace IASLib

#endif // IASLIB_STRINGARRAY_H__
