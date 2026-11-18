/**
 *  @file
 *  Base Result Class
 *
 *  This abstract class provides a common interface from the
 * results from a Stored Procedure (on databases that support
 * table results from a stored procedure) and results from 
 * a select statement.
 *  This class implements the common elements between these
 * two result types.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/28/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BASERESULT_H__
#define IASLIB_BASERESULT_H__

#include "../Collections/StringArray.h"
#include <stdlib.h>

#ifdef IASLIB_DATABASE__

namespace IASLib
{
        /// Rows of text read from a database: the base of cursors and result sets.
        ///
        /// Every value is held as a string. The rows are read in full when
        /// the result is made; the current row starts at 0 and moves with
        /// Next(), Prev() and SetRow(). Not thread-safe.
    class CBaseResult : public CObject
    {
        protected:
            size_t              m_nColumns;         ///< How many columns.
            size_t              m_nRows;            ///< How many rows.
            size_t             *m_anColumnWidths;   ///< Each column's width, where the vendor records it (may be NULL).
            CStringArray        m_astrHeaders;      ///< The column names.
            CStringArray      **m_aastrData;        ///< The rows, each an array of values (freed by the subclass's Close()).
            size_t              m_nCurrentRow;      ///< The current row.
            bool                m_bValid;           ///< True if the result holds data.

        public:
                                /// Creates an empty, invalid result.
                                CBaseResult( void );
                                /// Destroys the result (subclasses free the rows in Close()).
            virtual            ~CBaseResult( void );

                                DEFINE_OBJECT( CBaseResult )

            /// Frees the rows; the result is then invalid.
            /// @return 0, or a vendor error code.
            virtual int         Close( void ) = 0;

            /// A column's name ("" if the result is invalid or there is no such column).
            virtual const char *ColumnName( size_t nColumn );

            /// How many columns.
            virtual size_t      Columns( void ) { return m_nColumns; }
            /// How many rows.
            virtual size_t      Rows( void ) { return m_nRows; }

            /// True if the result holds data.
            virtual bool        IsValid( void ) { return m_bValid; }

            /// Moves to the next row.
            /// @return False if there is none (past the last row).
            virtual bool        Next( void );
            /// Moves to the previous row.
            /// @return False if already at the first row.
            virtual bool        Prev( void );

            /// A value of the current row, by column name (any case).
            /// @param pszColumn The column's name.
            /// @return The value, or "" if there is no such column.
            virtual CString     GetColumn( const char *pszColumn );
            /// A value of the current row, by column number.
            /// @param nColumn The column, from 0.
            /// @return The value, or "" if the result is invalid or there is no such
            ///         column or row.
            virtual CString     GetColumn( size_t nColumn );

            /// The same as GetColumn( nColumn ).
            virtual CString     operator []( size_t nColumn );

            /// True if there is a column with this name (any case).
            virtual bool        IsColumn( const char *pszColumn );

            /// Makes nRow the current row.
            /// @return False if there is no such row (the current row is unchanged).
            virtual bool        SetRow( size_t nRow );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__

#endif // IASLIB_BASERESULT_H__

