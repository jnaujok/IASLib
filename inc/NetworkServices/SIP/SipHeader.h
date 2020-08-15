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

#ifndef IASLIB_SIPHEADER_H__
#define IASLIB_SIPHEADER_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "NetworkServices/Header.h"

namespace IASLib
{
    class CSipHeader : public CHeader
    {
        public:
                                /// Used to set a header from a name and a value
                            CSipHeader( const char *strName, const char *strValue );
                                /// Builds an HTTP Header with only the name for adding values to later
                            CSipHeader( const char *strName )  : CHeader( strName ) {}
                                /// Builds an HTTP Header with only the name for adding values to later
                            CSipHeader( const CString &strName ) : CHeader( strName ) {}
                                /// Copy Constructor
                            CSipHeader( const CSipHeader &oSource );
                                /// Destructor
            virtual        ~CSipHeader( void );

                            DEFINE_OBJECT( CSipHeader )

            CSipHeader    &operator =( const CSipHeader &oSource );
    };
}   // namespace IASLib

#endif // IASLIB_SIPHEADER_H__

#endif // IASLIB_NETWORKING__
