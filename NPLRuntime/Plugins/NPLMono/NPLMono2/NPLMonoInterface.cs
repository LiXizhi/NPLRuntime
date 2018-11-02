//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: Exposing ParaEngine C++ functions to .Net via mono interface. All C# scripts(dll) in ParaEngine should reference this NPLMonoInterface.dll.
// it also contains some pure .Net helper class for message parsing etc. 
//  - APIs are in several global classes (NPL, ParaGlobal) 
//  - Helper functions are in NPLMono namespace, such as (NPLWriter)
//-----------------------------------------------------------------------------
using System;
using System.Runtime.CompilerServices;
using System.Collections.Generic;
using System.Xml.Serialization;
using System.Xml;
using System.Text;
using NPLMono;

#region ParaEngine API 
// Note: namespaces of exposed API is not supported at the moment. This is currently a mono implementation limit, may be removed in future. 

/** import C++ functions from ParaEngine API */
public class NPL
{
    /// <summary>
    /// This function is for testing NPLMonoInterface.cs
    /// </summary>
    /// <returns>All your monos belong to ParaEngine!</returns>
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static string HelloWorld();

    /// <summary>
    /// Get the current incoming message of a npl_runtime_state. 
    /// @remark: This function should only be called from activate() function, since activate() function runs in the same thread of its parent npl_runtime_state
    /// </summary>
    /// <param name="mono_scripting_state"></param>
    /// <returns></returns>
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static string GetCurrentMsg(System.IntPtr npl_runtime_state);

    /// <summary>
    /// activate a local or remote file
    /// </summary>
    /// <param name="npl_runtime_state">
    /// </param>
    /// <param name="file_name">
    ///  a globally unique name of a NPL file name instance. 
    /// The string format of an NPL file name is like below. 
    /// [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
    /// the following is a list of all valid file name combinations: 
    /// "user001@paraengine.com:script/hello.lua"	-- a file of user001 in its default gaming thread
    /// "(world1)server001@paraengine.com:script/hello.lua"		-- a file of server001 in its thread world1
    /// "(worker1)script/hello.lua"			-- a local file in the thread worker1
    /// "(gl)script/hello.lua"			-- a glia (local) file in the current runtime state's thread
    /// "script/hello.lua"			-- a file in the current thread. For a single threaded application, this is usually enough.
    /// "(worker1)NPLRouter.dll"			-- activate a C++ or C# dll. Please note that, in windows, it looks for NPLRonter.dll; in linux, it looks for ./libNPLRouter.so 
    /// "plugin/libNPLRouter.dll"			-- almost same as above, it is recommented to remove the heading 'lib' when loading. In windows, it looks for plugin/NPLRonter.dll; in linux, it looks for ./plugin/libNPLRouter.so
    /// </param>
    /// <param name="msg"></param>
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static void activate(System.IntPtr npl_runtime_state, string file_name, string msg);

    /// <summary>
    /// same as activate2. except that no npl_runtime_state is specified, the main runtime state is assumed. 
    /// </summary>
    /// <param name="file_name"></param>
    /// <param name="msg"></param>
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static void activate2(string file_name, string msg);

    /// <summary>
    /// accept a given connection. The connection will be regarded as authenticated once accepted. 
    /// </summary>
    /// <param name="tid">the temporary id or NID of the connection to be accepted. usually it is from msg.tid or msg.nid. </param>
    /// <param name="nid">if this is not nil, tid will be renamed to nid after accepted. </param>
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static void accept(string tid, string nid);

    /// <summary>
    ///  reject and close a given connection. The connection will be closed once rejected. 
    /// </summary>
    /// <param name="tid">the temporary id or NID of the connection to be rejected. usually it is from msg.tid or msg.nid. </param>
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static void reject(string tid);
}

/// <summary>
/// TODO: expose more ParaEngine API here
/// </summary>
public class ParaGlobal
{
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static void log(string str);

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    public extern static void applog(string str);
}
#endregion

#region NPLHelper
namespace NPLMono
{
     /** converting XML to string and vice versa.*/
    public class NPLHelper
    {
        /// <summary>
        /// this version is the same as EncodeStringInQuotation, except that it is in use on the server side. 
        /// the server side sees \r as "&#x000d;". This is a client/server fix for a bug of xml transmission of \r in .net 2. 
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        public static string EncodeStringInQuotationFix(string str)
        {
            return EncodeStringInQuotation(str).Replace("&#x000d;", "\\&#x000d;");
        }

        /// <summary>
        /// this is something like string.format("%q") in NPL.
        /// [thread-safe]
        /// replace quotation mark in string. e.g. "\" will be converted to \"\\\"
        /// </summary>
        /// <param name="buff">into which buffer to write the output</param>
        /// <param name="nOutputOffset">the output will be written to output[nOutputOffset]. default to 0,which is the beginning</param>
        /// <param name="input">the input string to be encoded</param>
        public static void EncodeStringInQuotation(ref StringBuilder buff, int nOutputOffset, string input)
        {
            int nSize = (int) input.Length;
            int nFinalSize = nOutputOffset + nSize + 2;
            buff.EnsureCapacity(nFinalSize);

            // replace quotation mark in string. 
            buff.Length = nOutputOffset;
            buff.Append('"');
            for (int i = 0; i < nSize; ++i)
            {
                char c = input[i];
                switch (c)
                {
                    case '"':
                    case '\\':
                    case '\n':
                        {
                            buff.Append('\\');
                            buff.Append(c);
                            break;
                        }
                    case '\r':
                        {
                            buff.Append('\\');
                            buff.Append('r');
                            break;
                        }
                    case '\0':
                        {
                            buff.Append('\\');
                            buff.Append('0');
                            buff.Append('0');
                            buff.Append('0');
                            break;
                        }
                    default:
                        {
                            buff.Append(c);
                            break;
                        }
                }
            }
            buff.Append('"');
        }

        /// <summary>
        /// replace quotation mark in string
        /// e.g. "\" will be converted to \"\\\"
        /// </summary>
        /// <param name="str">if nil, it will return "nil" </param>
        /// <returns></returns>
        public static string EncodeStringInQuotation(string str)
        {
            if (str == null)
            {
                return "nil";
            }
            int nSize = (int)str.Length;
            // this is something like string.format("%q") in NPL.
            char[] buff = new char[nSize * 4 + 2]; // just make sure there are enough buffer

            // replace quotation mark in string. 
            int nPos = 0;
            buff[nPos++] = '"';
            for (int i = 0; i < nSize; ++i)
            {
                char c = str[i];
                switch (c)
                {
                    case '"':
                    case '\\':
                    case '\n':
                        {
                            buff[nPos++] = '\\';
                            buff[nPos++] = c;
                            break;
                        }
                    case '\r':
                        {
                            buff[nPos++] = '\\';
                            buff[nPos++] = 'r';
                            break;
                        }
                    case '\0':
                        {
                            buff[nPos++] = '\\';
                            buff[nPos++] = '0';
                            buff[nPos++] = '0';
                            buff[nPos++] = '0';
                            break;
                        }
                    default:
                        {
                            buff[nPos++] = c;
                            break;
                        }
                }
            }
            buff[nPos++] = '"';
            return new string(buff, 0, nPos);
        }

