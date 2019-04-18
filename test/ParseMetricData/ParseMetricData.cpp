// ParseMetricData.cpp : Extracts Process Information from metric data.
//

#include "stdafx.h"
#include <iostream>
using namespace IASLib;
using namespace std;

class CMeasurement : public CObject
{
   protected:
      double      m_nMinimum;
      double      m_nMaximum;
      double      m_nAverage;
      double      m_nCount;
      CString     m_strName;
      double      m_nTPSMin;
      double      m_nTPSMax;
      double      m_nTPSAve;

   public:
                  CMeasurement( const char *strName );
      virtual    ~CMeasurement( void ) {}
                    
                  DECLARE_OBJECT(CMeasurement,CObject);

      void        AddMeasurement( double nCount, double nTime, double nTPS );

      CString     getName( void ) { return m_strName; }

      int         getCount( void ) { return (int)m_nCount; }

      double      getMin( void ) { return m_nMinimum; }
      double      getMax( void ) { return m_nMaximum; }
      double      getAve( void ) { return m_nAverage; }

      double      getTPSMin( void ) { return m_nTPSMin; }
      double      getTPSMax( void ) { return m_nTPSMax; }
      double      getTPSAve( void ) { return m_nTPSAve; }
};

CMeasurement::CMeasurement( const char *strName )
{
   m_nMinimum = 1000000.0;
   m_nMaximum = 0.0;
   m_nAverage = 0.0;
   m_nTPSMin = 2000000.0;
   m_nTPSMax = 0.0;
   m_nTPSAve = 0.0;
   m_nCount = 0.0;
   m_strName = strName;
}

void CMeasurement::AddMeasurement( double nCount, double nTime, double nTPS )
{
   if ( nCount == 0.0 )
      return;

   double dVal = nTime/nCount;

   if ( dVal < m_nMinimum )
   {
      m_nMinimum = dVal;
   }
   if ( dVal > m_nMaximum )
   {
      m_nMaximum = dVal;
   }

   if ( nTPS < m_nTPSMin )
   {
      m_nTPSMin = nTPS;
   }

   if ( nTPS > m_nTPSMax )
   {
      m_nTPSMax = nTPS;
   }

   double dTemp = m_nAverage * m_nCount;
   dTemp += nTime * nCount;
   m_nAverage = dTemp / ( m_nCount + nCount );

   dTemp = m_nTPSAve * m_nCount;
   dTemp += nTPS * nCount;
   m_nTPSAve = dTemp / ( m_nCount + nCount );

   m_nCount += nCount;
}

#ifdef IASLIB_WIN32__
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char *argv[] )
#endif
{
   CString strFileName;

   if ( argc == 1 )
   {
      std::cerr << "Usage: ParseMetricData <transaction filename>" << std::endl;
      return -1;
   }
   else
   {
      strFileName = argv[1];
   }


   try
   {
      cerr << "Opening file " << strFileName << endl;

      CFile  oFile( strFileName, CFile::READ );

      CHash  hashMetrics( "MEDIUM" );

      cerr << "Parsing file " << strFileName << endl;

      int nLine = 0;

      while ( ! oFile.IsEOF() )
      {
         CString strLine = oFile.GetLine(); 

         // Skip the header lines
         if ( nLine < 2 )
         {
            nLine++;
            continue;
         }

         CStringTokenizer stTokens( strLine, "|" );

         CString strCount;
         CString strName;
         CString strTPM;
         CString strSeconds;
         CString strWallTime;

         int nStep = 0;
         int nGot = 0;

         while ( stTokens.HasMoreTokens() )
         {
            CString strTemp = stTokens.NextToken();
            strTemp.Trim();
            nStep++;

            switch ( nStep )
            {
               case 8:
                  strSeconds = strTemp;
                  nGot++;
                  break;

               case 14:
                  strCount = strTemp;
                  nGot++;
                  break;

               case 20:
                  strName = strTemp;
                  nGot++;
                  break;

               case 15:
                  strTPM = strTemp;
                  nGot++;
                  break;

               case 16:
                  strWallTime = strTemp;
                  nGot++;
                  break;
   
               default:
                  break;
            }
         }

         if ( nGot == 5 )
         {
            CMeasurement *pMeasure = (CMeasurement *)hashMetrics.Get( strName );

            if ( pMeasure == NULL )
            {
               pMeasure = new CMeasurement( strName );

               hashMetrics.Push( strName, pMeasure );
            }

            double dCount = atof( strCount );
            double dTPS = atof( strTPM ) / 60.0;
            double dWallTime = atof( strWallTime );

            pMeasure->AddMeasurement( dCount, dWallTime, dTPS );
         }       
         nLine++;

         if ( ( nLine % 13 ) == 0 )
         {
            cerr << "\rProcessing Line " << nLine; cerr.flush();
         }
      }

      cerr << endl << endl;

      cerr << "Parsing file " << strFileName << " Complete.  " << nLine << " lines read." << endl;


      size_t nOut = 0;

      while ( nOut < hashMetrics.GetCount() )
      {
         CMeasurement *pMeasure = (CMeasurement *)hashMetrics.Enum( nOut );
         
         if ( ( pMeasure != NULL ) && ( pMeasure->getCount() > 0 ) )
         {
            cout << pMeasure->getName() << ": Count - " << pMeasure->getCount()
                 << "   WallTime - " << pMeasure->getAve() << "(" << pMeasure->getMin() << "/" <<pMeasure->getMax() << ")"
                 << "   TPS - " << pMeasure->getTPSAve() << "(" << pMeasure->getTPSMin() << "/" << pMeasure->getTPSMax() << ")" << endl;
         }
         nOut++;
      }
      hashMetrics.EmptyAll();
   }
   catch (CException &oE )
   {
       std::cerr << oE.DumpStack() << std::endl;
   }

	return 0;
}

