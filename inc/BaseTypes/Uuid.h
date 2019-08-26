/**
 * UUID class
 * 
 * This class generates and stores UUIDs that meet requirements for uniqueness.
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
        private:
            static bool bSeeded;
            unsigned char bytes[32];
            CString       strUUID;
            bool            m_IsValid;
        public:
            DEFINE_OBJECT( CUUID );

            CUUID( void );
            CUUID( const char *uuid );
            CUUID( const CUUID &oSource );

            virtual ~CUUID( void );

            unsigned char  *getBytes( void );
            CString         toString( void );

            bool            isValid( void ) { return m_IsValid; }

            static CUUID   fromString( const char *uuid );

            CUUID &operator =( const CUUID &oSource );

            // Helper methods
            static CString bytesToString( unsigned char *bytes );
            static bool stringToBytes( const char *string, unsigned char *bytes );

        private:
            void generateRandomUUID( void );
            static unsigned char hexToByte( char val );
    };
}

#endif // IASLIB_UUID_H__