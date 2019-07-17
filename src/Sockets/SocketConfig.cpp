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

    /**
     *  Constructor
     *
     *      This method builds an empty socket configurator object.
     */
    CSocketConfig::CSocketConfig( void )
    {
    }

    /**
     *  Destructor
     *
     *      This method is used to clean up the socket configurator object.
     */
    CSocketConfig::~CSocketConfig()
    {
    }

    IMPLEMENT_OBJECT( CSocketConfigBuilder, CObject );

    CSocketConfig *CSocketConfigBuilder::build( void )
    {
        CSocketConfig *config = new CSocketConfig();
        config->recvBufferSize = recvBufferSize;
        config->backlogSize = backlogSize;
        config->recvBufferSize = recvBufferSize;
        config->sendBufferSize = sendBufferSize;
        config->keepAlive = keepAlive;
        config->linger = linger;
        config->reuseSocket = reuseSocket;
        config->timeoutMillis = timeoutMillis;
        config->noDelay = noDelay;
        config->blocking = blocking;

        return config;
    }

    CSocketConfigBuilder CSocketConfigBuilder::setBacklogSize( int backlogSize )
    {
        return *this;
    }

    CSocketConfigBuilder CSocketConfigBuilder::setReceiveBufferSize( int bufferSize )
    {
        this->recvBufferSize = bufferSize;
        return *this;
    }

    CSocketConfigBuilder CSocketConfigBuilder::setSendBufferSize( int bufferSize )
    {
        this->sendBufferSize = bufferSize;
        return *this;
    }
    CSocketConfigBuilder CSocketConfigBuilder::setKeepalive( bool keepalive )
    {
        this->keepAlive = keepalive;
        return *this;
    }
    CSocketConfigBuilder CSocketConfigBuilder::setLinger( bool linger )
    {
        this->linger = linger;
        return *this;
    }
    CSocketConfigBuilder CSocketConfigBuilder::setReuseAddress( bool reuse )
    {
        this->reuseSocket = reuse;
        return *this;
    }
    CSocketConfigBuilder CSocketConfigBuilder::setTimeout( int timeoutMillis )
    {
        this->timeoutMillis = timeoutMillis;
        return *this;
    }
    CSocketConfigBuilder CSocketConfigBuilder::setTCPNoDelay( bool noDelay )
    {
        this->noDelay = noDelay;
        return *this;
    }
    CSocketConfigBuilder CSocketConfigBuilder::setBlocking( bool blocking )
    {
        this->blocking = blocking;
        return *this;
    }

    CSocketConfigBuilder::CSocketConfigBuilder( void )
    {
        backlogSize = 32;
        recvBufferSize = 4096;
        sendBufferSize = 4096;
        keepAlive = false;
        linger = false;
        reuseSocket = false;
        timeoutMillis = 60000;
        noDelay = false;
        blocking = false;
    }

    CSocketConfigBuilder::CSocketConfigBuilder( CSocketConfigBuilder &oSource )
    {
        backlogSize = oSource.backlogSize;
        recvBufferSize = oSource.recvBufferSize;
        sendBufferSize = oSource.sendBufferSize;
        keepAlive = oSource.keepAlive;
        linger = oSource.linger;
        reuseSocket = oSource.reuseSocket;
        timeoutMillis = oSource.timeoutMillis;
        noDelay = oSource.noDelay;
        blocking = oSource.blocking;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
