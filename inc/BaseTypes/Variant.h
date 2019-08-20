/**
 * Variant Class Header File
 *
 * A Variant object can store data of any type.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/31/2019
 *	Log:
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_VARIANT_H__
#define IASLIB_VARIANT_H__

#include "Object.h"
#include "String_.h"
#include "Date.h"
#include "Object.h"

namespace IASLib
{
    // Forward declaration of Array
    class CArray;
    
    class CVariant : public CObject
    {
        enum VARIANT_TYPE
        {
            UNKNOWN,
            STRING,
            DATE,
            INTEGER,
            LONG,
            FLOAT,
            DOUBLE,
            BOOLEAN,
            ARRAY,
            OBJECT
        };

        private:
            VARIANT_TYPE    valType;
            CString         strVal;
            CDate           dateVal;
            int             intVal;
            long            longVal;
            float           floatVal;
            double          doubleVal;
            bool            boolVal;
            CArray         *arrayVal;
            CObject        *objVal;

        public:
                        DEFINE_OBJECT( CVariant );

                        CVariant( void );
                        CVariant( const char *oSource ) { init(); valType = STRING; strVal = oSource; }
                        CVariant( const CString &oSource ) { init(); valType = STRING; strVal = oSource; }
                        CVariant( const CDate &oSource ) { init(); valType = DATE; dateVal = oSource; }
                        CVariant( const int source ) { init(); valType = INTEGER; intVal = source; }
                        CVariant( const long source ) { init(); valType = LONG; longVal = source; }
                        CVariant( const float source ) { init(); valType = FLOAT; floatVal = source; }
                        CVariant( const double source ) { init(); valType = DOUBLE; doubleVal = source; }
                        CVariant( const bool source ) { init(); valType = BOOLEAN; boolVal = source; }
                        CVariant( CArray *array ) { init(); valType = ARRAY; arrayVal = array; }
                        CVariant( CObject *object ) { init(); valType = OBJECT; objVal = object; }

                        CVariant( const CVariant &oSource );

            CVariant   &operator =( const CVariant &oSource );

            virtual    ~CVariant( void );

            void        init( void ) 
                        { 
                            valType = UNKNOWN;
                            strVal = CString("");
                            dateVal = CDate();
                            intVal = 0;
                            longVal = 0L;
                            floatVal = (float)0.0;
                            doubleVal = 0.0;
                            boolVal = false;
                            arrayVal = NULL;
                            objVal = NULL;
                        }

            CVariant   &operator =( const CString &oSource ) { valType = STRING; strVal = oSource; return *this; }
            CVariant   &operator =( const CDate &oSource ) { valType = DATE; dateVal = oSource; return *this; }
            CVariant   &operator =( const int source ) { valType = INTEGER; intVal = source; return *this; }
            CVariant   &operator =( const long source ) { valType = LONG; longVal = source; return *this; }
            CVariant   &operator =( const float source ) { valType = FLOAT; floatVal = source; return *this; }
            CVariant   &operator =( const double source ) { valType = DOUBLE; doubleVal = source; return *this; }
            CVariant   &operator =( const bool source ) { valType = BOOLEAN; boolVal = source; return *this; }
            CVariant   &operator =( CArray *array ) { valType = ARRAY; arrayVal = array; return *this; }
            CVariant   &operator =( CObject *object ) { valType = OBJECT; objVal = object; return *this; }

            VARIANT_TYPE getType( void ) { return valType; }

            CString     getString( void );
            CDate       getDate( void );
            int         getInt( void );
            long        getLong( void );
            float       getFloat( void );
            double      getDouble( void );
            bool        getBoolean( void );
            CArray     *getArray( void );
            CObject    *getObject( void );

    };
}

#endif // IASLIB_VARIANT_H__
