#pragma once
#include <boost/array.hpp>
namespace NPL
{
	namespace WebSocket
	{
		typedef boost::array<char, 8192> Buffer_Type;
		enum OpCode : unsigned char {
			TEXT = 1,
			BINARY = 2,
			CLOSE = 8,
			PING = 9,
			PONG = 10
		};
		enum State
		{
			START,
			PAYLOAD_LEN,
			PAYLOAD_LEN_BYTES,
			MASK,
			MASK_BYTES,
			PAYLOAD

		};

		class WebSocketCommon {
		public:
			static bool isKnown(unsigned char opcode) {
				return ((opcode == TEXT) || (opcode == BINARY) || (opcode == CLOSE) || (opcode == PING) || (opcode == PONG));
			};
		};
		
	}
}
