/**
 *  Generic Client Class
 *
 *      This class provides a base interface for connecting
 * to a Request/Response type server using the command/URI,
 * header block, data body, possible trailer format.
 *      Since this format was popularized by HTTP, this is now
 * being used by dozens of modern formats for transferring data
 * and thus it makes sense to abstract this protocol where 90%
 * of it can be reused.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 6/03/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_GENERICCLIENT_H__
#define IASLIB_GENERICCLIENT_H__

#ifdef IASLIB_NETWORKING__

#include "../Sockets/ClientSocket.h"
#include "Header.h"
#include "HeaderList.h"

namespace IASLib
{
    class CGenericClient : public CClientSocket
    {
        protected:
            bool            m_bUseHTTP11;
            bool            m_bUseKeepalive;
            bool            m_bConnected;
            CHTTPHeaderList m_hlHeaders;
            int             m_nLastResponseCode;
        public:
	                        CHTTPClient( const char *strRemoteHost, int nHTTPPort=80 );
	        virtual        ~CHTTPClient( void );

                            DEFINE_OBJECT( CHTTPClient )

            void            AddHeader( const char *strName, const char *strValue );
            CHTTPHeader    *GetHeader( const char *strName );

            bool            Get( const char *strURI );
            bool            Put( const char *strURI, void *pData, size_t nLength );
            bool            Head( const char *strURI );
            bool            Post( const char *strURI, void *pData, size_t nLength );
            bool            Options( const char *strURI );
            bool            Trace( const char *strURI, void *pData, size_t nLength );
            bool            Delete( const char *strURI );
        private:
            CString         BuildRequest( const char *strType, const char *strURI, void *pData = NULL, size_t nLength = 0 );
    };
} // namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // IASLIB_HTTPCLIENT_H__
