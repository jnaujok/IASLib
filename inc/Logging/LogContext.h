/**
 * @file
 * Log Context Class Header File
 *
 * The Log Context provides a context for storing names and values
 * that can be extracted and used to generate log messages. It also
 * includes some defined macros that build some helpful data into
 * the context each time it's used.
 *
 * A context should be generated on a per-event or per-thread basis
 * and not shared across threads, as it includes certain thread-based
 * information that should not be traded across contexts.
 *
 * A context can be created from another context to inherit certain
 * information from a parent context.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 04/25/2019
 *	Log:
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_LOG_CONTEXT_H__
#define IASLIB_LOG_CONTEXT_H__

#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "../Collections/Hash.h"
#include "../Collections/Array.h"
#include "ContextValue.h"

namespace IASLib
{
        /// Named values for log messages.
        ///
        /// CLogSink::writeContextLog() puts them in place of ${name}.
        /// Adding a name again replaces (and deletes) its old value. The
        /// context owns its CContextValue entries, but not the arrays or
        /// objects they point to.
        /// Not thread-safe: one context per request or thread. Not copyable.
        ///
        /// @code
        /// CLogContext context;
        /// context.addValue( "user", CString( "alice" ) );
        /// context.addValue( "id", 42 );
        /// CString s = context.expand( "${user} #${id}" );   // "alice #42"
        /// @endcode
    class CLogContext : public CObject
    {
        protected:
            CHash       m_hashValues;   ///< Name to CContextValue (owned).

        public:
                        DEFINE_OBJECT( CLogContext );
                        /// Creates an empty context.
                        CLogContext( void );
                        /// Deletes every value in the context.
            virtual    ~CLogContext( void );

                /// Adds a string value, replacing any value with that name.
                /// @param name  The value's name.
                /// @param value The value.
            virtual void    addValue( const char *name, CString value );
                /// Adds a date value. @copydetails addValue(const char*,CString)
            virtual void    addValue( const char *name, CDate value );
                /// Adds an int value. @copydetails addValue(const char*,CString)
            virtual void    addValue( const char *name, int value );
                /// Adds a long value. @copydetails addValue(const char*,CString)
            virtual void    addValue( const char *name, long value );
                /// Adds a float value. @copydetails addValue(const char*,CString)
            virtual void    addValue( const char *name, float value );
                /// Adds a double value. @copydetails addValue(const char*,CString)
            virtual void    addValue( const char *name, double value );
                /// Adds a boolean value. @copydetails addValue(const char*,CString)
            virtual void    addValue( const char *name, bool value );
                /// Adds an array value.
                /// Refers to value (not a copy): it must outlive the context.
                /// @param name  The value's name.
                /// @param value The array (not owned).
                /// @note It used to copy the array, and both copies deleted
                /// the same objects.
            virtual void    addValue( const char *name, CArray &value );
                /// Adds an object value that points to value (not owned).
                /// @param name  The value's name.
                /// @param value The object; it must outlive the context.
            virtual void    addObject( const char *name, CObject *value );

                /// The named value as a string ("" if there is none).
                /// @param name The value's name.
            virtual CString getStringValue( const char *name );
                /// The named value as a date (a default CDate if there is none).
                /// @param name The value's name.
            virtual CDate   getDateValue( const char *name );
                /// The named value as an int (0 if there is none).
                /// @param name The value's name.
            virtual int     getIntValue( const char *name );
                /// The named value as a long (0 if there is none).
                /// @param name The value's name.
            virtual long    getLongValue( const char *name );
                /// The named value as a float (0 if there is none).
                /// @param name The value's name.
            virtual float   getFloatValue( const char *name );
                /// The named value as a double (0 if there is none).
                /// @param name The value's name.
            virtual double  getDoubleValue( const char *name );
                /// The named value as a boolean (false if there is none).
                /// @param name The value's name.
            virtual bool    getBoolValue( const char *name );
                /// The array the named value points to (NULL if there is none).
                /// @param name The value's name.
                /// @note It used to return one shared empty array, created
                /// without a lock and open to changes by any caller.
            virtual CArray *getArrayValue( const char *name );
                /// The object the named value points to (NULL if there is none).
                /// @param name The value's name.
            virtual CObject *getObjectValue( const char *name );

                /// The named entry itself (NULL if there is none; still owned by the context).
                /// @param name The value's name.
            virtual CContextValue *get( const char *name );
                /// True if the context has a value with this name (false for NULL).
                /// @param name The value's name.
            bool            has( const char *name ) const;
                /// Removes and deletes a value.
                /// @param name The value's name.
                /// @return False if there was none.
            bool            remove( const char *name );

                /// The text with each ${name} replaced by that value's string
                /// form.
                /// Names not in the context are left as written; "$${"
                /// gives a literal "${".
                /// @param strText The text to expand (NULL gives "").
                /// @return The expanded text.
            CString         expand( const char *strText ) const;

        private:
                            /// Not copyable.
                            CLogContext( const CLogContext & );
                            /// Not assignable.
            CLogContext    &operator =( const CLogContext & );

    };
} // namespace IASLib

#endif // IASLIB_LOG_CONTEXT_H__
