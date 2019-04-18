/*
 *  COracleStoredProc
 *
 *  This class defines an Oracle 7.3 stored procedure call. It should
 * be noted that Oracle does not allow for result sets (select statements)
 * to be returned from a stored procedure. Thus, this function only
 * supports parameters, return values, and output parameters.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/20/1999
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifdef IASLIB_DATABASE__
#ifdef IASLIB_DB_ORACLE__

#include "Ora_Cursor.h"
#include "Ora_ResultSet.h"
#include "Ora_StoredProc.h"
#include "OutParam.h"
#include "StringTokenizer.h"

extern "C"
{
#include <oci.h>
}

#include <stdio.h>

#define MIN(x,y) ( (x<y)?x:y )

namespace IASLib
{
    static const char  *astrVariableTypes[] = { "numeric(*)", "int", "float", "char(*)", "varchar(*)", "datetime", "**" };
    static int          anVariableTokens[] = { SQLT_NUM, SQLT_INT, SQLT_FLT, SQLT_CHR, SQLT_VCS, SQLT_DATE, -1 };

    IMPLEMENT_OBJECT( COracleStoredProc, CStoredProc );

    COracleStoredProc::COracleStoredProc( CConnection *pCon, Cda_Def *pCDA, const char *strOutParams )
    {
        COracleResultSet   *pNewResultSet;
        CStringArray        aParamNames;
        void              **aParamSpace = NULL;
        int                *aParamType = NULL;
        int                *aParamLength = NULL;
        int                 nParams = 0;
        int                 nCount = 0;

        m_pCDA = pCDA;
        m_pConnection = pCon;
        m_nReturnValue = 0;
        m_nResultSets = 0;
        m_aResultSets = NULL;
        m_nOutParams = 0;
        m_aOutParams = NULL;
        m_nCurrentResultSet = 0;
        m_bValid = 0;

        if ( strOutParams )
        {
            CStringTokenizer    stTokens( strOutParams, "|", false );
            CString             strToken;

            nParams = stTokens.CountTokens();

            aParamSpace = new void *[ nParams ];
            aParamType = new int[ nParams ];
            aParamLength = new int[ nParams ];

            nCount = 0;

            while ( stTokens.HasMoreTokens() )
            {
                strToken = stTokens.NextToken();  // Tokens should be in  "name type" format

                CStringTokenizer stBreakType( strToken );

                if ( stBreakType.CountTokens() < 2 )
                {
                }
                else
                {
                    aParamNames.Push( stBreakType.NextToken() );
                    CString strType = stBreakType.NextToken();
                    CString strValue = stBreakType.NextToken();
                    aParamType[ nCount ] = CString::ConvertToToken( strType, astrVariableTypes, anVariableTokens, -1, false, true );

                    if ( strType.IndexOf( '(' ) != -1 )
                    {
                        int nStart = strType.IndexOf( '(' ) + 1;
                        int nLength = strType.IndexOf( ')' ) - nStart;
                        aParamLength[ nCount ] = atoi( strType.Substring( nStart, nLength ) );
                    }
                    else
                    {
                        aParamLength[ nCount ] = -1;
                    }

                    aParamSpace[ nCount ] = NULL;

                    switch ( aParamType[ nCount ] )
                    {
                        case SQLT_NUM:
                            break;

                        case SQLT_INT:
                            aParamSpace[ nCount ] = new int;
                            if ( strValue.GetLength() )
                            {
                                *((int *)aParamSpace[nCount]) = atoi( strValue );
                            }
                            aParamLength[ nCount ] = sizeof(int);
                            break;

                        case SQLT_FLT:
                            aParamSpace[ nCount ] = new float;
                            if ( strValue.GetLength() )
                            {
                                *((float *)aParamSpace[nCount]) = (float)atof( strValue );
                            }
                            aParamLength[ nCount ] = sizeof(float);
                            break;

                        case SQLT_CHR:
                            aParamSpace[ nCount ] = new char[ aParamLength[nCount] + 1 ];
                            if ( strValue.GetLength() )
                            {
                                memcpy( (char *)aParamSpace[nCount], (const char *)strValue, MIN( (int)strValue.GetLength(), aParamLength[nCount] ) );
                            }
                            break;

                        case SQLT_VCS:
                            aParamSpace[ nCount ] = new char[ aParamLength[nCount] + 1 ];
                            if ( strValue.GetLength() )
                            {
                                memcpy( (char *)aParamSpace[nCount], (const char *)strValue, MIN( (int)strValue.GetLength(), aParamLength[nCount] ) );
                            }
                            break;

                        case SQLT_DATE:
                            aParamSpace[ nCount ] = new char[ 256 ];
                            aParamLength[ nCount ] = 256;
                            if ( strValue.GetLength() )
                            {
                                memcpy( (char *)aParamSpace[nCount], (const char *)strValue, MIN( (int)strValue.GetLength(), aParamLength[nCount] ) );
                            }
                            break;

                        default:
                            break;
                    
                    }
                }

                nCount++;
            }
        }

        nCount = 0;

        while ( nCount < nParams )
        {
            if ( obndrv( m_pCDA, (text *)(const char *)aParamNames[ nCount ], -1, (ub1 *)aParamSpace[ nCount ], aParamLength[ nCount ], aParamType[ nCount ], -1, 0 ,0 ,-1, -1 ) )
            {
            }
            nCount++;
        }

        if ( oexec( m_pCDA ) )
        {
            return;
        }


        pNewResultSet = new COracleResultSet( m_pCDA );
        if ( pNewResultSet->IsValid() )
        {
            if ( ( m_nResultSets % 4 ) == 0 )
            {
                CResultSet **aTemp = new CResultSet *[ m_nResultSets + 4 ];
                for ( int nTemp = 0 ; nTemp < m_nResultSets ; nTemp++ )
                {
                    aTemp[ nTemp ] = m_aResultSets[ nTemp ];
                }
                delete m_aResultSets;
                m_aResultSets = aTemp;
            }
            m_aResultSets[ m_nResultSets ] = pNewResultSet;
            m_nResultSets++;
        }

        CString strTemp;

        if ( nParams )
        {
            m_aOutParams = new COutParam *[nParams];
            m_nOutParams = nParams;

            for ( nCount = 0; nCount < nParams; nCount++ )
            {
                char *pTemp = NULL;
                switch ( aParamType[ nCount ] )
                {
                    case SQLT_NUM:
                        strTemp = "Unknown Numeric";
                        break;

                    case SQLT_INT:
                        strTemp.Format( "%d", *(int *)aParamSpace[nCount] );
                        break;

                    case SQLT_FLT:
                        strTemp.Format( "%f", *(float *)aParamSpace[nCount] );
                        break;

                    case SQLT_CHR:
                    case SQLT_VCS:
                    case SQLT_DATE:
                        pTemp = (char *)aParamSpace[nCount];
                        pTemp[aParamLength[nCount]] = 0;
                        strTemp = (const char *)pTemp;
                        break;

                    default:
                        strTemp = "Unknown Type";
                        break;
                }

                COutParam *pParam = new COutParam( aParamNames[ nCount ], strTemp );
                m_aOutParams[ nCount ] = pParam;

                delete aParamSpace[ nCount ];
            }

            delete aParamSpace;
            delete aParamType;
            delete aParamLength;
            aParamNames.DeleteAll();
        }

        m_nReturnValue = m_pCDA->rc;

        m_bValid = 1;
    }

    COracleStoredProc::~COracleStoredProc( void )
    {
        Close();
    }

    bool COracleStoredProc::Close( void )
    {
        if ( m_nOutParams )
        {
            for ( size_t nCount = 0; nCount < m_nOutParams ; nCount++ )
            {
                delete m_aOutParams[ nCount ];
            }
            delete m_aOutParams;
            m_aOutParams = NULL;
            m_nOutParams = 0;
        }
        if ( m_nResultSets )
        {
            for ( int nCount = 0; nCount < m_nResultSets ; nCount++ )
            {
                delete m_aResultSets[ nCount ];
            }
            delete m_aResultSets;
            m_aResultSets = NULL;
            m_nResultSets = 0;
        }
        return 0;
    }

    size_t COracleStoredProc::OutParamCount( void )
    {
        return m_nOutParams;
    }

    const char *COracleStoredProc::OutParameters( size_t nParam )
    {
        if (( nParam >= 0 ) && ( nParam < m_nOutParams ) )
        {
            return m_aOutParams[ nParam ]->GetValue();
        }
        return NULL;
    }

    CResultSet *COracleStoredProc::ResultSet( void )
    {
        if ( m_nCurrentResultSet < m_nResultSets )
        {
            CResultSet *pResultSet = m_aResultSets[ m_nCurrentResultSet ];
            m_nCurrentResultSet++;
            return pResultSet;
        }

        return NULL;
    }

    int COracleStoredProc::ReturnValue()
    {
        return m_nReturnValue;
    }

    void COracleStoredProc::GetOutputParams( void )
    {
    }

    void COracleStoredProc::GetStatusParam( void )
    {
    }
} // namespace IASLib

#endif // IASLIB_DB_ORACLE__
#endif // IASLIB_DATABASE__
