
#include "imcrvtip.h"
#include "TextService.h"

void CTextService::_StartConfigure()
{
	HANDLE hMutex = OpenMutexW(SYNCHRONIZE, FALSE, cnfmutexname);
	if(hMutex != NULL)
	{
		CloseHandle(hMutex);
		return;
	}

	_StartProcess(VIMCNFEXE);
}

void CTextService::_StartProcess(LPCWSTR fname)
{
	WCHAR path[MAX_PATH];
	WCHAR drive[_MAX_DRIVE];
	WCHAR dir[_MAX_DIR];
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;

	GetModuleFileNameW(g_hInst, path, _countof(path));
	_wsplitpath_s(path, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);
	_snwprintf_s(path, _TRUNCATE, L"%s%s%s", drive, dir, fname);

	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	if(CreateProcessW(path, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}
