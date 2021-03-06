/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtomcrypt.org
 */
#include "tomcrypt.h"

/**
   @file dsa_export.c
   DSA implementation, export key, Tom St Denis
*/

#ifdef MDSA

/**
  Export a DSA key to a binary packet
  @param out    [out] Where to store the packet
  @param outlen [in/out] The max size and resulting size of the packet
  @param type   The type of key to export (PK_PRIVATE or PK_PUBLIC)
  @param key    The key to export
  @return CRYPT_OK if successful
*/
int dsa_export(unsigned char *out, unsigned long *outlen, int type, dsa_key *key)
{
   unsigned long y, z;
   int err;

   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);
   LTC_ARGCHK(key    != NULL);

   /* can we store the static header?  */
   if (*outlen < (PACKET_SIZE + 1 + 2)) {
      return CRYPT_BUFFER_OVERFLOW;
   }
   
   if (type == PK_PRIVATE && key->type != PK_PRIVATE) {
      return CRYPT_PK_TYPE_MISMATCH;
   }

   if (type != PK_PUBLIC && type != PK_PRIVATE) {
      return CRYPT_INVALID_ARG;
   }

   /* store header */
   packet_store_header(out, PACKET_SECT_DSA, PACKET_SUB_KEY);
   y = PACKET_SIZE;

   /* store g, p, q, qord */
   out[y++] = type;
   out[y++] = (key->qord>>8)&255;
   out[y++] = key->qord & 255;

   OUTPUT_BIGNUM(&key->g,out,y,z);
   OUTPUT_BIGNUM(&key->p,out,y,z);
   OUTPUT_BIGNUM(&key->q,out,y,z);

   /* public exponent */
   OUTPUT_BIGNUM(&key->y,out,y,z);
   
   if (type == PK_PRIVATE) {
      OUTPUT_BIGNUM(&key->x,out,y,z);
   }

   *outlen = y;
   return CRYPT_OK;
}

#endif

