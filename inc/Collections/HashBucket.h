/*
 * Hash Bucket
 *
 *  This file defines and implments the hash bucket object. Hash
 * buckets are used as the individual entries in a hash table and may
 * contain an array of elements, keyed on a string.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1995
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HASHBUCKET_H__
#define IASLIB_HASHBUCKET_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/Bool.h"
#include "HashSlat.h"
#include "StringArray.h"

namespace IASLib
{
    class CHashBucket : public CObject
    {
        protected:
            CHashSlat     **m_aSlats;
            size_t          m_nSlats;
        public:
                            CHashBucket( void );
            virtual        ~CHashBucket( void );

                            DEFINE_OBJECT( CHashBucket )

            size_t          GetLength( void );
            bool            Push( const char *strKey, CObject *pElement, bool bDeleteCurrent );
            CObject        *Get( const char *strKey );
            bool            Delete( const char *strKey );
            CObject        *Remove( const char *strKey );

            bool            Push( int nKey, CObject *pElement, bool bDeleteCurrent );
            CObject        *Get( int nKey );
            bool            Delete( int nKey );
            CObject        *Remove( int nKey );

            bool            HasKey( const char *strKey );
            bool            HasKey( int nKey );

            CObject        *Enum( size_t nIndex );
            void            DeleteAll( void );
            void            Empty( void );

        private:
            friend class CHash;
            void            keySet( CStringArray &oKeyArray );
    };
} // namespace IASLib

#endif  // IASLIB_HASHBUCKET_H__
