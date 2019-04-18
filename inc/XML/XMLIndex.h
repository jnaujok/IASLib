/*
 * XML Index Class
 *
 *      This class is used to convert an XML file into an XML Index. 
 * In general, this class is simply a container for XML Elements within
 * an XML file. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLINDEX_H__
#define IASLIB_XMLINDEX_H__

#include "../BaseTypes/String_.h"
#include "../Collections/Array.h"
#include "../Files/File.h"
#include "XMLElement.h"
#include "XMLData.h"

namespace IASLib
{
    class CXMLIndex : public CObject
    {
        protected:
            CArray          m_aElements;
            CString         m_strXMLFileName;
        public:
                            CXMLIndex( void );
                            CXMLIndex( const char *strFileName, CFile *pFile = NULL );
            virtual        ~CXMLIndex( void );

                            DEFINE_OBJECT( CXMLIndex )

            CXMLElement    *GetElement( size_t nCount ) const;
            CXMLElement    *GetElement( const char *strElementType, size_t nCount = 0 ) const;
            size_t          GetElementCount( void ) const;
            size_t          GetElementCount( const char *strElementType ) const;

			void            AddElement( CXMLElement *pNew, size_t nCount = IASLib::NOT_FOUND );
            bool            DeleteElement( size_t nCount );

            void            AddData( CXMLData *pData );

            CXMLData       *GetData( size_t nCount ) const;
            size_t          GetDataCount( void ) const;

            CXMLChunk      *GetChunk( size_t nCount ) const;
            size_t          GetChunkCount( void ) const;
    };
} // namespace IASLib

#endif // IASLIB_XMLINDEX_H__
