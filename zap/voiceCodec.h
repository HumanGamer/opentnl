//-----------------------------------------------------------------------------------
//
//   Torque Network Library - ZAP example multiplayer vector graphics space game
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

#ifndef _VOICECODEC_H_
#define _VOICECODEC_H_

#ifndef _TNL_BYTEBUFFER_H_
#include "tnlByteBuffer.h"
#endif

using namespace TNL;

namespace Zap
{

/// The VoiceEncoder class is an abstract base class for the various
/// voice compression algorithms supported by Zap - initially just the
/// HawkVoice variable bitrate LPC10 codec.  It takes an arbitrary buffer
/// of 16 bit samples at 8KHz and returns a compressed buffer.  The 
/// original buffer is modified to contain any samples that were not used 
/// due to unfilled frame sizing.
class VoiceEncoder : public Object
{
   virtual U32 getSamplesPerFrame() = 0;
   virtual U32 getMaxCompressedFrameSize() = 0;
   virtual U32 compressFrame(S16 *samplePtr, U8 *outputPtr) = 0;
public:
   ByteBufferPtr compressBuffer(ByteBufferPtr sampleBuffer);
};

/// The VoiceDecoder class is an abstract base class for the various
/// voice decompression algorithms supported by Zap - initially just the
/// HawkVoice variable bitrate LPC10 codec.  It takes a buffer compressed
/// with the appropriate VoiceEncoder analogue and returns the decompressed
/// 16 bit sample buffer.
class VoiceDecoder : public Object
{
   virtual U32 getSamplesPerFrame() = 0;
   virtual U32 getAvgCompressedFrameSize() = 0;

   virtual U32 decompressFrame(S16 *framePtr, U8 *inputPtr, U32 inSize) = 0;
public:
   ByteBufferPtr decompressBuffer(ByteBufferRef compressedBuffer);
};

/// The LPC10VoiceEncoder class implements the HawkVoice LPC10 codec
/// compressor.
class LPC10VoiceEncoder : public VoiceEncoder
{
   void *encoderState;
   U32 getSamplesPerFrame();
   U32 getMaxCompressedFrameSize();
   U32 compressFrame(S16 *samplePtr, U8 *outputPtr);
public:
   LPC10VoiceEncoder();
   ~LPC10VoiceEncoder();
};

/// The LPC10VoiceDecoder class implements the HawkVoice LPC10 codec
/// decompressor.
class LPC10VoiceDecoder : public VoiceDecoder
{
   void *decoderState;
   U32 getSamplesPerFrame();
   U32 getAvgCompressedFrameSize();

   U32 decompressFrame(S16 *framePtr, U8 *inputPtr, U32 inSize);
public:
   LPC10VoiceDecoder();
   ~LPC10VoiceDecoder();
};

/// The GSMVoiceEncoder class implements the HawkVoice GSM codec
/// compressor.
class GSMVoiceEncoder : public VoiceEncoder
{
   void *encoderState;
   U32 getSamplesPerFrame();
   U32 getMaxCompressedFrameSize();
   U32 compressFrame(S16 *samplePtr, U8 *outputPtr);
public:
   GSMVoiceEncoder();
   ~GSMVoiceEncoder();
};

/// The GSMVoiceDecoder class implements the HawkVoice GSM codec
/// decompressor.
class GSMVoiceDecoder : public VoiceDecoder
{
   void *decoderState;
   U32 getSamplesPerFrame();
   U32 getAvgCompressedFrameSize();

   U32 decompressFrame(S16 *framePtr, U8 *inputPtr, U32 inSize);
public:
   GSMVoiceDecoder();
   ~GSMVoiceDecoder();
};

};

#endif

