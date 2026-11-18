/**
 * Mail tests
 *
 * Tests CMail against a small fake SMTP server run on a thread of this
 * program, on a loopback port the system picks: the conversation (EHLO,
 * HELO fallback, multi-line replies, refusals), the message it sends
 * (headers, line endings, dot-stuffing, blank lines) and the refusal of
 * addresses and subjects that could inject commands or headers.
 */

#include "IASLib.h"
#include "NetworkServices/Mail.h"
#include "Sockets/ServerSocket.h"

#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace IASLib;

static int g_nChecks = 0;
static int g_nFailures = 0;

#define CHECK( condition ) \
    do { \
        g_nChecks++; \
        if ( ! ( condition ) ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition ); \
        } \
    } while ( 0 )

    // How the fake server behaves.
struct CSmtpScript
{
    bool        bRefuseEhlo;            // answer EHLO with 502 (so HELO is used)
    bool        bSilent;                // never send the greeting
    std::string strRefuseRecipient;     // answer RCPT TO for this mailbox with 550
    int         nDataReply;             // the reply to the end of the data

    CSmtpScript( void ) : bRefuseEhlo( false ), bSilent( false ), nDataReply( 250 ) {}
};

    // Serves one SMTP session, recording the commands and the message.
class CFakeSmtpServer
{
    public:
        CFakeSmtpServer( const CSmtpScript &script ) : m_script( script ), m_stop( false )
        {
            m_pListen = new CServerSocket( 0, 4 );
            m_nPort = m_pListen->GetPort();
            m_thread = std::thread( [this] { run(); } );
        }
        ~CFakeSmtpServer( void )
        {
            m_stop = true;
            m_thread.join();
            delete m_pListen;
        }
        int port( void ) const { return m_nPort; }
        std::vector<std::string> commands( void ) { std::lock_guard<std::mutex> lock( m_mutex ); return m_commands; }
        std::string message( void ) { std::lock_guard<std::mutex> lock( m_mutex ); return m_strMessage; }

    private:
        void send( CSocket *pSocket, const std::string &strReply )
        {
            try { pSocket->Send( strReply.data(), (int)strReply.size() ); } catch ( ... ) {}
        }

            // One line (without its CRLF), or false when the client is gone.
        bool readLine( CSocket *pSocket, std::string &strBuffer, std::string &strLine )
        {
            while ( ! m_stop )
            {
                size_t nEnd = strBuffer.find( "\r\n" );
                if ( nEnd != std::string::npos )
                {
                    strLine = strBuffer.substr( 0, nEnd );
                    strBuffer.erase( 0, nEnd + 2 );
                    return true;
                }
                char achBuffer[ 4096 ];
                int nRead = pSocket->ReadWithTimeout( achBuffer, (int)sizeof( achBuffer ), 100 );
                if ( nRead == CSocket::READ_TIMEOUT )
                    continue;
                if ( nRead <= 0 )
                    return false;
                strBuffer.append( achBuffer, (size_t)nRead );
            }
            return false;
        }

        void run( void )
        {
            CSocket *pSocket = NULL;
            while ( ! m_stop && pSocket == NULL )
                pSocket = m_pListen->Accept( 100 );
            if ( pSocket == NULL )
                return;

            if ( m_script.bSilent )
            {
                while ( ! m_stop )
                    std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
                delete pSocket;
                return;
            }

                // A multi-line greeting.
            send( pSocket, "220-fake.example ESMTP\r\n220 ready\r\n" );
            std::string strBuffer;
            std::string strLine;
            while ( readLine( pSocket, strBuffer, strLine ) )
            {
                {
                    std::lock_guard<std::mutex> lock( m_mutex );
                    m_commands.push_back( strLine );
                }
                std::string strVerb = strLine.substr( 0, 4 );
                for ( size_t n = 0; n < strVerb.size(); n++ ) strVerb[ n ] = (char)toupper( (unsigned char)strVerb[ n ] );

                if ( strVerb == "EHLO" )
                    send( pSocket, m_script.bRefuseEhlo ? "502 not here\r\n" : "250-fake.example\r\n250-8BITMIME\r\n250-SIZE 1000000\r\n250 HELP\r\n" );
                else if ( strVerb == "HELO" )
                    send( pSocket, "250 fake.example\r\n" );
                else if ( strVerb == "MAIL" )
                    send( pSocket, "250 sender ok\r\n" );
                else if ( strVerb == "RCPT" )
                {
                    bool bRefuse = ! m_script.strRefuseRecipient.empty() && strLine.find( "<" + m_script.strRefuseRecipient + ">" ) != std::string::npos;
                    send( pSocket, bRefuse ? "550-no such user\r\n550 really\r\n" : "250 recipient ok\r\n" );
                }
                else if ( strVerb == "DATA" )
                {
                    send( pSocket, "354 go ahead\r\n" );
                    std::string strMessage;
                    while ( readLine( pSocket, strBuffer, strLine ) && strLine != "." )
                        strMessage += strLine + "\r\n";
                    {
                        std::lock_guard<std::mutex> lock( m_mutex );
                        m_strMessage = strMessage;
                    }
                    send( pSocket, std::to_string( m_script.nDataReply ) + ( m_script.nDataReply == 250 ? " queued\r\n" : " no\r\n" ) );
                }
                else if ( strVerb == "RSET" )
                    send( pSocket, "250 reset\r\n" );
                else if ( strVerb == "QUIT" )
                {
                    send( pSocket, "221 bye\r\n" );
                    break;
                }
                else
                    send( pSocket, "500 what?\r\n" );
            }
            pSocket->Close();
            delete pSocket;
        }

