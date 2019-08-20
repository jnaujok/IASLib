/*
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
    class CSQLiteDatabase : public CDatabase
    {
        protected:
            CString                         m_strDBFile;
            int                             m_nLastErrorCode;
            CString                         m_strLastError;
            static CDynamicLibrary         *m_pSqliteLibrary;
        public:
                                            CSQLiteDatabase( void );
            virtual                        ~CSQLiteDatabase( void );

                                            DEFINE_OBJECT( CSQLiteDatabase )

            bool                            Connect( const char *strVendor );
            virtual bool                    Connect( void );

            virtual CConnection            *Connection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            virtual CBulkCopyConnection    *BulkCopyConnection( const char *strDBName, const char *strUserName, const char *strPassword, const char *strApplication = "", const char *strName = "NoName" );

            virtual int                     Disconnect( void );

            virtual int                     MajorErrorCode( void );

            virtual int                     MinorErrorCode( void );

            virtual const char             *MajorErrorMessage( void );

            virtual const char             *MinorErrorMessage( void );

            virtual int                     ReleaseConnection( CConnection *pCon );

            virtual bool                    SetTimeout( int nTimeout );
            static void                    *GetFunction( const char *strName );

        private:
            static void                     initLibrary( void );
    };
} // namespace IASLib

#endif // IASLIB_DB_SQLITE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_SQLITEDATABSE_H__
