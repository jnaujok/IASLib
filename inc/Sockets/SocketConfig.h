/**
 * Socket Config class
 *
 *      This class provides a simple and quick builder interface for setting
 * any number of socket connection settings rapidly by creating a configuration
 * object that can be re-used any number of times.
 *      It also simplifies sockets by not requiring memory of the myriad
 * obscure socket settings.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 6/03/2019
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef SOCKETCONFIG_H__
#define SOCKETCONFIG_H__

#ifdef IASLIB_NETWORKING__

#include "ClientSocket.h"
#include "Socket.h"

namespace IASLib
{
    class CSocketConfig : public CObject
    {
        private:
            int backlogSize;
            int recvBufferSize;
            int sendBufferSize;
            bool keepAlive;
            bool linger;
            bool reuseSocket;
            int timeoutMillis;
            bool noDelay;
        protected:
            CSocketConfig( void );
        public:
            DEFINE_OBJECT( CSocketConfig );

            int getBacklogSize() { return backlogSize; }
            int getReceiveBufferSize() {return recvBufferSize;}
            int getSendBufferSize(){return sendBufferSize;}
            bool isKeepAlive(){ return keepAlive;}
            bool isLinger(){return linger;}
            bool isReuseSocket(){return reuseSocket;}
            int getTimeout(){return timeoutMillis;}
            bool    isTcpNoDelay(){return noDelay;}

            friend class CSocketConfigBuilder;
    };
}

namespace IASLib
{
    class CSocketConfigBuilder : public CObject
    {
        private:
            int backlogSize;
            int recvBufferSize;
            int sendBufferSize;
            bool keepAlive;
            bool linger;
            bool reuseSocket;
            int timeoutMillis;
            bool noDelay;
        public:
            DEFINE_OBJECT(CSocketConfigBuilder );

            CSocketConfig *build( void );

            CSocketConfigBuilder setBacklogSize( int backlogSize );
            CSocketConfigBuilder setReceiveBufferSize( int bufferSize );
            CSocketConfigBuilder setSendBufferSize( int bufferSize );
            CSocketConfigBuilder setKeepalive( bool keepalive );
            CSocketConfigBuilder setLinger( bool linger );
            CSocketConfigBuilder setReuseAddress( bool reuse );
            CSocketConfigBuilder setTimeout( int timeoutMillis );
            CSocketConfigBuilder setTCPNoDelay( bool noDelay );

            CSocketConfigBuilder( void );
            CSocketConfigBuilder( CSocketConfigBuilder &oSource );
    };

} // end of namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // SOCKETCONFIG_H__
