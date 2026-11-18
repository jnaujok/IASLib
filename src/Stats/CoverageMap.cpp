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
#include "../BaseTypes/String_.h"

namespace IASLib
{
    IMPLEMENT_OBJECT( CCoverageMap, CObject );

    CCoverageMap::CCoverageMap( void )
    {
        m_nMinimum = 0;
        m_nMaximum = 0;
        m_nUsed = 0;
        m_nBlockSize = 4096;
    }

    CCoverageMap::~CCoverageMap( void )
    {
    }

    bool CCoverageMap::Allocate( size_t nBase, size_t nSize )
    {
        if ( nSize == 0 )
            return false;
            // (base + size could wrap.)
        size_t nEnd = ( nSize > (size_t)-1 - nBase ) ? (size_t)-1 : nBase + nSize;
        if ( nEnd == nBase )
            return false;

        bool bFresh = ( UsedIn( nBase, nEnd ) == 0 );

            // Merge with every run that overlaps or touches [nBase, nEnd).
        std::map<size_t, size_t>::iterator it = m_mapRuns.upper_bound( nBase );
        if ( it != m_mapRuns.begin() )
        {
            std::map<size_t, size_t>::iterator prev = it;
            --prev;
            if ( prev->second >= nBase )
                it = prev;
        }
        while ( it != m_mapRuns.end() && it->first <= nEnd )
        {
            if ( it->first < nBase )
                nBase = it->first;
            if ( it->second > nEnd )
                nEnd = it->second;
            m_nUsed -= it->second - it->first;
            m_mapRuns.erase( it++ );
        }
        m_mapRuns[ nBase ] = nEnd;
        m_nUsed += nEnd - nBase;

        if ( m_nMaximum == m_nMinimum )
        {
            m_nMinimum = nBase;
            m_nMaximum = nEnd;
        }
        else
        {
            if ( nBase < m_nMinimum )
                m_nMinimum = nBase;
            if ( nEnd > m_nMaximum )
                m_nMaximum = nEnd;
        }
        return bFresh;
    }

    bool CCoverageMap::Release( size_t nBase, size_t nSize )
    {
        size_t nEnd;
        if ( nSize == IASLib::NOT_FOUND )
        {
                // To the end of the run nBase is in.
            std::map<size_t, size_t>::iterator it = m_mapRuns.upper_bound( nBase );
            if ( it == m_mapRuns.begin() )
                return false;
            --it;
            if ( it->second <= nBase )
                return false;
            nEnd = it->second;
        }
        else
        {
            if ( nSize == 0 )
                return false;
            nEnd = ( nSize > (size_t)-1 - nBase ) ? (size_t)-1 : nBase + nSize;
        }

        bool bReleased = false;
        std::map<size_t, size_t>::iterator it = m_mapRuns.upper_bound( nBase );
        if ( it != m_mapRuns.begin() )
        {
            std::map<size_t, size_t>::iterator prev = it;
            --prev;
            if ( prev->second > nBase )
                it = prev;
        }
        while ( it != m_mapRuns.end() && it->first < nEnd )
        {
            size_t nRunStart = it->first;
            size_t nRunEnd = it->second;
            m_mapRuns.erase( it++ );
            m_nUsed -= nRunEnd - nRunStart;
            bReleased = true;

                // Keep what lies outside [nBase, nEnd).
            if ( nRunStart < nBase )
            {
                m_mapRuns[ nRunStart ] = nBase;
                m_nUsed += nBase - nRunStart;
            }
            if ( nRunEnd > nEnd )
            {
                m_mapRuns[ nEnd ] = nRunEnd;
                m_nUsed += nRunEnd - nEnd;
                break;
            }
        }
        return bReleased;
    }

