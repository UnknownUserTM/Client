/*********************************************************************
* title_name		: Language System
* date_created		: 2017.10.21
* filename			: PythonLanguage.cpp
* author			: VegaS
* version_actual	: Version 0.1.0
*/

#include "StdAfx.h"
#include "Locale.h"
#include "PythonApplication.h"
#include "PythonNonPlayer.h"
#include "PythonLanguage.h"
#include "PythonChat.h"
#include "PythonGuild.h"
#include "PythonCharacterManager.h"
#include "PythonPlayer.h"
#include "PythonBackground.h"
#include "PythonTextTail.h"
#include "PythonItem.h"
#include "PythonEventManager.h"
#include "Locale_inc.h"
#include "AbstractApplication.h"
#include "AbstractCharacterManager.h"
#include "InstanceBase.h"
#include "PythonNetworkStream.h"
#include "ProcessCRC.h"
#include <Windows.h>
#include "../gamelib/ActorInstance.h"
#include "../gamelib/ItemManager.h"
#include <Shellapi.h>

/*******************************************************************\
|				CPythonLanguageManager - CLASS						|
\*******************************************************************/

extern bool SetDefaultCodePage(DWORD codePage);
static char c_pszLanguageClient[256] = LANGUAGE_DEFAULT;

TLanguageInitInfo kLanguageInitInfo[LANGUAGE_MAX_NUM] =
{
	{ 0, "en", 1252 }, // 1252
	{ 1, "de", 1252 }, // 1252
};

static const DWORD kMobNameFromServerInfo[] =
{
	0
};

/*******************************************************************\
| [PUBLIC] Load Mobs Names From Client or Server By List

void CPythonLanguageManager::LoadConfigMobNameFromServer()
{
	for (int i = 0; i < _countof(kMobNameFromServerInfo); i++)
		m_vecMobNameFromServer.push_back(kMobNameFromServerInfo[i]);
}

bool CPythonLanguageManager::GetMobNameFromServer(DWORD dwRace)
{
	if (m_vecMobNameFromServer.empty())
		return false;
	
	return std::find(m_vecMobNameFromServer.begin(), m_vecMobNameFromServer.end(), dwRace) != m_vecMobNameFromServer.end();
}
\*******************************************************************/

const char * CPythonLanguageManager::GetMobNameFromClient(BYTE bType, DWORD dwRace, const char * c_szOriginalName)
{
	if (bType == CActorInstance::TYPE_NPC)
	{
		const char * c_szConvertedName;
		if (CPythonNonPlayer::Instance().GetName(dwRace, &c_szConvertedName))
			return c_szConvertedName;
	}
	
	return c_szOriginalName;
}

/*******************************************************************\
| [PUBLIC] Get Items Names From Client
\*******************************************************************/
void CPythonLanguageManager::RecvPickupItemPacket(const char * c_pszBuf, DWORD dwVnum)
{
	CItemManager::Instance().SelectItemData(dwVnum);
	CItemData * pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	if (!pItemData)
		return;

	static char c_pszLine[CHAT_MAX_NUM * 4 + 1];
	_snprintf(c_pszLine, sizeof(c_pszLine), "%s%s", c_pszBuf, pItemData->GetName());
	CPythonChat::Instance().AppendChat(CHAT_TYPE_INFO, c_pszLine);
}

/*******************************************************************\
| [PUBLIC] General Functions
\*******************************************************************/

CPythonLanguageManager::CPythonLanguageManager()
{
}

CPythonLanguageManager::~CPythonLanguageManager()
{
}

void CPythonLanguageManager::SetConfigCodePage(const char * c_pszLanguage)
{
	DWORD codePage = GetDefaultCodePage(c_pszLanguage);
	SetDefaultCodePage(codePage);
	
	FILE * file = fopen(FILE_NAME_MAIN_LOCALE, "w");
	if (!file)
		return;

	fprintf(file, "10002 %d %s", codePage, MAIN_LOCALE_LANGUAGE);
	fclose(file);
}

void CPythonLanguageManager::SetConfigLanguage(const char * c_pszLanguage)
{
	FILE * file = fopen(FILE_NAME_LANGUAGE, "w");
	if (!file)
		return;

	fprintf(file, c_pszLanguage);
	fclose(file);

	strcpy(c_pszLanguageClient, c_pszLanguage);
	SetConfigCodePage(c_pszLanguageClient);
}

void CPythonLanguageManager::LoadConfigLanguage()
{
	char line[256];
	char c_pszLanguage[256];

	FILE * fp = fopen(FILE_NAME_LANGUAGE, "rt");
	if (!fp)
	{
		SetConfigLanguage(LANGUAGE_DEFAULT);
		return;
	}

	if (fgets(line, sizeof(line) - 1, fp))
	{
		line[sizeof(line) - 1] = '\0';
		if (sscanf(line, "%s", c_pszLanguage))
			strcpy(c_pszLanguageClient, c_pszLanguage);
	}

	fclose(fp);
}

const char * CPythonLanguageManager::Get()
{
	return c_pszLanguageClient;
}

void CPythonLanguageManager::Set(const char * c_pszLanguage)
{
	if (!strcmp(c_pszLanguage, Get()))
		return;

	strcpy(c_pszLanguageClient, c_pszLanguage);
	SetConfigLanguage(c_pszLanguage);
	
	char szModuleName[MAX_PATH + 1];
	GetModuleFileName(GetModuleHandle(NULL), szModuleName, sizeof(szModuleName));
	ShellExecute(NULL, "open", szModuleName, NULL, NULL, SW_SHOWNORMAL);
	PostQuitMessage(NULL);
}