        CSmtpScript                 m_script;
        std::atomic<bool>           m_stop;
        CServerSocket              *m_pListen;
        int                         m_nPort;
        std::thread                 m_thread;
        std::mutex                  m_mutex;
        std::vector<std::string>    m_commands;
        std::string                 m_strMessage;
};

static bool startsWith( const std::string &strText, const std::string &strPrefix )
{
    return strText.compare( 0, strPrefix.size(), strPrefix ) == 0;
}

static std::string headerOf( const std::string &strMessage, const std::string &strName )
{
    size_t nPos = 0;
    while ( nPos < strMessage.size() )
    {
        size_t nEnd = strMessage.find( "\r\n", nPos );
        if ( nEnd == std::string::npos || nEnd == nPos )
            break;
        std::string strLine = strMessage.substr( nPos, nEnd - nPos );
        if ( startsWith( strLine, strName + ": " ) )
            return strLine.substr( strName.size() + 2 );
        nPos = nEnd + 2;
    }
    return std::string();
}

static void testAddresses( void )
{
    printf( "Addresses\n" );
    CHECK( CMail::MailboxOf( "jo@example.com" ) == "jo@example.com" );
    CHECK( CMail::MailboxOf( "Jo Bloggs <jo@example.com>" ) == "jo@example.com" );
    CHECK( CMail::MailboxOf( "\"Bloggs, Jo\" <jo@example.com>" ) == "jo@example.com" );
    CHECK( CMail::MailboxOf( "jo@example.com>\r\nRCPT TO:<evil@example.com" ) == "" );
    CHECK( CMail::MailboxOf( "jo@example.com\r\nBcc: evil@example.com" ) == "" );
    CHECK( CMail::MailboxOf( "jo smith@example.com" ) == "" );
    CHECK( CMail::MailboxOf( "nobody" ) == "" );
    CHECK( CMail::MailboxOf( "<jo@example.com> trailing" ) == "" );

    CMail mail;
    CHECK( mail.From( "app@example.com" ) );
    CHECK( ! mail.From( "app@example.com\r\nRCPT TO:<x@y.z>" ) );
    CHECK( mail.AddRecipient( "a@example.com, \"Bloggs, Jo\" <jo@example.com>" ) );
    CHECK( ! mail.AddRecipient( "ok@example.com, bad@example.com\nDATA" ) );
    CHECK( mail.Subject( "Fine" ) );
    CHECK( ! mail.Subject( "Bad\r\nBcc: evil@example.com" ) );
    CHECK( ! mail.HeloName( "bad name" ) && mail.HeloName( "client.example" ) );

        // Line endings, blank lines and dot-stuffing.
    CHECK( CMail::NormalizeBody( "a\nb\r\nc\rd" ) == "a\r\nb\r\nc\r\nd\r\n" );
    CHECK( CMail::NormalizeBody( "one\n\n\nfour\n" ) == "one\r\n\r\n\r\nfour\r\n" );
    CHECK( CMail::NormalizeBody( ".\n..x\nmid.dle\n.end" ) == "..\r\n...x\r\nmid.dle\r\n..end\r\n" );
    CHECK( CMail::NormalizeBody( "" ) == "" );
}

