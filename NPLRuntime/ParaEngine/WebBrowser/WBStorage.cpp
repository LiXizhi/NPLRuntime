#include "ParaEngine.h"
#include "WBStorage.h"
#include "WebBrowser.h"

namespace ParaEngine
{
	////////////////////////////////////// WBStorage functions  /////////////////////////////////////////
	// NOTE: The browser object doesn't use the IStorage functions, so most of these are us just returning
	// E_NOTIMPL so that anyone who *does* call these functions knows nothing is being done here.

	HRESULT STDMETHODCALLTYPE WBStorage::QueryInterface( 
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
	{
		NOTIMPLEMENTED;
	}

	ULONG STDMETHODCALLTYPE WBStorage::AddRef( void)
	{
		return 1;
	}

	ULONG STDMETHODCALLTYPE WBStorage::Release( void)
	{
		return 0;
	}


	HRESULT STDMETHODCALLTYPE WBStorage::CreateStream( 
		/* [string][in] */ const OLECHAR *pwcsName,
		/* [in] */ DWORD grfMode,
		/* [in] */ DWORD reserved1,
		/* [in] */ DWORD reserved2,
		/* [out] */ IStream **ppstm)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::OpenStream( 
		/* [string][in] */ const OLECHAR *pwcsName,
		/* [unique][in] */ void *reserved1,
		/* [in] */ DWORD grfMode,
		/* [in] */ DWORD reserved2,
		/* [out] */ IStream **ppstm)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::CreateStorage( 
		/* [string][in] */ const OLECHAR *pwcsName,
		/* [in] */ DWORD grfMode,
		/* [in] */ DWORD reserved1,
		/* [in] */ DWORD reserved2,
		/* [out] */ IStorage **ppstg)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::OpenStorage( 
		/* [string][unique][in] */ const OLECHAR *pwcsName,
		/* [unique][in] */ IStorage *pstgPriority,
		/* [in] */ DWORD grfMode,
		/* [unique][in] */ SNB snbExclude,
		/* [in] */ DWORD reserved,
		/* [out] */ IStorage **ppstg)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::CopyTo( 
		/* [in] */ DWORD ciidExclude,
		/* [size_is][unique][in] */ const IID *rgiidExclude,
		/* [unique][in] */ SNB snbExclude,
		/* [unique][in] */ IStorage *pstgDest)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::MoveElementTo( 
		/* [string][in] */ const OLECHAR *pwcsName,
		/* [unique][in] */ IStorage *pstgDest,
		/* [string][in] */ const OLECHAR *pwcsNewName,
		/* [in] */ DWORD grfFlags)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::Commit( 
		/* [in] */ DWORD grfCommitFlags)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::Revert( void)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::EnumElements( 
		/* [in] */ DWORD reserved1,
		/* [size_is][unique][in] */ void *reserved2,
		/* [in] */ DWORD reserved3,
		/* [out] */ IEnumSTATSTG **ppenum)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::DestroyElement( 
		/* [string][in] */ const OLECHAR *pwcsName)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::RenameElement( 
		/* [string][in] */ const OLECHAR *pwcsOldName,
		/* [string][in] */ const OLECHAR *pwcsNewName)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::SetElementTimes( 
		/* [string][unique][in] */ const OLECHAR *pwcsName,
		/* [unique][in] */ const FILETIME *pctime,
		/* [unique][in] */ const FILETIME *patime,
		/* [unique][in] */ const FILETIME *pmtime)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::SetClass( 
		/* [in] */ REFCLSID clsid)
	{
		//NOTIMPLEMENTED;
		return(S_OK);
	}

	HRESULT STDMETHODCALLTYPE WBStorage::SetStateBits( 
		/* [in] */ DWORD grfStateBits,
		/* [in] */ DWORD grfMask)
	{
		NOTIMPLEMENTED;
	}

	HRESULT STDMETHODCALLTYPE WBStorage::Stat( 
		/* [out] */ STATSTG *pstatstg,
		/* [in] */ DWORD grfStatFlag)
	{
		NOTIMPLEMENTED;
	}
}