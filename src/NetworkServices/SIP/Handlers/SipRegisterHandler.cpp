/**
 * SIP Register Handler class
 *
 * Author: Jeffrey R. Naujok
 * Created: September 16, 2019
 */

#ifdef IASLIB_NETWORKING__

#include "NetworkServices/SIP/Handlers/SipRegisterHandler.h"
#include "NetworkServices/SIP/SipServer.h"
#include "NetworkServices/SIP/SipUtils.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CSipRegisterHandler, CSipHandler );

    CSipRegisterHandler::CSipRegisterHandler( CString uri, CString version ) : CSipHandler( uri, version )
    {
    }

    CSipRegisterHandler::~CSipRegisterHandler( void )
    {
    }

    bool CSipRegisterHandler::process( CSipRequest *request, CSipResponse *response )
    {
            // There are no credentials to check them against, so a request
            // that already answered the challenge is refused outright.
        if ( request->hasHeader( "Authorization" ) )
        {
            response->setStatus( 403 );
            return true;
        }

        CString realm = ( m_pServer ) ? m_pServer->getRealm() : CString( "IASLib" );
            // Quotes would break the quoted-string; the realm is configured by
            // the application, but keep the header well-formed regardless.
        realm.Remove( '"' );

        CString challenge = "Digest realm=\"";
        challenge += realm;
        challenge += "\", nonce=\"";
        challenge += generateNonce();
        challenge += "\", algorithm=MD5, qop=\"auth\"";

        response->addHeader( "WWW-Authenticate", challenge );
        response->setStatus( 401 );

        return true;
    }

    CString CSipRegisterHandler::generateNonce( void )
    {
        return CSipUtils::randomToken( 16 );
    }
} // namespace IASLib

#endif // IASLIB_NETWORKING__
