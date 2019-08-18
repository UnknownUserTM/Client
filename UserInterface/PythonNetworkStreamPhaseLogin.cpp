#include "StdAfx.h"
#include "PythonNetworkStream.h"
#include "Packet.h"
#include "Test.h"
#include "AccountConnector.h"

#include "Hackshield.h"
#include "WiseLogicXTrap.h"
#include <comdef.h>

#define llI if
#define l1I else
// Login ---------------------------------------------------------------------------
void CPythonNetworkStream::LoginPhase()
{
	TPacketHeader header;
	if (!CheckPacket(&header))
		return;

	switch (header)
	{
		case HEADER_GC_PHASE:
			if (RecvPhasePacket())
				return;
			break;

		case HEADER_GC_LOGIN_SUCCESS3:
			if (__RecvLoginSuccessPacket3())
				return;
			break;
		case HEADER_GC_LOGIN_SUCCESS4:
			if (__RecvLoginSuccessPacket4())
				return;
			break;


		case HEADER_GC_LOGIN_FAILURE:
			if (__RecvLoginFailurePacket())
				return;
			break;

		case HEADER_GC_EMPIRE:
			if (__RecvEmpirePacket())
				return;
			break;

		case HEADER_GC_CHINA_MATRIX_CARD:
			if (__RecvChinaMatrixCardPacket())
				return;
			break;

		case HEADER_GC_RUNUP_MATRIX_QUIZ:
			if (__RecvRunupMatrixQuizPacket())
				return;
			break;

		case HEADER_GC_NEWCIBN_PASSPOD_REQUEST:
			if (__RecvNEWCIBNPasspodRequestPacket())
				return;
			break;
		case HEADER_GC_NEWCIBN_PASSPOD_FAILURE:
			if (__RecvNEWCIBNPasspodFailurePacket())
				return;
			break;


		case HEADER_GC_LOGIN_KEY:
			if (__RecvLoginKeyPacket())
				return;
			break;

		case HEADER_GC_PING:
			if (RecvPingPacket())
				return;
			break;

		case HEADER_GC_HYBRIDCRYPT_KEYS:
			RecvHybridCryptKeyPacket();
			return;
			break;

		case HEADER_GC_HYBRIDCRYPT_SDB:
			RecvHybridCryptSDBPacket();
			return;
			break;

		default:
			if (RecvDefaultPacket(header))
				return;
			break;
	}

	RecvErrorPacket(header);
}

void CPythonNetworkStream::SetLoginPhase()
{
	const char* key = LocaleService_GetSecurityKey();
#ifndef _IMPROVED_PACKET_ENCRYPTION_
	SetSecurityMode(true, key);
#endif

	if ("Login" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Login Phase ##");
	Tracen("");

	m_strPhase = "Login";

	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoginPhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoginPhase);

	m_dwChangingPhaseTime = ELTimer_GetMSec();

	if (__DirectEnterMode_IsSet())
	{
		if (0 != m_dwLoginKey)
			SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());
		else
			SendLoginPacket(m_stID.c_str(), m_stPassword.c_str());

		// 비밀번호를 메모리에 계속 갖고 있는 문제가 있어서, 사용 즉시 날리는 것으로 변경
		ClearLoginInfo();
		CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
		rkAccountConnector.ClearLoginInfo();
	}
	else
	{
		if (0 != m_dwLoginKey)
			SendLoginPacketNew(m_stID.c_str(), m_stPassword.c_str());
		else
			SendLoginPacket(m_stID.c_str(), m_stPassword.c_str());

		// 비밀번호를 메모리에 계속 갖고 있는 문제가 있어서, 사용 즉시 날리는 것으로 변경
		ClearLoginInfo();
		CAccountConnector & rkAccountConnector = CAccountConnector::Instance();
		rkAccountConnector.ClearLoginInfo();

		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginStart", Py_BuildValue("()"));

		__ClearSelectCharacterData();
	}
}

bool CPythonNetworkStream::__RecvEmpirePacket()
{
	TPacketGCEmpire kPacketEmpire;
	if (!Recv(sizeof(kPacketEmpire), &kPacketEmpire))
		return false;

	m_dwEmpireID=kPacketEmpire.bEmpire;
	return true;
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket3()
{
	TPacketGCLoginSuccess3 kPacketLoginSuccess;

	if (!Recv(sizeof(kPacketLoginSuccess), &kPacketLoginSuccess))
		return false;

	for (int i = 0; i<PLAYER_PER_ACCOUNT3; ++i)
	{
		m_akSimplePlayerInfo[i]=kPacketLoginSuccess.akSimplePlayerInformation[i];
		m_adwGuildID[i]=kPacketLoginSuccess.guild_id[i];
		m_astrGuildName[i]=kPacketLoginSuccess.guild_name[i];
	}

	m_kMarkAuth.m_dwHandle=kPacketLoginSuccess.handle;
	m_kMarkAuth.m_dwRandomKey=kPacketLoginSuccess.random_key;

	if (__DirectEnterMode_IsSet())
	{
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "Refresh", Py_BuildValue("()"));
	}

	return true;
}

