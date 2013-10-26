
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
		//OnPreservedKey()経由ならひらがなモード
		//OnChange()経由なら前回のモード
		switch(exinputmode)
		{
		case im_default:
			inputmode = im_hiragana;
			break;
		default:
			inputmode = exinputmode;
			break;
		}

		_StartManager();

		_ResetStatus();

		_LoadBehavior();
		_LoadSelKey();

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_LoadKeyMap(SectionKeyMap, ckeymap);
		_LoadKeyMap(SectionVKeyMap, vkeymap);
		_LoadConvPoint();
		_LoadKana();
		_LoadJLatin();
	}
	else
	{
		exinputmode = inputmode;
		inputmode = im_default;

		if(exinputmode != im_default)
		{
			_SaveUserDic();
		}

		_UninitPreservedKey();
		_LoadPreservedKey();
		_InitPreservedKey();

		_ResetStatus();

		postbuf.clear();
	}

	_UpdateLanguageBar();
}

BOOL CTextService::_IsKeyVoid(WCHAR ch, BYTE vk)
{
	if(ch < KEYMAPNUM)
	{
		if(ckeymap.keyvoid[ch] == SKK_VOID)
		{
			return TRUE;
		}
	}
	if(vk < KEYMAPNUM)
	{
		if(vkeymap.keyvoid[vk] == SKK_VOID)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void CTextService::_ResetStatus()
{
	inputkey = FALSE;
	abbrevmode = FALSE;
	showentry = FALSE;
	showcandlist = FALSE;
	complement = FALSE;

	searchkey.clear();
	searchkeyorg.clear();

	candidates.clear();
	candidates.shrink_to_fit();
	candidx = 0;

	roman.clear();
	kana.clear();
	accompidx = 0;

	cursoridx = 0;
}
