
#include "configxml.h"
#include "imvimcnf.h"
#include "resource.h"

TF_PRESERVEDKEY preservedkeynormal[MAX_PRESERVEDKEY];
TF_PRESERVEDKEY preservedkeyotherime[MAX_PRESERVEDKEY];

static const TF_PRESERVEDKEY defaultpreservedkeynormal[] =
{
	 { VK_ESCAPE/*0x1B*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ 0, 0 }
};

static const TF_PRESERVEDKEY defaultpreservedkeyotherime[] =
{
	 { VK_OEM_3/*0xC0*/, TF_MOD_ALT }
	,{ VK_KANJI/*0x19*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_AUTO/*0xF3*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ VK_OEM_ENLW/*0xF4*/, TF_MOD_IGNORE_ALL_MODIFIER }
	,{ 0, 0 }
};

static void LoadConfigPreservedKeySub(LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[], const TF_PRESERVEDKEY defaultpreservedkey[])
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
					preservedkey[i].uModifiers = wcstoul(r_itr->second.c_str(), NULL, 0);
					if(preservedkey[i].uModifiers == 0)
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
		for(i = 0; defaultpreservedkey[i].uVKey != 0; i++)
		{
			preservedkey[i] = defaultpreservedkey[i];
		}
	}
}

void LoadConfigPreservedKey()
{
	LoadConfigPreservedKeySub(SectionPreservedKeyNormal, preservedkeynormal, defaultpreservedkeynormal);
	LoadConfigPreservedKeySub(SectionPreservedKeyOtherIme, preservedkeyotherime, defaultpreservedkeyotherime);
}

static void LoadPreservedKeySub(HWND hWndList, const TF_PRESERVEDKEY preservedkey[])
{
	int i;
	LVITEMW item;
	WCHAR num[8];

	for(i=0; i<MAX_PRESERVEDKEY; i++)
	{
		if(preservedkey[i].uVKey == 0 &&
			preservedkey[i].uModifiers == 0)
		{
			break;
		}

		item.mask = LVIF_TEXT;
		_snwprintf_s(num, _TRUNCATE, L"0x%02X", preservedkey[i].uVKey);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 0;
		ListView_InsertItem(hWndList, &item);
		_snwprintf_s(num, _TRUNCATE, L"%d", preservedkey[i].uModifiers & TF_MOD_ALT ? 1 : 0);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 1;
		ListView_SetItem(hWndList, &item);
		_snwprintf_s(num, _TRUNCATE, L"%d", preservedkey[i].uModifiers & TF_MOD_CONTROL ? 1 : 0);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 2;
		ListView_SetItem(hWndList, &item);
		_snwprintf_s(num, _TRUNCATE, L"%d", preservedkey[i].uModifiers & TF_MOD_SHIFT ? 1 : 0);
		item.pszText = num;
		item.iItem = i;
		item.iSubItem = 3;
		ListView_SetItem(hWndList, &item);
	}
}

void LoadPreservedKey(HWND hwnd)
{
	LoadConfigPreservedKey();

	LoadPreservedKeySub(GetDlgItem(hwnd, IDC_LIST_PRSRVKEY), preservedkeynormal);
	LoadPreservedKeySub(GetDlgItem(hwnd, IDC_LIST_PRSRVKEYOTHERIME), preservedkeyotherime);
}

static void SavePreservedKeySub(HWND hWndListView, LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[])
{
	int i, count;
	WCHAR key[8];
	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;

	count = ListView_GetItemCount(hWndListView);
	for(i=0; i<count && i<MAX_PRESERVEDKEY; i++)
	{
		ListView_GetItemText(hWndListView, i, 0, key, _countof(key));
		preservedkey[i].uVKey = wcstoul(key, NULL, 0);
		preservedkey[i].uModifiers = 0;
		ListView_GetItemText(hWndListView, i, 1, key, _countof(key));
		if(key[0] == L'1')
		{
			preservedkey[i].uModifiers |= TF_MOD_ALT;
		}
		ListView_GetItemText(hWndListView, i, 2, key, _countof(key));
		if(key[0] == L'1')
		{
			preservedkey[i].uModifiers |= TF_MOD_CONTROL;
		}
		ListView_GetItemText(hWndListView, i, 3, key, _countof(key));
		if(key[0] == L'1')
		{
			preservedkey[i].uModifiers |= TF_MOD_SHIFT;
		}
	}
	if(count < MAX_PRESERVEDKEY)
	{
		preservedkey[count].uVKey = 0;
		preservedkey[count].uModifiers = 0;
	}

	WriterStartSection(pXmlWriter, SectionPreservedKey);

	for(i=0; i<MAX_PRESERVEDKEY; i++)
	{
		if(preservedkey[i].uVKey == 0 &&
			preservedkey[i].uModifiers == 0)
		{
			break;
		}

		attr.first = AttributeVKey;
		_snwprintf_s(key, _TRUNCATE, L"0x%02X", preservedkey[i].uVKey);
		attr.second = key;
		row.push_back(attr);

		attr.first = AttributeMKey;
		_snwprintf_s(key, _TRUNCATE, L"%X", preservedkey[i].uModifiers);
		attr.second = key;
		row.push_back(attr);

		list.push_back(row);
		row.clear();
	}

	WriterList(pXmlWriter, list);

	WriterEndSection(pXmlWriter);
}

void SavePreservedKey(HWND hwnd)
{
	SavePreservedKeySub(GetDlgItem(hwnd, IDC_LIST_PRSRVKEY), SectionPreservedKeyNormal, preservedkeynormal);
	SavePreservedKeySub(GetDlgItem(hwnd, IDC_LIST_PRSRVKEYOTHERIME), SectionPreservedKeyOtherIme, preservedkeyotherime);
}
