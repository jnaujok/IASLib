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
        // A result whose rows can be changed: SetColumn() stages new values
        // for the current row (by number or by name, any case), and
        // UpdateRow(), InsertRow() and DeleteRow() apply them to a table.
        // Moving to another row does not clear what was staged:
        // ClearChanges() does, and the drivers do after applying them.
    class CCursor : public CBaseResult
    {
        protected:
            CStringArray        m_astrNewValues;
            std::vector<bool>   m_abChanged;
            std::map<size_t, CDataBlock> m_mapNewBlobs;     // staged binary values
            bool                m_bUpdatable;

                // (The staged values were never sized, so every SetColumn()
                // went to one scratch slot.)
            void                EnsureNewValues( void )
                                {
                                    while ( m_astrNewValues.Length() < m_nColumns )
                                        m_astrNewValues.Push( CString( "" ) );
                                    if ( m_abChanged.size() < m_nColumns )
                                        m_abChanged.resize( m_nColumns, false );
                                }

        public:
                                CCursor( void ) { m_bUpdatable = false; }
            virtual            ~CCursor( void ) { }

                                DECLARE_OBJECT( CCursor, CBaseResult );

            virtual int         DeleteRow( const char *strTable ) = 0;
            virtual int         InsertRow( const char *strTable ) = 0;
            virtual int         UpdateRow( const char *strTable ) = 0;

            bool                IsUpdatable( void ) const { return m_bUpdatable; }
                // Whether a column has a staged value, and that value.
            bool                IsChanged( size_t nColumn ) const { return nColumn < m_abChanged.size() && m_abChanged[ nColumn ]; }
            CString             GetNewValue( size_t nColumn ) const { return ( nColumn < m_astrNewValues.Length() ) ? m_astrNewValues.Get( nColumn ) : CString( "" ); }
                // Whether a column's staged value is binary, and its bytes.
            bool                IsNewValueBlob( size_t nColumn ) const { return IsChanged( nColumn ) && m_mapNewBlobs.count( nColumn ) != 0; }
            CDataBlock          GetNewBlob( size_t nColumn ) const
                                {
                                    std::map<size_t, CDataBlock>::const_iterator it = m_mapNewBlobs.find( nColumn );
                                    return ( it != m_mapNewBlobs.end() ) ? it->second : CDataBlock();
                                }
            void                ClearChanges( void ) { m_abChanged.assign( m_abChanged.size(), false ); m_mapNewBlobs.clear(); }

                // Stages a binary value (written as a BLOB).
            virtual int         SetColumn( size_t nColumn, const CDataBlock &oValue )
                                {
                                    if ( ( m_bValid != 0 ) && ( nColumn < m_nColumns ) )
                                    {
                                        EnsureNewValues();
                                        m_astrNewValues[ nColumn ] = "";
                                        m_mapNewBlobs[ nColumn ] = oValue;
                                        m_abChanged[ nColumn ] = true;
                                        return 1;
                                    }
                                    return 0;
                                }
            virtual size_t      SetColumn( const char *pszColumn, const CDataBlock &oValue )
                                {
                                    size_t nColumn = FindColumn( pszColumn );
                                    return ( nColumn == NOT_FOUND ) ? 0 : (size_t)SetColumn( nColumn, oValue );
                                }

            virtual int         SetColumn( size_t nColumn, const char *strValue )
                                {
                                    if ( ( m_bValid != 0 ) && ( nColumn < m_nColumns ) )
                                    {
                                        EnsureNewValues();
                                        m_astrNewValues[ nColumn ] = ( strValue ) ? strValue : "";
                                        m_mapNewBlobs.erase( nColumn );
                                        m_abChanged[ nColumn ] = true;
                                        return 1;
                                    }
                                    return 0;
                                }

            virtual int         SetColumn( size_t nColumn, size_t nValue )
                                {
                                    CString strValue;
                                    strValue.Format( "%llu", (unsigned long long)nValue );
                                    return SetColumn( nColumn, (const char *)strValue );
                                }

                // (The by-name forms compared names with case, though the
                // drivers report them in their own case.)
            virtual size_t      SetColumn( const char *pszColumn, const char *strValue )
                                {
                                    size_t nColumn = FindColumn( pszColumn );
                                    return ( nColumn == NOT_FOUND ) ? 0 : (size_t)SetColumn( nColumn, strValue );
                                }

            virtual size_t      SetColumn( const char *pszColumn, int nValue )
                                {
                                    CString strValue;
                                    strValue.Format( "%d", nValue );
                                    return SetColumn( pszColumn, (const char *)strValue );
                                }
    };
} // namespace IASLib

#endif // IASLIB_DATABASE__
#endif // IASLIB_CURSOR_H__

