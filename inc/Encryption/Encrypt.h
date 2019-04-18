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

#define PASSWORD_PAD "\041\105\163\160\162\145\163\163\157\040\123\166\143\153\163\041"

namespace IASLib
{
    class CEncrypt : public CObject
    {
        public:
                            CEncrypt( void );
            virtual        ~CEncrypt( void );

            const char     *Encrypt( const char *strPlainText, const char *strKey );
            const char     *Decrypt( const char *strCypherText, const char *strKey );

            CString         BlockToString( const char *strBlock );
            const char     *StringToBlock( const char *strBlock );

        private:
            unsigned long  *ConvertToBlocks( const unsigned char *strText );
            const char     *ConvertFromBlock( unsigned long *aBlock );
            void            XORBlocks( unsigned long *aTarget, unsigned long *aSource );
            int             RotPrime( int nRound, int nSlot, int nBlock );
            unsigned long   RotLeft( unsigned long nValue, int nSlots );
            unsigned long   RotRight( unsigned long nValue, int nSlots );
            void            RotateKey( unsigned long *aTarget, unsigned long *aSource, int nRound );
            CString         HexChar( unsigned long nValue );
            CString         BlockToString( unsigned long *aBlock );
    };
} // namespace IASLIB

#endif // IASLIB_ENCRYPT_H__
