#ifndef VIMULTI_H
#define VIMULTI_H

class ViMulti
{
public:
	enum ChClass
	{
		_INIT = 0,
		MARK = 1,
		ALPHA = 5,
		HIRAGANA = 2,
		KATAKANA = 10,
		KANJI = 20,
		NONKANJI = 100,
	};

	// TODO: surrogate pair
	static ChClass chclass(wchar_t c, ChClass curchclass);
	static bool Wordbound(ChClass oldchclass, ChClass curchclass, bool forward);
	static bool wordbound(ChClass oldchclass, ChClass curchclass, bool forward);
};
#endif //VIMULTI_H
