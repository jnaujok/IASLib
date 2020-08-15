#include "Blockchain/Block.h"
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
        char cstr[nDifficulty + 1];
        for (uint32_t i = 0; i < nDifficulty; ++i) 
        {
            cstr[i] = '0';
        }
        cstr[nDifficulty] = '\0';

        CString str(cstr);

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
        ss += _nNonce;
        ss += sPrevHash;

        return CSHA256::StringDigest( ss );
    }

};
