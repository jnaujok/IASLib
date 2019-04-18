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

#include "Stream.h"
#include <string.h>

namespace IASLib
{
    IMPLEMENT_OBJECT( CStream, CObject );

    CStream &CStream::operator << ( const short shPut)
    {
        CString strValue;

        strValue.Format( "%d", shPut );
        int nCount = 0;
        while ( nCount < (int)strValue.GetLength() )
        {
            PutChar( strValue[ nCount ] );
            nCount++;
        }

        return *this;
    }

    CStream &CStream::operator << ( const long lPut)
    {
        CString strValue;

        strValue.Format( "%ld", lPut );
        int nCount = 0;
        while ( nCount < (int)strValue.GetLength() )
        {
            PutChar( strValue[ nCount ] );
            nCount++;
        }

        return *this;
    }

    CStream &CStream::operator << ( const unsigned short shPut)
    {
        CString strValue;

        strValue.Format( "%ud", shPut );
        int nCount = 0;
        while ( nCount < (int)strValue.GetLength() )
        {
            PutChar( strValue[ nCount ] );
            nCount++;
        }

        return *this;
    }

    CStream &CStream::operator << ( const unsigned long lPut)
    {
        CString strValue;

        strValue.Format( "%uld", lPut );
        int nCount = 0;
        while ( nCount < (int)strValue.GetLength() )
        {
            PutChar( strValue[ nCount ] );
            nCount++;
        }

        return *this;
    }

    CStream &CStream::operator << ( const CString &strPut)
    {
        PutBuffer( (const char *)strPut, (int)strPut.GetLength() );
        return *this;
    }

    CStream &CStream::operator << ( const char *strPut )
    {
        PutBuffer( strPut, (int)strlen( strPut ) );
        return *this;
    }

    CStream &CStream::operator >> ( short &shGet)
    {
        char chHigh;
        char chLow;

        chHigh = GetChar();
        chLow = GetChar();

        shGet = (short)chHigh * 256;
        shGet = shGet + (short)chLow;

        return *this;
    }

    CStream &CStream::operator >> ( long &lGet)
    {
        char chHigh1;
        char chHigh2;
        char chLow1;
        char chLow2;

        chHigh1 = GetChar();
        chHigh2 = GetChar();
        chLow1 = GetChar();
        chLow2 = GetChar();

        lGet = (long)chHigh1 * (256 *256 *256);
        lGet += (long)chHigh2 * 65536;
        lGet += (long)chLow1 * 256;
        lGet += (long)chLow2;

        return *this;
    }

    CStream &CStream::operator >> ( unsigned short &shGet)
    {
        unsigned char chHigh;
        unsigned char chLow;

        chHigh = GetUChar();
        chLow = GetUChar();

        shGet = (unsigned short)chHigh * 256;
        shGet = shGet + (unsigned short)chLow;

        return *this;
    }

    CStream &CStream::operator >> ( unsigned long &lGet)
    {
        unsigned char chHigh1;
        unsigned char chHigh2;
        unsigned char chLow1;
        unsigned char chLow2;

        chHigh1 = GetUChar();
        chHigh2 = GetUChar();
        chLow1 = GetUChar();
        chLow2 = GetUChar();

        lGet = (unsigned long)chHigh1 * (256 *256 *256);
        lGet += (unsigned long)chHigh2 * 65536;
        lGet += (unsigned long)chLow1 * 256;
        lGet += (unsigned long)chLow2;

        return *this;
    }

    CStream &CStream::operator >> ( CString &strGet)
    {
        long    lLength;
        long    lCount;
        char   *pBuffer;

        (*this) >> lLength;

        pBuffer = new char[lLength + 1];
        pBuffer[ lLength ] = 0;

        for ( lCount = 0; lCount < lLength ; lCount++ )
        {
            pBuffer[ lCount ] = GetChar();
        }

        strGet = pBuffer;
        delete pBuffer;

        return *this;
    }
} // namespace IASLib

