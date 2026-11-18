/**
 * @file
 * @brief Data Field Class.
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
    /// One column of a CDataTable, read from a \<field\> element of a
    /// table definition: name="...", type="...", null="no" for NOT NULL,
    /// and the element's text as the default value.
    class CDataField : public CObject
    {
        protected:
            CString         m_strFieldName;         ///< The column's name.
            CString         m_strFieldType;         ///< The column's SQL type.
            CString         m_strFieldDefaultValue; ///< The element's text (the default value).
            bool            m_bFieldNullable;       ///< False for NOT NULL.
            bool            m_bIsPrimaryKey;        ///< Part of the primary key (not set from the definition).
            bool            m_bIsForeignKey;        ///< A foreign key (not set from the definition).
            CString         m_strForeignKeySource;  ///< What a foreign key refers to (not set from the definition).

        public:
                            /// Reads a \<field\> element.
                            CDataField( const CXMLElement *pFieldDefinition );
            virtual        ~CDataField( void );     ///< Destructor.

                            DECLARE_OBJECT( CDataField, CObject );

                /// The column's definition for CREATE TABLE ("name type
                /// [not null]"), or "" when the name or type is not valid.
            CString         CreateField( void );
            const char     *GetName( void ) const { return (const char *)m_strFieldName; }    ///< The column's name.

                /// Whether @p strName is a valid SQL name.
                /// SQL names from a definition file are put into statements
                /// as they are, so they are checked first: letters, digits,
                /// '_' and '$' (not first), and '.' between parts.
            static bool     IsIdentifier( const char *strName );
                /// Whether @p strType is a column type: letters, digits, spaces,
                /// '_' and "(10,2)".
            static bool     IsColumnType( const char *strType );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAFIELD_H__
