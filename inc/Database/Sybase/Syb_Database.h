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

/**
 * @file
 * @brief CSybaseDatabase, the Sybase (Client-Library) database.
 */

#ifndef __SYB_DATABASE_H__
#define __SYB_DATABASE_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "Database.h"
#include "Connection.h"
#include <ctpublic.h>

        /// Sybase (Client-Library): makes connections to Sybase servers.
        ///
        /// Connect() sets up the Client-Library context (once, shared) and
        /// its message handlers, which record each message as the last error in static
        /// members shared by every Sybase database and connection.
        /// @note Legacy code: it is built only with GENLIB_INCLUDE_SYBASE (the
        /// old library's switch), sits outside the IASLib namespace, and has
        /// not been brought up to date with the current base classes.
class CSybaseDatabase : public CDatabase
{
    protected:
            // Error Message Storage
        static CString          m_strLastMajorMessage;      ///< The last error's message ("None" at first).
        static CString          m_strLastMinorMessage;      ///< The last error's secondary message.
        static int              m_nLastMajorErrorCode;      ///< The last error's code.
        static int              m_nLastMinorErrorCode;      ///< The last error's secondary code.
            // Client message related storage
        static int              m_nMinimumClientSeverity;   ///< 12; not used by the handlers yet.
            // Server message related storage
        static int              m_nMinimumServerSeverity;   ///< 12; not used by the handlers yet.

        static bool             m_bInitialized;             ///< True once Initialize() succeeded.
        static CS_CONTEXT      *m_pContext;                 ///< The Client-Library context.
    public:
                                /// Creates the database (vendor "Sybase").
                                CSybaseDatabase( void );
                                /// Deletes every connection.
        virtual                ~CSybaseDatabase( void );

        /// Sets up Client-Library (once).
        /// @return True if it is ready for connections.
        bool                    Connect( void );

        /// Logs in to a server.
        /// @copydetails IASLib::CDatabase::Connection()
        virtual CConnection    *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

        /// Disabled. @return NULL.
        virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

        /// Deletes every connection. @return 0.
        virtual int             Disconnect( void );

        /// The last error's code.
        virtual int             MajorErrorCode( void );

        /// The last error's secondary code.
        virtual int             MinorErrorCode( void );

        /// The last error's message.
        virtual const char     *MajorErrorMessage( void );

        /// The last error's secondary message.
        virtual const char     *MinorErrorMessage( void );

        /// A placeholder for stored procedure arguments.
        virtual void            StoredProcArgs( const char *strProcName, ... );

        /// @copydoc IASLib::CDatabase::ReleaseConnection()
        virtual int             ReleaseConnection( CConnection *pCon );

        /// Sets Client-Library's command timeout (CS_TIMEOUT), for every connection.
        /// @param nTimeout The timeout in seconds.
        /// @return False if it could not be set.
        virtual bool            SetTimeout( int nTimeout );
	private:
        /// Records each client message as the last error; returns CS_FAIL for a timeout, which cancels the command.
        static CS_RETCODE       ClientMessageHandler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg);
        /// Records each server message as the last error, and marks the connection deadlocked on error 1205.
        static CS_RETCODE       ServerMessageHandler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg);
        /// Sets up the context and message handlers, once.
        static bool             Initialize( void );


};

#endif
#endif
