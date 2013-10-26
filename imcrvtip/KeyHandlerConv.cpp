
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
