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
 *      Starting: a thread created with bStartSuspended runs when Resume()
 * (or Start()) is called, from any thread. A thread created without it
 * starts as the CThread constructor ends, which is before a derived
 * class's constructor has run; so derived classes should start suspended
 * and call Resume() at the end of their own constructor.
 *
 *      Ending: Run() returns (or ExitThread() is called from the thread).
 * Join() waits for that and gives the return code; it may be called after
 * the thread has finished. Detached threads cannot be joined, and the
 * thread monitor deletes them when they finish (unless SetNoDelete()).
 * Destroying a CThread whose thread is still running kills it (Kill()),
 * which leaves whatever it held (locks, memory) as it was: ask it to stop
 * with RequestShutdown() and Join() it first.
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

#include "ThreadPlatform.h"

#ifdef IASLIB_MULTI_THREADED__

    #ifdef IASLIB_WIN32__
        #include "../BaseTypes/Platform.h"
        #define THREAD_T HANDLE
    #else
        #ifdef IASLIB_PTHREAD__
            #include <pthread.h>
            #define THREAD_T pthread_t
        #else
            #error No Thread type defined, but multi-threading is active!
        #endif
    #endif

    #include "../BaseTypes/Object.h"
    #include "../BaseTypes/IASLib_String.h"
    #include "Mutex.h"
    #include <atomic>
    #include <condition_variable>
    #include <mutex>

    namespace IASLib
    {
        class CThread : public CObject
        {
            protected:
                THREAD_T            m_ptThreadID;
        #ifdef IASLIB_WIN32__
                DWORD               m_dwThreadID;       // GetCurrentThread() is not unique
        #endif
                std::atomic<bool>   m_bIsRunning;       // Shared with the thread itself.
                std::atomic<int>    m_nReturnCode;
                bool                m_bDetached;
                std::atomic<bool>   m_bShutdown;        // Set by other threads.
                bool                m_bContained;       // We created the thread
                bool                m_bNoDelete;
                bool                m_bDaemon;
                CString             m_strThreadName;

                    // The gate the new thread waits at until it is started.
                std::mutex          m_mutexGate;
                std::condition_variable m_cvGate;
                bool                m_bReleased;
                bool                m_bAbandoned;       // destroyed before it started
                bool                m_bLeftGate;
                    // Whether the thread has been joined (Join() can be called
                    // again, and after the thread has ended).
                std::mutex          m_mutexJoin;
                bool                m_bReaped;

        #ifdef IASLIB_PTHREAD__
                pthread_attr_t      m_stAttributes;
        #endif
                static CMutex       m_mutexCriticalSection;
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

                    // Creates the thread (see above for when it starts).
                                        CThread( const char *strThreadName = "Unknown Thread", bool bStartSuspended = false, bool bDetached = false, bool bNoDelete = false, bool bDaemon = false );
                    // Stands for a thread that already exists (the calling
                    // thread, if ptThreadID is 0); it is not joined, killed or
                    // started through this object.
                                        CThread( THREAD_T ptThreadID, const char *strThreadName = "Unknown Thread" );
                virtual                ~CThread( void );

                                        DECLARE_OBJECT(CThread,CObject);

                    // What the thread does; the value is ignored (set the
                    // return code with SetReturnCode() or ExitThread()).
                virtual void           *Run( void );

                    // Ends the thread at its next safe point: a Millisleep(),
                    // Sleep() or CSemaphore wait (and, outside Windows, the
                    // system's other cancellation points). 0 once it has
                    // ended; -1 if it has not reached one within two seconds
                    // (it still ends when it does). A thread that never waits
                    // can only be stopped by its own code: see
                    // RequestShutdown(). (On Windows this was TerminateThread(),
                    // which can stop a thread while it holds a lock - the heap's,
                    // the loader's - so that the whole process then hangs.)
                virtual int             Kill( void );

                    // Waits for the thread to end, returning its return code
                    // (0 for a detached thread, or one not created here).
                virtual int             Join( void );

                virtual bool            Detach( void );

                    // Resume() starts a suspended thread; on Windows it also
                    // resumes one Suspend() stopped (Suspend() does nothing
                    // elsewhere: POSIX threads cannot be suspended).
                void                    Suspend( void );
                void                    Resume( void );
                void                    Start( void ) { Resume(); }
                bool                    IsStarted( void );

                THREAD_T                GetThreadID( void ) const { return m_ptThreadID; }
                int                     GetReturnCode( void ) const { return m_nReturnCode; }
                void                    SetReturnCode( int nReturnCode ) { m_nReturnCode = nReturnCode; }
                bool                    IsRunning( void ) const { return m_bIsRunning; }
                void                    SetRunning( bool bRunning ) { m_bIsRunning = bRunning; }

                const char             *GetName( void ) const { return (const char *)m_strThreadName; }

                bool                    IsDetached( void ) const { return m_bDetached; }
                bool                    IsContained( void ) const { return m_bContained; }

                void                    SetNoDelete( bool bNoDelete ) { m_bNoDelete = bNoDelete; }
                bool                    IsNoDelete( void ) const { return m_bNoDelete; }

                bool                    IsDaemon( void ) const { return m_bDaemon; }

                    // The thread monitor kills a thread still running nSeconds
                    // after it started.
                void                    SetTimeout( int nSeconds );

                virtual void            RequestShutdown( void ) { m_bShutdown = true; }
                bool                    IsShutdown( void ) const { return m_bShutdown; }

                    // The CThread for the calling thread (NULL if it has none).
                static CThread         *Self( void );

                static bool             Equals( THREAD_T ptThread1, THREAD_T ptThread2 );

                    // Sleeps (a point at which Kill() can end the thread).
                static void             Sleep( int nSeconds );
                static void             Millisleep( int milliseconds );

                virtual unsigned long   GetCapabilities( void );

                static void             EnterCriticalSection( void ) { m_mutexCriticalSection.Lock(); }
                static void             LeaveCriticalSection( void ) { m_mutexCriticalSection.Unlock(); }

                    // Runs the thread (the platform's start routine calls it).
                void                    ThreadMain( void );

            protected:
                    // Ends the calling thread, which must be this one, with a
                    // return code.
                void                    ExitThread( int nExitCode );

            private:
                void                    Release( void );
                                        CThread( const CThread & );
                CThread                &operator =( const CThread & );
        };

    } // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREAD_H__
