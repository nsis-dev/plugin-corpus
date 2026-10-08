// StringFileInfo.cpp: implementation of the CStringFileInfo class.
//
//////////////////////////////////////////////////////////////////////
//#include <assert.h>
//#include <string>
//#include <vector>
//#include <map>

#include "stdafx.h"

#include "StringFileInfo.h"
#include "StringTable.h"
#include "VersionInfoBuffer.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CStringFileInfo::CStringFileInfo()
{
}

CStringFileInfo::CStringFileInfo(StringFileInfo* pStringFI)
{
	FromStringFileInfo(pStringFI);
}

CStringFileInfo::~CStringFileInfo()
{
    #ifdef _DEBUG
    OutputDebugString("~CStringFileInfo()");
    #endif
	Reset();
}

//////////////////////////////////////////////////////////////////////
// Loading/saving

void CStringFileInfo::FromStringFileInfo(StringFileInfo* pStringFI)
{
	assert(pStringFI);

	StringTable* pStringTable = (StringTable*) DWORDALIGN(&pStringFI->szKey[wcslen(pStringFI->szKey)+1]);
	while ((DWORD)pStringTable < ((DWORD) pStringFI + pStringFI->wLength))
	{
		CStringTable* pObStringTable = new CStringTable(pStringTable);
		AddStringTable(pObStringTable);

		pStringTable = (StringTable*) DWORDALIGN((DWORD)pStringTable + pStringTable->wLength);
	}
}

void CStringFileInfo::Write(CVersionInfoBuffer & viBuf)
{
	//Check string tables
	if(m_lstStringTables.empty())
		return;

	//Pad to DWORD and save position for wLength
	DWORD pos = viBuf.PadToDWORD();

	//Skip size for now;
	viBuf.Pad(sizeof(WORD));

	//Write wValueLength
	viBuf.WriteWord(0);

	//Write wType
	viBuf.WriteWord(1);

	//Write key
	viBuf.WriteString(L"StringFileInfo");

	for(vector<CStringTable*>::iterator iter = m_lstStringTables.begin(); iter != m_lstStringTables.end(); iter ++)
		(*iter)->Write(viBuf);

	//Set the size of the structure based on current offset from the position
	viBuf.WriteStructSize(pos);
}

//////////////////////////////////////////////////////////////////////
// Operations

BOOL CStringFileInfo::IsEmpty()
{
	return m_lstStringTables.empty();
}

/**
 * Table count
 */
DWORD CStringFileInfo::GetStringTableCount()
{
	return m_lstStringTables.size();
}

/**
 * Convenient references to first usually the only string table
 */
CStringTable& CStringFileInfo::GetFirstStringTable()
{
	return (CStringTable&) *m_lstStringTables.at(0);
}

/**
 * Convenient references to first usually the only string table
 */
const CStringTable& CStringFileInfo::GetFirstStringTable() const
{
	return (CStringTable&) *m_lstStringTables.at(0);
}

/**
 * Access string tables by keys (language ID + Code Page)
 */
CStringTable& CStringFileInfo::GetStringTable(const wstring& strKey)
{
	CStringTable *pStringTable = NULL;
	map<wstring, CStringTable*>::iterator found;
	found = m_mapStringTables.find(strKey);
	if(found != m_mapStringTables.end())
	{
		pStringTable = found->second;
		AddStringTable(pStringTable);
	}

	return *pStringTable;
}

/**
 * Access string tables by keys (language ID + Code Page)
 */
const CStringTable& CStringFileInfo::GetStringTable(const wstring& strKey) const
{
	CStringTable *pStringTable = NULL;
	map<wstring, CStringTable*>::const_iterator miter = m_mapStringTables.find(strKey);
	if(miter != m_mapStringTables.end())
		pStringTable = miter->second;
	// This may return *NULL, be carefull
	return *pStringTable;
}

/**
 * Bracket operators allowing easy access to string tables
 */
CStringTable& CStringFileInfo::operator[] (const wstring &strKey)
{
	return GetStringTable(strKey);
}

/**
 * Bracket operators allowing easy access to string tables
 */
const CStringTable& CStringFileInfo::operator[] (const wstring &strKey) const
{
	return GetStringTable(strKey);
}

/**
 * Checks if string table for specified key already defined
 */
BOOL CStringFileInfo::HasStringTable(const wstring &strKey) const
{
	return (m_mapStringTables.find(strKey))->second != NULL;
}

CStringTable& CStringFileInfo::AddStringTable(const wstring &strKey)
{
	return GetStringTable(strKey);
}

CStringTable& CStringFileInfo::AddStringTable(CStringTable* pStringTable)
{
	m_lstStringTables.push_back(pStringTable);
	m_mapStringTables.insert(pair<wstring,CStringTable*>(pStringTable->GetKey(), pStringTable));
	return *pStringTable;
}

vector<CStringTable*>::const_iterator  CStringFileInfo::GetFirstStringTablePosition() const
{
	return m_lstStringTables.begin();
}

BOOL  CStringFileInfo::IsLastStringTablePosition(vector<CStringTable*>::const_iterator it) const
{
	return (it == m_lstStringTables.end());
}

CStringTable* CStringFileInfo::GetNextStringTable(vector<CStringTable*>::const_iterator  &pos)
{
	return (CStringTable*) *pos;
}

const CStringTable* CStringFileInfo::GetNextStringTable(vector<CStringTable*>::const_iterator  &pos) const
{
	return (CStringTable*) *pos;
}

BOOL CStringFileInfo::SetStringTableKey(const wstring &strOldKey, const wstring &strNewKey)
{
	CStringTable *pStringTable = NULL;
	map<wstring, CStringTable*>::iterator found;
	found = m_mapStringTables.find(strOldKey);
	if(found != m_mapStringTables.end())
	{
		pStringTable = found->second;
		pStringTable->SetKey(strNewKey);
		m_mapStringTables.erase(strOldKey);
		m_mapStringTables.insert(pair<wstring,CStringTable*>(strNewKey, pStringTable));

		return TRUE;
	}
	return FALSE;
}

void CStringFileInfo::Reset()
{
	m_lstStringTables.clear();
	m_mapStringTables.clear();
}