    size_t CCoverageMap::UsedIn( size_t nStart, size_t nEnd ) const
    {
        size_t nUsed = 0;
        std::map<size_t, size_t>::const_iterator it = m_mapRuns.upper_bound( nStart );
        if ( it != m_mapRuns.begin() )
        {
            std::map<size_t, size_t>::const_iterator prev = it;
            --prev;
            if ( prev->second > nStart )
                it = prev;
        }
        for ( ; it != m_mapRuns.end() && it->first < nEnd; ++it )
        {
            size_t nFrom = ( it->first > nStart ) ? it->first : nStart;
            size_t nTo = ( it->second < nEnd ) ? it->second : nEnd;
            if ( nTo > nFrom )
                nUsed += nTo - nFrom;
        }
        return nUsed;
    }

    bool CCoverageMap::IsUsed( size_t nBase, size_t nSize ) const
    {
        if ( nSize == 0 )
            return true;
        size_t nEnd = ( nSize > (size_t)-1 - nBase ) ? (size_t)-1 : nBase + nSize;
        return UsedIn( nBase, nEnd ) == nEnd - nBase;
    }

    double CCoverageMap::GetFragmentation( void ) const
    {
        size_t nFree = GetFree();
        if ( nFree == 0 )
            return 0.0;

        size_t nLargest = 0;
        size_t nLast = m_nMinimum;
        for ( std::map<size_t, size_t>::const_iterator it = m_mapRuns.begin(); it != m_mapRuns.end(); ++it )
        {
            if ( it->first > nLast && it->first - nLast > nLargest )
                nLargest = it->first - nLast;
            nLast = it->second;
        }
        if ( m_nMaximum > nLast && m_nMaximum - nLast > nLargest )
            nLargest = m_nMaximum - nLast;
        return 1.0 - (double)nLargest / (double)nFree;
    }

    void CCoverageMap::Dump( CStream &oStream, OutputFormatSpecifier nFormat, bool bFillInGaps )
    {
        CString strLine;

        if ( nFormat == CCoverageMap::XML )
            strLine = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<table>\n"
                      "  <tr>\n    <td>Start</td>\n    <td>Used</td>\n    <td>Free</td>\n  </tr>\n";
        else
            strLine = "Start, Used, Free\n";
        oStream.PutBuffer( (const char *)strLine, (int)strLine.GetLength() );

        if ( ! m_mapRuns.empty() )
        {
            size_t nFirst = ( m_mapRuns.begin()->first / m_nBlockSize ) * m_nBlockSize;
            size_t nLast = m_mapRuns.rbegin()->second;

            for ( size_t nBlock = nFirst; nBlock < nLast; )
            {
                size_t nBlockEnd = ( m_nBlockSize > (size_t)-1 - nBlock ) ? (size_t)-1 : nBlock + m_nBlockSize;
                size_t nUsed = UsedIn( nBlock, nBlockEnd );

                if ( nUsed || bFillInGaps )
                {
                    if ( nFormat == CCoverageMap::XML )
                        strLine.Format( "  <tr>\n    <td>%llu</td>\n    <td>%llu</td>\n    <td>%llu</td>\n  </tr>\n",
                                        (unsigned long long)nBlock, (unsigned long long)nUsed,
                                        (unsigned long long)( nBlockEnd - nBlock - nUsed ) );
                    else
                        strLine.Format( "%llu, %llu, %llu\n", (unsigned long long)nBlock, (unsigned long long)nUsed,
                                        (unsigned long long)( nBlockEnd - nBlock - nUsed ) );
                    oStream.PutBuffer( (const char *)strLine, (int)strLine.GetLength() );
                }
                else
                {
                        // Skip straight to the next used run.
                    std::map<size_t, size_t>::const_iterator it = m_mapRuns.upper_bound( nBlock );
                    if ( it == m_mapRuns.end() )
                        break;
                    size_t nNext = ( it->first / m_nBlockSize ) * m_nBlockSize;
                    if ( nNext > nBlock )
                    {
                        nBlock = nNext;
                        continue;
                    }
                }
                if ( nBlockEnd == (size_t)-1 )
                    break;
                nBlock = nBlockEnd;
            }
        }

        if ( nFormat == CCoverageMap::XML )
            oStream.PutBuffer( "</table>\n", 9 );
    }
} // namespace IASLib
