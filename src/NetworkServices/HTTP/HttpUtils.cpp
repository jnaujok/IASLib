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

            // Finds the blank line that ends the header section, from nStart.
            // Sets nHeaderEnd to the first '\n' of it and nBodyStart to the
            // byte after it; false if it is not in the buffer yet.
        bool findHeaderEnd( const char *pBuffer, size_t nLength, size_t nStart, size_t &nHeaderEnd, size_t &nBodyStart )
        {
            for ( size_t nX = nStart; nX < nLength; nX++ )
            {
                if ( pBuffer[ nX ] != '\n' )
                    continue;
                if ( nX + 1 < nLength && pBuffer[ nX + 1 ] == '\n' )
                {
                    nHeaderEnd = nX;
                    nBodyStart = nX + 2;
                    return true;
                }
                if ( nX + 2 < nLength && pBuffer[ nX + 1 ] == '\r' && pBuffer[ nX + 2 ] == '\n' )
                {
                    nHeaderEnd = nX;
                    nBodyStart = nX + 3;
                    return true;
                }
            }
            return false;
        }

            // A header section may not hold NUL, or a CR that does not end a
            // line: software that splits lines on a lone CR would see a header
            // boundary where this parser sees none (RFC 9112 2.2).
        bool hasForbiddenBytes( const char *pBuffer, size_t nStart, size_t nEnd )
        {
            for ( size_t nX = nStart; nX < nEnd; nX++ )
            {
                if ( pBuffer[ nX ] == '\0' )
                    return true;
                if ( pBuffer[ nX ] == '\r' && ( nX + 1 >= nEnd || pBuffer[ nX + 1 ] != '\n' ) )
                    return true;
            }
            return false;
        }

            // The framing headers of a message, and the other lines kept.
        struct CHeaderScan
        {
            std::string                 firstLine;
            std::string                 keptHeaders;        // "line\r\n" each
            std::vector<std::string>    contentLengths;
            std::vector<std::string>    transferCodings;
            bool                        hasTransferEncoding;
            bool                        expectContinue;
            bool                        connectionClose;
            bool                        connectionKeepAlive;

            CHeaderScan( void ) : hasTransferEncoding( false ), expectContinue( false ), connectionClose( false ), connectionKeepAlive( false ) {}
        };

            // Splits the header section (up to and including nHeaderEnd) into
            // its start line and fields. False for whitespace before a colon
            // (RFC 9112 5.1) or an empty Content-Length.
        bool scanHeaders( const char *pBuffer, size_t nStart, size_t nHeaderEnd, CHeaderScan &scan )
        {
            std::string headerText( pBuffer + nStart, nHeaderEnd + 1 - nStart );
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
                    scan.firstLine = line;
                    continue;
                }

                size_t nColon = line.find( ':' );
                std::string name = lowerCase( trim( ( nColon == std::string::npos ) ? line : line.substr( 0, nColon ) ) );
                std::string value = ( nColon == std::string::npos ) ? std::string() : trim( line.substr( nColon + 1 ) );

                if ( nColon != std::string::npos && nColon > 0 && ( line[ nColon - 1 ] == ' ' || line[ nColon - 1 ] == '\t' ) )
                    return false;

                if ( name == "content-length" )
                {
                    std::vector<std::string> values = splitCommas( value );
                    if ( values.empty() )
                        return false;
                    scan.contentLengths.insert( scan.contentLengths.end(), values.begin(), values.end() );
                    continue;
                }
                if ( name == "transfer-encoding" )
                {
                    scan.hasTransferEncoding = true;
                    std::vector<std::string> codings = splitCommas( lowerCase( value ) );
                    scan.transferCodings.insert( scan.transferCodings.end(), codings.begin(), codings.end() );
                    continue;
                }
                if ( name == "expect" && lowerCase( value ) == "100-continue" )
                {
                    scan.expectContinue = true;
                }
                if ( name == "connection" )
                {
                    std::vector<std::string> tokens = splitCommas( lowerCase( value ) );
                    for ( size_t nX = 0; nX < tokens.size(); nX++ )
                    {
                        if ( tokens[ nX ] == "close" )
                            scan.connectionClose = true;
                        else if ( tokens[ nX ] == "keep-alive" )
                            scan.connectionKeepAlive = true;
                    }
                }
                scan.keptHeaders += line + "\r\n";
            }
            return true;
        }

            // The one Content-Length all of the values agree on. False if one
            // is not a number or they differ.
        bool agreedLength( const std::vector<std::string> &contentLengths, size_t &nContentLength )
        {
            nContentLength = 0;
            for ( size_t nX = 0; nX < contentLengths.size(); nX++ )
            {
                size_t nValue = 0;
                if ( ! parseDecimal( contentLengths[ nX ], nValue ) )
                    return false;
                if ( nX > 0 && nValue != nContentLength )
                    return false;
                nContentLength = nValue;
            }
            return true;
        }

        enum ChunkResult { CHUNKS_NEED_MORE, CHUNKS_COMPLETE, CHUNKS_FAILED };

            // Decodes a chunked body (RFC 9112 7.1) that starts at nPos.
            // On CHUNKS_COMPLETE, nConsumed is the end of the message; on
            // CHUNKS_FAILED, nError is the status to report.
        ChunkResult decodeChunked( const char *pBuffer, size_t nLength, size_t nPos, size_t nMaxHeader, size_t nMaxBody,
                                   std::string &body, size_t &nConsumed, int &nError )
        {
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
                    {
                        nError = 400;
                        return CHUNKS_FAILED;
                    }
                    return CHUNKS_NEED_MORE;
                }

                std::string sizeLine( pBuffer + nPos, nLineEnd - nPos );
                if ( ! sizeLine.empty() && sizeLine[ sizeLine.size() - 1 ] == '\r' )
                    sizeLine.erase( sizeLine.size() - 1 );
                size_t nSemi = sizeLine.find( ';' );
                std::string hex = trim( ( nSemi == std::string::npos ) ? sizeLine : sizeLine.substr( 0, nSemi ) );
                if ( hex.empty() || hex.size() > 8 )
                {
                    nError = 400;
                    return CHUNKS_FAILED;
                }
                size_t nChunk = 0;
                for ( size_t nX = 0; nX < hex.size(); nX++ )
                {
                    if ( ! isxdigit( (unsigned char)hex[ nX ] ) )
                    {
                        nError = 400;
                        return CHUNKS_FAILED;
                    }
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
                            {
                                nError = 431;
                                return CHUNKS_FAILED;
                            }
                            return CHUNKS_NEED_MORE;
                        }
                        bool bBlank = ( nTrailerEnd == nPos ) || ( nTrailerEnd == nPos + 1 && pBuffer[ nPos ] == '\r' );
                        nPos = nTrailerEnd + 1;
                        if ( bBlank )
                            break;
                    }
                    nConsumed = nPos;
                    return CHUNKS_COMPLETE;
                }

                if ( body.size() + nChunk > nMaxBody )
                {
                    nError = 413;
                    return CHUNKS_FAILED;
                }
                if ( nLength - nPos < nChunk + 2 )
                {
                        // Wait for the chunk and its CRLF (or LF).
                    if ( nLength - nPos == nChunk + 1 && pBuffer[ nPos + nChunk ] == '\n' )
                    {
                        body.append( pBuffer + nPos, nChunk );
                        nPos += nChunk + 1;
                        continue;
                    }
                    return CHUNKS_NEED_MORE;
                }
                body.append( pBuffer + nPos, nChunk );
                nPos += nChunk;
                if ( pBuffer[ nPos ] == '\r' && pBuffer[ nPos + 1 ] == '\n' )
                    nPos += 2;
                else if ( pBuffer[ nPos ] == '\n' )
                    nPos += 1;
                else
                {
                    nError = 400;
                    return CHUNKS_FAILED;
                }
            }
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
        if ( ! findHeaderEnd( pBuffer, nLength, nStart, nHeaderEnd, nBodyStart ) )
        {
            if ( nLength - nStart > nMaxHeader )
                return failed( 431, false );
            return frame;   // NEED_MORE
        }

        if ( nBodyStart - nStart > nMaxHeader )
            return failed( 431, true );

        frame.headersComplete = true;

            // (NUL and lone CRs were passed on: the request was parsed with its
            // text cut at the NUL, and a lone CR could hide a header from this
            // check while another parser saw it.)
        if ( hasForbiddenBytes( pBuffer, nStart, nBodyStart ) )
            return failed( 400, true );

            // Walk the header lines (skipping the request line), collecting the
            // framing headers and keeping the others.
        CHeaderScan scan;
        if ( ! scanHeaders( pBuffer, nStart, nHeaderEnd, scan ) )
            return failed( 400, true );
        frame.expectContinue = scan.expectContinue;

        size_t nAvailable = nLength - nBodyStart;
        std::string body;
        size_t nConsumed = 0;

        if ( scan.hasTransferEncoding )
        {
                // RFC 9112 6.1 lets a server reject a message with both; doing
                // so closes a request-smuggling hole.
            if ( ! scan.contentLengths.empty() )
                return failed( 400, true );
            if ( scan.transferCodings.size() != 1 || scan.transferCodings[0] != "chunked" )
                return failed( scan.transferCodings.empty() ? 400 : 501, true );

            int nError = 0;
            ChunkResult result = decodeChunked( pBuffer, nLength, nBodyStart, nMaxHeader, nMaxBody, body, nConsumed, nError );
            if ( result == CHUNKS_FAILED )
                return failed( nError, true );
            if ( result == CHUNKS_NEED_MORE )
                return frame;
        }
        else if ( ! scan.contentLengths.empty() )
        {
            size_t nContentLength = 0;
            if ( ! agreedLength( scan.contentLengths, nContentLength ) )
                return failed( 400, true );
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
        frame.message = scan.firstLine + "\r\n" + scan.keptHeaders;
        frame.message += "Content-Length: " + std::to_string( body.size() ) + "\r\n\r\n";
        frame.message += body;
        return frame;
    }

    CHttpFrame CHttpUtils::frameResponse( const char *pBuffer, size_t nLength, bool bHeadRequest, bool bClosed,
                                          size_t nMaxHeader, size_t nMaxBody )
    {
        CHttpFrame frame;

            // Tolerate stray line ends before the status line (some servers
            // send an extra CRLF after a body).
        size_t nStart = 0;
        while ( nStart < nLength && ( pBuffer[ nStart ] == '\r' || pBuffer[ nStart ] == '\n' ) )
            nStart++;

        size_t nHeaderEnd = std::string::npos;
        size_t nBodyStart = 0;
        if ( ! findHeaderEnd( pBuffer, nLength, nStart, nHeaderEnd, nBodyStart ) )
        {
            if ( nLength - nStart > nMaxHeader )
                return failed( 431, false );
            if ( bClosed )
                return failed( 502, false );    // Closed before the headers ended.
            return frame;
        }
        if ( nBodyStart - nStart > nMaxHeader )
            return failed( 431, true );
        frame.headersComplete = true;

        if ( hasForbiddenBytes( pBuffer, nStart, nBodyStart ) )
            return failed( 502, true );

        CHeaderScan scan;
        if ( ! scanHeaders( pBuffer, nStart, nHeaderEnd, scan ) )
            return failed( 502, true );

            // status-line = HTTP-version SP status-code SP [ reason-phrase ]
            // (RFC 9112 4). The SP before an empty reason is often left out.
        const std::string &line = scan.firstLine;
        if ( line.size() < 12 || line.compare( 0, 5, "HTTP/" ) != 0 || ! isdigit( (unsigned char)line[5] ) || line[6] != '.' ||
             ! isdigit( (unsigned char)line[7] ) || line[8] != ' ' || ! isdigit( (unsigned char)line[9] ) ||
             ! isdigit( (unsigned char)line[10] ) || ! isdigit( (unsigned char)line[11] ) || ( line.size() > 12 && line[12] != ' ' ) )
            return failed( 502, true );
        int nStatus = ( line[9] - '0' ) * 100 + ( line[10] - '0' ) * 10 + ( line[11] - '0' );
        if ( nStatus < 100 )
            return failed( 502, true );
        frame.statusCode = nStatus;
        bool bHttp10 = ( line[5] == '1' && line[7] == '0' );
        frame.keepAlive = ! scan.connectionClose && ( ! bHttp10 || scan.connectionKeepAlive );

        std::string head = line + "\r\n" + scan.keptHeaders;
        std::string body;
        size_t nConsumed = nBodyStart;

            // RFC 9112 6.3: responses to HEAD, 1xx, 204 and 304 never have a
            // body, whatever their headers say. Their Content-Length (which
            // describes the body a GET would get) is kept.
        if ( bHeadRequest || nStatus < 200 || nStatus == 204 || nStatus == 304 )
        {
            if ( ! scan.contentLengths.empty() )
                head += "Content-Length: " + scan.contentLengths[0] + "\r\n";
            if ( nStatus == 101 )
                frame.keepAlive = false;    // The connection now speaks another protocol.
        }
        else if ( scan.hasTransferEncoding )
        {
                // Transfer-Encoding wins over Content-Length (6.3 rule 3).
            if ( ! scan.transferCodings.empty() && scan.transferCodings.back() == "chunked" )
            {
                if ( scan.transferCodings.size() != 1 )
                    return failed( 502, true );     // Other codings are not decoded.
                int nError = 0;
                ChunkResult result = decodeChunked( pBuffer, nLength, nBodyStart, nMaxHeader, nMaxBody, body, nConsumed, nError );
                if ( result == CHUNKS_FAILED )
                    return failed( nError == 413 ? 413 : 502, true );
                if ( result == CHUNKS_NEED_MORE )
                    return bClosed ? failed( 502, true ) : frame;
                head += "Content-Length: " + std::to_string( body.size() ) + "\r\n";
            }
            else
            {
                    // Not chunked last: the body runs to the end of the
                    // connection (6.3 rule 4), and the codings are left as
                    // they are for the caller.
                if ( nLength - nBodyStart > nMaxBody )
                    return failed( 413, true );
                if ( ! bClosed )
                    return frame;
                for ( size_t nX = 0; nX < scan.transferCodings.size(); nX++ )
                    head += std::string( nX == 0 ? "Transfer-Encoding: " : ", " ) + scan.transferCodings[ nX ];
                head += "\r\n";
                body.assign( pBuffer + nBodyStart, nLength - nBodyStart );
                nConsumed = nLength;
                frame.closeDelimited = true;
                frame.keepAlive = false;
            }
        }
        else if ( ! scan.contentLengths.empty() )
        {
            size_t nContentLength = 0;
            if ( ! agreedLength( scan.contentLengths, nContentLength ) )
                return failed( 502, true );
            if ( nContentLength > nMaxBody )
                return failed( 413, true );
            if ( nLength - nBodyStart < nContentLength )
                return bClosed ? failed( 502, true ) : frame;
            body.assign( pBuffer + nBodyStart, nContentLength );
            nConsumed = nBodyStart + nContentLength;
            head += "Content-Length: " + std::to_string( nContentLength ) + "\r\n";
        }
        else
        {
                // Neither: the body is everything until the server closes the
                // connection (6.3 rule 8).
            if ( nLength - nBodyStart > nMaxBody )
                return failed( 413, true );
            if ( ! bClosed )
                return frame;
            body.assign( pBuffer + nBodyStart, nLength - nBodyStart );
            nConsumed = nLength;
            frame.closeDelimited = true;
            frame.keepAlive = false;
        }

        frame.status = CHttpFrame::COMPLETE;
        frame.consumed = nConsumed;
        frame.message = head + "\r\n" + body;
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
