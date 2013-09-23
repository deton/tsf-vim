#ifndef VICHARSTREAM_H
#define VICHARSTREAM_H

inline int iswblank(wchar_t c)
{
	return c == L' ' || c == L'\t';
}

enum cs_flags
{
	CS_NONE = 0,
	CS_EMP,                       /* Empty line. */
	CS_EOF,                       /* End-of-file. */
	CS_EOL,                       /* End-of-line. */
	CS_SOF,                       /* Start-of-file. */
};

/* Character stream structure, prototypes. */
class ViCharStream
{
public:
	ViCharStream(const std::wstring &buf);
	~ViCharStream();

	int bblank();
	int fblank();
	int fspace();
	int next();
	int prev();
	wchar_t ch();
	cs_flags flags();
	size_t index();

private:
	size_t _index; //current index in buf
	size_t _cno; //current column in line
	size_t _len; //current line length
	std::wstring _buf;
	cs_flags _flags;
};
#endif //VICHARSTREAM_H
