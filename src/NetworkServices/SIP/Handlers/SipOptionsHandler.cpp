/**
 * SIP Options Handler class
 *
 * This class handles Options requests for the SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: September 13, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "Logging/LogSink.h"
#include "NetworkServices/SIP/Handlers/SipOptionsHandler.h"
#include "NetworkServices/Entities/SdpEntity.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipOptionsHandler, CSipHandler );
    IASLIB_REGISTER_SIP_HANDLER( CSipOptionsHandler );

    CSipOptionsHandler::CSipOptionsHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {

    }

    CSipOptionsHandler::~CSipOptionsHandler( void )
    {

    }

    bool CSipOptionsHandler::process( CSipRequest *request, CSipResponse *response )
    {
        INFO_LOG( (const char *)request->toString() );

        response->addHeader( "From", request->getHeaderValue( "From" ) );
        response->addHeader( "To", request->getHeaderValue( "To" ) );
        response->addHeader( "Call-ID", request->getHeaderValue( "Call-ID" ) );
        response->addHeader( "CSeq", request->getHeaderValue( "CSeq" ) );
        response->addHeaders( "Via", request->getHeaderValues( "Via" ) );

        // OPTIONS call must return a 200 OK response, or the server will be considered down by most other systems.
        // We need to generate a set of header values that tell the other end what commands we accept, and what 
        // encodings we will allow in.
        response->setStatus( 200, "OK" );

        response->addHeader( "Allow", CSipHandler::getAllowedList() );
        response->addHeader( "Accept", "application/sdp" );
        response->addHeader( "Accept-Language", "en" );
        response->addHeader( "Accept-Encoding", "identity" );
        response->addHeader( "Accept-Charset", "utf-8;q=1, iso-8859-5;q=0.9" );

        response->addHeader( "Supported", "uui, timer" );

        response->addHeader( "Content-Type", "application/sdp" );

        CSDPEntity *entity = new CSDPEntity();

        // SDP Version
        entity->addAttribute( "v", "0" );
        // Origin tag 
        // (username: '-' means no usernames)
        // (session-id: unique session ID value -- usually NTP time to milliseconds )
        // (session-version: Also usually an NTP timestamp)
        // (nettype: Internet = "IN")
        // (addrtype: IP4 or IP6 supported)
        // (unicast-address: machine IP from which packet was created, either FQDN or dotted decimal. FQDN by preference.)
        entity->addAttribute( "o", CString::FormatString( "- %s %s IN IP4 fonebook.org", (const char *)CDate::NTPTimestamp(), (const char *)CDate::NTPTimestamp() ) ); 
        // Connection tag
        entity->addAttribute( "c", "IN IP4 https://fonebook.org?domain=63.231.96.109");

        // Email 
        entity->addAttribute( "e", "Payfone Support <support@payfone.com>" );

        // Session description
        entity->addAttribute( "s", " " );

        // Session Time As a server, both start and stop are unbounded, so both zeros.
        entity->addAttribute( "t", "0 0" );

        // Available media
        // (Media: audio, video, text, application, message)
        // (Port: port number for connections / number of ports)
        // (Protocol: Name of the protocol)
        // (Format: protocol dependent)
        entity->addAttribute( "m", "audio 30438/2 RTP/AVP 8 0 101" );

        // Session Attributes
        response->SetEntity( entity );

        response->addHeader( "Content-Length", entity->toString().GetLength() );

        return true;
    }
}; // namespace IASLib

#endif // IASLIB_NETWORKING__