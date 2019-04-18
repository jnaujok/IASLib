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

#include "CoverageMap.h"

#ifndef IASLIB_WIN32
#include <iostream>
#else
#include <ostream>
#endif

#ifdef IASLIB_STATS__

namespace IASLib
{
    class CCoverageObject : public CObject
    {
        protected:
            size_t      m_nLocation;
            size_t      m_nSize;
        public:
                        CCoverageObject( size_t nLoc, size_t nSize )
                        {
                            m_nLocation = nLoc;
                            m_nSize = nSize;
                        }
            virtual    ~CCoverageObject() { }

                        DECLARE_OBJECT( CCoverageObject, CObject );

            size_t      GetSize( void ) { return m_nSize; }
            size_t      GetLocation( void ) {return m_nLocation; }

            void        SetSize( size_t nSize ) { m_nSize = nSize; }
    };

    static int ObjectCompareFunc( const CObject *p1, const CObject *p2 )
    {
        if ( p1 == p2 )
            return 0;

        return -1;
    }

    CCoverageMap::CCoverageMap( void )
        : m_aCoverageMap( ObjectCompareFunc, 2048 )
    {
        m_nMinimum = IASLib::NOT_FOUND;
        m_nMaximum = 0;
        m_fFragmentation = 0.0;
        m_nUsed = 0;
        m_nFree = 0;
        m_nBlockSize = 4096;
    }

    CCoverageMap::~CCoverageMap( void )
    {
        m_aCoverageMap.DeleteAll();
    }

    bool CCoverageMap::Allocate( size_t nBase, size_t nSize )
    {
        CCoverageObject *pNew = new CCoverageObject( nBase, nSize );
        bool bRetVal = true;
        size_t nEnd = nBase + nSize;

        size_t nFound = m_aCoverageMap.Find( pNew );
        if ( nFound != IASLib::NOT_FOUND )
        {
            delete pNew;
            pNew = (CCoverageObject *)m_aCoverageMap.Get( nFound );
            if ( ( pNew->GetLocation() + pNew->GetSize() ) < nEnd )
            {
                pNew->SetSize( nEnd - pNew->GetLocation() );
            }
            bRetVal = false;
        }
        else
        {
            m_aCoverageMap.Append( pNew );            
        }
        if ( nBase < m_nMinimum )
        {
            m_nMinimum = nBase;
        }
        if ( nEnd > m_nMaximum )
        {
            m_nMaximum = nEnd;
        }
        return bRetVal;
    }

    bool CCoverageMap::Release( size_t nBase, size_t nSize )
    {
        CCoverageObject *pNew;
        bool bRetVal = true;

        if ( nSize != IASLib::NOT_FOUND )
        {
            pNew = new CCoverageObject( nBase, nSize );
        }
        else
        {
            pNew = new CCoverageObject( nBase, 0 );
        }

        size_t nFound = m_aCoverageMap.Find( pNew );

        delete pNew;

        pNew = (CCoverageObject *)m_aCoverageMap.Get( nFound );

        if ( nFound == IASLib::NOT_FOUND )
        {
            bRetVal = false;
        }
        else
        {
            if ( nBase != pNew->GetLocation() )
            {
                // Uh-oh, we have a chunk being freed out of the middle 
                // of a block, or possibly crossing two or more blocks.
                // We need to figure out how to do this...
                if ( nBase < pNew->GetLocation() )
                {
                    // we have to back up through the coverage array and 
                    // look for the first block that contains this memory
                    // block.
                    while ( ( nFound > 0 ) && ( nBase < pNew->GetLocation() ) )
                    {
                        nFound--;
                        pNew = (CCoverageObject *)m_aCoverageMap.Get( nFound );
                    }
                    
                    if ( ( pNew->GetLocation() < nBase ) && 
                         ( nBase > ( pNew->GetLocation() + pNew->GetSize() ) ) )
                    {
                        nFound++;
                    }

                    // Well, by here we've found the bottom edge. Now
                    // we have to walk forward until we have cleaned up to
                    // the top edge. This might mean making two partial 
                    // blocks (one at the begining, one at the end.
                    if ( pNew->GetLocation() != nBase )
                    {
                        pNew->SetSize( nBase - pNew->GetLocation() );
                    }
                    else
                    {

                    }
                }
            }
            else
            {
                if ( nSize == IASLib::NOT_FOUND )
                {
                    nSize = pNew->GetSize();
                }
            }

            if ( nSize < pNew->GetSize() )
            {
                // We need to split this item into one or more blocks.
            }
        }
        return bRetVal;
    }

    void CCoverageMap::Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps )
    {
        CIterator  *pIterator1 = m_aCoverageMap.Enumerate();
        size_t      nBucket;
        size_t      nStart = IASLib::NOT_FOUND;
        size_t      nCurrent = IASLib::NOT_FOUND;

        FormatStart( oStream, nFormat );
        FormatStartLine( oStream, nFormat );
        size_t      nTemp = 0;

            // Leave an empty column for Bucket Name
        FormatValue( oStream, "", nFormat );

        FormatEnd( oStream, nFormat );
        if ( pIterator1 )
        {
            delete pIterator1;
        }
    }

    void CCoverageMap::FormatStartLine( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CCoverageMap::CSV:
                break;

            case CCoverageMap::XML:
                strTemp = "  <tr>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nLinePosition = 0;
    }

    void CCoverageMap::FormatEndLine( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CCoverageMap::CSV:
                strTemp = "\n";
                break;

            case CCoverageMap::XML:
                strTemp = "  </tr>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nLinePosition = 0;
    }

    void CCoverageMap::FormatValue( CStream &oStream, int nValue, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        strTemp.Format( "%d", nValue );

        FormatValue( oStream, strTemp, nFormat );
    }

    void CCoverageMap::FormatValue( CStream &oStream, const char *strValue, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CCoverageMap::CSV:
                if ( m_nLinePosition != 0 )
                {
                    strTemp = ", ";
                }
                strTemp += strValue;
                break;

            case CCoverageMap::XML:
                strTemp.Format( "    <td>%s</td>\n", strValue );
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
        m_nLinePosition += (int)strTemp.GetLength();
    }

    void CCoverageMap::FormatStart( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        m_nLinePosition = 0;

        switch ( nFormat )
        {
            case CCoverageMap::CSV:
                break;

            case CCoverageMap::XML:
                strTemp = "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<table>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
    }

    void CCoverageMap::FormatEnd( CStream &oStream, OutputFormatSpecifier nFormat )
    {
        CString strTemp;

        switch ( nFormat )
        {
            case CCoverageMap::CSV:
                break;

            case CCoverageMap::XML:
                strTemp = "</table>\n";
                break;

            default:
                break;
        }

        oStream.PutBuffer( (const char *)strTemp, (int)strTemp.GetLength() );
    }

    size_t CCoverageMap::FindCategory( const char *strCategoryName )
    {
        size_t nCount = 0;

        return IASLib::NOT_FOUND;
    }
} // namespace IASLib

#endif // IASLIB_STATS__

