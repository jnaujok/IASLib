/***********************************************************************
**  Mail Class
**
**  Description:
**      This class wraps the sending of mail through an SMTP server.
** Please note that the SMTP server must be set-up prior to calling the
** send method of the class.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#include "Mail.h"
#include "ClientSocket.h"
#include "SocketStream.h"
#include "SocketException.h"
#include "StringTokenizer.h"
#include "Date.h"

#ifdef IASLIB_NETWORKING__

namespace IASLib
{
	CString CMail::m_strMailServer = "pop.mail.com";
	int     CMail::m_nMailServerPort = 25;

	IMPLEMENT_OBJECT( CMail, CObject );

	CMail::CMail( void )
	{
		m_strRecipients = "";
		m_strFrom = "Unknown@unknown.com";
		m_strSubject = "<No Subject>";
		m_strMailBody = "<No Body>";
	}

	CMail::~CMail( void )
	{
		// Do Nothing
	}

	void CMail::SetServer( const char *strMailServer, int nPort )
	{
		m_strMailServer = strMailServer;
		m_nMailServerPort = nPort;
	}

	void CMail::AddRecipient( const char *strTo )
	{
		if ( m_strRecipients.GetLength() )
		{
			m_strRecipients += ", ";
		}

		m_strRecipients += strTo;
	}

	bool CMail::Send( void )
	{
		bool            bRetVal = false;
		CString         strIPAddress;

		struct hostent *pHost = gethostbyname( m_strMailServer );
		if ( pHost )
		{
			int nCount = 0;
			while ( ( !bRetVal ) && ( pHost->h_addr_list[nCount] ) )
			{
				unsigned long nAddress = *( (unsigned long *)pHost->h_addr_list[nCount] );
				struct in_addr addrConvert;
				addrConvert.s_addr = nAddress;
				strIPAddress = inet_ntoa( addrConvert );

				CClientSocket  *pMailServer = new CClientSocket( strIPAddress, m_nMailServerPort );
				CString         strResponse;
				CString         strCommand;
				CString         strWork;

				if ( pMailServer->IsConnected() )
				{
					bRetVal = true;
					CSocketStream   oStream( pMailServer );

					try
					{
						strResponse = oStream.GetLine();

						strWork = m_strFrom;
						if ( strWork.IndexOf( '@' ) != NOT_FOUND )
						{
							strWork = strWork.Substring( strWork.IndexOf( '@' ) + 1 );
						}

						strCommand.Format( "HELO <%s>\r\n", (const char *)strWork );
						oStream << strCommand;

						strResponse = oStream.GetLine();

						strCommand.Format( "MAIL FROM:<%s>\r\n", (const char *)m_strFrom );
						oStream << strCommand;

						strResponse = oStream.GetLine();

						CStringTokenizer    stRecipients( m_strRecipients, ",", false );

						while ( stRecipients.HasMoreTokens() )
						{
							strWork = stRecipients.NextToken();
							strWork.Trim();

							strCommand.Format( "RCPT TO:<%s>\r\n", (const char *)strWork );
							oStream << strCommand;

							strResponse = oStream.GetLine();
						}

						strCommand = "DATA\r\n";
						oStream << strCommand;

						strResponse = oStream.GetLine();

							// Begin sending mail body
						CDate dttNow;

						strCommand.Format( "From: %s\r\n", (const char *)m_strFrom );
						oStream << strCommand;

						strCommand.Format( "To: %s\r\n", (const char *)m_strRecipients );
						oStream << strCommand;

						strCommand.Format( "Date: %s\r\n", (const char *)dttNow.HTTPDate() );
						oStream << strCommand;

						strCommand.Format( "Subject: %s\r\n", (const char *)m_strSubject );
						oStream << strCommand;

						strCommand = "\r\n";
						oStream << strCommand;

						CStringTokenizer  stLines( m_strMailBody, "\n", false );

						while ( stLines.HasMoreTokens() )
						{
							strWork = stLines.NextToken();

							if ( strWork.IndexOf( '.' ) == 0 )
							{
								strWork = "." + strWork;
							}

							strWork += "\r\n";
							oStream << strWork;
						}
						strCommand = "\r\n.\r\n";
						oStream << strCommand;

						strResponse = oStream.GetLine();

						strCommand = "QUIT\r\n";
						oStream << strCommand;

						strResponse = oStream.GetLine();

						bRetVal = true;

							// Clear recipient list.
						m_strRecipients = "";
					}
					catch (CSocketException *pExcept )
					{
						bRetVal = false;
						delete pExcept;
					}
				}
				else
				{
					delete pMailServer;
				}
				nCount++;
			}
		}
		else
		{
			// No Mail Host
		}

		return bRetVal;
	}
} // namespace IASLib

#endif // IASLIB_NETWORKING