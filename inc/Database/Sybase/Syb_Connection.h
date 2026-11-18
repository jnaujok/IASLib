/***********************************************************************
**  CSybaseConection
**
**  Description:
**      This class defines and implements a connection to a Sybase
** database.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

/**
 * @file
 * @brief CSybaseConnection, a connection to a Sybase server.
 */

#ifndef __SYB_CONNECTION_H__
#define __SYB_CONNECTION_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "String_.h"
#include "Date.h"
#include "Connection.h"
#include "Database.h"
#include "Mutex.h"
#include <ctpublic.h>

        /// A connection to a Sybase server through Client-Library (ct_* calls).
        ///
        /// Made by CSybaseDatabase::Connection(). A failed command marks the
        /// connection dead. The error functions report the database's
        /// (shared) last error. Transactions are not implemented: the
        /// transaction calls only return true.
        /// @note Legacy code: it is built only with GENLIB_INCLUDE_SYBASE (the
        /// old library's switch), sits outside the IASLib namespace, and has
        /// not been brought up to date with the current base classes.
class CSybaseConnection : public CConnection
{
    protected:
        CDatabase              *m_pDatabase;        ///< The database that made it (not owned).
        CS_CONNECTION          *m_pConnection;      ///< The Client-Library connection.
        CS_COMMAND             *m_pCommand;         ///< The command structure for queries.
        bool                    m_bValid;           ///< True once logged in.
        bool                    m_bDeadlock;        ///< Set by SetDeadlock() (from the server message handler).
        bool                    m_bDead;            ///< True once a command failed.
        long                    m_lAffectedRows;    ///< Not set yet (always 0).
        static CMutex           m_mutexSynchro;     ///< Serializes Client-Library calls.
    public:
                                /// Logs in to a server.
                                /// @param pConnect       The Client-Library connection to use.
                                /// @param strDatabase    The server.
                                /// @param strUserName    The user.
                                /// @param strPassword    The password.
                                /// @param strApplication The application name to report.
                                /// @param strName        A name for the connection.
                                CSybaseConnection( CS_CONNECTION *pConnect, const char *strDatabase, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName = "NoName" );
                                /// Closes the connection.
        virtual                ~CSybaseConnection( void );

        /// Sets the database that made the connection (for errors and Release()).
        void                    SetDatabase( CDatabase *pDatabase );

        /// Not implemented. @return True.
        virtual bool            BeginTransaction( void );
        /// Not implemented. @return True.
        virtual bool            CommitTransaction( void );
        /// Not implemented. @return True.
        virtual bool            RollbackTransaction( void );

        /// @copydoc IASLib::CConnection::Cursor()
        virtual CCursor        *Cursor( const char *strQueryString, bool bUpdatable = 0 );
        /// @copydoc IASLib::CConnection::Execute()
        virtual bool            Execute( const char *strExecString );

        /// The database's last error code.
        virtual int             MajorErrorCode( void );
        /// The database's last secondary error code.
        virtual int             MinorErrorCode( void );

        /// The database's last error message.
        virtual const char     *MajorErrorMessage( void );
        /// The database's last secondary error message.
        virtual const char     *MinorErrorMessage( void );

        /// @copydoc IASLib::CConnection::Release()
        virtual bool            Release( void );

        /// @copydoc IASLib::CConnection::SQLTable()
        virtual CString         SQLTable( const char *strQueryString, CStream *pStream = NULL );

        /// Runs a stored procedure; Sybase can return result sets as well.
        /// @param strProcName  The procedure call.
        /// @param strParamList The output parameters to collect (NULL: none).
        /// @return A new result (the caller deletes it), or NULL on failure.
        virtual CStoredProc    *StoredProc( const char *strProcName, const char *strParamList = NULL );

        /// Records whether the connection is in a deadlock.
        void                    SetDeadlock( bool bLocked ) { m_bDeadlock = bLocked; }

        /// True once a command has failed.
        bool                    IsDead( void ) { return m_bDead; }

		/// DB_SYBASE.
		virtual int				GetDBType( void ) { return DB_SYBASE; }
};

#endif
#endif
