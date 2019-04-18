/**
 *  HTTP Response Class
 *
 *      This class provides an interface for retrieving the data from
 * an HTTP request. The response is used by the HTTP Client to return
 * data from the response to the calling program.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 9/07/2006
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HTTPRESPONSE_H__
#define IASLIB_HTTPRESPONSE_H__

#ifdef IASLIB_NETWORKING__

#include "../Streams/Stream.h"
#include "HTTPHeaderList.h"

namespace IASLib
{
    class CHTTPResponse :: public CObject
    {
        protected:
            CHTTPHeaderList     m_HeaderList;
            CString             m_strResponseData;
            CString             m_strResponseLine;
            int                 m_nReturnCode;
        public:

    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_HTTPRESPONSE_H__
