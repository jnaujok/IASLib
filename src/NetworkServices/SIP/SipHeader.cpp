/**
 *  SIP Header Class
 *
 *      This class supports an interface for accessing, sending, and 
 * receiving SIP Header values (name-value pairs in SIP formats). 
 * It also attempts to correctly interpret well-known header entries
 * (e.g. date types) so that it can do validity checking.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/22/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipHeader.h"
#include "BaseTypes/StringTokenizer.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipHeader, CHeader );

    CSipHeader::CSipHeader( const char *strName, const char *strValue ) : CHeader( strName )
    {
        this->addValue( strValue );
    }

    CSipHeader::CSipHeader( const CSipHeader &oSource ) : CHeader( oSource )
    {
    }

    CSipHeader::~CSipHeader( void )
    {
        // Destructor doesn't need to do anything.
    }

    CSipHeader &CSipHeader::operator =( const CSipHeader &oSource )
    {
        CHeader::operator =( oSource );
        return *this;
    }
} // Namespace IASLib

#endif