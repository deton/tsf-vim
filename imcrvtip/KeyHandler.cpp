
#include "imcrvtip.h"
#include "EditSession.h"
#include "TextService.h"
#include "LanguageBar.h"

class CKeyHandlerEditSession : public CEditSessionBase
{
public:
	CKeyHandlerEditSession(CTextService *pTextService, ITfContext *pContext, WPARAM wParam, BYTE bSf) : CEditSessionBase(pTextService, pContext)
	{
		_wParam = wParam;
		_bSf = bSf;
	}

	// ITfEditSession
	STDMETHODIMP DoEditSession(TfEditCookie ec)
	{
#ifdef _DEBUG
		_pTextService->_HandleKey(ec, _pContext, _wParam, _bSf);
#else
		__try
		{
			_pTextService->_HandleKey(ec, _pContext, _wParam, _bSf);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			_pTextService->_ResetStatus();
		}

#endif
		return S_OK;
	}

private:
	WPARAM _wParam;
	BYTE _bSf;
};

HRESULT CTextService::_InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE bSf)
{
	CKeyHandlerEditSession *pEditSession;
	HRESULT hr = E_FAIL;

	pEditSession = new CKeyHandlerEditSession(this, pContext, wParam, bSf);
	if(pEditSession != NULL)
	{
		hr = pContext->RequestEditSession(_ClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
		pEditSession->Release();
	}

	return hr;
}

HRESULT CTextService::_HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf)
{
	WCHAR ch;
	
	ch = _GetCh((BYTE)wParam);
	if(ch != L'\0')
	{
		vihandler.HandleKey(ec, pContext, ch, (BYTE)wParam);
	}

	return S_OK;
}

void CTextService::_KeyboardOpenCloseChanged()
{
	if(_pThreadMgr == NULL)
	{
		return;
	}

	_dwActiveFlags = 0;

	ITfThreadMgrEx *pThreadMgrEx;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pThreadMgrEx)) == S_OK)
	{
		pThreadMgrEx->GetActiveFlags(&_dwActiveFlags);
		pThreadMgrEx->Release();
	}

	BOOL fOpen = _IsKeyboardOpen();
	if(fOpen)
	{
		_ResetStatus();

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();
	}
	else
	{
		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_ResetStatus();
	}

	_UpdateLanguageBar();
}

void CTextService::_KeyboardInputConversionChanged()
{
	VARIANT var;

	if(_GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var) != S_OK)
	{
		var.vt = VT_I4;
		var.lVal = TF_CONVERSIONMODE_NATIVE | TF_CONVERSIONMODE_FULLSHAPE;
		_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &var);
	}
}

void CTextService::_ResetStatus()
{
	vihandler.Reset();
}
