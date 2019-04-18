/*
 * String Exception Class
 *
 *      This class is used for throwing whenever an exception occurs in
 * the string handling classes. String exception examples are failure to
 * allocate memory for a string, or passing a non-null terminated string
 * to a string function, or causing reference counting errors in the
 * string stub.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "StringException.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CStringException, CObject );

    CStringException::CStringException( const char *strMessage, CException::ExceptionSeverity nExceptionLevel )
    : CException( strMessage, nExceptionLevel )
    {
    }

    CStringException::CStringException( int nStringMessage )
        : CException( "Unknown Exception", CException::NORMAL )
    {
        switch ( nStringMessage )
        {
            case 0:
                m_strExceptionMessage = "Successful. (?)";
                break;

            default:
                m_strExceptionMessage = "Unknown String Exception!";
                break;
        }
    }
    CStringException::~CStringException( void )
    {
    }

} // namespace IASLib
