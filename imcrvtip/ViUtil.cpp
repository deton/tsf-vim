#include "ViUtil.h"

void ViUtil::NormalizeNewline(const std::wstring src, std::wstring *dest)
{
	dest->clear();
	// \r\n: notepad
	*dest = std::regex_replace(src, std::wregex(L"\r\n"), std::wstring(L"\n"));
	// only \r: WordPad, Word, Outlook
	replace(dest->begin(), dest->end(), L'\r', L'\n');
}
