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

#include "../Collections/SparseArray.h"
#include "../Collections/StringArray.h"
#include "../Streams/Stream.h"

namespace IASLib
{
        // Counts in numbered buckets, split into categories (numbered, or
        // named with AddCategory()), written out as CSV or XML: a row per
        // bucket, a column per category. Not thread-safe: share one between
        // threads behind a lock.
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
            size_t          m_nColumns;         // highest category used, plus one

        public:
                                // nGrowBy is how the sparse bucket arrays grow
                                // (0: their default); it is not a bucket count.
                            CHistogram( size_t nGrowBy = 0 );
            virtual        ~CHistogram( void );

                            DEFINE_OBJECT( CHistogram )

            bool            AddToBucket( size_t nBucketNumber, size_t nCategory = 0, long long llAdd = 1 );
            bool            AddToBucket( size_t nBucketNumber, const char *strCategory, long long llAdd = 1 );
                // The count in a bucket (0 if none).
            long long       GetCount( size_t nBucketNumber, size_t nCategory = 0 );

            size_t          AddCategory( const char *strCategoryName );
            size_t          GetCategoryCount( void ) const { return m_nColumns; }

                // Writes every bucket that has a count (with bFillInGaps, the
                // empty ones between them too). Every row has a cell per
                // category; numbered categories without a name are headed
                // by their number. Text is escaped for the format.
            void            Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps = false );

            bool            HasBucketName( size_t nBucketNumber );
            void            SetBucketName( size_t nBucketNumber, const char *strBucketName );
                // Good until the bucket's name is next set.
            const char     *GetBucketName( size_t nBucketNumber );
        private:
            size_t          m_nField;
            size_t          FindCategory( const char *strCategoryName );

            void            FormatStart( CStream &oStream, OutputFormatSpecifier nFormat );
            void            FormatStartLine( CStream &oStream, OutputFormatSpecifier nFormat );
            void            FormatNumber( CStream &oStream, unsigned long long ullValue, OutputFormatSpecifier nFormat );
            void            FormatCount( CStream &oStream, long long llValue, OutputFormatSpecifier nFormat );
            void            FormatValue( CStream &oStream, const char *strValue, OutputFormatSpecifier nFormat );
            void            FormatBucketLabel( CStream &oStream, size_t nBucket, OutputFormatSpecifier nFormat );
            void            FormatEndLine( CStream &oStream, OutputFormatSpecifier nFormat );
            void            FormatEnd( CStream &oStream, OutputFormatSpecifier nFormat );

                            CHistogram( const CHistogram & );
            CHistogram     &operator =( const CHistogram & );
    };
} // namespace IASLib

#endif // IASLIB_HISTOGRAM_H__
