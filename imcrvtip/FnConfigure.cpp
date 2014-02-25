
#include "configxml.h"
#include "imcrvtip.h"
#include "TextService.h"

//EscapeキーをNormal mode移行用に登録
static const TF_PRESERVEDKEY configpreservedkeynormal[] =
{
	 { VK_ESCAPE/*0x1B*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ 0, 0 }
};

//半/全キー等は、他IME切替に使用
static const TF_PRESERVEDKEY configpreservedkeyotherime[] =
{
	 { VK_OEM_3/*0xC0*/, TF_MOD_ALT }
	,{ VK_KANJI/*0x19*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_AUTO/*0xF3*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_ENLW/*0xF4*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ 0, 0 }
};

//他IMEに切り替えて、他IMEをオフ状態にする
static const TF_PRESERVEDKEY configpreservedkeyotherimeoff[] =
{
	 { VK_OEM_ATTN/*VK_DBE_ALPHANUMERIC 0xF0*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ 0, 0 }
};

void CTextService::_CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s%s", appdata, fnconfigxml);

	LPWSTR pszUserSid;
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;
	int i;

	ZeroMemory(szDigest, sizeof(szDigest));

	if(GetUserSid(&pszUserSid))
	{
		if(GetMD5(&digest, (CONST BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid) * sizeof(WCHAR)))
		{
			for(i = 0; i < _countof(digest.digest); i++)
			{
				_snwprintf_s(&szDigest[i * 2], _countof(szDigest) - i * 2, _TRUNCATE, L"%02x", digest.digest[i]);
			}
		}

		LocalFree(pszUserSid);
	}

	_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", VIMCNFMUTEX, szDigest);
}

void CTextService::_LoadBehavior()
{
	std::wstring strxmlval;

	ReadValue(pathconfigxml, SectionBehavior, ValueOtherIme1, strxmlval);
	if(strxmlval == L"Alt+Shift")
	{
		c_otherime1 = L'A';
	}
	else if(strxmlval == L"Ctrl+Shift")
	{
		c_otherime1 = L'C';
	}
	else if(strxmlval == L"Win+Space")
	{
		c_otherime1 = L'W';
	}
	else
	{
		if (IsVersion62AndOver()) // Windows 8
		{
			c_otherime1 = L'W';
		}
		else
		{
			c_otherime1 = L'A';
		}
	}

	ReadValue(pathconfigxml, SectionBehavior, ValueOtherIme2, strxmlval);
	if(strxmlval.size() < 2)
	{
		c_otherime2 = 0;
	}
	else if(strxmlval[0] == L'*' && iswdigit(strxmlval[1]))
	{
		c_otherime2 = -(strxmlval[1] - L'0');
	}
	else if(strxmlval[0] == L'+' && iswdigit(strxmlval[1]))
	{
		c_otherime2 = strxmlval[1];
	}
	else
	{
		c_otherime2 = 0;
	}

	ReadValue(pathconfigxml, SectionBehavior, ValueOtherImeOffWait, strxmlval);
	c_otherimeoffwait = wcstol(strxmlval.c_str(), NULL, 0);
}
static bool operator ==(const TF_PRESERVEDKEY &a, const TF_PRESERVEDKEY &b)
{
	return a.uVKey == b.uVKey && a.uModifiers == b.uModifiers;
}

void CTextService::_LoadPreservedKey()
{
	_LoadPreservedKeySub(SectionPreservedKeyNormal, preservedkeynormal, configpreservedkeynormal);
	_LoadPreservedKeySub(SectionPreservedKeyOtherIme, preservedkeyotherime, configpreservedkeyotherime);
	_LoadPreservedKeySub(SectionPreservedKeyOtherImeOff, preservedkeyotherimeoff, configpreservedkeyotherimeoff);
	vihandler.SetPreservedKeyNormal(preservedkeynormal);
}

void CTextService::_LoadPreservedKeySub(LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[], const TF_PRESERVEDKEY configpreservedkey[])
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;

	ZeroMemory(preservedkey, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);

	if(ReadList(pathconfigxml, SectionPreservedKey, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < MAX_PRESERVEDKEY; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeVKey)
				{
					preservedkey[i].uVKey = wcstoul(r_itr->second.c_str(), NULL, 0);
				}
				else if(r_itr->first == AttributeMKey)
				{
					preservedkey[i].uModifiers =
						wcstoul(r_itr->second.c_str(), NULL, 0) & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT);
					if((preservedkey[i].uModifiers & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT)) == 0)
					{
						preservedkey[i].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
					}
				}
			}

			i++;
		}
	}
	else
	{
		for(i = 0; configpreservedkey[i].uVKey != 0; i++)
		{
			preservedkey[i] = configpreservedkey[i];
		}
	}
}

