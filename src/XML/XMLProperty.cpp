/*
 * XML Property Class
 *
 *      This class provides a means of storing property=value pairs as
 * they are stored in XML files. This allows us to store and access
 * properties by name and get back their respective values. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "XMLProperty.h"

namespace IASLib 
{
    IMPLEMENT_OBJECT( CXMLProperty, CObject )

    CXMLProperty::CXMLProperty( const char *strName, const char *strValue )
    {
        m_strName = strName;

            // Property names don't include leading/trailing white space,
            // but they are case sensitive.
        m_strName.Trim();

        m_strValue = strValue;
    }

    CXMLProperty::~CXMLProperty( void )
    {
    }
} // namespace IASLib
