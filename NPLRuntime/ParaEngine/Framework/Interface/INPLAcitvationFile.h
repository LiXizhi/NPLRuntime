#pragma once

namespace NPL
{
	class INPLRuntimeState;

	/* c++ base activation file that is callable from scripting interface or NPL.
	* @sa NPL_C_Func_ActivationFile and CNPLFile_Stats for example usage:
	*
	* Usage One: we can explicitly register a function by deriving from INPLActivationFile.
	* Usage Two: or we can register automatically by defining static function with special name: NPL_activate_XXXX_cpp, such as :
	*
	* extern "C"{
	*	NPL::NPLReturnCode NPL_activate_script_helloworld_cpp(INPLRuntimeState* pState);
	* }
	* NPL.activate("script/helloworld.cpp", {}) or NPL.activate("script_helloworld.cpp", {}) will both activate it.
	*/
	class INPLActivationFile
	{
	public:
		INPLActivationFile() : m_refcount(0) {};
		virtual ~INPLActivationFile(){};

		/** Function to be called when NPL.activate(filename, {msg});
		* subclass should always overwrite this function.
		* usage: the input message in secure code format. Read data as follows:
		* auto msg = NPLHelper::MsgStringToNPLTable(pState->GetCurrentMsg(), pState->GetCurrentMsgLength());
		* std::string sType = msg["type"];
		*/
		virtual NPL::NPLReturnCode OnActivate(INPLRuntimeState* pState) = 0;

	public:
		/** add reference count of the object. */
		void addref()const
		{
			++m_refcount;
		}

		/** decrease reference count of the object.
		* @return : return true if the the reference count is zero after decreasing it*/
		bool delref()const
		{
			return --m_refcount <= 0;
		}
		/** get the reference count */
		int GetRefCount()const
		{
			return m_refcount;
		}

		//all overridden functions should call this function in order to use the "delete when 
		//reference is zero" scheme
		virtual int Release()
		{
			if (delref()) {
				int nRefCount = GetRefCount();
				delete this;
				return nRefCount;
			}
			return GetRefCount();
		}

	protected:
		mutable int m_refcount;
	};
}