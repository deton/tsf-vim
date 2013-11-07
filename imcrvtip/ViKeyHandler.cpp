
#include "imcrvtip.h"
#include "TextService.h"
#include "ViKeyHandler.h"
#include "ViCharStream.h"
#include "ViMulti.h"
#include "ViUtil.h"
#include "VimCharStream.h"
#include "VimMByte.h"
#include "mozc/win32/tip/tip_surrounding_text.h"
#include "mozc/win32/base/keyboard.h"

static int inword(WCHAR c)
{
	return isalnum(c) || c == '_';
}

ViKeyHandler::ViKeyHandler(CTextService *textService)
	: _textService(textService),
	keyboard_(mozc::win32::Win32KeyboardInterface::CreateDefault()),
	isThroughSelfSentKey(FALSE)
{
}

ViKeyHandler::~ViKeyHandler()
{
}

void ViKeyHandler::Reset()
{
	vicmd.Reset();
}

void ViKeyHandler::ResetThroughSelfSentKey()
{
	isThroughSelfSentKey = FALSE;
}

BOOL ViKeyHandler::IsThroughSelfSentKey()
{
	return isThroughSelfSentKey;
}

BOOL ViKeyHandler::IsWaitingNextKey()
{
	return !vicmd.IsEmpty();
}

