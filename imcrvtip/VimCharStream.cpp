#include "VimCharStream.h"
#include "ViUtil.h"
#include "mozc/win32/tip/tip_surrounding_text.h"
//#define DEBUGLOG 1
#if DEBUGLOG
#include <fstream>
#endif

VimCharStream::VimCharStream(CTextService *textService, ITfContext *tfContext)
	: _textService(textService), _tfContext(tfContext)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if (mozc::win32::tsf::TipSurroundingText::Get(_textService, _tfContext, &info))
	{
		remove_copy(info.preceding_text.begin(), info.preceding_text.end(), back_inserter(_buf), L'\r');
		_orig = _index = _buf.size();
		remove_copy(info.following_text.begin(), info.following_text.end(), back_inserter(_buf), L'\r');
		_preceding_count = info.preceding_text.size();
		_following_count = info.following_text.size();
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
		offset = -_preceding_count;
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
		remove_copy(info.preceding_text.begin(), info.preceding_text.end(),
				inserter(_buf, _buf.begin()), L'\r');
		size_t addsize = _buf.size() - oldsize;
		_orig += addsize;
		_index += addsize;
		_preceding_count += info.preceding_text.size();
#if DEBUGLOG
		std::wofstream log("\\tsfvim.log");
		log << _buf.c_str() << std::endl;
#endif
	}
	else
	{
		if (info.following_text.size() == 0)
		{
			return -1;
		}
		remove_copy(info.following_text.begin(), info.following_text.end(),
				back_inserter(_buf), L'\r');
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
	if (_index == _buf.size() - 1)
	{
		// acquire more following text
		if (_GetMore(false) == -1 || _index == _buf.size() - 1)
		{
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

int VimCharStream::fblank()
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
