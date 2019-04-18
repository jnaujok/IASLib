// GetQueues.cpp : Defines the entry point for the console application.
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

   if ( argc > 1 )
   {
       strFileName = argv[1];
   }
   else
   {
      cerr << "Usage: GetQueues <MI Config File.xml>" << endl;
      return -1; 
   }
   
   CFile  *pFile1 = new CFile( strFileName, CFile::READ );
   CFileStream osInput2( pFile1 );

   try
   {
      CXMLDocument         xDoc( &osInput2 );
      CXMLIndex           *pIndex = xDoc.GetIndex();
      const CXMLElement   *pTag;
      CString              strDestName;
      CString              strFactoryName;
      CString              strLdapName;
      CString              strDestinationQueue;
 
      pTag = pIndex->GetElement( "MIConfiguration-data" );

      if ( pTag == NULL )
      {
         cerr << "Base configuration data not found." << endl; cerr.flush();
         return -1;
      }

      pTag = pTag->GetSubElement( "MIAppInfo" );

      if ( pTag == NULL )
      {
         cerr << "Base configuration data not found." << endl; cerr.flush();
         return -1;
      }

      size_t nCount = 0;

      // We look for the first input queue
      const CXMLElement *pDestInfo = NULL;
      do
      {
         pDestInfo = pTag->GetSubElement( "DestInfo", nCount );

         if ( pDestInfo )
         {
            CString strAttrib = pDestInfo->GetProperty( "IsConsumer" );
            if ( strAttrib == "TRUE" )
            {
               strDestName = pDestInfo->GetProperty( "Name" );
            }
         }
         nCount++;
      } while ( ( pDestInfo ) && ( strDestName.GetLength() == 0 ) );

      if ( ! pDestInfo )
      {
         cerr << "Could not find destination input queue in " << strFileName << endl;
         return -1;
      }
      else
      {
         //cerr << "DestInfo = [" << strDestName << "]" << endl; cerr.flush();
      }

      // Now we look for the factory and queue information for the determined 
      // input queue
      nCount = 0;
      const CXMLElement *pConnFactory = NULL;
      do
      {
         pConnFactory = pTag->GetSubElement( "ConnFactoryInfo", nCount );

         if ( pConnFactory )
         {
            size_t nDest = 0;
            const CXMLElement *pDestInfo = NULL;
            do
            {
               pDestInfo = pConnFactory->GetSubElement( "DestInfo", nDest );
               
               if ( pDestInfo )
               {
                  CString strAttrib = pDestInfo->GetProperty( "default" );
                  if ( strAttrib == strDestName )
                  {
                     strFactoryName = pConnFactory->GetProperty( "Name" );
                     strLdapName = pConnFactory->GetProperty( "Context" );
                     strDestinationQueue = pDestInfo->GetProperty( "Name" );
                  }
               }
               nDest++;
            } while ( ( pDestInfo != NULL ) && ( strDestinationQueue.GetLength() == 0 ) && ( strFactoryName.GetLength() == 0 ) );
         }
         nCount++;
      } while ( ( pConnFactory != NULL ) && ( strFactoryName.GetLength() == 0 ) );

      if ( strFactoryName.GetLength() == 0 )
      {
         // We still haven't hit paydirt, we need to see if there's a ConnMgr group and
         // iterate through those.
         const CXMLElement *pConnMgr = NULL;
         size_t nMgr = 0;

         do
         {
            pConnMgr = pTag->GetSubElement( "ConnMgrInfo", nMgr );
            if ( pConnMgr )
            {
               nCount = 0;
               const CXMLElement *pConnFactory = NULL;
               do
               {
                  pConnFactory = pConnMgr->GetSubElement( "ConnFactoryInfo", nCount );

                  if ( pConnFactory )
                  {
                     size_t nDest = 0;
                     const CXMLElement *pDestInfo = NULL;
                     do
                     {
                        pDestInfo = pConnFactory->GetSubElement( "DestInfo", nDest );

                        if ( pDestInfo )
                        {
                           CString strAttrib = pDestInfo->GetProperty( "default" );
                           if ( strAttrib == strDestName )
                           {
                              strFactoryName = pConnFactory->GetProperty( "Name" );
                              strLdapName = pConnFactory->GetProperty( "Context" );
                              strDestinationQueue = pDestInfo->GetProperty( "Name" );
                           }
                        }
                        nDest++;
                     } while ( ( pDestInfo != NULL ) && ( strDestinationQueue.GetLength() == 0 ) && ( strFactoryName.GetLength() == 0 ) );
                  }
                  nCount++;
               } while ( ( pConnFactory != NULL ) && ( strFactoryName.GetLength() == 0 ) );
            }
            nMgr++;
         } while ( ( pConnMgr ) && ( strFactoryName.GetLength() == 0 ) );
      }

         // If we haven't found our answer, then we need to dig into the environment information      
      if ( strFactoryName.GetLength() == 0 )
      {
         const CXMLElement *pEnv = NULL;
         int nEnv = 0;
         do
         {
            pEnv = pTag->GetSubElement( "EnvInfo", nEnv );
            if ( pEnv )
            {
               CString strName = pEnv->GetProperty( "Name" );
         
               if ( strName == "DEFAULT" )
               {
                  // Now we look for the factory and queue information for the determined
                  // input queue
                  nCount = 0;
                  const CXMLElement *pConnFactory = NULL;
                  do
                  {
                     pConnFactory = pEnv->GetSubElement( "ConnFactoryInfo", nCount );
            
                     if ( pConnFactory )
                     {
                        size_t nDest = 0;
                        const CXMLElement *pDestInfo = NULL;
                        do
                        {
                           pDestInfo = pConnFactory->GetSubElement( "DestInfo", nDest );
      
                           if ( pDestInfo )
                           {
                              CString strAttrib = pDestInfo->GetProperty( "default" );
                              if ( strAttrib == strDestName )
                              {
                                 strFactoryName = pConnFactory->GetProperty( "Name" );
                                 strLdapName = pConnFactory->GetProperty( "Context" );
                                 strDestinationQueue = pDestInfo->GetProperty( "Name" );
                              }
                           }
                           nDest++;
                        } while ( ( pDestInfo != NULL ) && ( strDestinationQueue.GetLength() == 0 ) && ( strFactoryName.GetLength() == 0 ) );
                     }
                     nCount++;
                  } while ( ( pConnFactory != NULL ) && ( strFactoryName.GetLength() == 0 ) );
               }
            }
            nEnv++;
         } while ( ( pEnv ) && ( strFactoryName.GetLength() == 0 ) );

         if ( strFactoryName.GetLength() == 0 )
         {
            // We still haven't hit paydirt, we need to see if there's a ConnMgr group and 
            // iterate through those.
            nEnv = 0;
            do
            {
               pEnv = pTag->GetSubElement( "EnvInfo", nEnv );
               if ( pEnv )
               {
                  CString strName = pEnv->GetProperty( "Name" );

                  if ( strName == "DEFAULT" )
                  {
                     const CXMLElement *pConnMgr = NULL;
                     size_t nMgr = 0;

                     do
                     {
                        pConnMgr = pEnv->GetSubElement( "ConnMgrInfo", nMgr );
                        if ( pConnMgr )
                        {
                           nCount = 0;
                           const CXMLElement *pConnFactory = NULL;
                           do
                           {
                              pConnFactory = pConnMgr->GetSubElement( "ConnFactoryInfo", nCount );
         
                              if ( pConnFactory )
                              {
                                 size_t nDest = 0;
                                 const CXMLElement *pDestInfo = NULL;
                                 do
                                 {
                                    pDestInfo = pConnFactory->GetSubElement( "DestInfo", nDest );
   
                                    if ( pDestInfo )
                                    {
                                       CString strAttrib = pDestInfo->GetProperty( "default" );
                                       if ( strAttrib == strDestName )
                                       {
                                          strFactoryName = pConnFactory->GetProperty( "Name" );
                                          strLdapName = pConnFactory->GetProperty( "Context" );
                                          strDestinationQueue = pDestInfo->GetProperty( "Name" );
                                       }
                                    }
                                    nDest++;
                                 } while ( ( pDestInfo != NULL ) && ( strDestinationQueue.GetLength() == 0 ) && ( strFactoryName.GetLength() == 0 ) );
                              }
                              nCount++;
                           } while ( ( pConnFactory != NULL ) && ( strFactoryName.GetLength() == 0 ) );
                        }
                        nMgr++;
                     } while ( ( pConnMgr ) && ( strFactoryName.GetLength() == 0 ) );
                  }
               }
               nEnv++;
            } while ( ( pEnv ) && ( strFactoryName.GetLength() == 0 ) );
         }
      }

      // Output the values in the correct format for use with the JMS Loader program
      if ( argc >= 3 )
      {
         CString strArg = argv[2];
         strArg.ToUpperCase();
         strArg.Trim();
         
         if ( strArg[1] == 'L' )
         {
            cout << strLdapName << endl; cout.flush();
         }
         else
         {
            if ( strArg[1] == 'F' )
            {
               cout << strFactoryName << endl; cout.flush();
            }
            else
            {
               if ( strArg[1] == 'D' )
               {
                  cout << strDestinationQueue << endl; cout.flush();
               }
            }
         }
      }
      else
      {
         cout << "-l \"" << strLdapName << "\" -f \"" << strFactoryName << "\" -d \"" << strDestinationQueue << "\"" << endl;
      }
   }
   catch (CXMLException &oE )
   {
      std::cout << oE.DumpStack() << std::endl;
   }

	return 0;
}

