/**
 * @file
 * @brief Base Result Class.
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

#include "../BaseTypes/DataBlock.h"

#include <map>
#include <vector>

namespace IASLib
{
        /// A result held in memory: column names, and rows of values as
        /// strings. The current row starts at the first; Next() and Prev()
        /// move it, and GetColumn() reads it ("" past the end, or for a SQL
        /// NULL: IsNull() tells them apart where the driver records NULLs).
        /// Column names are matched without regard to case.
        ///
        /// This is the base of the drivers' result and cursor classes; it
        /// cannot be copied.
        ///
        /// @code
        /// CBaseResult *pResult = pConnection->Cursor( "SELECT id, name FROM people" );
        /// if ( pResult && pResult->IsValid() && pResult->Rows() > 0 )
        /// {
        ///     do
        ///     {
        ///         printf( "%s\n", (const char *)pResult->GetColumn( "name" ) );
        ///     } while ( pResult->Next() );
        /// }
        /// delete pResult;         // the caller owns it
        /// @endcode
    class CBaseResult : public CObject
    {
        protected:
            size_t              m_nColumns;         ///< The number of columns.
            size_t              m_nRows;            ///< The number of rows.
            size_t             *m_anColumnWidths;   ///< Column widths, for drivers that record them (or NULL).
            CStringArray        m_astrHeaders;      ///< The column names.
            CStringArray      **m_aastrData;        ///< The rows (each owned), or NULL.
            size_t              m_nCurrentRow;      ///< The current row.
            bool                m_bValid;           ///< Whether the result holds data.
                /// Per row, per column: 1 for a SQL NULL (drivers that record
                /// NULLs fill it; empty otherwise).
            std::vector< std::vector<unsigned char> > m_aabNull;
                /// Per row: the binary values, by column. A blob's text
                /// column is empty; GetBlob() gives its bytes.
            std::vector< std::map<size_t, CDataBlock> > m_aaBlobs;
            size_t              m_nAllocatedRows;   ///< Room in m_aastrData (0 for drivers that keep their own rows).

                /// For drivers: adds a row (taking the array), with which of
                /// its columns are NULL (may be NULL: none are) and its binary
                /// values (may be NULL: none).
            void                AddRow( CStringArray *pRow, const std::vector<unsigned char> *pNulls = NULL, const std::map<size_t, CDataBlock> *pBlobs = NULL );
                /// Frees every row (the headers stay).
            void                ClearRows( void );
                /// The index of a column by name, or NOT_FOUND.
            size_t              FindColumn( const char *pszColumn ) const;

        public:
                                /// Makes an empty, invalid result.
                                CBaseResult( void );
                                /// Destructor. Frees rows added with AddRow().
            virtual            ~CBaseResult( void );

                                DEFINE_OBJECT( CBaseResult )

                /// Releases the result (driver specific).
            virtual int         Close( void ) = 0;

                /// The name of column @p nColumn, or "" if there is none.
            virtual const char *ColumnName( size_t nColumn );
            virtual size_t      Columns( void ) { return m_nColumns; }      ///< The number of columns.
            virtual size_t      Rows( void ) { return m_nRows; }            ///< The number of rows.
            virtual bool        IsValid( void ) { return m_bValid; }        ///< Whether the result holds data.
                /// Moves to the next row: false (staying at the last) if there
                /// is none.
                /// @note It used to keep counting past the end.
            virtual bool        Next( void );
                /// Moves to the previous row: false (staying at the first) if
                /// there is none.
            virtual bool        Prev( void );
                /// The current row's value in the named column ("" if there is
                /// no such column, no row, or the value is NULL).
                /// @note It used to read past the data after the last row.
            virtual CString     GetColumn( const char *pszColumn );
                /// Same, by column number.
            virtual CString     GetColumn( size_t nColumn );
                /// Same as GetColumn( nColumn ).
            virtual CString     operator []( size_t nColumn );
                /// Whether there is a column of that name.
            virtual bool        IsColumn( const char *pszColumn );
                /// Makes @p nRow the current row. False if there is no such row.
            virtual bool        SetRow( size_t nRow );
            size_t              GetRow( void ) const { return m_nCurrentRow; }  ///< The current row.
                /// Whether the current row's value is SQL NULL (false where the
                /// driver does not record NULLs).
            virtual bool        IsNull( size_t nColumn );
            virtual bool        IsNull( const char *pszColumn );    ///< Same, by column name.
                /// Whether the current row's value is binary (a BLOB).
            virtual bool        IsBlob( size_t nColumn );
            virtual bool        IsBlob( const char *pszColumn );    ///< Same, by column name.
                /// The current row's value as bytes: a BLOB's bytes, any other
                /// value's text, and an empty block for a NULL.
                /// @note Blobs used to come back as strings, which stop at a
                /// zero byte where they are used as text.
            virtual CDataBlock  GetBlob( size_t nColumn );
            virtual CDataBlock  GetBlob( const char *pszColumn );   ///< Same, by column name (empty if there is none).

        private:
                                CBaseResult( const CBaseResult & );     ///< Not copyable.
            CBaseResult        &operator =( const CBaseResult & );     ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__

#endif // IASLIB_BASERESULT_H__

