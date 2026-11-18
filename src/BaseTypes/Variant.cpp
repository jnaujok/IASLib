/**
 * CVariant Class
 *
 * C++ like c is a strongly typed language, but every once in a while it's 
 * really handy to have a variable you can just throw anything into. This
 * class provides a Variant type that can be used for that.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/26/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Variant.h"
#include "String_.h"
#include "Array.h"

#include <climits>
#include <cmath>
#include <cstdlib>

namespace IASLib
{
    IMPLEMENT_OBJECT( CVariant, CObject );

    namespace
    {
        double ReadNumber( const CString &strText )
        {
            CString strTrimmed = strText;
            strTrimmed.Trim();
            if ( strTrimmed.Compare( "true", false ) == 0 )
                return 1.0;
            if ( strTrimmed.Compare( "false", false ) == 0 )
                return 0.0;
            char *pEnd = NULL;
            double dValue = strtod( strTrimmed.c_str(), &pEnd );
            return ( pEnd != NULL && pEnd != strTrimmed.c_str() ) ? dValue : 0.0;
        }

        template <class T> T Clamp( double dValue, T nMin, T nMax )
        {
            if ( dValue != dValue )
                return 0;
            if ( dValue <= (double)nMin )
                return nMin;
            if ( dValue >= (double)nMax )
                return nMax;
            return (T)dValue;
        }
    }

    CVariant::CVariant( const CVariant &oSource ) : CObject( oSource )
    {
        init();
        *this = oSource;
    }

    CVariant::~CVariant( void )
    {
    }

    void CVariant::init( void )
    {
        valType = UNKNOWN;
        strVal.Clear();
        dateVal = CDate();
        intVal = 0;
        longVal = 0L;
        floatVal = 0.0f;
        doubleVal = 0.0;
        boolVal = false;
        arrayVal = NULL;
        objVal = NULL;
    }

    void CVariant::clear( VARIANT_TYPE newType )
    {
            // Only the string is worth freeing; the other values are
            // overwritten or ignored.
        if ( valType == STRING && newType != STRING )
            strVal.Clear();
        valType = newType;
    }

    CVariant &CVariant::operator =( const CVariant &oSource )
    {
        if ( this != &oSource )
        {
            valType = oSource.valType;
            strVal = oSource.strVal;
            dateVal = oSource.dateVal;
            intVal = oSource.intVal;
            longVal = oSource.longVal;
            floatVal = oSource.floatVal;
            doubleVal = oSource.doubleVal;
            boolVal = oSource.boolVal;
            arrayVal = oSource.arrayVal;
            objVal = oSource.objVal;
        }
        return *this;
    }

    bool CVariant::isNumber( void ) const
    {
        return valType == INTEGER || valType == LONG || valType == FLOAT || valType == DOUBLE;
    }

    CString CVariant::getString( void ) const
    {
        CString temp;

        switch ( valType )
        {
            case STRING:
                return strVal;

            case DATE:
                return dateVal.FormatDate();

            case INTEGER:
                temp = intVal;
                return temp;

            case LONG:
                temp = longVal;
                return temp;

            case FLOAT:
                temp = floatVal;
                return temp;

            case DOUBLE:
                temp = doubleVal;
                return temp;

            case BOOLEAN:
                temp = boolVal;
                return temp;

            case ARRAY:
                if ( arrayVal == NULL )
                    return CString( "[null]" );
                return CString::FormatString( "[%lu elements]", (unsigned long)arrayVal->Length() );

            case OBJECT:
                if ( objVal == NULL )
                    return CString( "Object[null]" );
            #ifdef IASLIB_RTTI__
                return CString::FormatString( "Object[%s(%lu)]", objVal->GetType(), objVal->GetID() );
            #else
                return CString::FormatString( "Object[%lu]", objVal->GetID() );
            #endif

            case UNKNOWN:
            default:
                return CString();
        }
    }

    CDate CVariant::getDate( void ) const
    {
        return ( valType == DATE ) ? dateVal : CDate();
    }

    double CVariant::getDouble( void ) const
    {
        switch ( valType )
        {
            case INTEGER:   return (double)intVal;
            case LONG:      return (double)longVal;
            case FLOAT:     return (double)floatVal;
            case DOUBLE:    return doubleVal;
            case BOOLEAN:   return boolVal ? 1.0 : 0.0;
            case STRING:    return ReadNumber( strVal );
            default:        return 0.0;
        }
    }

    long CVariant::getLong( void ) const
    {
        switch ( valType )
        {
            case INTEGER:   return (long)intVal;
            case LONG:      return longVal;
            case BOOLEAN:   return boolVal ? 1L : 0L;
            case STRING:
            {
                    // Integers read exactly (a double would lose digits).
                CString strTrimmed = strVal;
                strTrimmed.Trim();
                char *pEnd = NULL;
                long nValue = strtol( strTrimmed.c_str(), &pEnd, 10 );
                if ( pEnd != NULL && *pEnd == 0 && pEnd != strTrimmed.c_str() )
                    return nValue;
                return Clamp<long>( ReadNumber( strVal ), LONG_MIN, LONG_MAX );
            }
            default:        return Clamp<long>( getDouble(), LONG_MIN, LONG_MAX );
        }
    }

    int CVariant::getInt( void ) const
    {
        if ( valType == INTEGER )
            return intVal;
        long nValue = getLong();
        if ( nValue < INT_MIN )
            return INT_MIN;
        if ( nValue > INT_MAX )
            return INT_MAX;
        return (int)nValue;
    }

    float CVariant::getFloat( void ) const
    {
        return ( valType == FLOAT ) ? floatVal : (float)getDouble();
    }

    bool CVariant::getBoolean( void ) const
    {
        switch ( valType )
        {
            case BOOLEAN:   return boolVal;
            case INTEGER:   return intVal != 0;
            case LONG:      return longVal != 0;
            case FLOAT:     return floatVal != 0.0f;
            case DOUBLE:    return doubleVal != 0.0;
            case STRING:
            {
                CString strTrimmed = strVal;
                strTrimmed.Trim();
                if ( strTrimmed.Compare( "true", false ) == 0 || strTrimmed.Compare( "yes", false ) == 0 || strTrimmed.Compare( "on", false ) == 0 )
                    return true;
                return ReadNumber( strTrimmed ) != 0.0;
            }
            default:        return false;
        }
    }

    CArray *CVariant::getArray( void ) const
    {
        return ( valType == ARRAY ) ? arrayVal : NULL;
    }

    CObject *CVariant::getObject( void ) const
    {
        return ( valType == OBJECT ) ? objVal : NULL;
    }

} // namespace IASLib
