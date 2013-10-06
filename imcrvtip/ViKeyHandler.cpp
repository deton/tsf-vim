
#include "imcrvtip.h"
#include "TextService.h"
#include "ViCharStream.h"
#include "mozc/win32/tip/tip_surrounding_text.h"
#include "mozc/win32/base/keyboard.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, WCHAR ch)
{
	if(vicmd.GetCharWaiting())
	{
		switch(vicmd.GetCharWaiting())
		{
		case L'f':
			_Vi_f(pContext, ch);
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

void CTextService::_HandleFunc(TfEditCookie ec, ITfContext *pContext, WCHAR ch)
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
		vicmd.SetCharWaiting(ch);
		return;
	case CTRL('F'):
		deleter.UnsetModifiers(); //CTRLキー押下状態のままNEXT押しても期待外
		_SendKey(VK_NEXT);
		//TODO: CTRLキー押下状態に戻す。でないとCTRLキー押したまま連続Fできず
		vicmd.Reset();
		return;
	case CTRL('B'):
		deleter.UnsetModifiers();
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
		_ViOpOrMove(VK_DOWN, vicmd.GetCount()); //TODO: linewise operator
		return;
	case L'k':
		_ViOpOrMove(VK_UP, vicmd.GetCount()); //TODO: linewise operator
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
	case L')':
		_ViNextSentence(pContext);
		vicmd.Reset();
		return;
	default:
		vicmd.Reset();
		break;
	}
}

void CTextService::_QueueKey(vector<INPUT> *inputs, UINT vk, int count)
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

void CTextService::_QueueKeyForSelection(vector<INPUT> *inputs)
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

void CTextService::_QueueKeyForModifier(vector<INPUT> *inputs, UINT vk, BOOL up)
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

void CTextService::_QueueKeyWithControl(vector<INPUT> *inputs, UINT vk)
{
	_QueueKeyForModifier(inputs, VK_CONTROL, FALSE);
	_QueueKey(inputs, vk);
	_QueueKeyForModifier(inputs, VK_CONTROL, TRUE);
}

void CTextService::_SendKey(UINT vk, int count)
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, vk, count);
	keyboard_->SendInput(inputs);
}

void CTextService::_SendKeyWithControl(UINT vk)
{
	vector<INPUT> inputs;
	_QueueKeyWithControl(&inputs, vk);
	keyboard_->SendInput(inputs);
}

void CTextService::_ViOpOrMove(UINT vk, int count)
{
	deleter.UnsetModifiers();
	vector<INPUT> inputs;
	_QueueKey(&inputs, vk, count);
	switch(vicmd.GetOperatorPending())
	{
	case L'c':
		_QueueKeyForSelection(&inputs);
		_QueueKeyWithControl(&inputs, 'X');
		keyboard_->SendInput(inputs);
		_SetKeyboardOpen(FALSE);
		break;
	case L'd':
		_QueueKeyForSelection(&inputs);
		_QueueKeyWithControl(&inputs, 'X');
		keyboard_->SendInput(inputs);
		break;
	case L'y':
		_QueueKeyForSelection(&inputs);
		_QueueKeyWithControl(&inputs, 'C');
		keyboard_->SendInput(inputs);
		break;
	default:
		keyboard_->SendInput(inputs);
		break;
	}
	vicmd.Reset();
}

void CTextService::_Vi_o()
{
	vector<INPUT> inputs;
	_QueueKey(&inputs, VK_END);
	_QueueKey(&inputs, VK_RETURN);
	keyboard_->SendInput(inputs);
	_SetKeyboardOpen(FALSE);
	vicmd.Reset();
}

void CTextService::_Vi_p(ITfContext *pContext)
{
	vector<INPUT> inputs;
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(mozc::win32::tsf::TipSurroundingText::Get(this, pContext, &info))
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
	keyboard_->SendInput(inputs);
	vicmd.Reset();
}

