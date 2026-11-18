/**
 * @file
 * @brief Data Table Class.
 *
 *  This class stores the meta-data for a complete database table. Initially
 * this was used by the versioning system to generate database tables, but
 * now includes functionality to extract database metadata from a database
 * as well.
 *  Database tables can be loaded to and from an XML document.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/01/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DATATABLE_H__
#define IASLIB_DATATABLE_H__

#include "../../BaseTypes/String_.h"
#include "../../Collections/Array.h"
#include "../../XML/XMLElement.h"
#include "DataField.h"
#include "DataIndex.h"

#include "../Database.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    /// A database table's definition, read from a \<table name="..."\>
    /// element: \<field\> elements (CDataField), \<index\> elements
    /// (CDataIndex) and an optional \<initialize\> element of SQL
    /// statements, one per line. The table owns its fields and indexes.
    ///
    /// @code
    /// CDataTable oTable( pTableElement );     // from a parsed XML document
    /// if ( ! oTable.CreateTable( pConnection ) )
    ///     printf( "%s\n", pConnection->MajorErrorMessage() );
    /// @endcode
    class CDataTable : public CObject
    {
        protected:
            CString             m_strName;          ///< The table's name.
            CArray              m_aDataFields;      ///< The CDataField objects (owned).
            CArray              m_aDataIndexes;     ///< The CDataIndex objects (owned).
            CString             m_strInitialize;    ///< The \<initialize\> statements, one per line.

        public:
                                /// Reads a \<table\> element.
                                CDataTable( const CXMLElement *pTableDefinition );
                                /// Destructor. Deletes the fields and indexes.
            virtual            ~CDataTable( void );

                                DEFINE_OBJECT( CDataTable )

                /// Drops the table. False if its name is not a valid SQL name,
                /// or the statement fails.
            bool                DropTable( CConnection *pConnection );
                /// The table from the definition: its columns, indexes and
                /// the \<initialize\> statements (one per line), in one
                /// transaction. False, with nothing left behind, if any part
                /// fails or a name in the definition is not a valid SQL name.
                /// @note The commit's result used to be ignored, and an invalid
                /// name went into the SQL as written.
            bool                CreateTable( CConnection *pConnection );

            size_t              GetFieldCount( void ) const { return m_aDataFields.Length(); }      ///< The number of columns.
            size_t              GetIndexCount( void ) const { return m_aDataIndexes.Length(); }     ///< The number of indexes.

            const char         *GetName( void ) const { return (const char *)m_strName; }        ///< The table's name.
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_DATATABLE_H__
