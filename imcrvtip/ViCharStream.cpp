#include "ViCharStream.h"
#include "ViUtil.h"
#include "mozc/win32/tip/tip_surrounding_text.h"

ViCharStream::ViCharStream(CTextService *textService, ITfContext *tfContext)
	: _textService(textService), _tfContext(tfContext)
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
	}
	_update_sol();
	_update_eol();
	_update_flags();
	_eol_save = _eol;
	_sol_save = _sol;
	_flags_save = _flags;
}

// find start of line
void ViCharStream::_update_sol()
{
	_sol = _buf.rfind(L'\n', _index);
	if (_sol == std::wstring::npos)
	{
		_sol = 0;
	}
	else
	{
		_sol++;
	}
}

// find end of line
void ViCharStream::_update_eol()
{
	_eol = _buf.find(L'\n', _index);
	if (_eol == std::wstring::npos)
	{
		_eol = _buf.size();
	}
}

void ViCharStream::_update_flags()
{
	if (_buf.find_first_not_of(L" \t", _sol) >= _eol)
	{
		_index = _eol;
		_flags = CS_EMP;
	}
	else
	{
		_flags = CS_NONE;
	}
}

ViCharStream::~ViCharStream()
{
}

int ViCharStream::_GetMore(bool backward)
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
		_update_sol();
		_sol_save += addsize;
		_eol += addsize;
		_eol_save += addsize;
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
		_update_eol();
		_following_count += info.following_text.size();
	}
	return 0;
}


wchar_t ViCharStream::ch()
{
	return _buf[_index];
}

ViCharStream::cs_flags ViCharStream::flags()
{
	return _flags;
}

int ViCharStream::difference()
{
	return _index - _orig;
}

void ViCharStream::save_state()
{
	_index_save = _index;
	_sol_save = _sol;
	_eol_save = _eol;
	_flags_save = _flags;
}

void ViCharStream::restore_state()
{
	_index = _index_save;
	_sol = _sol_save;
	_eol = _eol_save;
	_flags = _flags_save;
}

// Eat backward to the next non-whitespace character.
int ViCharStream::bblank()
{
	for (;;)
	{
		if (prev())
		{
			return 1;
		}
		if (flags() == CS_EOL || flags() == CS_EMP ||
				flags() == CS_NONE && iswblank(ch()))
		{
			continue;
		}
		break;
	}
	return 0;
}

// Eat forward to the next non-whitespace character.
int ViCharStream::fblank()
{
	for (;;)
	{
		if (next())
		{
			return 1;
		}
		if (flags() == CS_EOL || flags() == CS_EMP ||
				flags() == CS_NONE && iswblank(ch()))
		{
			continue;
		}
		break;
	}
	return 0;
}

/*
 *	If on a space, eat forward until something other than a
 *	whitespace character.
 */
int ViCharStream::fspace()
{
	if (flags() != CS_NONE || !iswblank(ch()))
	{
		return 0;
	}
	for (;;)
	{
		if (next())
		{
			return 1;
		}
		if (flags() != CS_NONE || !iswblank(ch()))
		{
			break;
		}
	}
	return 0;
}

// set current position to the next character.
int ViCharStream::next()
{
// acquire more following text
#define GETMORE_FOLLOWING(failflag) \
	do { \
		if (_GetMore(false) == -1 || _index >= _buf.size() - 1) \
		{ \
			/* cannot get more text || get more text but emtpy */ \
			_index++; \
			_flags = (failflag); \
			return 0; \
		} \
	} while (0)

	switch (flags())
	{
	case CS_EMP: // EMP; get next line.
		while (_index >= _buf.size() - 1)
		{
			// here _index == _eol == _buf.size()
			GETMORE_FOLLOWING(CS_EOF);
			// yet EMP line or become non-EMP line
			_update_flags();
			if (flags() != CS_EMP)
			{
				return 0;
			}
		}
		//FALLTHRU
	case CS_EOL: // EOL; get next line.
		if (_index >= _buf.size() - 1)
		{
			GETMORE_FOLLOWING(CS_EOF);
		}
		_index++;
		_sol = _index;
		_update_eol();
		_update_flags();
		break;
	case CS_NONE:
		if (_index >= _buf.size() - 1)
		{
			GETMORE_FOLLOWING(CS_EOL);
		}
		_index++;
		if (_index == _eol)
		{
			_flags = CS_EOL;
		}
		break;
	case CS_EOF:
		break;
	default:
		break;
	}
	return 0;
}

// set current position to the previous character.
int ViCharStream::prev()
{
#define GETMORE_PRECEDING() \
	do { \
		/* acquire more preceding text */ \
		if (_GetMore(true) == -1 || _index == 0) \
		{ \
			/* cannot get more text || get more text but emtpy */ \
			_index = 0; \
			_flags = CS_SOF; \
			return 0; \
		} \
	} while (0)

	switch (flags())
	{
	case CS_EMP:				/* EMP; get previous line. */
		_index = _sol;
		while (_index == 0)
		{
			GETMORE_PRECEDING();
			// yet EMP line or become non-EMP line
			_update_flags();
			if (flags() != CS_EMP)
			{
				--_index;
				_flags = CS_NONE;
				return 0;
			}
			_index = _sol;
		}
		//FALLTHRU
	case CS_EOL:				/* EOL; get previous line. */
		--_index;
		_eol = _index; // '\n'
		if (_index == 0)		/* SOF. */
		{
			GETMORE_PRECEDING();
		}
		--_index;
		_update_sol();
		_update_flags();
		break;
	case CS_EOF:				/* EOF: get previous char. */
	case CS_NONE:
		if (_index == _sol)
		{
			if (_index == 0)
			{
				GETMORE_PRECEDING();
				--_index;
				_flags = CS_NONE;
			}
			else
			{
				_flags = CS_EOL;
			}
		}
		else
		{
			--_index;
			_flags = CS_NONE;
		}
		break;
	case CS_SOF:				/* SOF. */
		break;
	default:
		break;
	}
	return 0;
}
