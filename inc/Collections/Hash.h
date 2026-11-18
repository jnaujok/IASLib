/*
 * CHash Class
 *
 *  A hash table of CObject pointers (owned, as in CArray) by string key.
 * Keys ignore leading and trailing white space, and a number key is the
 * same as the string of its digits (Get( 5 ) finds Push( "5", ... )). The
 * table grows as needed (the size given to the constructor is only a
 * hint), and iterating (Enumerate(), Enum(), keySet()) gives the entries
 * in the order their keys were first added. It is thread-safe: each call
 * locks the table.
 *
 *  For a type safe hash table, or one of values, use THash.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994 (rebuilt on the template collections 11/18/2026)
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HASH_H__
#define IASLIB_HASH_H__

#include "../BaseTypes/String_.h"
#include "Collection.h"
#include "StringArray.h"
#include "../Templates/THash.h"
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
        public:
            enum HASH_SIZE
            {
                TINY, // < 100 objects
                SMALL, // < 500 objects
                NORMAL, // < 2,500 objects
                LARGE, //  < 10,000 objects
                ENORMOUS, // < 100,000 objects (was HUGE, which macOS math.h defines as a macro)
                GARGANTUAN // > 100,000 objects
            };

            typedef THash<CString, CObject *> Table;

        protected:
            CString         m_strSize;
            Table           m_hashTable;
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex  m_mutexProtect;
#endif

        public:
                            CHash( HASH_SIZE eSize = NORMAL );
                            CHash( const char *strSize );
#ifdef IASLIB_DATABASE__
                                // A table of the cursor's rows: strData's
                                // column (as a CString), by strKey's column.
                            CHash( CCursor *pData, const char *strKey, const char *strData );
#endif
            virtual        ~CHash( void );

                            DEFINE_OBJECT( CHash )

            virtual size_t  GetLength( void ) const;

                // Adds or replaces the object for a key. Replacing deletes
                // the object that was there if bDeleteCurrent (and it is not
                // pElement).
            void            Push( const char *strKey, CObject *pElement, bool bDeleteCurrent = true );
            CObject        *Get( const char *strKey ) const;
                // Removes the key and deletes its object.
            void            Delete( const char *strKey );
                // Removes the key; the caller owns the object (NULL if the key
                // was not there).
            CObject        *Remove( const char *strKey );

            void            Push( int nKey, CObject *pElement, bool bDeleteCurrent = true );
            CObject        *Get( int nKey ) const;
            void            Delete( int nKey );
            CObject        *Remove( int nKey );

            bool            HasKey( const char *strKey ) const;
            bool            HasKey( int nKey ) const;

            void            DeleteAll( void );
            void            EmptyAll( void );
            void            Empty( void ) { EmptyAll(); }

                // The nth object, in the order the keys were added.
            CObject        *Enum( size_t nIndex ) const;
            virtual CIterator *Enumerate( void );
                // The keys, in the order they were added.
            virtual CStringArray keySet( void ) const;

            const CString  &GetSizeName( void ) const { return m_strSize; }

            static CString  NormalizeKey( const char *strKey );
            static CString  NormalizeKey( int nKey );

        private:
            void            Init( const char *strSize );
                            CHash( const CHash & );
            CHash          &operator =( const CHash & );
    };
} // namespace IASLib

#endif