        public static bool DeserializePureNPLDataBlock(NPLLex.LexState ls, ref NPLObjectProxy objProxy)
        {
	        // data
	        int c = ls.t.token;
	        switch (c)
	        {
	        case (int)NPLLex.RESERVED.TK_TRUE:
		        objProxy.Assign(true);
		        NPLParser.next(ls);
		        return true;
            case (int)NPLLex.RESERVED.TK_FALSE:
		        objProxy.Assign(false);
		        NPLParser.next(ls);
		        return true;
            case (int)NPLLex.RESERVED.TK_NIL:
		        objProxy.MakeNil();
		        NPLParser.next(ls);
		        return true;
            case (int)NPLLex.RESERVED.TK_NUMBER:
		        objProxy.Assign(ls.t.seminfo.r);
		        NPLParser.next(ls);
		        return true;
            case (int)NPLLex.RESERVED.TK_STRING:
		        objProxy.Assign(ls.t.seminfo.ts);
		        NPLParser.next(ls);
		        return true;
	        case '-':
		        {
			        // negative number
			        NPLParser.next(ls);
			        if(ls.t.token == (int)NPLLex.RESERVED.TK_NUMBER)
			        {
				        objProxy.Assign(- ls.t.seminfo.r);
				        NPLParser.next(ls);
				        return true;
			        }
			        else
				        return false;
		        }
	        case '{':
		        {
                    // table object
                    NPLObjectProxy tabMsg = objProxy; 

			        NPLParser.enterlevel(ls);
			        bool bBreak = false;
			        NPLParser.next(ls);
			        // auto table index that begins with 1. 
			        int nTableAutoIndex = 1;
			        while (!bBreak)
			        {
				        c = ls.t.token;
				        if(c == '}')
				        {
					        // end of table
					        NPLParser.leavelevel(ls);
					        NPLParser.next(ls);
					        bBreak = true;
				        }
                        else if (c == (int)NPLLex.RESERVED.TK_NAME)
				        {
                            NPLObjectProxy proxy_ = tabMsg[ls.t.seminfo.ts];
					        // by name assignment, such as name = data|table
					        NPLParser.next(ls);
					        if(ls.t.token == '=')
					        {
						        NPLParser.next(ls);
                                if (!DeserializePureNPLDataBlock(ls, ref proxy_))
                                    return false;
                                NPLParser.testnext(ls, ',');
					        }
					        else 
						        return false;
				        }
				        else if(c == '[')
				        {
					        // by integer or string key assignment, such as [number|string] = data|table
					        NPLParser.next(ls);
                            if (ls.t.token == (int)NPLLex.RESERVED.TK_NUMBER)
					        {
						        // verify that it is an integer, instead of a floating value.
						        NPLObjectProxy proxy_ = tabMsg[(int)(ls.t.seminfo.r)];

						        NPLParser.next(ls);
						        if(ls.t.token == ']')
						        {
							        NPLParser.next(ls);
							        if(ls.t.token == '=')
							        {
								        NPLParser.next(ls);
                                        if (!DeserializePureNPLDataBlock(ls, ref proxy_))
									        return false;
								        NPLParser.testnext(ls, ',');
							        }
							        else 
								        return false;
						        }
					        }
                            else if (ls.t.token == (int)NPLLex.RESERVED.TK_STRING)
					        {
						        // verify that the string is a value key(non-empty);
						        if(ls.t.seminfo.ts == string.Empty)
							        return false; 

						        NPLObjectProxy proxy_ = tabMsg[ls.t.seminfo.ts];
						        NPLParser.next(ls);
						        if(ls.t.token == ']')
						        {
							        NPLParser.next(ls);
							        if(ls.t.token == '=')
							        {
								        NPLParser.next(ls);
                                        if (!DeserializePureNPLDataBlock(ls, ref proxy_))
									        return false;
								        NPLParser.testnext(ls, ',');
							        }
							        else 
								        return false;
						        }
					        }
					        else
						        return false;
				        }
				        /// Fixed: 2008.6.3 LiXizhi
				        /// the following is for auto indexed table items {"string1", "string2\r\n", 213, nil,["A"]="B", true, false, {"another table", "field1"}}
				        else
				        {
					        NPLObjectProxy proxy_ = tabMsg[nTableAutoIndex++];
					        if( ! DeserializePureNPLDataBlock(ls, ref proxy_) )
						        return false;
					        NPLParser.testnext(ls, ',');
				        }
			        }
			        return true;
		        }
	        default:
		        break;
	        }
	        return false;
        }

        /// <summary>
        /// same as StringToNPLTable(), except that it begins with "msg={...}" 
        /// </summary>
        /// <param name="input">such as "msg={nid=10, name=\"value\", tab={name1=\"value1\"}}"</param>
        /// <returns>may return null if failed.</returns>
        public static NPLObjectProxy MsgStringToNPLTable(string input)
        {
	        NPLLex lex = new NPLLex();
            NPLLex.LexState ls = lex.SetInput(input);
	        ls.nestlevel = 0;

            try
            {
                NPLParser.next(ls);  /* read first token */

                if (ls.t.token == (int)NPLLex.RESERVED.TK_NAME && ls.t.seminfo.ts == "msg")
                {
                    NPLParser.next(ls);
                    if (ls.t.token == '=')
                    {
                        NPLParser.next(ls);
                        NPLObjectProxy output = new NPLObjectProxy();
                        if (DeserializePureNPLDataBlock(ls, ref output))
                        {
                            NPLParser.testnext(ls, ';');
                            if (ls.t.token == (int)NPLLex.RESERVED.TK_EOS)
                            {
                                return output;
                            }
                        }
                    }
                }
            }
            catch (Exception err)
            {
                ParaGlobal.applog("error:" + err.ToString() + "in NPLHelper::StringToNPLTable()");
                return null;
            }
            return null;
        }

        /// <summary>
        /// converting string to NPL table object 
        /// </summary>
        /// <param name="input">such as "{nid=10, name=\"value\", tab={name1=\"value1\"}}"</param>
        /// <returns>may return null if failed.</returns>
        public static NPLObjectProxy StringToNPLTable(string input)
        {
            NPLLex lex = new NPLLex();
            NPLLex.LexState ls = lex.SetInput(input);
	        ls.nestlevel = 0;

	        try
	        {
		        NPLParser.next(ls);  /* read first token */

		        if(ls.t.token == '{')
		        {
			        NPLObjectProxy output = new NPLObjectProxy();
                    if (DeserializePureNPLDataBlock(ls, ref output))
			        {
				        NPLParser.testnext(ls, ';');
                        if (ls.t.token == (int)NPLLex.RESERVED.TK_EOS)
				        {
					        return output;
				        }
			        }
		        }
	        }
            catch (Exception err)
            {
                ParaGlobal.applog("error:" + err.ToString() + "in NPLHelper::StringToNPLTable()");
                return null;
            }
            return null;
        }

        /// <summary>
        /// safe convert the NPL table to string. if the input is nil, output is not assigned. 
        /// </summary>
        /// <param name="sStorageVar">if null, output is pure table{}, otherwise it is sStorageVar={}</param>
        /// <param name="input"></param>
        /// <param name="sCode"></param>
        /// <param name="nCodeOffset"></param>
        /// <returns>true if input is a string object and value is written to output. </returns>
        public static bool SerializeNPLTableToString(string sStorageVar, NPLObjectProxy input, StringBuilder sCode, int nCodeOffset)
        {
	        sCode.Length = nCodeOffset;

	        int nStorageVarLen = 0;
	        if(sStorageVar!= null)
	        {
                nStorageVarLen = sStorageVar.Length;
                if (nStorageVarLen > 0)
                {
                    sCode.Append(sStorageVar);
                    sCode.Append("=");
                }
	        }

	        NPLObjectType nType = input.GetMyType();
	        switch(nType)
	        {
	        case NPLObjectType.NPLObjectType_Number:
		        {
			        double value = (double)input;
			        sCode.Append(value);
			        break;
		        }
	        case NPLObjectType.NPLObjectType_Bool:
		        {
			        bool bValue = (bool)input;
			        sCode.Append(bValue ? "true" : "false");
			        break;
		        }
	        case NPLObjectType.NPLObjectType_String:
		        {
			        // this is something like string.format("%q") in NPL.
                    string str = (string)input;
			        EncodeStringInQuotation(ref sCode, sCode.Length, str);
			        break;
		        }
	        case NPLObjectType.NPLObjectType_Table:
		        {
			        sCode.Append("{");
                    foreach (KeyValuePair<string, NPLObjectProxy> pair in input)
			        {
				        // we only serialize item with a string key
                        string key = pair.Key;
                        NPLObjectProxy value = pair.Value;
				        int nOldSize = sCode.Length;
				        // if sKey contains only alphabetic letters, we will use sKey=data,otherwise, we go the safer one ["sKey"]=data.
				        // the first is more efficient in disk space. 
                        if (NPLParser.IsIdentifier(key))
				        {
                            sCode.Append(key);
					        sCode.Append("=");
				        }
				        else
				        {
					        sCode.Append("[");
                            EncodeStringInQuotation(ref sCode, sCode.Length, key);
					        sCode.Append("]=");
				        }
				        if(SerializeNPLTableToString(null, value, sCode, sCode.Length))
				        {
					        sCode.Append(",");
				        }
				        else
				        {
					        sCode.Length = nOldSize;
				        }
			        }
			        sCode.Append("}");
			        break;
		        }
	        default:
		        // we will escape any functions, etc. 
		        if(nStorageVarLen > 0)
		        {
			        sCode.Length = nCodeOffset;
		        }
                return false;
		        //break;
	        }
	        return true;
        }

