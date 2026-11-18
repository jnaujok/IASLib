/**
 * @file DataBlock.h
 * @brief CDataBlock, a growable block of bytes with optional guardposts.
 *
 * CDataBlock Class
 *
 *	One of the most common things to do in programs is to allocate a data
 * block, and then write past the end of it...
 *  This class supports a dynamically allocated data block and the support
 * functions (copy, dereference, etc.) that go with it. The data block also
 * supports guardposting to prevent unauthorized writes before and after
 * the data block, although this can be turned on and off at will to save
 * CPU time and memory space.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 03/26/2003
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_DATABLOCK_H__
#define IASLIB_DATABLOCK_H__

#include "Object.h"

namespace IASLib
{
        /// A block of bytes that grows as it is written to, optionally with
        /// guardposts.
        ///
        /// Guardposts are bytes of a fixed pattern (GUARDPOST_PATTERN) before
        /// and after the data, checked by every operation, that catch writes
        /// outside the block (through the pointers it hands out). A damaged
        /// guardpost throws a CException (or, when the block is destroyed, is
        /// reported on stderr, since a destructor must not throw). A
        /// guardpost size of 0 turns them off. New bytes are zero. The block
        /// owns its memory; pointers it gives out last until it is resized
        /// or destroyed. Running out of memory throws std::bad_alloc.
        ///
        /// @code
        /// CDataBlock block;                // empty, 16-byte guardposts
        /// block.Copy( "abc", 3, 10 );      // grows to 13 bytes
        /// unsigned char *p = block;        // the data
        /// block.CheckGuardposts();         // throws if p[ -1 ] or p[ 13 ] was written
        /// @endcode
    class CDataBlock : public CObject
    {
        protected:
            unsigned char  *m_pBuffer;          ///< Guardpost, data, guardpost.
            size_t          m_nSize;            ///< Bytes of data.
            size_t          m_nGuardpostSize;   ///< Bytes in each guardpost (0: none).
        public:
                /// The byte every guardpost is filled with.
            static const unsigned char GUARDPOST_PATTERN = 0xAA;

                            /// A block of nSize zero bytes.
                            /// @param nSize The size in bytes.
                            /// @param nGuardpostSize Bytes in each guardpost (0: none).
                            /// @throws std::bad_alloc When memory runs out.
                            CDataBlock( size_t nSize = 0, size_t nGuardpostSize = 16 );
                            /// A block holding a copy of nSize bytes from pData
                            /// (zeros if pData is NULL).
                            /// @param pData The bytes to copy, or NULL.
                            /// @param nSize The size in bytes.
                            /// @param nGuardpostSize Bytes in each guardpost (0: none).
                            /// @throws std::bad_alloc When memory runs out.
                            CDataBlock( const unsigned char *pData, size_t nSize, size_t nGuardpostSize = 16 );
                            /// @copydoc CDataBlock(const unsigned char*,size_t,size_t)
                            CDataBlock( const char *pData, size_t nSize, size_t nGuardpostSize = 16 );
                            /// A copy of oSource's data, with the same guardpost size.
                            /// @throws CException If oSource's guardposts are damaged.
                            CDataBlock( const CDataBlock &oSource );

                            /// Frees the block, reporting damaged guardposts on stderr.
            virtual        ~CDataBlock( void );

                            DEFINE_OBJECT( CDataBlock )

                /// The size of the data in bytes.
            size_t          GetSize( void ) const { return m_nSize; }
                /// The bytes in each guardpost (0: none).
            size_t          GetGuardpostSize( void ) const { return m_nGuardpostSize; }

                /// Copies nLength bytes to nOffset, growing the block if they
                /// go past its end (the source may overlap the block). Does
                /// nothing if pSource is NULL or nLength is 0.
                /// @param pSource The bytes to copy.
                /// @param nLength How many bytes.
                /// @param nOffset Where in the block they go.
                /// @throws CException If a guardpost is damaged.
                /// @throws std::bad_alloc When memory runs out.
            void            Copy( const void *pSource, size_t nLength, size_t nOffset = 0 );

                /// Sets nLength bytes from nOffset to chValue (like memset),
                /// growing the block if they go past its end.
                /// @param chValue The byte value.
                /// @param nLength How many bytes.
                /// @param nOffset The first byte to set.
                /// @throws CException If a guardpost is damaged.
                /// @throws std::bad_alloc When memory runs out.
            void            Set( unsigned char chValue, size_t nLength, size_t nOffset = 0 );
                /// @copydoc Set(unsigned char,size_t,size_t)
            void            Set( char chValue, size_t nLength, size_t nOffset = 0 );

                /// The data (a block of size 0 still gives a valid pointer).
                /// Every pointer conversion checks the guardposts first.
                /// @throws CException If a guardpost is damaged.
                            operator const char *() const;
                /// The data, as char (checks the guardposts; throws a CException if one is damaged).
                            operator char *() const;
                /// The data, as const unsigned char (checks the guardposts; throws a CException if one is damaged).
                            operator const unsigned char *() const;
                /// The data, as unsigned char (checks the guardposts; throws a CException if one is damaged).
                            operator unsigned char *() const;
                /// The data, as const void (checks the guardposts; throws a CException if one is damaged).
                            operator const void *() const;
                /// The data, as void (checks the guardposts; throws a CException if one is damaged).
                            operator void *() const;

                /// A byte of the block: this non-const form grows the block to
                /// include nIndex.
                /// @throws CException If a guardpost is damaged.
                /// @throws std::bad_alloc When memory runs out.
            char           &operator []( size_t nIndex );
                /// A byte of the block: this const form gives 0 past the end.
                /// @throws CException If a guardpost is damaged.
            char            operator []( size_t nIndex ) const;

                /// Makes this block a copy of oSource (data and guardpost
                /// size). On failure this block stays as it was.
                /// @throws CException If either block's guardposts are damaged.
                /// @throws std::bad_alloc When memory runs out.
            CDataBlock     &operator =( const CDataBlock &oSource );

                /// Changes the size, keeping the data that still fits
                /// (bPreserveData) or zeroing it all. New bytes are zero.
                /// @param nLength The new size in bytes.
                /// @param bPreserveData Whether to keep the data.
                /// @throws CException If a guardpost is damaged.
                /// @throws std::bad_alloc When memory runs out.
            void            ChangeSize( size_t nLength, bool bPreserveData = true );

                /// Throws a CException if a guardpost has been written over.
            void            CheckGuardposts( void ) const;

        private:
                /// Moves the data to a new buffer of nLength bytes, with fresh guardposts.
            void            Resize( size_t nLength, bool bPreserveData );
                /// Whether both guardposts still hold the pattern.
            bool            GuardpostsIntact( void ) const;
                /// The first byte of data, after the front guardpost.
            unsigned char  *Data( void ) const { return m_pBuffer + m_nGuardpostSize; }
    };
} // namespace IASLib

#endif // IASLIB_DATABLOCK_H__
