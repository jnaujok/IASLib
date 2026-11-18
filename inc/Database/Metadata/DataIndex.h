/**
 * @file
 * @brief Data Index Class.
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
    /// One index of a CDataTable, read from an \<index\> element of a table
    /// definition: name="...", unique="yes" for a unique index, and the
    /// element's text as the column list ("a, b DESC").
    class CDataIndex : public CObject
    {
        protected:
            CString         m_strIndexName;     ///< The index's name.
            CString         m_strIndexFields;   ///< The column list.
            bool            m_bUnique;          ///< Whether the index is unique.

        public:
                            /// Reads an \<index\> element.
                            CDataIndex( const CXMLElement *pIndexDefinition );
            virtual        ~CDataIndex( void );     ///< Destructor.

                            DECLARE_OBJECT(CDataIndex,CObject);

                /// Creates the index on @p strTableName. False if a name or the
                /// column list is not valid SQL, or the statement fails.
            bool            CreateIndex( const char *strTableName, CConnection *pCon );
                /// Drops the index ("drop index table.index" on Sybase, "drop
                /// index index" elsewhere). False if a name is not valid, or the
                /// statement fails.
                /// @note Only the Sybase form used to be written.
            bool            DropIndex( const char *strTableName, CConnection *pCon );

    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAINDEX_H__
