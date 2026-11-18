/**
 *  @file
 *  Data Table Class
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
        /// The definition of a database table, read from XML, that can create the table.
        ///
        /// The element holds "field" elements (CDataField), "index"
        /// elements (CDataIndex), and an optional "initialize" element
        /// whose text is SQL, one statement per line, run after the table
        /// is created:
        /// @code
        /// <table name="users">
        ///     <field name="id" type="integer" null="no"/>
        ///     <field name="name" type="varchar(64)"/>
        ///     <index name="users_id" unique="yes">id</index>
        ///     <initialize>insert into users values ( 0, 'admin' )</initialize>
        /// </table>
        /// @endcode
    class CDataTable : public CObject
    {
        protected:
            CString             m_strName;          ///< The table's name.
            CArray              m_aDataFields;      ///< Its columns (CDataField, owned).
            CArray              m_aDataIndexes;     ///< Its indexes (CDataIndex, owned).
            CString             m_strInitialize;    ///< SQL to run after creating it, one statement per line.

        public:
                                /// Reads a table definition from a "table" element.
                                /// @param pTableDefinition The element (not NULL; not kept).
                                CDataTable( const CXMLElement *pTableDefinition );
                                /// Deletes the field and index definitions.
            virtual            ~CDataTable( void );

                                DEFINE_OBJECT( CDataTable )

            /// Drops the table.
            /// @param pConnection The connection to run it on.
            /// @return False if the command failed.
            bool                DropTable( CConnection *pConnection );
            /// Creates the table, its indexes, and its initial rows, in one transaction.
            /// @param pConnection The connection to run it on.
            /// @return False if any statement failed (the transaction is then rolled back).
            bool                CreateTable( CConnection *pConnection );

            /// Reads a table's definition from the database.
            /// @note Declared but not implemented yet.
            bool                LoadTable( const char *strTableName, CConnection *pConnection );
            /// Same, for a CString name.
            /// @note Declared but not implemented yet.
            bool                LoadTable( CString strTableName, CConnection *pConnection );

            /// The table's name.
            const char         *GetName( void ) const { return (const char *)m_strName; }
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_DATATABLE_H__
