/**
 * @file
 * @brief SQL Lite Database.
 *
 *      The SQLite "vendor": it makes connections (Connection() opens a
 * database file) and owns them. Deleting the database, or Disconnect(),
 * deletes its connections; deleting a connection yourself is also fine
 * (it leaves the database's list). SQLite is linked in directly.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/13/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SQLITEDATABASE_H__
#define IASLIB_SQLITEDATABASE_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_SQLITE__

#include "../Database.h"

namespace IASLib
{
    /// The SQLite "vendor": it makes connections and owns them.
    ///
    /// Connection() opens a database file. Deleting the database, or
    /// Disconnect(), deletes its connections; deleting a connection yourself
    /// is also fine (it leaves the database's list). SQLite is linked in
    /// directly. Not copyable.
    ///
    /// @code
    /// CSQLiteDatabase oDatabase;
    /// CConnection *pConnection = oDatabase.Connection( "app.db", "", "" );
    /// if ( pConnection == NULL )
    ///     printf( "%s\n", oDatabase.MajorErrorMessage() );
    /// @endcode
    class CSQLiteDatabase : public CDatabase
    {
        protected:
            CString                         m_strDBFile;            ///< The last file Connection() was asked for.
            int                             m_nLastErrorCode;       ///< The last error's code.
            int                             m_nExtendedErrorCode;   ///< SQLite's extended result code for it.
            CString                         m_strLastError;         ///< The last error's text.
            int                             m_nTimeoutMs;           ///< The lock-wait timeout for connections, in milliseconds.

        public:
                                            /// Makes the vendor object (no connections yet).
                                            CSQLiteDatabase( void );
                                            /// Destructor. Deletes every connection.
            virtual                        ~CSQLiteDatabase( void );

                                            DEFINE_OBJECT( CSQLiteDatabase )

                /// CDatabase's other Connect() forms.
            using CDatabase::Connect;
                /// Marks the database connected (there is nothing to connect
                /// to). Returns true.
            virtual bool                    Connect( void );

                /// Opens a database file (strDBName; ":memory:" for one in
                /// memory). The user name, password and application do not
                /// apply to SQLite. NULL if it cannot be opened (see
                /// MajorErrorMessage()). The connection belongs to this object.
                /// @note A connection that failed to open used to be returned
                /// anyway, and the caller had to know to delete it.
            virtual CConnection            *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

                /// SQLite has no bulk copy: NULL.
            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

                /// Deletes every connection. Returns IASLIB_DB_ERROR_OK.
                /// @note Deleting them in place used to skip every other one.
            virtual int                     Disconnect( void );

                /// The last error of this object (opening a connection); each
                /// connection has its own.
                /// @note These used to report whatever the last connection in
                /// the list had.
            virtual int                     MajorErrorCode( void );
            virtual int                     MinorErrorCode( void );         ///< SQLite's extended result code for the last error.
            virtual const char             *MajorErrorMessage( void );      ///< The last error's text.
            virtual const char             *MinorErrorMessage( void );      ///< Same as MajorErrorMessage().

                /// Takes a connection out of this object's list, without
                /// deleting it (its destructor calls this).
                /// @return Its index in the list, or -1 if it was not there.
            virtual int                     ReleaseConnection( CConnection *pCon );

                /// The lock-wait timeout, in seconds, for every connection now
                /// and later. False for a negative timeout.
                /// @note It used to apply the timeout but always report failure.
            virtual bool                    SetTimeout( int nTimeout );

            int                             GetConnectionCount( void ) const { return m_aConnections.Length(); }   ///< The number of open connections.

        private:
                                            CSQLiteDatabase( const CSQLiteDatabase & );     ///< Not copyable.
            CSQLiteDatabase                &operator =( const CSQLiteDatabase & );         ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

#endif // IASLIB_SQLITEDATABASE_H__
