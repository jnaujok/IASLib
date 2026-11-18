/**
 * @file XMLIndex.h
 * @brief CXMLIndex, the top-level content of an XML document.
 *
 * The original header comment follows.
 */
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
    /// What an XML document holds at its top level, in order: the XML
    /// declaration and other meta tags, comments, the DOCTYPE, the root
    /// element (or, for documents read leniently, several), and the text
    /// between them.
    ///
    /// The index owns its chunks; the Add methods take ownership. Element
    /// positions count elements only (including comments and meta tags);
    /// chunk positions count everything.
    class CXMLIndex : public CObject
    {
        protected:
            CArray          m_aElements;        ///< The chunks, in order (owned).
            CString         m_strXMLFileName;   ///< The file it was read from ("" if none).

        public:
                /// An empty index.
                            CXMLIndex( void );
                                /// Reads a file (pFile if given, or opens
                                /// strFileName).
                                ///
                                /// A file that cannot be opened gives an empty index.
                                /// @param strFileName  The file's path (also kept as GetFileName()).
                                /// @param pFile  An open file to read instead; it is closed afterwards.
                                /// @throws CXMLException if it is not well formed.
                            CXMLIndex( const char *strFileName, CFile *pFile = NULL );
                /// Deletes the chunks.
            virtual        ~CXMLIndex( void );

                            DEFINE_OBJECT( CXMLIndex )

                /// An element (including comments and meta tags) by position.
                /// @param nCount  From 0, counting elements only.
                /// @return The element, which the index still owns, or NULL.
            CXMLElement    *GetElement( size_t nCount ) const;
                /// The nth element of a name.
                /// @param strElementType  The name (trimmed).
                /// @param nCount  Which of them, from 0.
                /// @return The element, which the index still owns, or NULL.
            CXMLElement    *GetElement( const char *strElementType, size_t nCount = 0 ) const;
                /// How many elements there are (including comments and meta tags).
            size_t          GetElementCount( void ) const;
                /// How many elements have this name.
                /// @param strElementType  The name (trimmed).
            size_t          GetElementCount( const char *strElementType ) const;
                /// The first plain element: the document's root (NULL if none).
            CXMLElement    *GetRoot( void ) const;

                /// Adds an element, taking ownership. NULL is ignored.
                /// @param pNew  The element.
                /// @param nCount  The chunk position to insert it at; NOT_FOUND
                ///                (or past the end) adds it at the end.
            void            AddElement( CXMLElement *pNew, size_t nCount = IASLib::NOT_FOUND );
                /// Removes (and deletes) the nth chunk.
                /// @param nCount  The chunk position (counting text too).
                /// @return false if there is no such chunk.
            bool            DeleteElement( size_t nCount );

                /// Adds text at the end, taking ownership. NULL is ignored.
            void            AddData( CXMLData *pData );
                /// The nth piece of text.
                /// @param nCount  From 0, counting text only.
                /// @return The text, which the index still owns, or NULL.
            CXMLData       *GetData( size_t nCount ) const;
                /// How many pieces of text there are.
            size_t          GetDataCount( void ) const;

                /// A chunk by position, elements and text in order.
                /// @param nCount  From 0 to GetChunkCount() - 1.
            CXMLChunk      *GetChunk( size_t nCount ) const;
                /// How many chunks there are.
            size_t          GetChunkCount( void ) const;
                /// Adds a chunk at the end, taking ownership. NULL is ignored.
            void            AddChunk( CXMLChunk *pChunk );

                /// Deletes everything.
            void            Clear( void );
                /// Takes another index's chunks, adding them at the end (and
                /// leaving it empty).
                /// @param oOther  The index to take from.
            void            TakeFrom( CXMLIndex &oOther );

                /// The file it was read from ("" if none).
            const CString  &GetFileName( void ) const { return m_strXMLFileName; }

        private:
                            CXMLIndex( const CXMLIndex & );        ///< Not copyable.
            CXMLIndex      &operator =( const CXMLIndex & );      ///< Not copyable.
    };
} // namespace IASLib

#endif // IASLIB_XMLINDEX_H__
