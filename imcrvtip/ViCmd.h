#ifndef VICMD_H
#define VICMD_H

class ViCmd
{
public:
	ViCmd();
	~ViCmd();

	void Reset();
	WCHAR GetOperatorPending();
	void SetOperatorPending(WCHAR op);
	WCHAR GetCharWaiting();
	void SetCharWaiting(WCHAR cmdch);
	int GetCount();
	BOOL HasCount();
	void AddCountChar(WCHAR ch);

private:
	WCHAR _operator_pending;		//operator-pending mode(c,d,y)
	WCHAR _char_waiting;		//waiting character(f,t)
	int _count1; //first count for operator or motion
	int _count2; //second count for motion
};
#endif //VICMD_H
