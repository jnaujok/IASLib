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

#include "HttpHeader.h"
#include "StringTokenizer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CHttpHeader, CHeader );

    CHttpHeader::CHttpHeader( const char *strName, const char *strValue ) : CHeader( strName )
    {
        this->addValue( strValue );
    }

    CHttpHeader::CHttpHeader( const CHttpHeader &oSource ) : CHeader( oSource )
    {
    }

    CHttpHeader::~CHttpHeader( void )
    {
        // Destructor doesn't need to do anything.
    }

    CHttpHeader &CHttpHeader::operator =( const CHttpHeader &oSource )
    {
        CHeader::operator =( oSource );
        return *this;
    }

} // Namespace IASLib
