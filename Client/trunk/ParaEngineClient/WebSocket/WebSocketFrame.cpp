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

	string WebSocketFrame::getPayloadAsUTF8()
	{
		
		vector<byte> buff = getData();
		string s(buff.begin(),buff.end());
		return s;
	}

	vector<byte> WebSocketFrame::getData()
	{
		int len = data.bytesRemaining();
		vector<byte> buff;
		for (int i = 0; i < len; i++)
		{
			byte b = data.get();
			if (isMasked())
			{
				b = b ^ mask[i % 4];
			}
			buff.push_back(b);
		}
		return buff;
	}
