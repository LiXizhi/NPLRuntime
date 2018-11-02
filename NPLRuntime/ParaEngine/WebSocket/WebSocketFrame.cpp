#include "ParaEngine.h"
#include "WebSocketFrame.h"
using namespace NPL::WebSocket;
	WebSocketFrame::WebSocketFrame()
	{
		reset();
	}

	WebSocketFrame::~WebSocketFrame()
	{

	}

	void WebSocketFrame::assertValid()
	{

	}

	void NPL::WebSocket::WebSocketFrame::setMask(const std::vector<byte>& maskingKey)
	{
		mask = maskingKey;
		masked = !mask.empty();
	}

	void NPL::WebSocket::WebSocketFrame::setMask(const byte* data, int nCount)
	{
		mask.assign(data, data + nCount);
		masked = !mask.empty();
	}

	void NPL::WebSocket::WebSocketFrame::append(ByteBuffer& buffer, int needed_len)
	{
		int len = buffer.bytesRemaining();
		needed_len = min(needed_len, len);

		int max_size = data.size() + needed_len;
		if ((int)data.capacity() < max_size)
		{
			data.reserve((uint32_t)max_size);
		}
		for (int i = 0; i < needed_len; i++)
		{
			data.put(buffer.get());
		}
	}

	void NPL::WebSocket::WebSocketFrame::reset()
	{
		finRsvOp = (byte)0x80; // FIN (!RSV, opcode 0)
		masked = false;
		data.clear();
		mask.clear();
	}

	void NPL::WebSocket::WebSocketFrame::loadData(vector<byte>& outData)
	{
		int len = data.bytesRemaining();
		for (int i = 0; i < len; i++)
		{
			byte b = data.get();
			if (isMasked())
			{
				b = b ^ mask[i % 4];
			}
			outData.push_back(b);
		}
	}
	
