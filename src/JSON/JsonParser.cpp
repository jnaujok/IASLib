/**
 * JSON Parser Class Implementation
 *
 *  This class tokenizes raw JSON input and converts it into JSON nodes.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 9/13/2020
 *
 * Copyright (C) 2020, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

/**
 * @formatter:off
 *
 * Complete JSON Parse Tokenization description
 *
 * json
 *   element
 *
 * value
 *   object
 *   array
 *   string
 *   number
 *   "true"
 *   "false"
 *   "null"
 *
 * object
 *   '{' ws '}'
 *   '{' members '}'
 *
 * members
 *   member
 *   member ',' members
 *
 * member
 *   ws string ws ':' element
 *
 * array
 *   '[' ws ']'
 *   '[' elements ']'
 *
 * elements
 *   element
 *   element ',' elements
 *
 * element
 *   ws value ws
 *
 * string
 *   '"' characters '"'
 *
 * characters
 *   ""
 *   character characters
 *
 * character
 *   '0020' . '10FFFF' - '"' - '\'
 *   '\' escape
 *
 * escape
 *   '"'
 *   '\'
 *   '/'
 *   'b'
 *   'f'
 *   'n'
 *   'r'
 *   't'
 *   'u' hex hex hex hex
 *
 * hex
 *   digit
 *   'A' . 'F'
 *   'a' . 'f'
 *
 * number
 *   integer fraction exponent
 *
 * integer
 *   digit
 *   onenine digits
 *   '-' digit
 *   '-' onenine digits
 *
 * digits
 *   digit
 *   digit digits
 *
 * digit
 *   '0'
 *   onenine
 *
 * onenine
 *   '1' . '9'
 *
 * fraction
 *   ""
 *   '.' digits
 *
 * exponent
 *   ""
 *   'E' sign digits
 *   'e' sign digits
 *
 * sign
 *   ""
 *   '+'
 *   '-'
 *
 * ws
 *   ""
 *   '0020' ws
 *   '000A' ws
 *   '000D' ws
 *   '0009' ws
 *
 * @formatter:on
 */

#include <StringStream.h>
#include <Stack.h>
#include "JsonParser.h"

namespace IASLib {

    CJsonNode *CJsonParser::parse( CString jsonData )
    {
        CStringStream *stream = new CStringStream( jsonData );
        CJsonNode *retVal = CJsonParser::parse( stream );
        delete stream;
        return retVal;
    }

    CJsonNode *CJsonParser::parse( CStream *jsonData )
    {
        CStack nodeStack( 16 );
        CJsonNode *retVal = NULL;
        CJsonNode *currentLevel = NULL;

        if ( jsonData->IsOpen() )
        {
            if ( ! jsonData->IsEOS() )
            {
                retVal = CJsonParser::parseElement( jsonData );
            }
        }

        return retVal;
    }

    CJsonNode *CJsonParser::parseElement(CStream *jsonData)
    {
        CJsonNode *retVal = NULL;
        if ( CJsonParser::parseWhitespace( jsonData ) ) {
            retVal = CJsonParser::parseValue( jsonData );
        }

        if (CJsonParser::parseWhitespace( jsonData ) )
        {
            return retVal;
        }

        return NULL;
    }

    bool CJsonParser::parseWhitespace( CStream *jsonData )
    {
        if ( isTokenWhitespace(reinterpret_cast<const IASLibChar__ *>(jsonData->PeekChar())) )
        {

        }
        return true;
    }


    bool CJsonParser::isTokenWhitespace( CString token ) {
        if ( token.GetLength() == 0 ) {
            return true;
        }
        if ( ( token[0] == ' ' ) || ( token[0] == '0x0a' ) || ( token[0] == '0x0d' ) || ( token[0] =='0x09') )
        {
            return isTokenWhitespace( token.Substring( 1 ) );
        }
        return false;
    }

    bool CJsonParser::isTokenSign( CString token ) {
        if ( token.GetLength() == 0 ) {
            return true;
        }
        if ( ( token[0] == '+' ) || ( token[0] == '-' ) )
        {
            return true;
        }
        return false;
    }

    bool CJsonParser::isTokenExponent( CString token ) {
        if ( token.GetLength() == 0 ) {
            return true;
        }
        if ( ( token[0] == 'e' ) || ( token[0] == 'E' ) )
        {
            return isTokenSign( token.Substring( 1, 1 ) ) && isTokenDigits( token.Substring( 2 ) );
        }
        return false;
    }

    bool CJsonParser::isTokenDigits( CString token ) {
        if ( token.GetLength() == 1 && isTokenDigit( token ) )
        {
            return true;
        }
        if ( isTokenDigit( token ) && isTokenDigits( token.Substring(1))) {
            return true;
        }
        return false;
    }

    bool CJsonParser::isTokenOneNine( CString token ) {
        if ( token[0] >= '1' && token[0] <= '9' ) {
            return true;
        }
        return false;
    }

    bool CJsonParser::isTokenDigit( CString token ) {
        if ( token[0] == '0' || isTokenOneNine( token ) )
        {
            return true;
        }
        return false;
    }

    bool CJsonParser::isTokenFraction( CString token ) {
        if ( token.GetLength() == 0 ) {
            return true;
        }
        if ( ( token[0] == '.' ) )
        {
            return isTokenDigits( token.Substring(1));
        }
        return false;
    }

    bool CJsonParser::isTokenInteger( CString token ) {
        if ( token.GetLength() >= 1 ) {
            if (token.GetLength() == 1 && isTokenDigit(token))
                return true;

            if (isTokenOneNine(token) && isTokenDigits(token.Substring(1)))
                return true;

            if (token.GetLength() == 2 && token[0] == '-' && isTokenDigit(token.Substring(1)))
                return true;

            if (token[0] == '-' && isTokenOneNine(token.Substring(1)) && isTokenDigits(token.Substring(2)))
                return true;
        }
        return false;
    }

    bool CJsonParser::isTokenNumber( CString token ) {
        size_t intPart = 1;
        size_t fractionPart = 0;
        while ( intPart < token.GetLength() && isTokenInteger( token.Substring(0,intPart ) ) ) {
            intPart++;
        }


    }

}