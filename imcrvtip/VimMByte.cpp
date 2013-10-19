#include "VimMByte.h"

static int iswblank(wchar_t c)
{
	return c == L' ' || c == L'\t';
}

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

	if (c == L'、' || c == L'。' || c == L'，' || c == L'．')
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
