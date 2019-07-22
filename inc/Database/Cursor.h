/*
 *  Cursor Class
 *
 *  This class stores the result of a selection and provides for deletion
 * and updates.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 07/01/1998
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_CURSOR_H__
#define IASLIB_CURSOR_H__

#include "BaseResult.h"
#include "../Collections/StringArray.h"
#include <stdlib.h>

#ifdef IASLIB_DATABASE__

namespace IASLib
{
    class CCursor : public CBaseResult
    {
        protected:
            CStringArray        m_astrNewValues;
            bool                m_bUpdatable;

        public:
                                CCursor( void ) { m_bUpdatable = false; }
            virtual            ~CCursor( void ) { }

                                DECLARE_OBJECT( CCursor, CBaseResult );

            virtual int         DeleteRow( const char *strTable ) = 0;
            virtual int         InsertRow( const char *strTable ) = 0;
            virtual int         UpdateRow( const char *strTable ) = 0;

            virtual int         SetColumn( size_t nColumn, const char *strValue )
                                {
                                    if ( ( m_bValid != 0 ) && ( nColumn < m_nColumns ) )
                                    {
                                        m_astrNewValues[ nColumn ] = strValue;
                                        return 1;
                                    }
                                    return 0;
                                }

            virtual int         SetColumn( size_t nColumn, size_t nValue )
                                {
                                    if ( ( m_bValid != 0 ) && ( nColumn < m_nColumns ) )
                                    {
                                        m_astrNewValues[ nColumn ].Format( "%d", nValue );
                                        return 1;
                                    }
                                    return 0;
                                }

            virtual size_t      SetColumn( const char *pszColumn, const char *strValue )
                                {
                                    CString strColumn = pszColumn;
                                    for ( size_t nCount = 0; nCount < m_nColumns; nCount ++ )
                                    {
                                        if ( m_astrHeaders[ nCount ] == strColumn )
                                        {
                                            m_astrNewValues[ nCount ] = strValue;
                                            return 1;
                                        }
                                    }
                                    return 0;
                                }

            virtual size_t      SetColumn( const char *pszColumn, int nValue )
                                {
                                    CString strColumn = pszColumn;
                                    for ( size_t nCount = 0; nCount < m_nColumns; nCount ++ )
                                    {
                                        if ( m_astrHeaders[ nCount ] == strColumn )
                                        {
                                            m_astrNewValues[ nCount ].Format( "%d", nValue );
                                            return 1;
                                        }
                                    }
                                    return 0;
                                }

    };
} // namespace IASLib
#endif // IASLIB_DATABASE__
#endif // IASLIB_CURSOR_H__