HRESULT ViKeyHandler::HandleKey(TfEditCookie ec, ITfContext *pContext, WCHAR ch, BYTE vk)
{
	WCHAR waiting = vicmd.GetCharWaiting();
	if (waiting)
	{
		switch (waiting)
		{
		case L'f':
			_Vi_f(pContext, ch);
			break;
		case L't':
			_Vi_t(pContext, ch);
			break;
		case L'F':
			_Vi_F(pContext, ch);
			break;
		case L'T':
			_Vi_T(pContext, ch);
			break;
		case L'r':
			_Vi_r(vk);
			break;
		case L'g':
			switch (ch)
			{
			case L'g': // gg
				_Vi_gg();
				break;
			case L'$':
				_ViOpOrMove(VK_END, 1); // TODO: support count
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		vicmd.Reset();
	}
	else
	{
		_HandleFunc(ec, pContext, ch);
	}
	return S_OK;
}

void ViKeyHandler::_HandleFunc(TfEditCookie ec, ITfContext *pContext, WCHAR ch)
{
	switch (ch)
	{
	case L'0':
		if (!vicmd.HasCount())
		{
			_ViOpOrMove(VK_HOME, 1);
		}
		else
		{
			vicmd.AddCountChar(ch);
		}
		return;
	case L'1':
	case L'2':
	case L'3':
	case L'4':
	case L'5':
	case L'6':
	case L'7':
	case L'8':
	case L'9':
		vicmd.AddCountChar(ch);
		return;
	case L'c':
	case L'd':
	case L'y':
		if (vicmd.GetOperatorPending())
		{
			if (vicmd.GetOperatorPending() == ch) // 'cc','dd','yy'
			{
				_ViOpLines(vicmd.GetCount() - 1);
			}
			else
			{
				vicmd.SetOperatorPending(0);
			}
		}
		else
		{
			vicmd.SetOperatorPending(ch);
		}
		return;
	case L'C':
	case L'D':
		vicmd.SetOperatorPending(towlower(ch));
		_ViEndOfLine(pContext);
		return;
	case L'f':
	case L't':
	case L'F':
	case L'T':
	case L'g':
	case L'r':
		vicmd.SetCharWaiting(ch);
		return;
	case L'G':
		_Vi_G();
		return;
	case CTRL('F'):
		_SendKey(VK_NEXT, vicmd.GetCount());
		vicmd.Reset();
		return;
	case L'/':
		_Vi_slash();
		return;
	case CTRL('B'):
		_SendKey(VK_PRIOR, vicmd.GetCount());
		vicmd.Reset();
		return;
	case L'+':
	case CTRL('M'):
		_ViDownFNB(pContext); // first non-blank
		return;
	case L'$':
		_ViEndOfLine(pContext);
		return;
	case L'h':
		_ViOpOrMove(VK_LEFT, vicmd.GetCount());
		return;
	case L'j':
		_Vi_j();
		return;
	case L'k':
		_Vi_k();
		return;
	case L'l':
	case L' ':
		_ViOpOrMove(VK_RIGHT, vicmd.GetCount());
		return;
	case L'i':
		_Vi_i();
		return;
	case L'I':
		_Vi_I(pContext);
		return;
	case L'a':
		_Vi_a(pContext);
		return;
	case L'A':
		_Vi_A();
		return;
	case L'o':
		_Vi_o();
		return;
	case L'O':
		_Vi_O();
		return;
	case L'p':
		_Vi_p(pContext);
		return;
	case L'P': // paste at caret
		_Vi_P();
		return;
	case L'u':
		vicmd.Reset();
		_SendKeyWithControl('Z');
		return;
	case L'x':
		vicmd.SetOperatorPending('d'); // cut to clipboard
		_ViOpOrMove(VK_RIGHT, vicmd.GetCount());
		return;
	case L'X':
		vicmd.SetOperatorPending('d'); // cut to clipboard
		_ViOpOrMove(VK_LEFT, vicmd.GetCount());
		return;
	case L's': // same as 'cl'
		vicmd.SetOperatorPending('c');
		_ViOpOrMove(VK_RIGHT, vicmd.GetCount());
		return;
	case L'w':
	case L'W':
		_ViNextWord(pContext, ch);
		vicmd.Reset();
		return;
	case L'e':
	case L'E':
		_ViNextWordE(pContext, ch);
		vicmd.Reset();
		return;
	case L'b':
	case L'B':
		_ViPrevWord(pContext, ch);
		vicmd.Reset();
		return;
	case L')':
		_VimForwardSent(pContext);
		vicmd.Reset();
		return;
	case L'(':
		_VimBackwardSent(pContext);
		vicmd.Reset();
		return;
	case L'J':
		_Vi_J(pContext);
		return;
	default:
		vicmd.Reset();
		break;
	}
}

static bool isextendedkey(UINT vk)
{
	// http://stackoverflow.com/questions/9233679/sendinput-doesnt-send-chars-or-numbers
	return vk >= VK_PRIOR && vk <= VK_DELETE || vk >= VK_LWIN && vk <= VK_APPS;
}

static void _QueueKey(vector<INPUT> *inputs, UINT vk, int count = 1)
{
	const KEYBDINPUT keyboard_input = {vk, 0, 0, 0, 0};
	INPUT keydown = {};
	keydown.type = INPUT_KEYBOARD;
	keydown.ki = keyboard_input;

	INPUT keyup = keydown;
	keyup.type = INPUT_KEYBOARD;
	keyup.ki.dwFlags = KEYEVENTF_KEYUP;

	if (isextendedkey(vk))
	{
		keydown.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		keyup.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
	}

	for (int i = 0; i < count; ++i)
	{
		inputs->push_back(keydown);
		inputs->push_back(keyup);
	}
}

static void _QueueKeyForSelection(vector<INPUT> *inputs)
{
	const KEYBDINPUT keyboard_input = {VK_SHIFT, 0, 0, 0, 0};
	INPUT keydown = {};
	keydown.type = INPUT_KEYBOARD;
	keydown.ki = keyboard_input;

	INPUT keyup = keydown;
	keyup.type = INPUT_KEYBOARD;
	keyup.ki.dwFlags = KEYEVENTF_KEYUP;

	inputs->insert(inputs->begin(), keydown);
	inputs->push_back(keyup);
}

static void _QueueKeyForModifier(vector<INPUT> *inputs, UINT vk, BOOL up, BOOL front = FALSE)
{
	const KEYBDINPUT keyboard_input = {vk, 0, 0, 0, 0};
	INPUT keydown = {};
	keydown.type = INPUT_KEYBOARD;
	keydown.ki = keyboard_input;

	if (up)
	{
		INPUT keyup = keydown;
		keyup.type = INPUT_KEYBOARD;
		keyup.ki.dwFlags = KEYEVENTF_KEYUP;
		if (front)
		{
			inputs->insert(inputs->begin(), keyup);
		}
		else
		{
			inputs->push_back(keyup);
		}
	}
	else
	{
		if (front)
		{
			inputs->insert(inputs->begin(), keydown);
		}
		else
		{
			inputs->push_back(keydown);
		}
	}
}

static void _QueueKeyWithControl(vector<INPUT> *inputs, UINT vk)
{
	_QueueKeyForModifier(inputs, VK_CONTROL, FALSE);
	_QueueKey(inputs, vk);
	_QueueKeyForModifier(inputs, VK_CONTROL, TRUE);
}

void ViKeyHandler::_SendInputs(vector<INPUT> *inputs)
{
	// cf. deleter.UnsetModifiers()
	mozc::win32::KeyboardStatus keyboard_state;
	bool shiftPressed = false;
	bool controlPressed = false;
	if (keyboard_->GetKeyboardState(&keyboard_state))
	{
		const BYTE kUnsetState = 0;
		bool to_be_updated = false;
		if (keyboard_state.IsPressed(VK_SHIFT))
		{
			shiftPressed = true;
			to_be_updated = true;
			keyboard_state.SetState(VK_SHIFT, kUnsetState);
			_QueueKeyForModifier(inputs, VK_SHIFT, TRUE, TRUE);
			// restore modifier
			// XXX:SendInput()直後にdeleter.EndDeletion()を呼んでも、
			// CTRL-F押下時にVK_NEXT送り付けても動かない
			// (おそらくCTRL押下状態になってCTRL-NEXTになるため)
			_QueueKeyForModifier(inputs, VK_SHIFT, FALSE);
		}
		if (keyboard_state.IsPressed(VK_CONTROL))
		{
			controlPressed = true;
			to_be_updated = true;
			keyboard_state.SetState(VK_CONTROL, kUnsetState);
			_QueueKeyForModifier(inputs, VK_CONTROL, TRUE, TRUE);
			_QueueKeyForModifier(inputs, VK_CONTROL, FALSE);
		}
		if (to_be_updated)
		{
			keyboard_->SetKeyboardState(keyboard_state);
		}
	}

	keyboard_->SendInput(*inputs);
}

void ViKeyHandler::_SendKey(UINT vk, int count)
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, vk, count);
	_SendInputs(&inputs);
}

void ViKeyHandler::_SendKeyWithControl(UINT vk)
{
	vector<INPUT> inputs;
	_QueueKeyWithControl(&inputs, vk);
	_SendInputs(&inputs);
}

