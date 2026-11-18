/*
 * XML Index Class
 *
 *      What an XML document holds at its top level, in order: the XML
 * declaration and other meta tags, comments, the DOCTYPE, the root
 * element (or, for documents read leniently, several), and the text
 * between them. The index owns them.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
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
            CArray          m_aElements;        // the chunks, in order
            CString         m_strXMLFileName;

        public:
                            CXMLIndex( void );
                                // Reads a file (pFile if given, or opens
                                // strFileName); throws a CXMLException if it is
                                // not well formed. A file that cannot be opened
                                // gives an empty index.
                            CXMLIndex( const char *strFileName, CFile *pFile = NULL );
            virtual        ~CXMLIndex( void );

                            DEFINE_OBJECT( CXMLIndex )

                // Elements (including comments and meta tags), by position or
                // as the nth of a name.
            CXMLElement    *GetElement( size_t nCount ) const;
            CXMLElement    *GetElement( const char *strElementType, size_t nCount = 0 ) const;
            size_t          GetElementCount( void ) const;
            size_t          GetElementCount( const char *strElementType ) const;
                // The first plain element: the document's root.
            CXMLElement    *GetRoot( void ) const;

            void            AddElement( CXMLElement *pNew, size_t nCount = IASLib::NOT_FOUND );
                // Removes (and deletes) the nth chunk.
            bool            DeleteElement( size_t nCount );

            void            AddData( CXMLData *pData );
            CXMLData       *GetData( size_t nCount ) const;
            size_t          GetDataCount( void ) const;

            CXMLChunk      *GetChunk( size_t nCount ) const;
            size_t          GetChunkCount( void ) const;
            void            AddChunk( CXMLChunk *pChunk );

                // Deletes everything.
            void            Clear( void );
                // Takes another index's chunks (leaving it empty).
            void            TakeFrom( CXMLIndex &oOther );

            const CString  &GetFileName( void ) const { return m_strXMLFileName; }

        private:
                            CXMLIndex( const CXMLIndex & );
            CXMLIndex      &operator =( const CXMLIndex & );
    };
} // namespace IASLib

#endif // IASLIB_XMLINDEX_H__
