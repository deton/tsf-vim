﻿
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "mozc/win32/base/keyboard.h"

CTextService::CTextService()
	: keyboard_(mozc::win32::Win32KeyboardInterface::CreateDefault())
{
	DllAddRef();

	_cRef = 1;

	_pThreadMgr = NULL;
	_dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
	_dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
	_dwCompartmentEventSinkCookie = TF_INVALID_COOKIE;
	_pTextEditSinkContext = NULL;
	_dwTextEditSinkCookie = TF_INVALID_COOKIE;
	_pComposition = NULL;
	_pCandidateList = NULL;

	_dwActiveFlags = 0;

	hPipe = INVALID_HANDLE_VALUE;

	inputmode = im_default;
	exinputmode = im_default;

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
	else if(IsEqualIID(riid, IID_ITfThreadFocusSink))
	{
		*ppvObj = (ITfThreadFocusSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfCompartmentEventSink))
	{
		*ppvObj = (ITfCompartmentEventSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfTextEditSink))
	{
		*ppvObj = (ITfTextEditSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfKeyEventSink))
	{
		*ppvObj = (ITfKeyEventSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfCompositionSink))
	{
		*ppvObj = (ITfKeyEventSink *)this;
	}
	else if(IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
	{
		*ppvObj = (ITfDisplayAttributeProvider *)this;
	}
	else if(IsEqualIID(riid, IID_ITfFunctionProvider))
	{
		*ppvObj = (ITfFunctionProvider *)this;
	}
	else if(IsEqualIID(riid, IID_ITfFnConfigure))
	{
		*ppvObj = (ITfFnConfigure *)this;
	}
	else if(IsEqualIID(riid, IID_ITfFnShowHelp))
	{
		*ppvObj = (ITfFnShowHelp *)this;
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
	
	_CreateConfigPath();

	_pThreadMgr = ptim;
	_pThreadMgr->AddRef();
	_ClientId = tid;
	
	if(!_InitThreadMgrEventSink())
	{
		goto exit;
	}

	if(!_InitThreadFocusSink())
	{
		goto exit;
	}

	if(!_InitCompartmentEventSink())
	{
		goto exit;
	}

	ITfDocumentMgr *pDocumentMgrFocus;
	if((_pThreadMgr->GetFocus(&pDocumentMgrFocus) == S_OK) && (pDocumentMgrFocus != NULL))
	{
		_InitTextEditSink(pDocumentMgrFocus);
		pDocumentMgrFocus->Release();
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

	if(!_InitDisplayAttributeGuidAtom())
	{
		goto exit;
	}
	
	if(!_InitFunctionProvider())
	{
		goto exit;
	}

	_KeyboardChanged();

	return S_OK;

exit:
	Deactivate();
	return E_FAIL;
}

STDAPI CTextService::Deactivate()
{
	_SaveUserDic();

	if(_pCandidateList != NULL)
	{
		delete _pCandidateList;
		_pCandidateList = NULL;
	}

	_UninitFunctionProvider();

	_UninitPreservedKey();

	_UninitKeyEventSink();

	_UninitLanguageBar();

	_InitTextEditSink(NULL);

	_UninitCompartmentEventSink();

	_UninitThreadFocusSink();

	_UninitThreadMgrEventSink();

	if(_pThreadMgr != NULL)
	{
		_pThreadMgr->Release();
		_pThreadMgr = NULL;
	}

	_ClientId = TF_CLIENTID_NULL;

	return S_OK;
}
