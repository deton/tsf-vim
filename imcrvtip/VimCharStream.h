#ifndef VIMCHARSTREAM_H
#define VIMCHARSTREAM_H

class CTextService;
struct ITfContext;

class VimCharStream
{
public:
	VimCharStream(CTextService *textService, ITfContext *tfContext);
	~VimCharStream();

	int inc();
	int incl();
	int dec();
	int decl();
	wchar_t gchar();
	int difference();
	void save_index();
	void restore_index();

private:
	int _GetMore();

	CTextService *_textService;
	ITfContext *_tfContext;

	size_t _orig; // original index in buf
	size_t _index; // current index in buf
	std::wstring _buf;

	size_t _preceding_count;
	size_t _following_count;

	size_t _index_save;
};
#endif //VIMCHARSTREAM_H
