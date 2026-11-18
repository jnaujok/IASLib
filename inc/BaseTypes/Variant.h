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

namespace IASLib
{
    // Forward declaration of Array
    class CArray;

        // A value of one of several types. The number getters convert
        // between the number types (and booleans), and read numbers from
        // strings; getString() writes any value as text. Arrays and
        // objects are held by pointer, not owned: the program keeps them
        // alive while the variant refers to them.
    class CVariant : public CObject
    {
        public:
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

                        CVariant( void ) { init(); }
                        CVariant( const char *oSource ) { init(); *this = oSource; }
                        CVariant( const CString &oSource ) { init(); *this = oSource; }
                        CVariant( const CDate &oSource ) { init(); *this = oSource; }
                        CVariant( const int source ) { init(); *this = source; }
                        CVariant( const long source ) { init(); *this = source; }
                        CVariant( const float source ) { init(); *this = source; }
                        CVariant( const double source ) { init(); *this = source; }
                        CVariant( const bool source ) { init(); *this = source; }
                        CVariant( CArray *array ) { init(); *this = array; }
                        CVariant( CObject *object ) { init(); *this = object; }

                        CVariant( const CVariant &oSource );

            CVariant   &operator =( const CVariant &oSource );

            virtual    ~CVariant( void );

                // Back to UNKNOWN, with every value cleared.
            void        init( void );

            CVariant   &operator =( const char *oSource ) { clear( STRING ); strVal = oSource; return *this; }
            CVariant   &operator =( const CString &oSource ) { clear( STRING ); strVal = oSource; return *this; }
            CVariant   &operator =( const CDate &oSource ) { clear( DATE ); dateVal = oSource; return *this; }
            CVariant   &operator =( const int source ) { clear( INTEGER ); intVal = source; return *this; }
            CVariant   &operator =( const long source ) { clear( LONG ); longVal = source; return *this; }
            CVariant   &operator =( const float source ) { clear( FLOAT ); floatVal = source; return *this; }
            CVariant   &operator =( const double source ) { clear( DOUBLE ); doubleVal = source; return *this; }
            CVariant   &operator =( const bool source ) { clear( BOOLEAN ); boolVal = source; return *this; }
            CVariant   &operator =( CArray *array ) { clear( ARRAY ); arrayVal = array; return *this; }
            CVariant   &operator =( CObject *object ) { clear( OBJECT ); objVal = object; return *this; }

            VARIANT_TYPE getType( void ) const { return valType; }
            bool        isEmpty( void ) const { return valType == UNKNOWN; }
            bool        isNumber( void ) const;

            CString     getString( void ) const;
                // The date (or an empty CDate, if this is not a date).
            CDate       getDate( void ) const;
                // The value as a number: numbers convert (doubles are cut
                // toward zero, and limited to the type's range), booleans are
                // 0 or 1, strings are read as numbers; anything else is 0.
            int         getInt( void ) const;
            long        getLong( void ) const;
            float       getFloat( void ) const;
            double      getDouble( void ) const;
                // True for true, a non-zero number, or a string "true",
                // "yes" or "on" (in any case) or a non-zero number.
            bool        getBoolean( void ) const;
            CArray     *getArray( void ) const;
            CObject    *getObject( void ) const;

        private:
            void        clear( VARIANT_TYPE newType );
    };
}

#endif // IASLIB_VARIANT_H__
