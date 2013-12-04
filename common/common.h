
#ifndef COMMON_H
#define COMMON_H

#define TEXTSERVICE_NAME	L"tsf-vim"
#define TEXTSERVICE_VER		L"0.0.1"

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

//for resource
#define RC_AUTHOR			"KIHARA Hideto"
#define RC_PRODUCT			"tsf-vim"
#define RC_VERSION			"0.0.1"
#define RC_VERSION_D		0,0,1,0

#define MAX_KRNLOBJNAME		256
#define MAX_PRESERVEDKEY	8

#define VIMCNFEXE		L"imvimcnf.exe"
#ifndef _DEBUG
#define VIMKRNLOBJ		L"tsf-vim-"
#else
#define VIMKRNLOBJ		L"tsf-vim-debug-"
#endif
#define VIMCNFMUTEX		VIMKRNLOBJ L"cnf-"

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

extern LPCWSTR fnconfigxml;	//設定

BOOL IsVersion62AndOver();
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
BOOL GetUserSid(LPWSTR *ppszUserSid);

#endif //COMMON_H
