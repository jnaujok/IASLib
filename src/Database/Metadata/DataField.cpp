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

#include "String_.h"
#include "DataField.h"

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    CDataField::CDataField( const CXMLElement *pFieldDefinition )
    {   
        CString  strNullable;

        m_strFieldName = pFieldDefinition->GetProperty( "name" );
        m_strFieldType = pFieldDefinition->GetProperty( "type" );
        strNullable = pFieldDefinition->GetProperty( "null" );
        strNullable.ToUpperCase();

        m_bFieldNullable = ( strNullable != "NO" );

        m_strFieldDefaultValue = pFieldDefinition->GetData();
    }

    CDataField::~CDataField( void )
    {
        // Nothing to do for now
    }

    bool CDataField::IsIdentifier( const char *strName )
    {
        if ( strName == NULL || *strName == '\0' )
            return false;
        bool bStart = true;
        for ( const char *p = strName; *p; p++ )
        {
            char c = *p;
            bool bLetter = ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c == '_';
            bool bDigit = ( c >= '0' && c <= '9' ) || c == '$';
            if ( c == '.' )
            {
                if ( bStart || p[1] == '\0' )
                    return false;
                bStart = true;
                continue;
            }
            if ( bStart ? ! bLetter : ! ( bLetter || bDigit ) )
                return false;
            bStart = false;
        }
        return true;
    }

    bool CDataField::IsColumnType( const char *strType )
    {
        if ( strType == NULL || *strType == '\0' )
            return false;
        for ( const char *p = strType; *p; p++ )
        {
            char c = *p;
            if ( ! ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) ||
                     c == '_' || c == ' ' || c == '(' || c == ')' || c == ',' ) )
                return false;
        }
        return true;
    }

    CString CDataField::CreateField( void )
    {
        CString     strRetVal;

        if ( ! IsIdentifier( m_strFieldName ) || ! IsColumnType( m_strFieldType ) )
            return strRetVal;

        strRetVal.Format( "%s %s%s", (const char *)m_strFieldName, (const char *)m_strFieldType, (m_bFieldNullable) ? "" : " not null" );

        return strRetVal;
    }
} // namespace IASLib

#endif // IASLIB_DATABASE__
