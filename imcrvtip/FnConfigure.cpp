
#include "imcrvtip.h"
#include "TextService.h"

static const TF_PRESERVEDKEY c_PreservedKey[] =
{
	 { VK_OEM_3/*0xC0*/, TF_MOD_ALT }
	,{ VK_KANJI/*0x19*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_AUTO/*0xF3*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_ENLW/*0xF4*/, TF_MOD_IGNORE_ALL_MODIFIER }
};

static bool operator ==(const TF_PRESERVEDKEY &a, const TF_PRESERVEDKEY &b)
{
	return a.uVKey == b.uVKey && a.uModifiers == b.uModifiers;
}

static void _LoadPreservedKeySub(TF_PRESERVEDKEY preservedkey[])
{
	int i = 0;

	ZeroMemory(preservedkey, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);

	for(i=0; i<_countof(c_PreservedKey); i++)
	{
		preservedkey[i] = c_PreservedKey[i];
	}
}

void CTextService::_LoadPreservedKey()
{
	//TODO:設定ファイルからの読込
	//EscapeキーをNormal mode移行用に登録
	ZeroMemory(preservedkeynormal, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	preservedkeynormal[0].uVKey = VK_ESCAPE;
	preservedkeynormal[0].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;

	//半/全キー等は、他IME切替に使用
	_LoadPreservedKeySub(preservedkeyotherime);
}
