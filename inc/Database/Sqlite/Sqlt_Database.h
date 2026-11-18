/*
 *  SQL Lite Database
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
    class CSQLiteDatabase : public CDatabase
    {
        protected:
            CString                         m_strDBFile;
            int                             m_nLastErrorCode;
            int                             m_nExtendedErrorCode;
            CString                         m_strLastError;
            int                             m_nTimeoutMs;

        public:
                                            CSQLiteDatabase( void );
            virtual                        ~CSQLiteDatabase( void );

                                            DEFINE_OBJECT( CSQLiteDatabase )

            using CDatabase::Connect;
            virtual bool                    Connect( void );

                // Opens a database file (strDBName; ":memory:" for one in
                // memory). The user name, password and application do not
                // apply to SQLite. NULL if it cannot be opened (see
                // MajorErrorMessage()). The connection belongs to this object.
            virtual CConnection            *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

                // SQLite has no bulk copy: NULL.
            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

                // Deletes every connection.
            virtual int                     Disconnect( void );

                // The last error of this object (opening a connection); each
                // connection has its own.
            virtual int                     MajorErrorCode( void );
            virtual int                     MinorErrorCode( void );
            virtual const char             *MajorErrorMessage( void );
            virtual const char             *MinorErrorMessage( void );

                // Takes a connection out of this object's list, without
                // deleting it (its destructor calls this).
            virtual int                     ReleaseConnection( CConnection *pCon );

                // The lock-wait timeout, in seconds, for every connection now
                // and later.
            virtual bool                    SetTimeout( int nTimeout );

            int                             GetConnectionCount( void ) const { return m_aConnections.Length(); }

        private:
                                            CSQLiteDatabase( const CSQLiteDatabase & );
            CSQLiteDatabase                &operator =( const CSQLiteDatabase & );
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__

#endif // IASLIB_SQLITEDATABASE_H__