void CTextService::_Vi_P()
{
	vector<INPUT> inputs;
	_QueueKeyForModifier(&inputs, VK_CONTROL, FALSE);
	_QueueKey(&inputs, 'V', vicmd.GetCount());
	_QueueKeyForModifier(&inputs, VK_CONTROL, TRUE);
	keyboard_->SendInput(inputs);
	vicmd.Reset();
}

//次の文に移動
void CTextService::_ViNextSentence(ITfContext *pContext)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(this, pContext, &info))
	{
		return;
	}
	ViCharStream cs(info.following_text);
	//TODO:取得した文字列に文末が含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	//cf. v_sentencef() in v_sentence.c of nvi-1.79
	//If in white-space, the next start of sentence counts as one.
	if(cs.flags() == CS_EMP || cs.flags() == CS_NONE && iswblank(cs.ch()))
	{
		if(cs.fblank())
		{
			return;
		}
		if(--cnt == 0)
		{
			goto okret;
			return;
		}
	}

	enum { BLANK, NONE, PERIOD } state;
	for(state = NONE;;)
	{
		//XXX:現在位置に'.'がある場合、'.'が読みとばされる。
		if(cs.next())
		{
			return;
		}
		if(cs.flags() == CS_EOF)
		{
			return;
		}
		if(cs.flags() == CS_EOL)
		{
			if((state == PERIOD || state == BLANK) && --cnt == 0)
			{
				if(cs.next())
				{
					return;
				}
				if(cs.flags() == CS_NONE && iswblank(cs.ch()))
				{
					if(cs.fblank())
					{
						return;
					}
					goto okret;
				}
			}
			state = NONE;
			continue;
		}
		if(cs.flags() == CS_EMP) // An EMP is two sentences.
		{
			if(--cnt == 0)
			{
				goto okret;
			}
			if(cs.fblank())
			{
				return;
			}
			if(--cnt == 0)
			{
				goto okret;
			}
			state = NONE;
			continue;
		}
		switch(cs.ch())
		{
		case L'.':
		case L'?':
		case L'!':
			state = PERIOD;
			break;
		case L')':
		case L']':
		case L'"':
		case L'\'':
			if(state != PERIOD)
			{
				state = NONE;
			}
			break;
		case L'\t':
			if(state == PERIOD)
			{
				state = BLANK;
			}
			//FALLTHROUGH
		case L' ':
			if(state == PERIOD)
			{
				state = BLANK;
				break;
			}
			if(state == BLANK && --cnt == 0)
			{
				if(cs.fblank())
				{
					return;
				}
				goto okret;
			}
			//FALLTHROUGH
		default:
			state = NONE;
			break;
		}
	}

okret:
	size_t movecnt = cs.index();
	_ViOpOrMove(VK_RIGHT, movecnt);
}

//	Search forward in the line for the next occurrence of the
//	specified character.
void CTextService::_Vi_f(ITfContext *pContext, WCHAR ch)
{
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(!mozc::win32::tsf::TipSurroundingText::Get(this, pContext, &info))
	{
		return;
	}
	ViCharStream cs(info.following_text);
	//TODO:取得した文字列にchが含まれていなかったら、
	//カーソルを移動して、さらに文字列を取得する処理を繰り返す

	int cnt = vicmd.GetCount();
	while(cnt--)
	{
		while(1)
		{
			if(cs.next())
			{
				return;
			}
			if(cs.flags() == CS_EOF)
			{
				return;
			}
			if(cs.flags() == CS_EOL)
			{
				return;
			}
			if(cs.flags() == CS_EMP)
			{
				return;
			}
			if(cs.ch() == ch)
			{
				break;
			}
		}
	}

	size_t movecnt = cs.index();
	if(vicmd.GetOperatorPending())
	{
		movecnt++;
	}
	_ViOpOrMove(VK_RIGHT, movecnt);
}
