/**
 * @file
 * @brief XML Exception Class.
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
    /// Thrown by the XML classes, for example on badly formed XML.
    class CXMLException : public CException
    {
        public:
                        /// Makes an XML exception from a message code, with NORMAL severity.
                        /// @param nXMLMessage 0 gives "Successful. (?)", 1 gives
                        ///        "No ending brace found.", anything else gives
                        ///        "Unknown XML Exception!". The bottom stack frame keeps
                        ///        the message "Unknown Exception".
                        CXMLException( int nXMLMessage );
                        /// Makes an XML exception with a message and NORMAL severity.
                        /// @param strException The message text.
                        CXMLException( const char *strException );
            /// Destructor.
            virtual    ~CXMLException( void );

                        DEFINE_OBJECT( CXMLExcetption )
            /// Throws a copy of this exception with its real type.
            /// @throws CXMLException always.
            virtual void        Throw( void ) { throw *this; }


    };

/// Throws a CXMLException for @p x (a message code or a message string),
/// recording the current file and line.
#define IASLIB_THROW_XML_EXCEPTION( x ) {CXMLException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_XMLEXCEPTION_H__
