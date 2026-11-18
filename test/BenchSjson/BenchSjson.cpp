/***********************************************************************
**  BenchSjson
**
**  Measures CSjson on message streams exported by the Python sjson
**  benchmarks (benchmarks/export_for_cpp.py in the sjson repository), and
**  prints the C++ results beside the Python library's for the same
**  configurations: bytes per message (packets plus NAK traffic) and
**  encode / decode time per message. Every decoded message is checked
**  against the original.
**
**      BenchSjson [--xsd schema.xsd] dataset.json [dataset.json ...]
**
**  Configurations: "learnt" (no setup), "schema" (needs --xsd: the schema
**  is loaded here), "samples dict" (the dataset's compiled dictionary),
**  each also with ", stream" for stream mode. One sender per stream id.
***********************************************************************/

#include "IASLib.h"

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace IASLib;

namespace
{
    std::string readFile( const std::string &strPath )
    {
        std::ifstream file( strPath.c_str(), std::ios::binary );
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string directoryOf( const std::string &strPath )
    {
        size_t n = strPath.find_last_of( "/\\" );
        return n == std::string::npos ? std::string( "." ) : strPath.substr( 0, n );
    }

    struct Result
    {
        size_t  nBytes = 0, nNak = 0, nOk = 0, nMessages = 0;
        double  dEncode = 0, dDecode = 0;
        std::string strError;
    };

    Result run( const CJsonNode &messages, bool bStateful, const CSjsonDictionary *pDictionary )
    {
        typedef std::chrono::steady_clock Clock;
        Result result;
        std::map<std::string, std::unique_ptr<CSjson> > mapSenders;
        CSjson receiver( 0xFFFF );
        if ( pDictionary )
            receiver.registerDictionary( *pDictionary );
        for ( int n = 0; n < messages.size(); n++ )
        {
            const CJsonNode &entry = messages.path( n );
            std::string strSource = entry.path( 0 ).textString();
            const CJsonNode &document = entry.path( 1 );
            std::unique_ptr<CSjson> &pSender = mapSenders[ strSource ];
            if ( ! pSender )
            {
                pSender.reset( new CSjson( (unsigned)mapSenders.size(), bStateful ) );
                if ( pDictionary )
                    pSender->setDictionary( *pDictionary );
            }
            unsigned nSender = pSender->senderId();
            std::vector<unsigned char> packet;
            Clock::time_point t0 = Clock::now();
            bool bEncoded = pSender->encode( document, packet );
            Clock::time_point t1 = Clock::now();
            result.dEncode += std::chrono::duration<double>( t1 - t0 ).count();
            if ( ! bEncoded )
            {
                result.strError = pSender->lastErrorText();
                break;
            }
            result.nBytes += packet.size();
            t0 = Clock::now();
            CJsonNode *pValue = receiver.decode( packet.data(), packet.size() );
            if ( pValue == NULL && receiver.lastError() == SJSON_ERROR_UNKNOWN_TAG )
            {
                std::vector<unsigned char> update = pSender->nakDictionary( nSender, receiver.lastKnownTag( nSender ), receiver.lastKnownValue( nSender ) );
                result.nNak += update.size();
                receiver.loadDictionary( update.data(), update.size() );
                pValue = receiver.decode( packet.data(), packet.size() );
            }
            t1 = Clock::now();
            result.dDecode += std::chrono::duration<double>( t1 - t0 ).count();
            std::unique_ptr<CJsonNode> pOwned( pValue );
            if ( pValue && SjsonSchema::pythonJson( pValue ) == SjsonSchema::pythonJson( &document ) )
                result.nOk++;
            else if ( result.strError.empty() )
                result.strError = pValue ? "decoded value differs" : receiver.lastErrorText();
            result.nMessages++;
        }
        result.nBytes += result.nNak;
        return result;
    }
}

int main( int argc, char **argv )
{
    std::string strXsd;
    std::vector<std::string> aDatasets;
    for ( int n = 1; n < argc; n++ )
    {
        if ( strcmp( argv[ n ], "--xsd" ) == 0 && n + 1 < argc )
            strXsd = argv[ ++n ];
        else
            aDatasets.push_back( argv[ n ] );
    }
    std::shared_ptr<const CSjsonSchemaModel> pSchema;
    std::unique_ptr<CSjsonDictionary> pSchemaDictionary;
    if ( ! strXsd.empty() )
    {
        std::chrono::steady_clock::time_point t0 = std::chrono::steady_clock::now();
        pSchema.reset( SjsonSchema::loadSchema( strXsd ) );
        pSchemaDictionary.reset( CSjsonDictionary::fromSchema( pSchema ) );
        double dLoad = std::chrono::duration<double>( std::chrono::steady_clock::now() - t0 ).count();
        printf( "Schema %s: %zu names, %zu enum strings, fingerprint %08x, loaded in %.2f s\n", strXsd.c_str(),
                pSchema->fieldNames().size(), pSchema->enumStrings().size(), pSchema->fingerprint(), dLoad );
    }
    printf( "LZ4 %s, zstd %s\n", SjsonUtil::lz4Available() ? "on" : "off", CSjson::zstdAvailable() ? "on" : "off" );

    int nFailures = 0;
    for ( size_t d = 0; d < aDatasets.size(); d++ )
    {
        std::unique_ptr<CJsonNode> pData( CJsonParser::parse( readFile( aDatasets[ d ] ) ) );
        if ( ! pData )
        {
            printf( "cannot read %s\n", aDatasets[ d ].c_str() );
            return 2;
        }
        const CJsonNode &messages = pData->path( "messages" );
        const CJsonNode &python = pData->path( "python" );
        double dJson = (double)pData->path( "json_bytes" ).asLong() / messages.size();
        printf( "\n%s: %d messages, %.0f bytes of JSON each\n", pData->path( "name" ).textString().c_str(), messages.size(), dJson );
        printf( "  %-22s %10s %10s %7s | %9s %9s %8s | %9s %9s %8s\n", "configuration", "C++ B/msg", "Py B/msg", "size",
                "C++ enc", "Py enc", "faster", "C++ dec", "Py dec", "faster" );
        std::unique_ptr<CSjsonDictionary> pSamples;
        if ( pData->path( "samples_dictionary" ).isTextual() )
            pSamples.reset( CSjsonDictionary::load( directoryOf( aDatasets[ d ] ) + "/" + pData->path( "samples_dictionary" ).textString() ) );
        const CObjectNode &configs = static_cast<const CObjectNode &>( python );
        for ( int c = 0; c < configs.size(); c++ )
        {
            CString strConfigName = configs.fieldName( c );
            std::string strConfig( (const char *)strConfigName, strConfigName.GetLength() );
            bool bStream = strConfig.find( ", stream" ) != std::string::npos;
            const CSjsonDictionary *pDictionary = NULL;
            if ( strConfig.compare( 0, 6, "schema" ) == 0 )
            {
                if ( ! pSchemaDictionary )
                    continue;
                pDictionary = pSchemaDictionary.get();
            }
            else if ( strConfig.compare( 0, 7, "samples" ) == 0 )
                pDictionary = pSamples.get();
            Result r = run( messages, bStream, pDictionary );
            const CJsonNode &py = *configs.fieldValue( c );
            double nCount = (double)r.nMessages;
            double dPyBytes = (double)py.path( "bytes" ).asLong() / messages.size();
            double dPyEnc = 1e6 * py.path( "encode_s" ).asDouble() / messages.size(), dPyDec = 1e6 * py.path( "decode_s" ).asDouble() / messages.size();
            double dEnc = 1e6 * r.dEncode / nCount, dDec = 1e6 * r.dDecode / nCount;
            printf( "  %-22s %10.1f %10.1f %+6.1f%% | %7.1fus %7.0fus %7.0fx | %7.1fus %7.0fus %7.0fx%s%s\n", strConfig.c_str(),
                    r.nBytes / nCount, dPyBytes, 100.0 * ( r.nBytes / nCount - dPyBytes ) / dPyBytes,
                    dEnc, dPyEnc, dPyEnc / dEnc, dDec, dPyDec, dPyDec / dDec,
                    r.nOk == (size_t)messages.size() ? "" : "  CHECK FAILED: ", r.nOk == (size_t)messages.size() ? "" : r.strError.c_str() );
            if ( r.nOk != (size_t)messages.size() )
                nFailures++;
        }
    }
    return nFailures ? 1 : 0;
}
