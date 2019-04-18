// StaticObj : Tests GCC support of static object initialization.
//

#include "stdafx.h"
#include <iostream>
using namespace IASLib;
using namespace std;

class CFactory : public CObject
{
   public:
      static CList    m_aLinkedList;
   
                      CFactory();

      virtual        ~CFactory();

      static void     Register( CObject *pObj )
      {
         m_aLinkedList.AddFirst( pObj );
      }

      static void    Dump( void )
      {
         for ( size_t nCount = 0; nCount < m_aLinkedList.GetCount() ; nCount++ )
         {
            cout << "Got Element #" << nCount + 1 << endl;
         }
      }
};

CList CFactory::m_aLinkedList;

class CFactoryStub : public CObject
{
   private:
         CString  m_strName;
   public:
         CFactoryStub( const char *strName )
         {
            m_strName = strName;
            CFactory::Register( this );
         }

         virtual ~CFactoryStub( void )
         {
         }
};


static CFactoryStub myStub1( "Stub 1" );

static CFactoryStub myStub2( "Stub 2" );



    
   

#ifdef IASLIB_WIN32__
int _tmain(int argc, _TCHAR* argv[])
#else
int main( int argc, char *argv[] )
#endif
{
   CFactory::Dump();

	return 0;
}

