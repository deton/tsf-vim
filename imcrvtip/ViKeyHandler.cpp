﻿
#include "imcrvtip.h"
#include "TextService.h"
#include "ViKeyHandler.h"
#include "ViCharStream.h"
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
	keyboard_(mozc::win32::Win32KeyboardInterface::CreateDefault())
{
}

ViKeyHandler::~ViKeyHandler()
{
}

void ViKeyHandler::Reset()
{
	vicmd.Reset();
}

BOOL ViKeyHandler::IsWaitingNextKey()
{
	return !vicmd.IsEmpty();
}

HRESULT ViKeyHandler::HandleKey(TfEditCookie ec, ITfContext *pContext, WCHAR ch)
{
	if(vicmd.GetCharWaiting())
	{
		switch(vicmd.GetCharWaiting())
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
	switch(ch)
	{
	case L'0':
		if(!vicmd.HasCount())
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
		if(vicmd.GetOperatorPending())
		{
			if(vicmd.GetOperatorPending() == ch) //'cc','dd','yy'
			{
				//TODO
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
	case L'f':
	case L't':
	case L'F':
	case L'T':
		vicmd.SetCharWaiting(ch);
		return;
	case CTRL('F'):
		_SendKey(VK_NEXT);
		vicmd.Reset();
		return;
	case CTRL('B'):
		_SendKey(VK_PRIOR);
		vicmd.Reset();
		return;
	case L'$':
		//TODO: OperatorPendingの場合、改行が含まれていたら除く
		_ViOpOrMove(VK_END, 1);
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
	case L'o':
		_Vi_o();
		return;
	case L'p':
		_Vi_p(pContext);
		return;
	case L'P': //paste at caret
		_Vi_P();
		return;
	case L'u':
		vicmd.Reset();
		_SendKeyWithControl('Z');
		return;
	case L'x':
		vicmd.SetOperatorPending('d'); //cut to clipboard
		_ViOpOrMove(VK_RIGHT, vicmd.GetCount());
		return;
	case L'X':
		vicmd.SetOperatorPending('d'); //cut to clipboard
		_ViOpOrMove(VK_LEFT, vicmd.GetCount());
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
	default:
		vicmd.Reset();
		break;
	}
}

void ViKeyHandler::_QueueKey(vector<INPUT> *inputs, UINT vk, int count)
{
	const KEYBDINPUT keyboard_input = {vk, 0, 0, 0, 0};
	INPUT keydown = {};
	keydown.type = INPUT_KEYBOARD;
	keydown.ki = keyboard_input;

	INPUT keyup = keydown;
	keyup.type = INPUT_KEYBOARD;
	keyup.ki.dwFlags = KEYEVENTF_KEYUP;

	for(int i = 0; i < count; ++i)
	{
		inputs->push_back(keydown);
		inputs->push_back(keyup);
	}
}

void ViKeyHandler::_QueueKeyForSelection(vector<INPUT> *inputs)
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

void ViKeyHandler::_QueueKeyForModifier(vector<INPUT> *inputs, UINT vk, BOOL up)
{
	const KEYBDINPUT keyboard_input = {vk, 0, 0, 0, 0};
	INPUT keydown = {};
	keydown.type = INPUT_KEYBOARD;
	keydown.ki = keyboard_input;

	if(up)
	{
		INPUT keyup = keydown;
		keyup.type = INPUT_KEYBOARD;
		keyup.ki.dwFlags = KEYEVENTF_KEYUP;
		inputs->push_back(keyup);
	}
	else
	{
		inputs->push_back(keydown);
	}
}

void ViKeyHandler::_QueueKeyWithControl(vector<INPUT> *inputs, UINT vk)
{
	_QueueKeyForModifier(inputs, VK_CONTROL, FALSE);
	_QueueKey(inputs, vk);
	_QueueKeyForModifier(inputs, VK_CONTROL, TRUE);
}

void ViKeyHandler::_SendInputs(vector<INPUT> *inputs)
{
	//cf. deleter.UnsetModifiers()
	mozc::win32::KeyboardStatus keyboard_state;
	if(keyboard_->GetKeyboardState(&keyboard_state))
	{
		const BYTE kUnsetState = 0;
		bool to_be_updated = false;
		if(keyboard_state.IsPressed(VK_SHIFT))
		{
			to_be_updated = true;
			keyboard_state.SetState(VK_SHIFT, kUnsetState);
			//restore modifier
			//XXX:SendInput()直後にdeleter.EndDeletion()を呼んでも、
			//CTRL-F押下時にVK_NEXT送り付けても動かない
			//(おそらくCTRL押下状態になってCTRL-NEXTになるため)
			_QueueKeyForModifier(inputs, VK_SHIFT, FALSE);
		}
		if(keyboard_state.IsPressed(VK_CONTROL))
		{
			to_be_updated = true;
			keyboard_state.SetState(VK_CONTROL, kUnsetState);
			_QueueKeyForModifier(inputs, VK_CONTROL, FALSE);
		}
		if(to_be_updated)
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

void ViKeyHandler::_ViOp(vector<INPUT> *inputs)
{
	switch(vicmd.GetOperatorPending())
	{
	case L'c':
		_QueueKeyWithControl(inputs, 'X');
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
	if(vicmd.GetOperatorPending())
	{
		_QueueKeyForSelection(&inputs);
	}
	_ViOp(&inputs);
}

void ViKeyHandler::_Vi_j()
{
	if(vicmd.GetOperatorPending()) //linewise operator
	{
		vector<INPUT> inputs;
		_QueueKey(&inputs, VK_HOME);
		_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
		_QueueKey(&inputs, VK_END);
		for(int count = vicmd.GetCount(); count > 0; --count)
		{
			_QueueKey(&inputs, VK_DOWN);
			_QueueKey(&inputs, VK_END);
		}
		_QueueKeyForModifier(&inputs, VK_SHIFT, TRUE);
		_ViOp(&inputs);
	}
	else
	{
		_ViOpOrMove(VK_DOWN, vicmd.GetCount());
	}
}

void ViKeyHandler::_Vi_k()
{
	if(vicmd.GetOperatorPending()) //linewise operator
	{
		vector<INPUT> inputs;
		_QueueKey(&inputs, VK_END);
		_QueueKeyForModifier(&inputs, VK_SHIFT, FALSE);
		_QueueKey(&inputs, VK_HOME);
		for(int count = vicmd.GetCount(); count > 0; --count)
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

void ViKeyHandler::_Vi_o()
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_END);
	_QueueKey(&inputs, VK_RETURN);
	_SendInputs(&inputs);
	_textService->_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

void ViKeyHandler::_Vi_p(ITfContext *pContext)
{
	vector<INPUT> inputs;
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		if(info.following_text.size() > 0
				&& info.following_text[0] != L'\n'
				&& info.following_text[0] != L'\r')
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
	_QueueKeyForModifier(&inputs, VK_CONTROL, FALSE);
	_QueueKey(&inputs, 'V', vicmd.GetCount());
	_QueueKeyForModifier(&inputs, VK_CONTROL, TRUE);
	_SendInputs(&inputs);
	vicmd.Reset();
}

#define RETURN_IF_FAIL(movefunc) if(movefunc()) return
#define CS_PREV() RETURN_IF_FAIL(cs.prev)
#define CS_NEXT() RETURN_IF_FAIL(cs.next)
#define CS_FSPACE() RETURN_IF_FAIL(cs.fspace)
#define CS_FBLANK() RETURN_IF_FAIL(cs.fblank)
#define CS_BBLANK() RETURN_IF_FAIL(cs.bblank)

void ViKeyHandler::_ViNextWord(ITfContext *pContext, WCHAR type)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		return;
	}
	ViCharStream cs(info.preceding_text, info.following_text);
	//TODO:取得した文字列に単語末が含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	//cf. fword() in v_word.c of nvi-1.79
	/*
	 * If in white-space:
	 *	If the count is 1, and it's a change command, we're done.
	 *	Else, move to the first non-white-space character, which
	 *	counts as a single word move.  If it's a motion command,
	 *	don't move off the end of the line.
	 */
	if(cs.flags() == CS_EMP || cs.flags() == CS_NONE && iswblank(cs.ch()))
	{
		if(vicmd.GetOperatorPending() && cs.flags() != CS_EMP && cnt == 1)
		{
			if(vicmd.GetOperatorPending() == L'c')
			{
				return;
			}
			if(vicmd.GetOperatorPending() == L'd' || vicmd.GetOperatorPending() == L'y')
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
	if(type == L'W')
	{
		while(cnt--)
		{
			for(;;)
			{
				CS_NEXT();
				if(cs.flags() == CS_EOF)
				{
					goto ret;
				}
				if(cs.flags() != CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
			}
			/*
			 * If a motion command and we're at the end of the
			 * last word, we're done.  Delete and yank eat any
			 * trailing blanks, but we don't move off the end
			 * of the line regardless.
			 */
			if(cnt == 0 && vicmd.GetOperatorPending())
			{
				if(vicmd.GetOperatorPending() == L'd' || vicmd.GetOperatorPending() == L'y')
				{
					CS_FSPACE();
					break;
				}
			}

			/* Eat whitespace characters. */
			CS_FBLANK();
			if(cs.flags() == CS_EOF)
			{
				goto ret;
			}
		}
	}
	else //'w'
	{
		while(cnt--)
		{
			enum { INWORD, NOTWORD } state;
			state = cs.flags() == CS_NONE && inword(cs.ch()) ? INWORD : NOTWORD;
			for(;;)
			{
				CS_NEXT();
				if(cs.flags() == CS_EOF)
				{
					goto ret;
				}
				if(cs.flags() != CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if(state == INWORD)
				{
					if(!inword(cs.ch()))
					{
						break;
					}
				}
				else
				{
					if(inword(cs.ch()))
					{
						break;
					}
				}
			}
			/* See comment above. */
			if(cnt == 0 && vicmd.GetOperatorPending())
			{
				if(vicmd.GetOperatorPending() == L'd' || vicmd.GetOperatorPending() == L'y')
				{
					CS_FSPACE();
					break;
				}
			}

			/* Eat whitespace characters. */
			if(cs.flags() != CS_NONE || iswblank(cs.ch()))
			{
				CS_FBLANK();
				if(cs.flags() == CS_EOF)
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
	if(!vicmd.GetOperatorPending() && movecnt == 0)
	{
		return;
	}

	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_ViNextWordE(ITfContext *pContext, WCHAR type)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		return;
	}
	ViCharStream cs(info.preceding_text, info.following_text);
	//TODO:取得した文字列に単語末が含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	//cf. eword() in v_word.c of nvi-1.79
	/*
	 * !!!
	 * If in whitespace, or the next character is whitespace, move past
	 * it.  (This doesn't count as a word move.)  Stay at the character
	 * past the current one, it sets word "state" for the 'e' command.
	 */
	if(cs.flags() == CS_NONE && !iswblank(cs.ch()))
	{
		CS_NEXT();
		if(cs.flags() == CS_NONE && !iswblank(cs.ch()))
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
	if(type == 'E')
	{
		while(cnt--)
		{
			for(;;)
			{
				CS_NEXT();
				if(cs.flags() == CS_EOF)
				{
					goto ret;
				}
				if(cs.flags() != CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
			}
			/*
			 * When we reach the start of the word after the last
			 * word, we're done.  If we changed state, back up one
			 * to the end of the previous word.
			 */
			if(cnt == 0)
			{
				if(cs.flags() == CS_NONE)
				{
					CS_PREV();
					break;
				}
			}

			/* Eat whitespace characters. */
			CS_FBLANK();
			if(cs.flags() == CS_EOF)
			{
				goto ret;
			}
		}
	}
	else //'e'
	{
		while(cnt--)
		{
			enum { INWORD, NOTWORD } state;
			state = cs.flags() == CS_NONE && inword(cs.ch()) ? INWORD : NOTWORD;
			for(;;)
			{
				CS_NEXT();
				if(cs.flags() == CS_EOF)
				{
					goto ret;
				}
				if(cs.flags() != CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if(state == INWORD)
				{
					if(!inword(cs.ch()))
					{
						break;
					}
				}
				else
				{
					if(inword(cs.ch()))
					{
						break;
					}
				}
			}
			/* See comment above. */
			if(cnt==0)
			{
				if(cs.flags() == CS_NONE)
				{
					CS_PREV();
				}
				break;
			}

			/* Eat whitespace characters. */
			if(cs.flags() != CS_NONE || iswblank(cs.ch()))
			{
				CS_FBLANK();
			}
			if(cs.flags() == CS_EOF)
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
	if(!vicmd.GetOperatorPending() && movecnt == 0)
	{
		return;
	}

	if(vicmd.GetOperatorPending() && cs.flags() == CS_NONE)
	{
		movecnt++;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_ViPrevWord(ITfContext *pContext, WCHAR type)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		return;
	}
	ViCharStream cs(info.preceding_text, info.following_text);
	//TODO:取得した文字列に単語頭が含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	//cf. bword() in v_word.c of nvi-1.79
	/*
	 * !!!
	 * If in whitespace, or the previous character is whitespace, move
	 * past it.  (This doesn't count as a word move.)  Stay at the
	 * character before the current one, it sets word "state" for the
	 * 'b' command.
	 */
	if(cs.flags() == CS_NONE && !iswblank(cs.ch()))
	{
		CS_PREV();
		if(cs.flags() == CS_NONE && !iswblank(cs.ch()))
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
	if(type == 'B')
	{
		while(cnt--)
		{
			for(;;)
			{
				CS_PREV();
				if(cs.flags() == CS_SOF)
				{
					goto ret;
				}
				if(cs.flags() != CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
			}
			/*
			 * When we reach the end of the word before the last
			 * word, we're done.  If we changed state, move forward
			 * one to the end of the next word.
			 */
			if(cnt == 0)
			{
				if(cs.flags() == CS_NONE)
				{
					CS_NEXT();
					break;
				}
			}

			/* Eat whitespace characters. */
			CS_BBLANK();
			if(cs.flags() == CS_SOF)
			{
				goto ret;
			}
		}
	}
	else //'b'
	{
		while(cnt--)
		{
			enum { INWORD, NOTWORD } state;
			state = cs.flags() == CS_NONE && inword(cs.ch()) ? INWORD : NOTWORD;
			for(;;)
			{
				CS_PREV();
				if(cs.flags() == CS_SOF)
				{
					goto ret;
				}
				if(cs.flags() != CS_NONE || iswblank(cs.ch()))
				{
					break;
				}
				if(state == INWORD)
				{
					if(!inword(cs.ch()))
					{
						break;
					}
				}
				else
				{
					if(inword(cs.ch()))
					{
						break;
					}
				}
			}
			/* See comment above. */
			if(cnt==0)
			{
				if(cs.flags() == CS_NONE)
				{
					CS_NEXT();
				}
				break;
			}

			/* Eat whitespace characters. */
			if(cs.flags() != CS_NONE || iswblank(cs.ch()))
			{
				CS_BBLANK();
			}
			if(cs.flags() == CS_SOF)
			{
				goto ret;
			}
		}
	}

	/* If we didn't move, we must be at SOF. */
ret:
	int movecnt = cs.difference();
	if(movecnt == 0)
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
	if(c == L'.' || c == L'!' || c == L'?')
	{
		pos->save_index();
		do {
			if(pos->inc() == -1)
			{
				return TRUE;
			}
		} while(issentendex(pos->gchar()));
		if(iswblank(pos->gchar()))
		{
			return TRUE;
		}
		else if(pos->gchar() == L'\n')
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
	if(VimMByte::chclass(pos->gchar()) == VimMByte::PUNCT)
	{
		for(;;)
		{
			if(pos->inc() == -1)
			{
				return TRUE;
			}
			WCHAR c = pos->gchar();
			if(!VimMByte::ismulti(c) || VimMByte::chclass(c) != VimMByte::PUNCT)
			{
				if(c == L'\n')
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

    BOOL noskip = FALSE;
	int count = vicmd.GetCount();
	// cf. findsent() in search.c of vim.
	while(count--)
	{
		/*
		 * if on an empty line, skip upto a non-empty line
		 */
		if(pos.gchar() == L'\n')
		{
			do {
				if(pos.incl() == -1)
				{
					break;
				}
			} while(pos.gchar() == L'\n');
			goto found;
		}

		/* go back to the previous non-blank char */
		while(iswblank(pos.gchar()))
		{
			int r = pos.decl();
			if(r == -1)
			{
				break;
			}
			/* Stop in front of empty line */
			if(r == 2)
			{
				pos.incl();
				goto found;
			}
		}

		for(;;)		/* find end of sentence */
		{
			if(pos.gchar() == L'\n')
			{
				break;
			}
			if(endsent(&pos))
			{
				break;
			}
			if(endmbsent(&pos))
			{
				break;
			}
			if(pos.incl() == -1)
			{
				if(count)
				{
					return;
				}
				noskip = TRUE;
				break;
			}
		}
found:
		/* skip white space */
		if(!noskip)
		{
			while(iswblank(pos.gchar()))
			{
				if(pos.incl() == -1)
				{
					break;
				}
			}
		}
	}

	size_t movecnt = pos.difference();
	_ViOpOrMove(VK_RIGHT, movecnt);
}

void ViKeyHandler::_VimBackwardSent(ITfContext *pContext)
{
	VimCharStream pos(_textService, pContext);

    BOOL noskip = FALSE;
	int count = vicmd.GetCount();
	// cf. findsent() in search.c of vim.
	while(count--)
	{
		/*
		 * if on an empty line, skip upto a non-empty line
		 */
		if(pos.gchar() == L'\n')
		{
			do {
				if(pos.decl() == -1)
				{
					break;
				}
			} while(pos.gchar() == L'\n');
		}
		else
		{
			pos.decl();
		}

		/* go back to the previous non-blank char */
		BOOL found_dot = FALSE;
		while(iswblank(pos.gchar()) || issentend(pos.gchar()))
		{
			WCHAR c = pos.gchar();
			if(c == L'.' || c == L'!' || c == L'?')
			{
				/* Only skip over a '.', '!' and '?' once. */
				if(found_dot)
				{
					break;
				}
				found_dot = TRUE;
			}
			if(pos.decl() == -1)
			{
				break;
			}
		}

		/* remember the index where the search started */
		size_t startindex = pos.index();

		for(;;)		/* find end of sentence */
		{
			if(pos.gchar() == L'\n')
			{
				if(pos.index() != startindex)
				{
					pos.inc();
				}
				break;
			}
			if(endsent(&pos))
			{
				break;
			}
			if(endmbsent(&pos))
			{
				break;
			}
			if(pos.decl() == -1)
			{
				if(count)
				{
					return;
				}
				noskip = TRUE;
				break;
			}
		}
//found:
		/* skip white space */
		if(!noskip)
		{
			while(iswblank(pos.gchar()))
			{
				if(pos.incl() == -1)
				{
					break;
				}
			}
		}
	}

	size_t movecnt = -pos.difference();
	_ViOpOrMove(VK_LEFT, movecnt);
}

int ViKeyHandler::_Vi_f_sub(ITfContext *pContext, WCHAR ch)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		return -1;
	}
	std::wstring text(info.following_text);
	// erase chars after newline to search in current line.
	size_t nl = text.find_first_of(L"\r\n");
	if(nl != std::wstring::npos)
	{
		text.erase(nl);
	}
	//TODO:取得した文字列にchが含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	size_t offset = 0;
	while(cnt--)
	{
		offset++;
		if(offset >= text.size())
		{
			return 0;
		}
		size_t i = text.find(ch, offset);
		if(i == std::wstring::npos)
		{
			return 0;
		}
		offset = i;
	}
	return offset;
}

//	Search forward in the line for the next occurrence of the
//	specified character.
void ViKeyHandler::_Vi_f(ITfContext *pContext, WCHAR ch)
{
	int movecnt = _Vi_f_sub(pContext, ch);
	if(movecnt <= 0)
	{
		return;
	}
	if(vicmd.GetOperatorPending())
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
	if(movecnt <= 0)
	{
		return;
	}
	if(!vicmd.GetOperatorPending())
	{
		movecnt--;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}

int ViKeyHandler::_Vi_F_sub(ITfContext *pContext, WCHAR ch)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(_textService, pContext, &info))
	{
		return -1;
	}
	std::wstring text(info.preceding_text);
	// erase chars before newline to search in current line.
	size_t nl = text.find_last_of(L"\r\n");
	if(nl != std::wstring::npos)
	{
		text.erase(0, nl);
	}
	//TODO:取得した文字列にchが含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	size_t offset = text.size();
	while(cnt--)
	{
		offset--;
		if(offset < 0)
		{
			return 0;
		}
		size_t i = text.rfind(ch, offset);
		if(i == std::wstring::npos)
		{
			return 0;
		}
		offset = i;
	}
	return text.size() - offset;
}

//	Search backward in the line for the next occurrence of the
//	specified character.
void ViKeyHandler::_Vi_F(ITfContext *pContext, WCHAR ch)
{
	int movecnt = _Vi_F_sub(pContext, ch);
	if(movecnt <= 0)
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
	if(movecnt <= 1)
	{
		return;
	}
	movecnt--;
	_ViOpOrMove(VK_LEFT, movecnt);
}
