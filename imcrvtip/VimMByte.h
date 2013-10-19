#ifndef VIMMBYTE_H
#define VIMMBYTE_H

class VimMByte
{
public:
	enum ChClass
	{
		BLANK_LF,
		PUNCT,
		ASCII_WORD,
		OTHER,
		JA_SYMBOLS,
		JA_ALPHANUM,
		JA_HIRAGANA,
		JA_KATAKANA,
		JA_GREEK,
		JA_RUSSIAN,
		JA_LINES,
		JA_KANJI,
		CHCLASS_SIZE
	};

	// TODO: surrogate pair
	static ChClass chclass(wchar_t c);
	static bool ismulti(wchar_t c);
};
#endif //VIMMBYTE_H
