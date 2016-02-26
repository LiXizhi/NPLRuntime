#pragma once
#include "util/StringBuilder.h"
namespace NPL
{
	template <typename StringBufferType = std::string>
	class CNPLWriterT;

	/** using StringBuilder for buffering. This is the prefered way to do the code generator */
	typedef CNPLWriterT< ParaEngine::StringBuilder > CNPLBufWriter;

	/** using std::string for buffering */
	typedef CNPLWriterT< std::string >  CNPLWriter; 

	using namespace std;

	/** a simple class for creating NPL script code, especially data table code. 
	this class is reentrant (thread-safe). Please note that this class does not ensure that the code is a pure table.
	See Example:
	// to generate the string : msg={name=1,2,{"3"="4",},};
	CNPLWriter writer; // if you can estimate the buffer size, use CNPLWriter writer(nReservedSize)
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("name");
	writer.WriteValue(1);
	writer.WriteValue(2);
	writer.BeginTable();
	writer.WriteName("3", true);
	writer.WriteValue("4");
	writer.EndTable();
	writer.EndTable();
	writer.WriteParamDelimiter();
	log(writer.ToString().c_str());

	One can also provide their own string buffer to write to, like below.
	std::string buff;
	buff.reserve(100);
	CNPLWriter writer(buff);
	*/
	template <typename StringBufferType>
	class CNPLWriterT
	{
	public:
		typedef StringBufferType  Buffer_Type;

		/** the internal buffer reserved size. */
		CNPLWriterT(int nReservedSize = -1);
		/** into which buff_ to write the */
		CNPLWriterT(Buffer_Type& buff_, int nReservedSize = -1);
		~CNPLWriterT();

		/** reset the writer */
		void Reset(int nReservedSize = -1);

		/** write begin table "{" */
		void BeginTable();
		/** write a parameter name
		@param bUseBrackets: if false, one has to make sure that the name is a valid NPL name string, without special characters. */
		void WriteName(const char* name, bool bUseBrackets = false);

		/** if bInQuotation is true, it writes a parameter text value. Otherwise it will just append the value without encoding it with quatation marks. */
		void WriteValue(const char* value, bool bInQuotation=true);
		/** if bInQuotation is true, it writes a parameter text value. Otherwise it will just append the value without encoding it with quatation marks. */
		void WriteValue(const char* buffer, int nSize, bool bInQuotation=true);
		void WriteValue(const string& sStr, bool bInQuotation=true);
		/** write a parameter value */
		void WriteValue(double value);
		/** write a parameter nil*/
		void WriteNil();

		/** write end table "}" */
		void EndTable();

		/** append any text */
		void Append(const char* text);
		void Append(const char* pData, int nSize);
		void Append(const string& sText);
		/** add a mem block of size nSize and return the address of the block. 
		* we may fill the block at a latter time. 
		* @param nSize: size in bytes. 
		*/
		char* AddMemBlock(int nSize);
		
		/** write ";" */
		void WriteParamDelimiter(){ Append(";"); };

		/** get the current NPL code. */
		const Buffer_Type& ToString() {return m_sCode;};
	public:
		// static strings. 

		/** return "msg=nil;"*/
		static const Buffer_Type& GetNilMessage();

		/** return "msg={};"*/
		static const Buffer_Type& GetEmptyMessage();
	private:
		Buffer_Type& m_sCode;
		Buffer_Type m_buf;
		bool m_bBeginAssignment;
		int m_nTableLevel;
	};
}


