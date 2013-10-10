#include "ViCharStream.h"

ViCharStream::ViCharStream(const std::wstring &buf)
	: _buf(buf), _cno(0), _index(0), _flags(CS_NONE)
{
	remove(_buf.begin(), _buf.end(), L'\r');
	_eol = _buf.find(L"\n");
	if(_eol == std::wstring::npos)
	{
		_eol = _buf.size();
	}
	if(_eol == 0 || _buf.find_first_not_of(L" \t", 0, _eol) == std::wstring::npos)
	{
		_flags = CS_EMP;
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

size_t ViCharStream::index()
{
	return _index;
}

int ViCharStream::bblank()
{
	//TODO
	return 1;
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
		//TODO: skip to \n
	case CS_EOL: //EOL; get next line.
		_index++;
		if(_index >= _buf.size())
		{
			//TODO: acquire following text after moving cursor
			_flags = CS_EOF;
		}
		else
		{
			_cno = 0;
			_eol = _buf.find(L"\n", _index);
			if(_eol == std::wstring::npos)
			{
				_eol = _buf.size() - _index;
			}
			if(_eol <= _index || _buf.find_first_not_of(L" \t", _index, _eol) == std::wstring::npos)
			{
				_flags = CS_EMP;
			}
			else
			{
				_flags = CS_NONE;
			}
		}
		break;
	case CS_NONE:
		_index++;
		if(_cno == _eol - 1)
		{
			_flags = CS_EOL;
		}
		else
		{
			_cno++;
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
	int sol;
	switch(flags())
	{
	case CS_EMP:				/* EMP; get previous line. */
		//TODO: skip to \n
	case CS_EOL:				/* EOL; get previous line. */
		--_index;
		if(_index <= 0)		/* SOF. */
		{
			//TODO: acquire preceding text after moving cursor
			_index = 0;
			_flags = CS_SOF;
			break;
		}
		sol = _buf.rfind(L"\n", _index); // find start of line
		if(sol == std::wstring::npos)
		{
			sol = 0;
		}
		_eol = _index + 1;
		if(_eol <= sol || _buf.find_first_not_of(L" \t", sol, _eol) == std::wstring::npos)
		{
			_cno = 0;
			_flags = CS_EMP;
		}
		else
		{
			_flags = CS_NONE;
			_cno = _eol - 1;
		}
		break;
	case CS_EOF:				/* EOF: get previous char. */
	case CS_NONE:
		if(_cno == 0)
		{
			if(_index == 0)
			{
				//TODO: acquire preceding text after moving cursor
				_flags = CS_SOF;
			}
			else
			{
				--_index;
				_flags = CS_EOL;
			}
		}
		else
		{
			--_cno;
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
