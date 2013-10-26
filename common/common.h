
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

#define MAX_PRESERVEDKEY	8

BOOL IsVersion6AndOver();
BOOL IsVersion62AndOver();

#endif //COMMON_H