static void _QueueKeyForOtherIME(vector<INPUT> *inputs)
{
	// switch to other IME
	if (IsVersion62AndOver()) // Win+Space for Windows 8
	{
		_QueueKeyForModifier(inputs, VK_LWIN, FALSE);
		_QueueKey(inputs, VK_SPACE);
		_QueueKeyForModifier(inputs, VK_LWIN, TRUE);
	}
	else // Alt+Shift for Windows 7
	{
		_QueueKeyForModifier(inputs, VK_MENU, FALSE);
		_QueueKey(inputs, VK_SHIFT);
		_QueueKeyForModifier(inputs, VK_MENU, TRUE);
	}
}

void ViKeyHandler::_ViOp(vector<INPUT> *inputs)
{
	switch (vicmd.GetOperatorPending())
	{
	case L'c':
		_QueueKeyWithControl(inputs, 'X');
		_QueueKeyForOtherIME(inputs);
		_SendInputs(inputs);
		_textService->_SetKeyboardOpen(FALSE);
		break;
	case L'd':
		_QueueKeyWithControl(inputs, 'X');
		_SendInputs(inputs);
		break;
	case L'y':
		_QueueKeyWithControl(inputs, 'C');
		_SendInputs(inputs);
		break;
	default:
		_SendInputs(inputs);
		break;
	}
	vicmd.Reset();
}

void ViKeyHandler::_ViOpOrMove(UINT vk, int count)
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, vk, count);
	if (vicmd.GetOperatorPending())
	{
		_QueueKeyForSelection(&inputs);
	}
	_ViOp(&inputs);
}

void ViKeyHandler::_ViOpLines(int count)
{
	vector<INPUT> inputs;
	// FIXME: 最終行にいる際に'dd'すると1つ上の行まで削除される
	for (int i = 0; i < count; ++i)
	{
		_QueueKey(&inputs, VK_DOWN);
	}
	// same as _Vi_k()
	// Wordの場合、選択中に既に行末にいる際にVK_ENDを送ると、
	// 次の行末まで移動するので
	_QueueKey(&inputs, VK_END);
	_QueueKey(&inputs, VK_RIGHT); // to include '\n'
	_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
	for (int i = count + 1; i > 0; --i)
	{
		_QueueKey(&inputs, VK_UP);
	}
	_QueueKeyForModifier(&inputs, VK_SHIFT, TRUE);
	_ViOp(&inputs);
}

void ViKeyHandler::_Vi_j()
{
	if (vicmd.GetOperatorPending()) // linewise operator
	{
		_ViOpLines(vicmd.GetCount());
	}
	else
	{
		_ViOpOrMove(VK_DOWN, vicmd.GetCount());
	}
}

// down and first non-blank
void ViKeyHandler::_ViDownFNB(ITfContext *pContext)
{
	if (vicmd.GetOperatorPending()) // linewise operator
	{
		_ViOpLines(vicmd.GetCount());
		return;
	}

	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return;
	}

	int cnt = vicmd.GetCount();
	while (cnt--)
	{
		for (int r = pos.inc(); r != 1; r = pos.inc())
		{
			if (r == -1) // end of file
			{
				pos.restore_index();
				goto end;
			}
		}
		if (pos.fblank() == -1)
		{
			goto end;
		}
		pos.save_index();
	}
end:
	int movecnt = pos.difference();
	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_Vi_k()
{
	if (vicmd.GetOperatorPending()) // linewise operator
	{
		vector<INPUT> inputs;
		_QueueKey(&inputs, VK_END);
		_QueueKey(&inputs, VK_RIGHT); // to include '\n'
		_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
		for (int count = vicmd.GetCount() + 1; count > 0; --count)
		{
			_QueueKey(&inputs, VK_UP);
		}
		_QueueKeyForModifier(&inputs, VK_SHIFT, TRUE);
		_ViOp(&inputs);
	}
	else
	{
		_ViOpOrMove(VK_UP, vicmd.GetCount());
	}
}

void ViKeyHandler::_Vi_I(ITfContext *pContext)
{
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return;
	}

	// go to start of line
	for (int r = pos.dec(); r != -1; r = pos.dec())
	{
		if (pos.gchar() == L'\n')
		{
			pos.inc();
			break;
		}
	}
	pos.fblank();

	vector<INPUT> inputs;
	int diff = pos.difference();
	if (diff < 0)
	{
		_QueueKey(&inputs, VK_LEFT, -diff);
	}
	else if (diff > 0)
	{
		_QueueKey(&inputs, VK_RIGHT, diff);
	}
	_QueueKeyForOtherIME(&inputs);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

BOOL ViKeyHandler::_AtEndOfLine(ITfContext *pContext)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if (mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		if (info.following_text.size() == 0
				|| info.following_text[0] == L'\r'
				|| info.following_text[0] == L'\n')
		{
			return TRUE;
		}
	}
	return FALSE;
}

void ViKeyHandler::_Vi_i()
{
	vector<INPUT> inputs;
	_QueueKeyForOtherIME(&inputs);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_a(ITfContext *pContext)
{
	vector<INPUT> inputs;
	// 行末にいる場合にVK_RIGHTを送り付けると次行に移動するので
	if (!_AtEndOfLine(pContext))
	{
		_QueueKey(&inputs, VK_RIGHT);
	}
	_QueueKeyForOtherIME(&inputs);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_A()
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_END);
	_QueueKeyForOtherIME(&inputs);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_o()
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_END);
	_QueueKey(&inputs, VK_RETURN);
	_QueueKeyForOtherIME(&inputs);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_O()
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_HOME);
	_QueueKey(&inputs, VK_RETURN);
	_QueueKey(&inputs, VK_UP);
	_QueueKeyForOtherIME(&inputs);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

