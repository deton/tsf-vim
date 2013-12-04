
#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

#include "imcrvtip.h"
#include "ViKeyHandler.h"

class CLangBarItemButton;

class CTextService :
	public ITfTextInputProcessorEx,
	public ITfThreadMgrEventSink,
	public ITfCompartmentEventSink,
	public ITfKeyEventSink,
	public ITfFunctionProvider,
	public ITfFnConfigure,
	public ITfFnShowHelp
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

	// ITfFunctionProvider
	STDMETHODIMP GetType(GUID *pguid);
	STDMETHODIMP GetDescription(BSTR *pbstrDesc);
	STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

	// ITfFunction
	STDMETHODIMP GetDisplayName(BSTR *pbstrName);

	// ITfFnConfigure
	STDMETHODIMP Show(HWND hwndParent, LANGID langid, REFGUID rguidProfile);

	// ITfFnShowHelp
	STDMETHODIMP Show(HWND hwndParent);

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
	HRESULT _GetCompartment(REFGUID rguid, VARIANT *pvar);
	BOOL _IsKeyboardDisabled();
	BOOL _IsKeyboardOpen();
	HRESULT _SetKeyboardOpen(BOOL fOpen);

	// LanguageBar
	void _UpdateLanguageBar();
	
	// KeyHandler
	HRESULT _InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE bSf);
	HRESULT _HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf);
	void _KeyboardOpenCloseChanged();
	void _KeyboardInputConversionChanged();
	void _ResetStatus();

	// KeyHandlerConv
	WCHAR _GetCh(BYTE vk, BYTE vkoff = 0);

	// KeyHandlerDictionary
	void _StartConfigure();
	void _StartProcess(LPCWSTR fname);

	// FnConfigure
	void _CreateConfigPath();
	void _LoadPreservedKey();
	void _LoadPreservedKeySub(LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[], const TF_PRESERVEDKEY configpreservedkey[]);

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

	BOOL _InitFunctionProvider();
	void _UninitFunctionProvider();

	int _IsKeyEaten(ITfContext *pContext, WPARAM wParam, LPARAM lParam, bool isKeyDown, bool isTest);

	ITfThreadMgr *_pThreadMgr;
	TfClientId _ClientId;

	DWORD _dwThreadMgrEventSinkCookie;
	DWORD _dwCompartmentEventSinkOpenCloseCookie;
	DWORD _dwCompartmentEventSinkInputmodeConversionCookie;

	CLangBarItemButton *_pLangBarItem;
	CLangBarItemButton *_pLangBarItemI;

private:
	//ファイルパス
	WCHAR pathconfigxml[MAX_PATH];	//設定

	//ミューテックス
	WCHAR cnfmutexname[MAX_KRNLOBJNAME];

public:
	//状態
	ViKeyHandler vihandler;			//Viキー処理

	//preserved key
	TF_PRESERVEDKEY preservedkeynormal[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY preservedkeyotherime[MAX_PRESERVEDKEY];
};

#endif //TEXTSERVICE_H
