// TestXMLDocument.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
using namespace IASLib;
using namespace std;

   // If this ever gets moved, we're screwed.
CString strTagFileName = "/home/devtestrepo/usr/local/lib/python2.4/site-packages/asn_tags.dat";

class CAsnTag : public CObject
{
   protected:
      int      m_nTagNum;
      CString  m_strName;
      CString  m_strSynonym;
      CArray   m_aSubTags;
      bool     m_bIsGroup;
   public:
               CAsnTag( int nNum, const char *strName )
               {
                  m_nTagNum = nNum;
                  m_strName = strName;
                  m_bIsGroup = false;
               }
      virtual ~CAsnTag( void ) 
               {
                   m_aSubTags.EmptyAll(); 
               }
      
               DECLARE_OBJECT( CAsnTag, CObject );

      int      getNumber( void ) { return m_nTagNum; }
      CString  getName( void ) { return m_strName; }
      bool     isGroup( void ) { return m_bIsGroup; }
      void     setSynonym( CString strSyn ) { m_strSynonym = strSyn; }
      CString  getSynonym( void ) { return m_strSynonym; }

      int      addSubTag( CAsnTag *pTag )
               {
                  m_aSubTags.Push( pTag );
                  m_bIsGroup = true;
                  return (int)m_aSubTags.GetCount();
               }

      CAsnTag *getLastTag( void )
               {
                  if ( m_aSubTags.GetCount() > 0 )
                  {
                     return (CAsnTag *)m_aSubTags.Get( m_aSubTags.GetCount() -1 );
                  }
                  return NULL;
               }

      CAsnTag *findSubTag( const char *strName )
               {
                  for( int nCount = 0; nCount < m_aSubTags.GetCount(); nCount++ )
                  {
                     CAsnTag *pTag = (CAsnTag *)m_aSubTags.Get( nCount );
                     if ( ( pTag->getName() == strName ) || ( pTag->getSynonym() == strName ) )
                     {
                        return pTag;
                     }
                  }
                  return NULL;
               }

      CString  asBERTag( int nIndent, CString strValue )
               {
                  CString strTemp;
                  CString strNumber;
                  
                  strTemp = strTemp.Pad( nIndent, ' ' );
                  strTemp += m_strName;
                  strTemp = strTemp.Pad( 54 + nIndent, ' ' );
                  strNumber.Format( "[%4d]", m_nTagNum );
                  strTemp += strNumber;
                  strTemp += " \"";
                  strTemp += strValue;
                  strTemp += "\"";
                  
                  return strTemp;
               }
};
                  
CArray g_ApplicationTags;

bool BuildTagLibrary( void )
{
   CFile oFile( strTagFileName, CFile::READ );
   CAsnTag *pCurrentTag;
   CAsnTag *aTags[128];
   int nFileLine = 0;
   int nValidTags = 0;
   int nInvalidTags = 0;

   memset( aTags, 0, sizeof( aTags ) );

   while ( ! oFile.IsEOF() )
   {
      CString strLine = oFile.GetLine();
      strLine.Trim();
      nFileLine++;

         // Break up the value and the name
      CStringTokenizer stTokens( strLine, "|", false );
      if ( stTokens.CountTokens() < 2 )
      {
            // This isn't a valid line -- bail.
         nInvalidTags++;
         continue;
      }

         // Pull the number and the value out of the line
      CString strDepth = stTokens.NextToken();
      CString strName = stTokens.NextToken();

         // Tokenize the number
      CStringTokenizer stCrackNew( strDepth, "." );
      int nTokens = stCrackNew.CountTokens();

      int nStep;
      pCurrentTag = NULL;

         // Find the correct level to add the tag.
      for ( nStep = 0; nStep < ( nTokens - 1 ); nStep++ )
      {
         CString nCracked = stCrackNew.NextToken();
         pCurrentTag = aTags[ nStep ];
      }
      
      int nNumber = atoi( stCrackNew.NextToken() );

      if ( pCurrentTag )
      {
         CAsnTag *pLast = pCurrentTag->getLastTag();
         if ( ( pLast ) && ( pLast->getNumber() == nNumber ) )
         {
            pLast->setSynonym( strName );
            aTags[ nStep ] = pLast;
            nValidTags++;
            continue;
         }
      }

         // Create our new tag.
      CAsnTag *pNew = new CAsnTag( nNumber, strName );

      if ( pCurrentTag == NULL )
      {
         g_ApplicationTags.Push( pNew );
      }
      else
      {
         pCurrentTag->addSubTag( pNew );
      }

      aTags[ nStep ] = pNew;
      nValidTags++;
   }

   cerr << "Read " << nFileLine << " ASN Tags -- Valid[" << nValidTags << "] Invalid[" << nInvalidTags << "]" << endl;

   return true;
}


