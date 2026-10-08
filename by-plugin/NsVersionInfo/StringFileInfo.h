// StringFileInfo.h: interface for the CStringFileInfo class.
//
//////////////////////////////////////////////////////////////////////

#ifndef STRINGFILEINFO_H
#define STRINGFILEINFO_H

#include <string>
#include <vector>
#include <map>

#include "stdafx.h"
#include "VersionInfoHelperStructures.h"
//#include "DeclSpec.h"


class CVersionInfoBuffer;
class CStringTable;

class VER_INFO_DECLSPEC CStringFileInfo
{
public:
	void Reset();
	CStringFileInfo();
	CStringFileInfo(StringFileInfo* pStringFI);
	virtual ~CStringFileInfo();

	void FromStringFileInfo(StringFileInfo* pStringFI);
	void Write(CVersionInfoBuffer & viBuf);

	BOOL IsEmpty();

	// Table count
	DWORD GetStringTableCount();

	// Iterative Access to StringTables
	std::vector<CStringTable*>::const_iterator GetFirstStringTablePosition() const;

	const CStringTable* GetNextStringTable(std::vector<CStringTable*>::const_iterator  &pos) const;
	CStringTable* GetNextStringTable(std::vector<CStringTable*>::const_iterator  &pos);

	// Convenient references to first usually the only string table
	const CStringTable& GetFirstStringTable() const;
	CStringTable& GetFirstStringTable();

	// Access string tables by keys (language ID + Code Page)
	const CStringTable& GetStringTable(const std::wstring& strKey) const;
	CStringTable& GetStringTable(const std::wstring& strKey);
	BOOL IsLastStringTablePosition(std::vector<CStringTable*>::const_iterator it) const;

	// Bracket operators allowing easy access to string tables
	const CStringTable& operator [] (const std::wstring &strKey) const;
	CStringTable &operator [] (const std::wstring &strKey);

	// Checks if string table for specified key already defined
	BOOL HasStringTable(const std::wstring &strKey) const;

	// Add new String table
	CStringTable& AddStringTable(const std::wstring &strKey);
	CStringTable& AddStringTable(CStringTable* pStringTable);

	// Change language of the string table (the proper way, do not use CStringTable::SetKey() directly)
	BOOL SetStringTableKey(const std::wstring &strOldKey, const std::wstring &strNewKey);

private:
	std::vector<CStringTable*> m_lstStringTables;
	//CMapStringToOb m_mapStringTables;
	std::map<std::wstring, CStringTable*> m_mapStringTables;
};

#endif //STRINGFILEINFO_H
