/**
 * @file HttpHeader.h
 * @brief One HTTP header field (CHttpHeader).
 *
 *  Http Header Class
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

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_HTTPHEADER_H__
#define IASLIB_HTTPHEADER_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"
#include "NetworkServices/Header.h"

namespace IASLib
{
    /// One HTTP header field: a name and its values.
    ///
    /// It adds nothing to CHeader beyond its constructors; CHttpHeaderList
    /// stores its headers as CHttpHeader objects.
    class CHttpHeader : public CHeader
    {
        public:
                                /// Used to set a header from a name and a value.
                                /// A value holding CR, LF or NUL is not stored
                                /// (see CHeader::addValue()).
                            CHttpHeader( const char *strName, const char *strValue );
                                /// Builds an HTTP Header with only the name for adding values to later.
                            CHttpHeader( const char *strName )  : CHeader( strName ) {}
                                /// Same, from a CString name.
                            CHttpHeader( const CString &strName ) : CHeader( strName ) {}
                                /// Copy Constructor.
                            CHttpHeader( const CHttpHeader &oSource );
                                /// Destructor.
            virtual        ~CHttpHeader( void );

                            DEFINE_OBJECT( CHttpHeader )

                                /// Copies the name and values of another header.
            CHttpHeader    &operator =( const CHttpHeader &oSource );
    };
}   // namespace IASLib

#endif // IASLIB_HTTPHEADER_H__

#endif // IASLIB_NETWORKING__
