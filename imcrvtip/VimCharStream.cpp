#include "VimCharStream.h"
#include "ViUtil.h"
#include "mozc/win32/tip/tip_surrounding_text.h"
//#define DEBUGLOG 1
#if DEBUGLOG
#include <fstream>
#endif

VimCharStream::VimCharStream(CTextService *textService, ITfContext *tfContext)
	: _textService(textService), _tfContext(tfContext),
	  _orig(0), _index(0), _eof(false),
	  _preceding_count(0), _following_count(0),
	  _index_save(0)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if (mozc::win32::tsf::TipSurroundingText::Get(_textService, _tfContext, &info))
	{
		std::wstring tmp;
		ViUtil::NormalizeNewline(info.preceding_text, &tmp);
		_buf.append(tmp);
		_orig = _index = _index_save = _buf.size();
		ViUtil::NormalizeNewline(info.following_text, &tmp);
		_buf.append(tmp);
		_preceding_count = info.preceding_text.size();
		_following_count = info.following_text.size();
		_eof = false;
#if DEBUGLOG
		std::wofstream log("c:\\tsfvim\\log");
		log << info.preceding_text << '|' << info.following_text << std::endl;
#endif
	}
	else
	{
		_eof = true;
	}
}

VimCharStream::~VimCharStream()
{
}

int VimCharStream::_GetMore(bool backward)
{
	int offset = _following_count; // includes '\r'
	if (backward)
	{
		offset = 0 - _preceding_count;
	}
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if (!mozc::win32::tsf::TipSurroundingText::GetMore(_textService, _tfContext, offset, &info))
	{
		return -1;
	}
	if (offset < 0)
	{
		if (info.preceding_text.size() == 0)
		{
			return -1;
		}
		size_t oldsize = _buf.size();
		std::wstring tmp;
		ViUtil::NormalizeNewline(info.preceding_text, &tmp);
		_buf.insert(0, tmp);
		size_t addsize = _buf.size() - oldsize;
		_orig += addsize;
		_index += addsize;
		_index_save += addsize;
		_preceding_count += info.preceding_text.size();
	}
	else
	{
		if (info.following_text.size() == 0)
		{
			return -1;
		}
		std::wstring tmp;
		ViUtil::NormalizeNewline(info.following_text, &tmp);
		_buf.append(tmp);
		_following_count += info.following_text.size();
	}
	return 0;
}

wchar_t VimCharStream::gchar()
{
	return _buf[_index];
}

size_t VimCharStream::index()
{
	return _index;
}

bool VimCharStream::eof()
{
	return _eof;
}

int VimCharStream::difference()
{
	return _index - _orig;
}

void VimCharStream::save_index()
{
	_index_save = _index;
}

void VimCharStream::restore_index()
{
	_index = _index_save;
}

/*
 * Increment the currnet index crossing line boundaries as necessary.
 * Return 1 when going to the next line.
 * Return 2 when moving forward onto a '\n' at the end of the line.
 * Return -1 when at the end of file.
 * Return 0 otherwise.
 */
int VimCharStream::inc()
{
	if (_index >= _buf.size() - 1)
	{
		// acquire more following text
		if (_GetMore(false) == -1 || _index >= _buf.size() - 1)
		{
			_eof = true;
			return -1; // cannot get more text || get more text but emtpy
		}
	}
	if (gchar() == L'\n')
	{
		++_index;
		return 1;
	}
	++_index;
	return (gchar() == L'\n') ? 2 : 0;
}

/*
 * incl(): same as inc(), but skip the '\n' at the end of non-empty lines
 */
int VimCharStream::incl()
{
	int r = inc();
	if (r == 2)
	{
		r = inc();
	}
	return r;
}

/*
 * Decrement the current index crossing line boundaries as necessary.
 * Return 1 when crossing a line, -1 when at start of file, 0 otherwise.
 */
int VimCharStream::dec()
{
	if (_index == 0)
	{
		// acquire more preceding text
		if (_GetMore(true) == -1 || _index == 0)
		{
			return -1; // cannot get more text || get more text but emtpy
		}
	}
	_eof = false;
	--_index;
	if (gchar() == L'\n')
	{
		return 1;
	}
	return 0;
}

/*
 * decl(): same as dec(), but skip the '\n' at the end of non-empty lines.
 * Return 2 when moving onto empty line.
 */
int VimCharStream::decl()
{
	int r = dec();
	if (r == 1)
	{
		r = dec();
		if (gchar() == L'\n') // empty line
		{
			inc();
			return 2;
		}
	}
	return r;
}

int VimCharStream::fblankl()
{
	while (iswblank(gchar()))
	{
		if (incl() == -1)
		{
			return -1;
		}
	}
	return 0;
}

int VimCharStream::fblank()
{
	while (iswblank(gchar()))
	{
		if (inc() == -1)
		{
			return -1;
		}
	}
	return 0;
}

int VimCharStream::forward_eol()
{
	if (gchar() == L'\n')
	{
		return 0;
	}

	for (int r = inc(); r != 2; r = inc())
	{
		if (r == -1) // end of file
		{
			return -1;
		}
	}
	return 0;
}

