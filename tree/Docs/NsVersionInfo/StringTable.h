// StringTable.h: interface for the CStringTable class.
//
//////////////////////////////////////////////////////////////////////

#ifndef STRINGTABLE_H
#define STRINGTABLE_H

#include <string>
#include <map>
#include <vector>
#include "VersionInfoString.h"

#include "VersionInfoHelperStructures.h"

using namespace std;

class VER_INFO_DECLSPEC CStringTable
{
public:
	//Construction
	CStringTable(const wstring& strKey);
	CStringTable(WORD wLang, WORD wCodePageC);
	CStringTable(StringTable* pStringTable);
	virtual ~CStringTable();

	// Returns table key (language ID/codepage)
	const wstring& GetKey() const;
	
	// Loads string table from resource structure in memory
	void FromStringTable(StringTable* pStringTable);
	
	// Saves string table to version info buffer
	void Write(CVersionInfoBuffer & viBuf);

	// Overloaded bracket operators used to access strings in the table
	const wstring operator[] (const wstring &strName) const;
	wstring &operator[] (const wstring &strName);

	// Iterative access to string objects in table
	POSITION GetFirstStringPosition() const;
	const CVersionInfoString* GetNextString(POSITION &pos) const;
	CVersionInfoString* GetNextString(POSITION &pos);

	// Retrieves the list of string names into a CStringList
	void GetStringNames(vector<wstring> &slNames, BOOL bMerge = FALSE) const;

	friend class CStringFileInfo;
private:
	//Set key renames/changes the language/codepage for the table, accessible only via CStringFileInfo::SetStringTableKey()
	void SetKey(const wstring& strKey);

	vector<CVersionInfoString*> m_lstStrings;
	map<wstring, CVersionInfoString*> m_mapStrings;
	wstring m_strKey;
};

#endif //STRINGTABLE_H
