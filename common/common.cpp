
#include "common.h"

BOOL IsVersion62AndOver()
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG mask = 0;

	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;

	VER_SET_CONDITION(mask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(mask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION, mask);
}
