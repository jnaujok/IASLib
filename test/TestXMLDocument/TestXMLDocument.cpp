// TestXMLDocument.cpp : Defines the entry point for the console application.
//

#include "IASLib.h"
#include <iostream>
using namespace IASLib;

bool testMemoryStream() {
    try
    {
        std::cout << "TESTING XMLDocument from In-Memory Stream" << std::endl;

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
        CStringStream osInput1( strXML );

        CDate dttStart;
        CCPUUsage Timer1;
        CXMLDocument xDoc( &osInput1 );
        CCPUUsage Timer2;
        int nSec;
        int nMSec;
        Timer1.TotalElapsed(Timer2, nSec, nMSec );
        CDate dttEnd;
        printf( "%01d.%03d - Wall Time\n", dttEnd.Elapsed( dttStart ) / 1000, dttEnd.Elapsed( dttStart ) % 1000 );
        printf( "%01d.%06d - CPU Time To Load\n", nSec, nMSec );

        CXMLIndex *pIndex = xDoc.GetIndex();

        // -- FORMATTED OUTPUT --
        ASSERT( pIndex->GetElementCount() == 4 );
        auto outDoc = xDoc.toString( 4 );
        ASSERT( outDoc == strCompareIndent );

        // -- ORIGINAL FORMAT --
        outDoc = xDoc.toString();
        ASSERT( outDoc == strXML );

        std::cout << "Memory Stream Testing complete." << std::endl << std::endl;
    }
    catch (CXMLException &oE )
    {
        std::cout << oE.DumpStack() << std::endl;
    }
}

bool testFileStream( int argc, char *argv[] ) {
    std::cout << "TESTING XMLDocument Class" << std::endl;

    CString strFileName = "./DatabaseLayout.xml";
    if ( argc > 1 )
    {
        strFileName = argv[1];
    }

    CFile  *pFile1 = new CFile( strFileName, CFile::READ );
    CFileStream osInput2( pFile1 );

    try
    {
        CDate dttStart;
        CCPUUsage Timer1;
        CXMLDocument xDoc( &osInput2 );
        CCPUUsage Timer2;
        int nSec;
        int nMSec;
        Timer1.TotalElapsed(Timer2, nSec, nMSec );
        CDate dttEnd;
        printf( "%01d.%03d - Wall Time\n", dttEnd.Elapsed( dttStart ) / 1000, dttEnd.Elapsed( dttStart ) % 1000 );
        printf( "%01d.%06d - CPU Time To Load\n", nSec, nMSec );
        CXMLIndex *pIndex = xDoc.GetIndex();

        ASSERT( pIndex->GetElementCount() == 73 );

        auto outDoc = xDoc.toString( 4 );
        ASSERT( outDoc.GetLength() == 83486 );
        ASSERT( outDoc.hashcode() == 482108729 );

        outDoc = xDoc.toString();
        ASSERT( outDoc.GetLength() == 83408 );
        ASSERT( outDoc.hashcode() == 1554375035 );
    }
    catch (CXMLException &oE )
    {
        std::cout << oE.DumpStack() << std::endl;
    }
}

#ifdef IASLIB_WIN32__
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char *argv[] )
#endif
{
    testMemoryStream();
    testFileStream(argc,argv);
	return 0;
}

