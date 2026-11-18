/**
 * @file Thread.h
 * @brief CThread, the library's thread class, and the THREAD_T handle type.
 *
 * Derive from CThread and override Run() to make an object that runs as a
 * thread. See the CThread class for when a thread starts and how it ends.
 * The original header comment follows.
 */
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
            /// The platform's thread handle type (HANDLE on Windows).
        #define THREAD_T HANDLE
    #else
        #ifdef IASLIB_PTHREAD__
            #include <pthread.h>
                /// The platform's thread handle type (pthread_t with POSIX threads).
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
        /// A thread of execution, run by an object.
        ///
        /// Derive from CThread and override Run(); the new thread calls
        /// Run() once it is started, and ends when Run() returns.
        ///
        /// Starting: a thread created with @p bStartSuspended runs when
        /// Resume() (or Start()) is called, from any thread. A thread
        /// created without it starts as the CThread constructor ends, which
        /// is before a derived class's constructor has run; so derived
        /// classes should start suspended and call Resume() at the end of
        /// their own constructor.
        ///
        /// Ending: Run() returns, or ExitThread() is called from the
        /// thread. Join() waits for that and gives the return code; it may
        /// be called after the thread has finished. An exception that
        /// escapes Run() ends the thread, not the program.
        ///
        /// Ownership: whoever creates a joinable thread owns the object and
        /// deletes it. Detached threads cannot be joined; the thread monitor
        /// (CThreadMonitor) deletes them when they finish, unless
        /// SetNoDelete() was called. Destroying a CThread whose thread is
        /// still running kills it (Kill()), which leaves whatever it held
        /// (locks, memory) as it was: ask it to stop with RequestShutdown()
        /// and Join() it first.
        ///
        /// Not all systems support every operation; GetCapabilities() says
        /// which ones work here.
        ///
        /// @code
        /// class CWorker : public CThread
        /// {
        ///     public:
        ///         CWorker( void ) : CThread( "Worker", true ) { Resume(); }
        ///         virtual void *Run( void )
        ///         {
        ///             while ( ! IsShutdown() )
        ///                 CThread::Millisleep( 10 );
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
                DWORD               m_dwThreadID;       ///< The Windows thread ID (GetCurrentThread() is not unique).
        #endif
                std::atomic<bool>   m_bIsRunning;       ///< Whether the thread is running. Shared with the thread itself.
                std::atomic<int>    m_nReturnCode;      ///< The code Join() returns.
                bool                m_bDetached;        ///< Whether the thread is detached.
                std::atomic<bool>   m_bShutdown;        ///< Set by other threads, through RequestShutdown().
                bool                m_bContained;       ///< We created the thread (it is not one that already existed).
                bool                m_bNoDelete;        ///< The thread monitor must not delete this object.
                bool                m_bDaemon;          ///< Whether the thread was created as a daemon.
                CString             m_strThreadName;    ///< The thread's name, as given to the constructor.

                    /// The gate the new thread waits at until it is started.
                std::mutex          m_mutexGate;
                std::condition_variable m_cvGate;       ///< Signals changes at the start gate.
                bool                m_bReleased;        ///< The thread has been let through the gate (started).
                bool                m_bAbandoned;       ///< Destroyed before it started.
                bool                m_bLeftGate;        ///< The thread has left the gate without running.
                    /// Guards the join; whether the thread has been joined is
                    /// in m_bReaped (Join() can be called again, and after the
                    /// thread has ended).
                std::mutex          m_mutexJoin;
                bool                m_bReaped;          ///< The thread has been joined (or never existed).

        #ifdef IASLIB_PTHREAD__
                pthread_attr_t      m_stAttributes;     ///< The attributes the thread was created with.
        #endif
                static CMutex       m_mutexCriticalSection; ///< The process-wide lock behind EnterCriticalSection().
            public:
                /// What a thread implementation can do; GetCapabilities()
                /// returns a combination of these flags.
                enum CapabilityFlags {
                                        THREADS           = 0x0001,    ///< Threads exist at all.
                                        SUSPEND           = 0x0002,    ///< Suspend() stops a running thread (Windows only).
                                        RESUME            = 0x0004,    ///< Resume() starts (or resumes) a thread.
                                        KILL              = 0x0008,    ///< Kill() ends a thread.
                                        JOIN              = 0x0010,    ///< Join() waits for a thread.
                                        DETACH            = 0x0020,    ///< Detach() detaches a thread.
                                        DAEMON            = 0x0040,    ///< Daemon threads are supported.
                                        SLEEP             = 0x0080,    ///< Sleep() and Millisleep() work.
                                        YIELD             = 0x0100,    ///< A thread can yield.
                                        NAMED             = 0x0200,    ///< Threads have names.
                                        STATE             = 0x0400,    ///< A thread's state can be read (IsRunning()).
                                      };

                    /// Creates the thread and registers it with the thread monitor.
                    ///
                    /// The thread starts as this constructor ends, unless
                    /// @p bStartSuspended is set; then it starts when Resume()
                    /// or Start() is called. If the platform cannot create the
                    /// thread, the object is left not running and not contained.
                    /// @param strThreadName  The thread's name (shown by the thread monitor).
                    /// @param bStartSuspended  Wait for Resume() before running.
                    /// @param bDetached  Create the thread detached: it cannot be
                    ///                   joined, and the monitor deletes it when it ends.
                    /// @param bNoDelete  The thread monitor must not delete this object.
                    /// @param bDaemon  Mark the thread as a daemon (only recorded).
                                        CThread( const char *strThreadName = "Unknown Thread", bool bStartSuspended = false, bool bDetached = false, bool bNoDelete = false, bool bDaemon = false );
                    /// Stands for a thread that already exists.
                    ///
                    /// The thread is not joined, killed or started through
                    /// this object, and the monitor never deletes it.
                    /// @param ptThreadID  The thread; 0 means the calling thread.
                    /// @param strThreadName  The name to register it under.
                                        CThread( THREAD_T ptThreadID, const char *strThreadName = "Unknown Thread" );
                    /// Ends the thread and unregisters it from the monitor.
                    ///
                    /// A thread never started leaves without calling Run(); a
                    /// running one is killed (Kill()); a joinable one is then
                    /// joined.
                virtual                ~CThread( void );

                                        DECLARE_OBJECT(CThread,CObject);

                    /// What the thread does; override it.
                    ///
                    /// The base version does nothing. The value is ignored (set
                    /// the return code with SetReturnCode() or ExitThread()).
                    /// @return Ignored.
                virtual void           *Run( void );

                    /// Ends the thread at once (pthread_cancel, TerminateThread).
                    ///
                    /// Anything the thread held (locks, memory) is left as it was.
                    /// @return 0 on success or if there is nothing to kill (the
                    ///         thread is not running, or was not created here);
                    ///         otherwise the platform's error code (-1 on Windows).
                virtual int             Kill( void );

                    /// Waits for the thread to end, returning its return code.
                    ///
                    /// A thread still suspended is started first, so that it can
                    /// end. Join() may be called again, and after the thread has
                    /// ended.
                    /// @return The return code; 0 for a detached thread, or one
                    ///         not created here.
                virtual int             Join( void );

                    /// Detaches the thread, so that the monitor deletes it when it ends.
                    /// @return false if the thread was not created here, is already
                    ///         detached or has already been joined; true otherwise.
                virtual bool            Detach( void );

                    /// Suspends a running thread, on Windows only.
                    ///
                    /// Suspend() does nothing elsewhere: POSIX threads cannot be
                    /// suspended.
                void                    Suspend( void );
                    /// Starts a suspended thread.
                    ///
                    /// On Windows it also resumes one Suspend() stopped. Does
                    /// nothing for a thread not created here.
                void                    Resume( void );
                    /// Starts a suspended thread; the same as Resume().
                void                    Start( void ) { Resume(); }
                    /// Whether the thread has been started (let past its start gate).
                bool                    IsStarted( void );

                    /// The platform's handle for the thread.
                THREAD_T                GetThreadID( void ) const { return m_ptThreadID; }
                    /// The thread's return code (what Join() returns).
                int                     GetReturnCode( void ) const { return m_nReturnCode; }
                    /// Sets the code Join() will return.
                    /// @param nReturnCode  The return code.
                void                    SetReturnCode( int nReturnCode ) { m_nReturnCode = nReturnCode; }
                    /// Whether the thread is running (created and not yet ended).
                bool                    IsRunning( void ) const { return m_bIsRunning; }
                    /// Sets the running flag.
                    /// @param bRunning  The new value.
                void                    SetRunning( bool bRunning ) { m_bIsRunning = bRunning; }

                    /// The thread's name.
                const char             *GetName( void ) const { return (const char *)m_strThreadName; }

                    /// Whether the thread is detached.
                bool                    IsDetached( void ) const { return m_bDetached; }
                    /// Whether this object created the thread (rather than
                    /// standing for one that already existed).
                bool                    IsContained( void ) const { return m_bContained; }

                    /// Sets whether the thread monitor must leave this object alone.
                    /// @param bNoDelete  true: the monitor never deletes it.
                void                    SetNoDelete( bool bNoDelete ) { m_bNoDelete = bNoDelete; }
                    /// Whether the thread monitor must leave this object alone.
                bool                    IsNoDelete( void ) const { return m_bNoDelete; }

                    /// Whether the thread was created as a daemon.
                bool                    IsDaemon( void ) const { return m_bDaemon; }

                    /// Sets a time limit: the thread monitor kills a thread still
                    /// running nSeconds after it started.
                    /// @param nSeconds  The limit in seconds; 0 for none.
                void                    SetTimeout( int nSeconds );

                    /// Asks the thread to stop; Run() should check IsShutdown().
                virtual void            RequestShutdown( void ) { m_bShutdown = true; }
                    /// Whether the thread has been asked to stop.
                bool                    IsShutdown( void ) const { return m_bShutdown; }

                    /// The CThread for the calling thread.
                    /// @return The object, or NULL if the thread has none.
                static CThread         *Self( void );

                    /// Whether two thread handles name the same thread.
                    /// @param ptThread1  A thread.
                    /// @param ptThread2  Another thread.
                    /// @return true if they are the same thread.
                static bool             Equals( THREAD_T ptThread1, THREAD_T ptThread2 );

                    /// Sleeps the calling thread for nSeconds seconds (not at all if 0 or less).
                    /// @param nSeconds  How long to sleep, in seconds.
                static void             Sleep( int nSeconds );
                    /// Sleeps the calling thread for some milliseconds (not at all if 0 or less).
                    /// @param milliseconds  How long to sleep, in milliseconds.
                static void             Millisleep( int milliseconds );

                    /// What this thread implementation can do.
                    /// @return A combination of CapabilityFlags.
                virtual unsigned long   GetCapabilities( void );

                    /// Locks the process-wide critical section (one recursive mutex
                    /// shared by all threads).
                static void             EnterCriticalSection( void ) { m_mutexCriticalSection.Lock(); }
                    /// Unlocks the process-wide critical section.
                static void             LeaveCriticalSection( void ) { m_mutexCriticalSection.Unlock(); }

                    /// Runs the thread (the platform's start routine calls it).
                    ///
                    /// Waits to be started, then calls Run(). Not for other callers.
                void                    ThreadMain( void );

            protected:
                    /// Ends the calling thread, which must be this one, with a
                    /// return code.
                    ///
                    /// Does nothing if called from another thread.
                    /// @param nExitCode  The code Join() will return.
                void                    ExitThread( int nExitCode );

            private:
                    /// Lets the thread past its start gate.
                void                    Release( void );
                                        CThread( const CThread & );    ///< Not copyable.
                CThread                &operator =( const CThread & ); ///< Not copyable.
        };

    } // Namespace IASLib
#endif // IASLIB_MULTI_THREADED__
#endif // IASLIB_THREAD_H__
