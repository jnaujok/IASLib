/**
 * @file SipServer.h
 * @brief The protocol core of a SIP user agent server (CSipServer).
 *
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
 *          scheme gives 416 (so does a sips: Request-URI that did not
 *          arrive over TLS, unless setRequireTlsForSips( false )) and an
 *          unsupported Require option gives 420;
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

    /// The protocol core of a SIP user agent server (UAS), RFC 3261.
    ///
    /// It owns no sockets and its Run() does nothing: a concrete server (see
    /// CBasicSipServer) receives messages from its transports, passes each
    /// complete message to handleMessage(), and calls processTimers()
    /// regularly. The core checks each request, runs the server transactions
    /// (retransmissions, CANCEL, ACK), builds the response and hands it to the
    /// method's handler; see the file description for the full list.
    ///
    /// The server owns its handler factories. Add factories and change the
    /// settings before the server starts. handleMessage(), processTimers() and
    /// transportClosed() may then be called from several threads at once; the
    /// transaction tables are guarded by a mutex. Received responses are
    /// ignored, as the core has no client transactions.
    /// @code
    /// CBasicSipServer oServer( 5060 );
    /// oServer.AddHandlerFactory( new CBaseSipHandlerFactory() );
    /// oServer.setServerName( "MyPbx/1.0" );
    /// oServer.Start();
    /// @endcode
    class CSipServer : public CGenericServer
    {
        protected:
            bool            m_bSecure;              ///< Whether the server is meant to serve TLS.
            bool            m_bIsUDP;               ///< Whether the server is meant to serve UDP.
            int             m_nPort;                ///< The port for a derived server to bind.
            CString         m_strBindAddress;       ///< The interface for a derived server to bind, or "".
            CArray          m_aHandlerFactories;    ///< The CSipHandlerFactory objects (owned), newest first.

            CStringArray    m_aSupported;           ///< The option tags this server supports.
            CString         m_strRealm;             ///< The authentication realm ("IASLib" by default).
            CString         m_strServerName;        ///< The Server header value ("IASLib/2.0" by default; "" for none).
            bool            m_bSend100Trying;       ///< Whether INVITE gets 100 Trying first (true by default).
            bool            m_bRequireTlsForSips;   ///< Whether a sips: Request-URI must arrive over TLS (true by default).

            unsigned long   m_nT1;                  ///< Timer T1, the round-trip estimate, in ms (500 by default).
            unsigned long   m_nT2;                  ///< Timer T2, the longest retransmit interval, in ms (4000 by default).
            unsigned long   m_nT4;                  ///< Timer T4, the longest time a message stays in the network, in ms (5000 by default).

            std::mutex      m_transactionMutex;     ///< Guards m_transactions and m_pending2xx.
                /// The live server transactions, by transaction key.
            std::map< std::string, std::shared_ptr<CSipServerTransaction> > m_transactions;
                /// The 2xx responses to INVITE waiting for their ACK.
            std::map< std::string, std::shared_ptr<CSipPending2xx> > m_pending2xx;

        public:
                /// Creates the server core.
                ///
                /// The port, TLS and UDP flags are kept for derived servers that
                /// bind sockets; the core itself does not use them.
                /// @param nSipPort The port to serve.
                /// @param bSecure  Whether to serve TLS.
                /// @param useUDP   Whether to serve UDP.
	                        CSipServer( int nSipPort=5060, bool bSecure = false, bool useUDP = false );
	                        /// Same, also keeping the interface to bind to.
	                        /// @param boundInterface The local address to bind; NULL or "" for all.
	                        /// @param nSipPort       The port to serve.
	                        /// @param bSecure        Whether to serve TLS.
	                        /// @param useUDP         Whether to serve UDP.
	                        CSipServer( const char *boundInterface, int nSipPort=5060, bool bSecure = false, bool useUDP = false );

	            /// Deletes the handler factories.
	        virtual        ~CSipServer( void );

                            DEFINE_OBJECT( CSipServer )

                /// The base server does nothing when run and returns NULL.
                /// Concrete servers override this to receive messages from their
                /// transports.
            virtual void   *Run( void );

                /// Adds a handler factory.
                ///
                /// Factories are consulted in the reverse order they were added
                /// (the newest first). The server takes ownership. NULL is
                /// ignored.
            virtual void    AddHandlerFactory( CSipHandlerFactory *pFactory );
                /// Removes the factory and returns ownership of it to the caller.
                /// Does nothing if the factory was not added.
            virtual void    RemoveHandlerFactory( CSipHandlerFactory *pFactory );
                /// Returns a new handler for the request, or NULL.
                ///
                /// The first handler a factory offers is given this server and
                /// returned. The caller owns the handler.
            virtual CSipHandler   *GetHandler( CSipRequest *request, CUUID erid );

                /// The methods of all factories, without repeats, for an Allow
                /// header ("INVITE, ACK, BYE").
            virtual CString getAllowedList( void );

                /// Adds an option tag this server supports (RFC 3261 19.2).
                ///
                /// Supported tags are listed in the Supported header and let
                /// requests that Require them through. The tag is trimmed;
                /// empty tags and repeats (ignoring case) are ignored.
            virtual void    addSupportedExtension( const char *optionTag );
                /// Whether the option tag is supported (ignoring case).
            virtual bool    isSupportedExtension( const CString &optionTag );
                /// The supported option tags, comma-separated, or "".
            virtual CString getSupportedList( void );

                /// Sets the realm used in authentication challenges.
            virtual void    setRealm( const char *realm ) { m_strRealm = realm; }
                /// The realm used in authentication challenges ("IASLib" by default).
            virtual CString getRealm( void ) { return m_strRealm; }

                /// Sets the value of the Server header added to responses, or ""
                /// for none.
            virtual void    setServerName( const char *name ) { m_strServerName = name; }
                /// The value of the Server header ("IASLib/2.0" by default).
            virtual CString getServerName( void ) { return m_strServerName; }

                /// Sets whether INVITE is answered with 100 Trying before its
                /// handler runs (on by default; RFC 3261 17.2.1).
            virtual void    setSend100Trying( bool bSend ) { m_bSend100Trying = bSend; }

                /// Sets whether a sips: Request-URI must arrive over TLS (on by
                /// default).
                ///
                /// A sips: Request-URI asks for TLS on every hop (RFC 3261 26.2,
                /// RFC 5630), so one that arrives over UDP or TCP is refused with
                /// 416. Turn off for a server behind a proxy that terminates TLS
                /// for it.
            virtual void    setRequireTlsForSips( bool bRequire ) { m_bRequireTlsForSips = bRequire; }
                /// Whether a sips: Request-URI must arrive over TLS.
            virtual bool    getRequireTlsForSips( void ) { return m_bRequireTlsForSips; }

                /// Sets the RFC 3261 timer values in milliseconds (defaults 500,
                /// 4000 and 5000).
                ///
                /// A T1 of 0 is taken as 500, and a T2 below T1 as T1.
            virtual void    setTimerValues( unsigned long nT1, unsigned long nT2, unsigned long nT4 );

                /// Whether the server was created to serve UDP.
            virtual bool    isUdp( void ) { return m_bIsUDP; }
                /// Whether the server was created to serve TLS.
            virtual bool    isSecure( void ) { return m_bSecure; }
                /// The port given at construction.
            virtual int     getPort( void ) { return m_nPort; }
                /// The interface given at construction, or "".
            virtual CString getBindAddress( void ) { return m_strBindAddress; }

                /// Processes one complete message received on a transport from
                /// the given address.
                ///
                /// A message of only line ends is a keep-alive; on a reliable
                /// transport a double-CRLF ping gets a CRLF pong (RFC 5626
                /// 4.4.1). Responses are ignored. Exceptions do not escape.
                /// Thread-safe: concrete servers may call it from several
                /// threads at once.
                /// @param message    The whole message.
                /// @param pTransport The transport it arrived on, used for the
                ///                   answers (NULL: the message is ignored).
                /// @param source     The sender's address.
            virtual void    handleMessage( const CString &message, CSipTransport *pTransport, const CInternetAddress &source );

                /// Runs transaction timers: retransmissions and clean-up, at the
                /// current time. Call this every 50-100 ms or so.
            virtual void    processTimers( void );
                /// Same, at the given time (from the same clock as
                /// currentTimeMs()), for tests.
            virtual void    processTimers( unsigned long long nNowMs );

                /// Tells the core that a transport is going away, so that it is
                /// no longer used for retransmissions.
            virtual void    transportClosed( CSipTransport *pTransport );

                /// The number of live server transactions (for monitoring and tests).
            virtual size_t  getTransactionCount( void );

                /// Builds a response to the request with the headers RFC 3261
                /// 8.2.6 requires.
                ///
                /// It copies every Via in order, From, To, Call-ID and CSeq (and
                /// Timestamp for 100), and adds Server if a name is set. The To
                /// tag is added unless the status is 100 or the request's To
                /// already has a tag.
                /// @param request The request being answered.
                /// @param nStatus The status code.
                /// @param reason  The reason phrase; NULL or "" for the standard one.
                /// @param toTag   The tag to put on To.
                /// @return A new response, owned by the caller.
            virtual CSipResponse *createResponse( CSipRequest *request, int nStatus, const char *reason, const CString &toTag );

        protected:
                /// The clock used by the transaction timers, in milliseconds
                /// (a steady clock). Tests override this to control time.
            virtual unsigned long long currentTimeMs( void );

                /// Processes one parsed request: the checks, transactions and
                /// handler call described for the class.
                /// @param request    The request.
                /// @param parseOk    false if the request was malformed.
                /// @param pTransport The transport it arrived on.
                /// @param source     The sender's address.
            virtual void    processRequest( CSipRequest *request, bool parseOk, CSipTransport *pTransport, const CInternetAddress &source );

        private:
                /// Matches an ACK to its INVITE transaction or pending 2xx, and
                /// lets the handler see it; nothing is sent.
            void            handleAck( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &source );
                /// Answers a CANCEL (with the transaction mutex held).
                /// @return false if there is no INVITE for it to cancel.
            bool            handleCancel( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &target );
                /// Sends a response outside any transaction, with an optional
                /// extra header.
            void            sendStateless( CSipRequest *request, int nStatus, const char *reason, CSipTransport *pTransport, const CInternetAddress &target, const char *extraName = NULL, const char *extraValue = NULL );
                /// Sends the final response of a transaction, taking the mutex.
            void            sendFinal( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response );
                /// Sends the final response of a transaction and starts its
                /// timers (with the transaction mutex held).
            void            sendFinalLocked( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response );
                /// Sends a provisional response of a transaction (with the
                /// transaction mutex held).
            void            sendProvisionalLocked( std::shared_ptr<CSipServerTransaction> txn, CSipResponse *response );
                /// Builds a response from the saved parts of a request; see
                /// createResponse(). The caller owns the result.
            CSipResponse   *buildResponse( const CSipResponseParts &parts, int nStatus, const char *reason, const CString &toTag );
                /// The key that identifies a request's transaction (RFC 3261
                /// 17.2.3, with the RFC 2543 fallback).
            std::string     transactionKey( CSipRequest *request, const char *method );
                /// Adds the received and rport parameters to the top Via
                /// (RFC 3261 18.2.1, RFC 3581).
            void            updateTopVia( CSipRequest *request, const CInternetAddress &source );
                /// Where responses to the request go (RFC 3261 18.2.2).
            CInternetAddress responseTarget( CSipRequest *request, CSipTransport *pTransport, const CInternetAddress &source );
    };
} // namespace IASLib

#endif // IASLIB_SIPSERVER_H__

#endif // IASLIB_NETWORKING__
