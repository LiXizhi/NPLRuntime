#pragma once
#include "WebSocketCommon.h"
#include "ByteBuffer.h"
#include "WebSocketFrame.h"
namespace NPL
{
	namespace WebSocket
	{
		class WebSocketReader
		{
		public:
			WebSocketReader();
			~WebSocketReader();
			bool parse(ByteBuffer& buffer);
			bool parseFrame(ByteBuffer& buffer);

			bool isRsv1InUse() { return (flagsInUse & 0x40) != 0; };
			bool isRsv2InUse() { return (flagsInUse & 0x20) != 0; };
			bool isRsv3InUse() { return (flagsInUse & 0x10) != 0; };

			static ByteBuffer load(Buffer_Type* buffer, int bytes_transferred);


			WebSocketFrame* getFrame();

		private:
			void assertSanePayloadLength(int length);
			/**
			* Implementation specific parsing of a payload
			*
			* @param buffer
			*            the payload buffer
			* @return true if payload is done reading, false if incomplete
			*/
			bool parsePayload(ByteBuffer& buffer);
		private:
			ByteBuffer payload;
			int payloadLength;
			int cursor;
			State state;

			/**
			* Is there an extension using RSV flag?
			* <p>
			*
			* <pre>
			*   0100_0000 (0x40) = rsv1
			*   0010_0000 (0x20) = rsv2
			*   0001_0000 (0x10) = rsv3
			* </pre>
			*/
			byte flagsInUse;
			WebSocketFrame* frame;
		};
	}
}