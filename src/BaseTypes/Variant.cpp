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

namespace IASLib
{
    IMPLEMENT_OBJECT( CVariant, CObject );

    CVariant::CVariant( void )
    {
        valType = UNKNOWN;
    }

    CVariant::CVariant( const CVariant &oSource ) 
    { 
        init(); 
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

    CVariant::~CVariant( void )
    {
    }

    CVariant &CVariant::operator =( const CVariant &oSource ) 
    { 
        if ( this != &oSource )
        {
            init(); 
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

    CString CVariant::getString( void )
    {
        CString temp;

        switch ( valType )
        {
            case UNKNOWN:
                return CString("");

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
                temp = CString::FormatString( "[%d elements]", arrayVal->Length() );
                return temp;

            case OBJECT:
            #ifdef IASLIB_RTTI__
                temp = CString::FormatString( "Object[%s(%ld)]", objVal->GetType(), objVal->GetID() );
            #else
                temp = CString::FormatString( "Object[%ld]", objVal->GetID() );
            #endif
                return temp;

            default:
                temp = "";
                return temp;
        }
        
    }

    CDate CVariant::getDate( void )
    {
        if ( valType == DATE )
        {
            return dateVal;
        }

        return CDate();
    }

    int CVariant::getInt( void )
    {
        if ( valType == INTEGER )
        {
            return intVal;
        }

        return 0;
    }

    long CVariant::getLong( void )
    {
        if ( valType == LONG )
        {
            return longVal;
        }

        if ( valType == FLOAT )
        {
            return (long)floatVal;
        }

        if ( valType == DOUBLE )
        {
            return (long)doubleVal;
        }

        if ( valType == INTEGER )
        {
            return (long)intVal;
        }

        if ( valType == BOOLEAN )
        {
            return (long)(boolVal ? 1L:0L);
        }

        return 0L;
    }

    float CVariant::getFloat( void )
    {
        if ( valType == FLOAT )
        {
            return floatVal;
        }

        return (float)0.0;

    }

    double CVariant::getDouble( void )
    {
        if ( valType == DOUBLE )
        {
            return doubleVal;
        }

        return 0.0;
    }

    bool CVariant::getBoolean( void )
    {
        if ( valType == BOOLEAN )
        {
            return boolVal;
        }

        return false;
    }

    CArray *CVariant::getArray( void )
    {
        if ( valType == ARRAY )
        {
            return arrayVal;
        }
        return NULL;
    }

    CObject *CVariant::getObject( void )
    {
        if ( valType == OBJECT )
        {
            return objVal;
        }
        return NULL;
    }

} // namespace IASLib
