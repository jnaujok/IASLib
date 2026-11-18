/**
 *  @file
 *  SQL Lite Database
 *
 *      Database class for connecting to SQLite. Please note that the
 * SQL Lite database requires a unique connection for each thread, which 
 * means that this database type really doesn't do much other then store
 * the information making the connection to SQL Lite. Each connection
 * will then be it's own connection/database.
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
#include "../../DynamicLibraries/DynamicLibrary.h"

namespace IASLib
{
        /// SQLite: opens connections to database files.
        ///
        /// SQLite needs a connection for each thread, so the database does
        /// little more than make connections and keep the list of them.
        /// The SQLite shared library ("sqlite3") is loaded the first time a
        /// function is needed, and stays loaded.
        ///
        /// @code
        /// CSQLiteDatabase db;
        /// db.Connect();
        /// CConnection *pCon = db.Connection( "app.db", NULL, NULL );
        /// if ( ! pCon->Connected() )
        ///     printf( "%s\n", pCon->MajorErrorMessage() );   // not kept by db: delete it
        /// @endcode
    class CSQLiteDatabase : public CDatabase
    {
        protected:
            CString                         m_strDBFile;        ///< The last database file opened.
            int                             m_nLastErrorCode;   ///< The last error code collected.
            CString                         m_strLastError;     ///< The last error message collected.
            static CDynamicLibrary         *m_pSqliteLibrary;   ///< The SQLite library (loaded once, never freed).
        public:
                                            /// Creates the database (vendor "SQL Lite").
                                            CSQLiteDatabase( void );
                                            /// Deletes every connection.
            virtual                        ~CSQLiteDatabase( void );

                                            DEFINE_OBJECT( CSQLiteDatabase )

            /// @copydoc CDatabase::Connect(const char*)
            bool                            Connect( const char *strVendor );
            /// Resets the database; SQLite needs no other set-up.
            /// @return True.
            virtual bool                    Connect( void );

            /// Opens a database file.
            /// @param strDBName      The database file.
            /// @param strUserName    Ignored.
            /// @param strPassword    Ignored.
            /// @param strApplication Ignored.
            /// @param strName        A name for the connection.
            /// @return A new connection (never NULL). If it connected, the database
            ///         keeps it and Disconnect() deletes it; if not, the caller deletes it.
            virtual CConnection            *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            /// Not supported.
            /// @return NULL.
            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            /// Deletes every connection.
            /// @return IASLIB_DB_ERROR_OK.
            virtual int                     Disconnect( void );

            /// The error code of the last connection in the list that has one (0 if none).
            virtual int                     MajorErrorCode( void );

            /// The code MajorErrorCode() last found.
            virtual int                     MinorErrorCode( void );

            /// The error message of the last connection in the list that has one ("" if none).
            virtual const char             *MajorErrorMessage( void );

            /// "".
            virtual const char             *MinorErrorMessage( void );

            /// @copydoc CDatabase::ReleaseConnection()
            virtual int                     ReleaseConnection( CConnection *pCon );

            /// Sets every connection's busy timeout.
            /// @param nTimeout The timeout in seconds.
            /// @return False (always).
            virtual bool                    SetTimeout( int nTimeout );
            /// A function of the SQLite library, loading the library if need be.
            /// @param strName The function's name.
            /// @return Its address, or NULL if it is not found.
            static void                    *GetFunction( const char *strName );

        private:
            /// Loads the SQLite library if it is not loaded yet.
            static void                     initLibrary( void );
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_SQLITEDATABSE_H__
