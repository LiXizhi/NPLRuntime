/*
---------------------------------------------
the base64 encode and decode
---------------------------------------------
*/
#include <string>
using namespace std;

class ZBase64
{
public:
	/*����
	DataByte
	[in]��������ݳ���,���ֽ�Ϊ��λ

	-- δ�����Ƿ���Ч
	*/
	string Encode(const unsigned char* Data, int DataByte);
	/*����
	DataByte
	[in]��������ݳ���,���ֽ�Ϊ��λ
	OutByte
	[out]��������ݳ���,���ֽ�Ϊ��λ,�벻Ҫͨ������ֵ����
	������ݵĳ���
	*/
	string Decode(const char* Data, int DataByte, int& OutByte);
};