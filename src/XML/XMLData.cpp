/*
 * XML Data Class
 *
 *      The text of an XML document or element.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/09/2007
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLData.h"

namespace IASLib
{
    CXMLData::CXMLData( void )
    {
        m_bCData = false;
    }

    CXMLData::CXMLData( const CString &strData, bool bCData )
    {
        m_strData = strData;
        m_bCData = bCData;
    }

    CXMLData::~CXMLData( void )
    {
    }

    bool CXMLData::IsWhitespace( void ) const
    {
        for ( size_t n = 0; n < m_strData.GetLength(); n++ )
        {
            char ch = m_strData[n];
            if ( ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' )
                return false;
        }
        return true;
    }

    CString CXMLData::toString( int offset, int indent ) const
    {
        CString strText;
        if ( m_bCData )
        {
                // "]]>" cannot be in a CDATA section: split it across two.
            CString strSafe = m_strData;
            strSafe.Replace( "]]>", "]]]]><![CDATA[>" );
            strText = "<![CDATA[";
            strText += strSafe;
            strText += "]]>";
        }
        else if ( indent > 0 )
        {
            CString strTrimmed = m_strData;
            strTrimmed.Trim( " \t\r\n" );
            if ( strTrimmed.IsEmpty() )
                return CString();
            strText = EscapeText( strTrimmed );
        }
        else
        {
            strText = EscapeText( m_strData );
        }

        if ( indent > 0 )
        {
            CString strLine = CString().Pad( ( offset > 0 ) ? (size_t)offset : 0 );
            strLine += strText;
            strLine += "\n";
            return strLine;
        }
        return strText;
    }
} // namespace IASLib
