/**
 * SIP Handler class
 *
 * This class handles a particular request type for a SIP server.
 *
 * Author: Jeffrey R. Naujok
 * Created: May 20, 2019
 */

#ifdef IASLIB_NETWORKING__

#ifndef IASLIB_SIPHANDLER_H__
#define IASLIB_SIPHANDLER_H__

#include "NetworkServices/SIP/SipRequest.h"
#include "NetworkServices/SIP/SipResponse.h"
#include "BaseTypes/Uuid.h"

namespace IASLib
{
    #define IASLIB_SET_SIP_HANDLER( className, methodName )  \
                                                    public: \
                                                        virtual CString getMethod( void ) { return CString( methodName ); } \
                                                        CSipHandler *generateHandler( CString uri, CString version ) \
                                                        { \
                                                            return new className( uri, version ); \
                                                        }

                                    
    #define IASLIB_REGISTER_SIP_HANDLER(className)  \
                                        static className internalHandler( #className, #className ); \
                                        static RegisterSipHandlerStub registerStub( &internalHandler );

    class CSipHandler : public CObject
    {
        protected:
            CString m_strUri;
            CString m_strVersion;
            CUUID erid;
            static size_t handlerCount;
            static CSipHandler *handlers[128];
            static CHash   m_handlerMap;
            static bool    m_initialized;

        public:
            DEFINE_OBJECT( CSipHandler );

            CSipHandler( CString uri, CString version );

            virtual ~CSipHandler( void );

            virtual bool process( CSipRequest *request, CSipResponse *response ) = 0;

            static CSipHandler *getHandler( CSipRequest *request, CUUID erid );
            static void registerHandler( CSipHandler *handlerFactory );
            static bool isValidHandler( const char *method ); 

            virtual CString getMethod( void ) = 0;

            virtual void setErid( CUUID erid ) { this->erid = erid; }
            virtual CUUID getErid( void ) { return erid; }

            static CString getAllowedList( void );

            virtual CSipHandler *generateHandler( CString uri, CString version ) = 0;

        private:
            static void initHash( void );
    };

    class RegisterSipHandlerStub
    {
        private:
            CSipHandler *handler;
        public: 
            RegisterSipHandlerStub( CSipHandler *handlerFactory )
            {
                handler = handlerFactory;
                CSipHandler::registerHandler( handler ); 
            } 
    }; 
} // namespace IASLib

#endif // IASLIB_SIPHANDLER_H__

#endif // IASLIB_NETWORKING__

