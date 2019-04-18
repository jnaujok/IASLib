/**
 *  HTTP Header Class
 *
 *      This class supports an interface for accessing, sending, and 
 * receiving HTTP Header values (name-value pairs in HTTP formats). 
 * It also attempts to correctly interpret well-known header entries
 * (e.g. date types) so that it can do validity checking.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/16/2006
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "HTTPHeader.h"
#include "StringTokenizer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHTTPHeader, CObject );

    CHTTPHeader::CHTTPHeader( const char *strName, const char *strValue )
    {
        m_strHeaderName = strName;
        m_strHeaderValue = strValue;
    }

    CHTTPHeader::CHTTPHeader( const CHTTPHeader &oSource )
    {
        m_strHeaderName = oSource.m_strHeaderName;
        m_strHeaderValue = oSource.m_strHeaderValue;
    }

    CHTTPHeader::~CHTTPHeader( void )
    {
        // Destructor doesn't need to do anything.
    }

    CHTTPHeader &CHTTPHeader::operator =( const CHTTPHeader &oSource )
    {
        if ( &oSource != this )
        {
            m_strHeaderName = oSource.m_strHeaderName;
            m_strHeaderValue = oSource.m_strHeaderValue;
        }
        return *this;
    }

    bool CHTTPHeader::IsHeader( const char *strCompareName )
    {
        CString strCompare1 = m_strHeaderName;
        CString strCompare2 = strCompareName;

        strCompare1.ToUpperCase();
        strCompare2.ToUpperCase();

        if ( strCompare1 == strCompare2 )
        {
            return true;
        }

        return false;
    }

    CString CHTTPHeader::GetValue( void )
    {
        return m_strHeaderValue;
    }

    void CHTTPHeader::AddValue( const char *strValue )
    {
        if ( m_strHeaderValue.GetLength() > 0 )
        {
            m_strHeaderValue += ", ";
        }

        m_strHeaderValue += strValue;
    }
} // Namespace IASLib
