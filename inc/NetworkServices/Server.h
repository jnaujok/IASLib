/**
 *  Server Class
 *
 *      This class provides an abstract base class for providing a server
 * interface. Servers run in the background and wait for connections and 
 * then dispatch requests to handlers. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/16/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SERVER_H__
#define IASLIB_SERVER_H__

namespace IASLib
{
    class CServer : public CObject  
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
