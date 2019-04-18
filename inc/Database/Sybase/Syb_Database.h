/***********************************************************************
**  SybaseDatabase
**
**  Description:
**      This class defines and implements the Sybase Database class. It
** manages connections to and from Sybase databases.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef __SYB_DATABASE_H__
#define __SYB_DATABASE_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "Database.h"
#include "Connection.h"
#include <ctpublic.h>

class CSybaseDatabase : public CDatabase
{
    protected:
            // Error Message Storage
        static CString          m_strLastMajorMessage;
        static CString          m_strLastMinorMessage;
        static int              m_nLastMajorErrorCode;
        static int              m_nLastMinorErrorCode;
            // Client message related storage
        static int              m_nMinimumClientSeverity;
            // Server message related storage
        static int              m_nMinimumServerSeverity;

        static bool             m_bInitialized;
        static CS_CONTEXT      *m_pContext;
    public:
                                CSybaseDatabase( void );
        virtual                ~CSybaseDatabase( void );

        int                     Connect( void );

        virtual CConnection    *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

        virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

        virtual int             Disconnect( void );

        virtual int             MajorErrorCode( void );

        virtual int             MinorErrorCode( void );

        virtual const char     *MajorErrorMessage( void );

        virtual const char     *MinorErrorMessage( void );

        virtual void            StoredProcArgs( const char *strProcName, ... );

        virtual int             ReleaseConnection( CConnection *pCon );

        virtual bool            SetTimeout( int nTimeout );
	private:
        static CS_RETCODE       ClientMessageHandler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg);
        static CS_RETCODE       ServerMessageHandler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg);
        static bool             Initialize( void );


};

#endif
#endif