        public static bool NPLTableToString(string sStorageVar, NPLObjectProxy input, StringBuilder sCode)
        {
            return SerializeNPLTableToString(sStorageVar, input, sCode, 0);
        }
        public static bool NPLTableToString(string sStorageVar, NPLObjectProxy input, StringBuilder sCode, int nCodeOffset)
        {
            return SerializeNPLTableToString(sStorageVar, input, sCode, nCodeOffset);
        }
    }
}
#endregion

#region NPLWriter

namespace NPLMono
{
    /// <summary>
    /// a simple class for creating NPL script code, especially data table code. 
    /// this class is reentrant (thread-safe). Please note that this class does not ensure that the code is a pure table.
	/// See Example:
	/// // to generate the string : msg={name=1,2,{"3"="4",},};
	/// NPLWriter writer = new NPLWriter();
	/// writer.WriteName("msg");
	/// writer.BeginTable();
	/// writer.WriteName("name");
	/// writer.WriteValue(1);
	/// writer.WriteValue(2);
	/// writer.BeginTable();
	/// writer.WriteName("3", true);
	/// writer.WriteValue("4");
	/// writer.EndTable();
	/// writer.EndTable();
	/// writer.WriteParamDelimiter();
	/// ParaGlobal.log(writer.ToString());
    /// 
	/// One can also provide their own string buffer to write to, like below.
    /// StringBuilder buff = new StringBuilder();
	/// buff.EnsureCapacity(100);
    /// NPLWriter writer = new NPLWriter(buff);
    /// 
    /// </summary>
    public class NPLWriter
    {
        /** the internal buffer reserved size. */
		public NPLWriter(int nReservedSize)
        {
            m_sCode = new StringBuilder(nReservedSize);
            m_bBeginAssignment = false;
            m_nTableLevel = 0;
        }
        
        public NPLWriter()
        {
            m_sCode = new StringBuilder();
            m_bBeginAssignment = false;
            m_nTableLevel = 0;
        }
        
		public NPLWriter(StringBuilder buff_)
        {
            m_sCode = buff_;
            m_bBeginAssignment = false;
            m_nTableLevel = 0;
        }

        public void Reset() { Reset(-1); }

        public void Reset(int nReservedSize)
        {
            m_sCode.Length = 0;
            if (nReservedSize > 0)
                m_sCode.EnsureCapacity(nReservedSize);
        }

        public void WriteName(string name) { WriteName(name, false); }

        /// <summary>
        ///  write a parameter name
        /// </summary>
        /// <param name="name"></param>
        /// <param name="bUseBrackets">if false, one has to make sure that the name is a valid NPL name string, without special characters. </param>
        public void WriteName( string name, bool bUseBrackets /*= false*/ )
        {
	        if(name!=null)
	        {
		        m_bBeginAssignment = true;
		        if(!bUseBrackets)
		        {
                    m_sCode.Append(name);
		        }
		        else
		        {
                    m_sCode.Append("[\"");
			        m_sCode.Append(name);
                    m_sCode.Append("\"]");
		        }
	        }
        }
        public void WriteValue(string value) { WriteValue(value, true); }

        /// <summary>
        /// rite a parameter value
        /// </summary>
        /// <param name="value"></param>
        /// <param name="bInQuotation">if bInQuotation is true, it writes a parameter text value. Otherwise it will just append the value without encoding it with quotation marks. </param>
        public void WriteValue(string value, bool bInQuotation/*=true*/ )
        {
            if(value == null)
            {
	            WriteNil();
            }
            if(m_bBeginAssignment)
            {
	            m_sCode.Append('=');
            }
            if(bInQuotation)
            {
	            NPLHelper.EncodeStringInQuotation(ref m_sCode, m_sCode.Length, value);
            }
            else
            {
	            m_sCode.Append(value);
            }
            if(m_nTableLevel>0)
                m_sCode.Append(',');
            m_bBeginAssignment = false;
        }

        public void WriteValue(double value)
        {
            WriteValue(value.ToString(), false);
        }

        public void WriteNil()
        {
            WriteValue("nil", false);
        }
        public void WriteParamDelimiter() { m_sCode.Append(';'); }

        /// <summary>
        /// append any text 
        /// </summary>
        /// <param name="sText"></param>
        public void Append(string sText) { m_sCode.Append(sText); }

        public void BeginTable()
        {
	        m_sCode.Append(m_bBeginAssignment ? "={" : "{");
	        m_nTableLevel++;
	        m_bBeginAssignment = false;
        }

        public void EndTable()
        {
            m_sCode.Append('}');
	        if((--m_nTableLevel)>0)
                m_sCode.Append(',');
        }

        /** get the current NPL code. */
        public override string ToString() { return m_sCode.ToString(); }

        /// <summary>
        /// 
        /// </summary>
        /// <returns>"msg=nil;"</returns>
        public static string GetNilMessage() { return "msg=nil;"; }

        /// <summary>
        /// 
        /// </summary>
        /// <returns> "msg={};"</returns>
		public static string GetEmptyMessage() { return  "msg={};"; }

        private bool m_bBeginAssignment;
        private int m_nTableLevel;
        private StringBuilder m_sCode;
    }
}
#endregion

#region NPLLex
namespace NPLMono
{
    /// <summary>
    /// lexer for NPL files. It is the same lexer of lua 5.1
    /// </summary>
    public class NPLLex
    {
        private static string[] token2string = {
	        "and", "break", "do", "else", "elseif",
	        "end", "false", "for", "function", "if",
	        "in", "local", "nil", "not", "or", "repeat",
	        "return", "then", "true", "until", "while", "*name",
	        "..", "...", "==", ">=", "<=", "~=",
	        "*number", "*string", "<eof>"
        };

        /* end of stream */
        public const int EOZ = -1;
        public const int MAX_INT = 65530;
        public const int FIRST_RESERVED = 257;

        /* maximum number of chars that can be read without checking buffer size */
        public const int MAXNOCHECK = 5;
        /* extra space to allocate when growing buffer */
        public const int EXTRABUFF = 32;

        /* maximum length of a reserved word */
        public const int TOKEN_LEN = 8; // "function".Length;
        public const int UCHAR_MAX = 0xff;

        public enum RESERVED
        {
            /* terminal symbols denoted by reserved words */
            TK_AND = FIRST_RESERVED, TK_BREAK,
            TK_DO, TK_ELSE, TK_ELSEIF, TK_END, TK_FALSE, TK_FOR, TK_FUNCTION,
            TK_IF, TK_IN, TK_LOCAL, TK_NIL, TK_NOT, TK_OR, TK_REPEAT,
            TK_RETURN, TK_THEN, TK_TRUE, TK_UNTIL, TK_WHILE,
            /* other terminal symbols */
            TK_NAME, TK_CONCAT, TK_DOTS, TK_EQ, TK_GE, TK_LE, TK_NE, TK_NUMBER,
            TK_STRING, TK_EOS
        };
        /* number of reserved words */
        const int NUM_RESERVED = (int)RESERVED.TK_WHILE - FIRST_RESERVED + 1;

        /* semantics information */
        public class SemInfo
        {
            public double r;
            public string ts;
            public SemInfo(double r_, string ts_)
            {
                r = r_;
                ts = ts_;
            }
        };

