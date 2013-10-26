
#include "imcrvtip.h"
#include "TextService.h"
#include "convtype.h"

WCHAR CTextService::_GetCh(BYTE vk, BYTE vkoff)
{
	BYTE keystate[256];
	WCHAR szU[4];
	WCHAR u;
	
	GetKeyboardState(keystate);

	int retu = ToUnicode(vk, 0, keystate, szU, _countof(szU), 0);
	if(retu != 1)
	{
		u = L'\0';
	}
	else
	{
		u = szU[0];
	}

	return u;
}

BYTE CTextService::_GetSf(BYTE vk, WCHAR ch)
{
	BYTE k = SKK_NULL;

	if(ch == L'\0' && vk < KEYMAPNUM)
	{
		switch(inputmode)
		{
		case im_ascii:
		case im_jlatin:
			k = vkeymap.keylatin[vk];
			break;
		case im_hiragana:
		case im_katakana:
		case im_katakana_ank:
			k = vkeymap.keyjmode[vk];
			break;
		default:
			break;
		}
	}
	else if(ch < KEYMAPNUM)
	{
		switch(inputmode)
		{
		case im_ascii:
		case im_jlatin:
			k = ckeymap.keylatin[ch];
			break;
		case im_hiragana:
		case im_katakana:
		case im_katakana_ank:
			k = ckeymap.keyjmode[ch];
			break;
		default:
			break;
		}
	}

	return k;
}
