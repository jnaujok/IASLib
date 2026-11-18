/***********************************************************************
**  CSybaseCursor
**
**  Description:
**      This class defines and implements a Sybase cursor which is used
** to store the results from a SQL select statement.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

/**
 * @file
 * @brief CSybaseCursor, the rows of a Sybase SELECT.
 */

#ifndef __SYB_CURSOR_H__
#define __SYB_CURSOR_H__

#ifdef GENLIB_INCLUDE_SYBASE

#include "Cursor.h"

#include <ctpublic.h>

class CSybaseConnection;

        /// The rows of a Sybase SELECT, read in full when it is made.
        ///
        /// Made by CSybaseConnection::Cursor(); the caller deletes it. On
        /// an updatable cursor, moving to a row copies its values into the
        /// new values.
        /// @note Legacy code: it is built only with GENLIB_INCLUDE_SYBASE (the
        /// old library's switch), sits outside the IASLib namespace, and has
        /// not been brought up to date with the current base classes.
class CSybaseCursor : public CCursor
{
    protected:
        CSybaseConnection  *m_pConnection;  ///< The connection that ran the query (not owned).
        CS_INT             *m_apnTypes;     ///< Each column's Client-Library data type.

    public:
                            /// Reads every row of the command's results.
                            /// @param pConnection The connection.
                            /// @param pCommand    The command that ran the query.
                            /// @param bUpdatable  True to allow row changes.
                            CSybaseCursor( CSybaseConnection *pConnection, CS_COMMAND *pCommand, bool bUpdatable );
                            /// Frees the rows.
        virtual            ~CSybaseCursor( void );

        /// Frees the rows. @return 0.
        virtual int         Close( void );
        /// @copydoc IASLib::CCursor::DeleteRow()
        virtual int         DeleteRow( const char *strTable );
        /// @copydoc IASLib::CCursor::InsertRow()
        virtual int         InsertRow( const char *strTable );
        /// @copydoc IASLib::CCursor::UpdateRow()
        virtual int         UpdateRow( const char *strTable );

        /// Moves to the next row (and, if updatable, copies it into the new values).
        virtual bool        Next( void );
        /// Moves to the previous row (and, if updatable, copies it into the new values).
        virtual bool        Prev( void );
};

#endif

#endif