        public class Token
        {
            public int token;
            public SemInfo seminfo;
            public Token(int token_, SemInfo seminfo_)
            {
                token = token_;
                seminfo = seminfo_;
            }
        };

        public class Zio
        {
            public int nCurrentPosition;			/* bytes still unread */
            public string data;		/* current position in buffer */
            private int dataLength;

            public Zio() { }
            public Zio(string input)
            {
                setinput(input);
            }
            public void setinput(string input)
            {
                data = input;
                dataLength = data.Length;
                nCurrentPosition = 0;
            }
            /// <summary>
            /// return the current character, but increase the pointer to the next one.
            /// </summary>
            /// <returns></returns>
            public int next()
            {
                if (nCurrentPosition < dataLength)
                {
                    return (int)(data[nCurrentPosition++]);
                }
                else
                {
                    return EOZ;
                }
            }
        };

        public class LexState
        {
            public int current;  /* current character (charint) */
            public int linenumber;  /* input line counter */
            public int lastline;  /* line of last token `consumed' */
            public Token t;  /* current token */
            public Token lookahead;  /* look ahead token */
            public Zio z;  /* input stream */
            public char[] buff;  /* buffer for tokens */
            public int nestlevel;  /* level of nested non-terminals */
            /** true if no error*/
            public bool bSucceed;

            public LexState()
            {
                current = EOZ;
                linenumber = 0;
                lastline = 0;
                z = null;
                buff = new char[0];
                nestlevel = 0;
                bSucceed = true;
                t = new Token(EOZ, new SemInfo(0, ""));
                lookahead = new Token(EOZ, new SemInfo(0, ""));
            }
        };

        public LexState m_lexState;
        public Zio m_zio;

        public NPLLex()
        {
            m_lexState = new LexState();
            m_zio = new Zio();
        }
        //////////////////////////////////////////////////////////////////////////
        // static lex functions
        //////////////////////////////////////////////////////////////////////////
        public static int next(LexState LS)
        {
            return LS.current = LS.z.next();
        }

        public static void checkbuffer(LexState LS, int len)
        {
            if (((len) + MAXNOCHECK) > LS.buff.Length)
            {
                if (len < 500)
                    Array.Resize(ref LS.buff, len + EXTRABUFF);
                else
                {
                    // added by LiXizhi 2007.6.20: in case the file contains super large string, such as a base64 encoded file of 2MB, we will double the size instead using a fixed length.
                    Array.Resize(ref LS.buff, len * 2);
                }
            }
        }

        public static void save(LexState LS, char c, ref int l)
        {
            LS.buff[l++] = c;
        }
        public static void save_and_next(LexState LS, ref int l)
        {
            save(LS, (char)LS.current, ref l);
            next(LS);
        }

        public LexState GetState() { return m_lexState; }

        /**
		* set the input stream. and clean up all states
		* @param input code to be parsed
		* @param nLen length of the code
		* @return the state
		*/
        public LexState SetInput(string input)
        {
            m_zio.setinput(input);

            m_lexState.bSucceed = true;
            m_lexState.lookahead.token = (int)RESERVED.TK_EOS;  /* no look-ahead token */
            m_lexState.z = m_zio;
            m_lexState.linenumber = 1;
            m_lexState.lastline = 1;
            m_lexState.buff = new char[0];

            next(m_lexState);  /* read first char */
            if (m_lexState.current == '#')
            {
                do
                {  /* skip first line */
                    next(m_lexState);
                } while (m_lexState.current != '\n' && m_lexState.current != EOZ);
            }
            return m_lexState;
        }

        public static void luaX_errorline(LexState ls, string s, string token, int line)
        {
            ThrowError(ls, string.Format("{0}: {1} near '{2}'", line, s, token));
        }

        public static void luaX_error(LexState ls, string s, string token)
        {
            luaX_errorline(ls, s, token, ls.linenumber);
        }

        public static void luaX_syntaxerror(LexState ls, string msg)
        {
            string lasttoken;
            switch (ls.t.token)
            {
                case (int)RESERVED.TK_NAME:
                    lasttoken = ls.t.seminfo.ts;
                    break;
                case (int)RESERVED.TK_STRING:
                case (int)RESERVED.TK_NUMBER:
                    lasttoken = new string(ls.buff);
                    break;
                default:
                    lasttoken = luaX_token2str(ls, ls.t.token);
                    break;
            }
            luaX_error(ls, msg, lasttoken);
        }

        public static void luaX_checklimit(LexState ls, int val, int limit, string msg)
        {
            if (val > limit)
            {
                msg = string.Format("too many {0} (limit={1})", msg, limit);
                luaX_syntaxerror(ls, msg);
            }
        }
        public static void luaX_lexerror(LexState ls, string s, int token)
        {
            if (token == (int)RESERVED.TK_EOS)
                luaX_error(ls, s, luaX_token2str(ls, token));
            else
                luaX_error(ls, s, new string(ls.buff));
        }

        public static string luaX_token2str(LexState ls, int token)
        {
            if (token < FIRST_RESERVED)
            {
                return string.Format("{0}", token);
            }
            else
                return token2string[token - FIRST_RESERVED];
        }

        public static void ThrowError(LexState ls, string errorMsg)
        {
            ls.bSucceed = false;
            throw new Exception(errorMsg);
        }

        public static void inclinenumber(LexState LS)
        {
            next(LS);  /* skip `\n' */
            ++LS.linenumber;
            luaX_checklimit(LS, LS.linenumber, MAX_INT, "lines in a chunk");
        }

        public static void read_long_string(LexState LS, SemInfo seminfo)
        {
            int cont = 0;
            int l = 0;
            checkbuffer(LS, l);
            save(LS, '[', ref l);  /* save first `[' */
            save_and_next(LS, ref l);  /* pass the second `[' */
            if (LS.current == '\n' || LS.current == '\r')  /* string starts with a newline? */
                inclinenumber(LS);  /* skip it */
            bool bBreak = false;
            for (; !bBreak; )
            {
                checkbuffer(LS, l);
                switch (LS.current)
                {
                    case EOZ:
                        save(LS, '\0', ref l);
                        luaX_lexerror(LS, (seminfo != null) ? "unfinished long string" :
                                                   "unfinished long comment", (int)RESERVED.TK_EOS);
                        break;  /* to avoid warnings */
                    case '[':
                        save_and_next(LS, ref l);
                        if (LS.current == '[')
                        {
                            cont++;
                            save_and_next(LS, ref l);
                        }
                        continue;
                    case ']':
                        save_and_next(LS, ref l);
                        if (LS.current == ']')
                        {
                            if (cont == 0) bBreak = true;
                            cont--;
                            save_and_next(LS, ref l);
                        }
                        continue;
                    case '\n':
                    case '\r': // lua 5.1 syntax fix
                        save(LS, '\n', ref l);
                        inclinenumber(LS);
                        if (seminfo == null) l = 0;  /* reset buffer to avoid wasting space */
                        continue;
                    default:
                        save_and_next(LS, ref l);
                        break;
                }
            }
            save_and_next(LS, ref l);  /* skip the second `]' */
            save(LS, '\0', ref l);
            if (seminfo != null)
            {
                seminfo.ts = new string(LS.buff, 2, l - 5);
            }
        }
        public static void read_string(LexState LS, int del, SemInfo seminfo)
        {
            int l = 0;
            checkbuffer(LS, l);
            save_and_next(LS, ref l);
            while (LS.current != del)
            {
                checkbuffer(LS, l);
                switch (LS.current)
                {
                    case EOZ:
                        save(LS, '\0', ref l);
                        luaX_lexerror(LS, "unfinished string", (int)RESERVED.TK_EOS);
                        break;  /* to avoid warnings */
                    case '\n':
                    case '\r': // lua 5.1 syntax fix
                        save(LS, '\0', ref l);
                        luaX_lexerror(LS, "unfinished string", (int)RESERVED.TK_STRING);
                        break;  /* to avoid warnings */
                    case '\\':
                        next(LS);  /* do not save the `\' */
                        switch (LS.current)
                        {
                            case 'a': save(LS, '\a', ref l); next(LS); break;
                            case 'b': save(LS, '\b', ref l); next(LS); break;
                            case 'f': save(LS, '\f', ref l); next(LS); break;
                            case 'n': save(LS, '\n', ref l); next(LS); break;
                            case 'r': save(LS, '\r', ref l); next(LS); break;
                            case 't': save(LS, '\t', ref l); next(LS); break;
                            case 'v': save(LS, '\v', ref l); next(LS); break;
                            case '\n':
                            case '\r': // lua 5.1 syntax fix
                                save(LS, '\n', ref l); inclinenumber(LS); break;
                            case EOZ: break;  /* will raise an error next loop */
                            default:
                                {
                                    if (!(char.IsDigit((char)(LS.current))))
                                        save_and_next(LS, ref l);  /* handles \\, \", \', and \? */
                                    else
                                    {  /* \xxx */
                                        int c = 0;
                                        int i = 0;
                                        do
                                        {
                                            c = 10 * c + (LS.current - '0');
                                            next(LS);
                                        } while (++i < 3 && (char.IsDigit((char)(LS.current))));
                                        if (c > UCHAR_MAX)
                                        {
                                            save(LS, '\0', ref l);
                                            luaX_lexerror(LS, "escape sequence too large", (int)RESERVED.TK_STRING);
                                        }
                                        save(LS, (char)c, ref l);
                                    }
                                    break;
                                }
                        }
                        break;
                    default:
                        save_and_next(LS, ref l);
                        break;
                }
            }
            save_and_next(LS, ref l);  /* skip delimiter */
            save(LS, '\0', ref l);
            if (seminfo != null)
            {
                seminfo.ts = new string(LS.buff, 1, l - 3);
            }
        }
        public static int readname(LexState LS)
        {
            int l = 0;
            checkbuffer(LS, l);
            do
            {
                checkbuffer(LS, l);
                save_and_next(LS, ref l);
            } while (Char.IsLetterOrDigit((char)LS.current) || LS.current == '_');
            save(LS, '\0', ref l);
            return l - 1;
        }

