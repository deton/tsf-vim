
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
	TF_PRESERVEDKEY on[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY off[MAX_PRESERVEDKEY];
	_LoadPreservedKeySub(on);
	//半/全キー等は、トグルでなくNormal mode移行にのみ使用。
	//Insert mode移行は、'a','i'等いろんなキーがあるので
	ZeroMemory(off, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	//_LoadPreservedKeySub(off);

	ZeroMemory(preservedkeyon, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	ZeroMemory(preservedkeyoff, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	ZeroMemory(preservedkeyonoff, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	//OnとOff両方に同じ定義がある場合は、トグルとして扱う
	int i, j;
	int idxonoff = 0;
	int idxon = 0;
	for(i=0; i<MAX_PRESERVEDKEY; i++)
	{
		if(on[i].uVKey == 0 && on[i].uModifiers == 0)
		{
			break;
		}
		if(std::find(off, off + MAX_PRESERVEDKEY, on[i]) < off + MAX_PRESERVEDKEY)
		{
			preservedkeyonoff[idxonoff] = on[i];
			idxonoff++;
		}
		else
		{
			preservedkeyon[idxon] = on[i];
			idxon++;
		}
	}
	//EscapeキーをNormal mode移行用に登録
	preservedkeyon[idxon].uVKey = VK_ESCAPE;
	preservedkeyon[idxon].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
	idxon++;
	//Onに無くOffだけにある定義
	int idxoff = 0;
	for(j=0; j<MAX_PRESERVEDKEY; j++)
	{
		if(off[j].uVKey == 0 && off[j].uModifiers == 0)
		{
			break;
		}
		if(std::find(on, on + MAX_PRESERVEDKEY, off[j]) == on + MAX_PRESERVEDKEY)
		{
			preservedkeyoff[idxoff] = off[j];
			idxoff++;
		}
	}
}
