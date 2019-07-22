/*
 * Thread Exception Class
 *
 *      This class is used for throwing whenever an exception occurs in
 * the a thread related class. Thread exceptions include cancelling or stopping
 * an already completed thread, or returning a thread to a thread pool that
 * is not part of that thread pool
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/21/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "ThreadException.h"

namespace IASLib
{
    CThreadException::CThreadException( const char *strMessage )
        : CException( strMessage, CException::NORMAL )
    {
    }
         
    CThreadException::~CThreadException( void )
    {
    }
} // namespace IASLib
