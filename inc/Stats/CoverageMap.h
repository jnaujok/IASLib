/**
 * Coverage Map Class
 *
 *  This class provides a means of producing a map or snapshot of usage
 * of a limited resource based on the use of small, non-overlapping parts
 * of that range. For the main example, this is used to analyze memory
 * and disk fragmentation. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 8/31/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_COVERAGEMAP_H__
#define IASLIB_COVERAGEMAP_H__

#ifdef IASLIB_STATS__

#include "../Collections/SortedArray.h"
#include "../Streams/Stream.h"

namespace IASLib
{
    class CCoverageMap : public CObject
    {
        public:
            enum OutputFormatSpecifier
            {
                CSV         = 0x0001,
                XML         = 0x0002,
            };
        protected:
            CSortedArray    m_aCoverageMap;
            size_t          m_nMinimum;
            size_t          m_nMaximum;
            double          m_fFragmentation;
            size_t          m_nUsed;
            size_t          m_nFree;
            size_t          m_nBlockSize;
        public:
                            CCoverageMap( void );
            virtual        ~CCoverageMap( void );

            bool            Allocate( size_t nBase, size_t nSize );
            bool            Release( size_t nBase, size_t nSize = IASLib::NOT_FOUND );

            size_t          GetSize( void ) { return m_nMaximum - m_nMinimum; }
            size_t          GetUsed( void ) { return m_nUsed; }
            size_t          GetFree( void ) { return m_nFree; }

            void            SetBlockSize( size_t nBlockSize ) { m_nBlockSize = nBlockSize; }


            void            Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps = false );

        private:
            int             m_nLinePosition;
            size_t          FindCategory( const char *strCategoryName );

            void            FormatStart( CStream &oStream, OutputFormatSpecifier nFormat );

            void            FormatStartLine( CStream &oStream, OutputFormatSpecifier nFormat );

            void            FormatValue( CStream &oStream, int nValue, OutputFormatSpecifier nFormat );
            void            FormatValue( CStream &oStream, const char *strValue, OutputFormatSpecifier nFormat );

            void            FormatEndLine( CStream &oStream, OutputFormatSpecifier nFormat );

            void            FormatEnd( CStream &oStream, OutputFormatSpecifier nFormat );
    };
} // namespace IASLib

#endif // IASLIB_STATS__

#endif // IASLIB_COVERAGEMAP_H__
