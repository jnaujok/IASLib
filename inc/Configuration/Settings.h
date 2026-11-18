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
    /// Reads "name = value" settings from a plain text file.
    ///
    /// Each line of the file that contains '=' gives one setting. Lines
    /// whose first character is '#' are comments. Names and values are
    /// trimmed of surrounding spaces. The settings are kept in a hash, so
    /// lookups do not read the file again.
    ///
    /// @code
    /// CSettings oSettings;
    /// oSettings.SetFileName( "/etc/myapp.conf" );
    /// if ( oSettings.ReadSettings() )
    /// {
    ///     int nThreads = oSettings.GetIntSetting( "Threads", 4 );
    /// }
    /// @endcode
    class CSettings : public CObject
    {
        protected:
            CString         m_strSettingsFile;  ///< Path of the settings file.
            CHash           m_hashSettings;     ///< Setting name to value (CString *, owned).
        public:
                            /// Makes an empty settings object for the file "./Settings.txt".
                            CSettings( void );
            /// Deletes all stored values.
            virtual        ~CSettings( void );

                            DEFINE_OBJECT( CSettings )

            /// Sets the path of the settings file. It does not read it.
            void            SetFileName( const char *strFileName ) { m_strSettingsFile = strFileName; }
            /// The path of the settings file.
            const char     *GetFileName( void ) { return (const char *)m_strSettingsFile; }
            /// Reads the settings file and adds its settings.
            /// Settings already read are kept, so calling it twice adds
            /// duplicate entries.
            /// @return True if the file was opened and read, false if it could not be opened.
            bool            ReadSettings( void );

            /// Returns a setting's value.
            /// @param strSettingName The setting name.
            /// @param strDefault Returned if the setting is not present.
            /// @return The value, or @p strDefault.
            CString         GetSetting( const char *strSettingName, const char *strDefault = "" );
            /// Returns a setting's value converted with atoi().
            /// @param strSettingName The setting name.
            /// @param nDefault Returned if the setting is not present.
            /// @return The value (0 if it is not a number), or @p nDefault.
            int             GetIntSetting( const char *strSettingName, int nDefault = 0 );
    };

    /// The one and only settings object for the application.
    /// It is not created by the library; the application sets it.
    extern CSettings   *g_Settings;
} // namespace IASLib

#endif // IASLIB_SETTINGS_H__
