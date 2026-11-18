/**
 * @file Mail.h
 * @brief A simple SMTP mail sender (CMail).
 *
 * EMail Base Class
 *
 *      This class offers a very simple interface for sending electronic
 * mail. It does this via the SMTP protocol (RFC 5321). Please be aware that
 * the mail server must be set before sending the e-mail.
 *
 * @code
 *          CMail::SetServer( "mail.example.com", 25 );
 *          CMail mail;
 *          mail.From( "app@example.com" );
 *          mail.AddRecipient( "Jo Bloggs <jo@example.com>" );
 *          mail.Subject( "Report" );
 *          mail.Body( "Line one\nLine two\n" );
 *          if ( ! mail.Send() ) ... mail.GetLastError() ...
 * @endcode
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

	/// Sends a plain-text e-mail through an SMTP server (RFC 5321).
	///
	/// Set the server once with SetServer(), then fill a CMail with a sender,
	/// recipients, a subject and a body and call Send(). Send() connects,
	/// greets with EHLO (HELO if EHLO is refused), checks every reply and
	/// stops at the first refusal, recording why in GetLastError(). A refused
	/// recipient stops the whole message: nothing is sent.
	///
	/// Addresses and the subject are checked when set; an address may be
	/// "mailbox" or "Display Name <mailbox>". Anything holding CR, LF or NUL is
	/// refused, so no setter can add SMTP commands or headers.
	///
	/// The server setting is shared by all CMail objects and guarded by a
	/// mutex, so SetServer() may be called while another thread sends. A
	/// single CMail object must be used from one thread at a time.
	/// @code
	/// CMail::SetServer( "mail.example.com", 25 );
	/// CMail mail;
	/// mail.From( "app@example.com" );
	/// mail.AddRecipient( "Jo Bloggs <jo@example.com>, ops@example.com" );
	/// mail.Subject( "Report" );
	/// mail.Body( "Line one\nLine two\n" );
	/// if ( ! mail.Send() )
	///     printf( "%s\n", (const char *)mail.GetLastError() );
	/// @endcode
	class CMail : public CObject
	{
		protected:
			static CString      m_strMailServer;        ///< The SMTP server for every CMail ("localhost" by default).
			static int          m_nMailServerPort;      ///< The SMTP port for every CMail (25 by default).

			CString             m_strRecipients;        ///< The To header: the recipients, comma-separated.
			CString             m_strFrom;              ///< The sender's address ("Unknown@unknown.com" until set).
			CString             m_strSubject;           ///< The subject ("<No Subject>" until set).
			CString             m_strMailBody;          ///< The body text ("<No Body>" until set).
            CStringArray        m_aRecipientAddresses;  ///< The mailbox of each recipient, for RCPT TO.
            CString             m_strHeloName;          ///< The name to greet the server with, or empty for the default.
            CString             m_strLastError;         ///< Why the last Send() failed, or empty.
            int                 m_nTimeoutMs;           ///< The connect and reply timeout in milliseconds (30000 by default).

		public:
                /// Sets the server (a name or address) and port used by every CMail.
                /// NULL is taken as an empty name.
			static void         SetServer( const char *strMailServer, int nPort );

								/// Creates a mail with placeholder sender, subject and body
								/// and no recipients.
								CMail( void );
								/// Destroys the mail.
			virtual            ~CMail( void );

								DEFINE_OBJECT( CMail );

                /// Sets the sender, as "mailbox" or "Display Name <mailbox>".
                /// @return false (and nothing changes) for an address that is
                ///         refused (see MailboxOf()).
                /// @note Any text used to be taken and sent in "MAIL FROM:<...>"
                ///       and the From header, so a CR LF in it could add SMTP
                ///       commands or headers.
			bool                From( const char *strFrom );
                /// Adds one recipient or a comma-separated list of them.
                ///
                /// Commas inside quotes or angle brackets do not split the list,
                /// so "\"Bloggs, Jo\" <jo@example.com>" is one address.
                /// @return false (and none are added) if any address is refused.
			bool                AddRecipient( const char *strTo );
                /// Removes all recipients.
            void                ClearRecipients( void );
                /// Sets the subject. A subject that is not ASCII is sent as an
                /// RFC 2047 encoded word.
                /// @return false (and nothing changes) for a subject holding CR,
                ///         LF or NUL.
			bool                Subject( const char *strSubject );
                /// Sets the body text. NULL is taken as an empty body. Line ends
                /// are made CRLF when the message is built (see NormalizeBody()).
			void                Body( const char *strBodyText ) { m_strMailBody = strBodyText ? strBodyText : ""; }

                /// Sets the name to greet the server with.
                ///
                /// The default (an empty name) is this machine's address on the
                /// connection, as an address literal such as "[192.0.2.1]".
                /// @return false (and nothing changes) for a name holding a
                ///         space or a control character.
            bool                HeloName( const char *strName );
                /// Sets the limit on connecting and on each wait for a reply, in
                /// milliseconds (default 30 seconds; 0 or less waits without limit).
            void                SetTimeout( int nMilliseconds ) { m_nTimeoutMs = nMilliseconds; }

                /// Sends the mail.
                ///
                /// On success the recipient list is cleared. On failure the
                /// session is ended (QUIT) where possible and GetLastError() says
                /// why: no valid sender, no recipients, no connection, or the
                /// server's refusal with its code and text.
                /// @return true if the server accepted the message.
			bool                Send( void );
                /// Why the last Send() failed ("" if it did not).
            CString             GetLastError( void ) { return m_strLastError; }

                /// The message as it is sent after DATA.
                ///
                /// It holds From, To, Subject, Date, Message-ID, MIME-Version,
                /// Content-Type and Content-Transfer-Encoding headers, a blank
                /// line and the body with CRLF line ends and dot-stuffing,
                /// ending in the "." line.
                /// @param strHost Used as the Message-ID domain when the sender's
                ///                address has none ("localhost" if this is empty too).
                /// @note A `"\r\n"` used to come before the "." line as well, so
                ///       every message ended with an extra blank line.
            CString             BuildMessage( const CString &strHost );

                /// The mailbox of an address ("Name <a@b>" -> "a@b"), or "" if
                /// it is not an acceptable address.
                ///
                /// A mailbox is local\@domain, with both parts non-empty, and no
                /// spaces, control characters or any of `<>(),;\"`. Nothing may
                /// follow the closing '>'.
            static CString      MailboxOf( const CString &strAddress );
                /// The body with its line ends made CRLF and lines starting with
                /// '.' doubled, ending in CRLF.
                ///
                /// CRLF, LF and a lone CR each count as a line end, and blank
                /// lines are kept. An empty body stays empty.
                /// @note Lines used to be split on LF with empty ones dropped, so
                ///       blank lines vanished, and a CR before each LF was kept
                ///       and then doubled.
            static CString      NormalizeBody( const CString &strBody );

        protected:
                /// Reads one reply, which may be several lines ("250-..." up to
                /// "250 ...").
                /// @param pSocket   The connection.
                /// @param strBuffer Bytes read but not yet used; kept between calls.
                /// @param strText   Receives the reply text, one line per reply
                ///                  line, without the codes.
                /// @return The reply code, or 0 on a timeout, a closed connection
                ///         or a malformed reply.
                /// @note Only one line used to be read, so the rest of a
                ///       multi-line reply was taken as the reply to the next
                ///       command.
            int                 ReadReply( CSocket *pSocket, std::string &strBuffer, CString &strText );
                /// Sends a command and reads its reply.
                /// @return The reply code, or 0 if the command could not be sent
                ///         or no valid reply came (see ReadReply()).
            int                 Command( CSocket *pSocket, std::string &strBuffer, const CString &strCommand, CString &strText );
	};
} // namespace IASLib

#endif // IASLIB_MAIL_H__

#endif // IASLIB_NETWORKING__
