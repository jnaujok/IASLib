/*
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
        // A block of bytes that grows as it is written to, optionally with
        // "guardposts": bytes of a fixed pattern before and after the data,
        // checked by every operation, that catch writes outside the block
        // (through the pointers it hands out). A damaged guardpost throws a
        // CException (or, when the block is destroyed, is reported on
        // stderr, since a destructor must not throw). New bytes are zero.
    class CDataBlock : public CObject
    {
        protected:
            unsigned char  *m_pBuffer;          // guardpost, data, guardpost
            size_t          m_nSize;
            size_t          m_nGuardpostSize;
        public:
            static const unsigned char GUARDPOST_PATTERN = 0xAA;

                            CDataBlock( size_t nSize = 0, size_t nGuardpostSize = 16 );
                            CDataBlock( const unsigned char *pData, size_t nSize, size_t nGuardpostSize = 16 );
                            CDataBlock( const char *pData, size_t nSize, size_t nGuardpostSize = 16 );
                            CDataBlock( const CDataBlock &oSource );

            virtual        ~CDataBlock( void );

                            DEFINE_OBJECT( CDataBlock )

            size_t          GetSize( void ) const { return m_nSize; }
            size_t          GetGuardpostSize( void ) const { return m_nGuardpostSize; }

                // Copies nLength bytes to nOffset, growing the block if they
                // go past its end.
            void            Copy( const void *pSource, size_t nLength, size_t nOffset = 0 );

                // Sets nLength bytes from nOffset to chValue (like memset),
                // growing the block if they go past its end.
            void            Set( unsigned char chValue, size_t nLength, size_t nOffset = 0 );
            void            Set( char chValue, size_t nLength, size_t nOffset = 0 );

                // The data (a block of size 0 still gives a valid pointer).
                            operator const char *() const;
                            operator char *() const;
                            operator const unsigned char *() const;
                            operator unsigned char *() const;
                            operator const void *() const;
                            operator void *() const;

                // A byte of the block; the non-const form grows the block to
                // include nIndex, the const one gives 0 past the end.
            char           &operator []( size_t nIndex );
            char            operator []( size_t nIndex ) const;

            CDataBlock     &operator =( const CDataBlock &oSource );

                // Changes the size, keeping the data that still fits
                // (bPreserveData) or zeroing it all.
            void            ChangeSize( size_t nLength, bool bPreserveData = true );

                // Throws a CException if a guardpost has been written over.
            void            CheckGuardposts( void ) const;

        private:
            void            Resize( size_t nLength, bool bPreserveData );
            bool            GuardpostsIntact( void ) const;
            unsigned char  *Data( void ) const { return m_pBuffer + m_nGuardpostSize; }
    };
} // namespace IASLib

#endif // IASLIB_DATABLOCK_H__
