#include "Blockchain/Block.h"
#include <string>
#include "Encryption/Sha256.h"

namespace IASLib
{
    CBlock::CBlock(uint32_t nIndexIn, const CString &sDataIn) : _nIndex(nIndexIn), _sData(sDataIn), _tTime()
    {
        _nNonce = -1;
    }

    CString CBlock::getHash() 
    {
        return _sHash;
    }

    void CBlock::mineBlock(uint32_t nDifficulty) 
    {
        std::string strZeros( nDifficulty, '0' );
        CString str( strZeros.c_str() );

        do 
        {
            _nNonce++;
            _sHash = calculateHash();
        } while (_sHash.Substring(0, nDifficulty) != str);

        printf( "Block mined: %s\n", (const char *)_sHash );
    }

    CString CBlock::calculateHash() const 
    {
        CString ss;
        ss = _nIndex;
        ss += _tTime.FormatDate( CDate::DF_ISO_8601_MS_PACKED);
        ss += _sData;
        ss += std::to_string( (long long)_nNonce ).c_str();     // int64_t is long or long long by platform
        ss += sPrevHash;

        return CSHA256::StringDigest( ss );
    }

};
