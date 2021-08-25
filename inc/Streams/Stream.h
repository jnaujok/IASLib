/*
 * Stream Abstract Class
 *
 *      This abstract base class provides a consistant stream interface
 * for all of the different input/output objects. Classes that are derived
 * from this class must override the various methods that are required to
 * support streaming. Objects that need to be streamed must also support
 * friend functions to stream themself to a generic stream object.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 08/17/1997
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_STREAM_H__
#define IASLIB_STREAM_H__

#include "../BaseTypes/IASLib_String.h"

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
            virtual CString     GetLine( void ) = 0;
            virtual char        GetChar( void ) = 0;
            virtual void        PutChar( const char chPut ) = 0;
            virtual void        PutLine( const CString &strOutput ) = 0;
            virtual unsigned char        GetUChar( void ) = 0;
            virtual void        PutChar( const unsigned char chPut ) = 0;
            virtual int         PutBuffer( const char *achBuffer, int nLength ) = 0;
            virtual int         GetBuffer( char *achBuffer, int nLength ) = 0;

            virtual char        PeekChar( void ) = 0;

            virtual size_t      bytesRemaining( void ) = 0;
            virtual bool        IsEOS( void ) = 0;

            virtual void        Close( void ) = 0;

            CStream            &operator << ( const char chPut ) { PutChar( chPut ); return *this; }
            CStream            &operator << ( const short shPut);
            CStream            &operator << ( const long lPut);
            CStream            &operator << ( const unsigned char chPut ) { PutChar( chPut ); return *this; }
            CStream            &operator << ( const unsigned short shPut);
            CStream            &operator << ( const unsigned long lPut);
            CStream            &operator << ( const CString &strPut);
            CStream            &operator << ( const char *strPut );

            CStream            &operator >> ( char &chGet ) { chGet = GetChar( ); return *this; }
            CStream            &operator >> ( short &shGet);
            CStream            &operator >> ( long &lGet);
            CStream            &operator >> ( unsigned char &chGet ) { chGet = GetUChar( ); return *this; }
            CStream            &operator >> ( unsigned short &shGet);
            CStream            &operator >> ( unsigned long &lGet);
            CStream            &operator >> ( CString &strGet);
    };
} // namespace IASLib

#endif // __STREAM_H__
