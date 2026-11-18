/**
 *  @file
 *  Cursor Class
 *
 *  This class stores the result of a selection and provides for deletion
 * and updates.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CURSOR_H__
#define IASLIB_CURSOR_H__

#include "BaseResult.h"
#include "../Collections/StringArray.h"
#include <stdlib.h>

#ifdef IASLIB_DATABASE__

namespace IASLib
{
        /// The rows of a SELECT, with row changes where the vendor supports them.
        ///
        /// Made by CConnection::Cursor(); the caller deletes it. For an
        /// updatable cursor, SetColumn() sets the new values of the current
        /// row, and InsertRow() or UpdateRow() writes them to a table
        /// (SQLite does not support row changes). Not thread-safe.
    class CCursor : public CBaseResult
    {
        protected:
            CStringArray        m_astrNewValues;    ///< The new values for InsertRow() and UpdateRow().
            bool                m_bUpdatable;       ///< True if the cursor may change rows.

        public:
                                /// Creates an empty, read-only cursor.
                                CCursor( void ) { m_bUpdatable = false; }
                                /// Destroys the cursor.
            virtual            ~CCursor( void ) { }

                                DECLARE_OBJECT( CCursor, CBaseResult );

            /// Deletes the current row from a table.
            /// @param strTable The table.
            /// @return 1 if done, 0 if the SQL failed, or a negative IASLIB_DB_* code
            ///         (IASLIB_DB_CURSOR_READ_ONLY, IASLIB_DB_CURSOR_INVALID_ROW, or
            ///         IASLIB_DB_ERROR_UNSUPPORTED for SQLite). Sybase returns 1 for a
            ///         read-only cursor or a bad row.
            virtual int         DeleteRow( const char *strTable ) = 0;
            /// Inserts the new values as a row of a table.
            /// @copydetails DeleteRow()
            virtual int         InsertRow( const char *strTable ) = 0;
            /// Writes the new values over the current row of a table.
            /// @copydetails DeleteRow()
            virtual int         UpdateRow( const char *strTable ) = 0;

            /// Sets a column's new value, by number.
            /// @param nColumn  The column, from 0.
            /// @param strValue The value.
            /// @return 1, or 0 if the cursor is invalid or there is no such column.
            virtual int         SetColumn( size_t nColumn, const char *strValue )
                                {
                                    if ( ( m_bValid != 0 ) && ( nColumn < m_nColumns ) )
                                    {
                                        m_astrNewValues[ nColumn ] = strValue;
                                        return 1;
                                    }
                                    return 0;
                                }

            /// Sets a column's new value to a number, by column number.
            /// @param nColumn The column, from 0.
            /// @param nValue  The value.
            /// @return 1, or 0 if the cursor is invalid or there is no such column.
            virtual int         SetColumn( size_t nColumn, size_t nValue )
                                {
                                    if ( ( m_bValid != 0 ) && ( nColumn < m_nColumns ) )
                                    {
                                        m_astrNewValues[ nColumn ] = nValue;
                                        return 1;
                                    }
                                    return 0;
                                }

            /// Sets a column's new value, by name (exact case).
            /// @param pszColumn The column's name.
            /// @param strValue  The value.
            /// @return 1, or 0 if there is no such column.
            virtual size_t      SetColumn( const char *pszColumn, const char *strValue )
                                {
                                    CString strColumn = pszColumn;
                                    for ( size_t nCount = 0; nCount < m_nColumns; nCount ++ )
                                    {
                                        if ( m_astrHeaders[ nCount ] == strColumn )
                                        {
                                            m_astrNewValues[ nCount ] = strValue;
                                            return 1;
                                        }
                                    }
                                    return 0;
                                }

            /// Sets a column's new value to a number, by name (exact case).
            /// @param pszColumn The column's name.
            /// @param nValue    The value.
            /// @return 1, or 0 if there is no such column.
            virtual size_t      SetColumn( const char *pszColumn, int nValue )
                                {
                                    CString strColumn = pszColumn;
                                    for ( size_t nCount = 0; nCount < m_nColumns; nCount ++ )
                                    {
                                        if ( m_astrHeaders[ nCount ] == strColumn )
                                        {
                                            m_astrNewValues[ nCount ].Format( "%d", nValue );
                                            return 1;
                                        }
                                    }
                                    return 0;
                                }

    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CURSOR_H__

