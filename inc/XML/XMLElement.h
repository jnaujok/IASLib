/*
 * XML Element Class
 *
 *      This class stores an XML Element within an XML index. Note
 * that element relationships can be recursive, so the entire XML file
 * is considered as the first XML Element. Further elements are then
 * added to this initial element. Each of those elements may have other
 * elements and so forth. XML is highly receptive of exactly this format.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 06/15/2000
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XML_ELEMENT_H__
#define IASLIB_XML_ELEMENT_H__

#include "../BaseTypes/String_.h"
#include "../Streams/Stream.h"
#include "../Collections/Array.h"
#include "XMLProperty.h"
#include "XMLChunk.h"

namespace IASLib
{
    class CXMLElement : public CXMLChunk
    {
        protected:
            CString             m_strElementName;
            CArray              m_aProperties;
            CArray              m_aSubElements;
            bool                m_bCommentTag;
            bool                m_bMetaTag;
            bool                m_bHasClosingTag; // used during construction
            bool                m_bSelfClosingTag; // represents a <name/> tag.
        public:
                                CXMLElement( CString &strTag );
                                CXMLElement( void );
                                CXMLElement( CString strTagName, CString strData );
            virtual            ~CXMLElement( void );

                                DEFINE_OBJECT( CXMLElement )

            CXMLProperty       *GetProperty( size_t nIndex ) const;
            size_t              GetPropertyCount( void ) const { return m_aProperties.Length(); } 
            const char         *GetProperty( const char *strPropertyName ) const;
            const char         *GetName( void ) const { return m_strElementName; }
            size_t              GetSubElementCount( void ) const;
            size_t              GetSubElementCount( const char *strElementType ) const;
            const CXMLElement  *GetSubElement( size_t nElement ) const;
            const CXMLElement  *GetSubElement( const char *strElementType, size_t nIndex = 0 ) const;

            const char         *GetData( size_t nDataElement = 0 ) const;
            size_t              GetDataCount( void ) const;

            void                SetComment( void ) { m_bCommentTag = true; }
            void                SetMetaTag( void ) { m_bMetaTag = true; }
            bool                IsComment( void ) { return m_bCommentTag; }
            bool                IsMetaTag( void ) { return m_bMetaTag; }
            void                SetSelfClose( bool selfClose ) { m_bSelfClosingTag = selfClose; m_bHasClosingTag = !selfClose; }
            void                SetHasClosingTag( bool hasClose ) { m_bHasClosingTag = hasClose; m_bSelfClosingTag = !hasClose; }

            void                SetName( const CString& strTagName );
            void                SetData( const CString& strData, size_t nDataElement = 0 );
            void                AddProperty( CXMLProperty *pNewProperty );
            void                AddSubElement( CXMLElement *pNewElement );
            void                AddData( const CString& strData );

            bool                DeleteProperty( const char *strPropertyName );
            bool                DeleteSubElement( size_t nElement );

            virtual bool        IsElement( void ) const { return true; }
            virtual bool        IsData( void ) const { return false; }

            virtual CString     toString( int offset, int indent ) const;
    };
} // namespace IASLib

#endif // IASLIB_XML_ELEMENT_H__
