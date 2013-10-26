
#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

#include "imcrvtip.h"
#include "convtype.h"
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
	BOOL _IsKeyVoid(WCHAR ch, BYTE vk);
	void _ResetStatus();

	// KeyHandlerConv
	WCHAR _GetCh(BYTE vk, BYTE vkoff = 0);
	BYTE _GetSf(BYTE vk, WCHAR ch);

	// FnConfigure
	void _CreateConfigPath();
	void _ReadBoolValue(LPCWSTR key, BOOL &value);
	void _LoadBehavior();
	void _LoadSelKey();
	void _LoadPreservedKey();
	void _LoadPreservedKeySub(LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[]);
	void _LoadKeyMap(LPCWSTR section, KEYMAP &keymap);
	void _LoadConvPoint();
	void _LoadKana();
	void _LoadJLatin();

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

private:
	//ファイルパス
	WCHAR pathconfigxml[MAX_PATH];	//設定

	//imcrvmgr.exe との名前付きパイプ
	WCHAR mgrpipename[MAX_KRNLOBJNAME];
	HANDLE hPipe;
	//ミューテックス
	WCHAR mgrmutexname[MAX_KRNLOBJNAME];
	WCHAR cnfmutexname[MAX_KRNLOBJNAME];

	//キーマップ
	KEYMAP ckeymap;
	KEYMAP vkeymap;

	//変換位置指定（開始,代替,送り）
	WCHAR conv_point[CONV_POINT_NUM][3];

	//変換テーブル
	std::vector<ROMAN_KANA_CONV> roman_kana_conv;
	ASCII_JLATIN_CONV ascii_jlatin_conv[ASCII_JLATIN_TBL_NUM];

public:
	DWORD _dwActiveFlags;	//ITfThreadMgrEx::GetActiveFlags()
	BOOL _ImmersiveMode;
	BOOL _UILessMode;

	//状態
	int inputmode;			//入力モード (無し/ひらがな/カタカナ/半角ｶﾀｶﾅ/全英/アスキー)
	BOOL inputkey;			//見出し入力▽モード
	BOOL abbrevmode;		//abbrevモード
	BOOL showentry;			//候補表示▼モード
	BOOL showcandlist;		//候補リスト表示
	BOOL complement;		//補完

	ViKeyHandler vihandler;			//Viキー処理

	int exinputmode;		//入力モードの前回状態

	//動作設定
	WCHAR fontname[LF_FACESIZE];	//候補一覧のフォント設定
	int fontpoint;					//候補一覧のフォント設定
	int fontweight;					//候補一覧のフォント設定
	BOOL fontitalic;				//候補一覧のフォント設定
	LONG maxwidth;			//候補一覧の最大幅
	COLORREF colors[8];		//候補一覧の色
	size_t c_untilcandlist;	//候補一覧表示に要する変換回数(0:表示なし/1:1回目)
	BOOL c_dispcandnum;		//候補一覧表示なしのとき候補数を表示する
	BOOL c_annotation;		//注釈を表示する
	BOOL c_annotatlst;		//（候補一覧のみ）
	BOOL c_nomodemark;		//▽▼*マークを表示しない
	BOOL c_nookuriconv;		//送り仮名が決定したとき変換を開始しない
	BOOL c_delokuricncl;	//取消のとき送り仮名を削除する
	BOOL c_backincenter;	//後退に確定を含める
	BOOL c_addcandktkn;		//候補に片仮名変換を追加する
	BOOL c_showmodeimm;		//没入型で入力モードを表示する
	BOOL c_showromancomp;	//入力途中のキーシーケンスを表示する

	//ローマ字・仮名
	std::wstring roman;		//ローマ字
	std::wstring kana;		//仮名
	size_t accompidx;		//送り仮名インデックス

	//検索用見出し語
	std::wstring searchkey;		//数値変換で数値→#
	std::wstring searchkeyorg;	//オリジナル

	//候補
	CANDIDATES candidates;	//候補
	size_t candidx;			//候補インデックス
	size_t candorgcnt;		//オリジナル見出し語の候補数

	size_t cursoridx;		//カーソルインデックス

	std::wstring postbuf;	//直近に入力した文字列
	std::wstring prevkata;	//直前の後置型カタカナ変換で変換した文字列

	//候補一覧選択キー
	WCHAR selkey[MAX_SELKEY_C][2][2];

	TF_PRESERVEDKEY preservedkeyon[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY preservedkeyoff[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY preservedkeyonoff[MAX_PRESERVEDKEY];
};

#endif //TEXTSERVICE_H
