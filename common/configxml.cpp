
#include "configxml.h"

//tag
LPCWSTR TagRoot = L"tsfvim";
LPCWSTR TagSection = L"section";
LPCWSTR TagEntry = L"entry";
LPCWSTR TagKey = L"key";
LPCWSTR TagList = L"list";
LPCWSTR TagRow = L"row";

//attribute
LPCWSTR AttributeName = L"name";
LPCWSTR AttributeValue = L"value";
LPCWSTR AttributeKey = L"key";
LPCWSTR AttributeVKey = L"vkey";
LPCWSTR AttributeMKey = L"mkey";

//behavior section

LPCWSTR SectionBehavior = L"behavior";

//behavior keys

LPCWSTR ValueOtherIme1 = L"otherime1";
LPCWSTR ValueOtherIme2 = L"otherime2";

//preservedkey section

LPCWSTR SectionPreservedKeyNormal = L"preservedkeynormal";
LPCWSTR SectionPreservedKeyOtherIme = L"preservedkeyotherime";
LPCWSTR SectionPreservedKeyOtherImeOff = L"preservedkeyotherimeoff";

HRESULT CreateStreamReader(LPCWSTR path, IXmlReader **ppReader, IStream **ppFileStream)
{
	HRESULT hr = S_FALSE;

	if(ppReader != NULL && ppFileStream != NULL)
	{
		hr = CreateXmlReader(IID_PPV_ARGS(ppReader), NULL);
		EXIT_NOT_S_OK(hr);

		hr = SHCreateStreamOnFileW(path, STGM_READ, ppFileStream);
		EXIT_NOT_S_OK(hr);

		hr = (*ppReader)->SetInput(*ppFileStream);
		EXIT_NOT_S_OK(hr);
	}

NOT_S_OK:
	return hr;
}

void CloseStreamReader(IXmlReader *pReader, IStream *pFileStream)
{
	if(pReader != NULL)
	{
		pReader->Release();
		pReader = NULL;
	}
	if(pFileStream != NULL)
	{
		pFileStream->Release();
		pFileStream = NULL;
	}
}