static BOOL IsLinewiseClipboard()
{
	if (!::OpenClipboard(NULL))
	{
		return FALSE;
	}
	BOOL ret = FALSE;
#define CHECKNEWLINE(fmt, type, pred) \
	do { \
		HGLOBAL hMem; \
		if ((hMem = ::GetClipboardData(fmt)) != NULL) \
		{ \
			type *hMemStr = (type *)GlobalLock(hMem); \
			if (pred) \
			{ \
				ret = TRUE; \
			} \
			GlobalUnlock(hMem); \
		} \
	} while (0)

	if (::IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		CHECKNEWLINE(CF_UNICODETEXT, WCHAR, wcschr(hMemStr, L'\n') != NULL);
	}
	else if (::IsClipboardFormatAvailable(CF_TEXT))
	{
		CHECKNEWLINE(CF_TEXT, char, strchr(hMemStr, '\n') != NULL);
	}
	::CloseClipboard();
	return ret;
}

void ViKeyHandler::_Vi_p(ITfContext *pContext)
{
	vector<INPUT> inputs;
	// 行指向の場合は、次行にペースト
	if (IsLinewiseClipboard())
	{
		// TODO: 既に最終行にいる場合、新しい行を作成してペースト
		_QueueKey(&inputs, VK_END);
		_QueueKey(&inputs, VK_RIGHT);
	}
	else
	{
		if (!_AtEndOfLine(pContext))
		{
			_QueueKey(&inputs, VK_RIGHT);
		}
	}
	_QueueKeyForModifier(&inputs, VK_CONTROL, FALSE);
	_QueueKey(&inputs, 'V', vicmd.GetCount());
	_QueueKeyForModifier(&inputs, VK_CONTROL, TRUE);
	_SendInputs(&inputs);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_P()
{
	vector<INPUT> inputs;
	if (IsLinewiseClipboard())
	{
		_QueueKey(&inputs, VK_HOME);
	}
	_QueueKeyForModifier(&inputs, VK_CONTROL, FALSE);
	_QueueKey(&inputs, 'V', vicmd.GetCount());
	_QueueKeyForModifier(&inputs, VK_CONTROL, TRUE);
	_SendInputs(&inputs);
	vicmd.Reset();
}

#define RETURN_IF_FAIL(movefunc) if (movefunc()) return
#define CS_PREV() RETURN_IF_FAIL(cs.prev)
#define CS_NEXT() RETURN_IF_FAIL(cs.next)
#define CS_FSPACE() RETURN_IF_FAIL(cs.fspace)
#define CS_FBLANK() RETURN_IF_FAIL(cs.fblank)
#define CS_BBLANK() RETURN_IF_FAIL(cs.bblank)

void ViKeyHandler::_ViNextWord(ITfContext *pContext, WCHAR type)
{
	ViCharStream cs(_textService, pContext);
	if (cs.flags() == cs.CS_EOF)
	{
		return;
	}

	int cnt = vicmd.GetCount();
	// cf. fword() in v_word.c of nvi-1.79
	/*
	 * If in white-space:
	 *	If the count is 1, and it's a change command, we're done.
	 *	Else, move to the first non-white-space character, which
	 *	counts as a single word move.  If it's a motion command,
	 *	don't move off the end of the line.
	 */
	if (cs.flags() == cs.CS_EMP || cs.flags() == cs.CS_NONE && iswblank(cs.ch()))
	{
		if (vicmd.GetOperatorPending() && cs.flags() != cs.CS_EMP && cnt == 1)
		{
			if (vicmd.GetOperatorPending() == L'c')
			{
				return;
			}
			if (vicmd.GetOperatorPending() == L'd' || vicmd.GetOperatorPending() == L'y')
			{
				CS_FSPACE();
				goto ret;
			}
		}
		CS_FBLANK();
		--cnt;
	}

	/*
	 * Cyclically move to the next word -- this involves skipping
	 * over word characters and then any trailing non-word characters.
	 * Note, for the 'w' command, the definition of a word keeps
	 * switching.
	 */
	if (type == L'W')
	{
		while (cnt--)
		{
			if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
			{
				goto Singlebyte;
			}
			ViMulti::ChClass ochclass = ViMulti::chclass(cs.ch(), ViMulti::_INIT);
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
				{
					goto Taileater;
				}
				ViMulti::ChClass chclass = ViMulti::chclass(cs.ch(), ochclass);
				if (ViMulti::Wordbound(ochclass, chclass, true))
				{
					goto Taileater;
				}
				ochclass = chclass;
			}
Singlebyte:
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if (ViMulti::ismulti(cs.ch()))
				{
					break;
				}
			}
Taileater:
			/*
			 * If a motion command and we're at the end of the
			 * last word, we're done.  Delete and yank eat any
			 * trailing blanks, but we don't move off the end
			 * of the line regardless.
			 */
			if (cnt == 0 && vicmd.GetOperatorPending())
			{
				if (vicmd.GetOperatorPending() == L'd' || vicmd.GetOperatorPending() == L'y')
				{
					CS_FSPACE();
					break;
				}
			}

			/* Eat whitespace characters. */
			if (cs.flags() == cs.CS_NONE && ViMulti::ismulti(cs.ch()))
			{
				continue;
			}
			if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
			{
				CS_FBLANK();
			}
			if (cs.flags() == cs.CS_EOF)
			{
				goto ret;
			}
		}
	}
	else // 'w'
	{
		while (cnt--)
		{
			if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
			{
				goto singlebyte;
			}
			ViMulti::ChClass ochclass = ViMulti::chclass(cs.ch(), ViMulti::_INIT);
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
				{
					goto taileater;
				}
				ViMulti::ChClass chclass = ViMulti::chclass(cs.ch(), ochclass);
				if (ViMulti::wordbound(ochclass, chclass, true))
				{
					goto taileater;
				}
				ochclass = chclass;
			}
singlebyte:
			enum { INWORD, NOTWORD } state;
			state = cs.flags() == cs.CS_NONE && inword(cs.ch()) ? INWORD : NOTWORD;
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if (ViMulti::ismulti(cs.ch()))
				{
					break;
				}
				if (state == INWORD)
				{
					if (!inword(cs.ch()))
					{
						break;
					}
				}
				else
				{
					if (inword(cs.ch()))
					{
						break;
					}
				}
			}
