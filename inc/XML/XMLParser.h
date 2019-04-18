/**
 * @file XML Parser Header file
 *
 * @class XML Parser class
 * 
 *		This class implements a high-speed parser for XML to provide a 
 * fast and consistent means of loading an XML document from a multitude 
 * of sources. Internally, the parser will convert the source to a stream
 * for use. Streams can represent sockets, files, or other items, this will
 * improve the overall capability of the IAS Library versus the XML Index 
 * class which can only load files. 
 *		However, to retain compatibility, this class will actually load an
 * XML Index/Element/Property hierarchy as it runs. This should make any
 * conversion from the old system much simpler.
 *      As a builder class, most of the functions of this class are static
 * and don't require an instance to be created.
 *
 *	@author Jeffrey R. Naujok
 *  @date 04/16/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_XMLPARSER_H__
#define IASLIB_XMLPARSER_H__

#include "XMLDocument.h"
#include "../Sockets/Socket.h"

namespace IASLib
{

    class CXMLParser : public CObject
    {
        public:
                        CXMLParser( void );
            virtual    ~CXMLParser( void );

                        DECLARE_OBJECT(CXMLParser,CObject);

            static CXMLDocument *parseFile( CFile *file );
            static CXMLDocument *parseFile( CString fileName );
            static CXMLDocument *parseString( CString xmlData );
#ifdef IASLIB_NETWORKING__
            static CXMLDocument *parseSocket( CSocket *socket );
#endif      
            static CXMLDocument *parseStream( CStream *stream );
    };

}; // end of Namespace IASLib

#endif // IASLIB_XMLPARSER_H__
