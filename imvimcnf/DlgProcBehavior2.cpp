
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
		SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)L"");
		num[1] = L'\0';
		for(i = 0; i <= 9; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionBehavior, ValueOtherIme2, strxmlval);
		i = strxmlval.empty() ? 0 : _wtoi(strxmlval.c_str()) + 1;
		if(i > 10)
		{
			i = 10;
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
			if(i == 0)
			{
				WriterKey(pXmlWriter, ValueOtherIme2, L"");
			}
			else
			{
				num[0] = L'0' + i - 1;
				num[1] = L'\0';
				WriterKey(pXmlWriter, ValueOtherIme2, num);
			}

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
