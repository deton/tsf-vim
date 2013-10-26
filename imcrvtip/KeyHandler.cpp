
#include "configxml.h"
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
		vihandler.HandleKey(ec, pContext, ch);
	}

	return S_OK;
}

void CTextService::_KeyboardChanged()
{
	if(_pThreadMgr == NULL)
	{
		return;
	}

	_dwActiveFlags = 0;
	_ImmersiveMode = FALSE;
	_UILessMode = FALSE;

	ITfThreadMgrEx *pThreadMgrEx;
	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pThreadMgrEx)) == S_OK)
	{
		pThreadMgrEx->GetActiveFlags(&_dwActiveFlags);
		pThreadMgrEx->Release();
	}

	if((_dwActiveFlags & TF_TMF_IMMERSIVEMODE) != 0)
	{
		_ImmersiveMode = TRUE;
	}

	if((_dwActiveFlags & TF_TMF_UIELEMENTENABLEDONLY) != 0)
	{
		_UILessMode = TRUE;
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

void CTextService::_ResetStatus()
{
	vihandler.Reset();
}
