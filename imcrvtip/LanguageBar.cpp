
#include "imcrvtip.h"
#include "TextService.h"
#include "EditSession.h"
#include "LanguageBar.h"
#include "resource.h"

#define TEXTSERVICE_LANGBARITEMSINK_COOKIE 0x54ab516b

// langbar menu items
static const struct {
	UINT id;
	DWORD flag;
	LPCWSTR text;
} menuItems[] = {
	{IDM_NORMAL,		0, L"［－－］"},
	{IDM_INSERT,		0, L"［INS］"},
	{IDM_NONE,			TF_LBMENUF_SEPARATOR, L""},
	{IDM_CONFIG,		0, L"設定"},
	{IDM_NONE,			TF_LBMENUF_SEPARATOR, L""},
	{IDM_NONE,			0, L"キャンセル"}
};

// monochrome icons
static const WORD iconIDX[] =
{
	IDI_X_INSERT, IDI_X_NORMAL
};
// png icons
static const WORD iconIDZ[] =
{
	IDI_Z_INSERT, IDI_Z_NORMAL
};

CLangBarItemButton::CLangBarItemButton(CTextService *pTextService, REFGUID guid)
{
	DllAddRef();

	_LangBarItemInfo.clsidService = c_clsidTextService;
	_LangBarItemInfo.guidItem = guid;
	//TF_LBI_STYLE_TEXTCOLORICON
	// Any black pixel within the icon will be converted to the text color of the selected theme.
	// The icon must be monochrome.
	_LangBarItemInfo.dwStyle = TF_LBI_STYLE_SHOWNINTRAY |
		(IsEqualGUID(_LangBarItemInfo.guidItem, GUID_LBI_INPUTMODE) ? TF_LBI_STYLE_BTN_BUTTON : TF_LBI_STYLE_BTN_MENU) |
		(IsVersion62AndOver() ? 0 : TF_LBI_STYLE_TEXTCOLORICON);	//monochrome icon used under Windows 8
	_LangBarItemInfo.ulSort = 1;
	wcsncpy_s(_LangBarItemInfo.szDescription, LangbarItemDesc, _TRUNCATE);

	_pLangBarItemSink = NULL;

	_pTextService = pTextService;
	_pTextService->AddRef();

	_cRef = 1;
}

CLangBarItemButton::~CLangBarItemButton()
{
	DllRelease();

	_pTextService->Release();
}

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObj)
{
	if(ppvObj == NULL)
	{
		return E_INVALIDARG;
	}

	*ppvObj = NULL;

	if(IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITfLangBarItem) ||
		IsEqualIID(riid, IID_ITfLangBarItemButton))
	{
		*ppvObj = (ITfLangBarItemButton *)this;
	}
	else if(IsEqualIID(riid, IID_ITfSource))
	{
		*ppvObj = (ITfSource *)this;
	}

	if(*ppvObj)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
	return ++_cRef;
}

