/**
 *  @file
 *  COracleCursor
 *
 *  This class defines an Oracle 7.3 cursor that is used to store the 
 * results from a SQL select statement.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/19/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_ORA_CURSOR_H__
#define IASLIB_ORA_CURSOR_H__

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "Cursor.h"

extern "C"
{
#include <oci.h>
}

namespace IASLib
{
    class COracleConnection;

        /// The rows of an Oracle SELECT, read in full when it is made.
        ///
        /// Made by COracleConnection::Cursor(); the caller deletes it. On
        /// an updatable cursor, moving to a row copies its values into the
        /// new values, and the row changes are built as SQL with every
        /// column in the WHERE clause.
    class COracleCursor : public CCursor
    {
        protected:
            COracleConnection  *m_pConnection;  ///< The connection that ran the query (not owned).
            ub2                *m_apnTypes;     ///< Each column's OCI data type.
            Cda_Def            *m_pCDA;         ///< The OCI cursor data area (owned).

        public:
                                /// Runs the parsed query and reads every row (check IsValid()).
                                /// @param pConnection The connection.
                                /// @param pLDA        Its logon data area.
                                /// @param pCDA        The parsed cursor (owned from now on).
                                /// @param bUpdatable  True to allow row changes.
                                COracleCursor( COracleConnection *pConnection, Lda_Def *pLDA, Cda_Def *pCDA, bool bUpdatable );
                                /// Closes the cursor.
            virtual            ~COracleCursor( void );

                                DEFINE_OBJECT( COracleCursor )

            /// Frees the rows and the cursor data area.
            /// @return IASLIB_DB_OK.
            virtual int         Close( void );
            /// @copydoc CCursor::DeleteRow()
            virtual int         DeleteRow( const char *strTable );
            /// @copydoc CCursor::InsertRow()
            virtual int         InsertRow( const char *strTable );
            /// @copydoc CCursor::UpdateRow()
            virtual int         UpdateRow( const char *strTable );

            /// Moves to the next row (and, if updatable, copies it into the new values).
            /// @return False if there is none.
            virtual bool        Next( void );
            /// Moves to the previous row (and, if updatable, copies it into the new values).
            /// @return False if already at the first row.
            virtual bool        Prev( void );
        private:
            /// True if a column's type needs its values quoted in SQL (character, date and LOB types).
            bool                NeedsQuotes( size_t nColumn );
    };
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
#endif // IASLIB_ORA_CURSOR_H__
