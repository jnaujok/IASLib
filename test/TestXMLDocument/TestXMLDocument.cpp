/***********************************************************************
**  TestXMLDocument
**
**  Tests for the XML classes: reading (elements, attributes, text,
**  entities and character references, CDATA, comments, processing
**  instructions, DOCTYPE and its internal entities), well-formedness
**  errors with their positions, the protections against external
**  entities and entity expansion, writing (exact and pretty, with
**  escaping), the element API, a real file (DatabaseLayout.xml, as the
**  database metadata classes read it), and a fuzz pass.
***********************************************************************/

#include "IASLib.h"

#include <cstdio>
#include <random>
#include <string>

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

#define CHECK_STR( got, expected ) \
    do { \
        g_nChecks++; \
        std::string _got = (const char *)( got ); \
        std::string _exp = ( expected ); \
        if ( _got != _exp ) { \
            g_nFailures++; \
            printf( "  FAILED %s:%d: %s gave [%s], expected [%s]\n", __FILE__, __LINE__, #got, _got.c_str(), _exp.c_str() ); \
        } \
    } while ( 0 )

static void section( const char *pszName )
{
    printf( "%s\n", pszName );
}

    // Parses, returning the document (or NULL, with the error in strError).
static CXMLDocument *parse( const std::string &strXML, std::string *pError = NULL, bool bSingleRoot = false )
{
    CXMLDocument *pDocument = new CXMLDocument();
    try
    {
        CXMLParser oParser;
        oParser.SetSingleRoot( bSingleRoot );
        oParser.Parse( strXML.data(), strXML.size(), *pDocument->GetIndex() );
        return pDocument;
    }
    catch ( CXMLException &oE )
    {
        if ( pError != NULL )
            *pError = (const char *)oE.GetMessage();
        delete pDocument;
        return NULL;
    }
}

    // The error for a document that should not parse ("" if it parsed).
static std::string errorOf( const std::string &strXML, bool bSingleRoot = false )
{
    std::string strError;
    CXMLDocument *pDocument = parse( strXML, &strError, bSingleRoot );
    if ( pDocument != NULL )
    {
        delete pDocument;
        return "";
    }
    return strError;
}

static std::string rawOf( const std::string &strXML )
{
    CXMLDocument *pDocument = parse( strXML );
    if ( pDocument == NULL )
        return "<error>";
    std::string strOut = (const char *)pDocument->toString();
    delete pDocument;
    return strOut;
}

/***********************************************************************
**  The original tests
***********************************************************************/

static void testMemoryStream( void )
{
    section( "A document in memory" );

    CString strXML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                     "<?xml-stylesheet type=\"text/xsl\" href=\"_UpgradeReport_Files/UpgradeReport.xslt\"?>"
                     "<UpgradeLog><Properties>"
                     "<Property Name=\"Solution\" Value=\"BaseCamp2\">A Useful Solution for Software Management</Property>"
                     "</Properties>"
                     "</UpgradeLog>"
                     "<Dummy/>";
    CString strCompareIndent = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<?xml-stylesheet type=\"text/xsl\" href=\"_UpgradeReport_Files/UpgradeReport.xslt\"?>\n"
                               "<UpgradeLog>\n"
                               "    <Properties>\n"
                               "        <Property Name=\"Solution\" Value=\"BaseCamp2\">\n"
                               "            A Useful Solution for Software Management\n"
                               "        </Property>\n"
                               "    </Properties>\n"
                               "</UpgradeLog>\n"
                               "<Dummy/>\n";

    CStringStream osInput( strXML );
    CXMLDocument xDoc( &osInput );
    CXMLIndex *pIndex = xDoc.GetIndex();
    CHECK( pIndex->GetElementCount() == 4 );
    CHECK_STR( xDoc.toString( 4 ), (const char *)strCompareIndent );
    CHECK_STR( xDoc.toString(), (const char *)strXML );

    CXMLElement *pDeclaration = pIndex->GetElement( (size_t)0 );
    CHECK( pDeclaration->IsMetaTag() );
    CHECK_STR( pDeclaration->GetName(), "xml" );
    CHECK_STR( pDeclaration->GetProperty( "encoding" ), "UTF-8" );
    CHECK_STR( pIndex->GetElement( 1 )->GetProperty( "href" ), "_UpgradeReport_Files/UpgradeReport.xslt" );

    CXMLElement *pRoot = xDoc.GetRoot();
    CHECK( pRoot != NULL && strcmp( pRoot->GetName(), "UpgradeLog" ) == 0 );
    const CXMLElement *pProperty = pRoot->GetSubElement( "Properties" )->GetSubElement( "Property" );
    CHECK_STR( pProperty->GetProperty( "Value" ), "BaseCamp2" );
    CHECK_STR( pProperty->GetData(), "A Useful Solution for Software Management" );
}

static void testFile( const char *pszFileName )
{
    section( "DatabaseLayout.xml" );

    CXMLDocument *pDocument = NULL;
    try
    {
        pDocument = CXMLParser::parseFile( CString( pszFileName ) );
    }
    catch ( CXMLException &oE )
    {
        printf( "  %s\n", (const char *)oE.GetMessage() );
    }
    CHECK( pDocument != NULL );
    if ( pDocument == NULL )
        return;

    CXMLIndex *pIndex = pDocument->GetIndex();
    CHECK( pIndex->GetElementCount() == 73 );
    CHECK( pIndex->GetElementCount( "table" ) == 72 );

        // As the database metadata classes read it.
    CXMLElement *pTable = pIndex->GetElement( "table" );
    CHECK_STR( pTable->GetProperty( "name" ), "user_archive" );
    CHECK( pTable->GetSubElementCount( "field" ) == 3 );
    CHECK_STR( pTable->GetSubElement( "field", 1 )->GetProperty( "type" ), "varchar(40)" );
    CHECK( pTable->GetSubElement( "field", 3 ) == NULL );
    CHECK_STR( pTable->GetChildText( "index" ), "user_id" );
    CHECK( CString( pTable->GetSubElement( "initialize" )->GetData() ).Contains( "insert into user_archive" ) );

        // Writing: the same bytes as the previous version gave (83,408),
        // and output that reads back to the same document.
    CString strRaw = pDocument->toString();
    CString strPretty = pDocument->toString( 4 );
    CHECK( strRaw.GetLength() == 83408 );
    CHECK( strPretty.StartsWith( "<FileInfo name=\"BaseCamp Database Tables\" version=\"3.0.1001\" author=\"Jeffrey R. Naujok\"/>\n<table name=\"user_archive\">\n    <info>\n" ) );
    CXMLDocument *pRaw = parse( (const char *)strRaw );
    CXMLDocument *pPretty = parse( (const char *)strPretty );
    CHECK( pRaw != NULL && pRaw->toString() == strRaw );
    CHECK( pPretty != NULL && pPretty->toString( 4 ) == strPretty );
    delete pRaw;
    delete pPretty;

        // The other ways in.
    CFileStream oStream( new CFile( pszFileName, CFile::READ ) );
    CXMLDocument oFromStream( &oStream );
    CHECK( oFromStream.toString() == strRaw );
    CXMLIndex oIndex( pszFileName );
    CHECK( oIndex.GetElementCount() == 73 && oIndex.GetFileName() == pszFileName );
    CXMLIndex oMissing( "no-such-file.xml" );
    CHECK( oMissing.GetChunkCount() == 0 );
    bool bThrew = false;
    try { delete CXMLParser::parseFile( CString( "no-such-file.xml" ) ); } catch ( CXMLException & ) { bThrew = true; }
    CHECK( bThrew );

    delete pDocument;
}

/***********************************************************************
**  Reading
***********************************************************************/

static void testReading( void )
{
    section( "Reading" );

    CXMLDocument *pDocument = parse(
        "<?xml version='1.0'?>\n"
        "<!DOCTYPE note [\n"
        "  <!ENTITY who \"Irene &amp; Sherlock\">\n"
        "  <!ENTITY greeting \"Hello, &who;!\">\n"
        "  <!-- a comment with ] in it -->\n"
        "  <!ELEMENT note (#PCDATA)>\n"
        "]>\n"
        "<note id=\"n1\" lang='en' tabbed=\"a\tb\" ref=\"a&#10;b\">\n"
        "  <!-- first -->\n"
        "  <to>&lt;Watson&gt; &amp; &quot;Hudson&quot; &apos;</to>\n"
        "  <body>&greeting; &#65;&#x42;&#x20AC;&#x1F600;</body>\n"
        "  <code><![CDATA[if (a < b && c > d) { }]]></code>\n"
        "  <empty/><blank></blank>\n"
        "  <?render fast?>\n"
        "</note>\n" );
    CHECK( pDocument != NULL );
    if ( pDocument == NULL )
        return;

    CXMLIndex *pIndex = pDocument->GetIndex();
    CHECK( pIndex->GetElement( (size_t)0 )->IsMetaTag() );
    CXMLElement *pDoctype = pIndex->GetElement( 1 );
    CHECK( pDoctype->IsDeclaration() && strcmp( pDoctype->GetName(), "DOCTYPE" ) == 0 );
    CHECK( pDoctype->GetRawContent().StartsWith( " note [" ) );

    CXMLElement *pNote = pDocument->GetRoot();
    CHECK_STR( pNote->GetName(), "note" );
    CHECK_STR( pNote->GetProperty( "id" ), "n1" );
    CHECK_STR( pNote->GetProperty( "lang" ), "en" );
    CHECK_STR( pNote->GetProperty( "tabbed" ), "a b" );               // a literal tab becomes a space
    CHECK_STR( pNote->GetProperty( "ref" ), "a\nb" );                 // a reference does not
    CHECK( pNote->GetProperty( "missing" ) == NULL );

    CHECK( pNote->GetSubElement( (size_t)0 )->IsComment() );
    CHECK_STR( pNote->GetSubElement( (size_t)0 )->GetText(), " first " );
    CHECK_STR( pNote->GetChildText( "to" ), "<Watson> & \"Hudson\" '" );
    CHECK_STR( pNote->GetChildText( "body" ), "Hello, Irene & Sherlock! AB\xE2\x82\xAC\xF0\x9F\x98\x80" );
    const CXMLElement *pCode = pNote->GetSubElement( "code" );
    CHECK_STR( pCode->GetText(), "if (a < b && c > d) { }" );
    CHECK( ((CXMLData *)pCode->GetChunk( 0 ))->IsCData() );
    CHECK( pNote->GetSubElement( "empty" )->IsSelfClosing() );
    CHECK( ! pNote->GetSubElement( "blank" )->IsSelfClosing() );
    CHECK( pNote->GetSubElement( "render" )->IsMetaTag() );
    CHECK_STR( pNote->GetSubElement( "render" )->GetRawContent(), " fast" );
    CHECK( pNote->GetSubElement( "render" )->GetPropertyCount() == 0 );  // not attribute-like

        // Writing escapes what needs it, and reads back the same.
    CString strOut = pDocument->toString();
    CHECK( strOut.Contains( "<to>&lt;Watson> &amp; \"Hudson\" '</to>" ) );
    CHECK( strOut.Contains( "tabbed=\"a b\" ref=\"a&#10;b\"" ) );
    CHECK( strOut.Contains( "<![CDATA[if (a < b && c > d) { }]]>" ) );
    CHECK( strOut.Contains( "<empty/><blank></blank>" ) );
    CHECK( strOut.Contains( "<?render fast?>" ) );
    CHECK( strOut.StartsWith( "<?xml version='1.0'?>\n<!DOCTYPE note [" ) );
    CXMLDocument *pAgain = parse( (const char *)strOut );
    CHECK( pAgain != NULL && pAgain->toString() == strOut );
    delete pAgain;
    delete pDocument;

        // Line breaks, the byte-order mark and white space in tags.
    CHECK_STR( rawOf( "<a>x\r\ny\rz</a>" ).c_str(), "<a>x\ny\nz</a>" );
    CHECK_STR( rawOf( "\xEF\xBB\xBF<a/>" ).c_str(), "<a/>" );
    CHECK_STR( rawOf( "<a  x = \"1\"\n  y='2' ></a >" ).c_str(), "<a x=\"1\" y=\"2\"></a>" );   // "</a >" used to fail
    CHECK_STR( rawOf( "<a:b xmlns:a=\"urn:x\" _c.d-e=\"\"/>" ).c_str(), "<a:b xmlns:a=\"urn:x\" _c.d-e=\"\"/>" );
    CHECK_STR( rawOf( "<\xC3\xA9l\xC3\xA9ment/>" ).c_str(), "<\xC3\xA9l\xC3\xA9ment/>" );
    CHECK_STR( rawOf( "<a>]]&gt;</a>" ).c_str(), "<a>]]&gt;</a>" );
    CHECK_STR( rawOf( "<a>&#13;</a>" ).c_str(), "<a>&#13;</a>" );
    CHECK_STR( rawOf( "<a>a > b</a>" ).c_str(), "<a>a > b</a>" );
    CHECK_STR( rawOf( "<a>1<![CDATA[x]]]]><![CDATA[>y]]>2</a>" ).c_str(), "<a>1<![CDATA[x]]]]><![CDATA[>y]]>2</a>" );
    CHECK_STR( rawOf( "" ).c_str(), "" );

        // Several roots are allowed unless the parser is told otherwise.
    CHECK_STR( rawOf( "<a/><b/>" ).c_str(), "<a/><b/>" );
    CHECK( errorOf( "<a/><b/>", true ).find( "second root" ) != std::string::npos );
    CHECK( errorOf( "", true ).find( "no root" ) != std::string::npos );
    CHECK( errorOf( "<a/>x", true ).find( "outside the root" ) != std::string::npos );
    CHECK( errorOf( "<?xml version=\"1.0\"?><a/>", true ) == "" );
}

static void testErrors( void )
{
    section( "Well-formedness errors" );

    struct { const char *pszXML; const char *pszError; } aCases[] =
    {
        { "<a>",                              "line 1, column 1: <a> is never closed" },
        { "<a></b>",                          "line 1, column 4: </b> does not match <a>" },
        { "</a>",                             "line 1, column 1: </a> without a start tag" },
        { "<a>\n  <b>\n</a>",                 "line 3, column 1: </a> does not match <b>" },
        { "<a x=1/>",                         "line 1, column 6: expected a quoted attribute value" },
        { "<a x=\"1\" x=\"2\"/>",             "line 1, column 10: attribute x appears twice" },
        { "<a x=\"1\"y=\"2\"/>",              "line 1, column 9: expected white space before an attribute" },
        { "<a x=\"<\"/>",                     "line 1, column 7: '<' in an attribute value" },
        { "<a x=\"1/>",                       "line 1, column 6: unterminated quoted value" },
        { "<1a/>",                            "line 1, column 2: expected an element name after '<'" },
        { "<a>&nbsp;</a>",                    "line 1, column 10: undefined entity &nbsp;" },
        { "<a>&amp</a>",                      "line 1, column 4: expected ';' after the entity name" },
        { "<a>&#0;</a>",                      "line 1, column 4: character reference to a character XML does not allow" },
        { "<a>&#xD800;</a>",                  "line 1, column 4: character reference to a character XML does not allow" },
        { "<a>&#12a;</a>",                    "line 1, column 4: bad character reference" },
        { "<a>]]></a>",                       "line 1, column 4: ']]>' in text" },
        { "<a><!-- x -- y --></a>",           "line 1, column 11: '--' inside a comment" },
        { "<a><!-- x",                        "line 1, column 4: unterminated comment" },
        { "<a><![CDATA[x</a>",                "line 1, column 4: unterminated CDATA section" },
        { "<![CDATA[x]]>",                    "line 1, column 1: CDATA section outside an element" },
        { "<a/><?xml version=\"1.0\"?>",      "line 1, column 5: the XML declaration must come first, as <?xml ...?>" },
        { "<?xml version=\"1.0\" bad?><a/>",  "line 1, column 1: malformed XML declaration" },
        { "<?pi x",                           "line 1, column 1: unterminated processing instruction" },
        { "<!FOO>",                           "line 1, column 1: unknown markup declaration" },
        { "<a/><!DOCTYPE a>",                 "line 1, column 5: a DOCTYPE must come once, before the root element" },
        { "<a",                               "line 1, column 1: unterminated start tag <a>" },
        { "\xFE\xFF<a/>",                     "line 1, column 1: UTF-16 documents are not supported (only UTF-8)" },
    };
    for ( size_t n = 0; n < sizeof( aCases ) / sizeof( aCases[0] ); n++ )
    {
        std::string strError = errorOf( aCases[n].pszXML );
        g_nChecks++;
        if ( strError != aCases[n].pszError )
        {
            g_nFailures++;
            printf( "  FAILED: [%s] gave [%s], expected [%s]\n", aCases[n].pszXML, strError.c_str(), aCases[n].pszError );
        }
    }

        // Columns count characters, not bytes.
    CHECK_STR( errorOf( "<\xC3\xA9 x=1/>" ).c_str(), "line 1, column 6: expected a quoted attribute value" );

        // A failed read leaves the document as it was.
    CXMLDocument oDocument;
    oDocument.Parse( "<kept/>" );
    bool bThrew = false;
    try { oDocument.Parse( "<broken>" ); } catch ( CXMLException & ) { bThrew = true; }
    CHECK( bThrew );
    CHECK_STR( oDocument.toString(), "<kept/>" );
}

static void testSecurity( void )
{
    section( "External entities and entity expansion" );

        // XXE: an external entity could read a file or a URL.
    CHECK( errorOf( "<!DOCTYPE a [ <!ENTITY x SYSTEM \"file:///etc/passwd\"> ]><a>&x;</a>" ).find( "external entity &x; is not supported" ) != std::string::npos );
    CHECK( errorOf( "<!DOCTYPE a [ <!ENTITY x PUBLIC \"-//x\" \"http://example.com/x\"> ]><a/>" ).find( "external entity" ) != std::string::npos );

        // "Billion laughs": each entity ten of the one before.
    std::string strLaughs = "<!DOCTYPE lolz [ <!ENTITY lol0 \"lol\">";
    for ( int n = 1; n <= 9; n++ )
    {
        strLaughs += " <!ENTITY lol" + std::to_string( n ) + " \"";
        for ( int k = 0; k < 10; k++ )
            strLaughs += "&lol" + std::to_string( n - 1 ) + ";";
        strLaughs += "\">";
    }
    strLaughs += " ]><lolz>&lol9;</lolz>";
    CHECK( errorOf( strLaughs ).find( "entities expand to too much text" ) != std::string::npos );

        // An entity that refers to itself.
    CHECK( errorOf( "<!DOCTYPE a [ <!ENTITY x \"&y;\"> <!ENTITY y \"&x;\"> ]><a>&x;</a>" ).find( "nest too deeply" ) != std::string::npos );

        // Nesting too deep.
    std::string strDeep;
    for ( int n = 0; n < 300; n++ )
        strDeep += "<a>";
    CHECK( errorOf( strDeep ).find( "nest too deeply" ) != std::string::npos );
    CXMLParser oParser;
    oParser.SetMaxDepth( 1000 );
    std::string strClosed = strDeep;
    for ( int n = 0; n < 300; n++ )
        strClosed += "</a>";
    CXMLIndex oIndex;
    oParser.Parse( strClosed.data(), strClosed.size(), oIndex );
    CHECK( oIndex.GetElementCount() == 1 );

        // Markup in an entity's text stays text.
    CXMLDocument *pDocument = parse( "<!DOCTYPE a [ <!ENTITY m \"<b>bold</b>\"> ]><a>&m;</a>" );
    CHECK( pDocument != NULL && pDocument->GetRoot()->GetSubElementCount() == 0 && CString( pDocument->GetRoot()->GetText() ) == "<b>bold</b>" );
    delete pDocument;
}

/***********************************************************************
**  Building and changing documents
***********************************************************************/

static void testBuilding( void )
{
    section( "Building and changing documents" );

    CXMLDocument oDocument;
    CXMLElement *pRoot = new CXMLElement( "catalog" );
    oDocument.GetIndex()->AddElement( pRoot );
    pRoot->SetProperty( "owner", "Irene \"The Woman\" Adler & co. <ltd>" );
    CXMLElement *pBook = new CXMLElement( CString( "book" ), CString( "A Scandal in Bohemia" ) );
    pBook->SetProperty( "year", "1891" );
    pRoot->AddSubElement( pBook );
    pRoot->AddSubElement( new CXMLElement( "empty" ) );
    CXMLElement *pNote = new CXMLElement( "note" );
    pNote->AddData( "x < y & y > z" );
    pRoot->AddSubElement( pNote );
    CXMLElement *pScript = new CXMLElement( "script" );
    pScript->AddData( "a ]]> b", true );
    pRoot->AddSubElement( pScript );

    CHECK_STR( oDocument.toString(),
        "<catalog owner=\"Irene &quot;The Woman&quot; Adler &amp; co. &lt;ltd>\">"
        "<book year=\"1891\">A Scandal in Bohemia</book><empty></empty>"
        "<note>x &lt; y &amp; y > z</note><script><![CDATA[a ]]]]><![CDATA[> b]]></script></catalog>" );
    CHECK_STR( oDocument.toString( 2 ),
        "<catalog owner=\"Irene &quot;The Woman&quot; Adler &amp; co. &lt;ltd>\">\n"
        "  <book year=\"1891\">\n"
        "    A Scandal in Bohemia\n"
        "  </book>\n"
        "  <empty/>\n"
        "  <note>\n"
        "    x &lt; y &amp; y > z\n"
        "  </note>\n"
        "  <script>\n"
        "    <![CDATA[a ]]]]><![CDATA[> b]]>\n"
        "  </script>\n"
        "</catalog>\n" );

        // It reads back as it was built.
    CXMLDocument *pRead = parse( (const char *)oDocument.toString() );
    CHECK( pRead != NULL );
    if ( pRead != NULL )
    {
        CHECK_STR( pRead->GetRoot()->GetProperty( "owner" ), "Irene \"The Woman\" Adler & co. <ltd>" );
        CHECK_STR( pRead->GetRoot()->GetChildText( "script" ), "a ]]> b" );
        CHECK_STR( pRead->GetRoot()->GetChildText( "note" ), "x < y & y > z" );
        delete pRead;
    }

        // Changing it.
    pRoot->SetProperty( "owner", "Mycroft" );
    CHECK( pRoot->GetPropertyCount() == 1 && CString( pRoot->GetProperty( "owner" ) ) == "Mycroft" );
    CHECK( pRoot->DeleteProperty( "owner" ) && ! pRoot->DeleteProperty( "owner" ) );
    CHECK( pRoot->GetSubElementCount() == 4 && pRoot->GetSubElementCount( "book" ) == 1 );
    CHECK( pRoot->DeleteSubElement( 1 ) && pRoot->GetSubElement( "empty" ) == NULL );
    CHECK( ! pRoot->DeleteSubElement( 99 ) );
    pBook->SetData( "The Red-Headed League" );
    CHECK_STR( pBook->GetData(), "The Red-Headed League" );
    CHECK( pBook->GetData( 1 ) == NULL );
    pBook->SetData( "second", 1 );
    CHECK( pBook->GetDataCount() == 2 );
    CHECK_STR( pBook->GetText(), "The Red-Headed Leaguesecond" );

        // A copy is independent.
    CXMLElement *pCopy = (CXMLElement *)pRoot->Clone();
    pCopy->GetSubElement( "book" )->SetProperty( "year", "1892" );
    CHECK_STR( pBook->GetProperty( "year" ), "1891" );
    CHECK( CString( pCopy->toString() ).Contains( "year=\"1892\"" ) );
    delete pCopy;

        // A meta tag changed by its properties is written from them.
    CXMLDocument *pDeclared = parse( "<?xml version=\"1.0\"?><a/>" );
    CXMLElement *pDeclaration = pDeclared->GetIndex()->GetElement( (size_t)0 );
    pDeclaration->SetProperty( "encoding", "UTF-8" );
    CHECK_STR( pDeclared->toString(), "<?xml version=\"1.0\" encoding=\"UTF-8\"?><a/>" );
    delete pDeclared;

        // Writing to a stream.
    CStringStream oOut;
    size_t nWritten = oDocument.Write( &oOut );
    CHECK( nWritten == oDocument.toString().GetLength() );
}

/***********************************************************************
**  Fuzzing
***********************************************************************/

static void testFuzz( void )
{
    section( "Fuzzing" );

    const std::string strSeed =
        "<?xml version=\"1.0\"?><!DOCTYPE r [<!ENTITY e \"E&amp;\">]>"
        "<r a=\"1\" b='&lt;&#65;'><!-- c --><x>t&e;&#x42;</x><![CDATA[<d>]]><?p q?><y/></r>";
    const char *apszPieces[] = { "<", ">", "/", "&", ";", "\"", "'", "=", "]]>", "<!--", "-->", "<![CDATA[", "&#", "<?", "?>", "<!", "\n", " ", "x", "&e;" };

    std::mt19937 rng( 42 );
    int nParsed = 0, nStable = 0;
    for ( int n = 0; n < 20000; n++ )
    {
        std::string strXML = strSeed;
        int nEdits = 1 + (int)( rng() % 4 );
        for ( int e = 0; e < nEdits; e++ )
        {
            size_t nAt = rng() % ( strXML.size() + 1 );
            switch ( rng() % 3 )
            {
                case 0: if ( nAt < strXML.size() ) strXML.erase( nAt, 1 + rng() % 3 ); break;
                case 1: strXML.insert( nAt, apszPieces[ rng() % ( sizeof( apszPieces ) / sizeof( apszPieces[0] ) ) ] ); break;
                default: if ( nAt < strXML.size() ) strXML[nAt] = (char)( rng() % 256 ); break;
            }
        }
        if ( strXML.find( '\0' ) != std::string::npos )
            continue;

        CXMLDocument *pDocument = parse( strXML );
        if ( pDocument == NULL )
            continue;
        nParsed++;
            // Whatever parses must write out text that parses to the same.
        CString strOut = pDocument->toString();
        CXMLDocument *pAgain = parse( (const char *)strOut );
        if ( pAgain != NULL && pAgain->toString() == strOut )
            nStable++;
        else
            printf( "  unstable: [%s] -> [%s]\n", strXML.c_str(), (const char *)strOut );
        delete pAgain;
        delete pDocument;
    }
    CHECK( nParsed > 100 );
    CHECK( nStable == nParsed );
    printf( "  %d of 20000 mutated documents parsed; all wrote back stably\n", nParsed );
}

int main( int argc, char *argv[] )
{
    testMemoryStream();
    testFile( ( argc > 1 ) ? argv[1] : "./DatabaseLayout.xml" );
    testReading();
    testErrors();
    testSecurity();
    testBuilding();
    testFuzz();

    printf( "\n%d checks, %d failures\n", g_nChecks, g_nFailures );
    return ( g_nFailures == 0 ) ? 0 : 1;
}
