#include "ViCharStream.h"

ViCharStream::ViCharStream(const std::wstring &buf)
	: _buf(buf), _cno(0), _index(0), _flags(CS_NONE)
{
	_len = _buf.find(L"\n");
	if(_len == std::wstring::npos)
	{
		_len = _buf.size();
	}
	if(_len == 0 || _buf.find_first_not_of(L" \t", 0, _len) == std::wstring::npos)
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

int ViCharStream::fspace()
{
	//TODO
	return 1;
}

//Retrieve the next character.
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
			_len = _buf.find(L"\n", _index);
			if(_len == std::wstring::npos)
			{
				_len = _buf.size() - _index;
			}
			if(_len == 0 || _buf.find_first_not_of(L" \t", _index, _len) == std::wstring::npos)
			{
				_flags = CS_EMP;
			}
		}
		break;
	case CS_NONE:
		_index++;
		if(_cno == _len - 1)
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

int ViCharStream::prev()
{
	//TODO
	return 1;
}
