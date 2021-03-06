//-----------------------------------------------------------------------------------
//
//   Torque Network Library
//   Copyright (C) 2004 GarageGames.com, Inc.
//   For more information see http://www.opentnl.org
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   For use in products that are not compatible with the terms of the GNU 
//   General Public License, alternative licensing options are available 
//   from GarageGames.com.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#include "tnl.h"
#include "tnlSymmetricCipher.h"
#include "tnlByteBuffer.h"
#include "tomcrypt.h"

namespace TNL {

SymmetricCipher::SymmetricCipher(const U8 symmetricKey[SymmetricCipher::KeySize], const U8 initVector[SymmetricCipher::BlockSize])
{
   rijndael_setup(symmetricKey, KeySize, 0, (symmetric_key *) &mSymmetricKey);
   memcpy(mInitVector, initVector, BlockSize);
   memcpy(mCounter, initVector, BlockSize);
   rijndael_ecb_encrypt((U8 *) mCounter, mPad, (symmetric_key *) &mSymmetricKey);
   mPadLen = 0;
}

static void copyWrapBuffer(U8 *destBuffer, U32 destSize, const U8 *srcBuffer, U32 srcSize)
{
   if(!srcSize)
   {
      memset(destBuffer, 0, destSize);
      return;
   }
   U32 copyPos = 0;
   while(copyPos < destSize)
   {
      U32 copyBytes = getMin(destSize - copyPos, srcSize);
      memcpy(destBuffer + copyPos, srcBuffer, copyBytes);
      copyPos += copyBytes;
   }
}

SymmetricCipher::SymmetricCipher(const ByteBuffer *theByteBuffer)
{
   // if the buffer passed isn't exactly the right size,
   // wrap copy it into the dest buffer and init vector.
   if(theByteBuffer->getBufferSize() != KeySize * 2)
   {
      U8 buffer[KeySize * 2];
      memset(buffer, 0, KeySize * 2);
      memcpy(buffer, theByteBuffer->getBuffer(), getMin(U32(KeySize * 2), theByteBuffer->getBufferSize()));

      rijndael_setup(buffer, KeySize, 0, (symmetric_key *) &mSymmetricKey);
      memcpy(mInitVector, buffer + KeySize, BlockSize);
   }
   else
   {
      rijndael_setup(theByteBuffer->getBuffer(), KeySize, 0, (symmetric_key *) &mSymmetricKey);
      memcpy(mInitVector, theByteBuffer->getBuffer() + KeySize, BlockSize);
   }
   memcpy(mCounter, mInitVector, BlockSize);
   rijndael_ecb_encrypt((U8 *) mCounter, mPad, (symmetric_key *) &mSymmetricKey);
   mPadLen = 0;
}

void SymmetricCipher::setupCounter(U32 counterValue1, U32 counterValue2, U32 counterValue3, U32 counterValue4)
{   
   mCounter[0] = convertHostToLEndian(convertLEndianToHost(mInitVector[0]) + counterValue1);
   mCounter[1] = convertHostToLEndian(convertLEndianToHost(mInitVector[1]) + counterValue2);
   mCounter[2] = convertHostToLEndian(convertLEndianToHost(mInitVector[2]) + counterValue3);
   mCounter[3] = convertHostToLEndian(convertLEndianToHost(mInitVector[3]) + counterValue4);

   rijndael_ecb_encrypt((U8 *) mCounter, mPad, (symmetric_key *) &mSymmetricKey);
   mPadLen = 0;
}

void SymmetricCipher::encrypt(const U8 *plainText, U8 *cipherText, U32 len)
{
   while(len-- > 0)
   {
      if(mPadLen == BlockSize)
      {
         // we've reached the end of the pad, so compute a new pad
         rijndael_ecb_encrypt(mPad, mPad, (symmetric_key *) &mSymmetricKey);
         mPadLen = 0;
      }
      U8 encryptedChar = *plainText++ ^ mPad[mPadLen];
      mPad[mPadLen++] = *cipherText++ = encryptedChar;
   }
}

void SymmetricCipher::decrypt(const U8 *cipherText, U8 *plainText, U32 len)
{
   while(len-- > 0)
   {
      if(mPadLen == BlockSize)
      {
         rijndael_ecb_encrypt(mPad, mPad, (symmetric_key *) &mSymmetricKey);
         mPadLen = 0;
      }
      U8 encryptedChar = *cipherText++;
      *plainText++ = encryptedChar ^ mPad[mPadLen];
      mPad[mPadLen++] = encryptedChar;
   }
}

};
