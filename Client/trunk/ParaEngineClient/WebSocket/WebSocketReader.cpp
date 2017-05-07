//-----------------------------------------------------------------------------
// Class:	WebSocketReader
// Authors:	leio
// Date:	2017/4/26
// Desc:  Parse WebSocket protocol
// based on: http://git.eclipse.org/c/jetty/org.eclipse.jetty.project.git/plain/jetty-websocket/websocket-common/src/main/java/org/eclipse/jetty/websocket/common/Parser.java
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "WebSocketReader.h"
#include <vector>
using namespace NPL::WebSocket;


void WebSocketReader::assertSanePayloadLength(int length)
{

}

WebSocketReader::WebSocketReader()
	: state(START)
	, cursor(0)
	, flagsInUse(0x00)
{
	frame = new WebSocketFrame();
}

WebSocketReader::~WebSocketReader()
{
	delete frame;
}

ByteBuffer WebSocketReader::load(Buffer_Type* buffer, int bytes_transferred)
{
	ByteBuffer b;
	if (!buffer)
	{
		return b;
	}
	Buffer_Type::iterator curIt = buffer->begin();
	Buffer_Type::iterator curEnd = buffer->begin() + bytes_transferred;
	while (curIt != curEnd)
	{
		b.putChar(*curIt);
		curIt++;
	}

	return b;
}
bool WebSocketReader::parse(ByteBuffer& buffer)
{
	if (buffer.bytesRemaining() <= 0)
	{
		return false;
	}
	while (parseFrame(buffer))
	{
		return true;
	}
	return false;
}
bool WebSocketReader::parseFrame(ByteBuffer& buffer)
{
	int len = buffer.bytesRemaining();
	while ( len > 0)
	{
		switch (state)
		{
		case NPL::WebSocket::State::START:
		{
			byte b = buffer.get();
			bool fin = ((b & 0x80) != 0);
			byte opcode = (byte)(b & 0x0F);
			if (!WebSocketCommon::isKnown(opcode))
			{
				return false;
			}
			switch (opcode)
			{
			case NPL::WebSocket::OpCode::TEXT:
			case NPL::WebSocket::OpCode::BINARY:
			case NPL::WebSocket::OpCode::CLOSE:
			case NPL::WebSocket::OpCode::PING:
			case NPL::WebSocket::OpCode::PONG:
			{
				frame->reset();
				frame->setOpCode(opcode);
				break;
			}
			}
			frame->setFin(fin);
			// Are any flags set?
			if ((b & 0x70) != 0)
			{
				/*
				* RFC 6455 Section 5.2
				*
				* MUST be 0 unless an extension is negotiated that defines meanings for non-zero values. If a nonzero value is received and none of the
				* negotiated extensions defines the meaning of such a nonzero value, the receiving endpoint MUST _Fail the WebSocket Connection_.
				*/
				if ((b & 0x40) != 0)
				{
					if (isRsv1InUse())
						frame->setRsv1(true);
				}
				if ((b & 0x20) != 0)
				{
					if (isRsv2InUse())
						frame->setRsv2(true);
				}
				if ((b & 0x10) != 0)
				{
					if (isRsv3InUse())
						frame->setRsv3(true);
				}
			}
			state = PAYLOAD_LEN;
			break;
		}
			
		case NPL::WebSocket::State::PAYLOAD_LEN:
		{
			byte b = buffer.get();
			frame->setMasked((b & 0x80) != 0);
			payloadLength = (byte)(0x7F & b);

			if (payloadLength == 127) // 0x7F
			{
				// length 8 bytes (extended payload length)
				payloadLength = 0;
				state = PAYLOAD_LEN_BYTES;
				cursor = 8;
				break; // continue onto next state
			}
			else if (payloadLength == 126) // 0x7E
			{
				// length 2 bytes (extended payload length)
				payloadLength = 0;
				state = PAYLOAD_LEN_BYTES;
				cursor = 2;
				break; // continue onto next state
			}
			assertSanePayloadLength(payloadLength);
			if (frame->isMasked())
			{
				state = MASK;
			}
			else
			{
				// special case for empty payloads (no more bytes left in buffer)
				if (payloadLength == 0)
				{
					state = START;
					return true;
				}
				// what is maskProcessor?
				//maskProcessor.reset(frame);
				state = PAYLOAD;
			}
			break;
		}
			
		case NPL::WebSocket::State::PAYLOAD_LEN_BYTES:
		{
			byte b = buffer.get();
			--cursor;
			payloadLength |= (b & 0xFF) << (8 * cursor);
			if (cursor == 0)
			{
				assertSanePayloadLength(payloadLength);
				if (frame->isMasked())
				{
					state = MASK;
				}
				else
				{
					// special case for empty payloads (no more bytes left in buffer)
					if (payloadLength == 0)
					{
						state = START;
						return true;
					}

					// what is maskProcessor?
					//maskProcessor.reset(frame);
					state = PAYLOAD;
				}
			}
			break;
		}
		case NPL::WebSocket::State::MASK:
		{
			byte m[4];
			if (buffer.bytesRemaining() >= 4)
			{
				buffer.getBytes(m, 4);
				std::vector<byte> mm(m, m + 4);
				frame->setMask(mm);
				// special case for empty payloads (no more bytes left in buffer)
				if (payloadLength == 0)
				{
					state = START;
					return true;
				}
				// what is maskProcessor?
				//maskProcessor.reset(frame);
				state = PAYLOAD;
			}
			else
			{
				state = MASK_BYTES;
				cursor = 4;
			}
			break;
		}
		case NPL::WebSocket::State::MASK_BYTES:
		{
			byte b = buffer.get();
			std::vector<byte> mask = frame->getMask();
			mask[4 - cursor] = b;
			frame->setMask(mask);
			--cursor;
			if (cursor == 0)
			{
				// special case for empty payloads (no more bytes left in buffer)
				if (payloadLength == 0)
				{
					state = START;
					return true;
				}
				//// what is maskProcessor?
				//maskProcessor.reset(frame);
				state = PAYLOAD;
			}
			break;
		}
		case NPL::WebSocket::State::PAYLOAD:
		{
			frame->assertValid();
			if (parsePayload(buffer))
			{
				// special check for close
				if (frame->getOpCode() == OpCode::CLOSE)
				{
					// TODO: yuck. Don't create an object to do validation checks!
					// new CloseInfo(frame);
				}
				state = START;
				// we have a frame!
				return true;
			}
			break;
		}
		}
	}
	return true;
}

bool WebSocketReader::parsePayload(ByteBuffer& buffer)
{
	if (payloadLength == 0)
	{
		return true;
	}

	const int len = buffer.bytesRemaining();
	if ( len > 0)
	{

		frame->setPayload(buffer);
		
		return true;
		
	}
	return false;
}

WebSocketFrame* WebSocketReader::getFrame()
{
	return frame;
}

