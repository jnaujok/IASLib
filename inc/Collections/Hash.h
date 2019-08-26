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

#include "StringArray.h"

namespace IASLib
{
    class CHash : public CCollection
    {
        public:
            enum HASH_SIZE
            {
                TINY, // < 100 objects
                SMALL, // < 500 objects
                NORMAL, // < 2,500 objects
                LARGE, //  < 10,000 objects
                HUGE, // < 100,000 objects
                GARGANTUAN // > 100,000 objects. Performance will degrade with more than about 5M objects
            };

        protected:
            CString         m_strSize;
            size_t          m_nArraySize;
            int             m_nHashKey;
            CHashBucket   **m_aHashTable;
#ifdef IASLIB_MULTI_THREADED__
            CMutex          m_mutexProtect;
#endif
        public:
                            CHash( HASH_SIZE eSize = NORMAL );
                            CHash( const char *strSize = NULL );
#ifdef IASLIB_DATABASE__
                            CHash( CCursor *pData, const char *strKey, const char *strData );
#endif
                            CHash( const CHash &oSource );

            virtual        ~CHash( void );

                            DEFINE_OBJECT( CHash )

            CHash          &operator =( const CHash &oSource );
            
            void            Push( const char *strKey, CObject *pElement, bool bDeleteCurrent = true );
            CObject        *Get( const char *strKey );
            void            Delete( const char *strKey );
            CObject        *Remove( const char *strKey );

            void            Push( int nKey, CObject *pElement, bool bDeleteCurrent = true );
            CObject        *Get( int nKey );
            void            Delete( int nKey );
            CObject        *Remove( int nKey );


            bool            HasKey( const char *strKey );
            bool            HasKey( int nKey );

            void            DeleteAll( void );
            void            EmptyAll( void );

            CObject        *Enum( size_t nIndex );

            void            Empty( void );

            virtual CIterator      *Enumerate( void );

            virtual CStringArray keySet( void );
        private:
            int             BuildKey( const char *strKey );
            int             BuildKey( int nKey );
    };
} // namespace IASLib

#endif
