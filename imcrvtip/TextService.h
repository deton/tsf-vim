
#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

#include "imcrvtip.h"
#include "ViKeyHandler.h"

class CLangBarItemButton;

class CTextService :
	public ITfTextInputProcessorEx,
	public ITfThreadMgrEventSink,
	public ITfCompartmentEventSink,
	public ITfKeyEventSink
{
public:
	CTextService();
	~CTextService();
	
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfTextInputProcessor
	STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid);
	STDMETHODIMP Deactivate();

	// ITfTextInputProcessorEx
	STDMETHODIMP ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags);

	// ITfThreadMgrEventSink
	STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim);
	STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim);
	STDMETHODIMP OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus);
	STDMETHODIMP OnPushContext(ITfContext *pic);
	STDMETHODIMP OnPopContext(ITfContext *pic);

	// ItfCompartmentEventSink
	STDMETHODIMP OnChange(REFGUID rguid);

	// ITfKeyEventSink
	STDMETHODIMP OnSetFocus(BOOL fForeground);
	STDMETHODIMP OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten);

	// ITfCompositionSink
	STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

	ITfThreadMgr *_GetThreadMgr()
	{
		return _pThreadMgr;
	}
	TfClientId _GetClientId()
	{
		return _ClientId;
	}

	// Compartment
	HRESULT _SetCompartment(REFGUID rguid, const VARIANT *pvar);
	BOOL _IsKeyboardDisabled();
	BOOL _IsKeyboardOpen();
	HRESULT _SetKeyboardOpen(BOOL fOpen);

	// LanguageBar
	void _UpdateLanguageBar();
	
	// KeyHandler
	HRESULT _InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE bSf);
	HRESULT _HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf);
	void _KeyboardChanged();
	void _ResetStatus();

	// KeyHandlerConv
	WCHAR _GetCh(BYTE vk, BYTE vkoff = 0);

	// FnConfigure
	void _LoadPreservedKey();
	void _LoadPreservedKeySub(LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[]);

private:
	LONG _cRef;

	BOOL _InitThreadMgrEventSink();
	void _UninitThreadMgrEventSink();

	BOOL _InitCompartmentEventSink();
	void _UninitCompartmentEventSink();

	BOOL _InitKeyEventSink();
	void _UninitKeyEventSink();

	BOOL _InitPreservedKey();
	void _UninitPreservedKey();

	BOOL _InitLanguageBar();
	void _UninitLanguageBar();

	int _IsKeyEaten(ITfContext *pContext, WPARAM wParam, LPARAM lParam, bool isKeyDown, bool isTest);

	ITfThreadMgr *_pThreadMgr;
	TfClientId _ClientId;

	DWORD _dwThreadMgrEventSinkCookie;
	DWORD _dwCompartmentEventSinkCookie;

	CLangBarItemButton *_pLangBarItem;
	CLangBarItemButton *_pLangBarItemI;

public:
	DWORD _dwActiveFlags;	//ITfThreadMgrEx::GetActiveFlags()
	BOOL _ImmersiveMode;
	BOOL _UILessMode;

	//状態
	ViKeyHandler vihandler;			//Viキー処理

	TF_PRESERVEDKEY preservedkeyon[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY preservedkeyoff[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY preservedkeyonoff[MAX_PRESERVEDKEY];
};

#endif //TEXTSERVICE_H
