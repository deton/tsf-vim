
#include "imcrvtip.h"
#include "TextService.h"

CTextService::CTextService()
	: vihandler(this)
{
	DllAddRef();

	_cRef = 1;

	_pThreadMgr = NULL;
	_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
	_dwCompartmentEventSinkOpenCloseCookie = TF_INVALID_COOKIE;
	_dwCompartmentEventSinkInputmodeConversionCookie = TF_INVALID_COOKIE;

	_dwActiveFlags = 0;

	_ResetStatus();
}

CTextService::~CTextService()
{
	DllRelease();
}

STDAPI CTextService::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
	{
		*ppvObj = (ITfTextInputProcessor *)this;
	}
	else if(IsEqualIID(riid, IID_ITfTextInputProcessorEx))
	{
		*ppvObj = (ITfTextInputProcessorEx *)this;
	}
	else if(IsEqualIID(riid, IID_ITfThreadMgrEventSink))
	{
		*ppvObj = (ITfThreadMgrEventSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfCompartmentEventSink))
	{
		*ppvObj = (ITfCompartmentEventSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfKeyEventSink))
	{
		*ppvObj = (ITfKeyEventSink *)this;
	}

	if(*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextService::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CTextService::Release()
{
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CTextService::Activate(ITfThreadMgr *ptim, TfClientId tid)
{
	return ActivateEx(ptim, tid, 0);
}

STDAPI CTextService::ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags)
{
	//_wsetlocale(LC_ALL, L"JPN");
	
	_pThreadMgr = ptim;
	_pThreadMgr->AddRef();
	_ClientId = tid;
	
	if(!_InitThreadMgrEventSink())
	{
		goto exit;
	}

	if(!_InitCompartmentEventSink())
	{
		goto exit;
	}

	if(!_InitLanguageBar())
	{
		goto exit;
	}

	if(!_InitKeyEventSink())
	{
		goto exit;
	}

	_LoadPreservedKey();

	if(!_InitPreservedKey())
	{
		goto exit;
	}

	_KeyboardOpenCloseChanged();

	return S_OK;

exit:
	Deactivate();
	return E_FAIL;
}

STDAPI CTextService::Deactivate()
{
	_UninitPreservedKey();

	_UninitKeyEventSink();

	_UninitLanguageBar();

	_UninitCompartmentEventSink();

	_UninitThreadMgrEventSink();

	if(_pThreadMgr != NULL)
	{
		_pThreadMgr->Release();
		_pThreadMgr = NULL;
	}

	_ClientId = TF_CLIENTID_NULL;

	return S_OK;
}
