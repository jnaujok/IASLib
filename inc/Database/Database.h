/**
 *  @file
 *  Database Class
 *
 *  Abstract base class from which all of the vendor-specific
 * database classes are derived.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/24/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DATABASE_H__
#define IASLIB_DATABASE_H__

#include "../BaseTypes/String_.h"

#include "ConnectionArray.h"
#include "Connection.h"
#include "BulkCopyConnection.h"

#ifdef IASLIB_DATABASE__

/// No error.
#define IASLIB_DB_OK                     0
/// No error (the same as IASLIB_DB_OK).
#define IASLIB_DB_ERROR_OK               0
/// The database could not be opened or reached.
#define IASLIB_DB_ERROR_BAD_DATABASE    -1
/// The query was not accepted (for a cursor: not a SELECT).
#define IASLIB_DB_ERROR_BAD_QUERY       -2
/// The connection has been marked dead.
#define IASLIB_DB_ERROR_CONNECTION_DEAD -3
/// The operation is not supported by this database.
#define IASLIB_DB_ERROR_UNSUPPORTED     -4

/// The cursor was not opened for updates.
#define IASLIB_DB_CURSOR_READ_ONLY      -101
/// The cursor's current row is not valid.
#define IASLIB_DB_CURSOR_INVALID_ROW    -102

namespace IASLib
{
        /// A database vendor's client library: the factory for connections.
        ///
        /// Each vendor (CSQLiteDatabase, COracleDatabase, CSybaseDatabase)
        /// derives from this. Connect() starts the client library;
        /// Connection() opens a connection, which the database keeps in its
        /// list and deletes in Disconnect() (or its destructor). Not
        /// thread-safe: share a database between threads only with your
        /// own lock.
        ///
        /// @code
        /// CSQLiteDatabase db;
        /// db.Connect();
        /// CConnection *pCon = db.Connection( "app.db", "", "" );
        /// if ( pCon && pCon->IsValid() )
        ///     pCon->Execute( "create table t ( id integer )" );
        /// db.Disconnect();       // deletes the connections
        /// @endcode
    class CDatabase : public CObject
    {
        protected:
            bool                            m_bConnected;       ///< True once Connect() succeeded.
            int                             m_nConnections;     ///< Connections opened.
            CConnectionArray                m_aConnections;     ///< The open connections (owned).
            CString                         m_strVendor;        ///< The vendor's name.
        public:
                                            /// Creates a database with no vendor name.
                                            CDatabase( void );
                                            /// Creates a database for this vendor.
                                            CDatabase( const char *strVendor );
                                            /// Destroys the database (the connection array deletes its connections).
            virtual                        ~CDatabase( void ) {};

                                            DEFINE_OBJECT( CDatabase )

            /// Resets the database for a vendor: deletes every connection and
            /// sets the vendor name.
            /// @return False (subclasses set the connected state themselves).
            bool                            Connect( const char *strVendor );
            /// Starts the vendor's client library.
            /// @return True if it is ready for connections.
            virtual bool                    Connect( void ) = 0;

            /// Opens a connection.
            /// @param strDBName      The database (a file, server or service name, by vendor).
            /// @param strUserName    The user to log in as.
            /// @param strPassword    The password.
            /// @param strApplication The application name to report, where the vendor uses one.
            /// @param strName        A name for the connection.
            /// @return The connection (check IsValid()), or NULL. A connection
            ///         that opened is kept by the database and deleted by Disconnect().
            virtual CConnection            *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" ) = 0;

            /// Opens a connection for bulk copies.
            /// No database here makes one yet: SQLite, Oracle and Sybase all
            /// return NULL (the Sybase code for it is disabled).
            /// @copydetails Connection()
            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" ) = 0;

            /// Closes and deletes every connection.
            /// @return IASLIB_DB_ERROR_OK, or a vendor error code.
            virtual int                     Disconnect( void ) = 0;

            /// The last error's main code.
            virtual int                     MajorErrorCode( void ) = 0;

            /// The last error's secondary code.
            virtual int                     MinorErrorCode( void ) = 0;

            /// The last error's main message.
            virtual const char             *MajorErrorMessage( void ) = 0;

            /// The last error's secondary message.
            virtual const char             *MinorErrorMessage( void ) = 0;

            /// Takes a connection out of the database's list, without deleting it.
            /// @param pCon The connection.
            /// @return 0 or more if it was there (SQLite gives its index; Oracle and
            ///         Sybase give 0), or -1 if it was not.
            virtual int                     ReleaseConnection( CConnection *pCon ) = 0;

            /// Sets the query timeout of every connection.
            /// @param nTimeout The timeout in seconds (Oracle ignores it).
            /// @return True if the vendor took it (SQLite always returns false).
            virtual bool                    SetTimeout( int nTimeout ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATABASE_H__
