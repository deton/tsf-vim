#include "ViCharStream.h"
#include "ViUtil.h"

ViCharStream::ViCharStream(const std::wstring &preceding, const std::wstring &following)
{
	remove_copy(preceding.begin(), preceding.end(), back_inserter(_buf), L'\r');
	_orig = _index = _buf.size();
	remove_copy(following.begin(), following.end(), back_inserter(_buf), L'\r');

	_update_sol();
	_update_eol();
	_update_flags();
}

// find start of line
void ViCharStream::_update_sol()
{
	_sol = _buf.rfind(L'\n', _index);
	if(_sol == std::wstring::npos)
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
	if(_eol == std::wstring::npos)
	{
		_eol = _buf.size();
	}
}

void ViCharStream::_update_flags()
{
	if(_buf.find_first_not_of(L" \t", _sol) >= _eol)
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

wchar_t ViCharStream::ch()
{
	return _buf[_index];
}

cs_flags ViCharStream::flags()
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

//Eat backward to the next non-whitespace character.
int ViCharStream::bblank()
{
	for(;;)
	{
		if(prev())
		{
			return 1;
		}
		if(flags() == CS_EOL || flags() == CS_EMP ||
				flags() == CS_NONE && iswblank(ch()))
		{
			continue;
		}
		break;
	}
	return 0;
}

//Eat forward to the next non-whitespace character.
int ViCharStream::fblank()
{
	for(;;)
	{
		if(next())
		{
			return 1;
		}
		if(flags() == CS_EOL || flags() == CS_EMP ||
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
	if(flags() != CS_NONE || !iswblank(ch()))
	{
		return 0;
	}
	for(;;)
	{
		if(next())
		{
			return 1;
		}
		if(flags() != CS_NONE || !iswblank(ch()))
		{
			break;
		}
	}
	return 0;
}

//set current position to the next character.
int ViCharStream::next()
{
	switch(flags())
	{
	case CS_EMP: //EMP; get next line.
	case CS_EOL: //EOL; get next line.
		_index++;
		if(_index >= _buf.size())
		{
			//TODO: acquire following text after moving cursor
			_flags = CS_EOF;
		}
		else
		{
			_sol = _index;
			_update_eol();
			_update_flags();
		}
		break;
	case CS_NONE:
		_index++;
		if(_index == _eol)
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

//set current position to the previous character.
int ViCharStream::prev()
{
	switch(flags())
	{
	case CS_EMP:				/* EMP; get previous line. */
		_index = _sol;
	case CS_EOL:				/* EOL; get previous line. */
		--_index;
		_eol = _index;
		--_index;
		if(_index <= 0)		/* SOF. */
		{
			//TODO: acquire preceding text after moving cursor
			_index = 0;
			_flags = CS_SOF;
			break;
		}
		_update_sol();
		_update_flags();
		break;
	case CS_EOF:				/* EOF: get previous char. */
	case CS_NONE:
		if(_index == _sol)
		{
			if(_index == 0)
			{
				//TODO: acquire preceding text after moving cursor
				_flags = CS_SOF;
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
