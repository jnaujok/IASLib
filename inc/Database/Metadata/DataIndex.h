/**
 *  @file
 *  Data Index Class
 *
 *  This class stores the meta-data for a database table index. Initially
 * this was used by the versioning system to generate database tables, but
 * now includes functionality to extract database metadata from a database
 * as well.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 05/01/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DATAINDEX_H__
#define IASLIB_DATAINDEX_H__

#include "../../BaseTypes/String_.h"
#include "../../Streams/Stream.h"
#include "../../XML/XMLElement.h"
#include "../Connection.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
        /// The definition of an index of a CDataTable, read from XML.
        ///
        /// The element looks like
        /// @code
        /// <index name="users_name" unique="yes">last_name, first_name</index>
        /// @endcode
        /// where unique="yes" (any case) makes a unique index and the text
        /// is the column list.
    class CDataIndex : public CObject
    {
        protected:
            CString         m_strIndexName;     ///< The index's name.
            CString         m_strIndexFields;   ///< Its column list, as SQL.
            bool            m_bUnique;          ///< True for a unique index.

        public:
                            /// Reads an index definition from an "index" element.
                            /// @param pIndexDefinition The element (not NULL; not kept).
                            CDataIndex( const CXMLElement *pIndexDefinition );
                            /// Destroys the definition.
            virtual        ~CDataIndex( void );

                            DECLARE_OBJECT(CDataIndex,CObject);

            /// Creates the index on a table ("create [unique] index ...").
            /// @param strTableName The table.
            /// @param pCon         The connection to run it on.
            /// @return False if the command failed.
            bool            CreateIndex( const char *strTableName, CConnection *pCon );
            /// Drops the index ("drop index table.index").
            /// @param strTableName The table.
            /// @param pCon         The connection to run it on.
            /// @return False if the command failed.
            bool            DropIndex( const char *strTableName, CConnection *pCon );

            /// Reads an index's definition from the database.
            /// @note Declared but not implemented yet.
            bool            LoadIndex( const char *strIndexName, CConnection *pCon );
            /// Same, for a CString name.
            /// @note Declared but not implemented yet.
            bool            LoadIndex( CString strIndexName, CConnection *pCon );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAINDEX_H__
