/**
 * Histogram Class
 *
 *  This class provides a means of producing a histogram of data. The 
 * histogram can have any number of buckets and categories to drop 
 * data into. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 1/15/1994
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_HISTOGRAM_H__
#define IASLIB_HISTOGRAM_H__

#ifdef IASLIB_STATS__

#include "../Collections/SparseArray.h"
#include "../Collections/StringArray.h"
#include "../Streams/Stream.h"

namespace IASLib
{
    class CHistogram : public CObject
    {
        public:
            enum OutputFormatSpecifier
            {
                CSV         = 0x0001,
                XML         = 0x0002,
            };
        protected:
            CSparseArray    m_aBuckets;
            CStringArray    m_aCategories;
            CSparseArray    m_aBucketNames;

        public:
                            CHistogram( size_t nBuckets = 0 );
            virtual        ~CHistogram( void );

            bool            AddToBucket( size_t nBucketNumber, size_t nCategory = 0, int nAdd=1 );
            bool            AddToBucket( size_t nBucketNumber, const char *strCategory, int nAdd=1 );

            size_t          AddCategory( const char *strCategoryName );

            void            Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps = false );

            bool            HasBucketName( size_t nBucketNumber );
            void            SetBucketName( size_t nBucketNumber, const char *strBucketName );
            const char     *GetBucketName( size_t nBucketNumber );
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

#endif // IASLIB_HISTOGRAM_H__
