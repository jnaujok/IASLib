/**
 * @file Hash.h
 * @brief CHash, an owning hash table of CObject pointers by string key.
 *
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
    /**
     * A hash table of CObject pointers (owned, as in CArray) by string key.
     *
     * Keys ignore leading and trailing white space, and a number key is the
     * same as the string of its digits (Get( 5 ) finds Push( "5", ... )).
     * The table grows as needed (the size given to the constructor is only a
     * hint), and iterating (Enumerate(), Enum(), keySet()) gives the entries
     * in the order their keys were first added. It is thread-safe: each call
     * locks the table. It cannot be copied. For a type safe hash table, or
     * one of values, use THash.
     *
     * @code
     * CHash hash;
     * hash.Push( "name", new CString( "Irene" ) );    // the table owns it
     * hash.Push( 5, new CString( "five" ) );
     * CString *pName = (CString *)hash.Get( " name " );   // found: spaces ignored
     * @endcode
     */
    class CHash : public CCollection
    {
        public:
                /// How many entries a table is expected to hold. It is only a
                /// name now (GetSizeName()): the table grows as needed.
            enum HASH_SIZE
            {
                TINY,       ///< < 100 objects.
                SMALL,      ///< < 500 objects.
                NORMAL,     ///< < 2,500 objects.
                LARGE,      ///< < 10,000 objects.
                ENORMOUS,   ///< < 100,000 objects (was HUGE, which macOS math.h defines as a macro; its name is still "HUGE").
                GARGANTUAN  ///< > 100,000 objects.
            };

                /// The table type, in the order keys were first added.
            typedef THash<CString, CObject *> Table;

        protected:
            CString         m_strSize;          ///< The size name ("NORMAL" and so on).
            Table           m_hashTable;        ///< The entries.
#ifdef IASLIB_MULTI_THREADED__
            mutable CMutex  m_mutexProtect;     ///< Locked by every call.
#endif

        public:
                            /// An empty table.
                            /// @param eSize The expected size (only a hint).
                            CHash( HASH_SIZE eSize = NORMAL );
                            /// An empty table, sized by name ("TINY" to
                            /// "GARGANTUAN", in any case); others mean "NORMAL".
                            CHash( const char *strSize );
#ifdef IASLIB_DATABASE__
                                /// A table of the cursor's rows: strData's
                                /// column (as a CString), by strKey's column.
                                /// It reads every row, moving the cursor.
                            CHash( CCursor *pData, const char *strKey, const char *strData );
#endif
                            /// Deletes every object.
            virtual        ~CHash( void );

                            DEFINE_OBJECT( CHash )

                /// The number of entries.
            virtual size_t  GetLength( void ) const;

                /// Adds or replaces the object for a key. Replacing deletes
                /// the object that was there if bDeleteCurrent (and it is not
                /// pElement). The table owns pElement.
            void            Push( const char *strKey, CObject *pElement, bool bDeleteCurrent = true );
                /// The object for a key (still owned by the table), or NULL.
            CObject        *Get( const char *strKey ) const;
                /// Removes the key and deletes its object.
            void            Delete( const char *strKey );
                /// Removes the key; the caller owns the object (NULL if the key
                /// was not there).
            CObject        *Remove( const char *strKey );

                /// @copydoc Push(const char*,CObject*,bool)
            void            Push( int nKey, CObject *pElement, bool bDeleteCurrent = true );
                /// @copydoc Get(const char*) const
            CObject        *Get( int nKey ) const;
                /// @copydoc Delete(const char*)
            void            Delete( int nKey );
                /// @copydoc Remove(const char*)
            CObject        *Remove( int nKey );

                /// Whether the key is in the table.
            bool            HasKey( const char *strKey ) const;
                /// Whether the key is in the table.
            bool            HasKey( int nKey ) const;

                /// Removes and deletes every object.
            void            DeleteAll( void );
                /// Removes every entry without deleting the objects.
            void            EmptyAll( void );
                /// Same as EmptyAll().
            void            Empty( void ) { EmptyAll(); }

                /// The nth object, in the order the keys were added (NULL past
                /// the end). Each call walks from the start.
            CObject        *Enum( size_t nIndex ) const;
                /// A new iterator over the objects (the caller deletes it), in
                /// the order the keys were added.
            virtual CIterator *Enumerate( void );
                /// The keys, in the order they were added.
            virtual CStringArray keySet( void ) const;

                /// The size name: "TINY", "SMALL", "NORMAL", "LARGE", "HUGE"
                /// or "GARGANTUAN".
            const CString  &GetSizeName( void ) const { return m_strSize; }

                /// The key as the table stores it: without leading and
                /// trailing white space.
            static CString  NormalizeKey( const char *strKey );
                /// The key as the table stores it: the number's digits.
            static CString  NormalizeKey( int nKey );

        private:
                /// Sets the size name, what the constructors share.
            void            Init( const char *strSize );
                            /// Not copyable (the table owns its objects).
                            CHash( const CHash & );
                            /// Not assignable.
            CHash          &operator =( const CHash & );
    };
} // namespace IASLib

#endif