STDAPI_(ULONG) CLangBarItemButton::Release()
{
	if(--_cRef == 0)
	{
		delete this;
		return 0;
	}

	return _cRef;
}

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
	if(pInfo == NULL)
	{
		return E_INVALIDARG;
	}

	*pInfo = _LangBarItemInfo;

	return S_OK;
}

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
	if(pdwStatus == NULL)
	{
		return E_INVALIDARG;
	}

	if(_pTextService->_IsKeyboardDisabled())
	{
		*pdwStatus = TF_LBI_STATUS_DISABLED;
	}
	else
	{
		*pdwStatus = 0;
	}

	return S_OK;
}

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
	if(_pLangBarItemSink == NULL)
	{
		return E_FAIL;
	}

	return _pLangBarItemSink->OnUpdate(TF_LBI_STATUS);
}

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
	BSTR bstrToolTip;

	if(pbstrToolTip == NULL)
	{
		return E_INVALIDARG;
	}

	*pbstrToolTip = NULL;

	bstrToolTip = SysAllocString(LangbarItemDesc);

	if(bstrToolTip == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*pbstrToolTip = bstrToolTip;

	return S_OK;
}

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
	if(IsEqualGUID(_LangBarItemInfo.guidItem, GUID_LBI_INPUTMODE))
	{
		switch(click)
		{
		case TF_LBI_CLK_RIGHT:
			{
				HMENU hMenu = LoadMenuW(g_hInst, MAKEINTRESOURCE(IDR_SYSTRAY_MENU));
				if(hMenu)
				{
					UINT check = IDM_NORMAL;
					BOOL fOpen = _pTextService->_IsKeyboardOpen();
					if(!fOpen)
					{
						check = IDM_INSERT;
					}
					CheckMenuRadioItem(hMenu, IDM_NORMAL, IDM_INSERT, check, MF_BYCOMMAND);
					HMENU hSubMenu = GetSubMenu(hMenu, 0);
					if(hSubMenu)
					{
						TPMPARAMS tpm;
						TPMPARAMS *ptpm = NULL;
						if(prcArea != NULL)
						{
							tpm.cbSize = sizeof(tpm);
							tpm.rcExclude = *prcArea;
							ptpm = &tpm;
						}
						BOOL bRet = TrackPopupMenuEx(hSubMenu,
							TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_VERTICAL,
							pt.x, pt.y, GetFocus(), ptpm);
						this->OnMenuSelect(bRet);
					}
					DestroyMenu(hMenu);
				}
			}
			break;
		case TF_LBI_CLK_LEFT:
			{
				BOOL fOpen = _pTextService->_IsKeyboardOpen();
				_pTextService->_SetKeyboardOpen(fOpen ? FALSE : TRUE);
			}
			break;
		default:
			break;
		}
	}

	return S_OK;
}

STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
	if(pMenu == NULL)
	{
		return E_INVALIDARG;
	}

	BOOL fOpen = _pTextService->_IsKeyboardOpen();
#define ADDMENUITEM(checked) \
	pMenu->AddMenuItem(menuItems[i].id, menuItems[i].flag | \
		((checked) ? TF_LBMENUF_RADIOCHECKED : 0), \
		NULL, NULL, menuItems[i].text, (ULONG)wcslen(menuItems[i].text), NULL)

	for(int i = 0; i < _countof(menuItems); i++)
	{
		if(menuItems[i].id == IDM_NORMAL)
		{
			ADDMENUITEM(fOpen);
		}
		else if(menuItems[i].id == IDM_INSERT)
		{
			ADDMENUITEM(!fOpen);
		}
		else
		{
			ADDMENUITEM(FALSE);
		}
	}
#undef ADDMENUITEM

	return S_OK;
}

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
	BOOL fOpen = _pTextService->_IsKeyboardOpen();
	switch(wID)
	{
	case IDM_CONFIG:
		_pTextService->_StartConfigure();
		break;
	case IDM_NORMAL:
		if(!fOpen)
		{
			_pTextService->_SetKeyboardOpen(TRUE);
		}
		break;
	case IDM_INSERT:
		if(fOpen)
		{
			_pTextService->_SetKeyboardOpen(FALSE);
		}
		break;
	default:
		break;
	}

	return S_OK;
}

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
	size_t iconindex = 0;
	WORD iconid = 0;

	if(!_pTextService->_IsKeyboardDisabled() && _pTextService->_IsKeyboardOpen())
	{
		iconindex = 1;
	}

	if(IsVersion62AndOver())
	{
		if(iconindex < _countof(iconIDZ))
		{
			iconid = iconIDZ[iconindex];
		}
	}
	else
	{
		if(iconindex < _countof(iconIDX))
		{
			iconid = iconIDX[iconindex];
		}
	}

	//DPIを考慮
	HDC hdc = GetDC(NULL);
	int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(NULL, hdc);
	int size = MulDiv(16, dpiX, 96);

	*phIcon = (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(iconid), IMAGE_ICON, size, size, LR_SHARED);

	return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
	BSTR bstrText;

	if(pbstrText == NULL)
	{
		return E_INVALIDARG;
	}

	*pbstrText = NULL;

	bstrText = SysAllocString(LangbarItemDesc);

	if(bstrText == NULL)
	{
		return E_OUTOFMEMORY;
	}

	*pbstrText = bstrText;

	return S_OK;
}

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
	if(!IsEqualIID(IID_ITfLangBarItemSink, riid))
	{
		return CONNECT_E_CANNOTCONNECT;
	}

	if(_pLangBarItemSink != NULL)
	{
		return CONNECT_E_ADVISELIMIT;
	}

	if(punk->QueryInterface(IID_PPV_ARGS(&_pLangBarItemSink)) != S_OK)
	{
		_pLangBarItemSink = NULL;
		return E_NOINTERFACE;
	}

	*pdwCookie = TEXTSERVICE_LANGBARITEMSINK_COOKIE;

	return S_OK;
}

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
	if(dwCookie != TEXTSERVICE_LANGBARITEMSINK_COOKIE)
	{
		return CONNECT_E_NOCONNECTION;
	}

	if(_pLangBarItemSink == NULL)
	{
		return CONNECT_E_NOCONNECTION;
	}

	_pLangBarItemSink->Release();
	_pLangBarItemSink = NULL;

	return S_OK;
}

