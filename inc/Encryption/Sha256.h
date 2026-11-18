/**
 * @file
 * @brief SHA256 Class.
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

#include <array>
#include <stddef.h>
#include <string>
#include <vector>

namespace IASLib
{ 
    typedef unsigned char uint8;            ///< An 8-bit unsigned value.
    typedef unsigned int uint32;            ///< A 32-bit unsigned value.
    typedef unsigned long long uint64;      ///< A 64-bit unsigned value.

        /// A SHA-256 digest: 32 bytes.
    typedef std::array<unsigned char, 32> CSha256Digest;

    /// The SHA-256 hash function.
    ///
    /// Hash in one call with Digest(), or incrementally with update() and
    /// final(). A context may be copied; one context must not be used from
    /// two threads at once.
    ///
    /// @code
    /// CSha256Digest oDigest = CSHA256::Digest( std::string( "abc" ) );
    /// std::string strHex = CSHA256::ToHex( oDigest );     // "ba7816bf..."
    ///
    /// CSHA256 oHash;
    /// oHash.updateData( "ab", 2 );
    /// oHash.updateData( "c", 1 );
    /// uint8 achDigest[ CSHA256::DIGEST_SIZE ];
    /// oHash.final( achDigest );                           // the same digest
    /// @endcode
    class CSHA256 : public CObject
    {
        protected:
            static const uint32 sha256_k[];                     ///< The 64 round constants.
            static const uint32 SHA224_256_BLOCK_SIZE = (512/8);    ///< The block size in bytes (64).

        public:
            DECLARE_OBJECT( CSHA256, CObject );

                /// Ready to hash.
                /// @note The state used to be left uninitialised until init()
                /// was called.
                CSHA256( void ) { init(); }

            static const uint32 DIGEST_SIZE = ( 256 / 8);      ///< The digest size in bytes (32).

                /// Starts the context over.
                /// Incremental use: update() as often as needed, then final(),
                /// which also starts the context over (init() does too). A
                /// context can be copied part way through to hash several
                /// messages that share a prefix.
            void init();
                /// Adds @p len bytes of @p message to the hash.
            void update(const uint8 *message, uint32 len);
                /// Same, for any length.
            void updateData(const void *pData, size_t nLength);
                /// Writes the DIGEST_SIZE-byte digest to @p digest and starts
                /// the context over.
                /// @note A second final() or update() used to go on from the
                /// padded state.
            void final(uint8 *digest);

                /// The digest of some bytes, at once.
            static CSha256Digest Digest( const void *pData, size_t nLength );
            static CSha256Digest Digest( const std::vector<unsigned char> &aData );     ///< Same, for a vector of bytes.
            static CSha256Digest Digest( const std::string &strData );                  ///< Same, for a string's bytes.
                /// Lower-case hexadecimal (64 characters).
            static std::string   ToHex( const CSha256Digest &digest );
                /// Back from hexadecimal (either case). False for anything but
                /// 64 hexadecimal digits.
            static bool          FromHex( const std::string &strHex, CSha256Digest &digest );

                /// The hexadecimal digest of a string's text. (CString stops at
                /// a NUL byte: use Digest() for binary data.)
            static CString StringDigest(const CString &input);
        
        protected:
                /// Hashes @p block_nb whole blocks of @p message.
            void transform(const uint8 *message, uint32 block_nb);
            uint64  m_tot_len;      ///< bytes hashed so far, in whole blocks
            uint32  m_len;          ///< Bytes waiting in m_block.
            uint8   m_block[2*SHA224_256_BLOCK_SIZE];   ///< Bytes not yet hashed.
            uint32  m_h[8];         ///< The hash state.
    };
    
    
    /// @name SHA-2 helper macros
    /// Bit operations used by the implementation.
    ///@{
    #define SHA2_SHFR(x, n)    (x >> n)                                 ///< Shift right.
    #define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))    ///< Rotate right.
    #define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))    ///< Rotate left.
    #define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))                     ///< The "choose" function.
    #define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))            ///< The "majority" function.
    #define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))  ///< Sigma 0.
    #define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))  ///< Sigma 1.
    #define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))  ///< sigma 0.
    #define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))  ///< sigma 1.
    /// Stores 32-bit @p x big-endian at @p str.
    #define SHA2_UNPACK32(x, str)                 \
    {                                             \
        *((str) + 3) = (uint8) ((x)      );       \
        *((str) + 2) = (uint8) ((x) >>  8);       \
        *((str) + 1) = (uint8) ((x) >> 16);       \
        *((str) + 0) = (uint8) ((x) >> 24);       \
    }
    /// Loads a big-endian 32-bit value from @p str into *@p x.
    #define SHA2_PACK32(str, x)                   \
    {                                             \
        *(x) =   ((uint32) *((str) + 3)      )    \
               | ((uint32) *((str) + 2) <<  8)    \
               | ((uint32) *((str) + 1) << 16)    \
               | ((uint32) *((str) + 0) << 24);   \
    }
    ///@}

} // namespace IASLib

#endif // IASLIB_SHA256_H__
