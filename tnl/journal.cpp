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

#include "tnlJournal.h"
#include "tnlEndian.h"

namespace TNL
{

JournalEntryRecord *JournalEntryRecord::mList = NULL;

bool Journal::mInsideEntrypoint = false;
Journal::Mode Journal::mCurrentMode = Journal::Inactive;

FILE *Journal::mJournalFile = NULL;
BitStream Journal::mWriteStream;
BitStream Journal::mReadStream;
Journal *Journal::mJournal = NULL;
U32 Journal::mWritePosition = 0;

Journal::Journal()
{
   TNLAssert(mJournal == NULL, "Cannot construct more than one Journal instance.");
   mJournal = this;
}

// the journal stream is written as a single continuous bit stream.
// the first four bytes of the stream are the total number of bits in
// the stream.  As a block is written, the bits in the write stream are
// all erased except for those in the last byte, which are moved to the first.
void Journal::syncWriteStream()
{
   if(mWriteStream.getBytePosition() == 0)
      return;

   U32 totalBits = (mWritePosition << 3) + mWriteStream.getBitPosition();
   
   // seek back to the beginning
   fseek(mJournalFile, 0, SEEK_SET);

   // write the new total bits
   U32 writeBits = convertHostToLEndian(totalBits);
   fwrite(&writeBits, 1, sizeof(U32), mJournalFile);

   // seek to the writing position
   fseek(mJournalFile, mWritePosition, SEEK_SET);

   U32 bytesToWrite = mWriteStream.getBytePosition();
   // write the bytes to the file
   fwrite(mWriteStream.getBuffer(), 1, bytesToWrite, mJournalFile);

   // adjust the write stream
   if(totalBits & 7)
   {
      U8 *buffer = mWriteStream.getBuffer();
      buffer[0] = buffer[bytesToWrite - 1];
      mWriteStream.setBitPosition(totalBits & 7);
      mWritePosition += bytesToWrite - 1;
   }
   else
   {
      mWritePosition += bytesToWrite;
      mWriteStream.setBitPosition(0);
   }
}

void Journal::record(const char *fileName)
{
   mJournalFile = fopen(fileName, "wb");
   if(mJournalFile)
   {
      mCurrentMode = Record;
      mWritePosition = sizeof(U32);
   }
}

void Journal::load(const char *fileName)
{
   FILE *theJournal = fopen(fileName, "rb");
   if(!theJournal)
      return;

   fseek(theJournal, 0, SEEK_END);
   U32 fileSize = ftell(theJournal);
   fseek(theJournal, 0, SEEK_SET);

   mReadStream.resize(fileSize);
   U32 bitCount;
   fread(mReadStream.getBuffer(), 1, fileSize, theJournal);
   mReadStream.read(&bitCount);
   mReadStream.setMaxBitSizes(bitCount);
   fclose(theJournal);
   mCurrentMode = Playback;
}

void Journal::callEntry(const char *funcName, MarshalledCall *theCall)
{
   if(mCurrentMode == Playback)
      return;

   TNLAssert(mInsideEntrypoint == false, "Journal entries cannot be reentrant!");
   mInsideEntrypoint = true;
   if(mCurrentMode == Record)
   {
      mWriteStream.writeString(funcName);
      mWriteStream.writeBits(theCall->marshalledData.getBitPosition(), theCall->marshalledData.getBuffer());
      syncWriteStream();
   }

   BitStream unmarshallData(theCall->marshalledData.getBuffer(), theCall->marshalledData.getBytePosition());
   theCall->unmarshall(&unmarshallData);
   for(JournalEntryRecord *walk = JournalEntryRecord::mList; walk; walk = walk->mNext)
   {
      if(!strcmp(walk->mFunctionName, funcName))
      {
         MethodPointer p;
         walk->getFuncPtr(p);
         theCall->dispatch((void *) this, &p);
         mInsideEntrypoint = false;
         return;
      }
   }
   TNLAssert(0, "No entry point found!");
}

void Journal::checkReadPosition()
{
   if(!mReadStream.isValid() || mReadStream.getBitPosition() == mReadStream.getMaxReadBitPosition())
      Platform::debugBreak();
}

void Journal::processNextJournalEntry()
{
   if(mCurrentMode != Playback)
      return;

   char funcName[256];
   mReadStream.readString(funcName);
   JournalEntryRecord *theEntry;
   for(theEntry = JournalEntryRecord::mList; theEntry; theEntry = theEntry->mNext)
      if(!strcmp(theEntry->mFunctionName, funcName))
         break;

   // check for errors...
   if(!theEntry)
   {
      TNLAssert(0, "blech!");
   }
   checkReadPosition();

   MethodPointer p;
   theEntry->getFuncPtr(p);
   MarshalledCall theCall(theEntry->mMethodArgList);
   theCall.unmarshall(&mReadStream);
   mInsideEntrypoint = true;
   theCall.dispatch((void *) this, &p);
   mInsideEntrypoint = false;
}

};