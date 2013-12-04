﻿
#include "configxml.h"
#include "imvimcnf.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;

IXmlWriter *pXmlWriter;
IStream *pXmlFileStream;

WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
WCHAR pathconfigxml[MAX_PATH];	//設定

void CreateConfigPath()
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

	_wmkdir(appdata);
	SetCurrentDirectoryW(appdata);

	_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s%s", appdata, fnconfigxml);

	LPWSTR pszUserSid;
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;
	int i;

	ZeroMemory(cnfmutexname, sizeof(cnfmutexname));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(GetUserSid(&pszUserSid))
	{
		if(GetMD5(&digest, (CONST BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
		{
			for(i=0; i<_countof(digest.digest); i++)
			{
				_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
			}
		}

		LocalFree(pszUserSid);
	}

	_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", VIMCNFMUTEX, szDigest);
}

BOOL SetFileDacl(LPCWSTR path)
{
	BOOL bRet = FALSE;
	WCHAR sddl[MAX_KRNLOBJNAME] = {L'\0'};
	PSECURITY_DESCRIPTOR pSD = NULL;
	LPWSTR pszUserSid;

	if(GetUserSid(&pszUserSid))
	{
		_snwprintf_s(sddl, _TRUNCATE, L"D:%s(A;;FR;;;RC)(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;%s)",
			(IsVersion62AndOver() ? L"(A;;FR;;;AC)" : L""), pszUserSid);
		LocalFree(pszUserSid);
	}

	if(ConvertStringSecurityDescriptorToSecurityDescriptorW(sddl, SDDL_REVISION_1, &pSD, NULL))
	{
		if(SetFileSecurityW(path, DACL_SECURITY_INFORMATION, pSD))
		{
			bRet = TRUE;
		}
		LocalFree(pSD);
	}

	return bRet;
}

int GetScaledSizeX(HWND hwnd, int size)
{
	HDC hdc = GetDC(hwnd);
	int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(hwnd, hdc);
	return MulDiv(size, dpiX, 96);
}
