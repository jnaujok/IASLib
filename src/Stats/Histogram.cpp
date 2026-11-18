/**
 * Histogram Class
 *
 *  This class provides a means of producing a histogram of data. The 
 * histogram can have any number of buckets and categories to drop 
 * data into. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2/16/2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "Histogram.h"
#include <string.h>

    // (The Stats sources were wrapped in IASLIB_STATS__, which the CMake
    // build never defines, so the library shipped them empty while
    // IASLib.h declared them to anyone who did define it.)

namespace IASLib
{
    class CHistogramBucket : public CObject
    {
        protected:
            long long       m_llValue;          // (an int, which overflowed)
        public:
                            CHistogramBucket( void ) { m_llValue = 0; }
            virtual        ~CHistogramBucket( void ) {}

                            DECLARE_OBJECT(CHistogramBucket,CObject);

            long long       GetValue( void ) const { return m_llValue; }
            void            SetValue( long long llValue ) { m_llValue = llValue; }
    };

    namespace
    {
            // A CSV field, quoted when it holds a comma, quote or line break.
        CString csvField( const char *strValue )
        {
            CString strField( strValue ? strValue : "" );
            if ( strpbrk( (const char *)strField, ",\"\r\n" ) == NULL )
                return strField;
            CString strQuoted( "\"" );
            for ( const char *p = strValue; *p; p++ )
            {
                if ( *p == '"' )
                    strQuoted += "\"\"";
                else
                    strQuoted += *p;
            }
            strQuoted += "\"";
            return strQuoted;
        }

        CString xmlText( const char *strValue )
        {
            CString strText;
            for ( const char *p = strValue ? strValue : ""; *p; p++ )
            {
                switch ( *p )
                {
                    case '&':   strText += "&amp;";     break;
                    case '<':   strText += "&lt;";      break;
                    case '>':   strText += "&gt;";      break;
                    case '"':   strText += "&quot;";    break;
                    default:    strText += *p;          break;
                }
            }
            return strText;
        }
    }

    IMPLEMENT_OBJECT( CHistogram, CObject );

    CHistogram::CHistogram( size_t nGrowBy ) :
        m_aBuckets( nGrowBy ),
        m_aCategories( 16 ),
        m_aBucketNames( nGrowBy )
    {
        m_nColumns = 0;
        m_nField = 0;
    }

    CHistogram::~CHistogram( void )
    {
        m_aBuckets.DeleteAll();
        m_aBucketNames.DeleteAll();
    }

    bool CHistogram::AddToBucket( size_t nBucketNumber, size_t nCategory, long long llAdd )
    {
        if ( nCategory == IASLib::NOT_FOUND )
            return false;

        if ( ! m_aBuckets.Exists( nBucketNumber ) )
        {
            m_aBuckets.Insert( nBucketNumber, new CSparseArray( 16 ) );
        }
        CSparseArray *pCategoryArray = (CSparseArray *)( m_aBuckets.Get( nBucketNumber ) );

        if ( ! pCategoryArray->Exists( nCategory ) )
        {
            pCategoryArray->Insert( nCategory, new CHistogramBucket() );
        }
        CHistogramBucket *pBucket = (CHistogramBucket *)( pCategoryArray->Get( nCategory ) );
        pBucket->SetValue( pBucket->GetValue() + llAdd );

            // (A category past the named ones gave that row more cells than
            // the header.)
        if ( nCategory + 1 > m_nColumns )
            m_nColumns = nCategory + 1;
        return true;
    }

    bool CHistogram::AddToBucket( size_t nBucketNumber, const char *strCategoryName, long long llAdd )
    {
        return AddToBucket( nBucketNumber, AddCategory( strCategoryName ), llAdd );
    }

    long long CHistogram::GetCount( size_t nBucketNumber, size_t nCategory )
    {
        if ( ! m_aBuckets.Exists( nBucketNumber ) )
            return 0;
        CSparseArray *pCategoryArray = (CSparseArray *)( m_aBuckets.Get( nBucketNumber ) );
        if ( ! pCategoryArray->Exists( nCategory ) )
            return 0;
        return ((CHistogramBucket *)pCategoryArray->Get( nCategory ))->GetValue();
    }

    size_t CHistogram::AddCategory( const char *strCategoryName )
    {
        if ( strCategoryName == NULL )
            strCategoryName = "";

        size_t nFind = FindCategory( strCategoryName );
        if ( nFind == IASLib::NOT_FOUND )
        {
            CString strTemp = strCategoryName;
            m_aCategories.Push( strTemp );
            nFind = m_aCategories.GetLength() - 1;
        }
        if ( nFind + 1 > m_nColumns )
            m_nColumns = nFind + 1;
        return nFind;
    }

    bool CHistogram::HasBucketName( size_t nBucketNumber )
    {
       return m_aBucketNames.Exists( nBucketNumber );
    }

    void CHistogram::SetBucketName( size_t nBucketNumber, const char *strBucketName )
    {
        if ( strBucketName == NULL )
            strBucketName = "";
        if ( m_aBucketNames.Exists( nBucketNumber ) )
        {
            CString *pstrData = (CString *)m_aBucketNames[ nBucketNumber ];
            (*pstrData) = strBucketName;
        }
        else
        {
            m_aBucketNames.Insert( nBucketNumber, new CString( strBucketName ) );
        }
    }

    const char *CHistogram::GetBucketName( size_t nBucketNumber )
    {
        if ( m_aBucketNames.Exists( nBucketNumber ) )
        {
            CString *pstrData = (CString *)m_aBucketNames[ nBucketNumber ];
            return (const char *)(*pstrData);
        }
        return NULL;
    }

    void CHistogram::FormatBucketLabel( CStream &oStream, size_t nBucket, OutputFormatSpecifier nFormat )
    {
        if ( m_aBucketNames.Exists( nBucket ) )
            FormatValue( oStream, GetBucketName( nBucket ), nFormat );
        else
            FormatNumber( oStream, (unsigned long long)nBucket, nFormat );
    }

    void CHistogram::Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps )
    {
        size_t nColumns = m_nColumns;

        FormatStart( oStream, nFormat );

            // The header: an empty cell over the bucket labels, then the
            // categories (by number when they have no name).
        FormatStartLine( oStream, nFormat );
        FormatValue( oStream, "", nFormat );
        for ( size_t nColumn = 0; nColumn < nColumns; nColumn++ )
        {
            if ( nColumn < m_aCategories.GetCount() )
                FormatValue( oStream, m_aCategories.Get( nColumn ), nFormat );
            else
                FormatNumber( oStream, (unsigned long long)nColumn, nFormat );
        }
        FormatEndLine( oStream, nFormat );

        CIterator  *pIterator1 = m_aBuckets.Enumerate();
        bool        bFirst = true;
        size_t      nNext = 0;          // the bucket after the last row

        while ( pIterator1->HasMore() )
        {
            CSparseArray *pCategoryArray = (CSparseArray *)( pIterator1->Next() );
            size_t nBucket = ((CSparseArrayIterator *)pIterator1)->GetSparseNumber();

                // (The first-row marker was NOT_FOUND, so a bucket numbered
                // that was taken for the start.)
            if ( bFillInGaps && ! bFirst )
            {
                for ( size_t nGap = nNext; nGap < nBucket; nGap++ )
                {
                    FormatStartLine( oStream, nFormat );
                    FormatBucketLabel( oStream, nGap, nFormat );
                    for ( size_t nColumn = 0; nColumn < nColumns; nColumn++ )
                        FormatCount( oStream, 0, nFormat );
                    FormatEndLine( oStream, nFormat );
                }
            }
            bFirst = false;
            nNext = nBucket + 1;

            FormatStartLine( oStream, nFormat );
            FormatBucketLabel( oStream, nBucket, nFormat );

            CIterator *pIterator2 = pCategoryArray->Enumerate();
            size_t nCurrentCat = 0;
            while ( pIterator2->HasMore() )
            {
                CHistogramBucket *pBucket = (CHistogramBucket *)( pIterator2->Next() );
                size_t nCategory = ((CSparseArrayIterator *)pIterator2)->GetSparseNumber();

                while ( nCurrentCat < nCategory )
                {
                    FormatCount( oStream, 0, nFormat );
                    nCurrentCat++;
                }
                FormatCount( oStream, pBucket->GetValue(), nFormat );
                nCurrentCat++;
            }
            delete pIterator2;
            while ( nCurrentCat < nColumns )
            {
                FormatCount( oStream, 0, nFormat );
                nCurrentCat++;
            }
            FormatEndLine( oStream, nFormat );
        }
        delete pIterator1;

        FormatEnd( oStream, nFormat );
    }

    void CHistogram::FormatStartLine( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        if ( nFormat == CHistogram::XML )
            oStream.PutBuffer( "  <tr>\n", 7 );
        m_nField = 0;
    }

    void CHistogram::FormatEndLine( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        if ( nFormat == CHistogram::CSV )
            oStream.PutBuffer( "\n", 1 );
        else if ( nFormat == CHistogram::XML )
            oStream.PutBuffer( "  </tr>\n", 8 );
        m_nField = 0;
    }

    void CHistogram::FormatNumber( CStream &oStream, unsigned long long ullValue, OutputFormatSpecifier nFormat )
    {
            // (Bucket numbers were cast to int, so large ones came out
            // negative.)
        CString strTemp;
        strTemp.Format( "%llu", ullValue );
        FormatValue( oStream, strTemp, nFormat );
    }

    void CHistogram::FormatCount( CStream &oStream, long long llValue, OutputFormatSpecifier nFormat )
    {
        CString strTemp;
        strTemp.Format( "%lld", llValue );
        FormatValue( oStream, strTemp, nFormat );
    }

    void CHistogram::FormatValue( CStream &oStream, const char *strValue, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CHistogram::CSV:
                    // A separator before every field but the first. (It went
                    // by the characters written, so an empty first cell left
                    // out the comma after it and shifted the header left.)
                if ( m_nField != 0 )
                {
                    strTemp = ", ";
                }
                strTemp += csvField( strValue );
                break;

            case CHistogram::XML:
                strTemp = "    <td>";
                strTemp += xmlText( strValue );
                strTemp += "</td>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nField++;
    }

    void CHistogram::FormatStart( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        m_nField = 0;
        if ( nFormat == CHistogram::XML )
        {
            CString strTemp( "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<table>\n" );
            oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        }
    }

    void CHistogram::FormatEnd( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        if ( nFormat == CHistogram::XML )
            oStream.PutBuffer( "</table>\n", 9 );
    }

    size_t CHistogram::FindCategory( const char *strCategoryName )
    {
        for ( size_t nCount = 0; nCount < m_aCategories.GetLength(); nCount++ )
        {
            if ( m_aCategories.Get( nCount ) == strCategoryName )
            {
                return nCount;
            }
        }
        return IASLib::NOT_FOUND;
    }
} // namespace IASLib