static void testSend( void )
{
    printf( "Sending\n" );
    CSmtpScript script;
    CFakeSmtpServer server( script );
    CMail::SetServer( "localhost", server.port() );

    CMail mail;
    mail.SetTimeout( 5000 );
    CHECK( mail.From( "App <app@example.com>" ) );
    CHECK( mail.AddRecipient( "a@example.com" ) );
    CHECK( mail.AddRecipient( "Jo <jo@example.com>" ) );
    mail.Subject( "Report" );
    mail.Body( "Line one\n\n.hidden\r\n.\nlast" );
    CHECK( mail.Send() );
    CHECK( mail.GetLastError() == "" );

    std::vector<std::string> commands = server.commands();
    CHECK( commands.size() == 6 );
    if ( commands.size() == 6 )
    {
        CHECK( startsWith( commands[0], "EHLO [" ) );
        CHECK( commands[1] == "MAIL FROM:<app@example.com>" );
        CHECK( commands[2] == "RCPT TO:<a@example.com>" );
        CHECK( commands[3] == "RCPT TO:<jo@example.com>" );
        CHECK( commands[4] == "DATA" );
        CHECK( commands[5] == "QUIT" );
    }

    std::string strMessage = server.message();
    CHECK( headerOf( strMessage, "From" ) == "App <app@example.com>" );
    CHECK( headerOf( strMessage, "To" ) == "a@example.com, Jo <jo@example.com>" );
    CHECK( headerOf( strMessage, "Subject" ) == "Report" );
    CHECK( headerOf( strMessage, "MIME-Version" ) == "1.0" );
    CHECK( headerOf( strMessage, "Content-Type" ) == "text/plain; charset=UTF-8" );
    std::string strDate = headerOf( strMessage, "Date" );
    CHECK( strDate.size() == 31 && strDate.compare( strDate.size() - 5, 5, "+0000" ) == 0 && strDate[3] == ',' );
    std::string strId = headerOf( strMessage, "Message-ID" );
    CHECK( startsWith( strId, "<" ) && strId.find( "@example.com>" ) != std::string::npos );

        // The body as the server un-stuffs it: blank line kept, dots doubled
        // on the wire (the server's reader stops at a lone "." only).
    size_t nBody = strMessage.find( "\r\n\r\n" );
    std::string strBody = ( nBody == std::string::npos ) ? std::string() : strMessage.substr( nBody + 4 );
    CHECK( strBody == "Line one\r\n\r\n..hidden\r\n..\r\nlast\r\n" );

        // The recipients are cleared after a successful send.
    CHECK( ! mail.Send() && mail.GetLastError() == "There are no recipients." );

        // A second message gets a different Message-ID.
    CHECK( CMail().BuildMessage( "h" ) != CMail().BuildMessage( "h" ) );
}

static void testHeloFallback( void )
{
    printf( "HELO fallback\n" );
    CSmtpScript script;
    script.bRefuseEhlo = true;
    CFakeSmtpServer server( script );
    CMail::SetServer( "127.0.0.1", server.port() );

    CMail mail;
    mail.SetTimeout( 5000 );
    mail.From( "app@example.com" );
    mail.AddRecipient( "a@example.com" );
    mail.HeloName( "client.example" );
    mail.Body( "hi" );
    CHECK( mail.Send() );
    std::vector<std::string> commands = server.commands();
    CHECK( commands.size() >= 2 && commands[0] == "EHLO client.example" && commands[1] == "HELO client.example" );
}

static void testRefusals( void )
{
    printf( "Refusals\n" );
    {
            // A refused recipient (with a multi-line reply) stops the send.
        CSmtpScript script;
        script.strRefuseRecipient = "nobody@example.com";
        CFakeSmtpServer server( script );
        CMail::SetServer( "127.0.0.1", server.port() );
        CMail mail;
        mail.SetTimeout( 5000 );
        mail.From( "app@example.com" );
        mail.AddRecipient( "a@example.com, nobody@example.com" );
        CHECK( ! mail.Send() );
        CHECK( mail.GetLastError().IndexOf( "nobody@example.com" ) != NOT_FOUND && mail.GetLastError().IndexOf( "550" ) != NOT_FOUND );
        std::vector<std::string> commands = server.commands();
        bool bData = false;
        for ( size_t n = 0; n < commands.size(); n++ )
            if ( commands[ n ] == "DATA" )
                bData = true;
        CHECK( ! bData );
        CHECK( ! commands.empty() && commands.back() == "QUIT" );
    }
    {
            // The message refused at the end of the data.
        CSmtpScript script;
        script.nDataReply = 554;
        CFakeSmtpServer server( script );
        CMail::SetServer( "127.0.0.1", server.port() );
        CMail mail;
        mail.SetTimeout( 5000 );
        mail.From( "app@example.com" );
        mail.AddRecipient( "a@example.com" );
        CHECK( ! mail.Send() && mail.GetLastError().IndexOf( "554" ) != NOT_FOUND );
    }
    {
            // A server that never greets: the timeout ends the wait.
        CSmtpScript script;
        script.bSilent = true;
        CFakeSmtpServer server( script );
        CMail::SetServer( "127.0.0.1", server.port() );
        CMail mail;
        mail.SetTimeout( 300 );
        mail.From( "app@example.com" );
        mail.AddRecipient( "a@example.com" );
        auto start = std::chrono::steady_clock::now();
        CHECK( ! mail.Send() );
        long nElapsed = (long)std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - start ).count();
        CHECK( nElapsed < 3000 );
        CHECK( mail.GetLastError().IndexOf( "greet" ) != NOT_FOUND );
    }
    {
            // Nothing listening.
        int nFreePort = 0;
        {
            CServerSocket probe( 0, 1 );
            nFreePort = probe.GetPort();
        }
        CMail::SetServer( "127.0.0.1", nFreePort );
        CMail mail;
        mail.From( "app@example.com" );
        mail.AddRecipient( "a@example.com" );
        CHECK( ! mail.Send() && mail.GetLastError().IndexOf( "connect" ) != NOT_FOUND );
    }
}

int main( void )
{
    setvbuf( stdout, NULL, _IONBF, 0 );

    testAddresses();
    testSend();
    testHeloFallback();
    testRefusals();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
