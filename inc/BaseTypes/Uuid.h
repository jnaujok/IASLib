/**
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
    class CUUID : public CObject
    {
        public:
            static const size_t SIZE = 16;      // bytes

        private:
            unsigned char   m_achBytes[ SIZE ];
            bool            m_IsValid;

        public:
            DEFINE_OBJECT( CUUID );

                // A new random (version 4) UUID.
            CUUID( void );
                // Parses the 36-character form (or the 32 digits without the
                // dashes, with or without braces, in either case); an
                // invalid string gives an invalid UUID.
            CUUID( const char *uuid );
                // From 16 bytes, in the order they are written.
            CUUID( const unsigned char achBytes[ SIZE ] );
            CUUID( const CUUID &oSource );

            virtual ~CUUID( void );

                // The 16 bytes, in the order they are written (NULL if this
                // UUID is not valid).
            const unsigned char *getBytes( void ) const;
                // Lower-case 36-character form ("" if not valid).
            CString         toString( void ) const;

            bool            isValid( void ) const { return m_IsValid; }
                // The version (4 for random UUIDs), or -1 if not valid.
            int             getVersion( void ) const;
                // The all-zero UUID.
            bool            isNil( void ) const;
            static CUUID    Nil( void );

            static CUUID    fromString( const char *uuid );

            CUUID          &operator =( const CUUID &oSource );

                // Compares the bytes (invalid UUIDs are equal to each other
                // and less than any valid one).
            bool            operator ==( const CUUID &oOther ) const;
            bool            operator !=( const CUUID &oOther ) const { return ! ( *this == oOther ); }
            bool            operator <( const CUUID &oOther ) const;
            virtual int     hashCode( void ) const;

            // Helper methods
            static CString  bytesToString( const unsigned char *bytes );
                // Parses into SIZE bytes, returning whether the text is a
                // UUID (bytes is unchanged if not).
            static bool     stringToBytes( const char *string, unsigned char *bytes );

        private:
            void            generateRandomUUID( void );
    };
}

#endif // IASLIB_UUID_H__
