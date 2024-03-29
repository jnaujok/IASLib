#ifndef IASLIB_JSON_PARSER_H__
#define IASLIB_JSON_PARSER_H__

#ifdef IASLIB_JSON_SUPPORT__

#include "Stream.h"
#include "JsonNode.h"
#include "Object.h"
#include "String_.h"


namespace IASLib {
    class CJsonParser : public CObject {
    private:

    public:
        DEFINE_OBJECT( CJsonParser )

        static CJsonNode *parse( CString jsonData );
        static CJsonNode *parse( CStream *jsonData );

    private:
        static CJsonNode *parseElement( CStream *jsonData );
        static CJsonNode *parseWhitespace( CStream *jsonData );
        static CJsonNode *parseValue( CStream *jsonData );

        bool isTokenWhitespace( CString token );
        bool isTokenSign( CString token );
        bool isTokenExponent( CString token );
        bool isTokenDigits( CString token );
        bool isTokenOneNine( CString token );
        bool isTokenDigit( CString token );
        bool isTokenFraction( CString token );
        bool isTokenInteger( CString token );
        bool isTokenNumber( CString token );
    };
}


#endif // IASLIB_JSON_SUPPORT__
#endif // IASLIB_JSON_PARSER_H__
