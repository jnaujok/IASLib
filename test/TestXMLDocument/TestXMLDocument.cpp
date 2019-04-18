// TestXMLDocument.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace IASLib;

void DumpSubElements( const CXMLElement *pTag, int nDepth )
{
   int nPad = 0;
   while ( nPad < nDepth )
   {
      std::cout << " ";
      nPad++;
   }

   std::cout << "<" << pTag->GetName();
   int nProps = 0;
   while ( nProps < pTag->GetPropertyCount() )
   {
      CXMLProperty *pProp = pTag->GetProperty( nProps );

      std::cout << " " << pProp->GetName() << "=\"" << pProp->GetValue() << "\"";
      nProps++;
   }
   std::cout << ">"; 
   
   int nCount = 0;
   while ( nCount < pTag->GetSubElementCount() )
   {
      const CXMLElement *pDump = pTag->GetSubElement( nCount );
      if ( pDump )
      {
         DumpSubElements( pDump, nDepth + 3 );
      }
      nCount++;
   }

   int nData = 0;
   while ( nData < pTag->GetDataCount() )
   {
      std::cout << pTag->GetData( nData );
      nData++;
   }


   std::cout << "</" << pTag->GetName() << ">" << std::endl;
}


#ifdef IASLIB_WIN32__
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char *argv[] )
#endif
{

/*
    try
    {
        std::cout << "TESTING XMLDocument from In-Memory Stream" << std::endl;

        CString strXML = "<?xml version=1.0 encoding=UTF-8?><?xml-stylesheet type='text/xsl' href='_UpgradeReport_Files/UpgradeReport.xslt'?>\n"
                         "<UpgradeLog>\n<Properties>\n<Property Name=\"Solution\" Value=\"BaseCamp2\">"
                         "</Property>\n</Properties>\n</UpgradeLog>\n<Dummy/>\n";
        CStringStream osInput1( strXML );

        CXMLDocument xDoc( &osInput1 );

        CXMLIndex *pIndex = xDoc.GetIndex();

        std::cout << "Index loaded-> " << pIndex->GetElementCount() << " elements found -- Should be 4." << std::endl;

        CXMLElement  *pTag;
        size_t       nCount = 0;

        while ( nCount < pIndex->GetElementCount() )
        {
            pTag = pIndex->GetElement( nCount );
            if ( pTag )
            {
               DumpSubElements( pTag, 0 );
            }
            nCount++;
        }
        std::cout << "TESTING COMPLETE" << std::endl << std::endl;
    }
    catch (CXMLException &oE )
    {
        std::cout << oE.DumpStack() << std::endl;
    }
*/

    std::cout << "TESTING XMLDocument Class" << std::endl;

    CString strFileName = "DatabaseLayout.xml";
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
        int nSec;
        int nMSec;
        Timer1.GetTotalTime( nSec, nMSec );
        CDate dttEnd;
        printf( "%01d.%03d - Wall Time\n", dttEnd.Elapsed( dttStart ) / 1000, dttEnd.Elapsed( dttStart ) % 1000 );
        printf( "%01d.%06d - CPU Time To Load\n", nSec, nMSec );
        CXMLIndex *pIndex = xDoc.GetIndex();

        std::cout << "Index loaded-> " << pIndex->GetElementCount() << " elements found." << std::endl;
        std::cout.flush();

        CXMLElement  *pTag;
        size_t       nCount = 0;

        while ( nCount < pIndex->GetElementCount() )
        {
            pTag = pIndex->GetElement( nCount );
            if ( pTag )
            {
//               DumpSubElements( pTag, 0 );
            }
            nCount++;
        }
    }
    catch (CXMLException &oE )
    {
        std::cout << oE.DumpStack() << std::endl;
    }

/*
    try
    {
        CDate dttStart;
        CCPUUsage Timer2;
        CXMLIndex oIndex( strFileName );
        int nSec;
        int nMSec;
        Timer2.GetTotalTime( nSec, nMSec );
        CDate dttEnd;
        printf( "%01d.%03d - Wall Time\n", dttEnd.Elapsed( dttStart ) / 1000, dttEnd.Elapsed( dttStart ) % 1000 );
        printf( "%01d.%06d - Elapsed Time To Load\n", nSec, nMSec );

        std::cout << "Index loaded-> " << oIndex.GetElementCount() << " elements found." << std::endl;
        std::cout.flush();
    }
    catch (CXMLException &oE )
    {
        std::cout << oE.DumpStack() << std::endl;
    }
*/
	return 0;
}

