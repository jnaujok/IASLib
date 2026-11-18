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

#ifdef IASLIB_NETWORKING__

#include "SocketConfig.h"
#include "String_.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSocketConfig, CObject );

    CSocketConfig::CSocketConfig( void )
    {
            // (Every field was left uninitialised.)
        backlogSize = 32;
        recvBufferSize = 4096;
        sendBufferSize = 4096;
        keepAlive = false;
        linger = false;
        lingerSeconds = 10;
        reuseSocket = false;
        timeoutMillis = 0;
        noDelay = false;
        blocking = true;
    }

    CSocketConfig::CSocketConfig( const CSocketConfig &oSource ) : CObject()
    {
        *this = oSource;
    }

    CSocketConfig &CSocketConfig::operator =( const CSocketConfig &oSource )
    {
        backlogSize = oSource.backlogSize;
        recvBufferSize = oSource.recvBufferSize;
        sendBufferSize = oSource.sendBufferSize;
        keepAlive = oSource.keepAlive;
        linger = oSource.linger;
        lingerSeconds = oSource.lingerSeconds;
        reuseSocket = oSource.reuseSocket;
        timeoutMillis = oSource.timeoutMillis;
        noDelay = oSource.noDelay;
        blocking = oSource.blocking;
        return *this;
    }

    CSocketConfig::~CSocketConfig()
    {
    }

    IMPLEMENT_OBJECT( CSocketConfigBuilder, CObject );

    CSocketConfigBuilder::CSocketConfigBuilder( void )
    {
    }

    CSocketConfigBuilder::CSocketConfigBuilder( const CSocketConfigBuilder &oSource ) : CObject(), m_config( oSource.m_config )
    {
    }

    CSocketConfig *CSocketConfigBuilder::build( void ) const
    {
        return new CSocketConfig( m_config );
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setLinger( bool linger, int nSeconds )
    {
        m_config.linger = linger;
        m_config.lingerSeconds = ( nSeconds < 0 ) ? 0 : nSeconds;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setBacklogSize( int backlogSize )
    {
        m_config.backlogSize = backlogSize;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setReceiveBufferSize( int bufferSize )
    {
        m_config.recvBufferSize = bufferSize;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setSendBufferSize( int bufferSize )
    {
        m_config.sendBufferSize = bufferSize;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setKeepalive( bool keepalive )
    {
        m_config.keepAlive = keepalive;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setReuseAddress( bool reuse )
    {
        m_config.reuseSocket = reuse;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setTimeout( int timeoutMillis )
    {
        m_config.timeoutMillis = timeoutMillis;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setTCPNoDelay( bool noDelay )
    {
        m_config.noDelay = noDelay;
        return *this;
    }

    CSocketConfigBuilder &CSocketConfigBuilder::setBlocking( bool blocking )
    {
        m_config.blocking = blocking;
        return *this;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
