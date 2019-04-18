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
    class CDataBlock : public CObject
    {
        protected:
            unsigned char  *m_pBuffer;
            size_t          m_nSize;
            size_t          m_nGuardpostSize;
        public:
                            CDataBlock( size_t nSize, size_t nGuardpostSize = 16 );
                            CDataBlock( const unsigned char *pData, size_t nSize, size_t nGuardpostSize = 16 );
                            CDataBlock( const char *pData, size_t nSize, size_t nGuardpostSize = 16 );
                            CDataBlock( const CDataBlock &oSource );

            virtual        ~CDataBlock( void );

                            DEFINE_OBJECT( CDataBlock )

            void            Copy( const void *nSource, size_t nLength, size_t nOffset = 0 );

            void            Set( unsigned char chValue, size_t nLength, size_t nOffset = 0 );
            void            Set( char chValue, size_t nLength, size_t nOffset = 0 );

                            operator const char *() const;
                            operator char *() const;
                            operator const unsigned char *() const;
                            operator unsigned char *() const;
                            operator const void *() const;
                            operator void *() const;

            char           &operator []( size_t nIndex );

            CDataBlock     &operator =( const CDataBlock &oSource );
                            
            void            ChangeSize( size_t nLength, bool bPreserveData=true );
        private:
            void            Resize( size_t nLength, bool bCopy = true );
            void            CheckGuardposts( void ) const;
    };
} // namespace IASLib

#endif // IASLIB_DATABLOCK_H__

