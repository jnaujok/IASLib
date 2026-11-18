/**
 * @file
 * @brief Thread Class.
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
        /// The platform's thread handle type.
        #define THREAD_T HANDLE
    #else
        #ifdef IASLIB_PTHREAD__
            #include <pthread.h>
            /// The platform's thread handle type.
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
        /// A thread of execution: derive from it and override Run().
        ///
        /// Creating a CThread creates the thread, which then runs Run(). A
        /// thread created with @p bStartSuspended runs when Resume() (or
        /// Start()) is called, from any thread. One created without it starts
        /// as the CThread constructor ends, before a derived class's
        /// constructor has run, so derived classes should start suspended and
        /// call Resume() at the end of their own constructor.
        ///
        /// The thread ends when Run() returns, or ExitThread() is called from
        /// it. Join() waits for that and gives the return code. Detached
        /// threads cannot be joined; the thread monitor deletes them when they
        /// finish (unless SetNoDelete()). Destroying a CThread whose thread is
        /// still running kills it (Kill()), leaving whatever it held (locks,
        /// memory) as it was: ask it to stop with RequestShutdown() and Join()
        /// it first. An exception that escapes Run() ends the thread, not the
        /// program.
        ///
        /// Not every platform supports every operation: see GetCapabilities().
        ///
        /// @code
        /// class CWorker : public CThread
        /// {
        ///     public:
        ///         CWorker( void ) : CThread( "Worker", true ) { Resume(); }
        ///         virtual void *Run( void )
        ///         {
        ///             while ( ! IsShutdown() )
        ///                 Millisleep( 100 );
        ///             SetReturnCode( 7 );
        ///             return NULL;
        ///         }
        /// };
        ///
        /// CWorker oWorker;
        /// oWorker.RequestShutdown();
        /// int nCode = oWorker.Join();     // 7
        /// @endcode
        class CThread : public CObject
        {
            protected:
                THREAD_T            m_ptThreadID;       ///< The platform's handle for the thread.
        #ifdef IASLIB_WIN32__
                DWORD               m_dwThreadID;       ///< The thread's ID (GetCurrentThread() is not unique).
        #endif
                std::atomic<bool>   m_bIsRunning;       ///< Whether the thread is running. Shared with the thread itself.
                std::atomic<int>    m_nReturnCode;      ///< The code Join() returns.
                bool                m_bDetached;        ///< Whether the thread is detached.
                std::atomic<bool>   m_bShutdown;        ///< Whether a shutdown was requested. Set by other threads.
                bool                m_bContained;       ///< We created the thread
                bool                m_bNoDelete;        ///< Whether the thread monitor must not delete this object.
                bool                m_bDaemon;          ///< Whether this is a daemon thread.
                CString             m_strThreadName;    ///< The thread's name.

                    /// The gate the new thread waits at until it is started.
                std::mutex          m_mutexGate;
                std::condition_variable m_cvGate;       ///< Signals changes at the gate.
                bool                m_bReleased;        ///< Whether the thread has been let through the gate (started).
                bool                m_bAbandoned;       ///< destroyed before it started
                bool                m_bLeftGate;        ///< Whether the thread has stopped waiting at the gate.
                    /// Guards m_bReaped. Join() can be called again, and after
                    /// the thread has ended.
                std::mutex          m_mutexJoin;
                bool                m_bReaped;          ///< Whether the thread has been joined.

        #ifdef IASLIB_PTHREAD__
                pthread_attr_t      m_stAttributes;     ///< The attributes the thread was created with.
        #endif
                static CMutex       m_mutexCriticalSection;     ///< The lock for EnterCriticalSection().
            public:
                /// What a thread implementation can do (see GetCapabilities()).
                enum CapabilityFlags {
                                        THREADS           = 0x0001,     ///< Threads are supported.
                                        SUSPEND           = 0x0002,     ///< Suspend() works (Windows only).
                                        RESUME            = 0x0004,     ///< Resume() works.
                                        KILL              = 0x0008,     ///< Kill() works.
                                        JOIN              = 0x0010,     ///< Join() works.
                                        DETACH            = 0x0020,     ///< Detach() works.
                                        DAEMON            = 0x0040,     ///< Daemon threads are supported.
                                        SLEEP             = 0x0080,     ///< Sleep() and Millisleep() work.
                                        YIELD             = 0x0100,     ///< Yielding is supported.
                                        NAMED             = 0x0200,     ///< Threads have names.
                                        STATE             = 0x0400,     ///< IsRunning() and the like report the state.
                                      };

                    /// Creates the thread (see the class description for when it starts).
                    /// @param strThreadName The thread's name.
                    /// @param bStartSuspended True to wait for Resume() or Start().
                    /// @param bDetached True for a detached thread (cannot be joined).
                    /// @param bNoDelete True to keep the thread monitor from deleting
                    ///        this object when a detached thread finishes.
                    /// @param bDaemon True for a daemon thread.
                    /// If the thread cannot be created, the object is not running
                    /// and IsContained() is false.
                                        CThread( const char *strThreadName = "Unknown Thread", bool bStartSuspended = false, bool bDetached = false, bool bNoDelete = false, bool bDaemon = false );
                    /// Stands for a thread that already exists (the calling
                    /// thread, if ptThreadID is 0); it is not joined, killed or
                    /// started through this object.
                                        CThread( THREAD_T ptThreadID, const char *strThreadName = "Unknown Thread" );
                    /// Destructor. A thread that never started is let go without
                    /// running Run(); a running one is killed (Kill()); then the
                    /// thread is joined, unless detached.
                virtual                ~CThread( void );

                                        DECLARE_OBJECT(CThread,CObject);

                    /// What the thread does; the value is ignored (set the
                    /// return code with SetReturnCode() or ExitThread()). This
                    /// one returns at once.
                virtual void           *Run( void );

                    /// Ends the thread at its next safe point: a Millisleep(),
                    /// Sleep() or CSemaphore wait (and, outside Windows, the
                    /// system's other cancellation points). A thread that never
                    /// waits can only be stopped by its own code: see
                    /// RequestShutdown().
                    /// @return 0 once it has ended (or if it is not running or not
                    /// created here); on Windows, -1 if it has not reached one
                    /// within two seconds (it still ends when it does). With POSIX
                    /// threads, the result of pthread_cancel().
                    /// @note On Windows this was TerminateThread(), which can stop
                    /// a thread while it holds a lock - the heap's, the loader's -
                    /// so that the whole process then hangs.
                virtual int             Kill( void );

                    /// Waits for the thread to end, returning its return code
                    /// (0 for a detached thread, or one not created here). A
                    /// suspended thread is started first.
                virtual int             Join( void );

                    /// Detaches the thread, so it cannot be joined.
                    /// @return False if it was not created here, is already
                    /// detached, or has been joined.
                virtual bool            Detach( void );

                    /// On Windows, suspends a started thread. Does nothing
                    /// elsewhere: POSIX threads cannot be suspended.
                void                    Suspend( void );
                    /// Starts a suspended thread; on Windows it also resumes one
                    /// Suspend() stopped.
                void                    Resume( void );
                void                    Start( void ) { Resume(); }     ///< Same as Resume().
                bool                    IsStarted( void );              ///< Whether the thread has been started.

                THREAD_T                GetThreadID( void ) const { return m_ptThreadID; }              ///< The platform's handle for the thread.
                int                     GetReturnCode( void ) const { return m_nReturnCode; }           ///< The return code set so far.
                void                    SetReturnCode( int nReturnCode ) { m_nReturnCode = nReturnCode; } ///< Sets the code Join() returns.
                bool                    IsRunning( void ) const { return m_bIsRunning; }                ///< Whether the thread is running.
                void                    SetRunning( bool bRunning ) { m_bIsRunning = bRunning; }        ///< Sets the running flag.

                const char             *GetName( void ) const { return (const char *)m_strThreadName; } ///< The thread's name.

                bool                    IsDetached( void ) const { return m_bDetached; }        ///< Whether the thread is detached.
                bool                    IsContained( void ) const { return m_bContained; }      ///< Whether this object created the thread.

                void                    SetNoDelete( bool bNoDelete ) { m_bNoDelete = bNoDelete; }  ///< Sets whether the thread monitor must not delete this object.
                bool                    IsNoDelete( void ) const { return m_bNoDelete; }        ///< Whether the thread monitor must not delete this object.

                bool                    IsDaemon( void ) const { return m_bDaemon; }            ///< Whether this is a daemon thread.

                    /// The thread monitor kills a thread still running nSeconds
                    /// after it started.
                void                    SetTimeout( int nSeconds );

                    /// Asks the thread to stop: sets the flag IsShutdown() reports,
                    /// which Run() should check.
                virtual void            RequestShutdown( void ) { m_bShutdown = true; }
                bool                    IsShutdown( void ) const { return m_bShutdown; }        ///< Whether a shutdown was requested.

                    /// The CThread for the calling thread (NULL if it has none).
                static CThread         *Self( void );

                    /// Whether two handles are for the same thread.
                static bool             Equals( THREAD_T ptThread1, THREAD_T ptThread2 );

                    /// Sleeps (a point at which Kill() can end the thread).
                static void             Sleep( int nSeconds );
                    /// Same, in milliseconds.
                static void             Millisleep( int milliseconds );

                    /// The CapabilityFlags this implementation supports.
                virtual unsigned long   GetCapabilities( void );

                    /// Locks the one process-wide critical section.
                static void             EnterCriticalSection( void ) { m_mutexCriticalSection.Lock(); }
                    /// Unlocks the critical section.
                static void             LeaveCriticalSection( void ) { m_mutexCriticalSection.Unlock(); }

                    /// Runs the thread (the platform's start routine calls it).
                void                    ThreadMain( void );

            protected:
                    /// Ends the calling thread, which must be this one, with a
                    /// return code. Does nothing if called from another thread.
                    /// @note On Windows the old code called itself forever (this
                    /// member's name hid the Windows function).
                void                    ExitThread( int nExitCode );

            private:
                void                    Release( void );    ///< Lets the thread through the gate (starts it).
                                        CThread( const CThread & );     ///< Not copyable.
                CThread                &operator =( const CThread & );  ///< Not copyable.
        };

    } // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREAD_H__
