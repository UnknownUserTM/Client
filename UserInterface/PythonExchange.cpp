#include "stdafx.h"
#include "PythonExchange.h"

void CPythonExchange::SetSelfName(const char *name)
{
	strncpy(m_self.name, name, CHARACTER_NAME_MAX_LEN);
}

void CPythonExchange::SetTargetName(const char *name)
{
	strncpy(m_victim.name, name, CHARACTER_NAME_MAX_LEN);
}

char * CPythonExchange::GetNameFromSelf()
{
	return m_self.name;
}

char * CPythonExchange::GetNameFromTarget()
{
	return m_victim.name;
}

void CPythonExchange::SetElkToTarget(int64_t elk)
{
	m_victim.elk = elk;
}

void CPythonExchange::SetElkToSelf(int64_t elk)
{
	m_self.elk = elk;
}

int64_t CPythonExchange::GetElkFromTarget()
{
	return m_victim.elk;
}

int64_t CPythonExchange::GetElkFromSelf()
{
	return m_self.elk;
}

#ifdef ENABLE_NEW_EXCHANGE_WINDOW
void CPythonExchange::SetSelfRace(DWORD race)
{
	m_self.race = race;
}

void CPythonExchange::SetTargetRace(DWORD race)
{
	m_victim.race = race;
}

DWORD CPythonExchange::GetRaceFromSelf()
{
	return m_self.race;
}

DWORD CPythonExchange::GetRaceFromTarget()
{
	return m_victim.race;
}

void CPythonExchange::SetSelfLevel(DWORD level)
{
	m_self.level = level;
}

void CPythonExchange::SetTargetLevel(DWORD level)
{
	m_victim.level = level;
}

DWORD CPythonExchange::GetLevelFromSelf()
{
	return m_self.level;
}

DWORD CPythonExchange::GetLevelFromTarget()
{
	return m_victim.level;
}
#endif

void CPythonExchange::SetItemToTarget(DWORD pos, DWORD vnum, DWORD count)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = vnum;
	m_victim.item_count[pos] = count;
}

void CPythonExchange::SetItemToSelf(DWORD pos, DWORD vnum, DWORD count)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = vnum;
	m_self.item_count[pos] = count;
}

void CPythonExchange::SetItemMetinSocketToTarget(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemMetinSocketToSelf(int pos, int imetinpos, DWORD vnum)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_metin[pos][imetinpos] = vnum;
}

void CPythonExchange::SetItemAttributeToTarget(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_attr[pos][iattrpos].bType = byType;
	m_victim.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::SetItemAttributeToSelf(int pos, int iattrpos, BYTE byType, short sValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_attr[pos][iattrpos].bType = byType;
	m_self.item_attr[pos][iattrpos].sValue = sValue;
}

void CPythonExchange::DelItemOfTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_victim.item_vnum[pos] = 0;
	m_victim.item_count[pos] = 0;
}

void CPythonExchange::DelItemOfSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	m_self.item_vnum[pos] = 0;
	m_self.item_count[pos] = 0;
}

DWORD CPythonExchange::GetItemVnumFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_vnum[pos];
}

DWORD CPythonExchange::GetItemVnumFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_vnum[pos];
}

DWORD CPythonExchange::GetItemCountFromTarget(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_count[pos];
}

DWORD CPythonExchange::GetItemCountFromSelf(BYTE pos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_count[pos];
}

DWORD CPythonExchange::GetItemMetinSocketFromTarget(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_victim.item_metin[pos][iMetinSocketPos];
}

DWORD CPythonExchange::GetItemMetinSocketFromSelf(BYTE pos, int iMetinSocketPos)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return 0;

	return m_self.item_metin[pos][iMetinSocketPos];
}

void CPythonExchange::GetItemAttributeFromTarget(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_victim.item_attr[pos][iAttrPos].bType;
	*psValue = m_victim.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::GetItemAttributeFromSelf(BYTE pos, int iAttrPos, BYTE * pbyType, short * psValue)
{
	if (pos >= EXCHANGE_ITEM_MAX_NUM)
		return;

	*pbyType = m_self.item_attr[pos][iAttrPos].bType;
	*psValue = m_self.item_attr[pos][iAttrPos].sValue;
}

void CPythonExchange::SetAcceptToTarget(BYTE Accept)
{
	m_victim.accept = Accept ? true : false;
}

void CPythonExchange::SetAcceptToSelf(BYTE Accept)
{
	m_self.accept = Accept ? true : false;
}

bool CPythonExchange::GetAcceptFromTarget()
{
	return m_victim.accept ? true : false;
}

bool CPythonExchange::GetAcceptFromSelf()
{
	return m_self.accept ? true : false;
}

bool CPythonExchange::GetElkMode()
{
	return m_elk_mode;
}

void CPythonExchange::SetElkMode(bool value)
{
	m_elk_mode = value;
}

void CPythonExchange::Start()
{
	m_isTrading = true;
}

void CPythonExchange::End()
{
	m_isTrading = false;
}

bool CPythonExchange::isTrading()
{
	return m_isTrading;
}

void CPythonExchange::Clear()
{
	memset(&m_self, 0, sizeof(m_self));
	memset(&m_victim, 0, sizeof(m_victim));
/*
	m_self.item_vnum[0] = 30;
	m_victim.item_vnum[0] = 30;
	m_victim.item_vnum[1] = 40;
	m_victim.item_vnum[2] = 50;
*/
}

CPythonExchange::CPythonExchange()
{
	Clear();
	m_isTrading = false;
	m_elk_mode = false;
		// Clear로 옴겨놓으면 안됨.
		// trade_start 페킷이 오면 Clear를 실행하는데
		// m_elk_mode는 클리어 되선 안됨.;
}
CPythonExchange::~CPythonExchange()
{
}