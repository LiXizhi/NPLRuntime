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
	
