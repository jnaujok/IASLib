//
// Created by jnaujok on 2/14/21.
//

#ifndef IASLIB_JSONDOCUMENT_H
#define IASLIB_JSONDOCUMENT_H

namespace IASLib {
    class CJsonDocument : public CJsonNode {
    private:

    public:
        DEFINE_OBJECT( CJsonDocument )


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

#endif //IASLIB_JSONDOCUMENT_H