taileater:
			/* See comment above. */
			if (cnt == 0 && vicmd.GetOperatorPending())
			{
				if (vicmd.GetOperatorPending() == L'd' || vicmd.GetOperatorPending() == L'y')
				{
					CS_FSPACE();
					break;
				}
			}

			/* Eat whitespace characters. */
			if (cs.flags() == cs.CS_NONE && ViMulti::ismulti(cs.ch()))
			{
				continue;
			}
			if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
			{
				CS_FBLANK();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
			}
		}
	}

	/*
	 * If we didn't move, we must be at EOF.
	 *
	 * !!!
	 * That's okay for motion commands, however.
	 */
ret:
	int movecnt = cs.difference();
	if (!vicmd.GetOperatorPending() && movecnt == 0)
	{
		return;
	}

	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_ViNextWordE(ITfContext *pContext, WCHAR type)
{
	ViCharStream cs(_textService, pContext);
	if (cs.flags() == cs.CS_EOF)
	{
		return;
	}

	int cnt = vicmd.GetCount();
	// cf. eword() in v_word.c of nvi-1.79
	/*
	 * !!!
	 * If in whitespace, or the next character is whitespace, move past
	 * it.  (This doesn't count as a word move.)  Stay at the character
	 * past the current one, it sets word "state" for the 'e' command.
	 */
	if (cs.flags() == cs.CS_NONE && !iswblank(cs.ch()))
	{
		CS_NEXT();
		if (cs.flags() == cs.CS_NONE && !iswblank(cs.ch()))
		{
			goto start;
		}
	}
	CS_FBLANK();

	/*
	 * Cyclically move to the next word -- this involves skipping
	 * over word characters and then any trailing non-word characters.
	 * Note, for the 'e' command, the definition of a word keeps
	 * switching.
	 */
start:
	if (type == 'E')
	{
		while (cnt--)
		{
			if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
			{
				goto Singlebyte;
			}
			ViMulti::ChClass ochclass = ViMulti::chclass(cs.ch(), ViMulti::_INIT);
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
				{
					goto Taileater;
				}
				ViMulti::ChClass chclass = ViMulti::chclass(cs.ch(), ochclass);
				if (ViMulti::Wordbound(ochclass, chclass, true))
				{
					goto Taileater;
				}
				ochclass = chclass;
			}
Singlebyte:
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if (ViMulti::ismulti(cs.ch()))
				{
					break;
				}
			}
Taileater:
			/*
			 * When we reach the start of the word after the last
			 * word, we're done.  If we changed state, back up one
			 * to the end of the previous word.
			 */
			if (cnt == 0)
			{
				if (cs.flags() == cs.CS_NONE)
				{
					CS_PREV();
				}
				break;
			}

			/* Eat whitespace characters. */
			if (cs.flags() == cs.CS_NONE && ViMulti::ismulti(cs.ch()))
			{
				continue;
			}
			if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
			{
				CS_FBLANK();
			}
			if (cs.flags() == cs.CS_EOF)
			{
				goto ret;
			}
		}
	}
	else // 'e'
	{
		while (cnt--)
		{
			if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
			{
				goto singlebyte;
			}
			ViMulti::ChClass ochclass = ViMulti::chclass(cs.ch(), ViMulti::_INIT);
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
				{
					goto taileater;
				}
				ViMulti::ChClass chclass = ViMulti::chclass(cs.ch(), ochclass);
				if (ViMulti::wordbound(ochclass, chclass, true))
				{
					goto taileater;
				}
				ochclass = chclass;
			}
singlebyte:
			enum { INWORD, NOTWORD } state;
			state = cs.flags() == cs.CS_NONE && inword(cs.ch()) ? INWORD : NOTWORD;
			for (;;)
			{
				CS_NEXT();
				if (cs.flags() == cs.CS_EOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if (ViMulti::ismulti(cs.ch()))
				{
					break;
				}
				if (state == INWORD)
				{
					if (!inword(cs.ch()))
					{
						break;
					}
				}
				else
				{
					if (inword(cs.ch()))
					{
						break;
					}
				}
			}
taileater:
			/* See comment above. */
			if (cnt==0)
			{
				if (cs.flags() == cs.CS_NONE)
				{
					CS_PREV();
				}
				break;
			}

			/* Eat whitespace characters. */
			if (cs.flags() == cs.CS_NONE && ViMulti::ismulti(cs.ch()))
			{
				continue;
			}
			if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
			{
				CS_FBLANK();
			}
			if (cs.flags() == cs.CS_EOF)
			{
				goto ret;
			}
		}
	}

	/*
	 * If we didn't move, we must be at EOF.
	 *
	 * !!!
	 * That's okay for motion commands, however.
	 */