        /* LUA_NUMBER */
        public static void read_numeral(LexState LS, int comma, SemInfo seminfo)
        {
            int l = 0;
            checkbuffer(LS, l);
            if (comma > 0) save(LS, '.', ref l);
            while (Char.IsDigit((char)LS.current))
            {
                checkbuffer(LS, l);
                save_and_next(LS, ref l);
            }
            if (LS.current == '.')
            {
                save_and_next(LS, ref l);
                if (LS.current == '.')
                {
                    save_and_next(LS, ref l);
                    save(LS, '\0', ref l);
                    luaX_lexerror(LS,
                        "ambiguous syntax (decimal point x string concatenation)",
                        (int)RESERVED.TK_NUMBER);
                }
            }
            while (Char.IsDigit((char)LS.current))
            {
                checkbuffer(LS, l);
                save_and_next(LS, ref l);
            }
            if (LS.current == 'e' || LS.current == 'E')
            {
                save_and_next(LS, ref l);  /* read `E' */
                if (LS.current == '+' || LS.current == '-')
                    save_and_next(LS, ref l);  /* optional exponent sign */
                while (Char.IsDigit((char)LS.current))
                {
                    checkbuffer(LS, l);
                    save_and_next(LS, ref l);
                }
            }
            save(LS, '\0', ref l);
            try
            {
                seminfo.r = Convert.ToDouble(new string(LS.buff, 0, l - 1));
            }
            catch (Exception e)
            {
                luaX_lexerror(LS, "malformed number" + e.ToString(), (int)RESERVED.TK_NUMBER);
            }
        }

        public static int luaX_lex(LexState LS, SemInfo seminfo)
        {
            for (; ; )
            {
                switch (LS.current)
                {

                    case '\n':
                    case '\r':
                        {
                            inclinenumber(LS);
                            continue;
                        }
                    case '-':
                        {
                            next(LS);
                            if (LS.current != '-') return '-';
                            /* else is a comment */
                            next(LS);
                            if (LS.current == '[' && (next(LS) == '['))
                            {
                                read_long_string(LS, null);  /* long comment */
                            }
                            else
                            {/* short comment */
                                while (LS.current != '\n' && LS.current != '\r' && LS.current != EOZ)
                                    next(LS);
                            }
                            continue;
                        }
                    case '[':
                        {
                            next(LS);
                            if (LS.current != '[') return '[';
                            else
                            {
                                read_long_string(LS, seminfo);
                                return (int)RESERVED.TK_STRING;
                            }
                        }
                    case '=':
                        {
                            next(LS);
                            if (LS.current != '=') return '=';
                            else { next(LS); return (int)RESERVED.TK_EQ; }
                        }
                    case '<':
                        {
                            next(LS);
                            if (LS.current != '=') return '<';
                            else { next(LS); return (int)RESERVED.TK_LE; }
                        }
                    case '>':
                        {
                            next(LS);
                            if (LS.current != '=') return '>';
                            else { next(LS); return (int)RESERVED.TK_GE; }
                        }
                    case '~':
                        {
                            next(LS);
                            if (LS.current != '=') return '~';
                            else { next(LS); return (int)RESERVED.TK_NE; }
                        }
                    case '"':
                    case '\'':
                        {
                            read_string(LS, LS.current, seminfo);
                            return (int)RESERVED.TK_STRING;
                        }
                    case '.':
                        {
                            next(LS);
                            if (LS.current == '.')
                            {
                                next(LS);
                                if (LS.current == '.')
                                {
                                    next(LS);
                                    return (int)RESERVED.TK_DOTS;   /* ... */
                                }
                                else return (int)RESERVED.TK_CONCAT;   /* .. */
                            }
                            else if (!Char.IsDigit((char)LS.current)) return '.';
                            else
                            {
                                read_numeral(LS, 1, seminfo);
                                return (int)RESERVED.TK_NUMBER;
                            }
                        }
                    case EOZ:
                        {
                            return (int)RESERVED.TK_EOS;
                        }
                    default:
                        {
                            if (Char.IsWhiteSpace((char)LS.current))
                            {
                                next(LS);
                                continue;
                            }
                            else if (Char.IsDigit((char)LS.current))
                            {
                                read_numeral(LS, 0, seminfo);
                                return (int)RESERVED.TK_NUMBER;
                            }
                            else if (char.IsLetter((char)LS.current) || LS.current == '_')
                            {
                                /* identifier or reserved word */
                                int l = readname(LS);
                                string ts = new string(LS.buff, 0, l);

                                // The following code implemented below
                                //if (ts.tsv.reserved > 0)  /* reserved word? */
                                // 	return ts.tsv.reserved - 1 + FIRST_RESERVED;

                                /* reserved word? */
                                for (int i = 0; i < NUM_RESERVED; ++i)
                                {
                                    if (ts == token2string[i])
                                        return i + FIRST_RESERVED;
                                }

                                seminfo.ts = ts;
                                return (int)RESERVED.TK_NAME;
                            }
                            else
                            {
                                int c = LS.current;
                                if (Char.IsControl((char)c))
                                    luaX_error(LS, "invalid control char",
                                        string.Format("char(0})", c));
                                next(LS);
                                return c;  /* single-char tokens (+ - / ...) */
                            }
                            //break;
                        }
                }
            }//for (;;) {
        }

    }
}
#endregion

#region NPLParser
namespace NPLMono
{
    public class NPLParser
    {
        /*
		** maximum number of syntactical nested non-terminals: Not too big,
		** or may overflow the C stack...
		*/
        public const int LUA_MAXPARSERLEVEL = 200;

        public static void enterlevel(NPLLex.LexState ls)
        {
            if (++(ls.nestlevel) > LUA_MAXPARSERLEVEL)
                NPLLex.luaX_syntaxerror(ls, "too many syntax levels");
        }

        public static void leavelevel(NPLLex.LexState ls)
        {
            --ls.nestlevel;
        }

