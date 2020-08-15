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


#include "Blockchain/Blockchain.h"
#include "Blockchain/Block.h"

namespace IASLib
{
    CBlockchain::CBlockchain() 
    {
        m_aChain.Append( new CBlock(0, "Genesis Block"));
        m_nDifficulty = 6;
    }

    CBlockchain::CBlockchain( __uint32_t difficulty ) 
    {
        m_aChain.Append( new CBlock(0, "Genesis Block"));
        m_nDifficulty = difficulty;
    }

    void CBlockchain::addBlock(CBlock *bNew) 
    {
        bNew->setPreviousHash( getLastBlock()->getHash() );
        bNew->mineBlock(m_nDifficulty);
        m_aChain.Append(bNew);
    }

    CBlock *CBlockchain::getLastBlock() const 
    {
        return (CBlock *)m_aChain[ m_aChain.GetLength() - 1];
    }
}