ret:
	int movecnt = cs.difference();
	if (!vicmd.GetOperatorPending() && movecnt == 0)
	{
		return;
	}

	if (vicmd.GetOperatorPending() && cs.flags() == cs.CS_NONE)
	{
		movecnt++;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_ViPrevWord(ITfContext *pContext, WCHAR type)
{
	ViCharStream cs(_textService, pContext);
	if (cs.flags() == cs.CS_EOF)
	{
		return;
	}

	int cnt = vicmd.GetCount();
	// cf. bword() in v_word.c of nvi-1.79
	/*
	 * !!!
	 * If in whitespace, or the previous character is whitespace, move
	 * past it.  (This doesn't count as a word move.)  Stay at the
	 * character before the current one, it sets word "state" for the
	 * 'b' command.
	 */
	if (cs.flags() == cs.CS_NONE && !iswblank(cs.ch()))
	{
		CS_PREV();
		if (cs.flags() == cs.CS_NONE && !iswblank(cs.ch()))
		{
			goto start;
		}
	}
	CS_BBLANK();

	/*
	 * Cyclically move to the beginning of the previous word -- this
	 * involves skipping over word characters and then any trailing
	 * non-word characters.  Note, for the 'b' command, the definition
	 * of a word keeps switching.
	 */
start:
	if (type == 'B')
	{
		while (cnt--)
		{
			if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
			{
				goto Singlebyte;
			}
			ViMulti::ChClass ochclass = ViMulti::chclass(cs.ch(), ViMulti::_INIT);
			for (;;)
			{
				CS_PREV();
				if (cs.flags() == cs.CS_SOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
				{
					goto Cntmodify;
				}
				ViMulti::ChClass chclass = ViMulti::chclass(cs.ch(), ochclass);
				if (ViMulti::Wordbound(ochclass, chclass, false))
				{
					goto Cntmodify;
				}
				ochclass = chclass;
			}
Singlebyte:
			for (;;)
			{
				CS_PREV();
				if (cs.flags() == cs.CS_SOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if (ViMulti::ismulti(cs.ch()))
				{
					break;
				}
			}
Cntmodify:
			/*
			 * When we reach the end of the word before the last
			 * word, we're done.  If we changed state, move forward
			 * one to the end of the next word.
			 */
			if (cnt == 0)
			{
				if (cs.flags() == cs.CS_NONE)
				{
					CS_NEXT();
				}
				break;
			}

			/* Eat whitespace characters. */
			if (cs.flags() == cs.CS_NONE
					&& (ViMulti::ismulti(cs.ch()) || !iswblank(cs.ch())))
			{
				continue;
			}
			CS_BBLANK();
			if (cs.flags() == cs.CS_SOF)
			{
				goto ret;
			}
		}
	}
	else // 'b'
	{
		while (cnt--)
		{
			if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
			{
				goto singlebyte;
			}
			ViMulti::ChClass ochclass = ViMulti::chclass(cs.ch(), ViMulti::_INIT);
			for (;;)
			{
				CS_PREV();
				if (cs.flags() == cs.CS_SOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || !ViMulti::ismulti(cs.ch()))
				{
					goto cntmodify;
				}
				ViMulti::ChClass chclass = ViMulti::chclass(cs.ch(), ochclass);
				if (ViMulti::wordbound(ochclass, chclass, false))
				{
					goto cntmodify;
				}
				ochclass = chclass;
			}
singlebyte:
			enum { INWORD, NOTWORD } state;
			state = cs.flags() == cs.CS_NONE && inword(cs.ch()) ? INWORD : NOTWORD;
			for (;;)
			{
				CS_PREV();
				if (cs.flags() == cs.CS_SOF)
				{
					goto ret;
				}
				if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if (ViMulti::ismulti(cs.ch()))
				{
					break;
				}
				if (state == INWORD)
				{
					if (!inword(cs.ch()))
					{
						break;
					}
				}
				else
				{
					if (inword(cs.ch()))
					{
						break;
					}
				}
			}
cntmodify:
			/* See comment above. */
			if (cnt==0)
			{
				if (cs.flags() == cs.CS_NONE)
				{
					CS_NEXT();
				}
				break;
			}

			/* Eat whitespace characters. */
			if (cs.flags() == cs.CS_NONE && ViMulti::ismulti(cs.ch()))
			{
				continue;
			}
			if (cs.flags() != cs.CS_NONE || iswblank(cs.ch()))
			{
				CS_BBLANK();
			}
			if (cs.flags() == cs.CS_SOF)
			{
				goto ret;
			}
		}
	}

	/* If we didn't move, we must be at SOF. */
ret:
	int movecnt = cs.difference();
	if (movecnt == 0)
	{
		return;
	}

	_ViOpOrMove(VK_LEFT, -movecnt);
}

static int issentend(wchar_t c)
{
	const std::wstring chars(L".!?)]\"'");
	return chars.find(c) != std::wstring::npos
		|| VimMByte::chclass(c) == VimMByte::PUNCT;
}

static int issentendex(wchar_t c)
{
	const std::wstring chars(L")]\"'");
	return chars.find(c) != std::wstring::npos;
}

static BOOL endsent(VimCharStream *pos)
{
	WCHAR c = pos->gchar();
	if (c == L'.' || c == L'!' || c == L'?')
	{
		pos->save_index();
		do {
			if (pos->inc() == -1)
			{
				return TRUE;
			}
		} while (issentendex(pos->gchar()));
		if (iswblank(pos->gchar()))
		{
			return TRUE;
		}
		else if (pos->gchar() == L'\n')
		{
			pos->inc();
			return TRUE;
		}
		pos->restore_index();
	}
	return FALSE;
}

static BOOL endmbsent(VimCharStream *pos)
{
	if (VimMByte::chclass(pos->gchar()) == VimMByte::PUNCT)
	{
		for (;;)
		{
			if (pos->inc() == -1)
			{
				return TRUE;
			}
			WCHAR c = pos->gchar();
			if (!VimMByte::ismulti(c) || VimMByte::chclass(c) != VimMByte::PUNCT)
			{
				if (c == L'\n')
				{
					pos->inc();
				}
				return TRUE;
			}
		}
	}
	return FALSE;
}

void ViKeyHandler::_VimForwardSent(ITfContext *pContext)
{
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return;
	}

	BOOL noskip = FALSE;
	int count = vicmd.GetCount();
	// cf. findsent() in search.c of vim.
	while (count--)
	{
		/*
		 * if on an empty line, skip upto a non-empty line
		 */
		if (pos.gchar() == L'\n')
		{
			do {
				if (pos.incl() == -1)
				{
					break;
				}
			} while (pos.gchar() == L'\n');
			goto found;
		}

		/* go back to the previous non-blank char */
		while (iswblank(pos.gchar()))
		{
			int r = pos.decl();
			if (r == -1)
			{
				break;
			}
			/* Stop in front of empty line */
			if (r == 2)
			{
				pos.incl();
				goto found;
			}
		}

		for (;;)		/* find end of sentence */
		{
			if (pos.gchar() == L'\n')
			{
				break;
			}
			if (endsent(&pos))
			{
				break;
			}
			if (endmbsent(&pos))
			{
				break;
			}
			if (pos.incl() == -1)
			{
				if (count)
				{
					return;
				}
				noskip = TRUE;
				break;
			}
		}
found:
		/* skip white space */
		if (!noskip)
		{
			pos.fblankl();
		}
	}

	int movecnt = pos.difference();
	if (pos.eof())
	{
		++movecnt;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_VimBackwardSent(ITfContext *pContext)
{
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return;
	}

	BOOL noskip = FALSE;
	int count = vicmd.GetCount();
	// cf. findsent() in search.c of vim.
	while (count--)
	{
		/*
		 * if on an empty line, skip upto a non-empty line
		 */
		if (pos.gchar() == L'\n')
		{
			do {
				if (pos.decl() == -1)
				{
					break;
				}
			} while (pos.gchar() == L'\n');
		}
		else
		{
			pos.decl();
		}

		/* go back to the previous non-blank char */
		BOOL found_dot = FALSE;
		while (iswblank(pos.gchar()) || issentend(pos.gchar()))
		{
			WCHAR c = pos.gchar();
			if (c == L'.' || c == L'!' || c == L'?')
			{
				/* Only skip over a '.', '!' and '?' once. */
				if (found_dot)
				{
					break;
				}
				found_dot = TRUE;
			}
			if (pos.decl() == -1)
			{
				break;
			}
		}

		/* remember the index where the search started */
		size_t startindex = pos.index();

		for (;;)		/* find end of sentence */
		{
			if (pos.gchar() == L'\n')
			{
				if (pos.index() != startindex)
				{
					pos.inc();
				}
				break;
			}
			if (endsent(&pos))
			{
				break;
			}
			if (endmbsent(&pos))
			{
				break;
			}
			if (pos.decl() == -1)
			{
				if (count)
				{
					return;
				}
				noskip = TRUE;
				break;
			}
		}
//found:
		/* skip white space */
		if (!noskip)
		{
			pos.fblankl();
		}
	}

	int movecnt = -pos.difference();
	_ViOpOrMove(VK_LEFT, movecnt);
}

// return -1: not found
int ViKeyHandler::_Vi_f_sub(ITfContext *pContext, WCHAR ch)
{
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return -1;
	}
	if (pos.gchar() == L'\n')
	{
		return -1; // empty line
	}

	int cnt = vicmd.GetCount();
	while (cnt--)
	{
		do {
			if (pos.inc() != 0)
			{
				return -1;
			}
		} while (pos.gchar() != ch);
	}
	return pos.difference();
}

//	Search forward in the line for the next occurrence of the
//	specified character.
void ViKeyHandler::_Vi_f(ITfContext *pContext, WCHAR ch)
{
	int movecnt = _Vi_f_sub(pContext, ch);
	if (movecnt <= 0)
	{
		return;
	}
	if (vicmd.GetOperatorPending())
	{
		movecnt++;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

//	Search forward in the line for the character before the next
//	occurrence of the specified character.
void ViKeyHandler::_Vi_t(ITfContext *pContext, WCHAR ch)
{
	int movecnt = _Vi_f_sub(pContext, ch);
	if (movecnt <= 0)
	{
		return;
	}
	if (!vicmd.GetOperatorPending())
	{
		movecnt--;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

int ViKeyHandler::_Vi_F_sub(ITfContext *pContext, WCHAR ch)
{
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return -1;
	}

	int cnt = vicmd.GetCount();
	while (cnt--)
	{
		do {
			if (pos.dec() != 0)
			{
				return -1;
			}
		} while (pos.gchar() != ch);
	}
	return -pos.difference();
}

//	Search backward in the line for the next occurrence of the
//	specified character.
void ViKeyHandler::_Vi_F(ITfContext *pContext, WCHAR ch)
{
	int movecnt = _Vi_F_sub(pContext, ch);
	if (movecnt <= 0)
	{
		return;
	}
	_ViOpOrMove(VK_LEFT, movecnt);
}

//	Search backward in the line for the character after the next
//	occurrence of the specified character.
void ViKeyHandler::_Vi_T(ITfContext *pContext, WCHAR ch)
{
	int movecnt = _Vi_F_sub(pContext, ch);
	if (movecnt <= 1)
	{
		return;
	}
	movecnt--;
	_ViOpOrMove(VK_LEFT, movecnt);
}

void ViKeyHandler::_ViEndOfLine(ITfContext *pContext)
{
	// TODO: support count
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return;
	}

	int r = pos.forward_eol();
	int movecnt = pos.difference();
	if (r == -1) // end of file
	{
		++movecnt;
	}
	if (movecnt <= 0)
	{
		return;
	}
	if (!vicmd.GetOperatorPending())
	{
		--movecnt;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

static void _QueueKeyToLine(vector<INPUT> *inputs, int lnum)
{
	_QueueKeyForModifier(inputs, VK_CONTROL, FALSE);
	_QueueKey(inputs, VK_HOME);
	_QueueKeyForModifier(inputs, VK_CONTROL, TRUE);
	// lnum - 1 lines DOWN
	_QueueKey(inputs, VK_DOWN, lnum - 1);
}

void ViKeyHandler::_Vi_gg()
{
	vector<INPUT> inputs;
	if (vicmd.GetOperatorPending()) // linewise
	{
		// TODO: 移動先が現在行以降の場合、現在行を対象にするにはVK_HOME
		_QueueKey(&inputs, VK_END);
		_QueueKey(&inputs, VK_RIGHT); // to include '\n'
		_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
	}
	_QueueKeyToLine(&inputs, vicmd.GetCount());
	if (vicmd.GetOperatorPending())
	{
		_QueueKeyForModifier(&inputs, VK_SHIFT, TRUE);
	}
	// TODO: goto first non-blank character (!OperatorPending)
	_ViOp(&inputs);
}

void ViKeyHandler::_Vi_G()
{
	vector<INPUT> inputs;
	if (vicmd.GetOperatorPending()) // linewise
	{
		// TODO: 移動先が現在行以前の場合、現在行を対象にするにはVK_END
		_QueueKey(&inputs, VK_HOME);
		_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
	}
	if (vicmd.HasCount())
	{
		_QueueKeyToLine(&inputs, vicmd.GetCount());
	}
	else
	{
		_QueueKeyForModifier(&inputs, VK_CONTROL, FALSE);
		_QueueKey(&inputs, VK_END);
		_QueueKeyForModifier(&inputs, VK_CONTROL, TRUE);
	}
	if (vicmd.GetOperatorPending())
	{
		_QueueKeyForModifier(&inputs, VK_SHIFT, TRUE);
	}
	else
	{
		// TODO: goto first non-blank character
		_QueueKey(&inputs, VK_HOME);
	}
	_ViOp(&inputs);
}

void ViKeyHandler::_Vi_J(ITfContext *pContext)
{
	// TODO: support count
	VimCharStream pos(_textService, pContext);
	if (pos.eof())
	{
		return;
	}

	int r = pos.forward_eol();
	if (r == -1) // end of file
	{
		return;
	}
	int index_eol = pos.difference();
	// check last character
	bool ismulti1 = false;
	r = pos.dec();
	if (r != -1)
	{
		if (r == 0)
		{
			ismulti1 = VimMByte::ismulti(pos.gchar());
		}
		pos.inc();
	}

	// next line
	if (pos.inc() == -1)
	{
		return;
	}
	bool ismulti2 = false;
	if (pos.fblank() != -1)
	{
		ismulti2 = VimMByte::ismulti(pos.gchar());
	}
	int index_end = pos.difference();

	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_RIGHT, index_eol);
	_QueueKey(&inputs, VK_DELETE, index_end - index_eol);
	if (!ismulti1 && !ismulti2)
	{
		isThroughSelfSentKey = TRUE;
		_QueueKey(&inputs, VK_SPACE);
		_QueueKey(&inputs, VK_ESCAPE); // to reset isThroughSelfSentKey
	}
	_SendInputs(&inputs);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_r(BYTE vk)
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_DELETE, vicmd.GetCount());
	isThroughSelfSentKey = TRUE;

	mozc::win32::KeyboardStatus keyboard_state;
	bool shiftPressed = false;
	if (keyboard_->GetKeyboardState(&keyboard_state))
	{
		if (keyboard_state.IsPressed(VK_SHIFT))
		{
			shiftPressed = true;
		}
	}
	if (shiftPressed)
	{
		_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
	}
	_QueueKey(&inputs, vk, vicmd.GetCount());
	if (shiftPressed)
	{
		_QueueKeyForModifier(&inputs, VK_SHIFT, TRUE);
	}
	_QueueKey(&inputs, VK_ESCAPE); // to reset isThroughSelfSentKey
	_SendInputs(&inputs);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_slash()
{
	vector<INPUT> inputs;
	isThroughSelfSentKey = TRUE;
	_QueueKeyWithControl(&inputs, 'F');
	_QueueKey(&inputs, VK_ESCAPE); // to reset isThroughSelfSentKey
	_QueueKeyForOtherIME(&inputs); // to input search word
	_SendInputs(&inputs);
	vicmd.Reset();
}
