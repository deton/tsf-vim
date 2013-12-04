﻿
#include "imvimcnf.h"
#include "resource.h"

HINSTANCE hInst;

// static dialog procedure
INT_PTR CALLBACK DlgProcPreservedKey(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HANDLE hMutex;
	INITCOMMONCONTROLSEX icex;

	_wsetlocale(LC_ALL, L"JPN");

	hInst = hInstance;

	CreateConfigPath();

	hMutex = CreateMutexW(NULL, FALSE, cnfmutexname);
	if(hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	CreateProperty(hInstance);
	
	return 0;
}

void CreateProperty(HINSTANCE hInstance)
{
	PROPSHEETPAGEW psp;
	PROPSHEETHEADERW psh;
	struct {
		int id;
		DLGPROC DlgProc;
	} DlgPage[] = {
		{IDD_DIALOG_PRSRVKEY,	DlgProcPreservedKey},
	};
	HPROPSHEETPAGE hpsp[_countof(DlgPage)];
	int i;

	ZeroMemory(&psp, sizeof(PROPSHEETPAGEW));
	psp.dwSize = sizeof(PROPSHEETPAGEW);
	psp.dwFlags = PSP_PREMATURE;
	psp.hInstance = hInst;

	for(i=0; i<_countof(DlgPage); i++)
	{
		psp.pszTemplate = MAKEINTRESOURCE(DlgPage[i].id);
		psp.pfnDlgProc = DlgPage[i].DlgProc;
		hpsp[i] = CreatePropertySheetPageW(&psp);
	}

	ZeroMemory(&psh, sizeof(PROPSHEETHEADERW));
	psh.dwSize = sizeof(PROPSHEETHEADERW);
	psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP;
	psh.hInstance = hInstance;
	psh.hwndParent = NULL;
	psh.nPages = _countof(DlgPage);
	psh.phpage = hpsp;
	psh.pszCaption = TEXTSERVICE_DESC L" (ver. " TEXTSERVICE_VER L")";
	PropertySheetW(&psh);

	return;
}
