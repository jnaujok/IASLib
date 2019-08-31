/*
 *  Thread Class
 *
 *      This class encapsulates a thread. Creating an object of this
 * class creates a new thread. The new object then begins execution at
 * the virtual Run method of the class. Note that this class isn't
 * really meant to be created directly, since the Run method of this
 * class simply exits the thread.
 *      The real power comes from inheriting from this object and over-
 * riding the Run method. This lets you create unique objects that run
 * as threads.
 *
 *============================================================================
 *  Please Note: Not all systems are equal when it comes to threads, nor
 * is it possible to create such a similitude without sacrificing certain
 * functionality. To determine the functionality of a thread implementation,
 * simply use the "GetCapabilities" to determine if the thread will do
 * what you want.
 *============================================================================
 *
 *  Author: Jeffrey R. Naujok
 *  Created: 7/28/1995
 *  $LOG$
 *
 */

#ifndef IASLIB_THREAD_H__
#define IASLIB_THREAD_H__

#ifdef IASLIB_MULTI_THREADED__

    #ifdef IASLIB_SUN__
        #include <thread.h>
        #include <sched.h>
        #define THREAD_T thread_t
    #else
        #ifdef IASLIB_WIN32__
            #include <windows.h>
            #define THREAD_T HANDLE
        #else
            #ifdef IASLIB_PTHREAD__
                #include <pthread.h>
                #define THREAD_T pthread_t
            #else
                #error No Thread type defined, but multi-threading is active!
            #endif
        #endif
    #endif

    #include "../BaseTypes/Object.h"
    #include "../BaseTypes/IASLib_String.h"
    #include "Mutex.h"

    namespace IASLib
    {
        class CThread : public CObject
        {
            protected:
                THREAD_T        m_ptThreadID;
                bool            m_bIsRunning;
                int             m_nReturnCode;
                bool            m_bDetached;
                bool            m_bShutdown;
                bool            m_bContained;
                bool            m_bNoDelete;
                bool            m_bDaemon;
                CString         m_strThreadName;

        #ifdef IASLIB_PTHREAD__
                pthread_attr_t  m_stAttributes;
            public:
                CMutex          m_mutexStartSuspended;
        #endif
                static CMutex   m_mutexCriticalSection;
            public:
                enum CapabilityFlags {
                                        THREADS           = 0x0001,
                                        SUSPEND           = 0x0002,
                                        RESUME            = 0x0004,
                                        KILL              = 0x0008,
                                        JOIN              = 0x0010,
                                        DETACH            = 0x0020,
                                        DAEMON            = 0x0040,
                                        SLEEP             = 0x0080,
                                        YIELD             = 0x0100,
                                        NAMED             = 0x0200,
                                        STATE             = 0x0400,
                                      };


                                        CThread( const char *strThreadName = "Unknown Thread", bool bStartSuspended = false, bool bDetached = false, bool bNoDelete = false, bool bDaemon = false );
                                        CThread( THREAD_T ptThreadID, const char *strThreadName = "Unknown Thread" );
                virtual                ~CThread( void );

                                        DECLARE_OBJECT(CThread,CObject);

                virtual void           *Run( void );

                virtual int             Kill( void );

                virtual int             Join( void );

                virtual bool            Detach( void );

                void                    Suspend( void );
                void                    Resume( void );

                THREAD_T                GetThreadID( void ) { return m_ptThreadID; }
                int                     GetReturnCode( void ) { return m_nReturnCode; }
                bool                    IsRunning( void ) { return m_bIsRunning; }
                void                    SetRunning( bool bRunning ) { m_bIsRunning = bRunning; }

                const char             *GetName( void ) { return (const char *)m_strThreadName; }

                bool                    IsDetached( void ) { return m_bDetached; }
                bool                    IsContained( void ) { return m_bContained; }

                void                    SetNoDelete( bool bNoDelete ) { m_bNoDelete = bNoDelete; }
                bool                    IsNoDelete( void ) { return m_bNoDelete; }

                bool                    IsDaemon( void ) { return m_bDaemon; }

                void                    SetTimeout( int nSeconds );

                virtual void            RequestShutdown( void ) { m_bShutdown = true; }
                bool                    IsShutdown( void ) { return m_bShutdown; }

                static CThread         *Self( void );

                static bool             Equals( THREAD_T ptThread1, THREAD_T ptThread2 );

                void                    Sleep( int nSeconds );
                void                    Millisleep( int milliseconds );

                virtual unsigned long   GetCapabilities( void ) { return CapabilityFlags::THREADS; };

                static void             EnterCriticalSection( void ) { m_mutexCriticalSection.Lock(); }
                static void             LeaveCriticalSection( void ) { m_mutexCriticalSection.Unlock(); }

            protected:
                void            ExitThread( int nExitCode );
        };

    } // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREAD_H__
