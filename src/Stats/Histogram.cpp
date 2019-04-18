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

#ifndef IASLIB_WIN32
#include <iostream>
#else
#include <ostream>
#endif


#ifdef IASLIB_STATS__

namespace IASLib
{
    class CHistogramBucket : public CObject
    {
        protected:
            int             m_nValue;
        public:
                            CHistogramBucket( void )
                            {
                                m_nValue = 0;
                            }

            virtual        ~CHistogramBucket( void ) {}

                            DECLARE_OBJECT(CHistogramBucket,CObject);

            int             GetValue( void ) { return m_nValue; }
            void            SetValue( int nValue ) { m_nValue = nValue; }
    };

    CHistogram::CHistogram( size_t nBuckets ) :
        m_aBuckets( nBuckets ),
        m_aCategories( 16 ),
        m_aBucketNames( nBuckets )
    {
    }

    CHistogram::~CHistogram( void )
    {
        m_aBuckets.DeleteAll();
        m_aCategories.DeleteAll();
        m_aBucketNames.DeleteAll();
    }

    bool CHistogram::AddToBucket( size_t nBucketNumber, 
                                  size_t nCategory, 
                                  int nAdd )
    {
        CSparseArray *pCategoryArray = NULL;
        CHistogramBucket *pBucket = NULL;

        if ( ! m_aBuckets.Exists( nBucketNumber ) )
        {
            pCategoryArray = new CSparseArray( 16 );
            m_aBuckets.Insert( nBucketNumber, pCategoryArray );
        }

        pCategoryArray = (CSparseArray *)(m_aBuckets.Get( nBucketNumber ));

        if ( ! pCategoryArray->Exists( nCategory ) )
        {
            pBucket = new CHistogramBucket();
            pCategoryArray->Insert( nCategory, pBucket );
        }

        pBucket = (CHistogramBucket *)(pCategoryArray->Get( nCategory ) );
        pBucket->SetValue( pBucket->GetValue() + nAdd );

        return true;
    }

    bool CHistogram::AddToBucket( size_t nBucketNumber, 
                                  const char *strCategoryName, 
                                  int nAdd )
    {
        size_t nFind = FindCategory( strCategoryName );

        if ( nFind == IASLib::NOT_FOUND )
        {
            nFind = AddCategory( strCategoryName );
        }

        return AddToBucket( nBucketNumber, nFind, nAdd );
    }

    size_t CHistogram::AddCategory( const char *strCategoryName )
    {
        size_t nFind = FindCategory( strCategoryName );

        if ( nFind == IASLib::NOT_FOUND )
        {
            CString strTemp = strCategoryName;
            m_aCategories.Push( strTemp );

            return m_aCategories.GetLength() - 1;
        }

        return nFind;
    }

    bool CHistogram::HasBucketName( size_t nBucketNumber )
    {
       return  m_aBucketNames.Exists( nBucketNumber );
    }

