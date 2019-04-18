/*
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
    class CDataIndex : public CObject
    {
        protected:
            CString         m_strIndexName;
            CString         m_strIndexFields;
            bool            m_bUnique;

        public:
                            CDataIndex( const CXMLElement *pIndexDefinition );
            virtual        ~CDataIndex( void );

                            DECLARE_OBJECT(CDataIndex,CObject);

            bool            CreateIndex( const char *strTableName, CConnection *pCon );
            bool            DropIndex( const char *strTableName, CConnection *pCon );

            bool            LoadIndex( const char *strIndexName, CConnection *pCon );
            bool            LoadIndex( CString strIndexName, CConnection *pCon );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAINDEX_H__
