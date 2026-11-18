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
                        /// Makes an exception from a message number: 0 gives
                        /// "Successful. (?)", 1 "No ending brace found.", anything
                        /// else "Unknown XML Exception!".
                        /// @note The call stack used to keep saying "Unknown Exception".
                        CXMLException( int nXMLMessage );
                        /// Makes an exception with a message (NORMAL severity).
                        CXMLException( const char *strException );
            virtual    ~CXMLException( void );   ///< Destructor.

                        DEFINE_OBJECT( CXMLException )
            virtual void        Throw( void ) { throw *this; }   ///< Throws a copy of this exception, as this class.
            virtual void        Rethrow( void ) { throw *this; }   ///< Same as Throw(); used by IASLIB_RETHROW.


    };

/// Throws a CXMLException for @p x (a message number or text), recording the file and line.
#define IASLIB_THROW_XML_EXCEPTION( x ) {CXMLException oE(x);oE.FileLine(__FILE__,__LINE__);oE.Throw();}

} // namespace IASLib

#endif // IASLIB_XMLEXCEPTION_H__
