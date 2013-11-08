#ifndef VIKEYHANDLER_H
#define VIKEYHANDLER_H

#include "mozc/win32/base/keyboard.h"
#include "ViCmd.h"

class CTextService;

class ViKeyHandler
{
public:
	ViKeyHandler(CTextService *textService);
	~ViKeyHandler();

	void Reset();
	void ResetThroughSelfSentKey();
	BOOL IsThroughSelfSentKey();
	BOOL IsWaitingNextKey();
	HRESULT HandleKey(TfEditCookie ec, ITfContext *pContext, WCHAR ch, BYTE vk);

private:
	void _HandleFunc(TfEditCookie ec, ITfContext *pContext, WCHAR ch);
	void _SendInputs(std::vector<INPUT> *inputs);
	void _SendKey(UINT vk, int count = 1);
	void _SendKeyWithControl(UINT vk);
	void _ViOp(std::vector<INPUT> *inputs);
	void _ViOpOrMove(UINT vk, int count);
	void _ViOpLines(int count);
	void _Vi_j();
	void _ViDownFNB(ITfContext *pContext);
	void _Vi_k();
	void _Vi_i();
	void _Vi_I(ITfContext *pContext);
	BOOL _AtEndOfLine(ITfContext *pContext);
	void _Vi_a(ITfContext *pContext);
	void _Vi_A();
	void _Vi_o();
	void _Vi_O();
	void _Vi_p(ITfContext *pContext);
	void _Vi_P();
	void _ViNextWord(ITfContext *pContext, WCHAR type);
	void _ViNextWordE(ITfContext *pContext, WCHAR type);
	void _ViPrevWord(ITfContext *pContext, WCHAR type);
	void _VimForwardSent(ITfContext *pContext);
	void _VimBackwardSent(ITfContext *pContext);
	int _Vi_f_sub(ITfContext *pContext, WCHAR ch);
	void _Vi_f(ITfContext *pContext, WCHAR ch);
	void _Vi_t(ITfContext *pContext, WCHAR ch);
	int _Vi_F_sub(ITfContext *pContext, WCHAR ch);
	void _Vi_F(ITfContext *pContext, WCHAR ch);
	void _Vi_T(ITfContext *pContext, WCHAR ch);
	void _ViEndOfLine(ITfContext *pContext);
	void _Vi_gg();
	void _Vi_G();
	void _Vi_J(ITfContext *pContext);
	void _Vi_r(BYTE vk);

	CTextService *_textService;
	ViCmd vicmd;
	std::unique_ptr<mozc::win32::Win32KeyboardInterface> keyboard_;
	BOOL isThroughSelfSentKey;
};
#endif //VIKEYHANDLER_H
