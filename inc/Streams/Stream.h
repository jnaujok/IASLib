/*
 * Stream Abstract Class
 *
 *      A consistent interface for reading and writing bytes, whatever is
 * on the other end (a string, a file, a socket, another stream). A stream
 * provides three things, GetBuffer(), PutBuffer() and IsEOS(), plus
 * PeekChar() and Close(); everything else has a default built on them,
 * which streams may replace with something faster.
 *
 *      Reading past the end: GetChar(), GetUChar(), PeekChar() and the
 * binary and number reads throw a CStreamException; GetBuffer() returns
 * what there was (0 at the end); GetLine() returns what is left of the
 * line ("" at the end). GetLine() leaves off the "\n" and a "\r" before it.
 *
 *      Text: << writes numbers and strings as text, and >> reads them back
 * (skipping white space first; a string is one word). Binary: PutUInt16()
 * and the rest write big-endian (network order) integers, and GetUInt16()
 * and the rest read them.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STREAM_H__
#define IASLIB_STREAM_H__

#include "../BaseTypes/IASLib_String.h"

#include <stdint.h>

namespace IASLib
{
    class CStream : public CObject
    {
        protected:
            bool                m_bIsOpen;

        public:
                                CStream( void ) { m_bIsOpen = true; }
            virtual            ~CStream( void ) {}

                                DEFINE_OBJECT( CStream );

            virtual bool        IsOpen( void ) { return m_bIsOpen; }

                // What every stream provides.
                    // Reads up to nLength bytes, returning how many (0 at the end).
            virtual int         GetBuffer( char *achBuffer, int nLength ) = 0;
                    // Writes nLength bytes, returning how many were written.
            virtual int         PutBuffer( const char *achBuffer, int nLength ) = 0;
                    // Whether nothing more can be read.
            virtual bool        IsEOS( void ) = 0;
                    // The next character, without taking it.
            virtual char        PeekChar( void ) = 0;
            virtual void        Close( void ) = 0;

                // Defaults built on those.
            virtual char        GetChar( void );
            virtual unsigned char GetUChar( void );
            virtual void        PutChar( const char chPut );
            virtual void        PutChar( const unsigned char chPut );
            virtual CString     GetLine( void );
                    // Writes the text and "\n".
            virtual void        PutLine( const CString &strOutput );
                    // How many bytes are known to be left (NOT_FOUND if the
                    // stream cannot tell).
            virtual size_t      bytesRemaining( void ) { return NOT_FOUND; }
                    // Sends anything a stream has held back.
            virtual void        Flush( void ) {}

                // Binary, big-endian.
            void                PutUInt8( uint8_t nValue );
            void                PutUInt16( uint16_t nValue );
            void                PutUInt32( uint32_t nValue );
            void                PutUInt64( uint64_t nValue );
            uint8_t             GetUInt8( void );
            uint16_t            GetUInt16( void );
            uint32_t            GetUInt32( void );
            uint64_t            GetUInt64( void );
                    // Exactly nLength bytes (throws if the stream ends first).
            void                GetExact( char *achBuffer, int nLength );

                // Text.
            CStream            &operator << ( const char chPut ) { PutChar( chPut ); return *this; }
            CStream            &operator << ( const unsigned char chPut ) { PutChar( chPut ); return *this; }
            CStream            &operator << ( const short nPut );
            CStream            &operator << ( const unsigned short nPut );
            CStream            &operator << ( const int nPut );
            CStream            &operator << ( const unsigned int nPut );
            CStream            &operator << ( const long nPut );
            CStream            &operator << ( const unsigned long nPut );
            CStream            &operator << ( const long long nPut );
            CStream            &operator << ( const unsigned long long nPut );
            CStream            &operator << ( const double dPut );
            CStream            &operator << ( const CString &strPut );
            CStream            &operator << ( const char *strPut );

            CStream            &operator >> ( char &chGet ) { chGet = GetChar(); return *this; }
            CStream            &operator >> ( unsigned char &chGet ) { chGet = GetUChar(); return *this; }
            CStream            &operator >> ( short &nGet );
            CStream            &operator >> ( unsigned short &nGet );
            CStream            &operator >> ( int &nGet );
            CStream            &operator >> ( unsigned int &nGet );
            CStream            &operator >> ( long &nGet );
            CStream            &operator >> ( unsigned long &nGet );
            CStream            &operator >> ( long long &nGet );
            CStream            &operator >> ( unsigned long long &nGet );
            CStream            &operator >> ( double &dGet );
            CStream            &operator >> ( CString &strGet );

        protected:
                // Skips white space, then the characters of a number.
            CString             ReadNumberText( bool bFloat );
            long long           ReadSigned( long long nMin, long long nMax );
            unsigned long long  ReadUnsigned( unsigned long long nMax );
    };
} // namespace IASLib

#endif // IASLIB_STREAM_H__