void DumpSubElements( const CXMLElement *pTag, int nIndent, CAsnTag *pCurrentTag )
{
   CAsnTag *pWorkingTag = NULL;

   if ( pCurrentTag == NULL )
   {
      for ( int nApp = 0; nApp < g_ApplicationTags.GetCount(); nApp++ )
      {
         pWorkingTag = (CAsnTag *)g_ApplicationTags.Get( nApp );
         if ( ( pWorkingTag ) && ( pWorkingTag->getName() == pTag->GetName() ) )
         {
            CString strOutput;
            strOutput = strOutput.Pad( nIndent, ' ' );
            strOutput += pWorkingTag->getName();
            strOutput += " [Application ";
            strOutput += pWorkingTag->getNumber();
            strOutput += "]";
            cout << strOutput << endl;cout.flush();
            strOutput = "";
            strOutput = strOutput.Pad( nIndent, ' ' );
            strOutput += "{";
            cout << strOutput << endl;cout.flush();
            for ( int nSubTags = 0; nSubTags < pTag->GetSubElementCount(); nSubTags++ )
            {
               const CXMLElement *pNext = pTag->GetSubElement( nSubTags );
               if ( pNext )
               {
                  DumpSubElements( pNext, nIndent + 3, pWorkingTag );
               }
            }
            strOutput = "";
            strOutput = strOutput.Pad( nIndent, ' ' );
            strOutput += "}";
            cout << strOutput << endl;
            break;
         }
      }
   }
   else
   {
      pWorkingTag = pCurrentTag->findSubTag( pTag->GetName() );

      if ( pWorkingTag )
      {
         if ( pWorkingTag->isGroup() )
         {
            CString strOutput;
            strOutput = strOutput.Pad( nIndent, ' ' );
            strOutput += pWorkingTag->getName();
            strOutput += " [Context ";
            strOutput += pWorkingTag->getNumber();
            strOutput += "]";
            cout << strOutput << endl;
            strOutput = "";
            strOutput = strOutput.Pad( nIndent, ' ' );
            strOutput += "{";
            cout << strOutput << endl;
            for ( int nSubTags = 0; nSubTags < pTag->GetSubElementCount(); nSubTags++ )
            {
               const CXMLElement *pNext = pTag->GetSubElement( nSubTags );
               if ( pNext )
               {
                  DumpSubElements( pNext, nIndent + 3, pWorkingTag );
               }
            }
            strOutput = "";
            strOutput = strOutput.Pad( nIndent, ' ' );
            strOutput += "}";
            cout << strOutput << endl;
         }
         else
         {
            cout << pWorkingTag->asBERTag( nIndent, CString(pTag->GetData()) ) << endl;
         }
      }
   }
}


#ifdef IASLIB_WIN32__
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char *argv[] )
#endif
{
   CString strFileName;
   int nCurrent = 1;

   if ( argc < 2 )
   {
      cerr << "Usage: XER2BER <filename> [<filename2> ...]" << endl; 
      cerr << "Output goes to stdout" << endl;
      cerr.flush();
      return -1;
   }

   if ( ! BuildTagLibrary( ) )
   {
      cerr << "ERROR: Could not build tag definition dictionary." << endl; cerr.flush();
      return -1;
   }

   while ( nCurrent < argc )
   {
      strFileName = argv[ nCurrent ];

      cerr << "Converting file: " << strFileName << endl;cerr.flush();

      CFile *pFile = new CFile( strFileName, CFile::READ );
      CFileStream osInput( pFile );
 
      try
      {
         CXMLDocument xDoc( &osInput );
         CXMLIndex *pIndex = xDoc.GetIndex();

         CXMLElement  *pTag;
         size_t       nCount = 0;

         while ( nCount < pIndex->GetElementCount() )
         {
            pTag = pIndex->GetElement( nCount );
            if ( ( pTag ) && ( ! pTag->IsComment() ) && ( ! pTag->IsMetaTag() ) )
            {
               DumpSubElements( pTag, 0, NULL );
            }
            nCount++;
         }
      }
      catch (CXMLException &oE )
      {
         std::cout << oE.DumpStack() << std::endl;
      }
      nCurrent++;
   }

   g_ApplicationTags.EmptyAll();
   
	return 0;
}


/*
         int nFind = atoi( nCracked );
         if ( ( ! pTemp ) || ( pTemp->getNumber() != nFind ) )
         {
            // We've backed out more than one level. That's a bit scary
            // (i.e. we went from 2.215.0.7 to 2.216.0) This shouldn't
            // ever happen, so we need to stop here and spit out a nasty
            // error.
            cerr << "ERROR: More than one level of tag change in a single line (Line: " 
                 << nFileLine << endl; cerr.flush();
            return false;
         }
         pCurrentTag = pTemp;
*/








