/**
 * @file XMLDocument.h
 * @brief CXMLDocument, an XML document read from a stream or text.
 *
 * The original header comment follows.
 */
/*
 * XML Document Class
 *
 *      An XML document, read from a stream (or a string or file, through
 * CXMLParser) and written back out. Its content is in the index
 * (GetIndex()); GetRoot() is the root element.
 *
 *      Reading checks the document is well formed (XML 1.0): tags match
 * and nest, attributes are quoted and not repeated, entities and
 * character references are known, and so on; a document that is not
 * throws a CXMLException giving the line and column. By default a
 * document may have several root elements (as many configuration files
 * do); CXMLParser can require exactly one.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLDOCUMENT_H__
#define IASLIB_XMLDOCUMENT_H__

#include "../BaseTypes/Object.h"
#include "../Streams/Stream.h"
#include "XMLIndex.h"
#include "XMLElement.h"

namespace IASLib
{
    /// An XML document, read from a stream (or a string or file, through
    /// CXMLParser) and written back out.
    ///
    /// Its content is in the index (GetIndex()); GetRoot() is the root
    /// element. The document owns everything in it.
    ///
    /// Reading checks the document is well formed (XML 1.0): tags match
    /// and nest, attributes are quoted and not repeated, entities and
    /// character references are known, and so on; a document that is not
    /// throws a CXMLException giving the line and column. By default a
    /// document may have several root elements (as many configuration
    /// files do); CXMLParser can require exactly one.
    ///
    /// @code
    /// CXMLDocument oDocument;
    /// oDocument.Parse( "<config><port>8080</port></config>" );
    /// CString strPort = oDocument.GetRoot()->GetChildText( "port" );   // "8080"
    /// @endcode
    class CXMLDocument : public CObject
    {
        protected:
            CXMLIndex   m_xiIndex;      ///< The document's top-level content.

        public:
                /// An empty document.
                        CXMLDocument( void );
                            /// Reads the stream.
                            /// @param pInput  The stream to read to its end.
                            /// @throws CXMLException if the document is not well
                            ///         formed, or pInput is NULL.
                        CXMLDocument( CStream *pInput );
                /// Deletes the content.
            virtual    ~CXMLDocument( void );

                        DECLARE_OBJECT(CXMLDocument,CObject);

                /// Replaces the content with what the stream holds.
                /// @param pInput  The stream to read to its end.
                /// @return true.
                /// @throws CXMLException (leaving the document as it was) if it
                ///         is not well formed, or pInput is NULL.
            bool        Read( CStream *pInput );
                /// The same from text.
                /// @param strXML  The XML text.
                /// @param nLength  Its length; NOT_FOUND to use strlen().
                /// @return true.
                /// @throws CXMLException (leaving the document as it was) if it
                ///         is not well formed.
            bool        Parse( const char *strXML, size_t nLength = NOT_FOUND );

                /// Writes the document (see CXMLChunk::toString()).
                /// @param pOutput  The stream (NULL only counts the bytes).
                /// @param indent  0 to write it as it is; more to pretty-print.
                /// @return How many bytes.
            size_t      Write( CStream *pOutput, int indent = 0 ) const;
                /// The document as text (see Write()). Pretty-printed text ends
                /// with a line break.
            CString     toString( int indent = 0 ) const;

                /// The document's top-level content.
            CXMLIndex  *GetIndex( void ) { return &m_xiIndex; }
                /// The document's top-level content.
            const CXMLIndex *GetIndex( void ) const { return &m_xiIndex; }
                /// The root element (the first plain element), or NULL if there is none.
            CXMLElement *GetRoot( void ) const { return m_xiIndex.GetRoot(); }

        private:
                        CXMLDocument( const CXMLDocument & );      ///< Not copyable.
            CXMLDocument &operator =( const CXMLDocument & );      ///< Not copyable.
    };
}; // end of Namespace IASLib

#endif // IASLIB_XMLDOCUMENT_H__
