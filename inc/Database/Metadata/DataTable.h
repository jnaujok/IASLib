/*
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
    class CDataTable : public CObject
    {
        protected:
            CString             m_strName;
            CArray              m_aDataFields;
            CArray              m_aDataIndexes;
            CString             m_strInitialize;

        public:
                                CDataTable( const CXMLElement *pTableDefinition );
            virtual            ~CDataTable( void );

                                DEFINE_OBJECT( CDataTable )

                // The table from the definition: its columns, indexes and
                // the <initialize> statements (one per line), in one
                // transaction. False, with nothing left behind, if any part
                // fails or a name in the definition is not a valid SQL name.
                // (The commit's result was ignored, and an invalid name went
                // into the SQL as written.)
            bool                DropTable( CConnection *pConnection );
            bool                CreateTable( CConnection *pConnection );

            size_t              GetFieldCount( void ) const { return m_aDataFields.Length(); }
            size_t              GetIndexCount( void ) const { return m_aDataIndexes.Length(); }

            const char         *GetName( void ) const { return (const char *)m_strName; }
    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_DATATABLE_H__
