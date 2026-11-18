/*
 * XML Document Class
 *
 *      An XML document, read and written.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLDocument.h"
#include "XMLParser.h"
#include "XMLException.h"

namespace IASLib
{
    CXMLDocument::CXMLDocument( void )
    {
    }

    CXMLDocument::CXMLDocument( CStream *pInput )
    {
        Read( pInput );
    }

    CXMLDocument::~CXMLDocument( void )
    {
    }

    bool CXMLDocument::Read( CStream *pInput )
    {
        if ( pInput == NULL )
            IASLIB_THROW_XML_EXCEPTION( "Could not read input stream." );
        CString strXML = CXMLParser::ReadAll( pInput );
        return Parse( strXML, strXML.GetLength() );
    }

    bool CXMLDocument::Parse( const char *strXML, size_t nLength )
    {
        if ( strXML != NULL && nLength == NOT_FOUND )
            nLength = strlen( strXML );
        CXMLParser oParser;
        oParser.Parse( strXML, nLength, m_xiIndex );
        return true;
    }

    size_t CXMLDocument::Write( CStream *pOutput, int indent ) const
    {
        CString strDump = toString( indent );
        if ( pOutput != NULL )
            pOutput->PutBuffer( strDump, (int)strDump.GetLength() );
        return strDump.GetLength();
    }

    CString CXMLDocument::toString( int indent ) const
    {
        CString strRetVal;
        for ( size_t n = 0; n < m_xiIndex.GetChunkCount(); n++ )
            strRetVal += m_xiIndex.GetChunk( n )->toString( 0, indent );
        if ( indent > 0 && strRetVal.GetLength() > 0 && strRetVal[ strRetVal.GetLength() - 1 ] != '\n' )
            strRetVal += '\n';
        return strRetVal;
    }
} // namespace IASLib