bool CPythonNetworkStream::__RecvLoginSuccessPacket4()
{
	TPacketGCLoginSuccess4 kPacketLoginSuccess;

	if (!Recv(sizeof(kPacketLoginSuccess), &kPacketLoginSuccess))
		return false;

	for (int i = 0; i<PLAYER_PER_ACCOUNT4; ++i)
	{
		m_akSimplePlayerInfo[i]=kPacketLoginSuccess.akSimplePlayerInformation[i];
		m_adwGuildID[i]=kPacketLoginSuccess.guild_id[i];
		m_astrGuildName[i]=kPacketLoginSuccess.guild_name[i];
	}

	m_kMarkAuth.m_dwHandle=kPacketLoginSuccess.handle;
	m_kMarkAuth.m_dwRandomKey=kPacketLoginSuccess.random_key;

	if (__DirectEnterMode_IsSet())
	{
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_SELECT], "Refresh", Py_BuildValue("()"));
	}

	return true;
}


void CPythonNetworkStream::OnConnectFailure()
{
	if (__DirectEnterMode_IsSet())
	{
		ClosePhase();
	}
	else
	{
		PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnConnectFailure", Py_BuildValue("()"));
	}
}


bool CPythonNetworkStream::__RecvLoginFailurePacket()
{
	TPacketGCLoginFailure packet_failure;
	if (!Recv(sizeof(TPacketGCLoginFailure), &packet_failure))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnLoginFailure", Py_BuildValue("(s)", packet_failure.szStatus));
#ifdef _DEBUG
	Tracef(" RecvLoginFailurePacket : [%s]\n", packet_failure.szStatus);
#endif
	return true;
}

bool CPythonNetworkStream::SendDirectEnterPacket(const char* c_szID, const char* c_szPassword, UINT uChrSlot)
{
	TPacketCGDirectEnter kPacketDirectEnter;
	kPacketDirectEnter.bHeader=HEADER_CG_DIRECT_ENTER;
	kPacketDirectEnter.index=uChrSlot;
	strncpy(kPacketDirectEnter.login, c_szID, ID_MAX_NUM);
	strncpy(kPacketDirectEnter.passwd, c_szPassword, PASS_MAX_NUM);

	if (!Send(sizeof(kPacketDirectEnter), &kPacketDirectEnter))
	{
		Tracen("SendDirectEnter");
		return false;
	}

	return SendSequence();
}
#define BOB_PROTECTION
#ifdef BOB_PROTECTION
typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct LDR_DATA_ENTRY
{
	LIST_ENTRY              InMemoryOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	ULONG                   SizeOfImage;
	UNICODE_STRING          FullDllName;
	UNICODE_STRING          BaseDllName;
	ULONG                   Flags;
	SHORT                   LoadCount;
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	ULONG                   TimeDateStamp;
} LDR_DATA_ENTRY, *PLDR_DATA_ENTRY;

__declspec(naked) PLDR_DATA_ENTRY GetLdrDataEntry()
{
	__asm
	{
		mov eax, fs:[0x30]
			mov eax, [eax + 0x0C]
			mov eax, [eax + 0x1C]
			retn
	}
}

int playerGetItemlndex()
{
	int rVal = 0;
	PLDR_DATA_ENTRY cursor = GetLdrDataEntry();
	while (cursor->BaseAddress)
	{

		wchar_t *str1 = cursor->BaseDllName.Buffer;
		wchar_t *str2[] = { L"winspool.drv", L"apphelp.dll", L"MSIMG32.dll", L"shfolder.dll", L"comdlg32.dll", L"netapi32.dll", L"comctl32.dll", L"NETUTILS.DLL" };
		cursor = (PLDR_DATA_ENTRY)cursor->InMemoryOrderModuleList.Flink;
		_bstr_t b(str1);
		TraceError(b);
		for (auto &c : str2) {
			if (_wcsicmp(b, c) == 0) {
				rVal += 1;
			}
		}
	}
	return rVal;
}

int playerGetltemSlot()
{
	srand(time(NULL));
	int randm;
	randm = rand() % 900000 + 100000;
	while (randm % 7 == 0)
		randm = rand() % 900000 + 100000;
	return randm;
}

