
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "ViCharStream.h"
#include "mozc/win32/tip/tip_surrounding_text.h"
#include "mozc/win32/base/input_state.h"

HRESULT CTextService::_HandleChar(TfEditCookie ec, ITfContext *pContext, std::wstring &composition, WCHAR ch, WCHAR chO)
{
	_PrepareForFunc(ec, pContext);
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

HRESULT CTextService::_HandleCharReturn(TfEditCookie ec, ITfContext *pContext, BOOL back)
{
	_Update(ec, pContext, TRUE, back);
	_TerminateComposition(ec, pContext);
	_ResetStatus();

	return S_OK;
}

HRESULT CTextService::_HandleCharTerminate(TfEditCookie ec, ITfContext *pContext, std::wstring &composition)
{
	if(!composition.empty())
	{
		_Update(ec, pContext, composition, TRUE);
		_TerminateComposition(ec, pContext);
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
		break;
	}
	_HandleCharReturn(ec, pContext);
}

//入力シーケンスに割り当てられた「機能」の実行前に、composition表示等をクリア
void CTextService::_PrepareForFunc(TfEditCookie ec, ITfContext *pContext)
{
	//wordpadやWord2010だとcomposition表示をクリアしないとうまく動かず
	_ResetStatus();
	_HandleCharReturn(ec, pContext);
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

//後置型交ぜ書き変換
HRESULT CTextService::_HandlePostMaze(TfEditCookie ec, ITfContext *pContext, int count)
{
	//カーソル直前の文字列を取得
	std::wstring text;
	_AcquirePrecedingText(pContext, &text);
	int size = text.size();
	if(size == 0)
	{
		_HandleCharReturn(ec, pContext);
		return S_OK;
	}
	if(size < count)
	{
		count = size;
	}
	//TODO:サロゲートペアや結合文字等の考慮
	kana.insert(cursoridx, text.substr(size - count));
	cursoridx += kana.size();
	return _ReplacePrecedingText(ec, pContext, count, true);
}

//後置型カタカナ変換
HRESULT CTextService::_HandlePostKata(TfEditCookie ec, ITfContext *pContext, int count)
{
	//カーソル直前の文字列を取得
	std::wstring text;
	_AcquirePrecedingText(pContext, &text);
	int size = text.size();
	if(size == 0)
	{
		_HandleCharReturn(ec, pContext);
		return S_OK;
	}

	//ひらがなをカタカナに変換
	std::wstring kata;
	int st = size - count;
	if(st < 0)
	{
		st = 0;
	}
	if(count <= 0) //0: ひらがなが続く間、負: ひらがなとして残す文字数指定
	{
		//TODO:サロゲートペアや結合文字等の考慮
		for(st = size - 1; 0 <= st; st--)
		{
//TRUEの文字が続く間、後置型カタカナ変換対象とする(ひらがな、「ー」)
#define TYOON(m) ((m) == 0x30FC)
#define IN_KATARANGE(m) (0x3041 <= (m) && (m) <= 0x309F || TYOON(m))
			WCHAR m = text[st];
			if(!IN_KATARANGE(m))
			{
				// 「キーとばりゅー」に対し1文字残してカタカナ変換で
				// 「キーとバリュー」になるように「ー」は除く
				while(st < size - 1)
				{
					m = text[st + 1];
					if(TYOON(m))
					{
						st++;
					}
					else
					{
						break;
					}
				}
				break;
			}
		}
		st++;
		if(count < 0)
		{
			st += -count; // 指定文字数を除いてカタカナに変換
		}
	}
	int cnt = size - st;
	if(cnt > 0)
	{
		_ConvKanaToKana(kata, im_katakana, text.substr(st), im_hiragana);
		//カーソル直前の文字列を置換
		kana.insert(cursoridx, kata);
		cursoridx += kata.size();
		prevkata = kata;
		_ReplacePrecedingText(ec, pContext, cnt);
	}
	else
	{
		_HandleCharReturn(ec, pContext);
	}

	return S_OK;
}

//直前の後置型カタカナ変換を縮める
//例: 「例えばあぷりけーしょん」ひらがなが続く間カタカナに変換
//	→「例エバアプリケーション」2文字縮める
//	→「例えばアプリケーション」
HRESULT CTextService::_HandlePostKataShrink(TfEditCookie ec, ITfContext *pContext, int count)
{
	if(prevkata.empty())
	{
		_HandleCharReturn(ec, pContext);
		return S_OK;
	}

	//カーソル直前の文字列を取得
	std::wstring text;
	_AcquirePrecedingText(pContext, &text);

	int prevsize = prevkata.size();
	int size = text.size();
	if(size < prevsize || text.compare(size - prevsize, prevsize, prevkata) != 0)
	{
		_HandleCharReturn(ec, pContext);
		return S_OK;
	}
	//countぶん縮める部分をひらがなにする
	int kataLen = prevsize - count;
	if(kataLen < 0)
	{
		kataLen = 0;
		count = prevsize;
	}
	//縮めることでひらがなになる文字列
	std::wstring hira;
	_ConvKanaToKana(hira, im_hiragana, prevkata.substr(0, count), im_katakana);
	kana.insert(cursoridx, hira);
	cursoridx += hira.size();
	if(kataLen > 0)
	{
		//カタカナのままにする文字列
		//繰り返しShrinkできるように、prevkataを縮める
		prevkata.erase(0, count);
		kana.insert(cursoridx, prevkata);
		cursoridx += kataLen;
	}

	_ReplacePrecedingText(ec, pContext, prevsize);
	return S_OK;
}

//後置型部首合成変換
HRESULT CTextService::_HandlePostBushu(TfEditCookie ec, ITfContext *pContext)
{
	//カーソル直前の文字列を取得
	std::wstring text;
	_AcquirePrecedingText(pContext, &text);

	size_t size = text.size();
	if(size >= 2)
	{
		//TODO:サロゲートペアや結合文字等の考慮
		WCHAR bushu1 = text[size - 2];
		WCHAR bushu2 = text[size - 1];
		//部首合成変換
		WCHAR kanji = _SearchBushuDic(bushu1, bushu2);
		if(kanji != 0)
		{
			//カーソル直前の文字列を置換
			kana.insert(cursoridx, 1, kanji);
			cursoridx++;

			_ReplacePrecedingText(ec, pContext, 2);
			return S_OK;
		}
	}
	_HandleCharReturn(ec, pContext);

	return S_OK;
}

//カーソル直前の文字列を取得
HRESULT CTextService::_AcquirePrecedingText(ITfContext *pContext, std::wstring *text)
{
	text->clear();
	mozc::win32::tsf::TipSurroundingTextInfo info;
	if(mozc::win32::tsf::TipSurroundingText::Get(this, pContext, &info))
	{
		text->append(info.preceding_text);
	}
	else
	{
		text->append(postbuf);
	}
	return S_OK;
}

//カーソル直前の文字列を、kanaに置換
HRESULT CTextService::_ReplacePrecedingText(TfEditCookie ec, ITfContext *pContext, int delete_count, bool startMaze)
{
	if(!mozc::win32::tsf::TipSurroundingText::DeletePrecedingText(this, pContext, delete_count))
	{
		return _ReplacePrecedingTextIMM32(ec, pContext, delete_count, startMaze);
	}
	if(startMaze)
	{
		//(候補無し時、登録に入るため。でないと読みが削除されただけの状態)
		if(!_IsComposing())
		{
			_StartComposition(pContext);
		}
		//交ぜ書き変換候補表示開始
		showentry = TRUE;
		inputkey = TRUE;
		_StartConv();
		_Update(ec, pContext);
		//TODO:cancel時は前置型読み入力モードでなく後置型開始前の状態に
	}
	else
	{
		_HandleCharReturn(ec, pContext);
	}
	return S_OK;
}

//カーソル直前文字列をBackspaceを送って消した後、置換文字列を確定する。
HRESULT CTextService::_ReplacePrecedingTextIMM32(TfEditCookie ec, ITfContext *pContext, int delete_count, bool startMaze)
{
	mozc::commands::Output pending;
	mozc::win32::InputState dummy;
	pending.kana = kana;
	pending.maze = startMaze;
	_ResetStatus();
	_HandleCharReturn(ec, pContext);
	deleter.BeginDeletion(delete_count, pending, dummy);
	return E_PENDING;
}
