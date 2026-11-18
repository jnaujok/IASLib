/**
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
        // The base of loaded resources. A loader's subclass reports how much
        // memory it holds, for the budget.
    class CLoadedResource
    {
        friend class CResourceManager;
        protected:
            CString             m_strPath;
        public:
            virtual            ~CLoadedResource( void ) {}
            virtual size_t      GetMemorySize( void ) const = 0;
            const CString      &GetPath( void ) const { return m_strPath; }
    };

        // A resource no loader claimed: its bytes.
    class CRawResource : public CLoadedResource
    {
        protected:
            CDataBlock          m_data;
        public:
            explicit            CRawResource( const CDataBlock &oData ) : m_data( oData ) {}
            virtual size_t      GetMemorySize( void ) const { return m_data.GetSize(); }
            const CDataBlock   &GetData( void ) const { return m_data; }
            const unsigned char *GetBytes( void ) const { return (const unsigned char *)m_data; }
            size_t              GetLength( void ) const { return m_data.GetSize(); }
    };

    class CResourceManager
    {
        public:
                // Turns bytes into a resource (NULL, with strError, if it
                // cannot). May run on a worker thread.
            typedef std::function<std::shared_ptr<CLoadedResource>( const CString &strPath, const CDataBlock &oData, CString &strError )> Loader;
                // A finished LoadAsync(): the resource, or NULL and why.
            typedef std::function<void( const CString &strPath, const std::shared_ptr<CLoadedResource> &pResource, const CString &strError )> Callback;

            struct CStatistics
            {
                size_t      nLoaded;            // resources held
                size_t      nBytes;             // their memory
                size_t      nBudget;
                size_t      nPending;           // loads under way
                uint64_t    nHits;
                uint64_t    nMisses;            // loads started
                uint64_t    nFailed;
                uint64_t    nUnloaded;          // by the budget, Unload*() or Reload()
            };

        protected:
            struct CEntry
            {
                std::shared_ptr<CLoadedResource>    pResource;
                size_t                              nSize;
                std::list<std::string>::iterator    itLRU;
            };
            struct CPending
            {
                bool                                bDone;
                std::shared_ptr<CLoadedResource>    pResource;
                CString                             strError;
                std::vector<Callback>               aCallbacks;
                CPending( void ) : bDone( false ) {}
            };
            struct CCompleted
            {
                CString                             strPath;
                std::shared_ptr<CLoadedResource>    pResource;
                CString                             strError;
                Callback                            fnCallback;
            };
            struct CSource
            {
                std::shared_ptr<CResourceSource>    pSource;
                int                                 nPriority;
            };

            mutable std::mutex                      m_mutex;
            std::condition_variable                 m_cvDone;       // a pending load finished
            std::condition_variable                 m_cvWork;       // work for the threads
            std::vector<CSource>                    m_aSources;
            std::map<std::string, Loader>           m_mapLoaders;   // by lower-case extension
            Loader                                  m_fnDefaultLoader;
            std::map<std::string, CEntry>           m_mapLoaded;
            std::list<std::string>                  m_listLRU;      // most recent at the front
            std::map<std::string, std::shared_ptr<CPending> > m_mapPending;
            std::deque<std::string>                 m_queueWork;
            std::deque<CCompleted>                  m_queueCompleted;
            std::vector<std::thread>                m_aWorkers;
            bool                                    m_bStopping;
            size_t                                  m_nBytes;
            size_t                                  m_nBudget;
            uint64_t                                m_nHits;
            uint64_t                                m_nMisses;
            uint64_t                                m_nFailed;
            uint64_t                                m_nUnloaded;
            CString                                 m_strLastError;

        public:
                                // nWorkers threads for LoadAsync() (0: it
                                // loads at once, on the calling thread).
                                CResourceManager( size_t nBudgetBytes = 256 * 1024 * 1024, int nWorkers = 2 );
                                // Waits for loads under way; callbacks not yet
                                // processed are dropped.
            virtual            ~CResourceManager( void );

                // Searched highest priority first (equal priorities: the
                // first added first).
            void                AddSource( const std::shared_ptr<CResourceSource> &pSource, int nPriority = 0 );
                // For paths ending in strExtension (".png"; any case). An
                // empty extension replaces the default (raw) loader.
            void                RegisterLoader( const char *strExtension, const Loader &fnLoader );

                // The resource, loading it if need be (NULL if it cannot be:
                // GetLastError() says why).
            std::shared_ptr<CLoadedResource> Get( const char *strPath );
            template <class T>
            std::shared_ptr<T>  Get( const char *strPath ) { return std::dynamic_pointer_cast<T>( Get( strPath ) ); }

                // Starts loading in the background; fnCallback (if any) runs
                // in ProcessCompleted().
            void                LoadAsync( const char *strPath, const Callback &fnCallback = Callback() );
                // Runs the callbacks of finished loads, on this thread (at
                // most nMax of them); returns how many ran.
            size_t              ProcessCompleted( size_t nMax = (size_t)-1 );
                // Waits until no load is under way (false if the time ran out;
                // a negative timeout waits for ever).
            bool                WaitForPending( int nTimeoutMs = -1 );

            bool                IsLoaded( const char *strPath ) const;
                // Whether any source has the path.
            bool                Exists( const char *strPath );
                // Drops the manager's hold (holders keep theirs).
            bool                Unload( const char *strPath );
                // Drops every resource nobody else holds; returns how many.
            size_t              UnloadUnused( void );
                // Loads the path again; the new object replaces the old for
                // later Get()s (holders of the old one keep it).
            std::shared_ptr<CLoadedResource> Reload( const char *strPath );

            void                SetBudget( size_t nBytes );
            CStatistics         GetStatistics( void ) const;
            CString             GetLastError( void ) const;

                // A path in the manager's form ("" if it is not allowed).
            static CString      NormalizePath( const char *strPath );

        private:
            std::shared_ptr<CLoadedResource> LoadNow( const std::string &strPath, CString &strError );
            void                Finish( const std::string &strPath, const std::shared_ptr<CLoadedResource> &pResource, const CString &strError );
            void                Store( const std::string &strPath, const std::shared_ptr<CLoadedResource> &pResource );
            void                Drop( std::map<std::string, CEntry>::iterator it );
            void                EnforceBudget( void );
            void                WorkerLoop( void );

                                CResourceManager( const CResourceManager & );
            CResourceManager   &operator =( const CResourceManager & );
    };
} // namespace IASLib

#endif // IASLIB_RESOURCEMANAGER_H__
