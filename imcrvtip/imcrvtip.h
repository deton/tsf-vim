
#ifndef IMCRVTIP_H
#define IMCRVTIP_H

#include "common.h"

extern LPCWSTR TextServiceDesc;
extern LPCWSTR LangbarItemDesc;

extern HINSTANCE g_hInst;

extern const CLSID c_clsidTextService;
extern const GUID c_guidProfile;
extern const GUID c_guidPreservedKeyNormal;
extern const GUID c_guidPreservedKeyOtherIme;
extern const GUID c_guidPreservedKeyOtherImeOff;
extern const GUID c_guidLangBarItemButton;

LONG DllAddRef();
LONG DllRelease();

#define IID_IUNK_ARGS(pType) __uuidof(*(pType)), (IUnknown *)pType

// for Windows 8
#if 1
#define EVENT_OBJECT_IME_SHOW               0x8027
#define EVENT_OBJECT_IME_HIDE               0x8028
#define EVENT_OBJECT_IME_CHANGE             0x8029

#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_LBI_INPUTMODE;
#endif

#endif //IMCRVTIP_H
