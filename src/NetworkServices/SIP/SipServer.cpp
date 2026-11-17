/**
 *  SIP Server Class
 *
 *      The protocol core of a SIP user agent server. See SipServer.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: December 21, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/SipServer.h"
#include "NetworkServices/SIP/SipUtils.h"
#include "Logging/LogSink.h"

#include <chrono>
#include <cstring>

namespace IASLib
{
        // The headers of a request that its responses copy (RFC 3261 8.2.6.2).
    struct CSipResponseParts
    {
        CStringArray    vias;
        CString         from;
        CString         to;
        CString         callId;
        CString         cseq;
        CString         timestamp;
        CStringArray    recordRoutes;

        void fromRequest( CSipRequest *request )
        {
            vias = request->getHeaderValues( "Via" );
            from = request->getHeaderValue( "From" );
            to = request->getHeaderValue( "To" );
            callId = request->getHeaderValue( "Call-ID" );
            cseq = request->getHeaderValue( "CSeq" );
            timestamp = request->getHeaderValue( "Timestamp" );
            recordRoutes = request->getHeaderValues( "Record-Route" );
        }
    };

        // A server transaction (RFC 3261 17.2). The ACCEPTED state is the one
        // RFC 6026 adds after a 2xx response to INVITE.
    struct CSipServerTransaction
    {
        enum State { PROCEEDING, COMPLETED, CONFIRMED, ACCEPTED };

        std::string         key;
        std::string         mergeKey;
        bool                isInvite;
        State               state;
        CSipTransport      *transport;
        bool                reliable;
        CInternetAddress    target;
        CString             toTag;
        CString             lastResponse;
        int                 lastStatus;
        CSipResponseParts   parts;
        unsigned long long  nextRetransmit;
        unsigned long long  retransmitInterval;
        unsigned long long  expires;

        CSipServerTransaction( void ) : isInvite( false ), state( PROCEEDING ), transport( NULL ), reliable( false ),
                                        lastStatus( 0 ), nextRetransmit( 0 ), retransmitInterval( 0 ), expires( 0 )
        {
        }
    };

        // A 2xx response to INVITE waiting for its ACK (RFC 3261 13.3.1.4).
    struct CSipPending2xx
    {
        CSipTransport      *transport;
        bool                reliable;
        CInternetAddress    target;
        CString             response;
        unsigned long long  nextRetransmit;
        unsigned long long  retransmitInterval;
        unsigned long long  expires;

        CSipPending2xx( void ) : transport( NULL ), reliable( false ), nextRetransmit( 0 ), retransmitInterval( 0 ), expires( 0 )
        {
        }
    };

    namespace
    {
        std::string toStd( const CString &value )
        {
            return std::string( (const char *)value, value.GetLength() );
        }

        std::string pendingKey( const CString &callId, unsigned long cseq, const CString &toTag )
        {
            return toStd( callId ) + "|" + std::to_string( cseq ) + "|" + toStd( toTag );
        }

        CString lowerCase( const CString &value )
        {
            CString retVal = value;
            retVal.ToLowerCase();
            return retVal;
        }
    }

    IMPLEMENT_OBJECT( CSipServer, CGenericServer );

    CSipServer::CSipServer( int nSipPort, bool bSecure, bool useUDP ) : CGenericServer(), m_aHandlerFactories()
    {
        m_bSecure = bSecure;
        m_bIsUDP = useUDP;
        m_nPort = nSipPort;
        m_strRealm = "IASLib";
        m_strServerName = "IASLib/2.0";
        m_bSend100Trying = true;
        m_bRequireTlsForSips = true;
        m_nT1 = 500;
        m_nT2 = 4000;
        m_nT4 = 5000;
    }

    CSipServer::CSipServer( const char *boundInterface, int nSipPort, bool bSecure, bool useUDP ) : CGenericServer(), m_aHandlerFactories()
    {
        m_bSecure = bSecure;
        m_bIsUDP = useUDP;
        m_nPort = nSipPort;
        m_strBindAddress = boundInterface ? boundInterface : "";
        m_strRealm = "IASLib";
        m_strServerName = "IASLib/2.0";
        m_bSend100Trying = true;
        m_bRequireTlsForSips = true;
        m_nT1 = 500;
        m_nT2 = 4000;
        m_nT4 = 5000;
    }

    CSipServer::~CSipServer( void )
    {
        m_aHandlerFactories.DeleteAll();
    }

    void *CSipServer::Run( void )
    {
            // Intentionally empty: concrete servers provide the transports.
        return NULL;
    }

    CSipHandler *CSipServer::GetHandler( CSipRequest *request, CUUID erid )
    {
        CSipHandlerFactory *pFactory = NULL;
        CSipHandler *pRetVal = NULL;

        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            pFactory = (CSipHandlerFactory *)m_aHandlerFactories[ nX ];
            pRetVal = pFactory->getHandler( request, erid );
            if ( pRetVal )
            {
                pRetVal->setServer( this );
                break;
            }
        }

        return pRetVal;
    }

    void CSipServer::AddHandlerFactory( CSipHandlerFactory *pFactory )
    {
        if ( pFactory )
        {
            m_aHandlerFactories.Prepend( pFactory );
        }
    }

    void CSipServer::RemoveHandlerFactory( CSipHandlerFactory *pFactory )
    {
        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            if ( m_aHandlerFactories[ nX ] == pFactory )
            {
                m_aHandlerFactories.Remove( nX );
                break;
            }
        }
    }

    CString CSipServer::getAllowedList( void )
    {
        CStringArray all;

        for ( size_t nX = 0; nX < m_aHandlerFactories.GetCount(); nX++ )
        {
            CStringArray methods = ((CSipHandlerFactory *)m_aHandlerFactories[ nX ])->getMethods();
            for ( size_t nY = 0; nY < methods.Count(); nY++ )
            {
                bool bFound = false;
                for ( size_t nZ = 0; nZ < all.Count(); nZ++ )
                {
                    if ( all[ nZ ] == methods[ nY ] )
                    {
                        bFound = true;
                        break;
                    }
                }
                if ( ! bFound )
                {
                    all.Append( methods[ nY ] );
                }
            }
        }

        CString retVal;
        for ( size_t nX = 0; nX < all.Count(); nX++ )
        {
            if ( nX > 0 )
                retVal += ", ";
            retVal += all[ nX ];
        }
        return retVal;
    }

    void CSipServer::addSupportedExtension( const char *optionTag )
    {
        CString tag( optionTag );
        tag.Trim();
        if ( tag.GetLength() > 0 && ! isSupportedExtension( tag ) )
        {
            m_aSupported.Append( tag );
        }
    }

    bool CSipServer::isSupportedExtension( const CString &optionTag )
    {
        CString target = lowerCase( optionTag );
        target.Trim();
        for ( size_t nX = 0; nX < m_aSupported.Count(); nX++ )
        {
            if ( lowerCase( m_aSupported[ nX ] ) == target )
                return true;
        }
        return false;
    }

    CString CSipServer::getSupportedList( void )
    {
        CString retVal;
        for ( size_t nX = 0; nX < m_aSupported.Count(); nX++ )
        {
            if ( nX > 0 )
                retVal += ", ";
            retVal += m_aSupported[ nX ];
        }
        return retVal;
    }

    void CSipServer::setTimerValues( unsigned long nT1, unsigned long nT2, unsigned long nT4 )
    {
        m_nT1 = ( nT1 > 0 ) ? nT1 : 500;
        m_nT2 = ( nT2 >= m_nT1 ) ? nT2 : m_nT1;
        m_nT4 = nT4;
    }

    unsigned long long CSipServer::currentTimeMs( void )
    {
        return (unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch() ).count();
    }

    size_t CSipServer::getTransactionCount( void )
    {
        std::lock_guard<std::mutex> lock( m_transactionMutex );
        return m_transactions.size();
    }

    /**********************************************************************
     * Message entry point
     **********************************************************************/

    void CSipServer::handleMessage( const CString &message, CSipTransport *pTransport, const CInternetAddress &source )
    {
        if ( pTransport == NULL )
        {
            return;
        }

        size_t nLength = message.GetLength();
        size_t nStart = 0;
        while ( nStart < nLength && ( message[ nStart ] == '\r' || message[ nStart ] == '\n' ) )
        {
            nStart++;
        }

        if ( nStart == nLength )
        {
                // A keep-alive. On a connection, a double CRLF "ping" is
                // answered with a single CRLF "pong" (RFC 5626 4.4.1).
            if ( pTransport->isReliable() && nLength >= 4 )
            {
                pTransport->send( CString( "\r\n" ), source );
            }
            return;
        }

        if ( ( nLength - nStart ) >= 4 && strncmp( (const char *)message + nStart, "SIP/", 4 ) == 0 )
        {
                // This core is a server only; it has no client transactions
                // for a response to match.
            DEBUG_LOG( "Ignoring a SIP response received by the server." );
            return;
        }

        try
        {
            CSipRequest request( source );
            bool bParsed = request.parse( message );
            processRequest( &request, bParsed, pTransport, source );
        }
        catch ( ... )
        {
            ERROR_LOG( "Unexpected exception while processing a SIP message." );
        }
    }

    void CSipServer::processRequest( CSipRequest *request, bool parseOk, CSipTransport *pTransport, const CInternetAddress &source )
    {
        CString method = request->getRequestType();
        bool bIsAck = ( method == "ACK" );

            // Without a Via there is nowhere to send a response.
        if ( request->getTopVia().GetLength() == 0 )
        {
            CInternetAddress from( source );
            WARN_LOG( "Dropping a SIP request with no Via header from %s.", (const char *)from.toStringWithPort() );
            return;
        }

        updateTopVia( request, source );
        CInternetAddress target = responseTarget( request, pTransport, source );

            // An ACK is never answered, even when it is malformed.
        if ( ! parseOk )
        {
            if ( ! bIsAck )
                sendStateless( request, 400, request->getParseError(), pTransport, target );
            return;
        }

        CString version = request->getVersion();
        version.ToUpperCase();
        if ( version != "SIP/2.0" )
        {
            if ( ! bIsAck )
                sendStateless( request, 505, NULL, pTransport, target );
            return;
        }

        static const char *required[] = { "To", "From", "Call-ID", "CSeq", NULL };
        for ( int nX = 0; required[ nX ]; nX++ )
        {
            if ( request->getHeaderValue( required[ nX ] ).GetLength() == 0 )
            {
                if ( ! bIsAck )
                {
                    CString reason = "Missing ";
                    reason += required[ nX ];
                    reason += " Header";
                    sendStateless( request, 400, reason, pTransport, target );
                }
                return;
            }
        }

        unsigned long nCSeq = 0;
        CString cseqMethod;
        if ( ! CSipUtils::parseCSeq( request->getHeaderValue( "CSeq" ), nCSeq, cseqMethod ) )
        {
            if ( ! bIsAck )
                sendStateless( request, 400, "Invalid CSeq Header", pTransport, target );
            return;
        }
        if ( cseqMethod != method )
        {
            if ( ! bIsAck )
                sendStateless( request, 400, "CSeq Method Does Not Match", pTransport, target );
            return;
        }

        if ( bIsAck )
        {
            handleAck( request, pTransport, source );
            return;
        }

        std::string key = transactionKey( request, method );
        std::shared_ptr<CSipServerTransaction> txn;

        {
            std::lock_guard<std::mutex> lock( m_transactionMutex );

            std::map< std::string, std::shared_ptr<CSipServerTransaction> >::iterator it = m_transactions.find( key );
            if ( it != m_transactions.end() )
            {
                    // A retransmission: send the latest response again
                    // (17.2.1, 17.2.2). If there is none yet, absorb it.
                std::shared_ptr<CSipServerTransaction> existing = it->second;
                if ( existing->lastResponse.GetLength() > 0 && existing->transport )
                {
                    existing->transport->send( existing->lastResponse, existing->target );
                }
                return;
            }

            if ( method == "CANCEL" && handleCancel( request, pTransport, target ) )
            {
                return;
            }

                // Merged requests (8.2.2.2): the same request arriving again by
                // a different path has the same From tag, Call-ID and CSeq but a
                // different transaction.
            std::string mergeKey;
            bool bMerged = false;
            if ( request->getToTag().GetLength() == 0 )
            {
                mergeKey = toStd( request->getFromTag() ) + "|" + toStd( request->getHeaderValue( "Call-ID" ) ) + "|" +
                           std::to_string( nCSeq ) + "|" + toStd( method );
                for ( it = m_transactions.begin(); it != m_transactions.end(); ++it )
                {
                    if ( it->second->mergeKey == mergeKey )
                    {
                        bMerged = true;
                        break;
                    }
                }
            }

            txn = std::make_shared<CSipServerTransaction>();
            txn->key = key;
            txn->mergeKey = bMerged ? std::string() : mergeKey;
            txn->isInvite = ( method == "INVITE" );
            txn->state = CSipServerTransaction::PROCEEDING;
            txn->transport = pTransport;
            txn->reliable = pTransport->isReliable();
            txn->target = target;
            txn->toTag = CSipUtils::randomToken( 8 );
            txn->parts.fromRequest( request );
            m_transactions[ key ] = txn;

            if ( bMerged )
            {
                CSipResponse *response = buildResponse( txn->parts, 482, NULL, txn->toTag );
                sendFinalLocked( txn, response );
                delete response;
                return;
            }

            if ( txn->isInvite && m_bSend100Trying )
            {
                CSipResponse *trying = buildResponse( txn->parts, 100, NULL, txn->toTag );
                sendProvisionalLocked( txn, trying );
                delete trying;
            }
        }

        CUUID erid;
        CSipHandler *handler = GetHandler( request, erid );
        CSipResponse *response = createResponse( request, 500, NULL, txn->toTag );

        if ( handler == NULL )
        {
                // 8.2.1: an unknown method gets 501, a known one we do not
                // handle gets 405. Both list what is allowed.
            response->setStatus( CSipUtils::isKnownMethod( method ) ? 405 : 501 );
            response->addHeader( "Allow", getAllowedList() );
        }
        else
        {
            CString scheme = CSipUtils::uriScheme( request->getUri() );

                // Collect any Require option tags we do not support (8.2.2.3).
                // CANCEL and ACK must not be rejected this way.
            CString unsupported;
            if ( method != "CANCEL" )
            {
                CStringArray requires = request->getHeaderValues( "Require" );
                for ( size_t nX = 0; nX < requires.Count(); nX++ )
                {
                    CStringArray tags = CSipUtils::splitList( requires[ nX ] );
                    for ( size_t nY = 0; nY < tags.Count(); nY++ )
                    {
                        if ( ! isSupportedExtension( tags[ nY ] ) )
                        {
                            if ( unsupported.GetLength() > 0 )
                                unsupported += ", ";
                            unsupported += tags[ nY ];
                        }
                    }
                }
            }

            bool bSecureTransport = ( pTransport != NULL && strcmp( pTransport->getProtocol(), "TLS" ) == 0 );
            if ( scheme != "sip" && scheme != "sips" && scheme != "tel" )
            {
                response->setStatus( 416 );
            }
            else if ( scheme == "sips" && m_bRequireTlsForSips && ! bSecureTransport )
            {
                    // The sips: guarantee (TLS on every hop) cannot be kept.
                response->setStatus( 416 );
            }
            else if ( unsupported.GetLength() > 0 )
            {
                response->setStatus( 420 );
                response->addHeader( "Unsupported", unsupported );
            }
            else
            {
                bool bProcessed = false;
                try
                {
                    bProcessed = handler->process( request, response );
                }
                catch ( ... )
                {
                    ERROR_LOG( "Exception thrown by the %s handler.", (const char *)method );
                    bProcessed = false;
                }

                int nStatus = response->getStatusCode();
                if ( ! bProcessed || nStatus < 200 || nStatus > 699 )
                {
                    response->setStatus( 500 );
                }
            }
        }

        {
            CInternetAddress from( source );
            INFO_LOG( "f=%.32s cip=%s erid=%s uri=\"%.256s\" from=\"%.256s\" to=\"%.256s\" sc=%d",
                      (const char *)method,
                      (const char *)from.toStringWithPort(),
                      (const char *)erid.toString(),
                      (const char *)request->getUri(),
                      (const char *)request->getHeaderValue( "From" ),
                      (const char *)request->getHeaderValue( "To" ),
                      response->getStatusCode() );
        }

        if ( handler )
        {
            delete handler;
        }

        sendFinal( txn, response );
        delete response;
    }

    /**********************************************************************
     * ACK and CANCEL
     **********************************************************************/

    void CSipServer::handleAck( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &source )
    {
        std::string key = transactionKey( request, "INVITE" );

        {
            std::lock_guard<std::mutex> lock( m_transactionMutex );

            std::map< std::string, std::shared_ptr<CSipServerTransaction> >::iterator it = m_transactions.find( key );
            if ( it != m_transactions.end() && it->second->isInvite )
            {
                std::shared_ptr<CSipServerTransaction> txn = it->second;
                switch ( txn->state )
                {
                    case CSipServerTransaction::COMPLETED:
                            // The ACK for a non-2xx final response ends the
                            // retransmissions (17.2.1); Timer I absorbs any
                            // retransmitted ACKs.
                        txn->state = CSipServerTransaction::CONFIRMED;
                        txn->nextRetransmit = 0;
                        txn->expires = currentTimeMs() + ( txn->reliable ? 0 : m_nT4 );
                        return;

                    case CSipServerTransaction::CONFIRMED:
                    case CSipServerTransaction::PROCEEDING:
                        return;

                    case CSipServerTransaction::ACCEPTED:
                            // An ACK for a 2xx matched by RFC 2543 rules; handled
                            // as a 2xx ACK below.
                        break;
                }
            }

                // The ACK for a 2xx response stops its retransmission.
            unsigned long nCSeq = 0;
            CString cseqMethod;
            CSipUtils::parseCSeq( request->getHeaderValue( "CSeq" ), nCSeq, cseqMethod );
            m_pending2xx.erase( pendingKey( request->getHeaderValue( "Call-ID" ), nCSeq, request->getToTag() ) );
        }

            // Let the application see the ACK. Whatever the handler does with
            // the response, nothing is sent.
        CUUID erid;
        CSipHandler *handler = GetHandler( request, erid );
        if ( handler )
        {
            CSipResponse *response = createResponse( request, 200, NULL, CString( "" ) );
            try
            {
                handler->process( request, response );
            }
            catch ( ... )
            {
                ERROR_LOG( "Exception thrown by the ACK handler." );
            }
            delete response;
            delete handler;
        }
    }

    bool CSipServer::handleCancel( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &target )
    {
            // Called with the transaction mutex held. A CANCEL matches the
            // INVITE it cancels by the same rules as any other request, except
            // for the method (9.2).
        std::map< std::string, std::shared_ptr<CSipServerTransaction> >::iterator it = m_transactions.find( transactionKey( request, "INVITE" ) );
        if ( it == m_transactions.end() || ! it->second->isInvite )
        {
            return false;
        }

        std::shared_ptr<CSipServerTransaction> invite = it->second;

        std::shared_ptr<CSipServerTransaction> cancel = std::make_shared<CSipServerTransaction>();
        cancel->key = transactionKey( request, "CANCEL" );
        cancel->isInvite = false;
        cancel->transport = pTransport;
        cancel->reliable = pTransport->isReliable();
        cancel->target = target;
        cancel->toTag = invite->toTag;
        cancel->parts.fromRequest( request );
        m_transactions[ cancel->key ] = cancel;

        CSipResponse *ok = buildResponse( cancel->parts, 200, NULL, invite->toTag );
        sendFinalLocked( cancel, ok );
        delete ok;

        if ( invite->state == CSipServerTransaction::PROCEEDING )
        {
            CSipResponse *terminated = buildResponse( invite->parts, 487, NULL, invite->toTag );
            sendFinalLocked( invite, terminated );
            delete terminated;
        }

        return true;
    }

    /**********************************************************************
     * Responses
     **********************************************************************/

    CSipResponse *CSipServer::createResponse( CSipRequest *request, int nStatus, const char *reason, const CString &toTag )
    {
        CSipResponseParts parts;
        parts.fromRequest( request );
        return buildResponse( parts, nStatus, reason, toTag );
    }

    CSipResponse *CSipServer::buildResponse( const CSipResponseParts &parts, int nStatus, const char *reason, const CString &toTag )
    {
        CSipResponse *response = new CSipResponse();

        if ( reason && *reason )
            response->setStatus( nStatus, reason );
        else
            response->setStatus( nStatus );

        CHeaderList &headers = response->getHeaders();

            // Every Via, in order (8.2.6.2).
        for ( size_t nX = 0; nX < parts.vias.Count(); nX++ )
        {
            headers.appendHeader( "Via", parts.vias[ nX ] );
        }

        headers.addHeader( "From", parts.from );

        CString to = parts.to;
        if ( nStatus != 100 && toTag.GetLength() > 0 && CSipUtils::getTag( to ).GetLength() == 0 )
        {
            to = CSipUtils::setParameter( to, "tag", toTag );
        }
        headers.addHeader( "To", to );
        headers.addHeader( "Call-ID", parts.callId );
        headers.addHeader( "CSeq", parts.cseq );

        if ( nStatus == 100 && parts.timestamp.GetLength() > 0 )
        {
            headers.addHeader( "Timestamp", parts.timestamp );
        }

        if ( m_strServerName.GetLength() > 0 )
        {
            headers.addHeader( "Server", m_strServerName );
        }

        return response;
    }

    void CSipServer::sendStateless( CSipRequest *request, int nStatus, const char *reason, CSipTransport *pTransport, const CInternetAddress &target, const char *extraName, const char *extraValue )
    {
        CSipResponse *response = createResponse( request, nStatus, reason, CSipUtils::randomToken( 8 ) );
        if ( extraName && extraValue )
        {
            response->addHeader( extraName, extraValue );
        }
        pTransport->send( response->toString(), target );
        delete response;
    }

    void CSipServer::sendProvisionalLocked( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response )
    {
        CString data = response->toString();
        txn->lastResponse = data;
        txn->lastStatus = response->getStatusCode();
        if ( txn->transport )
        {
            txn->transport->send( data, txn->target );
        }
    }

    void CSipServer::sendFinal( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response )
    {
        std::lock_guard<std::mutex> lock( m_transactionMutex );

        if ( txn->state != CSipServerTransaction::PROCEEDING )
        {
                // Already finished, for example by a CANCEL while the handler ran.
            return;
        }
        sendFinalLocked( txn, response );
    }

    void CSipServer::sendFinalLocked( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response )
    {
        int nStatus = response->getStatusCode();

            // Responses that can create a dialog carry the Record-Route of the
            // request (12.1.1).
        if ( txn->isInvite && nStatus > 100 && nStatus < 300 && ! response->hasHeader( "Record-Route" ) &&
             txn->parts.recordRoutes.Count() > 0 )
        {
            response->addHeaders( "Record-Route", txn->parts.recordRoutes );
        }

        CString data = response->toString();
        txn->lastResponse = data;
        txn->lastStatus = nStatus;

        if ( txn->transport )
        {
            txn->transport->send( data, txn->target );
        }

        unsigned long long nNow = currentTimeMs();

        if ( txn->isInvite )
        {
            if ( nStatus >= 200 && nStatus < 300 )
            {
                    // The core retransmits a 2xx until its ACK arrives (13.3.1.4).
                std::shared_ptr<CSipPending2xx> pending = std::make_shared<CSipPending2xx>();
                pending->transport = txn->transport;
                pending->reliable = txn->reliable;
                pending->target = txn->target;
                pending->response = data;
                pending->retransmitInterval = m_nT1;
                pending->nextRetransmit = nNow + m_nT1;
                pending->expires = nNow + 64ULL * m_nT1;

                unsigned long nCSeq = 0;
                CString cseqMethod;
                CSipUtils::parseCSeq( txn->parts.cseq, nCSeq, cseqMethod );
                CString responseTag = CSipUtils::getTag( response->getHeaderValue( "To" ) );
                m_pending2xx[ pendingKey( txn->parts.callId, nCSeq, responseTag ) ] = pending;

                    // RFC 6026: keep the transaction to absorb retransmitted
                    // INVITEs until Timer L.
                txn->state = CSipServerTransaction::ACCEPTED;
                txn->nextRetransmit = 0;
                txn->expires = nNow + 64ULL * m_nT1;
            }
            else
            {
                    // Timer G (retransmit over UDP) and Timer H (wait for ACK).
                txn->state = CSipServerTransaction::COMPLETED;
                if ( ! txn->reliable )
                {
                    txn->retransmitInterval = m_nT1;
                    txn->nextRetransmit = nNow + m_nT1;
                }
                txn->expires = nNow + 64ULL * m_nT1;
            }
        }
        else
        {
                // Timer J: absorb retransmitted requests on UDP.
            txn->state = CSipServerTransaction::COMPLETED;
            txn->expires = nNow + ( txn->reliable ? 0 : 64ULL * m_nT1 );
        }
    }

    /**********************************************************************
     * Timers
     **********************************************************************/

    void CSipServer::processTimers( void )
    {
        processTimers( currentTimeMs() );
    }

    void CSipServer::processTimers( unsigned long long nNowMs )
    {
        std::lock_guard<std::mutex> lock( m_transactionMutex );

        std::map< std::string, std::shared_ptr<CSipServerTransaction> >::iterator it = m_transactions.begin();
        while ( it != m_transactions.end() )
        {
            std::shared_ptr<CSipServerTransaction> txn = it->second;
            bool bErase = false;

            if ( txn->state == CSipServerTransaction::COMPLETED && txn->isInvite &&
                 txn->nextRetransmit != 0 && nNowMs >= txn->nextRetransmit && nNowMs < txn->expires )
            {
                if ( txn->transport )
                {
                    txn->transport->send( txn->lastResponse, txn->target );
                }
                txn->retransmitInterval = ( txn->retransmitInterval * 2 > m_nT2 ) ? m_nT2 : txn->retransmitInterval * 2;
                txn->nextRetransmit = nNowMs + txn->retransmitInterval;
            }

            if ( txn->state != CSipServerTransaction::PROCEEDING && nNowMs >= txn->expires )
            {
                if ( txn->state == CSipServerTransaction::COMPLETED && txn->isInvite )
                {
                    WARN_LOG( "No ACK received for a %d response to INVITE (Call-ID %.128s).", txn->lastStatus, (const char *)txn->parts.callId );
                }
                bErase = true;
            }

            if ( bErase )
                it = m_transactions.erase( it );
            else
                ++it;
        }

        std::map< std::string, std::shared_ptr<CSipPending2xx> >::iterator pit = m_pending2xx.begin();
        while ( pit != m_pending2xx.end() )
        {
            std::shared_ptr<CSipPending2xx> pending = pit->second;

            if ( nNowMs >= pending->expires )
            {
                    // 13.3.1.4: the session should be ended with a BYE. This
                    // core keeps no dialogs, so that is left to the application.
                WARN_LOG( "No ACK received for a 2xx response to INVITE." );
                pit = m_pending2xx.erase( pit );
                continue;
            }

            if ( ! pending->reliable && nNowMs >= pending->nextRetransmit )
            {
                if ( pending->transport )
                {
                    pending->transport->send( pending->response, pending->target );
                }
                pending->retransmitInterval = ( pending->retransmitInterval * 2 > m_nT2 ) ? m_nT2 : pending->retransmitInterval * 2;
                pending->nextRetransmit = nNowMs + pending->retransmitInterval;
            }
            ++pit;
        }
    }

    void CSipServer::transportClosed( CSipTransport *pTransport )
    {
        std::lock_guard<std::mutex> lock( m_transactionMutex );

        for ( std::map< std::string, std::shared_ptr<CSipServerTransaction> >::iterator it = m_transactions.begin(); it != m_transactions.end(); ++it )
        {
            if ( it->second->transport == pTransport )
                it->second->transport = NULL;
        }
        for ( std::map< std::string, std::shared_ptr<CSipPending2xx> >::iterator it = m_pending2xx.begin(); it != m_pending2xx.end(); ++it )
        {
            if ( it->second->transport == pTransport )
                it->second->transport = NULL;
        }
    }

    /**********************************************************************
     * Transport details
     **********************************************************************/

    std::string CSipServer::transactionKey( CSipRequest *request, const char *method )
    {
        CString via = request->getTopVia();
        CString branch = CSipUtils::getParameter( via, "branch" );
        CString transport;
        CString host;
        int nPort = 0;
        CSipUtils::parseVia( via, transport, host, nPort );
        host.ToLowerCase();

        std::string sentBy = toStd( host ) + ":" + std::to_string( nPort );

            // 17.2.3: RFC 3261 branches are unique, so they (with the sent-by
            // and the method) identify the transaction.
        if ( strncmp( (const char *)branch, CSipUtils::MAGIC_COOKIE, strlen( CSipUtils::MAGIC_COOKIE ) ) == 0 )
        {
            return "3261|" + toStd( branch ) + "|" + sentBy + "|" + method;
        }

            // Older (RFC 2543) clients: match on the request's identifying
            // fields instead.
        unsigned long nCSeq = 0;
        CString cseqMethod;
        CSipUtils::parseCSeq( request->getHeaderValue( "CSeq" ), nCSeq, cseqMethod );

        return "2543|" + toStd( request->getUri() ) + "|" + toStd( request->getHeaderValue( "Call-ID" ) ) + "|" +
               toStd( request->getFromTag() ) + "|" + std::to_string( nCSeq ) + "|" + sentBy + "|" +
               toStd( branch ) + "|" + method;
    }

    void CSipServer::updateTopVia( CSipRequest *request, const CInternetAddress &source )
    {
        CString via = request->getTopVia();
        CString transport;
        CString host;
        int nPort = 0;

        if ( ! CSipUtils::parseVia( via, transport, host, nPort ) )
        {
            return;
        }

        CInternetAddress from( source );
        CString sourceIp = from.toString();
        int nSourcePort = CInternetAddress::getPort( from.GetAddress() );

        bool bRport = false;
        CSipUtils::getParameter( via, "rport", &bRport );

        bool bChanged = false;

            // 18.2.1: note where the request really came from.
        if ( host != sourceIp || bRport )
        {
            via = CSipUtils::setParameter( via, "received", sourceIp );
            bChanged = true;
        }
            // RFC 3581: a client that asks with "rport" wants the source port.
        if ( bRport && nSourcePort > 0 )
        {
            via = CSipUtils::setParameter( via, "rport", CString::FormatString( "%d", nSourcePort ) );
            bChanged = true;
        }

        if ( bChanged )
        {
            request->setTopVia( via );
        }
    }

    CInternetAddress CSipServer::responseTarget( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &source )
    {
            // 18.2.2: on a connection, responses go back on the connection.
        if ( pTransport->isReliable() )
        {
            return CInternetAddress( source );
        }

        CString via = request->getTopVia();
        CString transport;
        CString host;
        int nPort = 0;
        CSipUtils::parseVia( via, transport, host, nPort );

        CString received = CSipUtils::getParameter( via, "received" );
        CString targetHost = ( received.GetLength() > 0 ) ? received : host;

        bool bRport = false;
        CString rport = CSipUtils::getParameter( via, "rport", &bRport );

        int nTargetPort = nPort ? nPort : 5060;
        if ( bRport && rport.GetLength() > 0 )
        {
            nTargetPort = atoi( (const char *)rport );
        }

        CInternetAddress retVal( (const char *)targetHost, nTargetPort, SOCK_DGRAM );
        if ( ! retVal.isValid() )
        {
                // Fall back to the address the request came from.
            return CInternetAddress( source );
        }
        return retVal;
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
