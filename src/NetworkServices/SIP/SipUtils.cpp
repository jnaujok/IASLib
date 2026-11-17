/**
 * SIP Utilities
 *
 * Helper functions for working with SIP header values (RFC 3261).
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipUtils.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <random>
#ifndef IASLIB_WIN32__
#include <strings.h>
#else
#define strncasecmp _strnicmp
#endif

namespace IASLib
{
    const char *CSipUtils::MAGIC_COOKIE = "z9hG4bK";

    namespace
    {
            // Finds the position where the parameters of a header value start:
            // the first ';' outside of quotes and angle brackets.
        size_t findParameters( const CString &value )
        {
            bool bInQuote = false;
            int nAngle = 0;
            for ( size_t nX = 0; nX < value.GetLength(); nX++ )
            {
                char ch = value[ nX ];
                if ( bInQuote )
                {
                    if ( ch == '\\' && ( nX + 1 ) < value.GetLength() )
                        nX++;
                    else if ( ch == '"' )
                        bInQuote = false;
                    continue;
                }
                if ( ch == '"' )
                    bInQuote = true;
                else if ( ch == '<' )
                    nAngle++;
                else if ( ch == '>' && nAngle > 0 )
                    nAngle--;
                else if ( ch == ';' && nAngle == 0 )
                    return nX;
            }
            return NOT_FOUND;
        }

            // Splits the parameter section (after the first ';') into
            // "name[=value]" items.
        CStringArray splitParameters( const CString &params )
        {
            CStringArray retVal;
            CString current;
            bool bInQuote = false;
            for ( size_t nX = 0; nX < params.GetLength(); nX++ )
            {
                char ch = params[ nX ];
                if ( ch == '"' )
                    bInQuote = ! bInQuote;
                if ( ch == ';' && ! bInQuote )
                {
                    current.Trim();
                    if ( current.GetLength() > 0 )
                        retVal.Append( current );
                    current = "";
                }
                else
                {
                    current += ch;
                }
            }
            current.Trim();
            if ( current.GetLength() > 0 )
                retVal.Append( current );
            return retVal;
        }

        CString lowerCase( const CString &value )
        {
            CString retVal = value;
            retVal.ToLowerCase();
            return retVal;
        }
    }

    CStringArray CSipUtils::splitList( const CString &value )
    {
        CStringArray retVal;
        CString current;
        bool bInQuote = false;
        int nAngle = 0;

        for ( size_t nX = 0; nX < value.GetLength(); nX++ )
        {
            char ch = value[ nX ];
            if ( bInQuote )
            {
                current += ch;
                if ( ch == '\\' && ( nX + 1 ) < value.GetLength() )
                {
                    nX++;
                    current += value[ nX ];
                }
                else if ( ch == '"' )
                {
                    bInQuote = false;
                }
                continue;
            }
            if ( ch == '"' )
                bInQuote = true;
            else if ( ch == '<' )
                nAngle++;
            else if ( ch == '>' && nAngle > 0 )
                nAngle--;

            if ( ch == ',' && nAngle == 0 )
            {
                current.Trim();
                if ( current.GetLength() > 0 )
                    retVal.Append( current );
                current = "";
            }
            else
            {
                current += ch;
            }
        }
        current.Trim();
        if ( current.GetLength() > 0 )
            retVal.Append( current );
        return retVal;
    }

    CString CSipUtils::getParameter( const CString &headerValue, const char *name, bool *pFound )
    {
        if ( pFound )
            *pFound = false;

        size_t nStart = findParameters( headerValue );
        if ( nStart == NOT_FOUND )
            return CString( "" );

        CString target = lowerCase( CString( name ) );
        CStringArray params = splitParameters( headerValue.Substring( nStart + 1 ) );

        for ( size_t nX = 0; nX < params.Count(); nX++ )
        {
            CString param = params[ nX ];
            size_t nEquals = param.IndexOf( '=' );
            CString paramName = ( nEquals == NOT_FOUND ) ? param : param.Substring( 0, (int)nEquals );
            paramName.Trim();
            if ( lowerCase( paramName ) == target )
            {
                if ( pFound )
                    *pFound = true;
                if ( nEquals == NOT_FOUND )
                    return CString( "" );
                CString paramValue = param.Substring( nEquals + 1 );
                paramValue.Trim();
                return paramValue;
            }
        }
        return CString( "" );
    }

    CString CSipUtils::setParameter( const CString &headerValue, const char *name, const char *value )
    {
        CString newParam( name );
        if ( value )
        {
            newParam += "=";
            newParam += value;
        }

        size_t nStart = findParameters( headerValue );
        if ( nStart == NOT_FOUND )
        {
            CString retVal = headerValue;
            retVal.Trim();
            retVal += ";";
            retVal += newParam;
            return retVal;
        }

        CString target = lowerCase( CString( name ) );
        CString retVal = headerValue.Substring( 0, (int)nStart );
        retVal.Trim();
        CStringArray params = splitParameters( headerValue.Substring( nStart + 1 ) );
        bool bReplaced = false;

        for ( size_t nX = 0; nX < params.Count(); nX++ )
        {
            CString param = params[ nX ];
            size_t nEquals = param.IndexOf( '=' );
            CString paramName = ( nEquals == NOT_FOUND ) ? param : param.Substring( 0, (int)nEquals );
            paramName.Trim();
            retVal += ";";
            if ( lowerCase( paramName ) == target )
            {
                retVal += newParam;
                bReplaced = true;
            }
            else
            {
                retVal += param;
            }
        }

        if ( ! bReplaced )
        {
            retVal += ";";
            retVal += newParam;
        }
        return retVal;
    }

    bool CSipUtils::parseVia( const CString &via, CString &transport, CString &host, int &port )
    {
        transport = "";
        host = "";
        port = 0;

            // via-parm = sent-protocol LWS sent-by *( SEMI via-params )
            // sent-protocol = "SIP" SLASH "2.0" SLASH transport
        CString work = via;
        size_t nParams = findParameters( work );
        if ( nParams != NOT_FOUND )
            work = work.Substring( 0, (int)nParams );
        work.Trim();

        size_t nSpace = NOT_FOUND;
        for ( size_t nX = 0; nX < work.GetLength(); nX++ )
        {
            if ( work[ nX ] == ' ' || work[ nX ] == '\t' )
            {
                nSpace = nX;
                break;
            }
        }
        if ( nSpace == NOT_FOUND )
            return false;

        CString protocol = work.Substring( 0, (int)nSpace );
        CString sentBy = work.Substring( nSpace + 1 );
        sentBy.Trim();

        size_t nSlash = protocol.LastIndexOf( '/' );
        if ( nSlash == NOT_FOUND )
            return false;
        transport = protocol.Substring( nSlash + 1 );
        transport.Trim();
        transport.ToUpperCase();

        if ( sentBy.GetLength() == 0 )
            return false;

        if ( sentBy[0] == '[' )
        {
                // IPv6 reference: [addr]:port
            size_t nClose = sentBy.IndexOf( ']' );
            if ( nClose == NOT_FOUND )
                return false;
            host = sentBy.Substring( 1, (int)nClose - 1 );
            CString rest = sentBy.Substring( nClose + 1 );
            rest.Trim();
            if ( rest.GetLength() > 1 && rest[0] == ':' )
                port = atoi( (const char *)rest.Substring( 1 ) );
        }
        else
        {
            size_t nColon = sentBy.IndexOf( ':' );
            if ( nColon != NOT_FOUND )
            {
                host = sentBy.Substring( 0, (int)nColon );
                port = atoi( (const char *)sentBy.Substring( nColon + 1 ) );
            }
            else
            {
                host = sentBy;
            }
        }
        host.Trim();
        return ( host.GetLength() > 0 ) && ( port >= 0 ) && ( port <= 65535 );
    }

    bool CSipUtils::parseCSeq( const CString &cseq, unsigned long &number, CString &method )
    {
        CString work = cseq;
        work.Trim();
        number = 0;
        method = "";

        size_t nX = 0;
        while ( nX < work.GetLength() && isdigit( (unsigned char)work[ nX ] ) )
            nX++;

            // CSeq numbers are at most 2**31 - 1 (RFC 3261 8.1.1.5).
        if ( nX == 0 || nX > 10 )
            return false;

        CString digits = work.Substring( 0, (int)nX );
        unsigned long long value = strtoull( (const char *)digits, NULL, 10 );
        if ( value > 2147483647ULL )
            return false;
        number = (unsigned long)value;

        method = work.Substring( nX );
        method.Trim();
        if ( method.GetLength() == 0 )
            return false;
        for ( size_t nY = 0; nY < method.GetLength(); nY++ )
        {
            if ( method[ nY ] == ' ' || method[ nY ] == '\t' )
                return false;
        }
        return true;
    }

    CString CSipUtils::uriScheme( const CString &uri )
    {
        CString work = uri;
        work.Trim();
        if ( work.GetLength() > 0 && work[0] == '<' )
            work = work.Substring( 1 );
        size_t nColon = work.IndexOf( ':' );
        if ( nColon == NOT_FOUND )
            return CString( "" );
        CString scheme = work.Substring( 0, (int)nColon );
        scheme.Trim();
        scheme.ToLowerCase();
        return scheme;
    }

    CString CSipUtils::randomToken( size_t nBytes )
    {
        static std::mutex generatorMutex;
        static std::mt19937_64 generator( std::random_device{}() ^ ( (unsigned long long)std::random_device{}() << 32 ) );
        static const char *hex = "0123456789abcdef";

        CString retVal;
        std::lock_guard<std::mutex> lock( generatorMutex );
        for ( size_t nX = 0; nX < nBytes; nX++ )
        {
            unsigned int byte = (unsigned int)( generator() & 0xFF );
            retVal += hex[ byte >> 4 ];
            retVal += hex[ byte & 0x0F ];
        }
        return retVal;
    }

    const char *CSipUtils::reasonPhrase( int nStatusCode )
    {
        switch ( nStatusCode )
        {
            case 100: return "Trying";
            case 180: return "Ringing";
            case 181: return "Call Is Being Forwarded";
            case 182: return "Queued";
            case 183: return "Session Progress";
            case 200: return "OK";
            case 202: return "Accepted";
            case 300: return "Multiple Choices";
            case 301: return "Moved Permanently";
            case 302: return "Moved Temporarily";
            case 305: return "Use Proxy";
            case 380: return "Alternative Service";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 402: return "Payment Required";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 406: return "Not Acceptable";
            case 407: return "Proxy Authentication Required";
            case 408: return "Request Timeout";
            case 410: return "Gone";
            case 413: return "Request Entity Too Large";
            case 414: return "Request-URI Too Long";
            case 415: return "Unsupported Media Type";
            case 416: return "Unsupported URI Scheme";
            case 420: return "Bad Extension";
            case 421: return "Extension Required";
            case 423: return "Interval Too Brief";
            case 480: return "Temporarily Unavailable";
            case 481: return "Call/Transaction Does Not Exist";
            case 482: return "Loop Detected";
            case 483: return "Too Many Hops";
            case 484: return "Address Incomplete";
            case 485: return "Ambiguous";
            case 486: return "Busy Here";
            case 487: return "Request Terminated";
            case 488: return "Not Acceptable Here";
            case 491: return "Request Pending";
            case 493: return "Undecipherable";
            case 500: return "Server Internal Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Server Time-out";
            case 505: return "Version Not Supported";
            case 513: return "Message Too Large";
            case 600: return "Busy Everywhere";
            case 603: return "Decline";
            case 604: return "Does Not Exist Anywhere";
            case 606: return "Not Acceptable";
            default: break;
        }
        if ( nStatusCode >= 100 && nStatusCode < 200 ) return "Session Progress";
        if ( nStatusCode >= 200 && nStatusCode < 300 ) return "OK";
        if ( nStatusCode >= 300 && nStatusCode < 400 ) return "Redirection";
        if ( nStatusCode >= 400 && nStatusCode < 500 ) return "Request Failure";
        if ( nStatusCode >= 500 && nStatusCode < 600 ) return "Server Failure";
        return "Global Failure";
    }

    bool CSipUtils::isKnownMethod( const CString &method )
    {
        static const char *known[] = { "INVITE", "ACK", "BYE", "CANCEL", "OPTIONS", "REGISTER",
                                        "PRACK", "SUBSCRIBE", "NOTIFY", "PUBLISH", "INFO", "REFER",
                                        "MESSAGE", "UPDATE", NULL };
        for ( int nX = 0; known[ nX ]; nX++ )
        {
            if ( strcmp( (const char *)method, known[ nX ] ) == 0 )
                return true;
        }
        return false;
    }

    long CSipUtils::frameMessage( const char *pBuffer, size_t nLength, size_t &nLeadingCrlf, size_t nMaxMessage )
    {
        nLeadingCrlf = 0;
        while ( nLeadingCrlf < nLength && ( pBuffer[ nLeadingCrlf ] == '\r' || pBuffer[ nLeadingCrlf ] == '\n' ) )
            nLeadingCrlf++;

        size_t nStart = nLeadingCrlf;

            // Find the blank line that ends the headers.
        size_t nHeaderEnd = 0;
        size_t nBodyStart = 0;
        for ( size_t nX = nStart; nX < nLength; nX++ )
        {
            if ( pBuffer[ nX ] == '\n' )
            {
                if ( nX + 1 < nLength && pBuffer[ nX + 1 ] == '\n' )
                {
                    nHeaderEnd = nX;
                    nBodyStart = nX + 2;
                    break;
                }
                if ( nX + 2 < nLength && pBuffer[ nX + 1 ] == '\r' && pBuffer[ nX + 2 ] == '\n' )
                {
                    nHeaderEnd = nX;
                    nBodyStart = nX + 3;
                    break;
                }
            }
        }

        if ( nBodyStart == 0 )
        {
            return ( ( nLength - nStart ) > nMaxMessage ) ? -1 : 0;
        }

            // Look for Content-Length (or its compact form "l") in the headers.
        long nContentLength = 0;
        size_t nLineStart = nStart;
        while ( nLineStart < nHeaderEnd )
        {
            size_t nLineEnd = nLineStart;
            while ( nLineEnd < nHeaderEnd && pBuffer[ nLineEnd ] != '\n' )
                nLineEnd++;

            size_t nColon = nLineStart;
            while ( nColon < nLineEnd && pBuffer[ nColon ] != ':' )
                nColon++;

            if ( nColon < nLineEnd )
            {
                size_t nNameEnd = nColon;
                while ( nNameEnd > nLineStart && ( pBuffer[ nNameEnd - 1 ] == ' ' || pBuffer[ nNameEnd - 1 ] == '\t' ) )
                    nNameEnd--;
                size_t nNameLength = nNameEnd - nLineStart;

                bool bIsLength = ( nNameLength == 14 && strncasecmp( pBuffer + nLineStart, "content-length", 14 ) == 0 ) ||
                                 ( nNameLength == 1 && ( pBuffer[ nLineStart ] == 'l' || pBuffer[ nLineStart ] == 'L' ) );
                if ( bIsLength )
                {
                    size_t nValue = nColon + 1;
                    while ( nValue < nLineEnd && ( pBuffer[ nValue ] == ' ' || pBuffer[ nValue ] == '\t' ) )
                        nValue++;
                    size_t nDigits = 0;
                    long nParsed = 0;
                    while ( nValue < nLineEnd && isdigit( (unsigned char)pBuffer[ nValue ] ) )
                    {
                        nParsed = nParsed * 10 + ( pBuffer[ nValue ] - '0' );
                        nValue++;
                        nDigits++;
                        if ( nDigits > 9 )
                            return -1;
                    }
                    if ( nDigits == 0 )
                        return -1;
                    nContentLength = nParsed;
                }
            }
            nLineStart = nLineEnd + 1;
        }

        size_t nTotal = nBodyStart + (size_t)nContentLength;
        if ( ( nTotal - nStart ) > nMaxMessage )
            return -1;
        if ( nTotal > nLength )
            return 0;
        return (long)nTotal;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
