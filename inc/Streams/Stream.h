/**
 * @file
 * @brief Stream Abstract Class.
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
 * line ("" at the end). GetLine() leaves off the `"\n"` and a `"\r"` before it.
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
    /// Abstract base of all byte streams.
    ///
    /// A derived stream implements GetBuffer(), PutBuffer(), IsEOS(),
    /// PeekChar() and Close(). Everything else (characters, lines,
    /// big-endian binary integers, and text with << and >>) is built on
    /// those and may be overridden for speed. Streams are not thread-safe.
    ///
    /// @code
    /// CStringStream oOut;
    /// oOut << "x = " << 42 << '\n';
    /// oOut.PutUInt32( 0xCAFEBABE );
    ///
    /// CStringStream oIn( CString( "width 640" ) );
    /// CString strWord;
    /// int nWidth;
    /// oIn >> strWord >> nWidth;     // "width", 640
    /// @endcode
    class CStream : public CObject
    {
        protected:
            bool                m_bIsOpen;      ///< True until the stream is closed.

        public:
                                /// Makes an open stream.
                                CStream( void ) { m_bIsOpen = true; }
            /// Destructor. It does not close the stream; derived classes do.
            virtual            ~CStream( void ) {}

                                DEFINE_OBJECT( CStream );

            /// True while the stream is open.
            virtual bool        IsOpen( void ) { return m_bIsOpen; }

            /// @name What every stream provides
            ///@{

            /// Reads up to nLength bytes, returning how many (0 at the end).
            /// @param achBuffer Receives the bytes.
            /// @param nLength Most bytes to read.
            /// @return Bytes read; 0 at the end of the stream.
            virtual int         GetBuffer( char *achBuffer, int nLength ) = 0;
            /// Writes nLength bytes, returning how many were written.
            /// @param achBuffer The bytes.
            /// @param nLength Number of bytes.
            /// @return Bytes written.
            virtual int         PutBuffer( const char *achBuffer, int nLength ) = 0;
            /// Whether nothing more can be read.
            virtual bool        IsEOS( void ) = 0;
            /// The next character, without taking it.
            /// @throws CStreamException at the end of the stream.
            virtual char        PeekChar( void ) = 0;
            /// Closes the stream.
            virtual void        Close( void ) = 0;
            ///@}

            /// @name Defaults built on those
            ///@{

            /// Reads one character.
            /// @throws CStreamException at the end of the stream.
            virtual char        GetChar( void );
            /// Reads one character as unsigned.
            /// @throws CStreamException at the end of the stream.
            virtual unsigned char GetUChar( void );
            /// Writes one character.
            virtual void        PutChar( const char chPut );
            /// Writes one unsigned character.
            virtual void        PutChar( const unsigned char chPut );
            /// Reads up to the next newline or the end of the stream.
            /// The newline and a carriage return before it are left off, and
            /// NUL bytes are dropped.
            /// @return The line; "" at the end of the stream.
            virtual CString     GetLine( void );
            /// Writes the text and `"\n"`.
            virtual void        PutLine( const CString &strOutput );
            /// How many bytes are known to be left (NOT_FOUND if the
            /// stream cannot tell).
            virtual size_t      bytesRemaining( void ) { return NOT_FOUND; }
            /// Sends anything a stream has held back. The default does nothing.
            virtual void        Flush( void ) {}
            ///@}

            /// @name Binary, big-endian
            /// The Get functions throw CStreamException if the stream ends
            /// before the whole value is read.
            ///@{
            void                PutUInt8( uint8_t nValue );     ///< Writes one byte.
            void                PutUInt16( uint16_t nValue );   ///< Writes 2 bytes, big-endian.
            void                PutUInt32( uint32_t nValue );   ///< Writes 4 bytes, big-endian.
            void                PutUInt64( uint64_t nValue );   ///< Writes 8 bytes, big-endian.
            uint8_t             GetUInt8( void );               ///< Reads one byte.
            uint16_t            GetUInt16( void );              ///< Reads 2 bytes, big-endian.
            uint32_t            GetUInt32( void );              ///< Reads 4 bytes, big-endian.
            uint64_t            GetUInt64( void );              ///< Reads 8 bytes, big-endian.
            /// Reads exactly nLength bytes (throws if the stream ends first).
            /// @throws CStreamException if the stream ends first.
            void                GetExact( char *achBuffer, int nLength );
            ///@}

            /// @name Text output
            /// Each writes its value as text (no separators) and returns *this.
            ///@{
            CStream            &operator << ( const char chPut ) { PutChar( chPut ); return *this; }            ///< Writes the character.
            CStream            &operator << ( const unsigned char chPut ) { PutChar( chPut ); return *this; }   ///< Writes the character.
            CStream            &operator << ( const short nPut );               ///< Writes a decimal number.
            /// Writes a decimal number. (It used to print "%ud": "5d".)
            CStream            &operator << ( const unsigned short nPut );
            CStream            &operator << ( const int nPut );                 ///< Writes a decimal number.
            CStream            &operator << ( const unsigned int nPut );        ///< Writes a decimal number.
            CStream            &operator << ( const long nPut );                ///< Writes a decimal number.
            CStream            &operator << ( const unsigned long nPut );       ///< Writes a decimal number.
            CStream            &operator << ( const long long nPut );           ///< Writes a decimal number.
            CStream            &operator << ( const unsigned long long nPut );  ///< Writes a decimal number.
            CStream            &operator << ( const double dPut );              ///< Writes the number as CString formats it.
            CStream            &operator << ( const CString &strPut );          ///< Writes the string's bytes.
            CStream            &operator << ( const char *strPut );             ///< Writes the C string; NULL writes nothing.
            ///@}

            /// @name Text input
            /// Number reads skip white space, then read a decimal number.
            /// They throw CStreamException if there is no number, it is
            /// malformed, or it does not fit the type (a '-' for an unsigned
            /// type counts as malformed). Doubles are read in the C locale.
            ///@{
            CStream            &operator >> ( char &chGet ) { chGet = GetChar(); return *this; }            ///< Reads one character (no skipping).
            CStream            &operator >> ( unsigned char &chGet ) { chGet = GetUChar(); return *this; }  ///< Reads one character (no skipping).
            CStream            &operator >> ( short &nGet );                ///< Reads a number.
            CStream            &operator >> ( unsigned short &nGet );       ///< Reads a number.
            CStream            &operator >> ( int &nGet );                  ///< Reads a number.
            CStream            &operator >> ( unsigned int &nGet );         ///< Reads a number.
            CStream            &operator >> ( long &nGet );                 ///< Reads a number.
            CStream            &operator >> ( unsigned long &nGet );        ///< Reads a number.
            CStream            &operator >> ( long long &nGet );            ///< Reads a number.
            CStream            &operator >> ( unsigned long long &nGet );   ///< Reads a number.
            CStream            &operator >> ( double &dGet );               ///< Reads a floating-point number.
            /// Skips white space, then reads one word (up to the next white
            /// space). At the end of the stream the result is "".
            CStream            &operator >> ( CString &strGet );
            ///@}

        protected:
            /// Skips white space, then the characters of a number.
            /// @param bFloat Also accept '.', an exponent and its sign.
            /// @return The number's text.
            /// @throws CStreamException if no number characters follow.
            CString             ReadNumberText( bool bFloat );
            /// Reads a signed decimal number in [nMin, nMax].
            /// @throws CStreamException if it is malformed or out of range.
            long long           ReadSigned( long long nMin, long long nMax );
            /// Reads an unsigned decimal number no larger than nMax.
            /// @throws CStreamException if it is malformed, negative or out of range.
            unsigned long long  ReadUnsigned( unsigned long long nMax );
    };
} // namespace IASLib

#endif // IASLIB_STREAM_H__
