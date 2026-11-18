/**
 * @file Variant.h
 * @brief CVariant, a value that can hold any of several types.
 *
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

        /// A value of one of several types.
        ///
        /// The number getters convert between the number types (and
        /// booleans), and read numbers from strings; getString() writes any
        /// value as text. Arrays and objects are held by pointer, not owned:
        /// the program keeps them alive while the variant refers to them.
        ///
        /// @code
        /// CVariant v( "42" );
        /// int n = v.getInt();              // 42
        /// v = 2.5;
        /// CString s = v.getString();       // "2.5"
        /// bool b = CVariant( "yes" ).getBoolean();   // true
        /// @endcode
    class CVariant : public CObject
    {
        public:
                /// The type of value a variant holds.
            enum VARIANT_TYPE
            {
                UNKNOWN,    ///< No value.
                STRING,     ///< A CString.
                DATE,       ///< A CDate.
                INTEGER,    ///< An int.
                LONG,       ///< A long.
                FLOAT,      ///< A float.
                DOUBLE,     ///< A double.
                BOOLEAN,    ///< A bool.
                ARRAY,      ///< A CArray pointer (not owned).
                OBJECT      ///< A CObject pointer (not owned).
            };

        private:
            VARIANT_TYPE    valType;    ///< The type held.
            CString         strVal;     ///< The STRING value.
            CDate           dateVal;    ///< The DATE value.
            int             intVal;     ///< The INTEGER value.
            long            longVal;    ///< The LONG value.
            float           floatVal;   ///< The FLOAT value.
            double          doubleVal;  ///< The DOUBLE value.
            bool            boolVal;    ///< The BOOLEAN value.
            CArray         *arrayVal;   ///< The ARRAY value (not owned).
            CObject        *objVal;     ///< The OBJECT value (not owned).

        public:
                        DEFINE_OBJECT( CVariant );

                        /// An empty (UNKNOWN) variant.
                        CVariant( void ) { init(); }
                        /// A STRING.
                        CVariant( const char *oSource ) { init(); *this = oSource; }
                        /// A STRING.
                        CVariant( const CString &oSource ) { init(); *this = oSource; }
                        /// A DATE.
                        CVariant( const CDate &oSource ) { init(); *this = oSource; }
                        /// An INTEGER.
                        CVariant( const int source ) { init(); *this = source; }
                        /// A LONG.
                        CVariant( const long source ) { init(); *this = source; }
                        /// A FLOAT.
                        CVariant( const float source ) { init(); *this = source; }
                        /// A DOUBLE.
                        CVariant( const double source ) { init(); *this = source; }
                        /// A BOOLEAN.
                        CVariant( const bool source ) { init(); *this = source; }
                        /// An ARRAY (the array is not owned).
                        CVariant( CArray *array ) { init(); *this = array; }
                        /// An OBJECT (the object is not owned).
                        CVariant( CObject *object ) { init(); *this = object; }

                        /// A copy (the same array or object pointer, if any).
                        CVariant( const CVariant &oSource );

                        /// Copies the type and value (the same array or object pointer, if any).
            CVariant   &operator =( const CVariant &oSource );

            virtual    ~CVariant( void );

                /// Back to UNKNOWN, with every value cleared.
            void        init( void );

                /// Holds a STRING.
            CVariant   &operator =( const char *oSource ) { clear( STRING ); strVal = oSource; return *this; }
                /// Holds a STRING.
            CVariant   &operator =( const CString &oSource ) { clear( STRING ); strVal = oSource; return *this; }
                /// Holds a DATE.
            CVariant   &operator =( const CDate &oSource ) { clear( DATE ); dateVal = oSource; return *this; }
                /// Holds an INTEGER.
            CVariant   &operator =( const int source ) { clear( INTEGER ); intVal = source; return *this; }
                /// Holds a LONG.
            CVariant   &operator =( const long source ) { clear( LONG ); longVal = source; return *this; }
                /// Holds a FLOAT.
            CVariant   &operator =( const float source ) { clear( FLOAT ); floatVal = source; return *this; }
                /// Holds a DOUBLE.
            CVariant   &operator =( const double source ) { clear( DOUBLE ); doubleVal = source; return *this; }
                /// Holds a BOOLEAN.
            CVariant   &operator =( const bool source ) { clear( BOOLEAN ); boolVal = source; return *this; }
                /// Holds an ARRAY (not owned).
            CVariant   &operator =( CArray *array ) { clear( ARRAY ); arrayVal = array; return *this; }
                /// Holds an OBJECT (not owned).
            CVariant   &operator =( CObject *object ) { clear( OBJECT ); objVal = object; return *this; }

                /// The type held.
            VARIANT_TYPE getType( void ) const { return valType; }
                /// Whether the variant holds nothing (UNKNOWN).
            bool        isEmpty( void ) const { return valType == UNKNOWN; }
                /// Whether it holds an INTEGER, LONG, FLOAT or DOUBLE.
            bool        isNumber( void ) const;

                /// The value as text: numbers and booleans as CString writes
                /// them, a date in DF_PRETTY form, an array as "[n elements]",
                /// an object as "Object[type(id)]"; "" when empty.
            CString     getString( void ) const;
                /// The date (or a default CDate, which is the current time, if
                /// this is not a date).
            CDate       getDate( void ) const;
                /// The value as a number: numbers convert (doubles are cut
                /// toward zero, and limited to the type's range), booleans are
                /// 0 or 1, strings are read as numbers; anything else is 0.
            int         getInt( void ) const;
                /// @copydoc getInt()
            long        getLong( void ) const;
                /// The value as a number (see getInt()).
            float       getFloat( void ) const;
                /// The value as a number (see getInt()).
            double      getDouble( void ) const;
                /// True for true, a non-zero number, or a string "true",
                /// "yes" or "on" (in any case) or a non-zero number.
            bool        getBoolean( void ) const;
                /// The array, or NULL if this is not an ARRAY.
            CArray     *getArray( void ) const;
                /// The object, or NULL if this is not an OBJECT.
            CObject    *getObject( void ) const;

        private:
                /// Switches to newType (freeing a string no longer held).
            void        clear( VARIANT_TYPE newType );
    };
}

#endif // IASLIB_VARIANT_H__