int CPythonLanguageManager::GetDefaultCodePage(const char * c_pszLanguage)
{
	for (int i = 0; i < LANGUAGE_MAX_NUM; i++)
	{
		if (!strcmp(c_pszLanguage, kLanguageInitInfo[i].language))
			return kLanguageInitInfo[i].codePage;
	}
	
	return 1252;
}

PyObject* appGetLanguage(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("s", CPythonLanguageManager::Instance().Get());
}

PyObject* appSetLanguage(PyObject* poSelf, PyObject* poArgs)
{
	char* c_pszLanguage;
	if (!PyTuple_GetString(poArgs, 0, &c_pszLanguage))
		return Py_BuildException();

	CPythonLanguageManager::Instance().Set(c_pszLanguage);
	return Py_BuildNone();
}

PyObject* appGetLanguageList(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* pyObjectList = PyList_New(0);
	
	for (int i = 0; i < LANGUAGE_MAX_NUM; i++)
	{
		PyObject* pyLanguageName = PyString_FromString(kLanguageInitInfo[i].language);
		PyList_Append(pyObjectList, pyLanguageName);
	}

	return pyObjectList;
}

PyObject* appGetLanguageIndex(PyObject* poSelf, PyObject* poArgs)
{
	char* c_pszLanguage;
	if (!PyTuple_GetString(poArgs, 0, &c_pszLanguage))
		return Py_BuildException();
	
	for (int i = 0; i < LANGUAGE_MAX_NUM; i++)
	{
		if (!strcmp(c_pszLanguage, kLanguageInitInfo[i].language))
			return Py_BuildValue("i", kLanguageInitInfo[i].index);
	}

	return Py_BuildNone();
}

PyObject* appSendShoutLanguageFilterPacket(PyObject* poSelf, PyObject* poArgs)
{
	int bSubHeader;
	if (!PyTuple_GetInteger(poArgs, 0, &bSubHeader))
		return Py_BuildException();
	
	int iType;
	if (!PyTuple_GetInteger(poArgs, 1, &iType))
		return Py_BuildException();
	
	char * szText;
	if (!PyTuple_GetString(poArgs, 2, &szText))
		return Py_BuildException();
	
	PyObject * poDict = PyTuple_GetItem(poArgs, 3);
	if (!PyTuple_Check(poDict))
		return Py_BuildException();

	BYTE bFilterList[LANGUAGE_MAX_NUM + 1];
	for (int i = 0; i < LANGUAGE_MAX_NUM + 1; ++i)
		bFilterList[i] = PyInt_AsLong(PyTuple_GetItem(poDict, i));
	
	CPythonNetworkStream & rkNetStream = CPythonNetworkStream::Instance();
	rkNetStream.SendShoutLanguageFilterPacket(bSubHeader, iType, szText, bFilterList);
	return Py_BuildNone();
}

void initLanguageManager()
{
	static PyMethodDef s_methods[] = {
		{ "GetLanguage",				appGetLanguage,					METH_VARARGS },
		{ "SetLanguage",				appSetLanguage,					METH_VARARGS },
		{ "GetLanguageList",			appGetLanguageList,				METH_VARARGS },
		{ "GetLanguageIndex",			appGetLanguageIndex,			METH_VARARGS },
		{ "SendShoutLanguageFilterPacket",	appSendShoutLanguageFilterPacket,		METH_VARARGS },		
		{ NULL,						NULL,							NULL }
	};
	
	PyObject* poModule = Py_InitModule("app", s_methods);
	
	PyModule_AddIntConstant(poModule, "ENABLE_LANGUAGE_SYSTEM", 1);
	
	PyModule_AddStringConstant(poModule, "LANGUAGE_DEFAULT",					LANGUAGE_DEFAULT);
	PyModule_AddIntConstant(poModule, "LANGUAGE_MAX_NUM",						LANGUAGE_MAX_NUM);

	PyModule_AddIntConstant(poModule, "LANGUAGE_UNSELECTED",					LANGUAGE_UNSELECTED);
	PyModule_AddIntConstant(poModule, "LANGUAGE_SELECTED",						LANGUAGE_SELECTED);

	PyModule_AddIntConstant(poModule, "CLIENT_FROM_LOGIN_MODULE",				CLIENT_FROM_LOGIN_MODULE);
	PyModule_AddIntConstant(poModule, "CLIENT_FROM_GAME_MODULE",				CLIENT_FROM_GAME_MODULE);
	PyModule_AddIntConstant(poModule, "CLIENT_FROM_INTERFACE_MODULE",			CLIENT_FROM_INTERFACE_MODULE);

	PyModule_AddIntConstant(poModule, "LANGUAGE_SUB_HEADER_FILTER_CHAT",		LANGUAGE_SUB_HEADER_FILTER_CHAT);
	PyModule_AddIntConstant(poModule, "LANGUAGE_SUB_HEADER_GM_ANNOUNCEMENT",	LANGUAGE_SUB_HEADER_GM_ANNOUNCEMENT);	
	
	PyModule_AddIntConstant(poModule, "NOTICE_MAX_LEN",							NOTICE_MAX_LEN);
	PyModule_AddIntConstant(poModule, "BIG_NOTICE_MAX_LEN",						BIG_NOTICE_MAX_LEN);	
	PyModule_AddIntConstant(poModule, "MAP_NOTICE_MAX_LEN",						MAP_NOTICE_MAX_LEN);
	PyModule_AddIntConstant(poModule, "WHISHPER_MAX_LEN",						WHISHPER_MAX_LEN);

	PyModule_AddStringConstant(poModule, "MAIN_LOCALE_LANGUAGE",				MAIN_LOCALE_LANGUAGE);	
	PyModule_AddStringConstant(poModule, "LANGUAGE_INDEX_GLOBAL",				LANGUAGE_GLOBAL);
	PyModule_AddStringConstant(poModule, "EMPTY_STRING",						"");
}