/**
 * @file SocketConfig.h
 * @brief CSocketConfig, the settings for a new socket, and CSocketConfigBuilder.
 *
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
        /// Settings for a new socket. Defaults: backlog 32, 4096-byte
        /// buffers, no keep-alive, no linger, no address reuse, no timeout,
        /// Nagle on, blocking. A timeout (milliseconds, 0 for none) applies to
        /// each read and send; one that runs out makes Read() or Send() throw
        /// a CSocketException (EWOULDBLOCK).
        ///
        /// Make one with CSocketConfigBuilder; it is copied freely and can be
        /// used for any number of sockets.
    class CSocketConfig : public CObject
    {
        private:
            int backlogSize;            ///< The listen backlog.
            int recvBufferSize;         ///< SO_RCVBUF, in bytes.
            int sendBufferSize;         ///< SO_SNDBUF, in bytes.
            bool keepAlive;             ///< SO_KEEPALIVE.
            bool linger;                ///< SO_LINGER on.
            int lingerSeconds;          ///< SO_LINGER's time.
            bool reuseSocket;           ///< SO_REUSEPORT on listening sockets (where the system has it).
            int timeoutMillis;          ///< The read and send timeout; 0 for none.
            bool noDelay;               ///< TCP_NODELAY (Nagle off).
            bool blocking;              ///< Whether calls block.
        public:
                /// The defaults (see the class).
            CSocketConfig( void );
                /// A copy.
            CSocketConfig( const CSocketConfig &oSource );
                /// A copy.
            CSocketConfig &operator =( const CSocketConfig &oSource );
                /// Destroys the configuration.
            virtual ~CSocketConfig( void );
            DEFINE_OBJECT( CSocketConfig );

            int getBacklogSize() const { return backlogSize; }              ///< The listen backlog.
            int getReceiveBufferSize() const { return recvBufferSize; }     ///< The receive buffer size, in bytes.
            int getSendBufferSize() const { return sendBufferSize; }        ///< The send buffer size, in bytes.
            bool isKeepAlive() const { return keepAlive; }                  ///< Whether TCP keep-alive is on.
            bool isLinger() const { return linger; }                        ///< Whether Close() lingers for unsent data.
            int getLingerSeconds() const { return lingerSeconds; }          ///< How long Close() lingers, in seconds.
            bool isReuseSocket() const { return reuseSocket; }              ///< Whether several listeners may share the port.
            int getTimeout() const { return timeoutMillis; }                ///< The read and send timeout in milliseconds (0: none).
            bool isTcpNoDelay() const { return noDelay; }                   ///< Whether Nagle's algorithm is off.
            bool isBlocking() const { return blocking; }                    ///< Whether calls block.

            friend class CSocketConfigBuilder;
    };
}

namespace IASLib
{
        /// Builds a CSocketConfig. The setters return the builder, so they
        /// chain.
        ///
        /// @code
        /// CSocketConfig oConfig = CSocketConfigBuilder().setTimeout( 5000 ).setTCPNoDelay( true ).Build();
        /// @endcode
        ///
        /// @note The setters used to return a copy, so every setting after
        ///       the first in a chain was lost.
    class CSocketConfigBuilder : public CObject
    {
        private:
            CSocketConfig   m_config;       ///< The configuration being built.
        public:
            DEFINE_OBJECT(CSocketConfigBuilder );

                /// A new configuration, which the caller deletes.
            CSocketConfig *build( void ) const;
                /// The configuration, by value.
            CSocketConfig  Build( void ) const { return m_config; }

                /// Sets the listen backlog. @return This builder.
            CSocketConfigBuilder &setBacklogSize( int backlogSize );
                /// Sets the receive buffer size, in bytes. @return This builder.
            CSocketConfigBuilder &setReceiveBufferSize( int bufferSize );
                /// Sets the send buffer size, in bytes. @return This builder.
            CSocketConfigBuilder &setSendBufferSize( int bufferSize );
                /// Turns TCP keep-alive on or off. @return This builder.
            CSocketConfigBuilder &setKeepalive( bool keepalive );
                /// Whether Close() waits (up to nSeconds) for unsent data.
                /// @param linger  Whether to linger.
                /// @param nSeconds  How long (less than 0 counts as 0).
                /// @return This builder.
            CSocketConfigBuilder &setLinger( bool linger, int nSeconds = 10 );
                /// Lets several listening sockets share the port (SO_REUSEPORT,
                /// where the system has it). @return This builder.
            CSocketConfigBuilder &setReuseAddress( bool reuse );
                /// Sets the read and send timeout in milliseconds (0: none). @return This builder.
            CSocketConfigBuilder &setTimeout( int timeoutMillis );
                /// Turns Nagle's algorithm off (true) or on. @return This builder.
            CSocketConfigBuilder &setTCPNoDelay( bool noDelay );
                /// Sets whether calls block. @return This builder.
            CSocketConfigBuilder &setBlocking( bool blocking );

                /// A builder holding the defaults.
            CSocketConfigBuilder( void );
                /// A copy.
            CSocketConfigBuilder( const CSocketConfigBuilder &oSource );
    };

} // end of namespace IASLib

#endif // IASLIB_NETWORKING__
#endif // SOCKETCONFIG_H__