        public static void next(NPLLex.LexState ls)
        {
            ls.lastline = ls.linenumber;
            if (ls.lookahead.token != (int)NPLLex.RESERVED.TK_EOS)
            {  /* is there a look-ahead token? */
                ls.t = ls.lookahead;  /* use this one */
                ls.lookahead.token = (int)NPLLex.RESERVED.TK_EOS;  /* and discharge it */
            }
            else
                ls.t.token = NPLLex.luaX_lex(ls, ls.t.seminfo);  /* read next token */
        }
        public static void lookahead(NPLLex.LexState ls)
        {
            System.Diagnostics.Debug.Assert(ls.lookahead.token == (int)NPLLex.RESERVED.TK_EOS);
            ls.lookahead.token = NPLLex.luaX_lex(ls, ls.lookahead.seminfo);
        }
        public static void error_expected(NPLLex.LexState ls, int token)
        {
            NPLLex.luaX_syntaxerror(ls,
                string.Format("'{0}' expected", NPLLex.luaX_token2str(ls, token)));
        }
        public static int testnext(NPLLex.LexState ls, int c)
        {
            if (ls.t.token == c)
            {
                next(ls);
                return 1;
            }
            else return 0;
        }
        public static void check(NPLLex.LexState ls, int c)
        {
            if (testnext(ls, c) == 0)
                error_expected(ls, c);
        }

        public static void check_match(NPLLex.LexState ls, int what, int who, int where)
        {
            if (testnext(ls, what) == 0)
            {
                if (where == ls.linenumber)
                    error_expected(ls, what);
                else
                {
                    NPLLex.luaX_syntaxerror(ls, string.Format("'{0}' expected (to close `{1}' at line {2})",
                        NPLLex.luaX_token2str(ls, what), NPLLex.luaX_token2str(ls, who), where));
                }
            }
        }

        /// <summary>
        /// Parse: data | table
        /// NPL uses a similar element name for basic data types as in soap and xml/rpc
        /// Supported NPL types are table, boolean, string, double, index, nil. 
        /// e.g. the NPL table msg={x=2, [1]="string value",OK=true} will be serialized to following table.
        /// <msg>
        ///     <x type="double">2</x>
        ///     <param index=1 type="string" >string value</param>
        ///     <OK type="boolean">true</OK>
        /// </msg>
        /// the input NPL table key name better not be the above name to avoid confusion. 
        /// </summary>
        /// <param name="doc"></param>
        /// <param name="ls"></param>
        /// <param name="ParentNode"></param>
        /// <returns>true if there are data to fill </returns>
        public static bool FillDataToXmlElement(XmlDocument doc, NPLLex.LexState ls, XmlElement ParentNode)
        {
            // data
            int c = ls.t.token;
            switch (c)
            {
                case (int)NPLLex.RESERVED.TK_TRUE:
                    {
                        ParentNode.SetAttribute("type", "boolean");
                        ParentNode.InnerText = "true";
                        next(ls);
                        return true;
                    }
                case (int)NPLLex.RESERVED.TK_FALSE:
                    {
                        ParentNode.SetAttribute("type", "boolean");
                        ParentNode.InnerText = "false";
                        next(ls);
                        return true;
                    }
                case (int)NPLLex.RESERVED.TK_NIL:
                    {
                        ParentNode.SetAttribute("type", "nil");
                        next(ls);
                        return true;
                    }
                case (int)NPLLex.RESERVED.TK_NUMBER:
                    {
                        ParentNode.SetAttribute("type", "double");
                        // TODO: what kind of encoding should we use? otherwise, we may be sending long string, like 0.00000000
                        ParentNode.InnerText = ls.t.seminfo.r.ToString();
                        next(ls);
                        return true;
                    }
                case (int)NPLLex.RESERVED.TK_STRING:
                    {
                        ParentNode.SetAttribute("type", "string");
                        /// Note:2008.3.27: I found that the xmlserializer strips away \r\n out during deserialization (and just leave \n). WebService strips out \r\n (an leave \n) even using XMLNode.SetInnerText(). 
                        /// so the msg code sending using InnerText of XML web service call will have \r removed. This leads to errors for scode. 
                        /// A workaround is to replace \r and \n with &#x000d; and &#x000a; respectively in FillDataToXmlElement of NPLParser.cs for string type. In fact, I just replace \r, since \n is not removed. 
                        /// 
                        // ParentNode.InnerText = ls.t.seminfo.ts;
                        ParentNode.InnerText = ls.t.seminfo.ts.Replace("\r", "&#x000d;");
                        next(ls);
                        return true;
                    }
                case '-':
                    {
                        // negative number
                        next(ls);
                        if (ls.t.token == (int)NPLLex.RESERVED.TK_NUMBER)
                        {
                            ParentNode.SetAttribute("type", "double");
                            // TODO: what kind of encoding should we use? otherwise, we may be sending long string, like 0.00000000
                            ParentNode.InnerText = "-" + ls.t.seminfo.r.ToString();
                            next(ls);
                            return true;
                        }
                        else
                            return false;
                    }
                case '{':
                    {
                        enterlevel(ls);
                        bool bBreak = false;
                        next(ls);
                        int nAutoIndex = 1;
                        while (!bBreak)
                        {
                            c = ls.t.token;
                            if (c == '}')
                            {
                                // end of table
                                leavelevel(ls);
                                next(ls);
                                bBreak = true;
                            }
                            else if (c == (int)NPLLex.RESERVED.TK_NAME)
                            {
                                XmlElement node = doc.CreateElement(ls.t.seminfo.ts);

                                // by name assignment, such as name = data|table
                                next(ls);
                                if (ls.t.token == '=')
                                {
                                    next(ls);
                                    if (!FillDataToXmlElement(doc, ls, node))
                                        return false;
                                    testnext(ls, ',');
                                    ParentNode.AppendChild(node);
                                }
                                else
                                    return false;
                            }
                            else if (c == '[')
                            {
                                // by integer or string key assignment, such as [number|String] = data|table
                                next(ls);
                                XmlElement node = null;
                                if (ls.t.token == (int)NPLLex.RESERVED.TK_NUMBER)
                                {
                                    // verify that it is an integer, instead of a floating value.
                                    node = doc.CreateElement("param");
                                    node.SetAttribute("index", Convert.ToInt16(ls.t.seminfo.r).ToString());
                                }
                                else if (ls.t.token == (int)NPLLex.RESERVED.TK_STRING)
                                {
                                    // verify that the string is a value key(non-empty);
                                    if (ls.t.seminfo.ts.Length == 0)
                                        return false;
                                    node = doc.CreateElement(ls.t.seminfo.ts);
                                }
                                else
                                    return false;
                                next(ls);
                                if (ls.t.token == ']')
                                {
                                    next(ls);
                                    if (ls.t.token == '=')
                                    {
                                        next(ls);
                                        if (!FillDataToXmlElement(doc, ls, node))
                                            return false;
                                        testnext(ls, ',');
                                        ParentNode.AppendChild(node);
                                    }
                                    else
                                        return false;
                                }
                            }
                            /// Fixed: 2008.6.3 LiXizhi
                            /// the following is for auto indexed table items {"string1", "string2\r\n", 213, nil,["A"]="B", true, false, {"another table", "field1"}}
                            else if (c == (int)NPLLex.RESERVED.TK_STRING || c == (int)NPLLex.RESERVED.TK_NUMBER || c == (int)NPLLex.RESERVED.TK_NIL || c == (int)NPLLex.RESERVED.TK_FALSE || c == (int)NPLLex.RESERVED.TK_TRUE || c == '{')
                            {
                                XmlElement node = doc.CreateElement("param");
                                node.SetAttribute("index", nAutoIndex.ToString());
                                if (!FillDataToXmlElement(doc, ls, node))
                                    return false;
                                testnext(ls, ',');
                                ParentNode.AppendChild(node);
                                nAutoIndex++;
                            }
                            else
                            {
                                return false;
                            }
                        };
                        return true;
                    }
                default:
                    return false;
            }
        }

