/*********************************************************************
* title_name		: Language System
* date_created		: 2017.10.21
* filename			: PythonLanguage.h
* author			: VegaS
* version_actual	: Version 0.1.0
*/
#pragma once

#define LANGUAGE_DEFAULT "en"
#define LANGUAGE_GLOBAL "gl"
#define MAIN_LOCALE_LANGUAGE "en"
#define FILE_NAME_MAIN_LOCALE "locale.cfg"
#define FILE_NAME_LANGUAGE "language.cfg"

enum ELanguageInfoArgs
{
	CLIENT_FROM_LOGIN_MODULE = 0,
	CLIENT_FROM_GAME_MODULE = 1,
	CLIENT_FROM_INTERFACE_MODULE = 2,
	
	LANGUAGE_UNSELECTED = 0,
	LANGUAGE_SELECTED = 1,
	
	LANGUAGE_SUB_HEADER_FILTER_CHAT = 0,
	LANGUAGE_SUB_HEADER_GM_ANNOUNCEMENT = 1,
	
	NOTICE_MAX_LEN = 135,
	BIG_NOTICE_MAX_LEN = 120,	
	MAP_NOTICE_MAX_LEN = 120,
	WHISHPER_MAX_LEN = 450,
};

struct TLanguageInitInfo
{
	int index;
	const char * language;
	int codePage;
};

class CPythonLanguageManager : public CSingleton < CPythonLanguageManager >
{
public:
	CPythonLanguageManager();
	virtual ~CPythonLanguageManager();

	public:
		const char * Get();
		void Set(const char * c_pszLanguage);

		int GetDefaultCodePage(const char * c_pszLanguage);
		void SetConfigLanguage(const char * c_pszLanguage);
		void SetConfigCodePage(const char * c_pszLanguage);
		void LoadConfigLanguage();

		const char * GetMobNameFromClient(BYTE bType, DWORD dwRace, const char * c_szOriginalName);
		void RecvPickupItemPacket(const char * c_pszBuf, DWORD dwVnum);

};