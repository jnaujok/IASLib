/**
 * @file Uuid.h
 * @brief CUUID, a 128-bit universally unique identifier.
 *
 * UUID class
 *
 * This class generates and stores UUIDs (RFC 9562, formerly RFC 4122): 128
 * bits, written as 36 characters, "xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx".
 * New UUIDs are version 4 (random): 122 bits from the operating system's
 * random source, so they are unpredictable and, in practice, unique.
 *
 * Author: Jeffrey R. Naujok
 * Created: August 25, 2019
 *
 * Copyright (c) 2019, Irene Adler Software, All Rights Reserved.
 */

#ifndef IASLIB_UUID_H__
#define IASLIB_UUID_H__

#include "Object.h"
#include "String_.h"

namespace IASLib
{
    /**
     * A UUID (RFC 9562, formerly RFC 4122): 128 bits, written as 36
     * characters, "xxxxxxxx-xxxx-Mxxx-Nxxx-xxxxxxxxxxxx".
     *
     * New UUIDs are version 4 (random): 122 bits from the operating system's
     * random source, so they are unpredictable and, in practice, unique. A
     * UUID made from text that is not a UUID is invalid (isValid() false).
     * A UUID is a value; making new ones is safe on any thread.
     *
     * @code
     * CUUID id;                                   // new random UUID
     * CString strText = id.toString();            // "1b4e28ba-2fa1-4d2a-883f-0016d3cca427"
     * CUUID same( strText );
     * bool bEqual = ( same == id );               // true
     * @endcode
     */
    class CUUID : public CObject
    {
        public:
            static const size_t SIZE = 16;      ///< The size of a UUID in bytes.

        private:
            unsigned char   m_achBytes[ SIZE ]; ///< The bytes, in the order they are written.
            bool            m_IsValid;          ///< Whether this is a UUID.

        public:
            DEFINE_OBJECT( CUUID );

                /// A new random (version 4) UUID.
            CUUID( void );
                /// Parses the 36-character form (or the 32 digits without the
                /// dashes, with or without braces, in either case); an
                /// invalid string gives an invalid UUID.
            CUUID( const char *uuid );
                /// From 16 bytes, in the order they are written.
            CUUID( const unsigned char achBytes[ SIZE ] );
                /// A copy.
            CUUID( const CUUID &oSource );

            virtual ~CUUID( void );

                /// The 16 bytes, in the order they are written (NULL if this
                /// UUID is not valid).
            const unsigned char *getBytes( void ) const;
                /// Lower-case 36-character form ("" if not valid).
            CString         toString( void ) const;

                /// Whether this is a UUID (false for one made from bad text).
            bool            isValid( void ) const { return m_IsValid; }
                /// The version (4 for random UUIDs), or -1 if not valid.
            int             getVersion( void ) const;
                /// Whether this is the all-zero UUID (false if not valid).
            bool            isNil( void ) const;
                /// The all-zero UUID.
            static CUUID    Nil( void );

                /// Parses text, as CUUID( const char * ) does.
            static CUUID    fromString( const char *uuid );

                /// Copies the bytes and validity.
            CUUID          &operator =( const CUUID &oSource );

                /// Compares the bytes (invalid UUIDs are equal to each other
                /// and less than any valid one).
            bool            operator ==( const CUUID &oOther ) const;
                /// Whether the UUIDs differ (see operator ==).
            bool            operator !=( const CUUID &oOther ) const { return ! ( *this == oOther ); }
                /// Byte order (see operator ==).
            bool            operator <( const CUUID &oOther ) const;
                /// A hash of the bytes, for hashed collections.
            virtual int     hashCode( void ) const;

            // Helper methods
                /// The lower-case 36-character form of SIZE bytes ("" for NULL).
            static CString  bytesToString( const unsigned char *bytes );
                /// Parses into SIZE bytes, returning whether the text is a
                /// UUID (bytes is unchanged if not).
            static bool     stringToBytes( const char *string, unsigned char *bytes );

        private:
                /// Fills the bytes with a new version 4 UUID.
            void            generateRandomUUID( void );
    };
}

#endif // IASLIB_UUID_H__
