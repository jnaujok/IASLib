/**
 *  @file StringStub.h
 *  @brief CStringStub, the shared, reference-counted text behind CString.
 *
 *  CStringStub Class
 *
 *      This class encodes a "string stub" which is where actual string
 * data is stored for the CString class. This allows us to have strings
 * that are "copied" merely by pointing at the same string stub. If fifty
 * copies are made of a string, then they merely all point at the same
 * stub in memory. Only when a string is changed does it get its own copy
 * of the string stub. This will cut down on a large amount of un-needed
 * (and extremely slow) memory allocation and copying.
 *
 *      The reference count is atomic, so copies of a string can be used
 * (and changed) on different threads; one CString object is not itself
 * safe to change on one thread while another uses it, like any value.
 *
 *      A fixed stub holds a buffer given by the program: it is never moved
 * or grown (text that does not fit is cut off), and every copy of the
 * string shares it, so a change through one copy is seen by all.
 *
 *      Only CString uses this class.
 *
 *  Created:
 *      8/19/2004
 *  Creator:
 *      Jeffrey R. Naujok
 *  Revision History:
 *      ($LOG$)
 *
 */

#ifndef IASLIB_STRINGSTUB_H__
#define IASLIB_STRINGSTUB_H__

#include "Object.h"

#include <atomic>

namespace IASLib
{
    /**
     * The shared, reference-counted text behind CString.
     *
     * Copies of a CString point at the same stub; a string gets its own stub
     * only when it changes. The reference count is atomic, so copies of a
     * string can be used on different threads. A new stub has no references:
     * its owner calls AddRef(), and RemoveRef() deletes the stub with the
     * last one, so stubs must be made with new.
     *
     * A fixed stub holds a buffer given by the program. It is never moved or
     * grown (text that does not fit is cut off), and every copy of the string
     * shares it. Only CString uses this class.
     */
    class CStringStub
    {
        public:
            IASLibChar__       *m_strData;      ///< The text; always NUL terminated.
            size_t              m_nLength;      ///< Characters in the string.
            size_t              m_nCapacity;    ///< Characters that fit (not counting the NUL).
            std::atomic<int>    m_nReferences;  ///< How many strings use this stub.
            bool                m_bFixedStub;   ///< Whether the buffer is the program's (never moved or grown).
            bool                m_bDeletable;   ///< Whether m_strData is ours to free.

                /// A stub holding nLength characters copied from strData
                /// (or nLength NULs, if strData is NULL), with room for at
                /// least nCapacity.
                /// @param strData The characters, or NULL.
                /// @param nLength How many characters.
                /// @param nCapacity The room to make; less than nLength means nLength.
                /// @throws std::bad_alloc When memory runs out.
                                CStringStub( const IASLibChar__ *strData, size_t nLength, size_t nCapacity = 0 );
                /// A fixed stub on a buffer of nSize bytes (so nSize - 1
                /// characters and the NUL), holding nLength characters
                /// already in it (cut to fit).
                /// @param strBuffer The buffer.
                /// @param nLength Characters already in it.
                /// @param nSize The buffer's size, counting the NUL.
                /// @param bDeletable Whether the buffer came from malloc and
                ///     the stub frees it.
                                CStringStub( IASLibChar__ *strBuffer, size_t nLength, size_t nSize, bool bDeletable );
                /// Frees the buffer if it is ours (m_bDeletable).
                               ~CStringStub( void );

                /// Adds a reference.
            void                AddRef( void ) { m_nReferences.fetch_add( 1, std::memory_order_relaxed ); }
                /// Drops a reference, deleting the stub with the last one.
            void                RemoveRef( void );
                /// How many strings use this stub.
            int                 GetRefCount( void ) const { return m_nReferences.load( std::memory_order_acquire ); }
                /// Whether more than one string uses this stub.
            bool                IsShared( void ) const { return GetRefCount() > 1; }

                /// Sets the length, growing the buffer if it must (a fixed
                /// stub instead cuts the length to its capacity). The text
                /// up to the old length is kept, and the string is NUL
                /// terminated at the new length.
                ///
                /// Growth leaves a little room for further appends: half the
                /// new length, but never more than MAX_GROWTH_SLACK
                /// characters, so building a string piece by piece does not
                /// copy it every time, and no string carries more than that
                /// much unused space. Everything else allocates exactly.
                /// @param nLength The new length.
                /// @return The new length.
                /// @throws std::bad_alloc When memory runs out.
            size_t              ChangeSize( size_t nLength );

                /// Makes room for nCapacity characters without changing the
                /// string (does not change a fixed stub).
                /// @throws std::bad_alloc When memory runs out.
            void                Reserve( size_t nCapacity );
                /// Gives back unused room (does not change a fixed stub).
            void                Compact( void );

                /// The most unused room ChangeSize() leaves when it grows.
            static const size_t MAX_GROWTH_SLACK = 256;

        private:
                /// The room to leave when growing to nLength.
            static size_t       GrowthSlack( size_t nLength );
                /// Moves the text to a buffer of nCapacity characters (never
                /// less than the length); the stub then owns the buffer.
            void                Reallocate( size_t nCapacity );

                                /// Not copyable.
                                CStringStub( const CStringStub & );
                                /// Not assignable.
            CStringStub        &operator =( const CStringStub & );
    };
}

#endif // IASLIB_STRINGSTUB_H__
