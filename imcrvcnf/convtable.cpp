
#include "convtable.h"

// ローマ字仮名変換表

const ROMAN_KANA_CONV roman_kana_conv_default[ROMAN_KANA_TBL_DEF_NUM] =
{
	{L"0", L"0", L"-", L"-", FALSE, FALSE, TRUE},
	{L"1", L"1", L"-", L"-", FALSE, FALSE, TRUE},
	{L"2", L"2", L"-", L"-", FALSE, FALSE, TRUE},
	{L"3", L"3", L"-", L"-", FALSE, FALSE, TRUE},
	{L"4", L"4", L"-", L"-", FALSE, FALSE, TRUE},
	{L"5", L"5", L"-", L"-", FALSE, FALSE, TRUE},
	{L"6", L"6", L"-", L"-", FALSE, FALSE, TRUE},
	{L"7", L"7", L"-", L"-", FALSE, FALSE, TRUE},
	{L"8", L"8", L"-", L"-", FALSE, FALSE, TRUE},
	{L"9", L"9", L"-", L"-", FALSE, FALSE, TRUE},

	{L"h", L"h", L"-", L"-", FALSE, FALSE, TRUE},
	{L"j", L"j", L"-", L"-", FALSE, FALSE, TRUE},
	{L"k", L"k", L"-", L"-", FALSE, FALSE, TRUE},
	{L"l", L"l", L"-", L"-", FALSE, FALSE, TRUE},
	{L"o", L"o", L"-", L"-", FALSE, FALSE, TRUE},

	{L"\x20", L"\x20", L"-", L"-", FALSE, FALSE, TRUE},
	{L"$", L"$", L"-", L"-", FALSE, FALSE, TRUE},
	{L"(", L"(", L"-", L"-", FALSE, FALSE, TRUE},
	{L")", L")", L"-", L"-", FALSE, FALSE, TRUE},
	{L"+", L"+", L"-", L"-", FALSE, FALSE, TRUE},
	{L",", L",", L"-", L"-", FALSE, FALSE, TRUE},
	{L"-", L"-", L"-", L"-", FALSE, FALSE, TRUE},
	{L"/", L"/", L"-", L"-", FALSE, FALSE, TRUE},
	{L";", L";", L"-", L"-", FALSE, FALSE, TRUE},
	{L"^", L"^", L"-", L"-", FALSE, FALSE, TRUE},
	{L"_", L"_", L"-", L"-", FALSE, FALSE, TRUE},
	{L"~", L"~", L"-", L"-", FALSE, FALSE, TRUE},

	{L"",L"",L"",L""}
};

// ASCII全英変換表

const ASCII_JLATIN_CONV ascii_jlatin_conv_default[ASCII_JLATIN_TBL_NUM] =
{
	{L" ", L"　"}, {L"!", L"！"}, {L"\"", L"”"}, {L"#", L"＃"}, {L"$", L"＄"}, {L"%", L"％"}, {L"&", L"＆"}, {L"\'", L"’"},
	{L"(", L"（"}, {L")", L"）"}, {L"*", L"＊"}, {L"+", L"＋"}, {L",", L"，"}, {L"-", L"－"}, {L".", L"．"}, {L"/", L"／"},

	{L"0", L"０"}, {L"1", L"１"}, {L"2", L"２"}, {L"3", L"３"}, {L"4", L"４"}, {L"5", L"５"}, {L"6", L"６"}, {L"7", L"７"},
	{L"8", L"８"}, {L"9", L"９"}, {L":", L"："}, {L";", L"；"}, {L"<", L"＜"}, {L"=", L"＝"}, {L">", L"＞"}, {L"?", L"？"},

	{L"@", L"＠"}, {L"A", L"Ａ"}, {L"B", L"Ｂ"}, {L"C", L"Ｃ"}, {L"D", L"Ｄ"}, {L"E", L"Ｅ"}, {L"F", L"Ｆ"}, {L"G", L"Ｇ"},
	{L"H", L"Ｈ"}, {L"I", L"Ｉ"}, {L"J", L"Ｊ"}, {L"K", L"Ｋ"}, {L"L", L"Ｌ"}, {L"M", L"Ｍ"}, {L"N", L"Ｎ"}, {L"O", L"Ｏ"},

	{L"P", L"Ｐ"}, {L"Q", L"Ｑ"}, {L"R", L"Ｒ"}, {L"S", L"Ｓ"}, {L"T", L"Ｔ"}, {L"U", L"Ｕ"}, {L"V", L"Ｖ"}, {L"W", L"Ｗ"},
	{L"X", L"Ｘ"}, {L"Y", L"Ｙ"}, {L"Z", L"Ｚ"}, {L"[", L"［"}, {L"\\", L"＼"}, {L"]", L"］"}, {L"^", L"＾"}, {L"_", L"＿"},

	{L"`", L"‘"}, {L"a", L"ａ"}, {L"b", L"ｂ"}, {L"c", L"ｃ"}, {L"d", L"ｄ"}, {L"e", L"ｅ"}, {L"f", L"ｆ"}, {L"g", L"ｇ"},
	{L"h", L"ｈ"}, {L"i", L"ｉ"}, {L"j", L"ｊ"}, {L"k", L"ｋ"}, {L"l", L"ｌ"}, {L"m", L"ｍ"}, {L"n", L"ｎ"}, {L"o", L"ｏ"},

	{L"p", L"ｐ"}, {L"q", L"ｑ"}, {L"r", L"ｒ"}, {L"s", L"ｓ"}, {L"t", L"ｔ"}, {L"u", L"ｕ"}, {L"v", L"ｖ"}, {L"w", L"ｗ"},
	{L"x", L"ｘ"}, {L"y", L"ｙ"}, {L"z", L"ｚ"}, {L"{", L"｛"}, {L"|", L"｜"}, {L"}", L"｝"}, {L"~", L"～"},

	{L"",L""}
};
