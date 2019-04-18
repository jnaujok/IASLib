/*
 * XML Exception Class
 *
 *      This class is used for throwing whenever an exception occurs in
 * the XML handling classes. XML exceptions can take many forms, and 
 * include things like parsing poorly formed XML data.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/02/2005
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLEXCEPTION_H__
#define IASLIB_XMLEXCEPTION_H__

#include "Exception.h"

namespace IASLib
{
    class CXMLException : public CException
    {
        public:
                        CXMLException( int nXMLMessage );
                        CXMLException( const char *strException );
            virtual    ~CXMLException( void );

                        DEFINE_OBJECT( CXMLExcetption )
            virtual void        Throw( void ) { throw *this; }


    };

#define IASLIB_THROW_XML_EXCEPTION( x ) {static CXMLException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_XMLEXCEPTION_H__