        /// <summary>
        /// Parse: name = data | table
        /// Basically, there is only one statement like msg = {...} in a web service or NPL activation call.
        /// </summary>
        /// <param name="doc"></param>
        /// <param name="ls"></param>
        /// <returns> XmlElement with the name is returned. null is returned if input is invalid</returns>
        public static XmlElement DataStatementToXmlElement(XmlDocument doc, NPLLex.LexState ls)
        {
            if (ls.t.token == (int)NPLLex.RESERVED.TK_NAME && ls.t.seminfo.ts == "msg")
            {
                XmlElement node = doc.CreateElement(ls.t.seminfo.ts);
                next(ls);
                if (ls.t.token == '=')
                {
                    next(ls);
                    if (FillDataToXmlElement(doc, ls, node))
                    {
                        return node;
                    }
                }
            }
            return null;
        }

        /// <summary>
        /// converting from NPL code to xml element
        /// </summary>
        /// <param name="npl_code">the statement must be like msg = {...}</param>
        /// <returns></returns>
        public static XmlElement NPLCodeToXmlElement(string npl_code)
        {
            NPLLex lex = new NPLLex();
            NPLLex.LexState ls = lex.SetInput(npl_code);
            ls.nestlevel = 0;

            XmlDocument doc = new XmlDocument();

            try
            {
                /** read first token*/
                next(ls);

                XmlElement result = DataStatementToXmlElement(doc, ls);
                if (result != null)
                {
                    testnext(ls, ';');
                    if (ls.t.token == (int)NPLLex.RESERVED.TK_EOS)
                    {
                        return result;
                    }
                    else
                    {
                        throw new Exception("only one table or data statement in input string is allowed.");
                    }
                }
            }
            catch (Exception e)
            {
                ParaGlobal.log(e.ToString());
                ParaGlobal.log("\r\n");
                return null;
            }
            return null;
        }

        /** whether the string is an identifier. 
		* Identifiers in npl can be any string of letters, digits, and underscores, not beginning with a digit. This coincides with the definition of identifiers in most languages. (The definition of letter depends on the current locale: any character considered alphabetic by the current locale can be used in an identifier.)
		* @param str: string
		* @param nLength: it must be a valid length.
		*/
		public static bool IsIdentifier(string input)
        {
            int nLength = input.Length;
            bool bIsIdentifier = nLength>0 && !Char.IsDigit(input[0]);
            for (int i = 0; i < nLength && bIsIdentifier; ++i)
            {
                char c = input[i];
                bIsIdentifier = (Char.IsLetterOrDigit(c) || c == '_');
            }
            return bIsIdentifier;
        }
    }
}
#endregion

#region NPLTable
namespace NPLMono
{
    public enum NPLObjectType 
	{
		NPLObjectType_Nil, 
		NPLObjectType_Table, 
		NPLObjectType_Number, 
		NPLObjectType_String, 
		NPLObjectType_Bool,
	}

    /// <summary>
    /// /** NPL object proxy. This is both a smart pointer and accessors
	/// // Example 1: Create NPLTable and serialize to string
    /// NPLObjectProxy msg = new NPLObjectProxy();
    /// msg["nid"].Assign(10);
	/// msg["name"].Assign("value");
	/// msg["tab"]["name1"].Assign("value1");
    /// StringBuilder output = new StringBuilder();
    /// NPLHelper.NPLTableToString(null, msg, output);
    /// 	
	/// // Example 2: serialized NPLTable from string. 
	/// 
	/// NPLObjectProxy tabMsg = NPLHelper.StringToNPLTable("{nid=10, name=\"value\", tab={name1=\"value1\"}}");
    /// ParaGlobal.applog(String.Format("Example 5: {0}==10, {1}==value, {2}==value1", (double)tabMsg["nid"], (string)tabMsg["name"], (string)(tabMsg["tab"]["name1"])));
    /// </summary>
    public class NPLObjectProxy : System.Collections.IEnumerable
    {
        public NPLObjectProxy() {m_ptr=null;}
        public NPLObjectProxy(NPLObjectBase obj) {m_ptr=obj;}

        public NPLObjectBase get() { return m_ptr; }

        /** make the object invalid */
        public void MakeNil() { m_ptr = null;  }

        public NPLObjectType GetMyType() { return (m_ptr != null) ? m_ptr.GetMyType() : NPLObjectType.NPLObjectType_Nil; }

        public System.Collections.IEnumerator GetEnumerator()
        {
            if(get()==null ||  GetMyType() != NPLObjectType.NPLObjectType_Table)
	        {
                m_ptr = new NPLTable();
	        }
            return (m_ptr as NPLTable).GetEnumerator();
        }

        public void Assign(NPLObjectProxy value)
        {
            m_ptr = value.m_ptr;
        }

        public void Assign(bool value) 
        {
            if(get()==null ||  GetMyType() != NPLObjectType.NPLObjectType_Bool)
            {
                m_ptr = new NPLBoolObject();
	        }
            (m_ptr as NPLBoolObject).SetValue(value);

        }
        public void Assign(double value) 
        {
            if(get()==null ||  GetMyType() != NPLObjectType.NPLObjectType_Number)
            {
                m_ptr = new NPLNumberObject();
	        }
            (m_ptr as NPLNumberObject).SetValue(value);
        }
        public void Assign(int value)
        {
            if (get() == null || GetMyType() != NPLObjectType.NPLObjectType_Number)
            {
                m_ptr = new NPLNumberObject();
            }
            (m_ptr as NPLNumberObject).SetValue((double)value);
        }

        public void Assign(string value) 
        {
            if(get()==null ||  GetMyType() != NPLObjectType.NPLObjectType_String)
            {
                m_ptr = new NPLStringObject();
	        }
            (m_ptr as NPLStringObject).SetValue(value);
        }

        public static implicit operator bool ( NPLObjectProxy arg1)
        {
            if (arg1.get() == null || arg1.GetMyType() != NPLObjectType.NPLObjectType_Bool)
		        arg1.m_ptr = new NPLBoolObject();
	        return (arg1.m_ptr as NPLBoolObject).GetValue();
        }

        public static implicit operator int(NPLObjectProxy arg1)
        {
            if (arg1.get() == null || arg1.GetMyType() != NPLObjectType.NPLObjectType_Number)
                arg1.m_ptr = new NPLNumberObject();
            return (int)((arg1.m_ptr as NPLNumberObject).GetValue());
        }

        public static implicit operator double ( NPLObjectProxy arg1)
        {
            if (arg1.get() == null || arg1.GetMyType() != NPLObjectType.NPLObjectType_Number)
		        arg1.m_ptr = new NPLNumberObject();
	        return (arg1.m_ptr as NPLNumberObject).GetValue();
        }

        public static implicit operator long (NPLObjectProxy arg1)
        {
            if (arg1.get() == null || arg1.GetMyType() != NPLObjectType.NPLObjectType_Number)
                arg1.m_ptr = new NPLNumberObject();
            return (long)((arg1.m_ptr as NPLNumberObject).GetValue());
        }

        public static implicit operator string ( NPLObjectProxy arg1)
        {
            if (arg1.get() == null || arg1.GetMyType() != NPLObjectType.NPLObjectType_String)
		        arg1.m_ptr = new NPLStringObject();
	        return (arg1.m_ptr as NPLStringObject).GetValue();
        }

        NPLObjectProxy GetField(string sName)
        {
	       if (get() == null || GetMyType() != NPLObjectType.NPLObjectType_Table)
            {
                return null;
            }
	        else
	        {
		        return (m_ptr as NPLTable).GetField(sName);
	        }
        }

        /** this will create get field. */
        public NPLObjectProxy this[string sName]
        {
            set {
                if (get() == null || GetMyType() != NPLObjectType.NPLObjectType_Table)
                {
                    m_ptr = new NPLTable();
                }
                (m_ptr as NPLTable)[sName] = value;
            }
            get {
                if (get() == null || GetMyType() != NPLObjectType.NPLObjectType_Table)
                {
                    m_ptr = new NPLTable();
                }
                return (m_ptr as NPLTable).CreateGetField(sName);
            }
        }
        public NPLObjectProxy this[int nIndex]
        {
            set
            {
                if (get() == null || GetMyType() != NPLObjectType.NPLObjectType_Table)
                {
                    m_ptr = new NPLTable();
                }
                (m_ptr as NPLTable)[nIndex] = value;
            }
            get
            {
                if (get() == null || GetMyType() != NPLObjectType.NPLObjectType_Table)
                {
                    m_ptr = new NPLTable();
                }
                return (m_ptr as NPLTable).CreateGetField(nIndex);
            }
        }

