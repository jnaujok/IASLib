/*
 * EMail Base Class
 *
 *      This class offers a very simple interface for sending electronic
 * mail. It does this via the SMTP protocol (RFC 5321). Please be aware that
 * the mail server must be set before sending the e-mail.
 *
 *          CMail::SetServer( "mail.example.com", 25 );
 *          CMail mail;
 *          mail.From( "app@example.com" );
 *          mail.AddRecipient( "Jo Bloggs <jo@example.com>" );
 *          mail.Subject( "Report" );
 *          mail.Body( "Line one\nLine two\n" );
 *          if ( ! mail.Send() ) ... mail.GetLastError() ...
 *
 *      Send() greets the server with EHLO (HELO if EHLO is refused), checks
 * every reply and stops at the first refusal (a refused recipient included:
 * then nothing is sent). The message gets Date, Message-ID, MIME-Version
 * and Content-Type (text/plain; charset=UTF-8) headers; its body keeps its
 * blank lines, has its line endings made CRLF, and lines that start with a
 * '.' are doubled (RFC 5321 4.5.2) so that one cannot end the message.
 *
 *      Addresses, the subject and the body are checked: an address or
 * subject holding CR, LF or NUL (which could add SMTP commands or headers),
 * or an address that is not a plain mailbox, is refused and the setter
 * returns false. An address may be "mailbox" or "Display Name <mailbox>",
 * and AddRecipient() also takes a comma-separated list of them.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/15/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_MAIL_H__
#define IASLIB_MAIL_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/Bool.h"
#include "BaseTypes/String_.h"
#include "Collections/StringArray.h"

#include <string>

namespace IASLib
{
    class CSocket;

	class CMail : public CObject
	{
		protected:
			static CString      m_strMailServer;
			static int          m_nMailServerPort;

			CString             m_strRecipients;        // the To header: the recipients, comma-separated
			CString             m_strFrom;
			CString             m_strSubject;
			CString             m_strMailBody;
            CStringArray        m_aRecipientAddresses;  // the mailbox of each recipient
            CString             m_strHeloName;
            CString             m_strLastError;
            int                 m_nTimeoutMs;

		public:
                // The server (a name or address) and port used by every CMail.
			static void         SetServer( const char *strMailServer, int nPort );

								CMail( void );
			virtual            ~CMail( void );

								DEFINE_OBJECT( CMail );

                // False (and nothing changes) for an address that is refused
                // (see above).
			bool                From( const char *strFrom );
			bool                AddRecipient( const char *strTo );
            void                ClearRecipients( void );
                // False (and nothing changes) for a subject holding CR, LF or
                // NUL.
			bool                Subject( const char *strSubject );
			void                Body( const char *strBodyText ) { m_strMailBody = strBodyText ? strBodyText : ""; }

                // The name to greet the server with (default: this machine's
                // address, as an address literal such as "[192.0.2.1]").
            bool                HeloName( const char *strName );
                // The limit on connecting and on each wait for a reply, in
                // milliseconds (default 30 seconds).
            void                SetTimeout( int nMilliseconds ) { m_nTimeoutMs = nMilliseconds; }

                // Sends the mail. On success the recipient list is cleared.
			bool                Send( void );
                // Why the last Send() failed ("" if it did not).
            CString             GetLastError( void ) { return m_strLastError; }

                // The message as it is sent after DATA: headers, a blank line
                // and the body with CRLF line ends and dot-stuffing, ending in
                // the "." line. strHost is used in the Message-ID.
            CString             BuildMessage( const CString &strHost );

                // The mailbox of an address ("Name <a@b>" -> "a@b"), or "" if
                // it is not an acceptable address.
            static CString      MailboxOf( const CString &strAddress );
                // The body with its line ends made CRLF and lines starting with
                // '.' doubled, ending in CRLF.
            static CString      NormalizeBody( const CString &strBody );

        protected:
                // Reads one reply, which may be several lines ("250-..." up to
                // "250 ..."). Returns the code, or 0 on a timeout, a closed
                // connection or a malformed reply.
            int                 ReadReply( CSocket *pSocket, std::string &strBuffer, CString &strText );
                // Sends a command and reads its reply.
            int                 Command( CSocket *pSocket, std::string &strBuffer, const CString &strCommand, CString &strText );
	};
} // namespace IASLib

#endif // IASLIB_MAIL_H__

#endif // IASLIB_NETWORKING__
