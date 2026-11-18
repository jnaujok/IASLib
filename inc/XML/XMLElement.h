/*
 * XML Element Class
 *
 *      An XML element: a name, attributes (properties) and content (sub
 * elements and text, in document order). The same class stands for the
 * other kinds of markup a document can hold, told apart by IsComment(),
 * IsMetaTag() (a processing instruction, such as <?xml version="1.0"?>)
 * and IsDeclaration() (such as <!DOCTYPE html>):
 *
 *      comment         the comment's text is GetData()
 *      meta tag        the name is the target ("xml"); pseudo-attributes
 *                      (version="1.0") are its properties, and
 *                      GetRawContent() is everything after the target
 *      declaration     the name is the keyword ("DOCTYPE"), and
 *                      GetRawContent() is the rest
 *
 *      An element owns its properties and sub elements.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XML_ELEMENT_H__
#define IASLIB_XML_ELEMENT_H__

#include "../BaseTypes/String_.h"
#include "../Collections/Array.h"
#include "XMLProperty.h"
#include "XMLChunk.h"
#include "XMLData.h"

namespace IASLib
{
    class CXMLElement : public CXMLChunk
    {
        protected:
            CString             m_strElementName;
            CArray              m_aProperties;
            CArray              m_aSubElements;     // CXMLElements and CXMLDatas
            CString             m_strRawContent;    // meta tags and declarations
            bool                m_bCommentTag;
            bool                m_bMetaTag;
            bool                m_bDeclaration;
            bool                m_bHasClosingTag;   // <name>...</name>
            bool                m_bSelfClosingTag;  // <name/>

        public:
                                CXMLElement( void );
                                CXMLElement( const char *strTagName );
                                    // An element holding one piece of text.
                                CXMLElement( const CString &strTagName, const CString &strData );
            virtual            ~CXMLElement( void );

                                DEFINE_OBJECT( CXMLElement )

                // The name.
            const char         *GetName( void ) const { return m_strElementName; }
            void                SetName( const CString& strTagName );

                // Properties (attributes). GetProperty( name ) gives NULL if
                // there is none; SetProperty() adds or replaces.
            CXMLProperty       *GetProperty( size_t nIndex ) const;
            size_t              GetPropertyCount( void ) const { return m_aProperties.Length(); }
            const char         *GetProperty( const char *strPropertyName ) const;
            bool                HasProperty( const char *strPropertyName ) const { return GetProperty( strPropertyName ) != NULL; }
            void                SetProperty( const char *strPropertyName, const char *strValue );
            void                AddProperty( CXMLProperty *pNewProperty );
            bool                DeleteProperty( const char *strPropertyName );

                // Sub elements (not counting text), by position, or the nth
                // of a name.
            size_t              GetSubElementCount( void ) const;
            size_t              GetSubElementCount( const char *strElementType ) const;
            const CXMLElement  *GetSubElement( size_t nElement ) const;
            const CXMLElement  *GetSubElement( const char *strElementType, size_t nIndex = 0 ) const;
            CXMLElement        *GetSubElement( size_t nElement );
            CXMLElement        *GetSubElement( const char *strElementType, size_t nIndex = 0 );
            void                AddSubElement( CXMLElement *pNewElement );
                // Removes (and deletes) the nth sub element.
            bool                DeleteSubElement( size_t nElement );

                // Text: the nth piece (NULL if there is none), all of it
                // joined (GetText()), or the trimmed text of the first sub
                // element with a name (GetChildText(); "" if there is none).
            const char         *GetData( size_t nDataElement = 0 ) const;
            size_t              GetDataCount( void ) const;
            CString             GetText( void ) const;
            CString             GetChildText( const char *strElementType ) const;
                // Replaces the nth piece of text (adding one if there is none).
            void                SetData( const CString& strData, size_t nDataElement = 0 );
            void                AddData( const CString& strData, bool bCData = false );

                // All the content, elements and text, in order.
            size_t              GetChunkCount( void ) const { return m_aSubElements.Length(); }
            CXMLChunk          *GetChunk( size_t nIndex ) const { return (CXMLChunk *)m_aSubElements.Get( nIndex ); }
            void                AddChunk( CXMLChunk *pChunk ) { if ( pChunk != NULL ) m_aSubElements.Append( pChunk ); }

                // The kind of markup.
            void                SetComment( void ) { m_bCommentTag = true; m_bHasClosingTag = false; m_bSelfClosingTag = false; }
            void                SetMetaTag( void ) { m_bMetaTag = true; m_bHasClosingTag = false; m_bSelfClosingTag = false; }
            void                SetDeclaration( void ) { m_bDeclaration = true; m_bHasClosingTag = false; m_bSelfClosingTag = false; }
            bool                IsComment( void ) const { return m_bCommentTag; }
            bool                IsMetaTag( void ) const { return m_bMetaTag; }
            bool                IsDeclaration( void ) const { return m_bDeclaration; }
                // An ordinary element (not a comment, meta tag or declaration).
            bool                IsPlainElement( void ) const { return ! m_bCommentTag && ! m_bMetaTag && ! m_bDeclaration; }
            const CString      &GetRawContent( void ) const { return m_strRawContent; }
            void                SetRawContent( const CString &strContent ) { m_strRawContent = strContent; }

                // Whether the element was written <name/> (it is, when
                // written, if it has no content).
            void                SetSelfClose( bool selfClose ) { m_bSelfClosingTag = selfClose; m_bHasClosingTag = !selfClose; }
            void                SetHasClosingTag( bool hasClose ) { m_bHasClosingTag = hasClose; m_bSelfClosingTag = !hasClose; }
            bool                IsSelfClosing( void ) const { return m_bSelfClosingTag; }

            virtual bool        IsElement( void ) const { return true; }
            virtual bool        IsData( void ) const { return false; }

            virtual CString     toString( int offset = 0, int indent = 0 ) const;
            virtual CXMLChunk  *Clone( void ) const;

        private:
            void                Init( void );
            void                Write( CString &strOut, int offset, int indent, int nDepth ) const;
            CString             StartTag( void ) const;

                                CXMLElement( const CXMLElement & );
            CXMLElement        &operator =( const CXMLElement & );
    };
} // namespace IASLib

#endif // IASLIB_XML_ELEMENT_H__
