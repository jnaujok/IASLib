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

                // The column's definition for CREATE TABLE ("name type
                // [not null]"), or "" when the name or type is not valid.
            CString         CreateField( void );
            const char     *GetName( void ) const { return (const char *)m_strFieldName; }

                // SQL names from a definition file are put into statements
                // as they are, so they are checked first: letters, digits,
                // '_' and '$' (not first), and '.' between parts.
            static bool     IsIdentifier( const char *strName );
                // A column type: letters, digits, spaces, '_' and "(10,2)".
            static bool     IsColumnType( const char *strType );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAFIELD_H__
