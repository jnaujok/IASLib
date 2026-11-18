/**
 *  Resource Sources
 *
 *      Where CResourceManager finds a resource's bytes, by path
 * ("textures/hero.png"):
 *
 *      CArchiveSource      a CResourceFile archive, by resource name
 *      CDirectorySource    loose files under a directory (during
 *                          development, or for patches and mods)
 *
 *      A program can add its own (a network source, a compressed pack).
 * Sources must be safe to call from several threads at once: the manager's
 * background loaders do.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCESOURCE_H__
#define IASLIB_RESOURCESOURCE_H__

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "../BaseTypes/String_.h"
#include "../BaseTypes/DataBlock.h"
#include "../Threading/Mutex.h"
#include "ResourceFile.h"

namespace IASLib
{
    class CResourceSource
    {
        public:
            virtual            ~CResourceSource( void ) {}

                // For messages.
            virtual CString     GetName( void ) const = 0;
                // Whether the source has this path (normalised: '/'
                // separators, no leading '/', no "." or "..").
            virtual bool        Exists( const CString &strPath ) = 0;
                // The bytes, or false with strError saying why.
            virtual bool        Read( const CString &strPath, CDataBlock &oData, CString &strError ) = 0;
    };

        // Resources by name from an archive. The manager does the caching,
        // so the archive's own cache is turned off unless bKeepArchiveCache.
    class CArchiveSource : public CResourceSource
    {
        protected:
            std::shared_ptr<CResourceFile>      m_pArchive;
            std::map<std::string, uint32_t>     m_mapNames;     // name -> ID
            mutable CMutex                      m_mutex;

        public:
                                CArchiveSource( const std::shared_ptr<CResourceFile> &pArchive, bool bKeepArchiveCache = false );

            virtual CString     GetName( void ) const;
            virtual bool        Exists( const CString &strPath );
            virtual bool        Read( const CString &strPath, CDataBlock &oData, CString &strError );

                // Rereads the archive's names (after resources are added to
                // or removed from it).
            void                Refresh( void );
            std::shared_ptr<CResourceFile> GetArchive( void ) const { return m_pArchive; }
    };

        // Files under a directory: "textures/hero.png" is
        // <root>/textures/hero.png.
    class CDirectorySource : public CResourceSource
    {
        protected:
            CString             m_strRoot;

        public:
                                CDirectorySource( const char *strRoot );

            virtual CString     GetName( void ) const;
            virtual bool        Exists( const CString &strPath );
            virtual bool        Read( const CString &strPath, CDataBlock &oData, CString &strError );

            CString             FullPath( const CString &strPath ) const;
    };
} // namespace IASLib

#endif // IASLIB_RESOURCESOURCE_H__
