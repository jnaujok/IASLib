/*
 * Stream Exception Class
 *
 *      This class is used for throwing whenever an exception occurs in
 * the stream handling classes. Stream exceptions can fall into multiple
 * classes, including attempting to read past the end of a file, or
 * attempting to write to an input-only stream.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/08/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "StreamException.h"

namespace IASLib
{
    CStreamException::CStreamException( const char *strMessage )
        : CException( strMessage, CException::NORMAL )
    {
    }
         
    CStreamException::~CStreamException( void )
    {
    }
} // namespace IASLib
