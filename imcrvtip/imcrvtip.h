﻿
#ifndef IMCRVTIP_H
#define IMCRVTIP_H

#include "common.h"

//for resource
#define RC_FILE				"imcrvtip"

//入力モード
enum
{
    im_default = 0,		//デフォルト
    im_hiragana,		//ひらがな
    im_katakana,		//カタカナ
    im_katakana_ank,	//半角ｶﾀｶﾅ
    im_jlatin,			//全英
    im_ascii			//ASCII
};

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATEBASE;
//		pair< CANDIDATEBASE(表示用), CANDIDATEBASE(辞書登録用) >
//		例）数値変換の場合 < < "明治四五年", "年号(1868-1912)" >, < "明治#2年", "年号(1868-1912)" > >
typedef std::pair< CANDIDATEBASE, CANDIDATEBASE > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

#define KEYMAPNUM		0x80

//skk function code
#define SKK_NULL		0x00	// NUL

#define SKK_KANA		0x71	// かな／カナ	q
#define SKK_CONV_CHAR	0x11	// ｶﾅ全英変換	c-q
#define SKK_JLATIN		0x4C	// 全英			L
#define SKK_ASCII		0x6C	// アスキー		l
#define SKK_JMODE		0x0A	// ひらがな		c-j(LF)	(c-q)	(ASCII/全英モード)
#define SKK_ABBREV		0x2F	// abbrev		/
#define SKK_AFFIX		0x3E	// 接辞			> <
#define SKK_NEXT_CAND	0x20	// 次候補		SP	c-n
#define SKK_PREV_CAND	0x78	// 前候補		x	c-p
#define SKK_PURGE_DIC	0x58	// 辞書削除		X
#define SKK_NEXT_COMP	0x09	// 次補完		c-i(HT)
#define SKK_PREV_COMP	0x15	// 前補完		c-u

#define SKK_CONV_POINT	0x51	// 変換位置		Q ;
#define SKK_DIRECT		0x30	// 直接入力		0-9
#define SKK_ENTER		0x0D	// 確定			c-m(CR)	c-j(LF)
#define SKK_CANCEL		0x07	// 取消			c-g	(c-[)
#define SKK_BACK		0x08	// 後退			c-h(BS)	VK_BACK
#define SKK_DELETE		0x7F	// 削除			DEL	VK_DELETE
#define SKK_VOID		0xFF	// 無効
#define SKK_LEFT		0x02	// 左移動		c-b	VK_LEFT
#define SKK_UP			0x01	// 先頭移動		c-a	VK_UP
#define SKK_RIGHT		0x06	// 右移動		c-f	VK_RIGHT
#define SKK_DOWN		0x05	// 末尾移動		c-e	VK_DOWN
#define SKK_PASTE		0x19	// 貼付			c-y	(c-v)
#define SKK_AFTER_DELETER		0xFE	// Deleterによる直前文字列削除後

typedef struct {
	BYTE keylatin[KEYMAPNUM];	//全英/アスキー
	BYTE keyjmode[KEYMAPNUM];	//ひらがな/カタカナ
	BYTE keyvoid[KEYMAPNUM];	//無効
} KEYMAP;

//候補一覧選択キー数
#define MAX_SELKEY		7

#define CL_COLOR_BG		0
#define CL_COLOR_FR		1
#define CL_COLOR_SE		2
#define CL_COLOR_CO		3
#define CL_COLOR_CA		4
#define CL_COLOR_SC		5
#define CL_COLOR_AN		6
#define CL_COLOR_NO		7

extern LPCWSTR TextServiceDesc;
extern LPCWSTR LangbarItemDesc;
extern LPCWSTR LangbarFuncDesc;

extern HINSTANCE g_hInst;

extern const CLSID c_clsidTextService;
extern const GUID c_guidProfile;
extern const GUID c_guidPreservedKeyOn;
extern const GUID c_guidPreservedKeyOff;
extern const GUID c_guidPreservedKeyOnOff;
extern const GUID c_guidLangBarItemButton;

LONG DllAddRef();
LONG DllRelease();

#define IID_IUNK_ARGS(pType) __uuidof(*(pType)), (IUnknown *)pType

// for Windows 8
#if 1
#define EVENT_OBJECT_IME_SHOW               0x8027
#define EVENT_OBJECT_IME_HIDE               0x8028
#define EVENT_OBJECT_IME_CHANGE             0x8029

#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_LBI_INPUTMODE;
#endif

#endif //IMCRVTIP_H
