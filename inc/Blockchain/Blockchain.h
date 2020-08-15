/**
 *  Blockchain Class
 *
 *      This class provides the main basis for building a blockchain using a 
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

#ifndef IASLIB_BLOCKCHAIN_H__
#define IASLIB_BLOCKCHAIN_H__

#include "BaseTypes/String_.h"
#include "BaseTypes/Date.h"
#include "Collections/Array.h"
#include "Blockchain/Block.h"

namespace IASLib
{
    class CBlockchain : public CObject
    {
        private:
            __uint32_t m_nDifficulty;
            CArray   m_aChain;

        public:
            DECLARE_OBJECT( CBlockchain, CObject );

            CBlockchain();
            CBlockchain( __uint32_t difficulty );

            void addBlock(CBlock *bNew);

        private:
            CBlock *getLastBlock() const;
    };
} // namespace IASLib

#endif // IASLIB_BLOCKCHAIN_H__