HRESULT ReadList(LPCWSTR path, LPCWSTR section, APPDATAXMLLIST &list)
{
	HRESULT hr;
	IXmlReader *pReader = NULL;
	IStream *pFileStream = NULL;
	XmlNodeType nodeType;
	LPCWSTR pwszLocalName;
	LPCWSTR pwszAttributeName;
	LPCWSTR pwszAttributeValue;
	int sequence = 0;

	APPDATAXMLATTR attr;
	APPDATAXMLROW row;

	hr = CreateStreamReader(path, &pReader, &pFileStream);
	EXIT_NOT_S_OK(hr);

	while(pReader->Read(&nodeType) == S_OK)
	{
		switch(nodeType)
		{
		case XmlNodeType_Element:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 0:
				if(wcscmp(TagRoot, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 1:
				if(wcscmp(TagSection, pwszLocalName) == 0)
				{
					sequence = 2;
				}
				break;
			case 3:
				if(wcscmp(TagList, pwszLocalName) == 0)
				{
					sequence = 4;
				}
				break;
			case 4:
				if(wcscmp(TagRow, pwszLocalName) == 0)
				{
					sequence = 5;
					row.clear();
				}
				break;
			default:
				break;
			}

			for(hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
			{
				hr = pReader->GetLocalName(&pwszAttributeName, NULL);
				EXIT_NOT_S_OK(hr);
				hr = pReader->GetValue(&pwszAttributeValue, NULL);
				EXIT_NOT_S_OK(hr);

				switch(sequence)
				{
				case 2:
					if(wcscmp(TagSection, pwszLocalName) == 0 &&
						wcscmp(AttributeName, pwszAttributeName) == 0 && wcscmp(section, pwszAttributeValue) == 0)
					{
						sequence = 3;
					}
					break;
				case 5:
					if(wcscmp(TagRow, pwszLocalName) == 0)
					{
						attr.first = pwszAttributeName;
						attr.second = pwszAttributeValue;
						row.push_back(attr);
					}
					break;
				default:
					break;
				}
			}

			switch(sequence)
			{
			case 2:
				sequence = 1;
				break;
			case 5:
				list.push_back(row);
				row.clear();
				break;
			default:
				break;
			}
			break;

		case XmlNodeType_EndElement:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 1:
				if(wcscmp(TagRoot, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 2:
				if(wcscmp(TagSection, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 3:
				if(wcscmp(TagSection, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 4:
				if(wcscmp(TagSection, pwszLocalName) == 0 || wcscmp(TagList, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 5:
				if(wcscmp(TagList, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}

NOT_S_OK:
exit:
	CloseStreamReader(pReader, pFileStream);
	return hr;
}

HRESULT ReadValue(LPCWSTR path, LPCWSTR section, LPCWSTR key, std::wstring &strxmlval, LPCWSTR defval)
{
	IXmlReader *pReader = NULL;
	IStream *pFileStream = NULL;
	HRESULT hr;
	XmlNodeType nodeType;
	LPCWSTR pwszLocalName;
	LPCWSTR pwszAttributeName;
	LPCWSTR pwszAttributeValue;
	int sequence = 0;

	strxmlval = defval;

	hr = CreateStreamReader(path, &pReader, &pFileStream);
	EXIT_NOT_S_OK(hr);

	while(pReader->Read(&nodeType) == S_OK)
	{
		switch(nodeType)
		{
		case XmlNodeType_Element:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 0:
				if(wcscmp(TagRoot, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 1:
				if(wcscmp(TagSection, pwszLocalName) == 0)
				{
					sequence = 2;
				}
				break;
			case 3:
				if(wcscmp(TagKey, pwszLocalName) == 0)
				{
					sequence = 4;
				}
				break;
			default:
				break;
			}

			for(hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
			{
				hr = pReader->GetLocalName(&pwszAttributeName, NULL);
				EXIT_NOT_S_OK(hr);
				hr = pReader->GetValue(&pwszAttributeValue, NULL);
				EXIT_NOT_S_OK(hr);

				switch(sequence)
				{
				case 2:
					if(wcscmp(AttributeName, pwszAttributeName) == 0 && wcscmp(section, pwszAttributeValue) == 0)
					{
						sequence = 3;
					}
					break;
				case 4:
					if(wcscmp(AttributeName, pwszAttributeName) == 0 && wcscmp(key, pwszAttributeValue) == 0)
					{
						sequence = 5;
					}
					break;
				case 5:
					if(wcscmp(AttributeValue, pwszAttributeName) == 0)
					{
						strxmlval.assign(pwszAttributeValue);
						goto exit;
					}
					break;
				default:
					break;
				}
			}
			break;

		case XmlNodeType_EndElement:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 1:
				if(wcscmp(TagRoot, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 2:
				if(wcscmp(TagSection, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 3:
			case 4:
			case 5:
				if(wcscmp(TagSection, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}

NOT_S_OK:
exit:
	CloseStreamReader(pReader, pFileStream);
	return hr;
}

HRESULT CreateStreamWriter(LPCWSTR path, IXmlWriter **ppWriter, IStream **ppFileStream)
{
	HRESULT hr = S_FALSE;

	if(ppWriter != NULL && ppFileStream != NULL)
	{
		hr = CreateXmlWriter(IID_PPV_ARGS(ppWriter), NULL);
		EXIT_NOT_S_OK(hr);

		hr = SHCreateStreamOnFileW(path, STGM_WRITE | STGM_CREATE, ppFileStream);
		EXIT_NOT_S_OK(hr);

		hr = (*ppWriter)->SetOutput(*ppFileStream);
	}

NOT_S_OK:
	return hr;
}

void CloseStreamWriter(IXmlWriter *pWriter, IStream *pFileStream)
{
	if(pWriter != NULL)
	{
		pWriter->Release();
		pWriter = NULL;
	}
	if(pFileStream != NULL)
	{
		pFileStream->Release();
		pFileStream = NULL;
	}
}

HRESULT WriterInit(LPCWSTR path, IXmlWriter **ppWriter, IStream **pFileStream, BOOL indent)
{
	HRESULT hr = S_FALSE;

	if(ppWriter != NULL && pFileStream != NULL)
	{
		hr = CreateStreamWriter(path, ppWriter, pFileStream);
		EXIT_NOT_S_OK(hr);

		hr = (*ppWriter)->SetProperty(XmlWriterProperty_Indent, indent);
		EXIT_NOT_S_OK(hr);

		hr = (*ppWriter)->WriteStartDocument(XmlStandalone_Omit);
		EXIT_NOT_S_OK(hr);
	}

NOT_S_OK:
	return hr;
}

HRESULT WriterFinal(IXmlWriter **ppWriter, IStream **ppFileStream)
{
	HRESULT hr = S_FALSE;

	if(ppWriter != NULL && *ppWriter != NULL)
	{
		hr = (*ppWriter)->WriteEndDocument();
		EXIT_NOT_S_OK(hr);

		hr = (*ppWriter)->Flush();
	}

NOT_S_OK:
	if(ppWriter != NULL && ppFileStream != NULL)
	{
		CloseStreamWriter(*ppWriter, *ppFileStream);
	}
	return hr;
}

HRESULT WriterNewLine(IXmlWriter *pWriter)
{
	HRESULT hr = S_FALSE;

	if(pWriter != NULL)
	{
		hr = pWriter->WriteRaw(L"\r\n");
	}

	return hr;
}

HRESULT WriterStartElement(IXmlWriter *pWriter, LPCWSTR element)
{
	HRESULT hr = S_FALSE;

	if(pWriter != NULL)
	{
		hr = pWriter->WriteStartElement(NULL, element, NULL);
	}

	return hr;
}

HRESULT WriterEndElement(IXmlWriter *pWriter)
{
	HRESULT hr = S_FALSE;

	if(pWriter != NULL)
	{
		hr = pWriter->WriteEndElement();
	}

	return hr;
}

HRESULT WriterAttribute(IXmlWriter *pWriter, LPCWSTR name, LPCWSTR value)
{
	HRESULT hr = S_FALSE;

	if(pWriter != NULL)
	{
		hr = pWriter->WriteAttributeString(NULL, name, NULL, value);
	}

	return hr;
}

HRESULT WriterStartSection(IXmlWriter *pWriter, LPCWSTR name)
{
	HRESULT hr;

	hr = WriterStartElement(pWriter, TagSection);
	EXIT_NOT_S_OK(hr);

	hr = WriterAttribute(pWriter, AttributeName, name);

NOT_S_OK:
	return hr;
}

HRESULT WriterEndSection(IXmlWriter *pWriter)
{
	return WriterEndElement(pWriter);
}

HRESULT WriterKey(IXmlWriter *pWriter, LPCWSTR key, LPCWSTR value)
{
	HRESULT hr = S_FALSE;

	if(pWriter != NULL)
	{
		hr = WriterStartElement(pWriter, TagKey);
		EXIT_NOT_S_OK(hr);

		hr = WriterAttribute(pWriter, AttributeName, key);
		EXIT_NOT_S_OK(hr);

		hr = WriterAttribute(pWriter, AttributeValue, value);
		EXIT_NOT_S_OK(hr);

		hr = WriterEndElement(pWriter);	//key
	}

NOT_S_OK:
	return hr;
}

HRESULT WriterRow(IXmlWriter *pWriter, const APPDATAXMLROW &row)
{
	HRESULT hr;
	APPDATAXMLROW::const_iterator r_itr;

	for(r_itr = row.begin(); r_itr != row.end(); r_itr++)
	{
		hr = WriterAttribute(pWriter, r_itr->first.c_str(), r_itr->second.c_str());
		EXIT_NOT_S_OK(hr);
	}

NOT_S_OK:
	return hr;
}

HRESULT WriterList(IXmlWriter *pWriter, const APPDATAXMLLIST &list, BOOL newline)
{
	HRESULT hr;
	APPDATAXMLLIST::const_iterator l_itr;

	hr = WriterStartElement(pWriter, TagList);
	EXIT_NOT_S_OK(hr);

	if(newline)
	{
		hr = WriterNewLine(pWriter);
		EXIT_NOT_S_OK(hr);
	}

	for(l_itr = list.begin(); l_itr != list.end(); l_itr++)
	{
		hr = WriterStartElement(pWriter, TagRow);
		EXIT_NOT_S_OK(hr);

		hr = WriterRow(pWriter, *l_itr);
		EXIT_NOT_S_OK(hr);

		hr = WriterEndElement(pWriter);	//row
		EXIT_NOT_S_OK(hr);

		if(newline)
		{
			hr = WriterNewLine(pWriter);
			EXIT_NOT_S_OK(hr);
		}
	}

	hr = WriterEndElement(pWriter);	//list
	EXIT_NOT_S_OK(hr);

NOT_S_OK:
	return hr;
}
