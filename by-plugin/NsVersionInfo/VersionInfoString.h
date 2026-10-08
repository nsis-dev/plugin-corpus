// VersionInfoString.h: interface for the CVersionInfoString class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VERSIONINFOSTRING_H
#define VERSIONINFOSTRING_H

#include <string>
#include "stdafx.h"
#include "VersionInfoHelperStructures.h"

using namespace std;

class CVersionInfoBuffer;

class VER_INFO_DECLSPEC CVersionInfoString
{
public:
	CVersionInfoString(String* pString);
	CVersionInfoString(const wstring& strKey, const wstring& strValue = L"");

	const wstring& GetKey() const;
	const wstring& GetValue() const;

	wstring& GetValue();

	void FromString(String* pString);
	void Write(CVersionInfoBuffer & viBuf);
private:
	wstring m_strKey;
	wstring m_strValue;
};

#endif //VERSIONINFOSTRING_H
