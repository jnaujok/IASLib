/*
 *  Data Field Class
 *
 *  This class stores the meta-data for a field within a database table
 * object. 
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

#ifndef IASLIB_DATAFIELD_H__
#define IASLIB_DATAFIELD_H__

#include "../../BaseTypes/String_.h"
#include "../../Streams/Stream.h"
#include "../../XML/XMLElement.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CDataField : public CObject
    {
        protected:
            CString         m_strFieldName;
            CString         m_strFieldType;
            CString         m_strFieldDefaultValue;
            bool            m_bFieldNullable;
            bool            m_bIsPrimaryKey;
            bool            m_bIsForeignKey;
            CString         m_strForeignKeySource;

        public:
                            CDataField( const CXMLElement *pFieldDefinition );
            virtual        ~CDataField( void );

                            DECLARE_OBJECT( CDataField, CObject );

            CString         CreateField( void );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAFIELD_H__
