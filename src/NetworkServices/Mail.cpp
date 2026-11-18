/***********************************************************************
**  Mail Class
**
**  Description:
**      This class wraps the sending of mail through an SMTP server.
** Please note that the SMTP server must be set-up prior to calling the
** send method of the class. See Mail.h.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/Mail.h"
#include "Sockets/ClientSocket.h"
#include "Sockets/SocketConfig.h"

#include <atomic>
#include <cctype>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <ctime>
#include <mutex>
#include <random>
#include <vector>

namespace IASLib
{
	CString CMail::m_strMailServer = "localhost";
	int     CMail::m_nMailServerPort = 25;

	IMPLEMENT_OBJECT( CMail, CObject );

    namespace
    {
            // SetServer() may be called while another thread sends.
        std::mutex &serverMutex( void )
        {
            static std::mutex mutex;
            return mutex;
        }

        bool hasLineBreakOrNul( const CString &strText )
        {
            const char *pText = (const char *)strText;
            for ( size_t nX = 0; nX < strText.GetLength(); nX++ )
            {
                if ( pText[ nX ] == '\r' || pText[ nX ] == '\n' || pText[ nX ] == '\0' )
                    return true;
            }
            return false;
        }

        bool isAscii( const CString &strText )
        {
            const char *pText = (const char *)strText;
            for ( size_t nX = 0; nX < strText.GetLength(); nX++ )
            {
                if ( (unsigned char)pText[ nX ] >= 0x80 )
                    return false;
            }
            return true;
        }

        CString trimmed( const std::string &strText )
        {
            CString strResult( strText.c_str() );
            strResult.Trim();
            return strResult;
        }

            // Splits an address list on the commas that are not inside quotes
            // or angle brackets ("Bloggs, Jo" <jo@example.com> is one).
        std::vector<std::string> splitAddresses( const char *pszList )
        {
            std::vector<std::string> aResult;
            std::string strCurrent;
            bool bQuoted = false;
            bool bAngle = false;
            for ( const char *p = pszList; p && *p; p++ )
            {
                if ( *p == '"' && ! bAngle )
                    bQuoted = ! bQuoted;
                else if ( *p == '<' && ! bQuoted )
                    bAngle = true;
                else if ( *p == '>' && ! bQuoted )
                    bAngle = false;
                if ( *p == ',' && ! bQuoted && ! bAngle )
                {
                    aResult.push_back( strCurrent );
                    strCurrent.clear();
                    continue;
                }
                strCurrent += *p;
            }
            aResult.push_back( strCurrent );
            return aResult;
        }

        const char *BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string base64( const char *pData, size_t nLength )
        {
            std::string strResult;
            for ( size_t nX = 0; nX < nLength; nX += 3 )
            {
                unsigned long ulBits = (unsigned long)(unsigned char)pData[ nX ] << 16;
                if ( nX + 1 < nLength )
                    ulBits |= (unsigned long)(unsigned char)pData[ nX + 1 ] << 8;
                if ( nX + 2 < nLength )
                    ulBits |= (unsigned long)(unsigned char)pData[ nX + 2 ];
                strResult += BASE64[ ( ulBits >> 18 ) & 0x3F ];
                strResult += BASE64[ ( ulBits >> 12 ) & 0x3F ];
                strResult += ( nX + 1 < nLength ) ? BASE64[ ( ulBits >> 6 ) & 0x3F ] : '=';
                strResult += ( nX + 2 < nLength ) ? BASE64[ ulBits & 0x3F ] : '=';
            }
            return strResult;
        }

            // The date as RFC 5322 3.3 writes it, in UTC: "Wed, 18 Nov 2026
            // 09:30:00 +0000". (Names are written here, not by strftime(),
            // which follows the locale.)
        CString mailDate( void )
        {
            static const char *aDays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
            static const char *aMonths[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
            time_t tNow = time( NULL );
            struct tm tmNow;
#ifdef IASLIB_WIN32__
            gmtime_s( &tmNow, &tNow );
#else
            gmtime_r( &tNow, &tmNow );
#endif
            return CString::FormatString( "%s, %02d %s %04d %02d:%02d:%02d +0000", aDays[ tmNow.tm_wday % 7 ], tmNow.tm_mday,
                                          aMonths[ tmNow.tm_mon % 12 ], tmNow.tm_year + 1900, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec );
        }

        CString messageId( const CString &strDomain )
        {
            static std::atomic<unsigned long> ulSequence( 0 );
            std::random_device device;
            unsigned long long ullNow = (unsigned long long)std::chrono::duration_cast<std::chrono::microseconds>(
                                            std::chrono::system_clock::now().time_since_epoch() ).count();
            return CString::FormatString( "<%llx.%lx.%08x@%s>", ullNow, (unsigned long)ulSequence++, (unsigned int)device(),
                                          (const char *)strDomain );
        }
    }

	CMail::CMail( void )
	{
		m_strRecipients = "";
		m_strFrom = "Unknown@unknown.com";
		m_strSubject = "<No Subject>";
		m_strMailBody = "<No Body>";
        m_nTimeoutMs = 30000;
	}

	CMail::~CMail( void )
	{
		// Do Nothing
	}

	void CMail::SetServer( const char *strMailServer, int nPort )
	{
        std::lock_guard<std::mutex> lock( serverMutex() );
		m_strMailServer = strMailServer ? strMailServer : "";
		m_nMailServerPort = nPort;
	}

    CString CMail::MailboxOf( const CString &strAddress )
    {
        if ( hasLineBreakOrNul( strAddress ) )
            return CString( "" );

        std::string strText( (const char *)strAddress );
        std::string strMailbox = strText;
        size_t nOpen = strText.rfind( '<' );
        if ( nOpen != std::string::npos )
        {
                // "Display Name <mailbox>": nothing may follow the '>'.
            size_t nClose = strText.find( '>', nOpen );
            if ( nClose == std::string::npos || trimmed( strText.substr( nClose + 1 ) ).GetLength() > 0 )
                return CString( "" );
            strMailbox = strText.substr( nOpen + 1, nClose - nOpen - 1 );
        }

        CString strResult = trimmed( strMailbox );
            // A mailbox is local@domain without spaces, controls or the
            // characters that delimit addresses and SMTP parameters.
        const char *pText = (const char *)strResult;
        size_t nAt = strResult.LastIndexOf( '@' );
        if ( nAt == NOT_FOUND || nAt == 0 || nAt + 1 >= strResult.GetLength() )
            return CString( "" );
        for ( size_t nX = 0; nX < strResult.GetLength(); nX++ )
        {
            unsigned char uch = (unsigned char)pText[ nX ];
            if ( uch <= ' ' || uch == 0x7F || strchr( "<>(),;\\\"", uch ) != NULL )
                return CString( "" );
        }
        return strResult;
    }

    bool CMail::From( const char *strFrom )
    {
            // (Any text was used, in "MAIL FROM:<...>" and the From header, so
            // a CR LF in it could add SMTP commands or headers.)
        CString strAddress( strFrom ? strFrom : "" );
        strAddress.Trim();
        if ( MailboxOf( strAddress ).GetLength() == 0 )
            return false;
        m_strFrom = strAddress;
        return true;
    }

	bool CMail::AddRecipient( const char *strTo )
	{
            // Check them all before adding any.
        std::vector<std::string> aAddresses = splitAddresses( strTo );
        std::vector<CString> aMailboxes;
        for ( size_t nX = 0; nX < aAddresses.size(); nX++ )
        {
            CString strMailbox = MailboxOf( CString( aAddresses[ nX ].c_str() ) );
            if ( strMailbox.GetLength() == 0 )
                return false;
            aMailboxes.push_back( strMailbox );
        }

        for ( size_t nX = 0; nX < aAddresses.size(); nX++ )
        {
            if ( m_strRecipients.GetLength() )
            {
                m_strRecipients += ", ";
            }
            m_strRecipients += trimmed( aAddresses[ nX ] );
            m_aRecipientAddresses.Append( aMailboxes[ nX ] );
        }
        return true;
	}

    void CMail::ClearRecipients( void )
    {
        m_strRecipients = "";
        m_aRecipientAddresses.EmptyAll();
    }

    bool CMail::Subject( const char *strSubject )
    {
        CString strText( strSubject ? strSubject : "" );
        if ( hasLineBreakOrNul( strText ) )
            return false;
        m_strSubject = strText;
        return true;
    }

    bool CMail::HeloName( const char *strName )
    {
        CString strText( strName ? strName : "" );
        strText.Trim();
        const char *pText = (const char *)strText;
        for ( size_t nX = 0; nX < strText.GetLength(); nX++ )
        {
            if ( (unsigned char)pText[ nX ] <= ' ' || pText[ nX ] == 0x7F )
                return false;
        }
        m_strHeloName = strText;
        return true;
    }

    CString CMail::NormalizeBody( const CString &strBody )
    {
            // Every line end (CRLF, LF or a lone CR) becomes CRLF, blank lines
            // are kept, and a line starting with '.' gets another one so the
            // server does not take it for the end of the data. (Lines were
            // split on LF with empty ones dropped, so blank lines vanished, and
            // a CR before each LF was kept and then doubled.)
        const char *pBody = (const char *)strBody;
        size_t nLength = strBody.GetLength();
        std::string strResult;
        strResult.reserve( nLength + nLength / 32 + 4 );
        bool bLineStart = true;
        for ( size_t nX = 0; nX < nLength; nX++ )
        {
            char ch = pBody[ nX ];
            if ( ch == '\r' || ch == '\n' )
            {
                if ( ch == '\r' && nX + 1 < nLength && pBody[ nX + 1 ] == '\n' )
                    nX++;
                strResult += "\r\n";
                bLineStart = true;
                continue;
            }
            if ( bLineStart && ch == '.' )
                strResult += '.';
            strResult += ch;
            bLineStart = false;
        }
        if ( ! bLineStart )
            strResult += "\r\n";
        return CString::FromBytes( strResult.data(), strResult.size() );
    }

    CString CMail::BuildMessage( const CString &strHost )
    {
        CString strFromMailbox = MailboxOf( m_strFrom );
        CString strDomain = strHost;
        size_t nAt = strFromMailbox.LastIndexOf( '@' );
        if ( nAt != NOT_FOUND )
            strDomain = strFromMailbox.Substring( nAt + 1 );
        if ( strDomain.GetLength() == 0 )
            strDomain = "localhost";

        CString strSubject = m_strSubject;
        if ( ! isAscii( strSubject ) )
        {
                // RFC 2047 encoded word, so the header stays 7-bit.
            strSubject = CString( "=?UTF-8?B?" ) + base64( (const char *)m_strSubject, m_strSubject.GetLength() ).c_str() + "?=";
        }

        CString strMessage;
        strMessage += CString( "From: " ) + m_strFrom + "\r\n";
        strMessage += CString( "To: " ) + m_strRecipients + "\r\n";
        strMessage += CString( "Subject: " ) + strSubject + "\r\n";
        strMessage += CString( "Date: " ) + mailDate() + "\r\n";
        strMessage += CString( "Message-ID: " ) + messageId( strDomain ) + "\r\n";
        strMessage += "MIME-Version: 1.0\r\n";
        strMessage += "Content-Type: text/plain; charset=UTF-8\r\n";
        strMessage += isAscii( m_strMailBody ) ? "Content-Transfer-Encoding: 7bit\r\n" : "Content-Transfer-Encoding: 8bit\r\n";
        strMessage += "\r\n";
        strMessage += NormalizeBody( m_strMailBody );
            // (A "\r\n" came before the "." line as well, so every message
            // ended with an extra blank line.)
        strMessage += ".\r\n";
        return strMessage;
    }

    int CMail::ReadReply( CSocket *pSocket, std::string &strBuffer, CString &strText )
    {
            // Reply lines are "code-text" while more follow and "code text"
            // for the last (RFC 5321 4.2.1). (Only one line was read, so the
            // rest of a multi-line reply was taken as the reply to the next
            // command, and no reply was ever checked.)
        strText = "";
        int nCode = 0;
        char achBuffer[ 4096 ];
        while ( true )
        {
            size_t nEnd = strBuffer.find( '\n' );
            if ( nEnd == std::string::npos )
            {
                if ( strBuffer.size() > 65536 )
                    return 0;
                int nRead = pSocket->ReadWithTimeout( achBuffer, (int)sizeof( achBuffer ), ( m_nTimeoutMs > 0 ) ? m_nTimeoutMs : -1 );
                if ( nRead <= 0 )
                    return 0;       // A timeout, an error, or the server closed.
                strBuffer.append( achBuffer, (size_t)nRead );
                continue;
            }

            std::string strLine = strBuffer.substr( 0, nEnd );
            strBuffer.erase( 0, nEnd + 1 );
            if ( ! strLine.empty() && strLine[ strLine.size() - 1 ] == '\r' )
                strLine.erase( strLine.size() - 1 );

            if ( strLine.size() < 3 || ! isdigit( (unsigned char)strLine[0] ) || ! isdigit( (unsigned char)strLine[1] ) ||
                 ! isdigit( (unsigned char)strLine[2] ) || ( strLine.size() > 3 && strLine[3] != ' ' && strLine[3] != '-' ) )
                return 0;
            int nLineCode = atoi( strLine.substr( 0, 3 ).c_str() );
            if ( nCode != 0 && nLineCode != nCode )
                return 0;
            nCode = nLineCode;

            if ( strText.GetLength() > 0 )
                strText += "\n";
            strText += ( strLine.size() > 4 ) ? strLine.substr( 4 ).c_str() : "";

            if ( strLine.size() == 3 || strLine[3] == ' ' )
                return nCode;
        }
    }

    int CMail::Command( CSocket *pSocket, std::string &strBuffer, const CString &strCommand, CString &strText )
    {
        try
        {
            if ( pSocket->Send( (const char *)strCommand, (int)strCommand.GetLength() ) != (int)strCommand.GetLength() )
                return 0;
        }
        catch ( ... )
        {
            return 0;
        }
        return ReadReply( pSocket, strBuffer, strText );
    }

	bool CMail::Send( void )
	{
        m_strLastError = "";

        CString strServer;
        int nPort = 25;
        {
            std::lock_guard<std::mutex> lock( serverMutex() );
            strServer = m_strMailServer;
            nPort = m_nMailServerPort;
        }

        CString strFromMailbox = MailboxOf( m_strFrom );
        if ( strFromMailbox.GetLength() == 0 )
        {
            m_strLastError = "The sender's address is not valid.";
            return false;
        }
        if ( m_aRecipientAddresses.Count() == 0 )
        {
            m_strLastError = "There are no recipients.";
            return false;
        }

            // The client socket looks the name up and tries each address
            // (IPv6 and IPv4). (gethostbyname() and inet_ntoa() were used,
            // which are IPv4-only and not thread-safe, and each failed
            // connection leaked its socket.)
        CSocketConfigBuilder builder;
        if ( m_nTimeoutMs > 0 )
            builder.setTimeout( m_nTimeoutMs );
        CClientSocket oSocket( builder.Build(), strServer, nPort );
        if ( ! oSocket.IsConnected() )
        {
            m_strLastError = CString::FormatString( "Could not connect to the mail server %s:%d.", (const char *)strServer, nPort );
            return false;
        }

        std::string strBuffer;
        CString strReply;
        int nCode = ReadReply( &oSocket, strBuffer, strReply );
        if ( nCode != 220 )
        {
            m_strLastError = CString::FormatString( "The mail server did not greet us (%d %s).", nCode, (const char *)strReply );
            oSocket.Close();
            return false;
        }

        CString strHelo = m_strHeloName;
        if ( strHelo.GetLength() == 0 )
        {
                // An address literal for this end of the connection (RFC 5321
                // 4.1.3). (The domain of the sender's address was sent, in
                // angle brackets, which is not a valid HELO argument.)
            CString strLocal = oSocket.GetAddressString( false );
            strHelo = ( strLocal.IndexOf( ':' ) != NOT_FOUND ) ? CString( "[IPv6:" ) + strLocal + "]" : CString( "[" ) + strLocal + "]";
        }

        bool bEightBitMime = false;
        nCode = Command( &oSocket, strBuffer, CString( "EHLO " ) + strHelo + "\r\n", strReply );
        if ( nCode == 250 )
        {
            CString strUpper = CString( "\n" ) + strReply + "\n";
            strUpper.ToUpperCase();
            bEightBitMime = ( strUpper.IndexOf( "\n8BITMIME" ) != NOT_FOUND );
        }
        else if ( nCode >= 500 && nCode < 600 )
        {
                // A server without ESMTP.
            nCode = Command( &oSocket, strBuffer, CString( "HELO " ) + strHelo + "\r\n", strReply );
        }
        if ( nCode != 250 )
        {
            m_strLastError = CString::FormatString( "The mail server refused our greeting (%d %s).", nCode, (const char *)strReply );
            oSocket.Close();
            return false;
        }

        CString strMail = CString( "MAIL FROM:<" ) + strFromMailbox + ">";
        if ( bEightBitMime && ! isAscii( m_strMailBody ) )
            strMail += " BODY=8BITMIME";
        nCode = Command( &oSocket, strBuffer, strMail + "\r\n", strReply );
        if ( nCode != 250 )
        {
            m_strLastError = CString::FormatString( "The mail server refused the sender (%d %s).", nCode, (const char *)strReply );
            Command( &oSocket, strBuffer, CString( "QUIT\r\n" ), strReply );
            oSocket.Close();
            return false;
        }

        for ( size_t nX = 0; nX < m_aRecipientAddresses.Count(); nX++ )
        {
            nCode = Command( &oSocket, strBuffer, CString( "RCPT TO:<" ) + m_aRecipientAddresses[ nX ] + ">\r\n", strReply );
            if ( nCode != 250 && nCode != 251 )
            {
                m_strLastError = CString::FormatString( "The mail server refused the recipient %s (%d %s).",
                                                        (const char *)m_aRecipientAddresses[ nX ], nCode, (const char *)strReply );
                Command( &oSocket, strBuffer, CString( "RSET\r\n" ), strReply );
                Command( &oSocket, strBuffer, CString( "QUIT\r\n" ), strReply );
                oSocket.Close();
                return false;
            }
        }

        nCode = Command( &oSocket, strBuffer, CString( "DATA\r\n" ), strReply );
        if ( nCode != 354 )
        {
            m_strLastError = CString::FormatString( "The mail server refused DATA (%d %s).", nCode, (const char *)strReply );
            Command( &oSocket, strBuffer, CString( "QUIT\r\n" ), strReply );
            oSocket.Close();
            return false;
        }

        nCode = Command( &oSocket, strBuffer, BuildMessage( strHelo ), strReply );
        if ( nCode != 250 )
        {
            m_strLastError = CString::FormatString( "The mail server refused the message (%d %s).", nCode, (const char *)strReply );
            Command( &oSocket, strBuffer, CString( "QUIT\r\n" ), strReply );
            oSocket.Close();
            return false;
        }

            // The mail has been accepted; the reply to QUIT does not matter.
        Command( &oSocket, strBuffer, CString( "QUIT\r\n" ), strReply );
        oSocket.Close();

        ClearRecipients();
        return true;
	}
} // namespace IASLib

#endif // IASLIB_NETWORKING__
