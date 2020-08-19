/**
 * @file XML Document Header file
 *
 * @class XML Document class
 * 
 *		This class implements a high-speed parser for XML to provide a 
 * fast and consistent means of loading an XML document from a stream.
 * Because streams can represent sockets, files, or other items, this will
 * improve the overall capability of the IAS Library versus the XML Index 
 * class which can only load files. 
 *		However, to retain compatibility, this class will actually load an
 * XML Index/Element/Property hierarchy as it runs. This should make any
 * conversion from the old system much simpler.
 *
 *	@author Jeffrey R. Naujok
 *  @date 02/10/2006
 *
 * Copyright (C) 2006, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLDOCUMENT_H__
#define IASLIB_XMLDOCUMENT_H__

#include "../BaseTypes/Object.h"
#include "../Streams/Stream.h"
#include "../Collections/Stack.h"
#include "XMLIndex.h"
#include "XMLElement.h"

namespace IASLib
{

    class CXMLDocument : public CObject
    {
        protected:
            CXMLIndex   m_xiIndex;
            CStream    *m_pInput;
            CStack      m_stackTags;
        public:
                        CXMLDocument( void );
                        CXMLDocument( CStream *pInput );
            virtual    ~CXMLDocument( void );

                        DECLARE_OBJECT(CXMLDocument,CObject);

            bool        Read( CStream *pInput );
            size_t      Write( CStream *pOutput, int indent = 0 );

            CXMLIndex  *GetIndex( void ) { return &m_xiIndex; }

            CString     toString( int indent = 0 );

        private:
            enum TagReturns
            {
                OPEN_TAG,
                SELF_CLOSE,
                END_TAG,
                ERROR_TAG
            };

            TagReturns  StartTag( CStream &oStream, CXMLElement *pTag );
            TagReturns  StartProperty( CStream &oStream, CXMLElement *pTag, const CString& strName );
    };

}; // end of Namespace IASLib

#endif // IASLIB_XMLDOCUMENT_H__
