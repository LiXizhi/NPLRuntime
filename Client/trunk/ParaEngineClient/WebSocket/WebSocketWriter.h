#pragma once
#include "WebSocketCommon.h"
#include "ByteBuffer.h"
#include "WebSocketFrame.h"
namespace NPL
{
	namespace WebSocket
	{
		class WebSocketWriter
		{
		public:
			WebSocketWriter();
			~WebSocketWriter();

			bool isRsv1InUse() { return (flagsInUse & 0x40) != 0; };
			bool isRsv2InUse() { return (flagsInUse & 0x20) != 0; };
			bool isRsv3InUse() { return (flagsInUse & 0x10) != 0; };

			void setRsv1InUse(bool rsv1InUse) {
				flagsInUse = (byte)((flagsInUse & 0xBF) | (rsv1InUse ? 0x40 : 0x00));
			};
			void setRsv2InUse(bool rsv2InUse) {
				flagsInUse = (byte)((flagsInUse & 0xDF) | (rsv2InUse ? 0x20 : 0x00));
			};
			void setRsv3InUse(bool rsv3InUse) {
				flagsInUse = (byte)((flagsInUse & 0xEF) | (rsv3InUse ? 0x10 : 0x00));
			};
			void assertFrameValid(WebSocketFrame& frame);
			void generateHeaderBytes(WebSocketFrame& frame, ByteBuffer& buffer);
		 	void generateWholeFrame(WebSocketFrame& frame, ByteBuffer& buffer);

			void generate(const char * code, int nLength, vector<byte>& outData);
			void reset();
		private:
			/**
			* Are any flags in use
			* <p>
			*
			* <pre>
			*   0100_0000 (0x40) = rsv1
			*   0010_0000 (0x20) = rsv2
			*   0001_0000 (0x10) = rsv3
			* </pre>
			*/
			byte flagsInUse;
			WebSocketFrame frame;
			WebSocket::ByteBuffer input_buff;
			WebSocket::ByteBuffer out_buff;
		};
	}
}