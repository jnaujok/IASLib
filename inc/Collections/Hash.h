/*
 * Hash
 *
 *  This object provides a string keyed hash for looking up keyed items
 * very, very rapidly. It makes use of one other class, the Hash Bucket,
 * which is placed at each array entry to store data that has been
 * hashed. Array entries that do not have any data are simply set to
 * null rather than waste the memory for a hash bucket.
 *
 *  The default hash table is either 107, 347, or 1019 elements, with
 * hash keys of either 191, 421, or 1607 respectively. The initilizer
 * values of "small", "normal", or "large" are passed to the constructor
 * to determine what sizes are being used. The default size is "small".
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 01/01/1995
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HASH_H__
#define IASLIB_HASH_H__

#include "../BaseTypes/String_.h"
#include "Collection.h"
#include "HashBucket.h"
#ifdef IASLIB_MULTI_THREADED__
#include "../Threading/Mutex.h"
#endif
#ifdef IASLIB_DATABASE__
#include "../Database/Cursor.h"
#endif

namespace IASLib
{
    class CHash : public CCollection
    {
        protected:
            CString         m_strSize;
            size_t          m_nArraySize;
            int             m_nHashKey;
            CHashBucket   **m_aHashTable;
#ifdef IASLIB_MULTI_THREADED__
            CMutex          m_mutexProtect;
#endif
        public:
                            CHash( const char *strSize );
#ifdef IASLIB_DATABASE__
                            CHash( CCursor *pData, const char *strKey, const char *strData );
#endif

            virtual        ~CHash( void );

                            DEFINE_OBJECT( CHash )

            void            Push( const char *strKey, CObject *pElement, bool bDeleteCurrent = true );
            CObject        *Get( const char *strKey );
            void            Delete( const char *strKey );

            void            Push( int nKey, CObject *pElement, bool bDeleteCurrent = true );
            CObject        *Get( int nKey );
            void            Delete( int nKey );

            bool            HasKey( const char *strKey );
            bool            HasKey( int nKey );

            void            DeleteAll( void );
            void            EmptyAll( void );

            CObject        *Enum( size_t nIndex );

            void            Empty( void );

            virtual CIterator      *Enumerate( void );
        private:
            int             BuildKey( const char *strKey );
            int             BuildKey( int nKey );
    };
} // namespace IASLib

#endif
