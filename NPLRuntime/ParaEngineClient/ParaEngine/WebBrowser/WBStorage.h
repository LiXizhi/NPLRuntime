

#pragma  once

#include <ObjIdl.h>

namespace ParaEngine
{
	class WBStorage:public IStorage
	{
	public:
		// IUnknown methods
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);

		virtual ULONG STDMETHODCALLTYPE AddRef( void);

		virtual ULONG STDMETHODCALLTYPE Release( void);

		// IStorage methods
		virtual HRESULT STDMETHODCALLTYPE CreateStream( 
			/* [string][in] */ const OLECHAR *pwcsName,
			/* [in] */ DWORD grfMode,
			/* [in] */ DWORD reserved1,
			/* [in] */ DWORD reserved2,
			/* [out] */ IStream **ppstm);

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE OpenStream( 
			/* [string][in] */ const OLECHAR *pwcsName,
			/* [unique][in] */ void *reserved1,
			/* [in] */ DWORD grfMode,
			/* [in] */ DWORD reserved2,
			/* [out] */ IStream **ppstm);

		virtual HRESULT STDMETHODCALLTYPE CreateStorage( 
			/* [string][in] */ const OLECHAR *pwcsName,
			/* [in] */ DWORD grfMode,
			/* [in] */ DWORD reserved1,
			/* [in] */ DWORD reserved2,
			/* [out] */ IStorage **ppstg);

		virtual HRESULT STDMETHODCALLTYPE OpenStorage( 
			/* [string][unique][in] */ const OLECHAR *pwcsName,
			/* [unique][in] */ IStorage *pstgPriority,
			/* [in] */ DWORD grfMode,
			/* [unique][in] */ SNB snbExclude,
			/* [in] */ DWORD reserved,
			/* [out] */ IStorage **ppstg);

		virtual HRESULT STDMETHODCALLTYPE CopyTo( 
			/* [in] */ DWORD ciidExclude,
			/* [size_is][unique][in] */ const IID *rgiidExclude,
			/* [unique][in] */ SNB snbExclude,
			/* [unique][in] */ IStorage *pstgDest);

		virtual HRESULT STDMETHODCALLTYPE MoveElementTo( 
			/* [string][in] */ const OLECHAR *pwcsName,
			/* [unique][in] */ IStorage *pstgDest,
			/* [string][in] */ const OLECHAR *pwcsNewName,
			/* [in] */ DWORD grfFlags);

		virtual HRESULT STDMETHODCALLTYPE Commit( 
			/* [in] */ DWORD grfCommitFlags);

		virtual HRESULT STDMETHODCALLTYPE Revert( void);

		virtual /* [local] */ HRESULT STDMETHODCALLTYPE EnumElements( 
			/* [in] */ DWORD reserved1,
			/* [size_is][unique][in] */ void *reserved2,
			/* [in] */ DWORD reserved3,
			/* [out] */ IEnumSTATSTG **ppenum);

		virtual HRESULT STDMETHODCALLTYPE DestroyElement( 
			/* [string][in] */ const OLECHAR *pwcsName);

		virtual HRESULT STDMETHODCALLTYPE RenameElement( 
			/* [string][in] */ const OLECHAR *pwcsOldName,
			/* [string][in] */ const OLECHAR *pwcsNewName);

		virtual HRESULT STDMETHODCALLTYPE SetElementTimes( 
			/* [string][unique][in] */ const OLECHAR *pwcsName,
			/* [unique][in] */ const FILETIME *pctime,
			/* [unique][in] */ const FILETIME *patime,
			/* [unique][in] */ const FILETIME *pmtime);

		virtual HRESULT STDMETHODCALLTYPE SetClass( 
			/* [in] */ REFCLSID clsid);

		virtual HRESULT STDMETHODCALLTYPE SetStateBits( 
			/* [in] */ DWORD grfStateBits,
			/* [in] */ DWORD grfMask);

		virtual HRESULT STDMETHODCALLTYPE Stat( 
			/* [out] */ STATSTG *pstatstg,
			/* [in] */ DWORD grfStatFlag);

	};
}