STDAPI CLangBarItemButton::_Update()
{
	VARIANT var;

	var.vt = VT_I4;

	var.lVal = TF_SENTENCEMODE_PHRASEPREDICT;
	_pTextService->_SetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE, &var);

	if(_pLangBarItemSink == NULL)
	{
		return E_FAIL;
	}

	return _pLangBarItemSink->OnUpdate(TF_LBI_ICON | TF_LBI_STATUS);
}

BOOL CTextService::_InitLanguageBar()
{
	ITfLangBarItemMgr *pLangBarItemMgr;
	BOOL fRet = FALSE;
	BOOL fRetI = FALSE;

	_pLangBarItem = NULL;
	_pLangBarItemI = NULL;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pLangBarItemMgr)) == S_OK)
	{
		_pLangBarItem = new CLangBarItemButton(this, c_guidLangBarItemButton);
		if(_pLangBarItem != NULL)
		{
			if(pLangBarItemMgr->AddItem(_pLangBarItem) == S_OK)
			{
				fRet = TRUE;
			}
			else
			{
				_pLangBarItem->Release();
				_pLangBarItem = NULL;
			}
		}

		if(IsVersion62AndOver())
		{
			_pLangBarItemI = new CLangBarItemButton(this, GUID_LBI_INPUTMODE);
			if(_pLangBarItemI != NULL)
			{
				if(pLangBarItemMgr->AddItem(_pLangBarItemI) == S_OK)
				{
					fRetI = TRUE;
				}
				else
				{
					_pLangBarItemI->Release();
					_pLangBarItemI = NULL;
				}
			}
		}
		else
		{
			fRetI = TRUE;
		}

		pLangBarItemMgr->Release();
	}

	return (fRet && fRetI);
}

void CTextService::_UninitLanguageBar()
{
	ITfLangBarItemMgr *pLangBarItemMgr;

	if(_pLangBarItem != NULL)
	{
		if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pLangBarItemMgr)) == S_OK)
		{
			pLangBarItemMgr->RemoveItem(_pLangBarItem);
			pLangBarItemMgr->Release();
		}
		_pLangBarItem->Release();
		_pLangBarItem = NULL;
	}

	if(_pLangBarItemI != NULL)
	{
		if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pLangBarItemMgr)) == S_OK)
		{
			pLangBarItemMgr->RemoveItem(_pLangBarItemI);
			pLangBarItemMgr->Release();
		}
		_pLangBarItemI->Release();
		_pLangBarItemI = NULL;
	}
}

void CTextService::_UpdateLanguageBar()
{
	if(_pLangBarItem != NULL)
	{
		_pLangBarItem->_Update();
	}
	if(_pLangBarItemI != NULL)
	{
		_pLangBarItemI->_Update();
	}
}
