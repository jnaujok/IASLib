/**
 * @file
 * @brief Encrypt Class: declarations of an ad-hoc 128-bit block cypher.
 *
 * @warning This header has declarations only. There is no implementation
 * of CEncrypt anywhere in the library (no Encrypt.cpp), so any use of the
 * class fails to link. The description in the original header below says
 * what the algorithm was meant to do.
 */
/***********************************************************************
**  Encrypt Class
**
**  Description:
**      This module provides an ad-hoc multi-round block cypher to encrypt a string. Since this
** cypher uses a 128 bit key, it is illegal to export this code outside of the United States
** and Canada. 
**      The encryption method converts the plain-text and the key into 128 bit quad-blocks to 
** perform the encryption. It also creates an s-box from the key. The S-Box is the actual cypher
** box used to encrypt the plain text.
**      Each quad-block is then padded, the plain text and the key with the same pad value, the
** s-box with a different padding value.
**      The algorithm then proceeds to a series of rounds. In each round, the S-Box and Key are
** permutated (see the RotateKey function) to provide the encryption for the round. The S-Box is
** then XORed against the plain text. At this point, the round is complete, and we proceed to the
** next round with the new values.
**      When the rounds are completed, the "plain-text" quad-block is coverted back to a text string.
** This value is the encrypted text. This value is then returned from the function. It is highly likely
** that this value will contain *many* un-printable characters.
**      The BlockToString function and StringToBlock function can be used to convert these unprintable
** cypher-texts to and from a hex-encoded value of 32 characters. This can be used for storage of the
** values where printable text is a requirement. (Inserting into a database for instance.)
**      Decryption is similar to Encryption done in reverse. The main difference is that we must
** first generate all of the S-Boxes in order to apply them in reverse order.
**
**      This algorithm was invented and developed by Jeffrey R. Naujok. The algorithm is under research
** for patent by Jeffrey R. Naujok and is used by permission of the author.
**
**  The code in this module is Copyright (C), 1999 BlackStar Programming.
**
**  $AUTHOR$
**  $LOG$
**
***********************************************************************/

#ifndef IASLIB_ENCRYPT_H__
#define IASLIB_ENCRYPT_H__

#include "../BaseTypes/Object.h"
#include "../BaseTypes/String_.h"

/// Sixteen-byte pad string ("!Espresso Svcks!") meant for padding keys and
/// plain text to a full block.
#define PASSWORD_PAD "\041\105\163\160\162\145\163\163\157\040\123\166\143\153\163\041"

namespace IASLib
{
    /// An ad-hoc multi-round 128-bit block cypher (declarations only).
    ///
    /// @warning Not implemented. Only these declarations exist; there is no
    /// source file for them, so the class cannot be used. It is not a
    /// reviewed cypher either: use a real cryptographic library instead.
    ///
    /// As designed, the plain text and key are turned into 128-bit
    /// quad-blocks, an S-box is built from the key, and each round permutes
    /// the S-box and key and XORs the S-box into the text. The result may
    /// hold unprintable bytes; BlockToString() and StringToBlock() were to
    /// convert it to and from 32 hexadecimal characters.
    class CEncrypt : public CObject
    {
        public:
                            /// Constructor (not implemented).
                            CEncrypt( void );
            /// Destructor (not implemented).
            virtual        ~CEncrypt( void );

            /// Was to encrypt one block of text with a key (not implemented).
            /// @param strPlainText The text to encrypt.
            /// @param strKey The key.
            /// @return The cypher text, which may hold unprintable bytes.
            const char     *Encrypt( const char *strPlainText, const char *strKey );
            /// Was to decrypt one block of cypher text with a key (not implemented).
            /// @param strCypherText The cypher text from Encrypt().
            /// @param strKey The key used to encrypt it.
            /// @return The plain text.
            const char     *Decrypt( const char *strCypherText, const char *strKey );

            /// Was to hex-encode a cypher block into 32 characters (not implemented).
            CString         BlockToString( const char *strBlock );
            /// Was to decode 32 hex characters back into a cypher block (not implemented).
            const char     *StringToBlock( const char *strBlock );

        private:
            /// Text to a 128-bit quad-block (not implemented).
            unsigned long  *ConvertToBlocks( const unsigned char *strText );
            /// Quad-block back to text (not implemented).
            const char     *ConvertFromBlock( unsigned long *aBlock );
            /// XORs @p aSource into @p aTarget (not implemented).
            void            XORBlocks( unsigned long *aTarget, unsigned long *aSource );
            /// Rotation amount for a round, slot and block (not implemented).
            int             RotPrime( int nRound, int nSlot, int nBlock );
            /// Rotates bits left (not implemented).
            unsigned long   RotLeft( unsigned long nValue, int nSlots );
            /// Rotates bits right (not implemented).
            unsigned long   RotRight( unsigned long nValue, int nSlots );
            /// Permutes the key or S-box for a round (not implemented).
            void            RotateKey( unsigned long *aTarget, unsigned long *aSource, int nRound );
            /// Hex text for one word (not implemented).
            CString         HexChar( unsigned long nValue );
            /// Hex text for a quad-block (not implemented).
            CString         BlockToString( unsigned long *aBlock );
    };
} // namespace IASLIB

#endif // IASLIB_ENCRYPT_H__
