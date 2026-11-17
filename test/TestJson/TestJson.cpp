/***********************************************************************
**  TestJson
**
**  Tests for the JSON parser and nodes: RFC 8259 conformance (documents
**  that must be accepted and rejected), error positions, parser options,
**  numbers, strings and Unicode, building and changing trees, the find
**  methods and JSON Pointer, equality, copies, base64, streams, and a fuzz
**  pass that checks damaged input never crashes and that whatever parses
**  writes back to the same tree.
***********************************************************************/

#include "IASLib.h"

#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <vector>

using namespace IASLib;

static int g_nChecks = 0;
static int g_nFailures = 0;

#define CHECK( condition ) \
    do { \
        g_nChecks++; \
        if ( ! ( condition ) ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition ); \
        } \
    } while ( 0 )

#define CHECK_EQ_STR( input, got, expected ) \
    do { \
        g_nChecks++; \
        std::string _got = ( got ); \
        std::string _exp = ( expected ); \
        if ( _got != _exp ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: [%s] gave [%s], expected [%s]\n", __FILE__, __LINE__, std::string( input ).c_str(), _got.c_str(), _exp.c_str() ); \
        } \
    } while ( 0 )

    // Parses, returning the compact JSON, or "error: ..." on failure.
static std::string roundTrip( const std::string &strText, CJsonParser *pParser = NULL )
{
    CJsonParser parser;
    if ( pParser == NULL )
        pParser = &parser;
    CJsonNode *pNode = pParser->readTree( strText );
    if ( pNode == NULL )
        return std::string( "error: " ) + (const char *)pParser->getError();
    std::string strOut = pNode->toJson();
    delete pNode;
    return strOut;
}

static bool accepts( const std::string &strText )
{
    CJsonNode *pNode = CJsonParser::parse( strText );
    delete pNode;
    return pNode != NULL;
}

static void testConformance( void )
{
    printf( "Conformance\n" );
    struct { const char *pszText; const char *pszOut; } valid[] = {
        { "[]", "[]" }, { "{}", "{}" }, { "[1]", "[1]" }, { "\"x\"", "\"x\"" }, { "123", "123" },
        { "-0", "0" }, { "0", "0" }, { "1E2", "1E2" }, { "1e-2", "1e-2" }, { "-1.5e+3", "-1.5e+3" },
        { "[ ]", "[]" }, { " \t\r\n[1] \n", "[1]" }, { "true", "true" }, { "null", "null" },
        { "{\"a\":[1,{\"b\":null}]}", "{\"a\":[1,{\"b\":null}]}" },
        { "[true,false,null]", "[true,false,null]" },
        { "\"\\u00e9\"", "\"\xC3\xA9\"" }, { "\"\\ud83d\\ude00\"", "\"\xF0\x9F\x98\x80\"" },
        { "\"\\/\\\\\\\"\\b\\f\\n\\r\\t\"", "\"/\\\\\\\"\\b\\f\\n\\r\\t\"" },
        { "\"\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\"", "\"\xC3\xA9\xE2\x82\xAC\xF0\x9F\x98\x80\"" },
        { "\xEF\xBB\xBF[]", "[]" },
        { "[1e999]", "[1e999]" }, { "[123456789012345678901234567890]", "[123456789012345678901234567890]" },
        { "[-9223372036854775808]", "[-9223372036854775808]" }, { "[9223372036854775807]", "[9223372036854775807]" },
        { "[9223372036854775808]", "[9223372036854775808]" },
        { "\"\\u0000\"", "\"\\u0000\"" }, { "{\"a\":1,\"a\":2}", "{\"a\":2}" },
        { "{\"a\":1,\"b\":2,\"a\":3}", "{\"a\":3,\"b\":2}" }, { "[1.50]", "[1.50]" },
        { "{\"\":0}", "{\"\":0}" }, { "[\"\\u001f\"]", "[\"\\u001f\"]" }, { "\"\x7F\"", "\"\x7F\"" },
    };
    for ( size_t n = 0; n < sizeof( valid ) / sizeof( valid[0] ); n++ )
        CHECK_EQ_STR( valid[ n ].pszText, roundTrip( valid[ n ].pszText ), valid[ n ].pszOut );

    const char *invalid[] = {
        "", " ", "[", "]", "{", "}", "[1,]", "[,1]", "[,]", "{\"a\":1,}", "{,}", "{a:1}", "{'a':1}", "['a']",
        "[\"a\"", "[01]", "[-01]", "[00]", "[-]", "[.5]", "[1.]", "[1.e1]", "[1e]", "[1e+]", "[+1]", "[0x10]",
        "[NaN]", "[Infinity]", "[-Infinity]", "[tru]", "[True]", "[nul]", "[nulll]", "\"\\x\"", "\"\\u12\"",
        "\"\\u12G4\"", "\"\\ud800\"", "\"\\udc00\"", "\"\\ud800\\u0041\"", "\"\\ud800x\"", "\"\\U0041\"",
        "\"a\tb\"", "\"a\nb\"", "\"abc", "\"\\", "[1] x", "[1][2]", "{\"a\" 1}", "{\"a\":}", "{\"a\"}",
        "\"\xFF\"", "\"\xC0\xAF\"", "\"\xED\xA0\x80\"", "\"\xF4\x90\x80\x80\"", "\"\xE2\x82\"", "\"\x80\"",
        "/* c */ []", "// c\n[]", "[1 2]", "{\"a\":1 \"b\":2}", "[1,,2]", "\xEF\xBB\xBF", "\xEF\xBB[]",
        "[\xC3\xA9]", "\x0C[]", "[]\x0B", "1 2", "-", "--1", "1-", "[1e1.0]", "[\"\\u00\"]",
    };
    for ( size_t n = 0; n < sizeof( invalid ) / sizeof( invalid[0] ); n++ )
        CHECK_EQ_STR( invalid[ n ], accepts( invalid[ n ] ) ? "accepted" : "rejected", "rejected" );

        // A raw NUL is not allowed anywhere, including in a string.
    CJsonParser parser;
    CHECK( parser.readTree( "[1]\0", 4 ) == NULL );
    CHECK( parser.readTree( "\"a\0b\"", 5 ) == NULL );
    CJsonNode *pNode = parser.readTree( "[1] ", 3 );
    CHECK( pNode != NULL );
    delete pNode;
    CHECK( CJsonParser::parse( (const char *)NULL ) == NULL );
}

static void testErrors( void )
{
    printf( "Error positions\n" );
    CJsonParser parser;
    CHECK( parser.readTree( "{\n  \"a\": 1,\n  \"b\": x\n}" ) == NULL );
    CHECK( parser.hasError() );
    CHECK( parser.getErrorLine() == 3 && parser.getErrorColumn() == 8 && parser.getErrorOffset() == 19 );
    CHECK_EQ_STR( "error text", (const char *)parser.getError(), "line 3, column 8: expected a value but found 'x'" );
    CHECK_EQ_STR( "message", (const char *)parser.getErrorMessage(), "expected a value but found 'x'" );

        // Columns count characters, not bytes.
    CHECK( parser.readTree( "[\"\xC3\xA9\xC3\xA9\", ?]" ) == NULL );
    CHECK( parser.getErrorLine() == 1 && parser.getErrorColumn() == 8 );

    struct { const char *pszText; const char *pszError; } errors[] = {
        { "", "line 1, column 1: there is no JSON value" },
        { "[1,]", "line 1, column 4: expected a value but found ']'" },
        { "[1 2]", "line 1, column 4: expected ',' or ']' but found '2'" },
        { "{\"a\":1 \"b\"}", "line 1, column 8: expected ',' or '}' but found '\"'" },
        { "{1:2}", "line 1, column 2: expected a field name (a string) but found '1'" },
        { "{\"a\" 1}", "line 1, column 6: expected ':' after the field name but found '1'" },
        { "[\"abc", "line 1, column 2: the string is not closed" },
        { "[01]", "line 1, column 2: numbers cannot have leading zeros" },
        { "[1.]", "line 1, column 4: expected a digit after the decimal point but found ']'" },
        { "[1e]", "line 1, column 4: expected a digit in the exponent but found ']'" },
        { "\"\\q\"", "line 1, column 2: \\q is not a valid escape" },
        { "\"\\ud800\"", "line 1, column 2: a \\u escape gives the first half of a surrogate pair without the second" },
        { "\"a\tb\"", "line 1, column 3: control characters must be escaped in strings" },
        { "\"\xFF\"", "line 1, column 2: the text is not valid UTF-8" },
        { "[] []", "line 1, column 4: unexpected '[' after the JSON value" },
        { "[]\x01", "line 1, column 3: unexpected byte 0x01 after the JSON value" },
    };
    for ( size_t n = 0; n < sizeof( errors ) / sizeof( errors[0] ); n++ )
        CHECK_EQ_STR( errors[ n ].pszText, roundTrip( errors[ n ].pszText ), std::string( "error: " ) + errors[ n ].pszError );

        // A successful parse clears the error.
    CJsonNode *pNode = parser.readTree( "[]" );
    CHECK( pNode != NULL && ! parser.hasError() && parser.getError().GetLength() == 0 );
    delete pNode;
}

static void testOptions( void )
{
    printf( "Options\n" );
    CJsonParser parser;
    CHECK( ! parser.getAllowComments() && ! parser.getAllowTrailingCommas() && ! parser.getRejectDuplicateKeys() );
    CHECK( parser.getMaxDepth() == 512 );

    parser.setAllowComments( true );
    CHECK_EQ_STR( "comments", roundTrip( "// head\n{ /* a */ \"a\" /* b */ : // c\n 1 /**/ } // tail", &parser ), "{\"a\":1}" );
    CHECK_EQ_STR( "unterminated", roundTrip( "[1] /* open", &parser ), "error: line 1, column 5: unterminated comment" );
    CHECK_EQ_STR( "slash", roundTrip( "[1] /", &parser ), "error: line 1, column 5: unexpected '/' after the JSON value" );
    CHECK_EQ_STR( "no comment in string", roundTrip( "[\"// x /* y */\"]", &parser ), "[\"// x /* y */\"]" );
    parser.setAllowComments( false );

    parser.setAllowTrailingCommas( true );
    CHECK_EQ_STR( "trailing", roundTrip( "{\"a\":[1,2,],}", &parser ), "{\"a\":[1,2]}" );
    CHECK_EQ_STR( "not alone", roundTrip( "[,]", &parser ), "error: line 1, column 2: expected a value but found ','" );
    CHECK_EQ_STR( "not twice", roundTrip( "[1,,]", &parser ), "error: line 1, column 4: expected a value but found ','" );
    parser.setAllowTrailingCommas( false );

    parser.setRejectDuplicateKeys( true );
    CHECK_EQ_STR( "duplicate", roundTrip( "{\"a\":1,\n\"a\":2}", &parser ), "error: line 2, column 1: the field name \"a\" is repeated" );
    CHECK_EQ_STR( "distinct", roundTrip( "{\"a\":{\"a\":1}}", &parser ), "{\"a\":{\"a\":1}}" );
    parser.setRejectDuplicateKeys( false );

        // Nesting: 512 levels by default.
    std::string strDeep( 512, '[' );
    strDeep += std::string( 512, ']' );
    CHECK( roundTrip( strDeep, &parser ) == strDeep );
    std::string strTooDeep( 513, '[' );
    strTooDeep += std::string( 513, ']' );
    CHECK_EQ_STR( "too deep", roundTrip( strTooDeep, &parser ), "error: line 1, column 513: arrays and objects are nested more than 512 deep" );
        // Very deep input stops at the limit rather than overflowing the stack.
    std::string strHostile( 10000000, '[' );
    CHECK( roundTrip( strHostile, &parser ).compare( 0, 6, "error:" ) == 0 );
    parser.setMaxDepth( 2 );
    CHECK_EQ_STR( "depth 2", roundTrip( "{\"a\":[1]}", &parser ), "{\"a\":[1]}" );
    CHECK( roundTrip( "{\"a\":[[1]]}", &parser ).compare( 0, 6, "error:" ) == 0 );
    parser.setMaxDepth( 0 );
    CHECK( parser.getMaxDepth() == 1 );
}

static void testNumbers( void )
{
    printf( "Numbers\n" );
    CJsonNode *pRoot = CJsonParser::parse( "[0, -7, 2147483647, 2147483648, -9223372036854775808, 12345678901234567890, 1.5, -2.5e3, 1e999, 3.99, -3.99]" );
    CHECK( pRoot != NULL );
    if ( pRoot == NULL )
        return;
    const CJsonNode &a = *pRoot;
    CHECK( a.path( 0 ).isInt() && a.path( 0 ).intValue() == 0 );
    CHECK( a.path( 1 ).isInt() && a.path( 1 ).longValue() == -7 && a.path( 1 ).isIntegralNumber() );
    CHECK( a.path( 2 ).isInt() && a.path( 2 ).canConvertToInt() );
    CHECK( ! a.path( 3 ).isInt() && a.path( 3 ).isLong() && ! a.path( 3 ).canConvertToInt() && a.path( 3 ).longValue() == 2147483648LL );
    CHECK( a.path( 3 ).intValue() == 2147483647 );                       // clamped
    CHECK( a.path( 4 ).isLong() && a.path( 4 ).longValue() == -9223372036854775807LL - 1 );
    CHECK( a.path( 5 ).isBigInteger() && a.path( 5 ).isIntegralNumber() && ! a.path( 5 ).isLong() );
    CHECK( ! a.path( 5 ).canConvertToLong() && a.path( 5 ).longValue() == 9223372036854775807LL );
    CHECK( fabs( a.path( 5 ).doubleValue() - 12345678901234567890.0 ) < 1e4 );
    CHECK_EQ_STR( "big text", (const char *)a.path( 5 ).asText(), "12345678901234567890" );
    CHECK( a.path( 6 ).isDouble() && a.path( 6 ).isFloatingPointNumber() && a.path( 6 ).doubleValue() == 1.5 );
    CHECK( a.path( 6 ).intValue() == 1 && a.path( 6 ).canConvertToInt() );
    CHECK( a.path( 7 ).doubleValue() == -2500.0 && a.path( 7 ).longValue() == -2500 );
    CHECK( std::isinf( a.path( 8 ).doubleValue() ) && ! a.path( 8 ).canConvertToLong() );
    CHECK( a.path( 9 ).intValue() == 3 && a.path( 10 ).intValue() == -3 );         // truncated
    CHECK( a.path( 6 ).floatValue() == 1.5f && a.path( 1 ).shortValue() == -7 );
    delete pRoot;

        // Writing doubles: the shortest text that reads back the same, always
        // with a fraction or exponent; non-finite values as null.
    struct { double dValue; const char *pszText; } doubles[] = {
        { 0.1, "0.1" }, { 5.0, "5.0" }, { -0.0, "-0.0" }, { 1e21, "1e+21" }, { 1.0 / 3.0, "0.3333333333333333" },
        { 123456.789, "123456.789" }, { 5e-324, "5e-324" }, { 1.7976931348623157e308, "1.7976931348623157e+308" },
        { 100.0, "100.0" }, { 0.000001, "0.000001" }, { 1e-7, "0.0000001" }, { 1e-8, "1e-8" },
        { 1e20, "100000000000000000000.0" }, { 123e18, "123000000000000000000.0" }, { -1.5e-10, "-1.5e-10" }, { 2.5e300, "2.5e+300" },
    };
    for ( size_t n = 0; n < sizeof( doubles ) / sizeof( doubles[0] ); n++ )
    {
        CNumberNode number( doubles[ n ].dValue );
        CHECK_EQ_STR( doubles[ n ].pszText, number.toJson(), doubles[ n ].pszText );
        CJsonNode *pBack = CJsonParser::parse( number.toJson() );
        CHECK( pBack != NULL && pBack->doubleValue() == doubles[ n ].dValue && pBack->isDouble() );
        delete pBack;
    }
    CHECK_EQ_STR( "nan", CNumberNode( std::nan( "" ) ).toJson(), "null" );
    CHECK_EQ_STR( "inf", CNumberNode( HUGE_VAL ).toJson(), "null" );
    CHECK_EQ_STR( "ull", CNumberNode( 18446744073709551615ULL ).toJson(), "18446744073709551615" );
    CHECK( CNumberNode( 18446744073709551615ULL ).isBigInteger() );
    CHECK( CNumberNode( 5ULL ).isInt() );

        // Equality by value.
    CNumberNode one( 1 ), oneDouble( 1.0 ), two( 2 ), oneLong( 1LL );
    CHECK( one.equals( &oneDouble ) && oneDouble.equals( &one ) && one.equals( &oneLong ) && ! one.equals( &two ) );
    CNumberNode exact( 9007199254740993LL ), rounded( 9007199254740992.0 );
    CHECK( ! exact.equals( &rounded ) );
    CJsonNode *pBig1 = CJsonParser::parse( "123456789012345678901234567890" );
    CJsonNode *pBig2 = CJsonParser::parse( "123456789012345678901234567890" );
    CJsonNode *pBig3 = CJsonParser::parse( "123456789012345678901234567891" );
    CHECK( pBig1 && pBig2 && pBig3 && pBig1->equals( pBig2 ) && ! pBig1->equals( pBig3 ) );
    delete pBig1;
    delete pBig2;
    delete pBig3;
    CTextNode text( "1" );
    CHECK( ! one.equals( &text ) );

        // Locale: a comma decimal point must not leak into JSON.
    const char *pszLocales[] = { "de_DE.UTF-8", "de_DE.utf8", "fr_FR.UTF-8", "de_DE", "German" };
    for ( size_t n = 0; n < sizeof( pszLocales ) / sizeof( pszLocales[0] ); n++ )
    {
        if ( setlocale( LC_NUMERIC, pszLocales[ n ] ) == NULL )
            continue;
        CHECK_EQ_STR( "locale write", CNumberNode( 1.5 ).toJson(), "1.5" );
        CJsonNode *pNode = CJsonParser::parse( "[1.25]" );
        CHECK( pNode && pNode->path( 0 ).doubleValue() == 1.25 );
        delete pNode;
        setlocale( LC_NUMERIC, "C" );
        break;
    }
}

static void testStrings( void )
{
    printf( "Strings and Unicode\n" );
    CJsonNode *pNode = CJsonParser::parse( "[\"caf\\u00e9\", \"a\\u0000b\", \"\\ud834\\udd1e\", \"tab\\there\"]" );
    CHECK( pNode != NULL );
    if ( pNode == NULL )
        return;
    CHECK_EQ_STR( "text", (const char *)pNode->path( 0 ).textValue(), "caf\xC3\xA9" );
    CHECK( pNode->path( 1 ).textString() == std::string( "a\0b", 3 ) );
    CHECK( pNode->path( 1 ).textValue().GetLength() == 1 );                  // CString stops at the NUL
    CHECK( pNode->path( 2 ).textString() == "\xF0\x9D\x84\x9E" );
    CHECK_EQ_STR( "escaped out", pNode->toJson(), "[\"caf\xC3\xA9\",\"a\\u0000b\",\"\xF0\x9D\x84\x9E\",\"tab\\there\"]" );
    CHECK_EQ_STR( "ascii out", pNode->toJson( -1, true ), "[\"caf\\u00e9\",\"a\\u0000b\",\"\\ud834\\udd1e\",\"tab\\there\"]" );
    CJsonNode *pBack = CJsonParser::parse( pNode->toJson( -1, true ) );
    CHECK( pBack && pBack->equals( pNode ) );
    delete pBack;
    delete pNode;

        // Text that is not UTF-8 is written with U+FFFD, so output is valid.
    CTextNode bad( std::string( "a\xFF" "b\xC3", 4 ) );
    CHECK_EQ_STR( "bad utf8", bad.toJson(), "\"a\xEF\xBF\xBD" "b\xEF\xBF\xBD\"" );
    CHECK_EQ_STR( "bad utf8 ascii", bad.toJson( -1, true ), "\"a\\ufffdb\\ufffd\"" );
    CHECK( accepts( bad.toJson() ) );
    CTextNode controls( std::string( "\x01\x1f\x7f\"\\/", 6 ) );
    CHECK_EQ_STR( "controls", controls.toJson(), "\"\\u0001\\u001f\x7f\\\"\\\\/\"" );

        // Field names are escaped too.
    CObjectNode object;
    object.put( "a\"b\n", 1 );
    CHECK_EQ_STR( "key escape", object.toJson(), "{\"a\\\"b\\n\":1}" );
}

static void testNavigation( void )
{
    printf( "Navigation, JSON Pointer and find\n" );
    const char *pszDoc =
        "{ \"users\": [ { \"name\": \"Ann\", \"age\": 31, \"tags\": [\"a\", \"b\"] },"
        "              { \"name\": \"Bob\", \"age\": \"42\", \"admin\": true, \"boss\": { \"name\": \"Cy\" } } ],"
        "  \"count\": 2, \"a/b\": 1, \"m~n\": 2, \"\": 3, \"empty\": null }";
    CJsonNode *pRoot = CJsonParser::parse( pszDoc );
    CHECK( pRoot != NULL );
    if ( pRoot == NULL )
        return;
    const CJsonNode &root = *pRoot;
    CHECK( root.isObject() && root.isContainerNode() && ! root.isValueNode() && root.size() == 6 );
    CHECK_EQ_STR( "chain", (const char *)root.path( "users" ).path( 1 ).path( "name" ).asText(), "Bob" );
    CHECK( root.path( "users" ).path( 9 ).path( "name" ).isMissingNode() );
    CHECK( root.path( "nope" ).path( "deeper" ).path( 3 ).isMissingNode() );
    CHECK( &root.path( "nope" ) == &CMissingNode::getInstance() );
    CHECK( root.path( "nope" ).asText() == "" && root.path( "nope" ).asInt( 7 ) == 7 && root.path( "nope" ).toJson() == "" );
    CHECK( root.get( "users" ) != NULL && root.get( "nope" ) == NULL && root.get( 0 ) == NULL );
    CHECK( root.path( "users" ).get( 0 ) != NULL && root.path( "users" ).get( -1 ) == NULL && root.path( "users" ).get( "x" ) == NULL );
    CHECK( root.has( "empty" ) && ! root.hasNonNull( "empty" ) && root.hasNonNull( "count" ) && ! root.has( "nope" ) );
    CHECK( root.path( "users" ).has( 1 ) && ! root.path( "users" ).has( 2 ) );
    CString strKey( "count" );
    CHECK( root.path( (const char *)strKey ).asInt() == 2 && root.path( std::string( "count" ) ).asInt() == 2 );
    CHECK( root.get( strKey ) != NULL );

        // Coercions.
    CHECK( root.path( "users" ).path( 1 ).path( "age" ).asInt() == 42 );          // text "42"
    CHECK( root.path( "users" ).path( 1 ).path( "age" ).intValue() == 0 );        // not a number node
    CHECK( root.path( "users" ).path( 1 ).path( "admin" ).asBoolean() && root.path( "users" ).path( 1 ).path( "admin" ).asInt() == 1 );
    CHECK( root.path( "count" ).asBoolean() && root.path( "count" ).asDouble() == 2.0 && root.path( "count" ).asText() == "2" );
    CHECK( root.path( "empty" ).asText() == "null" && root.path( "empty" ).asInt( 5 ) == 5 && root.path( "users" ).asText() == "" );
    CTextNode t1( " true " ), t2( "12.9" ), t3( "1e3" ), t4( "abc" ), t5( "0x10" ), t6( "inf" ), t7( "99999999999999999999" );
    CHECK( t1.asBoolean() && ! t4.asBoolean() && t4.asBoolean( true ) );
    CHECK( t2.asInt() == 12 && t2.asDouble() == 12.9 && t3.asLong() == 1000 );
    CHECK( t4.asInt( -1 ) == -1 && t5.asInt( -1 ) == -1 && t6.asDouble( -1 ) == -1 );
    CHECK( t7.asLong() == 9223372036854775807LL && t7.asInt() == 2147483647 );

        // JSON Pointer (RFC 6901, including its examples' escapes).
    CHECK( &root.at( "" ) == pRoot );
    CHECK_EQ_STR( "/users/0/name", (const char *)root.at( "/users/0/name" ).asText(), "Ann" );
    CHECK_EQ_STR( "/users/0/tags/1", (const char *)root.at( "/users/0/tags/1" ).asText(), "b" );
    CHECK( root.at( "/a~1b" ).asInt() == 1 && root.at( "/m~0n" ).asInt() == 2 && root.at( "/" ).asInt() == 3 );
    CHECK( root.at( "/users/01" ).isMissingNode() && root.at( "/users/-" ).isMissingNode() && root.at( "/users/2" ).isMissingNode() );
    CHECK( root.at( "users" ).isMissingNode() && root.at( "/m~2n" ).isMissingNode() && root.at( "/count/0" ).isMissingNode() );
    CHECK( root.at( std::string( "/users/1/boss/name" ) ).asText() == "Cy" );

        // find: depth first, and a matching field is not searched inside.
    CHECK( root.findValue( "name" ) && root.findValue( "name" )->asText() == "Ann" );
    CHECK( root.findValue( "missing" ) == NULL && root.findPath( "missing" ).isMissingNode() );
    CStringArray aNames = root.findValuesAsText( "name" );
    CHECK( aNames.GetCount() == 3 && aNames.Get( 0 ) == "Ann" && aNames.Get( 1 ) == "Bob" && aNames.Get( 2 ) == "Cy" );
    CHECK( root.findValues( "admin" ).size() == 1 && root.findValues( "nothing" ).empty() );
    std::vector<CObjectNode *> aParents = root.findParents( "name" );
    CHECK( aParents.size() == 3 && aParents[2]->path( "name" ).asText() == "Cy" );
    CHECK( root.findParent( "admin" ) == &root.at( "/users/1" ) );
    CJsonNode *pOuter = CJsonParser::parse( "{\"x\":{\"x\":1}}" );
    CHECK( pOuter && pOuter->findValues( "x" ).size() == 1 && pOuter->findValue( "x" )->isObject() );
    delete pOuter;

        // Object fields by position.
    const CObjectNode *pObject = static_cast<const CObjectNode *>( pRoot );
    CHECK( pObject->fieldName( 0 ) == "users" && pObject->fieldName( 1 ) == "count" && pObject->fieldName( 99 ) == "" );
    CHECK( pObject->fieldValue( 1 )->asInt() == 2 && pObject->fieldValue( -1 ) == NULL );
    CStringArray aFields = pObject->fieldNames();
    CHECK( aFields.GetCount() == 6 && aFields.Get( 5 ) == "empty" );
    delete pRoot;
}

static void testBuilding( void )
{
    printf( "Building and changing trees\n" );
    CObjectNode *pRoot = new CObjectNode();
    pRoot->put( "name", "Ann" )->put( "age", 31 )->put( "height", 1.68 )->put( "admin", false )->putNull( "manager" );
    pRoot->put( "big", 18446744073709551615ULL )->put( "long", 5000000000LL )->put( "u", 7u )->put( "l", -3L );
    pRoot->put( CString( "cstring" ), CString( "value" ) )->put( std::string( "std" ), std::string( "value" ) );
    pRoot->put( "nullText", (const char *)NULL );
    CArrayNode *pTags = pRoot->putArray( "tags" );
    pTags->add( "a" )->add( 1 )->add( true )->addNull()->add( 2.5 )->add( std::string( "s" ) )->add( 9ULL );
    pTags->addObject()->put( "k", "v" );
    pTags->addArray()->add( 1 )->add( 2 );
    CHECK_EQ_STR( "built", pRoot->toJson(),
                  "{\"name\":\"Ann\",\"age\":31,\"height\":1.68,\"admin\":false,\"manager\":null,\"big\":18446744073709551615,"
                  "\"long\":5000000000,\"u\":7,\"l\":-3,\"cstring\":\"value\",\"std\":\"value\",\"nullText\":null,"
                  "\"tags\":[\"a\",1,true,null,2.5,\"s\",9,{\"k\":\"v\"},[1,2]]}" );

        // Replacing a field keeps its place; removing closes the gap.
    pRoot->put( "age", 32 );
    CHECK( pRoot->fieldName( 1 ) == "age" && pRoot->path( "age" ).asInt() == 32 );
    CHECK( pRoot->remove( "height" ) && ! pRoot->remove( "height" ) && pRoot->fieldName( 2 ) == "admin" );
    CJsonNode *pReleased = pRoot->release( "name" );
    CHECK( pReleased && pReleased->asText() == "Ann" && ! pRoot->has( "name" ) && pRoot->fieldName( 0 ) == "age" );
    CHECK( pRoot->path( "admin" ).isBoolean() && pRoot->get( "age" ) != NULL );
    delete pReleased;
    CHECK( pRoot->release( "nothing" ) == NULL );

        // with / withArray create or return; NULL for the wrong type.
    CObjectNode *pAddress = pRoot->with( "address" );
    pAddress->put( "city", "Paris" );
    CHECK( pRoot->with( "address" ) == pAddress && pRoot->with( "tags" ) == NULL );
    CHECK( pRoot->withArray( "tags" ) == pTags && pRoot->withArray( "address" ) == NULL );
    pRoot->withArray( "list" )->add( 1 );
    CHECK( pRoot->at( "/address/city" ).asText() == "Paris" && pRoot->at( "/list/0" ).asInt() == 1 );

        // Arrays: insert, set, remove, release.
    CArrayNode *pList = pRoot->withArray( "list" );
    CHECK( pList->insert( 0, new CTextNode( "first" ) ) && pList->insert( 2, new CTextNode( "last" ) ) );
    CHECK( ! pList->insert( 9, new CTextNode( "lost" ) ) && ! pList->insert( -1, NULL ) );
    CHECK_EQ_STR( "list", pList->toJson(), "[\"first\",1,\"last\"]" );
    CHECK( pList->set( 1, new CBooleanNode( true ) ) && ! pList->set( 3, new CNullNode() ) && pList->set( 0, NULL ) );
    CHECK_EQ_STR( "list set", pList->toJson(), "[null,true,\"last\"]" );
    CHECK( pList->remove( 0 ) && ! pList->remove( 5 ) && pList->size() == 2 );
    CJsonNode *pLast = pList->release( 1 );
    CHECK( pLast && pLast->asText() == "last" && pList->release( 1 ) == NULL );
    delete pLast;
    pList->removeAll();
    CHECK( pList->isEmpty() && pList->toJson() == "[]" );
    pRoot->set( "nested", CJsonParser::parse( "{\"x\":[1,{\"y\":2}]}" ) )->set( "nothing", NULL );
    CHECK( pRoot->at( "/nested/x/1/y" ).asInt() == 2 && pRoot->path( "nothing" ).isNull() );

        // Pretty printing.
    CObjectNode pretty;
    pretty.put( "a", 1 )->putArray( "b" )->add( 2 )->add( 3 );
    pretty.putObject( "c" );
    pretty.putArray( "d" );
    pretty.putObject( "e" )->put( "f", "g" );
    CHECK_EQ_STR( "pretty", (const char *)pretty.toPrettyString(),
                  "{\n  \"a\": 1,\n  \"b\": [\n    2,\n    3\n  ],\n  \"c\": {},\n  \"d\": [],\n  \"e\": {\n    \"f\": \"g\"\n  }\n}" );
    CHECK_EQ_STR( "indent 4", pretty.path( "e" ).toJson( 4 ), "{\n    \"f\": \"g\"\n}" );
    CHECK_EQ_STR( "indent 0", pretty.path( "b" ).toJson( 0 ), "[\n2,\n3\n]" );
    CHECK_EQ_STR( "toString", (const char *)pretty.toString(), "{\"a\":1,\"b\":[2,3],\"c\":{},\"d\":[],\"e\":{\"f\":\"g\"}}" );
    CJsonNode *pReparsed = CJsonParser::parse( (const char *)pretty.toPrettyString() );
    CHECK( pReparsed && pReparsed->equals( &pretty ) );
    delete pReparsed;

        // Equality and copies.
    CJsonNode *pCopy = pRoot->clone();
    CHECK( pCopy->equals( pRoot ) && pRoot->equals( pCopy ) && pCopy != pRoot );
    static_cast<CObjectNode *>( pCopy )->with( "address" )->put( "city", "Rome" );
    CHECK( ! pCopy->equals( pRoot ) && pRoot->at( "/address/city" ).asText() == "Paris" );
    delete pCopy;
    CJsonNode *pA = CJsonParser::parse( "{\"a\":1,\"b\":[1,2,{\"c\":null}]}" );
    CJsonNode *pB = CJsonParser::parse( "{\"b\":[1,2.0,{\"c\":null}],\"a\":1.0}" );
    CJsonNode *pC = CJsonParser::parse( "{\"b\":[2,1,{\"c\":null}],\"a\":1}" );
    CJsonNode *pD = CJsonParser::parse( "{\"a\":1,\"b\":[1,2,{\"c\":null}],\"x\":0}" );
    CHECK( pA && pB && pC && pD );
    CHECK( pA->equals( pB ) && ! pA->equals( pC ) && ! pA->equals( pD ) && ! pD->equals( pA ) && ! pA->equals( NULL ) );
    CHECK( CMissingNode::getInstance().equals( &CMissingNode::getInstance() ) && ! CMissingNode::getInstance().equals( pA ) );
    CJsonNode *pMissingCopy = CMissingNode::getInstance().clone();
    CHECK( pMissingCopy->isMissingNode() && pMissingCopy != &CMissingNode::getInstance() );
    delete pMissingCopy;
    delete pA;
    delete pB;
    delete pC;
    delete pD;
    delete pRoot;
}

static void testBinary( void )
{
    printf( "Binary and base64\n" );
    struct { const char *pszData; const char *pszBase64; } vectors[] = {     // RFC 4648 section 10
        { "", "" }, { "f", "Zg==" }, { "fo", "Zm8=" }, { "foo", "Zm9v" }, { "foob", "Zm9vYg==" },
        { "fooba", "Zm9vYmE=" }, { "foobar", "Zm9vYmFy" },
    };
    for ( size_t n = 0; n < sizeof( vectors ) / sizeof( vectors[0] ); n++ )
    {
        std::vector<unsigned char> aData( vectors[ n ].pszData, vectors[ n ].pszData + strlen( vectors[ n ].pszData ) );
        CHECK_EQ_STR( vectors[ n ].pszData, CJsonNode::encodeBase64( aData ), vectors[ n ].pszBase64 );
        std::vector<unsigned char> aBack;
        CHECK( CJsonNode::decodeBase64( vectors[ n ].pszBase64, aBack ) && aBack == aData );
    }
    std::vector<unsigned char> aOut;
    CHECK( CJsonNode::decodeBase64( "Zm9vYg", aOut ) && aOut.size() == 4 );          // no padding
    CHECK( CJsonNode::decodeBase64( "-_8", aOut ) && aOut.size() == 2 && aOut[0] == 0xFB && aOut[1] == 0xFF );   // URL-safe
    const char *bad[] = { "Zg=", "Z", "Zm9v Yg==", "Zm9v!", "Zh==", "Zm9=", "====", "Zg==Zg==" };
    for ( size_t n = 0; n < sizeof( bad ) / sizeof( bad[0] ); n++ )
        CHECK_EQ_STR( bad[ n ], CJsonNode::decodeBase64( bad[ n ], aOut ) ? "decoded" : "rejected", "rejected" );

    unsigned char achBytes[] = { 0, 1, 2, 0xFE, 0xFF };
    CObjectNode object;
    object.putBinary( "blob", std::vector<unsigned char>( achBytes, achBytes + 5 ) );
    CHECK( object.path( "blob" ).isBinary() && object.path( "blob" ).asText() == "AAEC/v8=" );
    CHECK_EQ_STR( "binary json", object.toJson(), "{\"blob\":\"AAEC/v8=\"}" );
    CJsonNode *pParsed = CJsonParser::parse( object.toJson() );
    CHECK( pParsed && pParsed->path( "blob" ).isTextual() );
    std::vector<unsigned char> aBlob;
    CHECK( pParsed && pParsed->path( "blob" ).binaryValue( aBlob ) && aBlob == std::vector<unsigned char>( achBytes, achBytes + 5 ) );
    CHECK( pParsed && ! pParsed->path( "missing" ).binaryValue( aBlob ) );
    CBinaryNode copy( achBytes, 5 );
    CHECK( copy.equals( object.get( "blob" ) ) && ! copy.equals( pParsed->get( "blob" ) ) );
    delete pParsed;
}

static void testStreams( void )
{
    printf( "Streams\n" );
    CStringStream stream( CString( "{\"a\": [1, 2, 3]}" ) );
    CJsonNode *pNode = CJsonParser::parse( &stream );
    CHECK( pNode && pNode->at( "/a/2" ).asInt() == 3 );
    delete pNode;

        // Larger than one read.
    std::string strBig = "[";
    for ( int n = 0; n < 5000; n++ )
        strBig += ( n ? "," : "" ) + std::to_string( n );
    strBig += "]";
    CStringStream bigStream( CString( strBig.c_str() ) );
    pNode = CJsonParser::parse( &bigStream );
    CHECK( pNode && pNode->size() == 5000 && pNode->path( 4999 ).asInt() == 4999 );
    delete pNode;

    CJsonParser parser;
    CHECK( parser.readTree( (CStream *)NULL ) == NULL );
    CHECK_EQ_STR( "null stream", (const char *)parser.getError(), "line 1, column 1: the stream is not open" );
}

    // Random trees, written and read back, must come back equal.
static CJsonNode *randomTree( std::mt19937 &random, int nDepth )
{
    int nKind = (int)( random() % ( nDepth > 4 ? 6 : 8 ) );
    switch ( nKind )
    {
        case 0: return new CNullNode();
        case 1: return new CBooleanNode( random() % 2 == 0 );
        case 2: return new CNumberNode( (long long)( random() ) - (long long)( random() ) * 4096 );
        case 3:
        {
            double dValue = std::ldexp( (double)random() / 4294967296.0, (int)( random() % 200 ) - 100 );
            return new CNumberNode( random() % 2 ? dValue : -dValue );
        }
        case 4:
        case 5:
        {
            std::string strText;
            int nLength = (int)( random() % 12 );
            const char *pieces[] = { "a", "\"", "\\", "\n", "\x01", "\xC3\xA9", "\xF0\x9F\x98\x80", "/", " ", "\x7F" };
            for ( int n = 0; n < nLength; n++ )
            {
                size_t nPiece = random() % 11;
                if ( nPiece == 10 )
                    strText += std::string( "\0", 1 );
                else
                    strText += pieces[ nPiece ];
            }
            return new CTextNode( strText );
        }
        case 6:
        {
            CArrayNode *pArray = new CArrayNode();
            int nCount = (int)( random() % 5 );
            for ( int n = 0; n < nCount; n++ )
                pArray->add( randomTree( random, nDepth + 1 ) );
            return pArray;
        }
        default:
        {
            CObjectNode *pObject = new CObjectNode();
            int nCount = (int)( random() % 5 );
            for ( int n = 0; n < nCount; n++ )
                pObject->set( std::string( "k" ) + std::to_string( random() % 8 ) + ( random() % 3 == 0 ? "\xC3\xA9\"" : "" ), randomTree( random, nDepth + 1 ) );
            return pObject;
        }
    }
}

static void testFuzz( void )
{
    printf( "Fuzzing\n" );
    std::mt19937 random( 8259 );

    int nRoundTripFailures = 0;
    for ( int n = 0; n < 3000; n++ )
    {
        CJsonNode *pTree = randomTree( random, 0 );
        for ( int nMode = 0; nMode < 3; nMode++ )
        {
            std::string strJson = pTree->toJson( nMode == 1 ? 2 : -1, nMode == 2 );
            CJsonNode *pBack = CJsonParser::parse( strJson );
            if ( pBack == NULL || ! pBack->equals( pTree ) || pBack->toJson( nMode == 1 ? 2 : -1, nMode == 2 ) != strJson )
            {
                if ( nRoundTripFailures++ == 0 )
                    printf( "  round trip failed: %s\n", strJson.c_str() );
            }
            delete pBack;
        }
        delete pTree;
    }
    CHECK( nRoundTripFailures == 0 );

        // Damaged documents: never a crash (the sanitizer builds check memory),
        // and anything accepted writes back to text that parses the same.
    const char *seeds[] = {
        "{\"a\":[1,2.5e-3,{\"b\":\"c\\u00e9\\ud83d\\ude00\"}],\"d\":true,\"e\":null,\"f\":-0}",
        "[\"\xC3\xA9\\n\",[[[]]],{},123456789012345678901234,{\"\":\"\"}]",
    };
    const char alphabet[] = "{}[]\",:\\u0123456789abcdefABCDEF.eE+- \t\n\xC3\xA9\xFF\x80/*tnrfl";
    int nAccepted = 0;
    int nInconsistent = 0;
    CJsonParser lenient;
    lenient.setAllowComments( true );
    lenient.setAllowTrailingCommas( true );
    for ( int n = 0; n < 60000; n++ )
    {
        std::string strText = seeds[ n % 2 ];
        int nEdits = 1 + (int)( random() % 4 );
        for ( int e = 0; e < nEdits && ! strText.empty(); e++ )
        {
            size_t nAt = random() % strText.size();
            switch ( random() % 4 )
            {
                case 0: strText[ nAt ] = alphabet[ random() % ( sizeof( alphabet ) - 1 ) ]; break;
                case 1: strText.erase( nAt, 1 + random() % 3 ); break;
                case 2: strText.insert( nAt, 1, alphabet[ random() % ( sizeof( alphabet ) - 1 ) ] ); break;
                default: strText.resize( nAt ); break;
            }
        }
        CJsonParser strict;
        CJsonParser *pParser = ( n % 3 == 0 ) ? &lenient : &strict;
        CJsonNode *pNode = pParser->readTree( strText );
        if ( pNode == NULL )
        {
            if ( ! pParser->hasError() || pParser->getErrorOffset() > strText.size() )
                nInconsistent++;
            continue;
        }
        nAccepted++;
        std::string strOut = pNode->toJson();
        CJsonNode *pBack = CJsonParser::parse( strOut );
        if ( pBack == NULL || ! pBack->equals( pNode ) || pBack->toJson() != strOut )
        {
            if ( nInconsistent++ == 0 )
                printf( "  inconsistent: %s -> %s\n", strText.c_str(), strOut.c_str() );
        }
        delete pBack;
        delete pNode;
    }
    CHECK( nInconsistent == 0 );
    CHECK( nAccepted > 0 );
}

int main( void )
{
    testConformance();
    testErrors();
    testOptions();
    testNumbers();
    testStrings();
    testNavigation();
    testBuilding();
    testBinary();
    testStreams();
    testFuzz();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
