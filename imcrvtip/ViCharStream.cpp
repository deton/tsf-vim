#include "ViCharStream.h"

ViCharStream::ViCharStream(const std::wstring &buf)
	: _buf(buf), _index(0), _sol(0), _flags(CS_NONE)
{
	remove(_buf.begin(), _buf.end(), L'\r');
	_eol = _buf.find(L"\n");
	if(_eol == std::wstring::npos)
	{
		_eol = _buf.size();
	}
	if(_eol == 0 || _buf.find_first_not_of(L" \t") >= _eol)
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
			_sol = _index;
			_eol = _buf.find(L"\n", _sol);
			if(_eol == std::wstring::npos)
			{
				_eol = _buf.size();
			}
			if(_eol == _sol || _buf.find_first_not_of(L" \t", _sol) >= _eol)
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
		//TODO: skip to \n
	case CS_EOL:				/* EOL; get previous line. */
		_eol = _index;
		--_index;
		if(_index <= 0)		/* SOF. */
		{
			//TODO: acquire preceding text after moving cursor
			_index = 0;
			_flags = CS_SOF;
			break;
		}
		// find start of line
		_sol = _buf.rfind(L"\n", _index);
		if(_sol == std::wstring::npos)
		{
			_sol = 0;
		}
		else
		{
			_sol++;
		}
		if(_buf.find_first_not_of(L" \t", _sol) >= _eol)
		{
			_flags = CS_EMP;
		}
		else
		{
			_flags = CS_NONE;
		}
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
				--_index;
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
