/**
 *  @file
 *  Bulk Copy Connection Class
 *
 *  This abstract class provides a base interface for database connections
 * that are used to do bulk copies. In some cases, this is the same as a 
 * regular connection, but (so far only for Sybase) it may be a special
 * connection that is optomized for the fast movement of large data blocks.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/05/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BULKCOPYCONNECTION_H__
#define IASLIB_BULKCOPYCONNECTION_H__

#include "../BaseTypes/String_.h"
#include "../Streams/Stream.h"
#include "StoredProc.h"
#include "Cursor.h"

#ifdef IASLIB_DATABASE__

/// BindColumn() data type: an int.
#define BULK_TYPE_INT       1
/// BindColumn() data type: a floating-point number.
#define BULK_TYPE_FLOAT     2
/// BindColumn() data type: fixed-length characters.
#define BULK_TYPE_CHAR      3
/// BindColumn() data type: a date.
#define BULK_TYPE_DATE      4
/// BindColumn() data type: variable-length characters.
#define BULK_TYPE_VARCHAR   5

namespace IASLib
{
        /// A connection for copying many rows into a table quickly.
        ///
        /// Made by CDatabase::BulkCopyConnection(), although no database
        /// here makes one yet (CSybaseBulkCopyConnection is the only
        /// implementation). Pick a table with SetTable(), bind a
        /// buffer to each column with BindColumn(), then fill the buffers
        /// and call SendRow() for each row, and Flush() at the end.
        ///
        /// @code
        /// int nID; char achName[ 32 ];
        /// pBulk->SetTable( "users" );
        /// pBulk->BindColumn( 1, BULK_TYPE_INT, &nID, sizeof( nID ) );
        /// pBulk->BindColumn( 2, BULK_TYPE_VARCHAR, achName, sizeof( achName ) );
        /// for ( ... ) { nID = ...; strcpy( achName, ... ); pBulk->SendRow(); }
        /// pBulk->Flush();
        /// @endcode
    class CBulkCopyConnection : public CObject
    {
        protected:
            CString                 m_strConnectionName;    ///< The connection's name.
            bool                    m_bConnected;           ///< True while connected.

        public:
                                    /// Creates an unconnected bulk copy connection.
                                    /// @param strName A name for the connection.
                                    CBulkCopyConnection( const char *strName = "" ) 
                                    { 
                                        m_strConnectionName = strName; 
                                        m_bConnected = false; 
                                    }

                                    /// Destroys the connection.
            virtual                ~CBulkCopyConnection( void ) 
                                    {
                                        // No current Destructor ops.
                                    }

                                    DECLARE_OBJECT( CBulkCopyConnection, CObject )

            /// True while connected.
            virtual bool            Connected( void ) { return m_bConnected; }

            /// True if connected and not dead.
            virtual bool            IsValid( void ) { return ( ( Connected() ) && ( ! IsDead() ) ); }

            /// True if the connection has been lost or closed.
            virtual bool            IsDead( void ) = 0;

            /// Starts a bulk copy into a table.
            /// @return False if the table cannot be used.
            virtual bool            SetTable( const char *strTableName ) = 0;
            /// Binds a buffer to a column of the table; SendRow() reads it.
            /// @param nColumnNumber The column, from 1.
            /// @param nDataType     A BULK_TYPE_* value.
            /// @param pDataBuffer   The buffer (not owned; it must stay valid while rows are sent).
            /// @param nBufferLength The buffer's length in bytes.
            /// @return False if the binding was refused.
            virtual bool            BindColumn( int nColumnNumber, int nDataType, void *pDataBuffer, int nBufferLength ) = 0;
            /// Sends one row from the bound buffers.
            /// @return False if it failed.
            virtual bool            SendRow( void ) = 0;
            /// Sends the rows still buffered and ends the batch.
            /// @return False if it failed.
            virtual bool            Flush( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_BULKCOPYCONNECTION_H__

