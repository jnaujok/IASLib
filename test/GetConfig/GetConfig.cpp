// GetConfig.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace IASLib;
using namespace std;

#ifdef IASLIB_WIN32__
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char *argv[] )
#endif
{
   CString strFileName;
   CString strTarget;
   CString strValue;

   if ( argc > 2 )
   {
       strFileName = argv[1];
       strTarget = argv[2];
   }
   else
   {
      cerr << "Usage: GetConfig <MI Config File.xml> <Name of field to recover>" << endl;
      return -1; 
   }
   try
   { 
      CFile oFile( strFileName, CFile::READ );

      while ( ( strValue.GetLength() == 0 ) && ( ! oFile.IsEOF() ) )
      {
         CString strLine = oFile.GetLine();
         strLine.Trim();
            // All lines that start with '#' are comments
         if ( strLine[0] != '#' )
         {
            if ( strLine.IndexOf( strTarget ) != IASLib::NOT_FOUND )
            {
               size_t nPos = strLine.IndexOf( strTarget ) + strTarget.GetLength();
               CString strTemp = strLine.Substring( nPos );
               strTemp.Trim();
               if ( strTemp[0] == ':' )
               {
                  // We have our value
                  strValue = strTemp.Substring( 1 );
                  strValue.Trim();
               }
            }
         }
      }

      // Output the values in the correct format for use with the JMS Loader program
      cout << strValue << endl;
   }
   catch (CException &oE )
   {
      std::cerr << oE.DumpStack() << std::endl;
   }

	return 0;
}

