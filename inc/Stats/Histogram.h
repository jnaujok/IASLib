/**
 * @file Histogram.h
 * @brief CHistogram, counts in numbered buckets split into categories.
 *
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
        /// Counts in numbered buckets, split into categories.
        ///
        /// Categories are numbered, or named with AddCategory(). The
        /// histogram is written out as CSV or XML: a row per bucket, a
        /// column per category. Bucket numbers may be sparse (only buckets
        /// with counts take room). Not thread-safe: share one between
        /// threads behind a lock. It cannot be copied.
        ///
        /// @code
        /// CHistogram hist;
        /// hist.AddToBucket( 3, "reads" );          // bucket 3, category "reads", +1
        /// hist.AddToBucket( 3, "writes", 5 );
        /// hist.SetBucketName( 3, "3 ms" );
        /// hist.Dump( stream, CHistogram::CSV );    // header row, then "3 ms, 1, 5"
        /// @endcode
    class CHistogram : public CObject
    {
        public:
                /// The formats Dump() writes.
            enum OutputFormatSpecifier
            {
                CSV         = 0x0001,   ///< Comma-separated values, a header row first.
                XML         = 0x0002,   ///< An XML table of tr and td elements.
            };
        protected:
            CSparseArray    m_aBuckets;         ///< Per bucket number, a CSparseArray of counts by category.
            CStringArray    m_aCategories;      ///< The category names, by number.
            CSparseArray    m_aBucketNames;     ///< The bucket names (CString), by bucket number.
            size_t          m_nColumns;         ///< Highest category used, plus one.

        public:
                                /// An empty histogram.
                                /// @param nGrowBy How the sparse bucket arrays grow
                                ///     (0: their default); it is not a bucket count.
                            CHistogram( size_t nGrowBy = 0 );
            virtual        ~CHistogram( void );

                            DEFINE_OBJECT( CHistogram )

                /// Adds llAdd to a bucket's count in a category.
                /// @param nBucketNumber The bucket.
                /// @param nCategory The category's number.
                /// @param llAdd The amount (may be negative).
                /// @return false if nCategory is NOT_FOUND.
            bool            AddToBucket( size_t nBucketNumber, size_t nCategory = 0, long long llAdd = 1 );
                /// Adds llAdd to a bucket's count in a named category, adding
                /// the category if it is new (see AddCategory()).
                /// @return true.
            bool            AddToBucket( size_t nBucketNumber, const char *strCategory, long long llAdd = 1 );
                /// The count in a bucket (0 if none).
            long long       GetCount( size_t nBucketNumber, size_t nCategory = 0 );

                /// The number of the named category, adding it if it is new
                /// (NULL means "").
            size_t          AddCategory( const char *strCategoryName );
                /// The number of categories: the highest used, plus one.
            size_t          GetCategoryCount( void ) const { return m_nColumns; }

                /// Writes every bucket that has a count (with bFillInGaps, the
                /// empty ones between them too). Every row has a cell per
                /// category; numbered categories without a name are headed
                /// by their number. Text is escaped for the format.
                /// @param oStream Where to write.
                /// @param nFormat CSV or XML.
                /// @param bFillInGaps Whether to write rows of zeros for the
                ///     buckets between those with counts.
            void            Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps = false );

                /// Whether the bucket has a name.
            bool            HasBucketName( size_t nBucketNumber );
                /// Names a bucket (NULL means ""); Dump() labels its row with
                /// the name instead of the number.
            void            SetBucketName( size_t nBucketNumber, const char *strBucketName );
                /// The bucket's name, or NULL if it has none. Good until the
                /// bucket's name is next set.
            const char     *GetBucketName( size_t nBucketNumber );
        private:
            size_t          m_nField;   ///< The cell number within the row being written.
                /// The number of the named category, or NOT_FOUND.
            size_t          FindCategory( const char *strCategoryName );

                /// Writes what comes before the rows.
            void            FormatStart( CStream &oStream, OutputFormatSpecifier nFormat );
                /// Starts a row.
            void            FormatStartLine( CStream &oStream, OutputFormatSpecifier nFormat );
                /// Writes a number cell.
            void            FormatNumber( CStream &oStream, unsigned long long ullValue, OutputFormatSpecifier nFormat );
                /// Writes a count cell.
            void            FormatCount( CStream &oStream, long long llValue, OutputFormatSpecifier nFormat );
                /// Writes a text cell, escaped for the format.
            void            FormatValue( CStream &oStream, const char *strValue, OutputFormatSpecifier nFormat );
                /// Writes a row's label: the bucket's name, or its number.
            void            FormatBucketLabel( CStream &oStream, size_t nBucket, OutputFormatSpecifier nFormat );
                /// Ends a row.
            void            FormatEndLine( CStream &oStream, OutputFormatSpecifier nFormat );
                /// Writes what comes after the rows.
            void            FormatEnd( CStream &oStream, OutputFormatSpecifier nFormat );

                            /// Not copyable.
                            CHistogram( const CHistogram & );
                            /// Not assignable.
            CHistogram     &operator =( const CHistogram & );
    };
} // namespace IASLib

#endif // IASLIB_HISTOGRAM_H__
