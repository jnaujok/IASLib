/*
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

#include <vector>

namespace IASLib
{
        // A result held in memory: column names, and rows of values as
        // strings. The current row starts at the first; Next() and Prev()
        // move it, and GetColumn() reads it ("" past the end, or for a SQL
        // NULL: IsNull() tells them apart where the driver records NULLs).
        // Column names are matched without regard to case.
    class CBaseResult : public CObject
    {
        protected:
            size_t              m_nColumns;
            size_t              m_nRows;
            size_t             *m_anColumnWidths;
            CStringArray        m_astrHeaders;
            CStringArray      **m_aastrData;
            size_t              m_nCurrentRow;
            bool                m_bValid;
                // Per row, per column: 1 for a SQL NULL (drivers that record
                // NULLs fill it; empty otherwise).
            std::vector< std::vector<unsigned char> > m_aabNull;
            size_t              m_nAllocatedRows;

                // For drivers: adds a row (taking the array), with which of
                // its columns are NULL (may be NULL: none are).
            void                AddRow( CStringArray *pRow, const std::vector<unsigned char> *pNulls = NULL );
                // Frees every row (the headers stay).
            void                ClearRows( void );
                // The index of a column by name, or NOT_FOUND.
            size_t              FindColumn( const char *pszColumn ) const;

        public:
                                CBaseResult( void );
            virtual            ~CBaseResult( void );

                                DEFINE_OBJECT( CBaseResult )

            virtual int         Close( void ) = 0;

            virtual const char *ColumnName( size_t nColumn );
            virtual size_t      Columns( void ) { return m_nColumns; }
            virtual size_t      Rows( void ) { return m_nRows; }
            virtual bool        IsValid( void ) { return m_bValid; }
                // Moves to the next row: false (staying at the last) if there
                // is none. (It kept counting past the end.)
            virtual bool        Next( void );
            virtual bool        Prev( void );
            virtual CString     GetColumn( const char *pszColumn );
            virtual CString     GetColumn( size_t nColumn );
            virtual CString     operator []( size_t nColumn );
            virtual bool        IsColumn( const char *pszColumn );
            virtual bool        SetRow( size_t nRow );
            size_t              GetRow( void ) const { return m_nCurrentRow; }
                // Whether the current row's value is SQL NULL.
            virtual bool        IsNull( size_t nColumn );
            virtual bool        IsNull( const char *pszColumn );

        private:
                                CBaseResult( const CBaseResult & );
            CBaseResult        &operator =( const CBaseResult & );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__

#endif // IASLIB_BASERESULT_H__

