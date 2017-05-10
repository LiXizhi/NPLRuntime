//-----------------------------------------------------------------------------
// Class:	WebSocketWriter
// Authors:	leio
// Date:	2017/5/5
// Desc:  Write WebSocket protocol
// based on:http://git.eclipse.org/c/jetty/org.eclipse.jetty.project.git/plain/jetty-websocket/websocket-common/src/main/java/org/eclipse/jetty/websocket/common/Generator.java
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "WebSocketWriter.h"
#include "ByteBuffer.h"
using namespace NPL::WebSocket;

NPL::WebSocket::WebSocketWriter::WebSocketWriter()
{
	reset();
}

NPL::WebSocket::WebSocketWriter::~WebSocketWriter()
{

}

void NPL::WebSocket::WebSocketWriter::assertFrameValid(WebSocketFrame& frame)
{

}

void NPL::WebSocket::WebSocketWriter::generateHeaderBytes(WebSocketFrame& frame, ByteBuffer& buffer)
{
	// we need a framing header
	assertFrameValid(frame);

	/*
	* start the generation process
	*/
	byte b = 0x00;

	// Setup fin thru opcode
	if (frame.isFin())
	{
		b |= 0x80; // 1000_0000
	}

	// Set the flags
	if (frame.isRsv1())
	{
		b |= 0x40; // 0100_0000
	}
	if (frame.isRsv2())
	{
		b |= 0x20; // 0010_0000
	}
	if (frame.isRsv3())
	{
		b |= 0x10; // 0001_0000
	}

	byte opcode = frame.getOpCode();

	b |= opcode & 0x0F;

	buffer.put(b);

	// is masked
	b = (frame.isMasked() ? (byte)0x80 : (byte)0x00);

	// payload lengths
	int payloadLength = frame.getPayloadLength();

	/*
	* if length is over 65535 then its a 7 + 64 bit length
	*/
	if (payloadLength > 0xFFFF)
	{
		// we have a 64 bit length
		b |= 0x7F;
		buffer.put(b); // indicate 8 byte length
		buffer.put((byte)0); //
		buffer.put((byte)0); // anything over an
		buffer.put((byte)0); // int is just
		buffer.put((byte)0); // insane!
		buffer.put((byte)((payloadLength >> 24) & 0xFF));
		buffer.put((byte)((payloadLength >> 16) & 0xFF));
		buffer.put((byte)((payloadLength >> 8) & 0xFF));
		buffer.put((byte)(payloadLength & 0xFF));
	}
	/*
	* if payload is greater that 126 we have a 7 + 16 bit length
	*/
	else if (payloadLength >= 0x7E)
	{
		b |= 0x7E;
		buffer.put(b); // indicate 2 byte length
		buffer.put((byte)(payloadLength >> 8));
		buffer.put((byte)(payloadLength & 0xFF));
	}
	/*
	* we have a 7 bit length
	*/
	else
	{
		b |= (payloadLength & 0x7F);
		buffer.put(b);
	}

	// masking key
	if (frame.isMasked())
	{
		vector<byte> mask = frame.getMask();

		int maskInt = 0;
		for (byte maskByte : mask)
		{
			maskInt = (maskInt << 8) + (maskByte & 0xFF);
			buffer.put(maskInt);
		}

		// perform data masking here
		ByteBuffer payload = frame.getPayload();
		if (payload.bytesRemaining() > 0)
		{
			int maskOffset = 0;
			int start = 0;
			int end = payload.bytesRemaining();
			int remaining;
			while ((remaining = end - start) > 0)
			{
				if (remaining >= 4)
				{
					payload.putInt(start, payload.getInt(start) ^ maskInt);
					start += 4;
				}
				else
				{
					payload.put(start, (byte)(payload.get(start) ^ mask[maskOffset & 3]));
					++start;
					++maskOffset;
				}
			}
		}
	}
}

void NPL::WebSocket::WebSocketWriter::generateWholeFrame(WebSocketFrame& frame, ByteBuffer& buffer)
{
	generateHeaderBytes(frame, buffer);
	if (frame.hasPayload())
	{
		ByteBuffer buff = frame.getPayload();
		int len = buff.bytesRemaining();
		for (int i = 0; i < len; i++)
		{
			buffer.put(buff.get());
		}
	}
}

void NPL::WebSocket::WebSocketWriter::generate(const char * code, int nLength,vector<byte>& outData)
{
	reset();
	frame.setOpCode(WebSocket::OpCode::TEXT);
	for (int i = 0; i < nLength; i++)
	{
		input_buff.putChar(code[i]);
	}
	frame.setPayload(input_buff);
	generateWholeFrame(frame, out_buff);

	int len = out_buff.bytesRemaining();
	for (int i = 0; i < len; i++)
	{
		byte b = out_buff.get();
		outData.push_back(b);
	}

}

void NPL::WebSocket::WebSocketWriter::reset()
{
	flagsInUse = 0x00;
	frame.reset();
	input_buff.clear();
	out_buff.clear();
}
