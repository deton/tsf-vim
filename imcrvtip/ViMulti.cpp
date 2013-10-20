#include "ViMulti.h"

ViMulti::ChClass ViMulti::chclass(wchar_t c, ViMulti::ChClass curchclass)
{
	// cf. jis0208_chclass() in multi_chclass.c of nvi-m17n.
	const static std::wstring ch_kanji(L"VWXY");
	const static std::wstring ch_kana(L"RSTU[`");
	if (ch_kanji.find(c) != std::wstring::npos)
	{
		return KANJI;
	}
	if (ch_kana.find(c) != std::wstring::npos)
	{
		return (curchclass == HIRAGANA) ? HIRAGANA : KATAKANA;
	}

	// cf. Util::GetScriptType() in base/util.cc of mozc
#define INRANGE(w, a, b) ((w) >= (a) && (w) <= (b))
	if (INRANGE(c, 0x0030, 0x0039) ||    // ascii number
		INRANGE(c, 0xFF10, 0xFF19) ||    // full width number
		INRANGE(c, 0x0041, 0x005A) ||    // ascii upper
		INRANGE(c, 0x0061, 0x007A) ||    // ascii lower
		INRANGE(c, 0xFF21, 0xFF3A) ||    // fullwidth ascii upper
		INRANGE(c, 0xFF41, 0xFF5A)) {    // fullwidth ascii lower
		return ALPHA;
	} else if (
		INRANGE(c, 0x3400, 0x4DBF) ||    // CJK Unified Ideographs Extension A
		INRANGE(c, 0x4E00, 0x9FFF) ||    // CJK Unified Ideographs
		INRANGE(c, 0xF900, 0xFAFF)) {    // CJK Compatibility Ideographs
		return KANJI;
	} else if (INRANGE(c, 0x3041, 0x309F)) {    // hiragana
		return HIRAGANA;
	} else if (
		INRANGE(c, 0x30A1, 0x30FF) ||  // full width katakana
		INRANGE(c, 0x31F0, 0x31FF) ||  // Katakana Phonetic Extensions for Ainu
		INRANGE(c, 0xFF65, 0xFF9F)) {  // half width katakana
		return KATAKANA;
	}

	if (c < 0x3041) // hiragana start
	{
		return MARK;
	}
	return NONKANJI;
}

bool ViMulti::Wordbound(ViMulti::ChClass oldchclass, ViMulti::ChClass curchclass, bool forward)
{
	/*
	 * if it is just beginning, we don't bother.
	 */
	if (oldchclass == _INIT)
	{
		return false;
	}

	/*
	 * if we are going forward and next char is stronger, we've hit
	 * word boundary.
	 * if we are going backward, and lefthandside char is weaker,
	 * we've hit word boundary.
	 */
	if (forward)
	{
		if (oldchclass < curchclass)
		{
			return true;
		}
	}
	else
	{
		if (oldchclass > curchclass)
		{
			return true;
		}
	}

	return false;
}

bool ViMulti::wordbound(ViMulti::ChClass oldchclass, ViMulti::ChClass curchclass, bool forward)
{
	/*
	 * if it is just beginning, we don't bother.
	 */
	if (oldchclass == _INIT)
	{
		return false;
	}

	/*
	 * if they are in different character class, we've hit word boundary.
	 */
	if (oldchclass != curchclass)
	{
		return true;
	}

	return false;
}
