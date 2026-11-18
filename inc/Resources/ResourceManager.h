/**
 *  @file
 *  Resource Manager
 *
 *      Loads a program's resources (a game's textures, sounds, levels...)
 * by path, and keeps them while they are wanted:
 *
 *      - Sources (CResourceSource) are searched in priority order, so loose
 *        files or a patch archive can override the main archive.
 *      - Loaders, registered by file extension, turn the bytes into typed
 *        resources (subclasses of CLoadedResource); without one, a resource
 *        is a CRawResource holding the bytes.
 *      - Get() returns a shared handle: everyone asking for a path gets
 *        the same object, which lives while anyone holds it.
 *      - Over its memory budget, the manager unloads the least recently
 *        used resources that nobody else holds. A resource in use is never
 *        unloaded (so the budget can be exceeded while they are held).
 *      - LoadAsync() loads on worker threads. Completion callbacks run on
 *        the thread that calls ProcessCompleted() (a game's main loop, say).
 *        Requests for a path already loading share that load, as does a
 *        Get() for it, which waits.
 *
 *      Paths use '/', are case-sensitive, and may not contain "..".
 * Thread-safe.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_RESOURCEMANAGER_H__
#define IASLIB_RESOURCEMANAGER_H__

#include <condition_variable>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "../BaseTypes/String_.h"
#include "../BaseTypes/DataBlock.h"
#include "ResourceSource.h"

namespace IASLib
{
        /// The base of loaded resources.
        ///
        /// A loader's subclass reports how much memory it holds, for the
        /// budget. The manager sets the path once the loader returns.
    class CLoadedResource
    {
        friend class CResourceManager;
        protected:
            CString             m_strPath;      ///< The path it was loaded from (set by the manager).
        public:
            /// Destroys the resource.
            virtual            ~CLoadedResource( void ) {}
            /// The memory the resource holds, in bytes (counted against the budget).
            virtual size_t      GetMemorySize( void ) const = 0;
            /// The (normalised) path it was loaded from.
            const CString      &GetPath( void ) const { return m_strPath; }
    };

        /// A resource no loader claimed: its bytes.
    class CRawResource : public CLoadedResource
    {
        protected:
            CDataBlock          m_data;     ///< The bytes.
        public:
            /// Holds a copy of the bytes.
            explicit            CRawResource( const CDataBlock &oData ) : m_data( oData ) {}
            /// The length of the bytes.
            virtual size_t      GetMemorySize( void ) const { return m_data.GetSize(); }
            /// The bytes as a block.
            const CDataBlock   &GetData( void ) const { return m_data; }
            /// The bytes.
            const unsigned char *GetBytes( void ) const { return (const unsigned char *)m_data; }
            /// The length of the bytes.
            size_t              GetLength( void ) const { return m_data.GetSize(); }
    };

        /// Loads resources by path from its sources, and caches them within a budget.
        ///
        /// See the file description for the rules. Every member is
        /// thread-safe, but callbacks run only in ProcessCompleted(). Not
        /// copyable.
        ///
        /// @code
        /// CResourceManager manager( 64 * 1024 * 1024 );
        /// manager.AddSource( std::make_shared<CDirectorySource>( "mods" ), 10 );
        /// manager.AddSource( std::make_shared<CArchiveSource>( pArchive ) );
        /// manager.RegisterLoader( ".png", &LoadTexture );
        ///
        /// std::shared_ptr<CTexture> pHero = manager.Get<CTexture>( "textures/hero.png" );
        /// manager.LoadAsync( "levels/2.dat", []( const CString &, const std::shared_ptr<CLoadedResource> &p, const CString &strError ) { ... } );
        /// for ( ;; ) { manager.ProcessCompleted(); ... }     // the main loop
        /// @endcode
    class CResourceManager
    {
        public:
                /// Turns bytes into a resource (NULL, with strError, if it
                /// cannot). May run on a worker thread.
                /// An exception it throws is caught and treated as a failure.
            typedef std::function<std::shared_ptr<CLoadedResource>( const CString &strPath, const CDataBlock &oData, CString &strError )> Loader;
                /// A finished LoadAsync(): the resource, or NULL and why.
            typedef std::function<void( const CString &strPath, const std::shared_ptr<CLoadedResource> &pResource, const CString &strError )> Callback;

            /// What the manager holds and has done (see GetStatistics()).
            struct CStatistics
            {
                size_t      nLoaded;            ///< Resources held.
                size_t      nBytes;             ///< Their memory.
                size_t      nBudget;            ///< The memory budget.
                size_t      nPending;           ///< Loads under way.
                uint64_t    nHits;              ///< Requests answered from what was held.
                uint64_t    nMisses;            ///< Loads started.
                uint64_t    nFailed;            ///< Loads (or bad paths) that failed.
                uint64_t    nUnloaded;          ///< Unloaded by the budget, Unload*() or Reload().
            };

        protected:
            /// A resource held.
            struct CEntry
            {
                std::shared_ptr<CLoadedResource>    pResource;  ///< The resource.
                size_t                              nSize;      ///< Its memory when stored.
                std::list<std::string>::iterator    itLRU;      ///< Its place in m_listLRU.
            };
            /// A load under way.
            struct CPending
            {
                bool                                bDone;      ///< True once it finished.
                std::shared_ptr<CLoadedResource>    pResource;  ///< The result (NULL: failed).
                CString                             strError;   ///< Why it failed.
                std::vector<Callback>               aCallbacks; ///< Callbacks waiting for it.
                /// A load not yet done.
                CPending( void ) : bDone( false ) {}
            };
            /// A finished load whose callback has not run yet.
            struct CCompleted
            {
                CString                             strPath;    ///< The path.
                std::shared_ptr<CLoadedResource>    pResource;  ///< The result (NULL: failed).
                CString                             strError;   ///< Why it failed.
                Callback                            fnCallback; ///< The callback to run.
            };
            /// A source and its priority.
            struct CSource
            {
                std::shared_ptr<CResourceSource>    pSource;    ///< The source.
                int                                 nPriority;  ///< Higher is searched first.
            };

            mutable std::mutex                      m_mutex;        ///< Guards everything below.
            std::condition_variable                 m_cvDone;       ///< A pending load finished.
            std::condition_variable                 m_cvWork;       ///< Work for the threads.
            std::vector<CSource>                    m_aSources;     ///< Sources, in search order.
            std::map<std::string, Loader>           m_mapLoaders;   ///< Loaders by lower-case extension.
            Loader                                  m_fnDefaultLoader;  ///< For paths no loader claims.
            std::map<std::string, CEntry>           m_mapLoaded;    ///< Resources held, by path.
            std::list<std::string>                  m_listLRU;      ///< Paths held, most recent at the front.
            std::map<std::string, std::shared_ptr<CPending> > m_mapPending;    ///< Loads under way, by path.
            std::deque<std::string>                 m_queueWork;    ///< Paths for the worker threads.
            std::deque<CCompleted>                  m_queueCompleted;   ///< Callbacks for ProcessCompleted().
            std::vector<std::thread>                m_aWorkers;     ///< The worker threads.
            bool                                    m_bStopping;    ///< True: the workers should stop.
            size_t                                  m_nBytes;       ///< Memory of the resources held.
            size_t                                  m_nBudget;      ///< The memory budget.
            uint64_t                                m_nHits;        ///< See CStatistics::nHits.
            uint64_t                                m_nMisses;      ///< See CStatistics::nMisses.
            uint64_t                                m_nFailed;      ///< See CStatistics::nFailed.
            uint64_t                                m_nUnloaded;    ///< See CStatistics::nUnloaded.
            CString                                 m_strLastError; ///< Why the last load failed.

        public:
                                /// Creates a manager with no sources and starts its threads.
                                /// @param nBudgetBytes The memory budget.
                                /// @param nWorkers     Threads for LoadAsync() (0: it
                                ///                     loads at once, on the calling thread).
                                CResourceManager( size_t nBudgetBytes = 256 * 1024 * 1024, int nWorkers = 2 );
                                /// Waits for loads under way (the workers finish their
                                /// queue); callbacks not yet processed are dropped.
            virtual            ~CResourceManager( void );

                /// Adds a source.
                /// Sources are searched highest priority first (equal
                /// priorities: the first added first).
                /// @param pSource   The source (NULL is ignored).
                /// @param nPriority Its priority.
            void                AddSource( const std::shared_ptr<CResourceSource> &pSource, int nPriority = 0 );
                /// Sets the loader for paths ending in strExtension (".png"
                /// or "png"; any case).
                /// An empty extension replaces the default (raw) loader. An
                /// empty fnLoader removes the extension's loader (or, for the
                /// default, restores the raw one).
            void                RegisterLoader( const char *strExtension, const Loader &fnLoader );

                /// The resource, loading it if need be (NULL if it cannot be:
                /// GetLastError() says why).
                /// If the path is already loading, waits for that load.
                /// @param strPath The path (normalised by NormalizePath()).
            std::shared_ptr<CLoadedResource> Get( const char *strPath );
                /// The same, cast to the loader's type (NULL if it is another type).
                /// @tparam T The resource's class.
            template <class T>
            std::shared_ptr<T>  Get( const char *strPath ) { return std::dynamic_pointer_cast<T>( Get( strPath ) ); }

                /// Starts loading in the background; fnCallback (if any) runs
                /// in ProcessCompleted().
                /// A resource already held, or a bad path, still gets its
                /// callback in ProcessCompleted().
                /// @param strPath    The path.
                /// @param fnCallback Called with the result.
            void                LoadAsync( const char *strPath, const Callback &fnCallback = Callback() );
                /// Runs the callbacks of finished loads, on this thread (at
                /// most nMax of them), then applies the budget.
                /// @return How many ran.
            size_t              ProcessCompleted( size_t nMax = (size_t)-1 );
                /// Waits until no load is under way.
                /// @param nTimeoutMs The most to wait; negative waits for ever.
                /// @return False if the time ran out.
            bool                WaitForPending( int nTimeoutMs = -1 );

            /// True if the manager holds the path now.
            bool                IsLoaded( const char *strPath ) const;
                /// Whether the path is held, or any source has it.
            bool                Exists( const char *strPath );
                /// Drops the manager's hold (holders keep theirs).
                /// @return False if it was not held.
            bool                Unload( const char *strPath );
                /// Drops every resource nobody else holds.
                /// @return How many.
            size_t              UnloadUnused( void );
                /// Loads the path again; the new object replaces the old for
                /// later Get()s (holders of the old one keep it).
                /// Waits for a load already under way first.
                /// @return The new object, or NULL as for Get().
            std::shared_ptr<CLoadedResource> Reload( const char *strPath );

            /// Sets the memory budget, unloading what it can to keep to it.
            void                SetBudget( size_t nBytes );
            /// A snapshot of the manager's statistics.
            CStatistics         GetStatistics( void ) const;
            /// Why the last load failed.
            CString             GetLastError( void ) const;

                /// A path in the manager's form ("" if it is not allowed).
                /// '\\' becomes '/', and empty and "." parts are dropped; a ".."
                /// part is not allowed.
            static CString      NormalizePath( const char *strPath );

        private:
            /// Finds and loads a path, without the lock (NULL, with strError, on failure).
            std::shared_ptr<CLoadedResource> LoadNow( const std::string &strPath, CString &strError );
            /// Under the lock: records a finished load and queues its callbacks.
            void                Finish( const std::string &strPath, const std::shared_ptr<CLoadedResource> &pResource, const CString &strError );
            /// Under the lock: holds a loaded resource, replacing any held for the path.
            void                Store( const std::string &strPath, const std::shared_ptr<CLoadedResource> &pResource );
            /// Under the lock: drops a held resource.
            void                Drop( std::map<std::string, CEntry>::iterator it );
            /// Under the lock: unloads the oldest unused resources until within the budget.
            void                EnforceBudget( void );
            /// A worker thread's loop.
            void                WorkerLoop( void );

                                /// Not copyable.
                                CResourceManager( const CResourceManager & );
                                /// Not assignable.
            CResourceManager   &operator =( const CResourceManager & );
    };
} // namespace IASLib

#endif // IASLIB_RESOURCEMANAGER_H__
