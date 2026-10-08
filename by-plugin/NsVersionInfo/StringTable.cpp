// StringTable.cpp: implementation of the CStringTable class.
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include <map>
#include "stdafx.h"
#include "StringTable.h"
#include "VersionInfoBuffer.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CStringTable::CStringTable(const wstring& strKey):m_strKey(strKey)
{

}

CStringTable::CStringTable(WORD wLang, WORD wCodePage)
{
	wstring strKey;
	strKey = wformat(L"%04x%04x", wLang, wCodePage);

	SetKey(strKey);
}

CStringTable::CStringTable(StringTable* pStringTable)
{
	FromStringTable(pStringTable);
}

CStringTable::~CStringTable()
{
    #ifdef _DEBUG
    OutputDebugString("~CStringTable()");
    #endif
	while (!m_lstStrings.empty())
		m_lstStrings.pop_back();

	m_mapStrings.clear();
}

//////////////////////////////////////////////////////////////////////
// Loading/saving

/**
 * Loads string table from resource structure in memory
 */
void CStringTable::FromStringTable(StringTable* pStringTable)
{
	m_strKey = pStringTable->szKey;

	String* pString = (String*) DWORDALIGN(&pStringTable->szKey[wcslen(pStringTable->szKey)+1]);
	while ((DWORD)pString < ((DWORD) pStringTable + pStringTable->wLength) )
	{
		CVersionInfoString* pVIString = new CVersionInfoString(pString);
		m_lstStrings.push_back(pVIString);
		m_mapStrings.insert(pair<wstring,CVersionInfoString*>(pVIString->GetKey(), pVIString));

		pString = (String*) DWORDALIGN((DWORD) pString + pString->wLength);
	}
}

/**
 * Saves string table to version info buffer
 */
void CStringTable::Write(CVersionInfoBuffer & viBuf)
{
	//Pad to DWORD and save position for wLength
	DWORD pos = viBuf.PadToDWORD();

	//Skip size for now;
	viBuf.Pad(sizeof(WORD));

	//Write wValueLength
	viBuf.WriteWord(0);

	//Write wType
	viBuf.WriteWord(1);

	//Write key
	viBuf.WriteString(m_strKey);

	for(vector<CVersionInfoString*>::iterator iter = m_lstStrings.begin(); iter != m_lstStrings.end(); iter ++)
			(*iter)->Write(viBuf);
	//Set the size of the structure based on current offset from the position
	viBuf.WriteStructSize(pos);
}

//////////////////////////////////////////////////////////////////////////
// Operations

/**
 * Returns table key (language ID/codepage)
 */
const wstring& CStringTable::GetKey() const
{
	return m_strKey;
}

void CStringTable::SetKey(const wstring& strKey)
{
	m_strKey = strKey;
}

/**
 * Overloaded bracket operators used to access strings in the table
 */
wstring & CStringTable::operator[] (const wstring &strName)
{
	CVersionInfoString* pVIString = NULL;
	map<wstring, CVersionInfoString*>::iterator it;
	it = m_mapStrings.find(strName);
	if (it == m_mapStrings.end()){
		//not in table
		pVIString = new CVersionInfoString(strName);
		m_lstStrings.push_back(pVIString);
		m_mapStrings.insert(pair<wstring,CVersionInfoString*>(strName, pVIString));
	}
	else{
		pVIString = it->second;
	}

	return pVIString->GetValue();
}

const wstring CStringTable::operator[] (const wstring &strName) const
{
	CVersionInfoString* pVIString = NULL;
	map<wstring, CVersionInfoString*>::const_iterator it;
	it = m_mapStrings.find(strName);
	if(it == m_mapStrings.end())
		return L"";
	else
		pVIString = it->second;

	return pVIString->GetValue();
}

POSITION CStringTable::GetFirstStringPosition() const
{
	return 0;//m_lstStrings.GetHeadPosition();
}

const CVersionInfoString* CStringTable::GetNextString(POSITION &pos) const
{
	return m_lstStrings.at(pos);//(CVersionInfoString*)m_lstStrings.GetNext(pos);
}

CVersionInfoString* CStringTable::GetNextString(POSITION &pos)
{
	return m_lstStrings.at(pos);//(CVersionInfoString*)m_lstStrings.GetNext(pos);
}

/**
 * Retrieves the list of string names into a CStringList
 */
void CStringTable::GetStringNames(vector<wstring> &slNames, BOOL bMerge) const
{
	if (!bMerge)
		slNames.clear();

	vector<CVersionInfoString*>::const_iterator iter; // /!\ on utilise un const_iterator et pas un iterator ?
	for(iter = m_lstStrings.begin(); iter != m_lstStrings.end(); iter ++)
		slNames.push_back((*iter)->GetKey());
}

string format_arg_list(const char *fmt, va_list args)
{
    if (!fmt) return "";
    int   result = -1, length = 256;
    char *buffer = 0;
    while (result == -1)
    {
        if (buffer) delete [] buffer;
        buffer = new char [length + 1];
        memset(buffer, 0, length + 1);
        result = _vsnprintf(buffer, length, fmt, args);
        length *= 2;
    }
    std::string s(buffer);
    delete [] buffer;
    return s;
}

string format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::string s = format_arg_list(fmt, args);
    va_end(args);
    return s;
}


wstring wformat_arg_list(const wchar_t *fmt, va_list args)
{
    if (!fmt) return wstring(L"");
    int   result = -1, length = 256;
    wchar_t *buffer = 0;
    while (result == -1)
    {
        if (buffer) delete [] buffer;
        buffer = new wchar_t [length + 1];
        memset(buffer, 0, sizeof(wchar_t) * (length + 1));
        result = _vsnwprintf(buffer, length, fmt, args);
        length *= 2;
    }
    std::wstring s(buffer);
    delete [] buffer;
    return s;
}

wstring wformat(const wchar_t *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::wstring s = wformat_arg_list(fmt, args);
    va_end(args);
    return s;
}