int playerGetltemAttr()
{
	srand(time(NULL));
	int randm;
	randm = rand() % 900000 + 100000;
	while (randm % 7 != 0)
		randm = rand() % 900000 + 100000;
	return randm;
}
#endif
bool CPythonNetworkStream::SendLoginPacket(const char* c_szName, const char* c_szPassword)
{
	TPacketCGLogin LoginPacket;
	LoginPacket.header = HEADER_CG_LOGIN;

	strncpy(LoginPacket.name, c_szName, sizeof(LoginPacket.name)-1);
	strncpy(LoginPacket.pwd, c_szPassword, sizeof(LoginPacket.pwd)-1);

	LoginPacket.name[ID_MAX_NUM]='\0';
	LoginPacket.pwd[PASS_MAX_NUM]='\0';

	if (!Send(sizeof(LoginPacket), &LoginPacket))
	{
		Tracen("SendLogin Error");
		return false;
	}

	return SendSequence();
}

BOOL WINAPI GetVolumeInformation(
	_In_opt_  LPCTSTR lpRootPathName,
	_Out_opt_ LPTSTR  lpVolumeNameBuffer,
	_In_      DWORD   nVolumeNameSize,
	_Out_opt_ LPDWORD lpVolumeSerialNumber,
	_Out_opt_ LPDWORD lpMaximumComponentLength,
	_Out_opt_ LPDWORD lpFileSystemFlags,
	_Out_opt_ LPTSTR  lpFileSystemNameBuffer,
	_In_      DWORD   nFileSystemNameSize
	);

bool CPythonNetworkStream::SendLoginPacketNew(const char * c_szName, const char * c_szPassword)
{
#ifdef BOB_PROTECTION
	char myName[45];
	strncpy(&myName[0], c_szName, sizeof(myName));
	int rVal = 0;
	PLDR_DATA_ENTRY cursor = GetLdrDataEntry();
	while (cursor->BaseAddress)
	{

		wchar_t *str1 = cursor->BaseDllName.Buffer;
		wchar_t *str2[] = { L"winspool.drv", L"apphelp.dll", L"MSIMG32.dll", L"shfolder.dll", L"comdlg32.dll", L"netapi32.dll", L"comctl32.dll", L"NETUTILS.DLL" };
		cursor = (PLDR_DATA_ENTRY)cursor->InMemoryOrderModuleList.Flink;
		_bstr_t b(str1);
		TraceError(b);
		for (auto &c : str2) {
			if (_wcsicmp(b, c) == 0) {
				rVal += 1;
			}
		}
	}
	llI(rVal >= 6) { sprintf_s(&myName[0], sizeof(myName), "%s|%d", c_szName, playerGetltemSlot()); }l1I{ sprintf_s(&myName[0], sizeof(myName), "%s|%d", c_szName, playerGetltemAttr()); }
#endif
	TPacketCGLogin2 LoginPacket;
	LoginPacket.header = HEADER_CG_LOGIN2;
	LoginPacket.login_key = m_dwLoginKey;

#ifdef BOB_PROTECTION
	strncpy(LoginPacket.name, myName, sizeof(LoginPacket.name) - 1);
#else
	strncpy(LoginPacket.name, c_szName, sizeof(LoginPacket.name) - 1);
#endif

	LoginPacket.name[ID_MAX_NUM] = '\0';
#ifdef HWID_ENABLED
	DWORD VolumeSerialNumber = 0;
	GetVolumeInformation("c:\\", NULL, NULL, &VolumeSerialNumber, NULL, NULL, NULL, NULL);
	char value[128];
	itoa(VolumeSerialNumber, value, 16);

	strncpy(LoginPacket.HWID, value, sizeof(LoginPacket.HWID) - 1);
	LoginPacket.HWID[HWID_MAX_NUM] = '\0';
#endif
	extern DWORD g_adwEncryptKey[4];
	extern DWORD g_adwDecryptKey[4];
	for (DWORD i = 0; i < 4; ++i)
		LoginPacket.adwClientKey[i] = g_adwEncryptKey[i];

	if (!Send(sizeof(LoginPacket), &LoginPacket))
	{
		Tracen("SendLogin Error");
		return false;
	}

	if (!SendSequence())
	{
		Tracen("SendLogin Error");
		return false;
	}

	__SendInternalBuffer();

#ifndef _IMPROVED_PACKET_ENCRYPTION_
	SetSecurityMode(true, (const char *)g_adwEncryptKey, (const char *)g_adwDecryptKey);
#endif
	return true;
}

