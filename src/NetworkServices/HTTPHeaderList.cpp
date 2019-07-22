/**
 *  HTTP Header List Class
 *
 *      This class provides a list of headers and is used by HTTP clients
 * and servers to maintain the header list for requests and responses.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/31/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */
#include "HTTPHeaderList.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{

    IMPLEMENT_OBJECT( CHTTPHeaderList, CHash );

    CHTTPHeaderList::CHTTPHeaderList( void )
    : CHash( "SMALL" )
    {

    }

    CHTTPHeaderList::CHTTPHeaderList( CStream &oStream ) : CHash( "SMALL" )
    {
        CString         strLine;
        CString         strUpcaseName;
        CString         strLastName;
        CHTTPHeader    *pHeader;
        int             nLines = 0;

        do
        {
            try
            {
                strLine = oStream.GetLine();
            }
            catch ( ... )
            {
                //ErrorLog( "Unexpected exception reading HTTP Header List." );
                break;
            }

            if ( ( strLine.IndexOf( ':' ) == NOT_FOUND ) && ( ( strLine[0] == ' ' ) || ( strLine[0] == '\t' ) ) )
            {
                // This is a continuation of the previous header.
                if ( strLastName.GetLength() != 0 )
                {
                    pHeader = (CHTTPHeader *)Get( strLastName );
                    strLine.Trim();
                    pHeader->AddValue( strLine );
                }
                else
                {
                    // We should really return a 400 Bad Request error at this point, since the headers
                    // are malformed. However, it'd be a pain in the butt, so I ain't gonna do it.
                    // Instead, I'm going to try to recover by tossing this line in the dumpster.
                }
            }
            else
            {
                    // Remove any leading/trailing white space.
                strLine.Trim();

                    // Now we see if this is a name:value pair, or if it's just crapola on the line.
                if ( strLine.IndexOf( ':' ) != NOT_FOUND )
                {
                        // This is a full-fledged header line with a name:value pair.
                        // There are two possibilities, one: this is a new header altogether,
                        // or two: this is a continuation of a previous tag.
                        //  To find out, we need to search the already parsed tags.
                    CString strName = strLine.Substring( 0, (int)strLine.IndexOf( ':' ) );
                    strName.Trim();
                    strUpcaseName = strName;
                    strUpcaseName.ToUpperCase();
                    CString strValue = strLine.Substring( strLine.IndexOf( ':' ) + 1 );
                    strValue.Trim();

                    pHeader = (CHTTPHeader *)Get( strUpcaseName );
                    if ( pHeader )
                    {
                        pHeader->AddValue( strValue );
                        strLastName = strUpcaseName;
                    }
                    else
                    {
                            // This is a brand new header:value pair.
                            // And here we creeate a header object to take care of it.
                        pHeader = new CHTTPHeader( strName, strValue );
                        Push( strUpcaseName, pHeader );
                        strLastName = strName;
                    }
                }
                else
                {
                    // We should really return a 400 Bad Request error at this point, since the headers
                    // are malformed. However, it'd be a pain in the butt, so I ain't gonna do it.
                    // Instead, I'm going to try to recover by tossing this line in the dumpster.
                }
            }
            nLines++;
        } while ( ( strLine.GetLength() > 0 ) && ( nLines < 256 ) ); // We go until a blank line (or an insane number of headers has come through)
    }

    CHTTPHeaderList::~CHTTPHeaderList( void )
    {
        DeleteAll();
    }

    CString CHTTPHeaderList::GetValue( const char *strHeaderName )
    {
        CHTTPHeader    *pHeader;
        CString         strTemp = strHeaderName;

        strTemp.Trim();
        strTemp.ToUpperCase();

        pHeader = (CHTTPHeader *)Get( strTemp );

        if ( pHeader )
        {
            return pHeader->GetValue();
        }

        strTemp = "";

        return strTemp;
    }

    CHTTPHeader *CHTTPHeaderList::GetHeader( const char *strHeaderName )
    {
        CHTTPHeader    *pHeader;
        CString         strTemp = strHeaderName;

        strTemp.Trim();
        strTemp.ToUpperCase();

        pHeader = (CHTTPHeader *)Get( strTemp );

        if ( pHeader )
        {
            return pHeader;
        }

        return NULL;
    }

    CIterator *CHTTPHeaderList::Enumerate( void )
    {
        return CHash::Enumerate();
    }

} // Namespace IASLib

#endif // IASLIB_NETWORKING__
