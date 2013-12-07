
#ifndef CONFIGXML_H
#define CONFIGXML_H

#define NOT_S_OK not_s_ok
#define EXIT_NOT_S_OK(hr) if((hr) != S_OK) goto NOT_S_OK

typedef std::pair<std::wstring, std::wstring> APPDATAXMLATTR;
typedef std::vector<APPDATAXMLATTR> APPDATAXMLROW;
typedef std::vector<APPDATAXMLROW> APPDATAXMLLIST;

HRESULT CreateStreamReader(LPCWSTR path, IXmlReader **ppReader, IStream **ppFileStream);
void CloseStreamReader(IXmlReader *pReader, IStream *pFileStream);

HRESULT ReadList(LPCWSTR path, LPCWSTR section, APPDATAXMLLIST &list);
HRESULT ReadValue(LPCWSTR path, LPCWSTR section, LPCWSTR key, std::wstring &strxmlval, LPCWSTR defval = L"");

HRESULT CreateStreamWriter(LPCWSTR path, IXmlWriter **ppWriter, IStream **ppFileStream);
void CloseStreamWriter(IXmlWriter *pWriter, IStream *pFileStream);

HRESULT WriterInit(LPCWSTR path, IXmlWriter **ppWriter, IStream **ppFileStream, BOOL indent = TRUE);
HRESULT WriterFinal(IXmlWriter **ppWriter, IStream **ppFileStream);

HRESULT WriterNewLine(IXmlWriter *pWriter);
HRESULT WriterStartElement(IXmlWriter *pWriter, LPCWSTR element);
HRESULT WriterEndElement(IXmlWriter *pWriter);
HRESULT WriterAttribute(IXmlWriter *pWriter, LPCWSTR name, LPCWSTR value);

HRESULT WriterStartSection(IXmlWriter *pWriter, LPCWSTR name);
HRESULT WriterEndSection(IXmlWriter *pWriter);
HRESULT WriterKey(IXmlWriter *pWriter, LPCWSTR key, LPCWSTR value);
HRESULT WriterRow(IXmlWriter *pWriter, const APPDATAXMLROW &row);
HRESULT WriterList(IXmlWriter *pWriter, const APPDATAXMLLIST &list, BOOL newline = FALSE);

//tag
extern LPCWSTR TagRoot;
extern LPCWSTR TagSection;
extern LPCWSTR TagKey;
extern LPCWSTR TagEntry;
extern LPCWSTR TagList;
extern LPCWSTR TagRow;

//attribute
extern LPCWSTR AttributeName;
extern LPCWSTR AttributeValue;
extern LPCWSTR AttributeKey;
extern LPCWSTR AttributeVKey;
extern LPCWSTR AttributeMKey;

//behavior section

extern LPCWSTR SectionBehavior;

//behavior keys

extern LPCWSTR ValueOtherIme1;
extern LPCWSTR ValueOtherIme2;

//preservedkey section

extern LPCWSTR SectionPreservedKeyNormal;
extern LPCWSTR SectionPreservedKeyOtherIme;

#endif //CONFIGXML_H
