#ifndef VICHARSTREAM_H
#define VICHARSTREAM_H

class CTextService;
struct ITfContext;

/* Character stream structure, prototypes. */
class ViCharStream
{
public:
	enum cs_flags
	{
		CS_NONE = 0,
		CS_EMP,                       /* Empty line. */
		CS_EOF,                       /* End-of-file. */
		CS_EOL,                       /* End-of-line. */
		CS_SOF,                       /* Start-of-file. */
	};

	ViCharStream(CTextService *textService, ITfContext *tfContext);
	~ViCharStream();

	int bblank();
	int fblank();
	int fspace();
	int next();
	int prev();
	wchar_t ch();
	cs_flags flags();
	int difference();
	void save_state();
	void restore_state();

private:
	int _GetMore(bool backward);
	void _update_sol();
	void _update_eol();
	void _update_flags();

	CTextService *_textService;
	ITfContext *_tfContext;

	size_t _orig; // original index in buf
	size_t _index; // current index in buf
	size_t _sol; // start index of current line
	size_t _eol; // end index of current line. _buf[_eol] == '\n' || _buf.size()
	std::wstring _buf;
	cs_flags _flags;

	size_t _preceding_count;
	size_t _following_count;

	size_t _index_save;
	size_t _sol_save;
	size_t _eol_save;
	cs_flags _flags_save;
};
#endif //VICHARSTREAM_H
