#pragma once
#include "WebSocketCommon.h"
#include "ByteBuffer.h"
#include <vector>
using namespace std;
namespace NPL
{
	namespace WebSocket
	{

		class WebSocketFrame
		{
		public:
			WebSocketFrame();
			~WebSocketFrame();
			vector<byte> getMask() {
				return mask;
			};
			byte getOpCode() {
				return (byte)(finRsvOp & 0x0F);
			};
			
			ByteBuffer getPayload() {
				return data;
			};
			int getPayloadLength() {
				return data.bytesRemaining();
			};
			bool isMasked() {
				return masked;
			}
			bool isFin() {
				return (byte)(finRsvOp & 0x80) != 0;
			};
			void setFin(bool fin) {
				// set bit 1
				finRsvOp = (byte)((finRsvOp & 0x7F) | (fin ? 0x80 : 0x00));
			};
			void setMask(vector<byte> maskingKey) {
				mask = maskingKey;
				masked = !mask.empty();
			};
			void setMasked(bool mask) {
				masked = mask;
			};
			void setOpCode(byte op) {
				finRsvOp = (byte)((finRsvOp & 0xF0) | (op & 0x0F));
			};
			void setPayload(ByteBuffer& buffer) {
				data.clear();
				int len = buffer.bytesRemaining();
				for (int i = 0; i< len; i++)
				{
					data.put(buffer.get());
				}
			};
			void setRsv1(bool rsv1) {
				// set bit 2
				finRsvOp = (byte)((finRsvOp & 0xBF) | (rsv1 ? 0x40 : 0x00));
			};
			void setRsv2(bool rsv2) {
				// set bit 3
				finRsvOp = (byte)((finRsvOp & 0xDF) | (rsv2 ? 0x20 : 0x00));
			};
			void setRsv3(bool rsv3) {
				// set bit 4
				finRsvOp = (byte)((finRsvOp & 0xEF) | (rsv3 ? 0x10 : 0x00));
			};
			bool isRsv1() {
				return (byte)(finRsvOp & 0x40) != 0;
			};
			bool isRsv2() {
				return (byte)(finRsvOp & 0x20) != 0;
			};
			bool isRsv3() {
				return (byte)(finRsvOp & 0x10) != 0;
			};
			bool hasPayload() {
				return getPayloadLength() > 0;
			};
			void reset() {
				finRsvOp = (byte)0x80; // FIN (!RSV, opcode 0)
				masked = false;
				data.clear();
				mask.clear();
			};
			void assertValid();

			void loadData(vector<byte>& outData);
		private:
			/**
			* Combined FIN + RSV1 + RSV2 + RSV3 + OpCode byte.
			*
			* <pre>
			*   1000_0000 (0x80) = fin
			*   0100_0000 (0x40) = rsv1
			*   0010_0000 (0x20) = rsv2
			*   0001_0000 (0x10) = rsv3
			*   0000_1111 (0x0F) = opcode
			* </pre>
			*/
			byte finRsvOp;
			bool masked;
			ByteBuffer data;
			vector<byte> mask;
		};
	}
}
