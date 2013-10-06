#ifndef VIKEYHANDLER_H
#define VIKEYHANDLER_H

#include "mozc/win32/base/deleter.h"
#include "ViCmd.h"

class CTextService;

class ViKeyHandler
{
public:
	ViKeyHandler(CTextService *textService);
	~ViKeyHandler();

	void Reset();
	BOOL IsWaitingNextKey();
	HRESULT HandleKey(TfEditCookie ec, ITfContext *pContext, WCHAR ch);

private:
	void _HandleFunc(TfEditCookie ec, ITfContext *pContext, WCHAR ch);
	void _QueueKey(std::vector<INPUT> *inputs, UINT vk, int count = 1);
	void _QueueKeyForSelection(std::vector<INPUT> *inputs);
	void _QueueKeyForModifier(std::vector<INPUT> *inputs, UINT vk, BOOL up);
	void _QueueKeyWithControl(std::vector<INPUT> *inputs, UINT vk);
	void _SendInputs(std::vector<INPUT> *inputs);
	void _SendKey(UINT vk, int count = 1);
	void _SendKeyWithControl(UINT vk);
	void _ViOpOrMove(UINT vk, int count);
	void _Vi_o();
	void _Vi_p(ITfContext *pContext);
	void _Vi_P();
	void _ViNextSentence(ITfContext *pContext);
	void _Vi_f(ITfContext *pContext, WCHAR ch);

	CTextService *_textService;
	ViCmd vicmd;
	mozc::win32::VKBackBasedDeleter deleter;
	std::unique_ptr<mozc::win32::Win32KeyboardInterface> keyboard_;
};
#endif //VIKEYHANDLER_H
