/**
 *  @file
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
        /// The definition of one column of a CDataTable, read from XML.
        ///
        /// The element looks like
        /// @code
        /// <field name="id" type="integer" null="no">0</field>
        /// @endcode
        /// where null="no" (any case) makes the column NOT NULL and the
        /// text is the default value.
    class CDataField : public CObject
    {
        protected:
            CString         m_strFieldName;         ///< The column's name.
            CString         m_strFieldType;         ///< Its SQL type.
            CString         m_strFieldDefaultValue; ///< Its default value (read, but not used by CreateField()).
            bool            m_bFieldNullable;       ///< False for NOT NULL.
            bool            m_bIsPrimaryKey;        ///< Not set or used yet.
            bool            m_bIsForeignKey;        ///< Not set or used yet.
            CString         m_strForeignKeySource;  ///< Not set or used yet.

        public:
                            /// Reads a column definition from a "field" element.
                            /// @param pFieldDefinition The element (not NULL; not kept).
                            CDataField( const CXMLElement *pFieldDefinition );
                            /// Destroys the definition.
            virtual        ~CDataField( void );

                            DECLARE_OBJECT( CDataField, CObject );

            /// The column's part of a CREATE TABLE: "name type", plus " not null" if needed.
            CString         CreateField( void );
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_DATAFIELD_H__