        private NPLObjectBase m_ptr;
    }

    
    /// <summary>
    /// base class for all NPL date members. Do not use this class directly. Use NPLObjectProxy
    /// </summary>
	public class NPLObjectBase
	{
		public NPLObjectBase() { m_type = NPLObjectType.NPLObjectType_Nil; }
		
		public NPLObjectType GetMyType(){return m_type;}

		protected NPLObjectType m_type;
	}

    /** a floating point number */
	public class NPLNumberObject : NPLObjectBase
	{
        public NPLNumberObject() { m_type = NPLObjectType.NPLObjectType_Number; m_value = 0; }

		/** create the table from a serialized string. */
		public NPLNumberObject(double value) 
        { 
            m_value = value; 
            m_type = NPLObjectType.NPLObjectType_Number;
        }

		public void SetValue(double value){m_value = value;}
		public double GetValue() {return m_value;}

		private double m_value;
	}


	/** a boolean */
	public class NPLBoolObject : NPLObjectBase
	{
        public NPLBoolObject() { m_type = NPLObjectType.NPLObjectType_Bool; m_value = false; }
		/** create the table from a serialized string. */
		public NPLBoolObject(bool value) { m_value = value; m_type = NPLObjectType.NPLObjectType_Bool;}

		public void SetValue(bool value){m_value = value;}
		public bool GetValue() {return m_value;}

	    private	bool m_value;
	}

	/** a floating point number */
	public class NPLStringObject : NPLObjectBase
	{
        public NPLStringObject() { m_type = NPLObjectType.NPLObjectType_String; m_value = null; }
		/** create the table from a serialized string. */
		public NPLStringObject(string value){m_value = value; m_type = NPLObjectType.NPLObjectType_String;}

		public void SetValue(string value){m_value = value;}
		public string GetValue(){return m_value;}

	    private string m_value;
	}

    /// <summary>
    /// Do not use this class directly, use NPLObjectProxy instead.
    /// </summary>
    public class NPLTable : NPLObjectBase, System.Collections.IEnumerable
	{
		/** this is an empty table*/
		public NPLTable()
        {
            m_type = NPLObjectType.NPLObjectType_Table;
            m_fields = new Dictionary<string, NPLObjectProxy>();
            m_index_fields = new Dictionary<int, NPLObjectProxy>();
        }
		
		/** output table to serialized string. */
		public override string ToString(){return null;}

		/** clear the table object */
		public void Clear() 
        {
            m_fields.Clear();
            m_index_fields.Clear();
        }

		/** Set the field.
		* @param sName: the field name
		* @param pObject: if NULL, it will remove the field, as if nil in lua. 
		*/
		public void SetField(string sName, NPLObjectProxy obj)
        {
            if (obj != null)
                m_fields[sName] = obj;
            else
                m_fields.Remove(sName);
        }

        public void SetField(int nIndex, NPLObjectProxy obj)
        {
            if (obj != null)
                m_index_fields[nIndex] = obj;
            else
                m_index_fields.Remove(nIndex);
        }

		/** get a field. It may return null if not found.*/
		public NPLObjectProxy GetField(string sName)
        {
            return (m_fields.ContainsKey(sName)) ? m_fields[sName] : null;
        }

		public NPLObjectProxy GetField(int nIndex)
        {
            return (m_index_fields.ContainsKey(nIndex)) ? m_index_fields[nIndex] : null;
        }

		/** create or get a field. It may return null proxy */
		public NPLObjectProxy CreateGetField(string sName)
        {
            if(m_fields.ContainsKey(sName))
            {
                return  m_fields[sName];
            }
            else
            {
                NPLObjectProxy obj = new NPLObjectProxy();
                m_fields.Add(sName, obj);
                return obj;
            }
        }
		public NPLObjectProxy CreateGetField(int nIndex)
        {
            if(m_index_fields.ContainsKey(nIndex))
            {
                return  m_index_fields[nIndex];
            }
            else
            {
                NPLObjectProxy obj = new NPLObjectProxy();
                m_index_fields.Add(nIndex, obj);
                return obj;
            }
        }

        public System.Collections.IEnumerator GetEnumerator()
        {
            return m_fields.GetEnumerator();
        }
    
		/** this will create get field. */
        public NPLObjectProxy this [string sName]
        {
          set{ m_fields[sName] = value; }
          get{ return CreateGetField(sName); }
        }
        public NPLObjectProxy this [int nIndex]
        {
          set{ m_index_fields[nIndex] = value; }
          get{ return CreateGetField(nIndex); }
        }

	    private Dictionary<string, NPLObjectProxy> m_fields;
        private Dictionary<int, NPLObjectProxy> m_index_fields;
	}
}
#endregion

#region Example Activation & Tests
namespace ParaMono
{
    /// <summary>
    /// This is an example of activation class inside a namespace. A singleton file can thus having many activation class in different namespaces. 
    /// class name must be identical to the file name
    /// </summary>
    public class NPLMonoInterface
    {
        public static void activate(ref int nType, ref IntPtr npl_runtime_state)
        {
            // write message to standard out put
            string msg = NPL.GetCurrentMsg(npl_runtime_state);
            Console.WriteLine("ParaMono.NPLMonoInterface.activate () is called with {0}. \n", msg);
        }
    }
}

/// <summary>
/// This is an example of activation class without a namespace.
/// class name must be identical to the file name
/// </summary>
class NPLMonoInterface
{
    /// <summary>
    /// To test the mono interface, call following from any NPL script.
    /// 
    /// NPL.activate("NPLMonoInterface.dll/NPLMonoInterface.cs", {data="test NPLMonoInterface"});
    /// </summary>
    public static void activate(ref int nType, ref IntPtr npl_runtime_state)
    {
        // example 1: getting input message and write to application log
        string msg = NPL.GetCurrentMsg(npl_runtime_state);
        ParaGlobal.applog("Example 1: " + msg);

        // example 2: activate another file
        NPL.activate(npl_runtime_state, "NPLMonoInterface.dll/ParaMono.NPLMonoInterface.cs", "msg={example=2}");

        // example 3: generate new message using NPLWriter:to generate the string : msg={name=1,2,{"3"="4",},};
        NPLWriter writer = new NPLWriter();
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
        ParaGlobal.applog("Example 3: " + writer.ToString());

        // Example 4: Create NPLTable and serialize to string
        NPLObjectProxy msg1 = new NPLObjectProxy();
        msg1["nid"].Assign(10);
        msg1["name"].Assign("value");
        msg1["tab"]["name1"].Assign("value1");
        StringBuilder output = new StringBuilder();
        NPLHelper.NPLTableToString(null, msg1, output);
        ParaGlobal.applog("Example 4: " + output.ToString());
        
        // Example 5: deserialize NPLTable from string. 
        NPLObjectProxy tabMsg = NPLHelper.StringToNPLTable("{nid=10, u_nid=1, [\"1a\"]=2, name=\"value\", tab={name1=\"value1\"}}");
        NPLHelper.NPLTableToString(null, tabMsg, output);
        ParaGlobal.applog("Example 5: " + output.ToString());
        ParaGlobal.applog(String.Format("Example 5 checking: {0}==10, {1}==value, {2}==value1", (double)tabMsg["nid"], (string)tabMsg["name"], (string)(tabMsg["tab"]["name1"])));

        // Example 6: parse a activation incoming message. 
        tabMsg = NPLHelper.MsgStringToNPLTable(msg);
        NPLHelper.NPLTableToString(null, tabMsg, output);
        ParaGlobal.applog("Example 6: " + output.ToString());

        Console.WriteLine("hello world!");
    }
}
#endregion