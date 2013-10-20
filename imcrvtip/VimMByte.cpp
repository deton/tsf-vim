#include "VimMByte.h"
#include "ViUtil.h"

VimMByte::ChClass VimMByte::chclass(wchar_t c)
{
	if (!ismulti(c))
	{
		if (iswblank(c) || c == L'\n')
		{
			return VimMByte::BLANK_LF;
		}
		if (iswalnum(c) || c == L'_')
		{
			return VimMByte::ASCII_WORD;
		}
		return VimMByte::OTHER;
	}

	if (c == L'�A' || c == L'�B' || c == L'�C' || c == L'�D')
	{
		return VimMByte::PUNCT;
	}
	// TODO: Support JA_KANJI etc.
	return VimMByte::OTHER;
}

bool VimMByte::ismulti(wchar_t c)
{
	return c >= 0x80;
}
