/*
 *  Directory Class
 *
 *      This class encapsulates a directory listing with a filter. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 02/19/2002
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DIRECTORY_H__
#define IASLIB_DIRECTORY_H__

#include "../BaseTypes/String_.h"
#include "../Collections/Array.h"
#include "DirectoryEntry.h"
#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif

namespace IASLib
{
    class CDirectory : public CObject
    {
        protected:
            CArray              m_aDirEntries;
            CString             m_strDirPath;
            CString             m_strFilter;
            bool                m_bLightweight;
#ifdef IASLIB_MULTI_THREADED__
            static CMutex       m_mutexThreadSafe;
#endif
        public:
                                CDirectory( const char *strDirPath, const char *strFilter = "*", bool bLightweight = false );
            virtual            ~CDirectory( void );

                                DEFINE_OBJECT( CDirectory )

            size_t              GetEntryCount( void );
            const char         *GetFileName( size_t nIndex );
            CDirectoryEntry    *GetEntry( size_t nIndex );

            void                Refresh( void );
    };
} // namespace IASLib
#endif // __DIRECTORY_H__
