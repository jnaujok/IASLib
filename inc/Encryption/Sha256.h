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
    typedef unsigned char uint8;        ///< An 8-bit unsigned byte.
    typedef unsigned int uint32;        ///< A 32-bit unsigned word.
    typedef unsigned long long uint64;  ///< A 64-bit unsigned word.

    /// A SHA-256 digest: 32 bytes.
    typedef std::array<unsigned char, 32> CSha256Digest;

    /// SHA-256 message digest (FIPS 180-4).
    ///
    /// Use the static Digest() functions to hash data in one call, or a
    /// context object for data that arrives in pieces. A context is a plain
    /// value with no locking: use one per thread.
    ///
    /// @code
    /// std::string strHex = CSHA256::ToHex( CSHA256::Digest( std::string( "abc" ) ) );
    /// // "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
    ///
    /// CSHA256 ctx;
    /// uint8 aDigest[ CSHA256::DIGEST_SIZE ];
    /// ctx.init();
    /// ctx.updateData( pPart1, nPart1 );
    /// ctx.updateData( pPart2, nPart2 );
    /// ctx.final( aDigest );
    /// @endcode
    class CSHA256 : public CObject
    {
        protected:
            static const uint32 sha256_k[];     ///< The 64 SHA-256 round constants.
            static const uint32 SHA224_256_BLOCK_SIZE = (512/8);   ///< Block size in bytes (64).

        public:
            DECLARE_OBJECT( CSHA256, CObject );

            static const uint32 DIGEST_SIZE = ( 256 / 8);   ///< Digest size in bytes (32).

            /// @name Incremental hashing
            /// Incremental use: init(), update() as often as needed, final().
            /// A context can be copied part way through to hash several
            /// messages that share a prefix.
            ///@{

            /// Starts a new digest. Must be called before the first update.
            void init();
            /// Adds bytes to the digest.
            /// @param message The bytes.
            /// @param len Number of bytes (a 32-bit count; use updateData() for more).
            void update(const uint8 *message, uint32 len);
            /// Adds bytes to the digest; any length (fed to update() in 1 GiB pieces).
            /// @param pData The bytes.
            /// @param nLength Number of bytes.
            void updateData(const void *pData, size_t nLength);     // any length
            /// Finishes the digest. Call init() again before reusing the context.
            /// @param digest Receives DIGEST_SIZE (32) bytes.
            void final(uint8 *digest);
            ///@}

            /// The digest of some bytes, at once.
            /// @param pData The bytes (may be NULL if @p nLength is 0).
            /// @param nLength Number of bytes.
            static CSha256Digest Digest( const void *pData, size_t nLength );
            /// The digest of a byte vector.
            static CSha256Digest Digest( const std::vector<unsigned char> &aData );
            /// The digest of a string's bytes (embedded NULs included).
            static CSha256Digest Digest( const std::string &strData );
            /// Lower-case hexadecimal (64 characters).
            static std::string   ToHex( const CSha256Digest &digest );
            /// Parses 64 hexadecimal digits (either case) back into a digest.
            /// @param strHex The hex text.
            /// @param digest Receives the bytes. It may be partly written when
            ///        the result is false.
            /// @return False for anything but 64 hexadecimal digits.
            static bool          FromHex( const std::string &strHex, CSha256Digest &digest );

            /// The hexadecimal digest of a string's text. (CString stops at
            /// a NUL byte: use Digest() for binary data.)
            /// @return 64 lower-case hexadecimal characters.
            static CString StringDigest(const CString &input);

        protected:
            /// Runs the compression function over @p block_nb 64-byte blocks.
            void transform(const uint8 *message, uint32 block_nb);
            uint64  m_tot_len;      ///< Bytes hashed so far, in whole blocks.
            uint32  m_len;          ///< Bytes waiting in m_block.
            uint8   m_block[2*SHA224_256_BLOCK_SIZE];   ///< Partial block buffer (two blocks, for padding).
            uint32  m_h[8];         ///< Current hash state.
    };
    
    
    /// @name SHA-256 helper macros
    /// Internal bit operations used by CSHA256::transform() and final().
    ///@{
    #define SHA2_SHFR(x, n)    (x >> n)                                     ///< Shift right.
    #define SHA2_ROTR(x, n)   ((x >> n) | (x << ((sizeof(x) << 3) - n)))   ///< Rotate right.
    #define SHA2_ROTL(x, n)   ((x << n) | (x >> ((sizeof(x) << 3) - n)))   ///< Rotate left.
    #define SHA2_CH(x, y, z)  ((x & y) ^ (~x & z))                         ///< Ch (choose) function.
    #define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))                ///< Maj (majority) function.
    #define SHA256_F1(x) (SHA2_ROTR(x,  2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))   ///< Big sigma 0.
    #define SHA256_F2(x) (SHA2_ROTR(x,  6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))   ///< Big sigma 1.
    #define SHA256_F3(x) (SHA2_ROTR(x,  7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x,  3))   ///< Small sigma 0.
    #define SHA256_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))   ///< Small sigma 1.
    /// Stores 32-bit word @p x big-endian at byte pointer @p str.
    #define SHA2_UNPACK32(x, str)                 \
    {                                             \
        *((str) + 3) = (uint8) ((x)      );       \
        *((str) + 2) = (uint8) ((x) >>  8);       \
        *((str) + 1) = (uint8) ((x) >> 16);       \
        *((str) + 0) = (uint8) ((x) >> 24);       \
    }
    /// Loads a big-endian 32-bit word from byte pointer @p str into *@p x.
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
