/**
 * HTTP Utilities
 *
 * Author: Jeffrey R. Naujok
 * Created: November 17, 2026
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/HTTP/HttpUtils.h"
#include "BaseTypes/Date.h"

#include <cctype>
#include <cstring>
#include <vector>

namespace IASLib
{
    namespace
    {
        std::string lowerCase( const std::string &value )
        {
            std::string retVal = value;
            for ( size_t nX = 0; nX < retVal.size(); nX++ )
                retVal[ nX ] = (char)tolower( (unsigned char)retVal[ nX ] );
            return retVal;
        }

        std::string trim( const std::string &value )
        {
            size_t nStart = 0;
            size_t nEnd = value.size();
            while ( nStart < nEnd && ( value[ nStart ] == ' ' || value[ nStart ] == '\t' || value[ nStart ] == '\r' ) )
                nStart++;
            while ( nEnd > nStart && ( value[ nEnd - 1 ] == ' ' || value[ nEnd - 1 ] == '\t' || value[ nEnd - 1 ] == '\r' ) )
                nEnd--;
            return value.substr( nStart, nEnd - nStart );
        }

        std::vector<std::string> splitCommas( const std::string &value )
        {
            std::vector<std::string> retVal;
            size_t nStart = 0;
            while ( nStart <= value.size() )
            {
                size_t nComma = value.find( ',', nStart );
                if ( nComma == std::string::npos )
                    nComma = value.size();
                std::string item = trim( value.substr( nStart, nComma - nStart ) );
                if ( ! item.empty() )
                    retVal.push_back( item );
                nStart = nComma + 1;
            }
            return retVal;
        }

        bool parseDecimal( const std::string &value, size_t &result )
        {
            if ( value.empty() || value.size() > 15 )
                return false;
            result = 0;
            for ( size_t nX = 0; nX < value.size(); nX++ )
            {
                if ( ! isdigit( (unsigned char)value[ nX ] ) )
                    return false;
                result = result * 10 + (size_t)( value[ nX ] - '0' );
            }
            return true;
        }

        CHttpFrame failed( int nStatus, bool bHeadersComplete )
        {
            CHttpFrame frame;
            frame.status = CHttpFrame::FAILED;
            frame.errorStatus = nStatus;
            frame.headersComplete = bHeadersComplete;
            return frame;
        }
    }

    CHttpFrame CHttpUtils::frameRequest( const char *pBuffer, size_t nLength, size_t nMaxHeader, size_t nMaxBody )
    {
        CHttpFrame frame;

            // RFC 9112 2.2: ignore empty lines before the request line.
        size_t nStart = 0;
        while ( nStart < nLength && ( pBuffer[ nStart ] == '\r' || pBuffer[ nStart ] == '\n' ) )
            nStart++;

            // Find the end of the header section.
        size_t nHeaderEnd = std::string::npos;   // Position of the first '\n' of the blank line.
        size_t nBodyStart = 0;
        for ( size_t nX = nStart; nX < nLength; nX++ )
        {
            if ( pBuffer[ nX ] != '\n' )
                continue;
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

        if ( nHeaderEnd == std::string::npos )
        {
            if ( nLength - nStart > nMaxHeader )
                return failed( 431, false );
            return frame;   // NEED_MORE
        }

        if ( nBodyStart - nStart > nMaxHeader )
            return failed( 431, true );

        frame.headersComplete = true;

            // Walk the header lines (skipping the request line), collecting the
            // framing headers and keeping the others.
        std::string headerText( pBuffer + nStart, nHeaderEnd + 1 - nStart );
        std::string keptHeaders;
        std::vector<std::string> contentLengths;
        std::vector<std::string> transferCodings;
        bool bHasTransferEncoding = false;

        size_t nLineStart = 0;
        bool bFirstLine = true;
        while ( nLineStart < headerText.size() )
        {
            size_t nLineEnd = headerText.find( '\n', nLineStart );
            if ( nLineEnd == std::string::npos )
                nLineEnd = headerText.size();
            std::string line = headerText.substr( nLineStart, nLineEnd - nLineStart );
            if ( ! line.empty() && line[ line.size() - 1 ] == '\r' )
                line.erase( line.size() - 1 );
            nLineStart = nLineEnd + 1;

            if ( bFirstLine )
            {
                bFirstLine = false;
                keptHeaders += line + "\r\n";
                continue;
            }

            size_t nColon = line.find( ':' );
            std::string name = lowerCase( trim( ( nColon == std::string::npos ) ? line : line.substr( 0, nColon ) ) );
            std::string value = ( nColon == std::string::npos ) ? std::string() : trim( line.substr( nColon + 1 ) );

                // RFC 9112 5.1: no whitespace between the field name and colon.
            if ( nColon != std::string::npos && nColon > 0 && ( line[ nColon - 1 ] == ' ' || line[ nColon - 1 ] == '\t' ) )
                return failed( 400, true );

            if ( name == "content-length" )
            {
                std::vector<std::string> values = splitCommas( value );
                if ( values.empty() )
                    return failed( 400, true );
                contentLengths.insert( contentLengths.end(), values.begin(), values.end() );
                continue;
            }
            if ( name == "transfer-encoding" )
            {
                bHasTransferEncoding = true;
                std::vector<std::string> codings = splitCommas( lowerCase( value ) );
                transferCodings.insert( transferCodings.end(), codings.begin(), codings.end() );
                continue;
            }
            if ( name == "expect" && lowerCase( value ) == "100-continue" )
            {
                frame.expectContinue = true;
            }
            keptHeaders += line + "\r\n";
        }

        size_t nAvailable = nLength - nBodyStart;
        std::string body;
        size_t nConsumed = 0;

        if ( bHasTransferEncoding )
        {
                // RFC 9112 6.1 lets a server reject a message with both; doing
                // so closes a request-smuggling hole.
            if ( ! contentLengths.empty() )
                return failed( 400, true );
            if ( transferCodings.size() != 1 || transferCodings[0] != "chunked" )
                return failed( transferCodings.empty() ? 400 : 501, true );

                // Decode the chunked body (RFC 9112 7.1).
            size_t nPos = nBodyStart;
            while ( true )
            {
                size_t nLineEnd = std::string::npos;
                for ( size_t nX = nPos; nX < nLength; nX++ )
                {
                    if ( pBuffer[ nX ] == '\n' )
                    {
                        nLineEnd = nX;
                        break;
                    }
                }
                if ( nLineEnd == std::string::npos )
                {
                    if ( nLength - nPos > 1024 )
                        return failed( 400, true );
                    return frame;   // NEED_MORE
                }

                std::string sizeLine( pBuffer + nPos, nLineEnd - nPos );
                if ( ! sizeLine.empty() && sizeLine[ sizeLine.size() - 1 ] == '\r' )
                    sizeLine.erase( sizeLine.size() - 1 );
                size_t nSemi = sizeLine.find( ';' );
                std::string hex = trim( ( nSemi == std::string::npos ) ? sizeLine : sizeLine.substr( 0, nSemi ) );
                if ( hex.empty() || hex.size() > 8 )
                    return failed( 400, true );
                size_t nChunk = 0;
                for ( size_t nX = 0; nX < hex.size(); nX++ )
                {
                    if ( ! isxdigit( (unsigned char)hex[ nX ] ) )
                        return failed( 400, true );
                    nChunk = nChunk * 16 + (size_t)( isdigit( (unsigned char)hex[ nX ] ) ? hex[ nX ] - '0' : tolower( (unsigned char)hex[ nX ] ) - 'a' + 10 );
                }
                nPos = nLineEnd + 1;

                if ( nChunk == 0 )
                {
                        // Trailer section: lines up to a blank line. Trailers
                        // are not merged into the headers.
                    while ( true )
                    {
                        size_t nTrailerEnd = std::string::npos;
                        for ( size_t nX = nPos; nX < nLength; nX++ )
                        {
                            if ( pBuffer[ nX ] == '\n' )
                            {
                                nTrailerEnd = nX;
                                break;
                            }
                        }
                        if ( nTrailerEnd == std::string::npos )
                        {
                            if ( nLength - nPos > nMaxHeader )
                                return failed( 431, true );
                            return frame;   // NEED_MORE
                        }
                        bool bBlank = ( nTrailerEnd == nPos ) || ( nTrailerEnd == nPos + 1 && pBuffer[ nPos ] == '\r' );
                        nPos = nTrailerEnd + 1;
                        if ( bBlank )
                            break;
                    }
                    nConsumed = nPos;
                    break;
                }

                if ( body.size() + nChunk > nMaxBody )
                    return failed( 413, true );
                if ( nLength - nPos < nChunk + 2 )
                {
                        // Wait for the chunk and its CRLF (or LF).
                    if ( nLength - nPos == nChunk + 1 && pBuffer[ nPos + nChunk ] == '\n' )
                    {
                        body.append( pBuffer + nPos, nChunk );
                        nPos += nChunk + 1;
                        continue;
                    }
                    return frame;   // NEED_MORE
                }
                body.append( pBuffer + nPos, nChunk );
                nPos += nChunk;
                if ( pBuffer[ nPos ] == '\r' && pBuffer[ nPos + 1 ] == '\n' )
                    nPos += 2;
                else if ( pBuffer[ nPos ] == '\n' )
                    nPos += 1;
                else
                    return failed( 400, true );
            }
        }
        else if ( ! contentLengths.empty() )
        {
            size_t nContentLength = 0;
            for ( size_t nX = 0; nX < contentLengths.size(); nX++ )
            {
                size_t nValue = 0;
                if ( ! parseDecimal( contentLengths[ nX ], nValue ) )
                    return failed( 400, true );
                if ( nX > 0 && nValue != nContentLength )
                    return failed( 400, true );
                nContentLength = nValue;
            }
            if ( nContentLength > nMaxBody )
                return failed( 413, true );
            if ( nAvailable < nContentLength )
                return frame;   // NEED_MORE
            body.assign( pBuffer + nBodyStart, nContentLength );
            nConsumed = nBodyStart + nContentLength;
        }
        else
        {
                // No body (RFC 9112 6.3: a request without either has none).
            nConsumed = nBodyStart;
        }

        frame.status = CHttpFrame::COMPLETE;
        frame.consumed = nConsumed;
        frame.message = keptHeaders;
        frame.message += "Content-Length: " + std::to_string( body.size() ) + "\r\n\r\n";
        frame.message += body;
        return frame;
    }

    const char *CHttpUtils::reasonPhrase( int nStatusCode )
    {
        switch ( nStatusCode )
        {
            case 100: return "Continue";
            case 101: return "Switching Protocols";
            case 200: return "OK";
            case 201: return "Created";
            case 202: return "Accepted";
            case 203: return "Non-Authoritative Information";
            case 204: return "No Content";
            case 205: return "Reset Content";
            case 206: return "Partial Content";
            case 300: return "Multiple Choices";
            case 301: return "Moved Permanently";
            case 302: return "Found";
            case 303: return "See Other";
            case 304: return "Not Modified";
            case 307: return "Temporary Redirect";
            case 308: return "Permanent Redirect";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 405: return "Method Not Allowed";
            case 406: return "Not Acceptable";
            case 408: return "Request Timeout";
            case 409: return "Conflict";
            case 410: return "Gone";
            case 411: return "Length Required";
            case 412: return "Precondition Failed";
            case 413: return "Content Too Large";
            case 414: return "URI Too Long";
            case 415: return "Unsupported Media Type";
            case 416: return "Range Not Satisfiable";
            case 417: return "Expectation Failed";
            case 421: return "Misdirected Request";
            case 422: return "Unprocessable Content";
            case 426: return "Upgrade Required";
            case 429: return "Too Many Requests";
            case 431: return "Request Header Fields Too Large";
            case 500: return "Internal Server Error";
            case 501: return "Not Implemented";
            case 502: return "Bad Gateway";
            case 503: return "Service Unavailable";
            case 504: return "Gateway Timeout";
            case 505: return "HTTP Version Not Supported";
            default: break;
        }
        if ( nStatusCode < 200 ) return "Informational";
        if ( nStatusCode < 300 ) return "Success";
        if ( nStatusCode < 400 ) return "Redirection";
        if ( nStatusCode < 500 ) return "Client Error";
        return "Server Error";
    }

    CString CHttpUtils::httpDate( void )
    {
            // CDate takes the current time in GMT, and DF_RFC7231 is the
            // IMF-fixdate format RFC 9110 requires.
        CDate now;
        return now.FormatDate( CDate::DF_RFC7231 );
    }

    bool CHttpUtils::isKnownMethod( const CString &method )
    {
        static const char *known[] = { "GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH", NULL };
        for ( int nX = 0; known[ nX ]; nX++ )
        {
            if ( strcmp( (const char *)method, known[ nX ] ) == 0 )
                return true;
        }
        return false;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
