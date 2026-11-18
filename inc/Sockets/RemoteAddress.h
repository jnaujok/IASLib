/**
 * @file RemoteAddress.h
 * @brief CRemoteAddress, an old name for CInternetAddress.
 *
 *  Remote Address Class
 *
 *      This was a second copy of CInternetAddress that nothing used (and
 * that dereferenced NULL in several places). The name is kept for code that
 * refers to it.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 7/12/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_REMOTEADDRESS_H__
#define IASLIB_REMOTEADDRESS_H__

#ifdef IASLIB_NETWORKING__

#include "InternetAddress.h"

namespace IASLib
{
        /// An old name for CInternetAddress, kept for code that refers to it.
    typedef CInternetAddress CRemoteAddress;
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_REMOTEADDRESS_H__
