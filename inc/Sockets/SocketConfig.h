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

#include "../BaseTypes/Object.h"

namespace IASLib
{
        // Settings for a new socket. Defaults: backlog 32, 4096-byte
        // buffers, no keep-alive, no linger, no address reuse, no timeout,
        // Nagle on, blocking. A timeout (milliseconds, 0 for none) applies to
        // each read and send; one that runs out makes Read() or Send() throw
        // a CSocketException (EWOULDBLOCK).
    class CSocketConfig : public CObject
    {
        private:
            int backlogSize;
            int recvBufferSize;
            int sendBufferSize;
            bool keepAlive;
            bool linger;
            int lingerSeconds;
            bool reuseSocket;
            int timeoutMillis;
            bool noDelay;
            bool blocking;
        public:
            CSocketConfig( void );
            CSocketConfig( const CSocketConfig &oSource );
            CSocketConfig &operator =( const CSocketConfig &oSource );
            virtual ~CSocketConfig( void );
            DEFINE_OBJECT( CSocketConfig );

            int getBacklogSize() const { return backlogSize; }
            int getReceiveBufferSize() const { return recvBufferSize; }
            int getSendBufferSize() const { return sendBufferSize; }
            bool isKeepAlive() const { return keepAlive; }
            bool isLinger() const { return linger; }
            int getLingerSeconds() const { return lingerSeconds; }
            bool isReuseSocket() const { return reuseSocket; }
            int getTimeout() const { return timeoutMillis; }
            bool isTcpNoDelay() const { return noDelay; }
            bool isBlocking() const { return blocking; }

            friend class CSocketConfigBuilder;
    };
}

namespace IASLib
{
        // Builds a CSocketConfig. The setters return the builder, so they
        // chain: CSocketConfigBuilder().setTimeout( 5000 ).setTCPNoDelay( true ).Build().
        // (They returned a copy, so every setting after the first in a chain
        // was lost.)
    class CSocketConfigBuilder : public CObject
    {
        private:
            CSocketConfig   m_config;
        public:
            DEFINE_OBJECT(CSocketConfigBuilder );

                // A new configuration, which the caller deletes.
            CSocketConfig *build( void ) const;
                // The configuration, by value.
            CSocketConfig  Build( void ) const { return m_config; }

            CSocketConfigBuilder &setBacklogSize( int backlogSize );
            CSocketConfigBuilder &setReceiveBufferSize( int bufferSize );
            CSocketConfigBuilder &setSendBufferSize( int bufferSize );
            CSocketConfigBuilder &setKeepalive( bool keepalive );
                // Whether Close() waits (up to nSeconds) for unsent data.
            CSocketConfigBuilder &setLinger( bool linger, int nSeconds = 10 );
            CSocketConfigBuilder &setReuseAddress( bool reuse );
            CSocketConfigBuilder &setTimeout( int timeoutMillis );
            CSocketConfigBuilder &setTCPNoDelay( bool noDelay );
            CSocketConfigBuilder &setBlocking( bool blocking );

            CSocketConfigBuilder( void );
            CSocketConfigBuilder( const CSocketConfigBuilder &oSource );
    };

} // end of namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // SOCKETCONFIG_H__
