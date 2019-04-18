/*
 * Settings Class
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
    class CSettings : public CObject
    {
        protected:
            CString         m_strSettingsFile;
            CHash           m_hashSettings;
        public:
                            CSettings( void );
            virtual        ~CSettings( void );

                            DEFINE_OBJECT( CSettings )

            void            SetFileName( const char *strFileName ) { m_strSettingsFile = strFileName; }
            const char     *GetFileName( void ) { return (const char *)m_strSettingsFile; }
            bool            ReadSettings( void );

            CString         GetSetting( const char *strSettingName, const char *strDefault = "" );
            int             GetIntSetting( const char *strSettingName, int nDefault = 0 );
    };

        // The one and only settings object for the application.
    extern CSettings   *g_Settings;
} // namespace IASLib

#endif // IASLIB_SETTINGS_H__
