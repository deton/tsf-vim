
#include "configxml.h"
#include "imvimcnf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcBehavior2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	size_t i;
	WCHAR num[16];
	std::wstring strxmlval;
	FILE *fp;

	switch(message)
	{
	case WM_INITDIALOG:
		hwnd = GetDlgItem(hDlg, IDC_COMBO_OTHERIME1);
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"Alt+Shift");
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"Ctrl+Shift");
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"Win+Space");
		ReadValue(pathconfigxml, SectionBehavior, ValueOtherIme1, strxmlval);
		if(strxmlval == L"Alt+Shift")
		{
			i = 0;
		}
		else if(strxmlval == L"Ctrl+Shift")
		{
			i = 1;
		}
		else if(strxmlval == L"Win+Space")
		{
			i = 2;
		}
		else
		{
			if (IsVersion62AndOver()) // Windows 8
			{
				i = 2; //default to "Win+Space"
			}
			else
			{
				i = 0; //default to "Alt+Shift"
			}
		}
		SendMessage(hwnd, CB_SETCURSEL, (WPARAM)i, 0);

		hwnd = GetDlgItem(hDlg, IDC_COMBO_OTHERIME2);
		//Alt+Shift等を指定回数送り付け
		//"*1","*2","*3","*4","*5","*6","*7","*8","*9",
		num[2] = L'\0';
		num[0] = L'*';
		for(i = 1; i <= 9; i++)
		{
			num[1] = L'0' + (WCHAR)i;
			SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		//Alt+Shift+0, Alt+Shift+1, ...を送り付け
		//"+0","+1",...,"+9"
		num[0] = L'+';
		for(i = 0; i <= 9; i++)
		{
			num[1] = L'0' + (WCHAR)i;
			SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionBehavior, ValueOtherIme2, strxmlval);
		if(strxmlval.size() < 2)
		{
			i = 0;
		}
		//  0,   1,   2,    3,   4,   5,   6,   7,   8,   9,  10,..., 18
		//"*1","*2","*3","*4","*5","*6","*7","*8","*9","+0","+1",...,"+9"
		else if(strxmlval[0] == L'*' && iswdigit(strxmlval[1]))
		{
			i = strxmlval[1] - L'1';
		}
		else if(strxmlval[0] == L'+' && iswdigit(strxmlval[1]))
		{
			i = 9 + strxmlval[1] - L'0';
		}
		else
		{
			i = 0;
		}
		SendMessage(hwnd, CB_SETCURSEL, (WPARAM)i, 0);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_COMBO_OTHERIME1:
		case IDC_COMBO_OTHERIME2:
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			_wfopen_s(&fp, pathconfigxml, L"ab");
			if(fp != NULL)
			{
				fclose(fp);
			}
			SetFileDacl(pathconfigxml);

			WriterInit(pathconfigxml, &pXmlWriter, &pXmlFileStream);

			WriterStartElement(pXmlWriter, TagRoot);

			WriterStartSection(pXmlWriter, SectionBehavior);	//Start of SectionBehavior -> End at DlgProcBehavior2

			hwnd = GetDlgItem(hDlg, IDC_COMBO_OTHERIME1);
			i = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
			WriterKey(pXmlWriter, ValueOtherIme1,
					i == 1 ? L"Ctrl+Shift" :
					i == 2 ? L"Win+Space" : L"Alt+Shift");

			hwnd = GetDlgItem(hDlg, IDC_COMBO_OTHERIME2);
			i = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
			//  0,   1,   2,    3,   4,   5,   6,   7,   8,   9,  10,..., 18
			//"*1","*2","*3","*4","*5","*6","*7","*8","*9","+0","+1",...,"+9"
			if(i < 9)
			{
				num[0] = L'*';
				num[1] = L'1' + i;
			}
			else
			{
				num[0] = L'+';
				num[1] = L'0' + i - 9;
			}
			num[2] = L'\0';
			WriterKey(pXmlWriter, ValueOtherIme2, num);

			WriterEndSection(pXmlWriter);						//End of SectionBehavior

			return TRUE;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}
