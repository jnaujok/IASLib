/**
 * @file
 * @brief Settings Class.
 *
 *      This class is used to pull various settings out of a settings
 * file. These settings can then be used to drive the various other 
 * classes that need those settings to operate.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 09/09/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SETTINGS_H__
#define IASLIB_SETTINGS_H__

#include "../Collections/Hash.h"

namespace IASLib
{
    /// Named settings read from a "name = value" text file.
    ///
    /// Lines are "name = value"; blank lines and lines whose first
    /// non-blank character is '#' or ';' are skipped. The value is
    /// everything after the first '=' (trimmed), so it may hold '=' itself.
    /// A later line replaces an earlier one with the same name.
    ///
    /// @code
    /// CSettings oSettings;
    /// oSettings.SetFileName( "server.conf" );
    /// if ( oSettings.ReadSettings() )
    ///     int nPort = oSettings.GetIntSetting( "port", 8080 );
    /// @endcode
    class CSettings : public CObject
    {
        protected:
            CString         m_strSettingsFile;      ///< The file to read.
            CHash           m_hashSettings;         ///< Name to CString value (owned).
        public:
                            /// Makes an empty set, to be read from "./Settings.txt".
                            CSettings( void );
            virtual        ~CSettings( void );      ///< Destructor.

                            DEFINE_OBJECT( CSettings )

            void            SetFileName( const char *strFileName ) { m_strSettingsFile = strFileName; }  ///< Sets the file to read.
            const char     *GetFileName( void ) { return (const char *)m_strSettingsFile; }             ///< The file to read.
                /// Reads the file (adding to what was read before): false
                /// if it cannot be opened.
                /// @note A missing file used to throw CFileException, a value
                /// was cut at its second '=', and an indented comment was read
                /// as a setting.
            bool            ReadSettings( void );
                /// Whether the setting was read.
            bool            HasSetting( const char *strSettingName ) const;

                /// The setting's value, or @p strDefault.
            CString         GetSetting( const char *strSettingName, const char *strDefault = "" );
                /// The setting's value as a number (atoi()), or @p nDefault if
                /// there is no such setting.
            int             GetIntSetting( const char *strSettingName, int nDefault = 0 );
    };

        /// The one and only settings object for the application.
    extern CSettings   *g_Settings;
} // namespace IASLib

#endif // IASLIB_SETTINGS_H__
