/**
 *  Resource Manager
 *
 *      See ResourceManager.h.
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 2007
 *	Log:
 *	  $LOG$
 *
 * Copyright (C) 2007, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#include "ResourceManager.h"

#include <algorithm>
#include <chrono>
#include <ctype.h>
#include <exception>

namespace IASLib
{
    namespace
    {
        std::string extensionOf( const std::string &strPath )
        {
            size_t nSlash = strPath.rfind( '/' );
            size_t nDot = strPath.rfind( '.' );
            if ( nDot == std::string::npos || ( nSlash != std::string::npos && nDot < nSlash ) )
                return std::string();
            std::string strExt = strPath.substr( nDot );
            for ( size_t n = 0; n < strExt.size(); n++ )
                strExt[n] = (char)tolower( (unsigned char)strExt[n] );
            return strExt;
        }

        std::shared_ptr<CLoadedResource> rawLoader( const CString &, const CDataBlock &oData, CString & )
        {
            return std::make_shared<CRawResource>( oData );
        }
    }

    CResourceManager::CResourceManager( size_t nBudgetBytes, int nWorkers )
        : m_fnDefaultLoader( &rawLoader ), m_bStopping( false ), m_nBytes( 0 ), m_nBudget( nBudgetBytes ),
          m_nHits( 0 ), m_nMisses( 0 ), m_nFailed( 0 ), m_nUnloaded( 0 )
    {
        for ( int n = 0; n < nWorkers; n++ )
            m_aWorkers.push_back( std::thread( [this]() { WorkerLoop(); } ) );
    }

    CResourceManager::~CResourceManager( void )
    {
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            m_bStopping = true;
        }
        m_cvWork.notify_all();
        for ( size_t n = 0; n < m_aWorkers.size(); n++ )
            m_aWorkers[n].join();
    }

    CString CResourceManager::NormalizePath( const char *strPath )
    {
        if ( strPath == NULL )
            return CString( "" );
        std::string strIn( strPath );
        std::replace( strIn.begin(), strIn.end(), '\\', '/' );

            // Rebuild from the parts, dropping empty and "." parts.
        std::string strOut;
        size_t nStart = 0;
        while ( nStart <= strIn.size() )
        {
            size_t nEnd = strIn.find( '/', nStart );
            if ( nEnd == std::string::npos )
                nEnd = strIn.size();
            std::string strPart = strIn.substr( nStart, nEnd - nStart );
            if ( strPart == ".." )
                return CString( "" );               // outside the sources
            if ( ! strPart.empty() && strPart != "." )
            {
                if ( ! strOut.empty() )
                    strOut += '/';
                strOut += strPart;
            }
            nStart = nEnd + 1;
        }
        return CString( strOut.c_str() );
    }

    void CResourceManager::AddSource( const std::shared_ptr<CResourceSource> &pSource, int nPriority )
    {
        if ( ! pSource )
            return;
        std::lock_guard<std::mutex> lock( m_mutex );
        CSource oSource;
        oSource.pSource = pSource;
        oSource.nPriority = nPriority;
            // After any of the same priority, so earlier ones are searched first.
        std::vector<CSource>::iterator it = m_aSources.begin();
        while ( it != m_aSources.end() && it->nPriority >= nPriority )
            ++it;
        m_aSources.insert( it, oSource );
    }

    void CResourceManager::RegisterLoader( const char *strExtension, const Loader &fnLoader )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        std::string strExt( ( strExtension ) ? strExtension : "" );
        if ( strExt.empty() )
        {
            m_fnDefaultLoader = ( fnLoader ) ? fnLoader : Loader( &rawLoader );
            return;
        }
        if ( strExt[0] != '.' )
            strExt = "." + strExt;
        for ( size_t n = 0; n < strExt.size(); n++ )
            strExt[n] = (char)tolower( (unsigned char)strExt[n] );
        if ( fnLoader )
            m_mapLoaders[ strExt ] = fnLoader;
        else
            m_mapLoaders.erase( strExt );
    }

        // Without the lock: the sources and loaders do the slow part.
    std::shared_ptr<CLoadedResource> CResourceManager::LoadNow( const std::string &strPath, CString &strError )
    {
        std::vector<CSource> aSources;
        Loader fnLoader;
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            aSources = m_aSources;
            std::map<std::string, Loader>::const_iterator it = m_mapLoaders.find( extensionOf( strPath ) );
            fnLoader = ( it != m_mapLoaders.end() ) ? it->second : m_fnDefaultLoader;
        }

        CString strPathText( strPath.c_str() );
        for ( size_t n = 0; n < aSources.size(); n++ )
        {
            CResourceSource *pSource = aSources[n].pSource.get();
            if ( ! pSource->Exists( strPathText ) )
                continue;

            CDataBlock oData;
            CString strReadError;
            if ( ! pSource->Read( strPathText, oData, strReadError ) )
            {
                strError = strPathText + ": " + pSource->GetName() + ": " + strReadError;
                return std::shared_ptr<CLoadedResource>();
            }

            std::shared_ptr<CLoadedResource> pResource;
            CString strLoadError;
            try
            {
                pResource = fnLoader( strPathText, oData, strLoadError );
            }
            catch ( std::exception &e )
            {
                strLoadError = e.what();
            }
            catch ( ... )
            {
                strLoadError = "the loader threw an exception";
            }
            if ( ! pResource )
            {
                strError = strPathText + ": " + ( strLoadError.GetLength() ? strLoadError : CString( "the loader failed" ) );
                return std::shared_ptr<CLoadedResource>();
            }
            pResource->m_strPath = strPathText;
            return pResource;
        }
        strError = strPathText + ": not found";
        return std::shared_ptr<CLoadedResource>();
    }

        // Under the lock.
    void CResourceManager::Drop( std::map<std::string, CEntry>::iterator it )
    {
        m_nBytes -= it->second.nSize;
        m_listLRU.erase( it->second.itLRU );
        m_mapLoaded.erase( it );
        m_nUnloaded++;
    }

        // Under the lock.
    void CResourceManager::Store( const std::string &strPath, const std::shared_ptr<CLoadedResource> &pResource )
    {
        std::map<std::string, CEntry>::iterator it = m_mapLoaded.find( strPath );
        if ( it != m_mapLoaded.end() )
            Drop( it );
        m_listLRU.push_front( strPath );
        CEntry oEntry;
        oEntry.pResource = pResource;
        oEntry.nSize = pResource->GetMemorySize();
        oEntry.itLRU = m_listLRU.begin();
        m_mapLoaded[ strPath ] = oEntry;
        m_nBytes += oEntry.nSize;
    }

        // Under the lock: the oldest resources only the manager holds go,
        // until it is within its budget (or nothing more can go).
    void CResourceManager::EnforceBudget( void )
    {
        std::list<std::string>::iterator it = m_listLRU.end();
        while ( m_nBytes > m_nBudget && it != m_listLRU.begin() )
        {
            --it;
            std::map<std::string, CEntry>::iterator itEntry = m_mapLoaded.find( *it );
            if ( itEntry->second.pResource.use_count() == 1 )
            {
                ++it;               // past the one being dropped
                Drop( itEntry );
            }
        }
    }

        // Under the lock.
    void CResourceManager::Finish( const std::string &strPath, const std::shared_ptr<CLoadedResource> &pResource, const CString &strError )
    {
        std::map<std::string, std::shared_ptr<CPending> >::iterator it = m_mapPending.find( strPath );
        std::shared_ptr<CPending> pPending = ( it != m_mapPending.end() ) ? it->second : std::shared_ptr<CPending>();
        if ( it != m_mapPending.end() )
            m_mapPending.erase( it );

        if ( pResource )
            Store( strPath, pResource );
        else
        {
            m_nFailed++;
            m_strLastError = strError;
        }

        if ( pPending )
        {
            pPending->bDone = true;
            pPending->pResource = pResource;
            pPending->strError = strError;
            for ( size_t n = 0; n < pPending->aCallbacks.size(); n++ )
            {
                CCompleted oDone;
                oDone.strPath = strPath.c_str();
                oDone.pResource = pResource;
                oDone.strError = strError;
                oDone.fnCallback = pPending->aCallbacks[n];
                m_queueCompleted.push_back( oDone );
            }
        }
        EnforceBudget();
        m_cvDone.notify_all();
    }

    std::shared_ptr<CLoadedResource> CResourceManager::Get( const char *strPath )
    {
        std::string strKey( (const char *)NormalizePath( strPath ) );
        std::unique_lock<std::mutex> lock( m_mutex );
        if ( strKey.empty() )
        {
            m_nFailed++;
            m_strLastError = CString( ( strPath ) ? strPath : "(null)" ) + ": not a valid resource path";
            return std::shared_ptr<CLoadedResource>();
        }

        std::map<std::string, CEntry>::iterator it = m_mapLoaded.find( strKey );
        if ( it != m_mapLoaded.end() )
        {
            m_nHits++;
            m_listLRU.splice( m_listLRU.begin(), m_listLRU, it->second.itLRU );
            return it->second.pResource;
        }

            // Loading already (in the background, or for another Get()):
            // wait for that.
        std::map<std::string, std::shared_ptr<CPending> >::iterator itPending = m_mapPending.find( strKey );
        if ( itPending != m_mapPending.end() )
        {
            std::shared_ptr<CPending> pPending = itPending->second;
            m_cvDone.wait( lock, [&pPending]() { return pPending->bDone; } );
            if ( ! pPending->pResource )
                m_strLastError = pPending->strError;
            return pPending->pResource;
        }

        m_nMisses++;
        m_mapPending[ strKey ] = std::make_shared<CPending>();
        lock.unlock();

        CString strError;
        std::shared_ptr<CLoadedResource> pResource = LoadNow( strKey, strError );

        lock.lock();
        Finish( strKey, pResource, strError );
        return pResource;
    }

    void CResourceManager::LoadAsync( const char *strPath, const Callback &fnCallback )
    {
        std::string strKey( (const char *)NormalizePath( strPath ) );
        std::unique_lock<std::mutex> lock( m_mutex );

        if ( strKey.empty() )
        {
            m_nFailed++;
            CCompleted oDone;
            oDone.strPath = ( strPath ) ? strPath : "";
            oDone.strError = oDone.strPath + ": not a valid resource path";
            m_strLastError = oDone.strError;
            oDone.fnCallback = fnCallback;
            if ( fnCallback )
                m_queueCompleted.push_back( oDone );
            return;
        }

        std::map<std::string, CEntry>::iterator it = m_mapLoaded.find( strKey );
        if ( it != m_mapLoaded.end() )
        {
            m_nHits++;
            m_listLRU.splice( m_listLRU.begin(), m_listLRU, it->second.itLRU );
            if ( fnCallback )
            {
                CCompleted oDone;
                oDone.strPath = strKey.c_str();
                oDone.pResource = it->second.pResource;
                oDone.fnCallback = fnCallback;
                m_queueCompleted.push_back( oDone );
            }
            return;
        }

        std::map<std::string, std::shared_ptr<CPending> >::iterator itPending = m_mapPending.find( strKey );
        if ( itPending != m_mapPending.end() )
        {
            if ( fnCallback )
                itPending->second->aCallbacks.push_back( fnCallback );
            return;
        }

        m_nMisses++;
        std::shared_ptr<CPending> pPending = std::make_shared<CPending>();
        if ( fnCallback )
            pPending->aCallbacks.push_back( fnCallback );
        m_mapPending[ strKey ] = pPending;

        if ( m_aWorkers.empty() )
        {
                // No threads: load now (the callback still waits for
                // ProcessCompleted()).
            lock.unlock();
            CString strError;
            std::shared_ptr<CLoadedResource> pResource = LoadNow( strKey, strError );
            lock.lock();
            Finish( strKey, pResource, strError );
            return;
        }
        m_queueWork.push_back( strKey );
        lock.unlock();
        m_cvWork.notify_one();
    }

    void CResourceManager::WorkerLoop( void )
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        for ( ;; )
        {
            m_cvWork.wait( lock, [this]() { return m_bStopping || ! m_queueWork.empty(); } );
            if ( m_queueWork.empty() )
                return;             // stopping, with nothing left to load
            std::string strPath = m_queueWork.front();
            m_queueWork.pop_front();
            lock.unlock();

            CString strError;
            std::shared_ptr<CLoadedResource> pResource = LoadNow( strPath, strError );

            lock.lock();
            Finish( strPath, pResource, strError );
        }
    }

    size_t CResourceManager::ProcessCompleted( size_t nMax )
    {
        size_t nRun = 0;
        while ( nRun < nMax )
        {
            CCompleted oDone;
            {
                std::lock_guard<std::mutex> lock( m_mutex );
                if ( m_queueCompleted.empty() )
                    break;
                oDone = m_queueCompleted.front();
                m_queueCompleted.pop_front();
            }
            if ( oDone.fnCallback )
                oDone.fnCallback( oDone.strPath, oDone.pResource, oDone.strError );
            nRun++;
        }
            // Callbacks held their resources; the budget may apply now.
        std::lock_guard<std::mutex> lock( m_mutex );
        EnforceBudget();
        return nRun;
    }

    bool CResourceManager::WaitForPending( int nTimeoutMs )
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        if ( nTimeoutMs < 0 )
        {
            m_cvDone.wait( lock, [this]() { return m_mapPending.empty(); } );
            return true;
        }
        return m_cvDone.wait_for( lock, std::chrono::milliseconds( nTimeoutMs ), [this]() { return m_mapPending.empty(); } );
    }

    bool CResourceManager::IsLoaded( const char *strPath ) const
    {
        std::string strKey( (const char *)NormalizePath( strPath ) );
        std::lock_guard<std::mutex> lock( m_mutex );
        return m_mapLoaded.find( strKey ) != m_mapLoaded.end();
    }

    bool CResourceManager::Exists( const char *strPath )
    {
        CString strKey = NormalizePath( strPath );
        if ( strKey.GetLength() == 0 )
            return false;
        std::vector<CSource> aSources;
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            if ( m_mapLoaded.find( std::string( (const char *)strKey ) ) != m_mapLoaded.end() )
                return true;
            aSources = m_aSources;
        }
        for ( size_t n = 0; n < aSources.size(); n++ )
        {
            if ( aSources[n].pSource->Exists( strKey ) )
                return true;
        }
        return false;
    }

    bool CResourceManager::Unload( const char *strPath )
    {
        std::string strKey( (const char *)NormalizePath( strPath ) );
        std::lock_guard<std::mutex> lock( m_mutex );
        std::map<std::string, CEntry>::iterator it = m_mapLoaded.find( strKey );
        if ( it == m_mapLoaded.end() )
            return false;
        Drop( it );
        return true;
    }

    size_t CResourceManager::UnloadUnused( void )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        size_t nDropped = 0;
        std::map<std::string, CEntry>::iterator it = m_mapLoaded.begin();
        while ( it != m_mapLoaded.end() )
        {
            std::map<std::string, CEntry>::iterator itNext = it;
            ++itNext;
            if ( it->second.pResource.use_count() == 1 )
            {
                Drop( it );
                nDropped++;
            }
            it = itNext;
        }
        return nDropped;
    }

    std::shared_ptr<CLoadedResource> CResourceManager::Reload( const char *strPath )
    {
        std::string strKey( (const char *)NormalizePath( strPath ) );
        if ( strKey.empty() )
            return Get( strPath );
        {
            std::unique_lock<std::mutex> lock( m_mutex );
                // A load under way: wait for it, then load again.
            std::map<std::string, std::shared_ptr<CPending> >::iterator itPending = m_mapPending.find( strKey );
            if ( itPending != m_mapPending.end() )
            {
                std::shared_ptr<CPending> pPending = itPending->second;
                m_cvDone.wait( lock, [&pPending]() { return pPending->bDone; } );
            }
            std::map<std::string, CEntry>::iterator it = m_mapLoaded.find( strKey );
            if ( it != m_mapLoaded.end() )
                Drop( it );
        }
        return Get( strKey.c_str() );
    }

    void CResourceManager::SetBudget( size_t nBytes )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        m_nBudget = nBytes;
        EnforceBudget();
    }

    CResourceManager::CStatistics CResourceManager::GetStatistics( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        CStatistics stats;
        stats.nLoaded = m_mapLoaded.size();
        stats.nBytes = m_nBytes;
        stats.nBudget = m_nBudget;
        stats.nPending = m_mapPending.size();
        stats.nHits = m_nHits;
        stats.nMisses = m_nMisses;
        stats.nFailed = m_nFailed;
        stats.nUnloaded = m_nUnloaded;
        return stats;
    }

    CString CResourceManager::GetLastError( void ) const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return m_strLastError;
    }
} // namespace IASLib
