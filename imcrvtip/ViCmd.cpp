#include "ViCmd.h"

ViCmd::ViCmd(): _operator_pending(0), _char_waiting(0), _count1(0), _count2(0)
{
}

ViCmd::~ViCmd()
{
}

void ViCmd::Reset()
{
	SetOperatorPending(0);
	SetCharWaiting(0);
	_count1 = 0;
	_count2 = 0;
}

WCHAR ViCmd::GetOperatorPending()
{
	return _operator_pending;
}

void ViCmd::SetOperatorPending(WCHAR op)
{
	_operator_pending = op;
}

WCHAR ViCmd::GetCharWaiting()
{
	return _char_waiting;
}

void ViCmd::SetCharWaiting(WCHAR cmdch)
{
	_char_waiting = cmdch;
}

int ViCmd::GetCount()
{
	return ((_count1 == 0) ? 1 : _count1) * ((_count2 == 0) ? 1 : _count2);
}

BOOL ViCmd::HasCount()
{
	return (_count1 != 0 || _count2 != 0);
}

void ViCmd::AddCountChar(WCHAR ch)
{
	if (GetOperatorPending())
	{
		_count2 = _count2 * 10 + ch - L'0';
	}
	else
	{
		_count1 = _count1 * 10 + ch - L'0';
	}
}

BOOL ViCmd::IsEmpty()
{
	return (GetOperatorPending() == 0 && GetCharWaiting() == 0 && !HasCount());
}
