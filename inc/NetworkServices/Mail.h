/*
 * EMail Base Class
 *
 *      This class offers a very simple interface for sending electronic
 * mail. It does this via the SMTP protocol. Please be aware that the
 * mail server must be set before calling sending the e-mail.
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

namespace IASLib
{
	class CMail : public CObject
	{
		protected:
			static CString      m_strMailServer;
			static int          m_nMailServerPort;

			CString             m_strRecipients;
			CString             m_strFrom;
			CString             m_strSubject;
			CString             m_strMailBody;
		public:

			static void         SetServer( const char *strMailServer, int nPort );

								CMail( void );
			virtual            ~CMail( void );

								DEFINE_OBJECT( CMail );

			void                From( const char *strFrom ) { m_strFrom = strFrom; }
			void                AddRecipient( const char *strTo );
			void                Subject( const char *strSubject ) { m_strSubject = strSubject; }
			void                Body( const char *strBodyText ) { m_strMailBody = strBodyText; }

			bool                Send( void );
	};
} // namespace IASLib

#endif // IASLIB_MAIL_H__

#endif // IASLIB_NETWORKING__