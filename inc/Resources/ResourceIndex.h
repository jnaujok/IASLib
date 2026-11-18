/**
 *  Resource Index Entry
 *
 *      What a resource archive's index records about one resource: its
 * folder and resource tags (together its ID), name, type, when and by whom
 * it was last changed, and where its data is in the file, with the data's
 * length and SHA-256 digest.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCE_INDEX_H__
#define IASLIB_RESOURCE_INDEX_H__

#include <stdint.h>
#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"
#include "../BaseTypes/Date.h"
#include "../Encryption/Sha256.h"

namespace IASLib
{
        // Compression of a resource's data. Only NONE is written; an archive
        // that names another is refused when that resource is read.
    static const int RESFILE_COMPRESSION_NONE       = 0;
    static const int RESFILE_COMPRESSION_DEFLATE    = 1;

    class CResourceIndexEntry : public CObject
    {
        public:
            short       m_nFolder;
            short       m_nResource;
            CString     m_strResourceName;
            int         m_nType;
            CDate       m_dttLastModified;
            CString     m_strModifiedBy;
            uint64_t    m_nOffset;          // of the data in the file
            uint64_t    m_nLength;          // of the data
            CSha256Digest m_digest;         // SHA-256 of the data
            int         m_nCompression;
            long        m_nUsage;           // reads since the archive was opened

                        CResourceIndexEntry( void );
                        CResourceIndexEntry( const CResourceIndexEntry &oSource );
            virtual    ~CResourceIndexEntry( void );
            CResourceIndexEntry &operator =( const CResourceIndexEntry &oSource );

                        DEFINE_OBJECT( CResourceIndexEntry )

                // The folder and resource tags as one number.
            uint32_t    GetID( void ) const { return MakeID( m_nFolder, m_nResource ); }
            static uint32_t MakeID( short nFolder, short nResource )
            {
                return ( (uint32_t)(unsigned short)nFolder << 16 ) | (uint32_t)(unsigned short)nResource;
            }
    };
} // namespace IASLib

#endif // IASLIB_RESOURCE_INDEX_H__
