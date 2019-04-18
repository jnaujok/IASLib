/*
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

#define BULK_TYPE_INT       1
#define BULK_TYPE_FLOAT     2
#define BULK_TYPE_CHAR      3
#define BULK_TYPE_DATE      4
#define BULK_TYPE_VARCHAR   5

namespace IASLib
{
    class CBulkCopyConnection : public CObject
    {
        protected:
            CString                 m_strConnectionName;
            bool                    m_bConnected;

        public:
                                    CBulkCopyConnection( const char *strName = "" ) 
                                    { 
                                        m_strConnectionName = strName; 
                                        m_bConnected = false; 
                                    }

            virtual                ~CBulkCopyConnection( void ) 
                                    {
                                        // No current Destructor ops.
                                    }

                                    DECLARE_OBJECT( CBulkCopyConnection, CObject )

            virtual bool            Connected( void ) { return m_bConnected; }

            virtual bool            IsValid( void ) { return ( ( Connected() ) && ( ! IsDead() ) ); }

            virtual bool            IsDead( void ) = 0;

            virtual bool            SetTable( const char *strTableName ) = 0;
            virtual bool            BindColumn( int nColumnNumber, int nDataType, void *pDataBuffer, int nBufferLength ) = 0;
            virtual bool            SendRow( void ) = 0;
            virtual bool            Flush( void ) = 0;
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_BULKCOPYCONNECTION_H__

