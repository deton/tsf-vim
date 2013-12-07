
#include "imcrvtip.h"
#include "TextService.h"
#include "LanguageBar.h"
#include "mozc/win32/base/keyboard.h"

static LPCWSTR c_PreservedKeyNormalDesc = L"Normal";
static LPCWSTR c_PreservedKeyOtherImeDesc = L"OtherIme";

int CTextService::_IsKeyEaten(ITfContext *pContext, WPARAM wParam, LPARAM lParam, bool isKeyDown, bool isTest)
{
	if(_IsKeyboardDisabled())
	{
		return FALSE;
	}

	if(!_IsKeyboardOpen())
	{
		return FALSE;
	}

	if(vihandler.IsThroughSelfSentKey())
	{
		return FALSE;
	}

	if(vihandler.IsWaitingNextKey())
	{
		return TRUE;
	}

	SHORT vk_ctrl = GetKeyState(VK_CONTROL) & 0x8000;

	WCHAR ch = _GetCh((BYTE)wParam);

	//TODO: 処理するキーを一か所で管理
	if(ch == CTRL('F') || ch == CTRL('B') || ch == CTRL('M'))
	{
		return TRUE;
	}

	//処理しないCtrlキー
	if(vk_ctrl)
	{
		vihandler.Reset();
		return FALSE;
	}

	if(ch >= L'\x20')
	{
		return TRUE;
	}

	vihandler.Reset();
	return FALSE;
}

STDAPI CTextService::OnSetFocus(BOOL fForeground)
{
	return S_OK;
}

STDAPI CTextService::OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	int eaten = _IsKeyEaten(pic, wParam, lParam, TRUE, TRUE);
	if(eaten == -1)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	*pfEaten = (eaten == TRUE);
	return S_OK;
}

STDAPI CTextService::OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	int eaten = _IsKeyEaten(pic, wParam, lParam, TRUE, FALSE);
	if(eaten == -1)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	*pfEaten = (eaten == TRUE);

	if(*pfEaten)
	{
		_InvokeKeyHandler(pic, wParam, lParam, 0);
	}
	return S_OK;
}

STDAPI CTextService::OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	int eaten = _IsKeyEaten(pic, wParam, lParam, FALSE, TRUE);
	if(eaten == -1)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	*pfEaten = (eaten == TRUE);
	return S_OK;
}

STDAPI CTextService::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	int eaten = _IsKeyEaten(pic, wParam, lParam, FALSE, FALSE);
	if(eaten == -1)
	{
		*pfEaten = TRUE;
		return S_OK;
	}
	*pfEaten = (eaten == TRUE);
	return S_OK;
}

STDAPI CTextService::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
	if(IsEqualGUID(rguid, c_guidPreservedKeyNormal))
	{
		BOOL fOpen = _IsKeyboardOpen();
		if(fOpen)
		{
			vihandler.Reset();
		}
		vihandler.ResetThroughSelfSentKey();
		_SetKeyboardOpen(TRUE);
		*pfEaten = TRUE;
	}
	else if(IsEqualGUID(rguid, c_guidPreservedKeyOtherIme))
	{
		vihandler.SwitchToOtherIme(c_otherime1, c_otherime2);
		_SetKeyboardOpen(FALSE);
		*pfEaten = TRUE;
	}
	else
	{
		*pfEaten = FALSE;
	}

	return S_OK;
}

BOOL CTextService::_InitKeyEventSink()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		hr = pKeystrokeMgr->AdviseKeyEventSink(_ClientId, (ITfKeyEventSink *)this, TRUE);
		pKeystrokeMgr->Release();
	}

	return (hr == S_OK);
}

void CTextService::_UninitKeyEventSink()
{
	ITfKeystrokeMgr *pKeystrokeMgr;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		pKeystrokeMgr->UnadviseKeyEventSink(_ClientId);
		pKeystrokeMgr->Release();
	}
}

#define _PRESERVE_KEY(preservedkey, c_guidPreservedKey, c_PreservedKeyDesc) \
do \
{ \
	for(i=0; i<MAX_PRESERVEDKEY; i++) \
	{ \
		if(preservedkey[i].uVKey == 0 && preservedkey[i].uModifiers == 0) \
		{ \
			break; \
		} \
		hr = pKeystrokeMgr->PreserveKey(_ClientId, c_guidPreservedKey, \
			&preservedkey[i], c_PreservedKeyDesc, (ULONG)wcslen(c_PreservedKeyDesc)); \
	} \
} while(0)

BOOL CTextService::_InitPreservedKey()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	HRESULT hr = E_FAIL;
	int i;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		_PRESERVE_KEY(preservedkeynormal, c_guidPreservedKeyNormal, c_PreservedKeyNormalDesc);
		_PRESERVE_KEY(preservedkeyotherime, c_guidPreservedKeyOtherIme, c_PreservedKeyOtherImeDesc);

		pKeystrokeMgr->Release();
	}

	return (hr == S_OK);
}

#define _UNPRESERVE_KEY(preservedkey, c_guidPreservedKey) \
do \
{ \
	for(i=0; i<MAX_PRESERVEDKEY; i++) \
	{ \
		if(preservedkey[i].uVKey == 0 && preservedkey[i].uModifiers == 0) \
		{ \
			break; \
		} \
		pKeystrokeMgr->UnpreserveKey(c_guidPreservedKey, &preservedkey[i]); \
	} \
} while(0)

void CTextService::_UninitPreservedKey()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	int i;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		_UNPRESERVE_KEY(preservedkeynormal, c_guidPreservedKeyNormal);
		_UNPRESERVE_KEY(preservedkeyotherime, c_guidPreservedKeyOtherIme);

		pKeystrokeMgr->Release();
	}
}
