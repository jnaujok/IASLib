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
 * @brief CSybaseBulkCopyConnection, a Sybase bulk copy (blk_*) connection.
 */

#ifndef __SYB_BULKCOPYCONNECTION_H__
#define __SYB_BULKCOPYCONNECTION_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "String_.h"
#include "BulkCopyConnection.h"
#include "Database.h"
#include "Mutex.h"
#include <ctpublic.h>

        /// A Sybase connection logged in for bulk copies (Client-Library blk_* calls).
        ///
        /// @note Legacy code: it is built only with GENLIB_INCLUDE_SYBASE (the
        /// old library's switch), sits outside the IASLib namespace, and has
        /// not been brought up to date with the current base classes.
class CSybaseBulkCopyConnection : public CBulkCopyConnection
{
    protected:
        CS_CONNECTION          *m_pConnection;      ///< The Client-Library connection.
        CS_BLKDESC             *m_pBulkDescriptor;  ///< The bulk copy descriptor.
        bool                    m_bValid;           ///< True once logged in.
        bool                    m_bDead;            ///< True once the connection failed.
        static CMutex           m_mutexSynchro;     ///< Serializes Client-Library calls.
        CString                 m_strApplication;   ///< The application name reported.
        bool                    m_bInitialized;     ///< True once SetTable() started a copy.
    public:
                                /// Logs in with bulk copies enabled.
                                /// @param pConnect       The Client-Library connection to use.
                                /// @param strDatabase    The server.
                                /// @param strUserName    The user.
                                /// @param strPassword    The password.
                                /// @param strApplication The application name to report.
                                /// @param strName        A name for the connection.
                                CSybaseBulkCopyConnection( CS_CONNECTION *pConnect, const char *strDatabase, const char *strUserName, const char *strPassword, const char *strApplication, const char *strName = "NoName" );
                                /// Ends any copy and closes the connection.
        virtual                ~CSybaseBulkCopyConnection( void );

        /// Starts a bulk copy into a table (blk_init()).
        virtual bool            SetTable( const char *strTableName );
        /// @copydoc IASLib::CBulkCopyConnection::BindColumn()
        virtual bool            BindColumn( int nColumnNumber, int nDataType, void *pDataBuffer, int nBufferLength );
        /// Sends one row (blk_rowxfer()).
        virtual bool            SendRow( void );
        /// Ends the batch (blk_done()).
        virtual bool            Flush( void );

        /// Records whether the connection is in a deadlock.
        void                    SetDeadlock( bool bLocked ) { m_bDeadlock = bLocked; }

        /// True once the connection has failed.
        virtual bool            IsDead( void ) { return m_bDead; }

};

#endif
#endif
