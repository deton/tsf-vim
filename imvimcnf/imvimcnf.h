
#ifndef IMVIMCNF_H
#define IMVIMCNF_H

#include "common.h"

//for resource
#define RC_FILE				"imvimcnf"

// imvimcnf
void CreateProperty(HINSTANCE hInst);

// ConfigCnf
void CreateConfigPath();
BOOL SetFileDacl(LPCWSTR path);
int GetScaledSizeX(HWND hwnd, int size);
void LoadPreservedKey(HWND hwnd);
void SavePreservedKey(HWND hwnd);

extern LPCWSTR TextServiceDesc;

extern IXmlWriter *pXmlWriter;
extern IStream *pXmlFileStream;

extern HINSTANCE hInst;

extern WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定

#endif //IMVIMCNF_H
