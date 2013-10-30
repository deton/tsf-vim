#ifndef VIUTIL_H
#define VIUTIL_H

inline int iswblank(wchar_t c)
{
	return c == L' ' || c == L'\t';
}

class ViUtil
{
public:
	static void NormalizeNewline(const std::wstring src, std::wstring *dest);
};
#endif // VIUTIL_H
