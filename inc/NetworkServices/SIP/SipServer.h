/**
 *  SIP Server Class
 *
 *      This class is the protocol core of a SIP user agent server (UAS)
 * as described by RFC 3261. It does not own any sockets and its Run()
 * method is an empty stub: a concrete server (see CBasicSipServer) derives
 * from it, receives messages from its transports and passes each complete
 * message to handleMessage(), and calls processTimers() regularly.
 *
 *      For every request the core:
 *        - checks the request (section 8.2): a malformed request or a
 *          missing To, From, Call-ID, CSeq or Via gives 400, another SIP
 *          version gives 505, an unknown method gives 501 and a known one
 *          with no handler gives 405 (both with Allow), an unsupported URI
 *          scheme gives 416 and an unsupported Require option gives 420;
 *        - adds the "received" and "rport" Via parameters (section 18.2.1,
 *          RFC 3581) and sends responses where section 18.2.2 says to;
 *        - runs the server transactions of section 17.2: retransmitted
 *          requests get the last response again, INVITE gets 100 Trying,
 *          final INVITE responses are retransmitted over UDP until the ACK
 *          arrives (Timers G, H and I), non-INVITE transactions are kept
 *          for Timer J, ACKs for non-2xx responses are absorbed, and 2xx
 *          responses to INVITE are retransmitted until their ACK arrives
 *          (section 13.3.1.4);
 *        - answers CANCEL (section 9.2): 200 for the CANCEL and 487 for the
 *          INVITE it cancels, or 481 if there is no such INVITE;
 *        - detects merged requests (section 8.2.2.2) with 482;
 *        - builds each response with the headers section 8.2.6 requires,
 *          including a To tag, and hands it to the method's handler.
 *
 *      Handlers come from the handler factories added with
 *  AddHandlerFactory(). The server owns its factories.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: December 21, 2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPSERVER_H__
#define IASLIB_SIPSERVER_H__

#include "Collections/Array.h"
#include "Collections/StringArray.h"
#include "Sockets/ServerSocket.h"
#include "NetworkServices/GenericServer.h"
#include "NetworkServices/SIP/SipTransport.h"
#include "NetworkServices/SIP/Handlers/SipHandler.h"
#include "NetworkServices/SIP/Handlers/SipHandlerFactory.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>

namespace IASLib
{
    struct CSipServerTransaction;
    struct CSipPending2xx;
    struct CSipResponseParts;

    class CSipServer : public CGenericServer
    {
        protected:
            bool            m_bSecure;
            bool            m_bIsUDP;
            int             m_nPort;
            CString         m_strBindAddress;
            CArray          m_aHandlerFactories;

            CStringArray    m_aSupported;
            CString         m_strRealm;
            CString         m_strServerName;
            bool            m_bSend100Trying;

            unsigned long   m_nT1;
            unsigned long   m_nT2;
            unsigned long   m_nT4;

            std::mutex      m_transactionMutex;
            std::map< std::string, std::shared_ptr<CSipServerTransaction> > m_transactions;
            std::map< std::string, std::shared_ptr<CSipPending2xx> > m_pending2xx;

        public:
                // Port and interface are kept for derived servers that bind
                // sockets; the core itself does not use them.
	                        CSipServer( int nSipPort=5060, bool bSecure = false, bool useUDP = false );
	                        CSipServer( const char *boundInterface, int nSipPort=5060, bool bSecure = false, bool useUDP = false );

	        virtual        ~CSipServer( void );

                            DEFINE_OBJECT( CSipServer )

                // The base server does nothing when run. Concrete servers
                // override this to receive messages from their transports.
            virtual void   *Run( void );

                // Factories are consulted in the reverse order they were added
                // (the newest first). The server takes ownership.
            virtual void    AddHandlerFactory( CSipHandlerFactory *pFactory );
                // Removes the factory and returns ownership of it to the caller.
            virtual void    RemoveHandlerFactory( CSipHandlerFactory *pFactory );
                // Returns a new handler for the request, or NULL. The caller
                // owns the handler.
            virtual CSipHandler   *GetHandler( CSipRequest *request, CUUID erid );

                // The methods of all factories, for an Allow header.
            virtual CString getAllowedList( void );

                // Option tags this server supports (RFC 3261 19.2), used for the
                // Supported header and to accept requests that Require them.
            virtual void    addSupportedExtension( const char *optionTag );
            virtual bool    isSupportedExtension( const CString &optionTag );
            virtual CString getSupportedList( void );

                // The realm used in authentication challenges.
            virtual void    setRealm( const char *realm ) { m_strRealm = realm; }
            virtual CString getRealm( void ) { return m_strRealm; }

                // The value of the Server header added to responses, or "" for none.
            virtual void    setServerName( const char *name ) { m_strServerName = name; }
            virtual CString getServerName( void ) { return m_strServerName; }

                // Whether INVITE is answered with 100 Trying before its handler
                // runs (on by default; RFC 3261 17.2.1).
            virtual void    setSend100Trying( bool bSend ) { m_bSend100Trying = bSend; }

                // The RFC 3261 timer values in milliseconds (defaults 500, 4000
                // and 5000).
            virtual void    setTimerValues( unsigned long nT1, unsigned long nT2, unsigned long nT4 );

            virtual bool    isUdp( void ) { return m_bIsUDP; }
            virtual bool    isSecure( void ) { return m_bSecure; }
            virtual int     getPort( void ) { return m_nPort; }
            virtual CString getBindAddress( void ) { return m_strBindAddress; }

                // Processes one complete message received on a transport from
                // the given address. Thread-safe: concrete servers may call it
                // from several threads at once.
            virtual void    handleMessage( const CString &message, CSipTransport *pTransport, const CInternetAddress &source );

                // Runs transaction timers: retransmissions and clean-up. Call
                // this every 50-100 ms or so.
            virtual void    processTimers( void );
            virtual void    processTimers( unsigned long long nNowMs );

                // Tells the core that a transport is going away, so that it is
                // no longer used for retransmissions.
            virtual void    transportClosed( CSipTransport *pTransport );

                // The number of live server transactions (for monitoring and tests).
            virtual size_t  getTransactionCount( void );

                // Builds a response to the request with the headers RFC 3261
                // 8.2.6 requires. The To tag is added unless the status is 100 or
                // the request's To already has a tag. The caller owns the result.
            virtual CSipResponse *createResponse( CSipRequest *request, int nStatus, const char *reason, const CString &toTag );

        protected:
                // The clock used by the transaction timers, in milliseconds.
                // Tests override this to control time.
            virtual unsigned long long currentTimeMs( void );

                // Processes one parsed request. parseOk is false if the request
                // was malformed.
            virtual void    processRequest( CSipRequest *request, bool parseOk, CSipTransport *pTransport, const CInternetAddress &source );

        private:
            void            handleAck( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &source );
            bool            handleCancel( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &target );
            void            sendStateless( CSipRequest *request, int nStatus, const char *reason, CSipTransport *pTransport, const CInternetAddress &target, const char *extraName = NULL, const char *extraValue = NULL );
            void            sendFinal( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response );
            void            sendFinalLocked( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response );
            void            sendProvisionalLocked( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response );
            CSipResponse   *buildResponse( const CSipResponseParts &parts, int nStatus, const char *reason, const CString &toTag );
            std::string     transactionKey( CSipRequest *request, const char *method );
            void            updateTopVia( CSipRequest *request, const CInternetAddress &source );
            CInternetAddress responseTarget( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &source );
    };
} // namespace IASLib

#endif // IASLIB_SIPSERVER_H__

#endif // IASLIB_NETWORKING__
