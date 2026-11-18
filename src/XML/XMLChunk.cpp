/*
 * XML Chunk Class
 *
 *      The base type of XML elements and text.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/09/2007
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLChunk.h"

#include <string>

namespace IASLib
{
    CXMLChunk::CXMLChunk( void )
    {
    }

    CXMLChunk::~CXMLChunk( void )
    {
    }

    CString CXMLChunk::EscapeText( const char *strText )
    {
        std::string strOut;
        for ( const char *p = ( strText != NULL ) ? strText : ""; *p; p++ )
        {
            switch ( *p )
            {
                case '&':   strOut += "&amp;"; break;
                case '<':   strOut += "&lt;"; break;
                case '>':
                        // Only "]]>" must be escaped; leaving other '>'s alone
                        // keeps text as it was written.
                    if ( p - strText >= 2 && p[-1] == ']' && p[-2] == ']' )
                        strOut += "&gt;";
                    else
                        strOut += '>';
                    break;
                case '\r':  strOut += "&#13;"; break;    // a reader would make it "\n"
                default:    strOut += *p; break;
            }
        }
        return CString( strOut.data(), strOut.size() );
    }

    CString CXMLChunk::EscapeAttribute( const char *strText )
    {
        std::string strOut;
        for ( const char *p = ( strText != NULL ) ? strText : ""; *p; p++ )
        {
            switch ( *p )
            {
                case '&':   strOut += "&amp;"; break;
                case '<':   strOut += "&lt;"; break;
                case '"':   strOut += "&quot;"; break;
                    // A reader turns these into spaces unless they are
                    // written as references.
                case '\t':  strOut += "&#9;"; break;
                case '\n':  strOut += "&#10;"; break;
                case '\r':  strOut += "&#13;"; break;
                default:    strOut += *p; break;
            }
        }
        return CString( strOut.data(), strOut.size() );
    }
} // namespace IASLib
