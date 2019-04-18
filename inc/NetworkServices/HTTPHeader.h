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

#ifndef IASLIB_HTTPHEADER_H__
#define IASLIB_HTTPHEADER_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
    class CHTTPHeader : public CObject
    {
        protected:
            CString         m_strHeaderName;
            CString         m_strHeaderValue;
        public:
                                /// Used to set a header from a name and a value
                            CHTTPHeader( const char *strName, const char *strValue );
                                /// Copy Constructor
                            CHTTPHeader( const CHTTPHeader &oSource );
                                /// Destructor
            virtual        ~CHTTPHeader( void );

                            DEFINE_OBJECT( CHTTPHeader )

            CHTTPHeader    &operator =( const CHTTPHeader &oSource );
            bool            IsHeader( const char *strCompareName );
            CString         GetValue( void );
            void            AddValue( const char *strValue );
    };
}   // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_HTTPHEADER_H__
