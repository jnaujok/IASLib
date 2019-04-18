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

#include "XMLException.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CXMLException, CException );

    CXMLException::CXMLException( int nXMLMessage )
        : CException( "Unknown Exception", CException::NORMAL )
    {
        switch ( nXMLMessage )
        {
            case 0:
                m_strExceptionMessage = "Successful. (?)";
                break;

            case 1:
                m_strExceptionMessage = "No ending brace found.";

            default:
                m_strExceptionMessage = "Unknown XML Exception!";
                break;
        }
    }

    CXMLException::CXMLException( const char *strMessage )
        : CException( strMessage, CException::NORMAL )
    {
    }

    CXMLException::~CXMLException( void )
    {
    }

} // namespace IASLib
