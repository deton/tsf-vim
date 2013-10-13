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
	ViCharStream(const std::wstring &preceding, const std::wstring &following);
	~ViCharStream();

	int bblank();
	int fblank();
	int fspace();
	int next();
	int prev();
	wchar_t ch();
	cs_flags flags();
	int difference();

private:
	void _update_sol();
	void _update_eol();
	void _update_flags();

	size_t _orig; // original index in buf
	size_t _index; // current index in buf
	size_t _sol; // start index of current line
	size_t _eol; // end index of current line. _buf[_eol] == '\n' || _buf.size()
	std::wstring _buf;
	cs_flags _flags;
};
#endif //VICHARSTREAM_H
