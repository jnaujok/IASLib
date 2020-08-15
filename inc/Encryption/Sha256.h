/**
 * SHA256 Class
 *
 * This class provides an implementation of the SHA256 digest class that
 * can be used to provide a one-way hash function that complies with the 
 * SHA256 standard. 
 *
 * Author: Jeffrey R. Naujok
 * Created: December 20, 2019
 * 
 * Copyright (C) 2019, The Irene Adler Software Group, all rights reserved.
 * [A division of BlackStar Enterprises, LLC.]
 */

#ifndef IASLIB_SHA256_H__
#define IASLIB_SHA256_H__

#include "BaseTypes/Object.h"
#include "BaseTypes/String_.h"

namespace IASLib
{ 
    typedef unsigned char uint8;
    typedef unsigned int uint32;
    typedef unsigned long long uint64;

    class CSHA256 : public CObject
    {
        protected:
            static const uint32 sha256_k[];
            static const uint32 SHA224_256_BLOCK_SIZE = (512/8);

        public:
            DECLARE_OBJECT( CSHA256, CObject );

            static const uint32 DIGEST_SIZE = ( 256 / 8);

            void init();
            void update(const uint8 *message, uint32 len);
            void final(uint8 *digest);

            static CString StringDigest(CString &input);
        
        protected:
            void transform(const uint8 *message, uint32 block_nb);
            uint32  m_tot_len;
            uint32  m_len;
            uint8   m_block[2*SHA224_256_BLOCK_SIZE];
            uint32  m_h[8];
    };
    
    
    #define SHA2_SHFR(x, n)    (x >> n)
    #define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))
    #define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))
    #define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))
    #define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
    #define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
    #define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
    #define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))
    #define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
    #define SHA2_UNPACK32(x, str)                 \
    {                                             \
        *((str) + 3) = (uint8) ((x)      );       \
        *((str) + 2) = (uint8) ((x) >>  8);       \
        *((str) + 1) = (uint8) ((x) >> 16);       \
        *((str) + 0) = (uint8) ((x) >> 24);       \
    }
    #define SHA2_PACK32(str, x)                   \
    {                                             \
        *(x) =   ((uint32) *((str) + 3)      )    \
               | ((uint32) *((str) + 2) <<  8)    \
               | ((uint32) *((str) + 1) << 16)    \
               | ((uint32) *((str) + 0) << 24);   \
    }

} // namespace IASLib

#endif // IASLIB_SHA256_H__