    void CHistogram::SetBucketName( size_t nBucketNumber, const char *strBucketName )
    {
       if ( m_aBucketNames.Exists( nBucketNumber ) )
       {
            CString *pstrData = (CString *)m_aBucketNames[ nBucketNumber ];
            (*pstrData) = strBucketName;
       }
       else
       {
            CString *pstrData = new CString( strBucketName );
            m_aBucketNames.Insert( nBucketNumber, pstrData );
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
 

    void CHistogram::Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps )
    {
        CIterator  *pIterator1 = m_aBuckets.Enumerate();
        size_t      nBucket;
        size_t      nStart = IASLib::NOT_FOUND;
        size_t      nCurrent = IASLib::NOT_FOUND;
        size_t      nCategories = m_aCategories.GetCount();

        FormatStart( oStream, nFormat );
        FormatStartLine( oStream, nFormat );
        size_t      nTemp = 0;

            // Leave an empty column for Bucket Name
        FormatValue( oStream, "", nFormat );

        while ( nTemp < m_aCategories.GetCount() )
        {
            FormatValue( oStream, m_aCategories.Get( nTemp ), nFormat );
            nTemp++;
        }
        FormatEndLine( oStream, nFormat );

        while ( pIterator1->HasMore() )
        {
            CSparseArray *pCategoryArray = (CSparseArray *)(pIterator1->Next());
            nBucket = ((CSparseArrayIterator *)pIterator1)->GetSparseNumber();

            if ( nStart == IASLib::NOT_FOUND )
            {
                nStart = nBucket;
                nCurrent = nBucket;
            }
            else
            {
                while ( ( bFillInGaps ) && ( nCurrent < nBucket ) )
                {
                    FormatStartLine( oStream, nFormat );
                    if ( m_aBucketNames.Exists( nCurrent ) )
                    {
                        FormatValue( oStream, GetBucketName( nCurrent ), nFormat );
                    }
                    else
                    {
                        FormatValue( oStream, (int)nCurrent, nFormat );
                    }
                    for ( size_t nCount = 0; nCount < nCategories; nCount++ )
                    {
                        FormatValue( oStream, 0, nFormat );
                    }
                    nCurrent++;
                    FormatEndLine( oStream, nFormat );
                }
                nCurrent = nBucket;
            }

            FormatStartLine( oStream, nFormat );
            if ( m_aBucketNames.Exists( nCurrent ) )
            {
                FormatValue( oStream, GetBucketName( nCurrent ), nFormat );
            }
            else
            {
                FormatValue( oStream, (int)nCurrent, nFormat );
            }

            CIterator *pIterator2 = pCategoryArray->Enumerate();
            size_t nCategory;
            size_t nCurrentCat = 0;

            while ( pIterator2->HasMore() )
            {
                CHistogramBucket *pBucket = (CHistogramBucket *)(pIterator2->Next());
                nCategory = ((CSparseArrayIterator *)pIterator2)->GetSparseNumber();

                while ( nCurrentCat < nCategory )
                {
                    FormatValue( oStream, 0, nFormat );
                    nCurrentCat++;
                }

                FormatValue( oStream, pBucket->GetValue(), nFormat );
                nCurrentCat++;
            }
            while ( nCurrentCat < nCategories )
            {
                FormatValue( oStream, 0, nFormat );
                nCurrentCat++;
            }

            FormatEndLine( oStream, nFormat );
            nCurrent++;
            if ( pIterator2 )
            {
               delete pIterator2;
            }
        }

        FormatEnd( oStream, nFormat );
        if ( pIterator1 )
        {
            delete pIterator1;
        }
    }

    void CHistogram::FormatStartLine( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CHistogram::CSV:
                break;

            case CHistogram::XML:
                strTemp = "  <tr>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nLinePosition = 0;
    }

    void CHistogram::FormatEndLine( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CHistogram::CSV:
                strTemp = "\n";
                break;

            case CHistogram::XML:
                strTemp = "  </tr>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nLinePosition = 0;
    }

    void CHistogram::FormatValue( CStream &oStream, int nValue, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        strTemp.Format( "%d", nValue );

        FormatValue( oStream, strTemp, nFormat );
    }

    void CHistogram::FormatValue( CStream &oStream, const char *strValue, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CHistogram::CSV:
                if ( m_nLinePosition != 0 )
                {
                    strTemp = ", ";
                }
                strTemp += strValue;
                break;

            case CHistogram::XML:
                strTemp.Format( "    <td>%s</td>\n", strValue );
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nLinePosition += (int)strTemp.GetLength();
    }

    void CHistogram::FormatStart( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        m_nLinePosition = 0;

        switch ( nFormat )
        {
            case CHistogram::CSV:
                break;

            case CHistogram::XML:
                strTemp = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<table>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
    }

    void CHistogram::FormatEnd( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CHistogram::CSV:
                break;

            case CHistogram::XML:
                strTemp = "</table>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
    }

    size_t CHistogram::FindCategory( const char *strCategoryName )
    {
        size_t nCount = 0;

        while ( nCount < m_aCategories.GetLength() )
        {
            if ( m_aCategories.Get( nCount ) == strCategoryName )
            {
                return nCount;
            }
            nCount++;
        }

        return IASLib::NOT_FOUND;
    }
} // namespace IASLib

#endif // IASLIB_STATS__

