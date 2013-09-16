
#include "imcrvtip.h"

HINSTANCE g_hInst;

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;
LPCWSTR LangbarItemDesc = L"ver. " TEXTSERVICE_VER;
LPCWSTR LangbarFuncDesc = TEXTSERVICE_DESC L" " TEXTSERVICE_VER;

// for Windows 8
#if 1
const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT =
{ 0x13A016DF, 0x560B, 0x46CD, { 0x94, 0x7A, 0x4C, 0x3A, 0xF1, 0xE0, 0xE3, 0x5D } };
const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT =
{ 0x25504FB4, 0x7BAB, 0x4BC1, { 0x9C, 0x69, 0xCF, 0x81, 0x89, 0x0F, 0x0E, 0xF5 } };
const GUID GUID_LBI_INPUTMODE =
{ 0x2C77A81E, 0x41CC, 0x4178, { 0xA3, 0xA7, 0x5F, 0x8A, 0x98, 0x75, 0x68, 0xE6 } };
#endif

#ifndef _DEBUG

// {75651692-5DBE-4f19-ACF7-E51848CDCAB1}
const GUID c_clsidTextService = 
{ 0x75651692, 0x5dbe, 0x4f19, { 0xac, 0xf7, 0xe5, 0x18, 0x48, 0xcd, 0xca, 0xb1 } };

// {5610A424-C061-4009-BCB8-39C24424D54E}
const GUID c_guidProfile = 
{ 0x5610a424, 0xc061, 0x4009, { 0xbc, 0xb8, 0x39, 0xc2, 0x44, 0x24, 0xd5, 0x4e } };

// {128434DD-F967-4d5b-9877-64FA4AC9DA35}
const GUID c_guidPreservedKeyOn = 
{ 0x128434dd, 0xf967, 0x4d5b, { 0x98, 0x77, 0x64, 0xfa, 0x4a, 0xc9, 0xda, 0x35 } };

// {CC3C23D8-9E40-4135-80BB-576519CFBEAC}
const GUID c_guidPreservedKeyOff = 
{ 0xcc3c23d8, 0x9e40, 0x4135, { 0x80, 0xbb, 0x57, 0x65, 0x19, 0xcf, 0xbe, 0xac } };

// {6FC1EF95-792F-4948-B70E-4008175A6A75}
const GUID c_guidPreservedKeyOnOff = 
{ 0x6fc1ef95, 0x792f, 0x4948, { 0xb7, 0xe, 0x40, 0x8, 0x17, 0x5a, 0x6a, 0x75 } };

// {5A18B688-A76C-4c07-A095-BEE6DB141D6D}
const GUID c_guidLangBarItemButton = 
{ 0x5a18b688, 0xa76c, 0x4c07, { 0xa0, 0x95, 0xbe, 0xe6, 0xdb, 0x14, 0x1d, 0x6d } };

// {9F977779-B922-421a-9E91-F3C7462CB2DA}
const GUID c_guidDisplayAttributeInput = 
{ 0x9f977779, 0xb922, 0x421a, { 0x9e, 0x91, 0xf3, 0xc7, 0x46, 0x2c, 0xb2, 0xda } };

// {97CC7D92-1BF5-4e7d-9994-B7802D2DE0C4}
const GUID c_guidDisplayAttributeCandidate = 
{ 0x97cc7d92, 0x1bf5, 0x4e7d, { 0x99, 0x94, 0xb7, 0x80, 0x2d, 0x2d, 0xe0, 0xc4 } };

// {CBAEABB9-CBDC-4e46-A451-F9545172BE16}
const GUID c_guidDisplayAttributeAnnotation = 
{ 0xcbaeabb9, 0xcbdc, 0x4e46, { 0xa4, 0x51, 0xf9, 0x54, 0x51, 0x72, 0xbe, 0x16 } };

// {8CB23002-E3F4-47ff-B035-46CF48FD5586}
const GUID c_guidCandidateListUIElement = 
{ 0x8cb23002, 0xe3f4, 0x47ff, { 0xb0, 0x35, 0x46, 0xcf, 0x48, 0xfd, 0x55, 0x86 } };

#else

// {4D97960C-1D59-4466-BEFE-4C1328D2550D}
const GUID c_clsidTextService = 
{ 0x4d97960c, 0x1d59, 0x4466, { 0xbe, 0xfe, 0x4c, 0x13, 0x28, 0xd2, 0x55, 0x0d } };

// {820E9894-024B-4bd1-98AF-3942B772CFF1}
const GUID c_guidProfile = 
{ 0x820e9894, 0x024b, 0x4bd1, { 0x98, 0xaf, 0x39, 0x42, 0xb7, 0x72, 0xcf, 0xf1 } };

// {D1930150-790A-437b-88B5-EB3E9FB9165F}
const GUID c_guidPreservedKeyOnOff = 
{ 0xd1930150, 0x790a, 0x437b, { 0x88, 0xb5, 0xeb, 0x3e, 0x9f, 0xb9, 0x16, 0x5f } };

// {F4BF0D3C-D4CE-456f-837E-FE6712C6A8C3}
const GUID c_guidLangBarItemButton = 
{ 0xf4bf0d3c, 0xd4ce, 0x456f, { 0x83, 0x7e, 0xfe, 0x67, 0x12, 0xc6, 0xa8, 0xc3 } };

// {6F99E3F1-36AC-4015-B334-211CFFCB3262}
const GUID c_guidDisplayAttributeInput = 
{ 0x6f99e3f1, 0x36ac, 0x4015, { 0xb3, 0x34, 0x21, 0x1c, 0xff, 0xcb, 0x32, 0x62 } };

// {6877D302-1C51-4ba4-9329-2F80B5E3A4E7}
const GUID c_guidDisplayAttributeCandidate = 
{ 0x6877d302, 0x1c51, 0x4ba4, { 0x93, 0x29, 0x2f, 0x80, 0xb5, 0xe3, 0xa4, 0xe7 } };

// {C6040719-6FF3-4b92-A589-36E93BFD53EC}
const GUID c_guidDisplayAttributeAnnotation = 
{ 0xc6040719, 0x6ff3, 0x4b92, { 0xa5, 0x89, 0x36, 0xe9, 0x3b, 0xfd, 0x53, 0xec } };

// {25A6388F-D3CB-4866-A2C3-94E00970BF45}
const GUID c_guidCandidateListUIElement = 
{ 0x25a6388f, 0xd3cb, 0x4866, { 0xa2, 0xc3, 0x94, 0xe0, 0x09, 0x70, 0xbf, 0x45 } };

#endif

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInput =
{
	{TF_CT_NONE, 0},			// TF_DA_COLOR crText;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crBk;
	TF_LS_DOT,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{ TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_INPUT				// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeCandidate =
{
	{TF_CT_NONE, 0},			// TF_DA_COLOR crText;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crBk;
	TF_LS_SOLID,				// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_TARGET_CONVERTED	// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeAnnotation =
{
	{TF_CT_NONE, 0},			// TF_DA_COLOR crText;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crBk;
	TF_LS_DASH,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_CONVERTED			// TF_DA_ATTR_INFO bAttr;
};