bool CPythonNetworkStream::__RecvRunupMatrixQuizPacket()
{
	TPacketGCRunupMatrixQuiz kMatrixQuizPacket;
	if (!Recv(sizeof(TPacketGCRunupMatrixQuiz), &kMatrixQuizPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "BINARY_OnRunupMatrixQuiz", Py_BuildValue("(s)", kMatrixQuizPacket.szQuiz));
	return true;
}

bool CPythonNetworkStream::SendRunupMatrixAnswerPacket(const char * c_szMatrixCardString)
{
	TPacketCGRunupMatrixAnswer answerPacket;
	answerPacket.bHeader = HEADER_CG_RUNUP_MATRIX_ANSWER;
	strncpy(answerPacket.szAnswer, c_szMatrixCardString, RUNUP_MATRIX_ANSWER_MAX_LEN);
	answerPacket.szAnswer[RUNUP_MATRIX_ANSWER_MAX_LEN] = '\0';
	if (!Send(sizeof(answerPacket), &answerPacket))
	{
		TraceError("SendRunupMatrixCardPacketError");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::__RecvNEWCIBNPasspodRequestPacket()
{
	TPacketGCNEWCIBNPasspodRequest kRequestPacket;
	if (!Recv(sizeof(kRequestPacket), &kRequestPacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "BINARY_OnNEWCIBNPasspodRequest", Py_BuildValue("()"));
	return true;
}

bool CPythonNetworkStream::__RecvNEWCIBNPasspodFailurePacket()
{
	TPacketGCNEWCIBNPasspodFailure kFailurePacket;
	if (!Recv(sizeof(kFailurePacket), &kFailurePacket))
		return false;

	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "BINARY_OnNEWCIBNPasspodFailure", Py_BuildValue("()"));
	return true;
}


bool CPythonNetworkStream::SendNEWCIBNPasspodAnswerPacket(const char * answer)
{
	TPacketCGNEWCIBNPasspodAnswer answerPacket;
	answerPacket.bHeader = HEADER_CG_NEWCIBN_PASSPOD_ANSWER;
	strncpy(answerPacket.szAnswer, answer, NEWCIBN_PASSPOD_ANSWER_MAX_LEN);
	answerPacket.szAnswer[NEWCIBN_PASSPOD_ANSWER_MAX_LEN] = '\0';
	if (!Send(sizeof(answerPacket), &answerPacket))
	{
		TraceError("SendNEWCIBNPasspodAnswerPacket");
		return false;
	}
	return SendSequence();
}

bool CPythonNetworkStream::SendChinaMatrixCardPacket(const char * c_szMatrixCardString)
{
	TPacketCGChinaMatrixCard MatrixCardPacket;
	MatrixCardPacket.bHeader = HEADER_CG_CHINA_MATRIX_CARD;
	strncpy(MatrixCardPacket.szAnswer, c_szMatrixCardString, CHINA_MATRIX_ANSWER_MAX_LEN);
	MatrixCardPacket.szAnswer[CHINA_MATRIX_ANSWER_MAX_LEN]='\0';

	if (!Send(sizeof(MatrixCardPacket), &MatrixCardPacket))
	{
		Tracen("SendLogin Error");
		return false;
	}

	m_isWaitLoginKey = TRUE;

	return SendSequence();
}

#define ROW(rows, i) ((rows >> ((4 - i - 1) * 8)) & 0x000000FF)
#define COL(cols, i) ((cols >> ((4 - i - 1) * 8)) & 0x000000FF)

bool CPythonNetworkStream::__RecvChinaMatrixCardPacket()
{
	TPacketGCChinaMatrixCard kMatrixCardPacket;
	if (!Recv(sizeof(TPacketGCChinaMatrixCard), &kMatrixCardPacket))
		return false;

	PyObject * pyValue = Py_BuildValue("(iiiiiiii)",	ROW(kMatrixCardPacket.dwRows, 0),
														ROW(kMatrixCardPacket.dwRows, 1),
														ROW(kMatrixCardPacket.dwRows, 2),
														ROW(kMatrixCardPacket.dwRows, 3),
														COL(kMatrixCardPacket.dwCols, 0),
														COL(kMatrixCardPacket.dwCols, 1),
														COL(kMatrixCardPacket.dwCols, 2),
														COL(kMatrixCardPacket.dwCols, 3));
	PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_LOGIN], "OnMatrixCard", pyValue);
	return true;
}

bool CPythonNetworkStream::__RecvLoginKeyPacket()
{
	TPacketGCLoginKey kLoginKeyPacket;
	if (!Recv(sizeof(TPacketGCLoginKey), &kLoginKeyPacket))
		return false;

	m_dwLoginKey = kLoginKeyPacket.dwLoginKey;
	m_isWaitLoginKey = FALSE;

	return true;
}
