/**
 *  Block Class
 *
 *      This class provides the basis for building a blockchain using a 
 * series of hash keyed data blocks. 
 *      Each block can be used to store data and is part of a blockchain 
 * that maintains a history of signatures to ensure that data is not changed
 * or mutated over time. 
 *      This basic block is similar to the original BitCoin blockchain
 * design. 
 *
 *	Author: Jeffrey R. Naujok
 *	Created: 12/20/2019
 *
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_BLOCK_H__
#define IASLIB_BLOCK_H__

#include "BaseTypes/String_.h"
#include "BaseTypes/Date.h"

namespace IASLib
{

    typedef signed char int8_t;
    typedef unsigned char uint8_t;
    typedef signed short int int16_t;
    typedef unsigned short int uint16_t;
    typedef signed int int32_t;
    typedef unsigned int uint32_t;
    #if __WORDSIZE == 64
    typedef signed long int int64_t;
    typedef unsigned long int uint64_t;
    #else
    __extension__ typedef signed long long int int64_t;
    __extension__ typedef unsigned long long int uint64_t;
    #endif

    class CBlock : public CObject
    {
        private:
            CString sPrevHash;

            uint32_t _nIndex;
            int64_t _nNonce;
            CString _sData;
            CString _sHash;
            CDate   _tTime;
        
        public:
            DECLARE_OBJECT( CBlock, CObject );

            CBlock(uint32_t nIndexIn, const CString &sDataIn);

            CString getPrevHash( void ) { return sPrevHash; }
        
            CString getHash( void );
        
            void mineBlock(uint32_t nDifficulty);

        private:
            CString calculateHash() const;
            friend class CBlockchain;
            void setPreviousHash( CString hash ) { sPrevHash = hash; }
    };
} // namespace IASLib

#endif // IASLIB_BLOCK_H__
