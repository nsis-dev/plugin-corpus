// VersionInfo.h: interface for the CVersionInfo class.

#ifndef VERSIONINFO_H
#define VERSIONINFO_H

#include <string>
#include "stdafx.h"
#include "VersionInfoString.h"
#include "StringTable.h"
#include "StringFileInfo.h"

class VER_INFO_DECLSPEC CVersionInfo  
{
public:
	// Construction/Destruction
	//CVersionInfo();
	CVersionInfo(const wchar_t* strModulePath = NULL, LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF /*Auto*/);
	virtual ~CVersionInfo();

	//Read version information from module
	BOOL FromFile(const wchar_t* strModulePath, LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF /*Auto*/);
	
	//Save version information to module resource (specify strModulePath, lpszResourceId & wLangId to copy resource to different module, resource, language)
	BOOL ToFile(const wchar_t* strModulePath = L"", LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF /*Auto*/);
	
	//Quick save (saves to the same module, resource, and language that it was loaded from)
	BOOL Save();
	
	//Resets (removes all string tables and cleans fixed version info
	void Reset();

	BOOL IsValid() const;

	// Get/Set the order of blocks (Regular (TRUE) = StringFileInfo first, VarFileInfo 2nd)
	BOOL GetInfoBlockOrder() const;
	void SetInfoBlockOrder(BOOL bRegularStringsFirst);

	// Get reference to CStringFileInfo 
	const CStringFileInfo& GetStringFileInfo() const;
	CStringFileInfo& GetStringFileInfo();

	// Overloaded bracket operators allow quick access to first string table in StringFileInfo r/w
	const wstring operator[] (const wstring &strName) const;
	wstring &operator[] (const wstring &strName);
	
	// Get reference to VS_FIXEDFILEINFO
	const VS_FIXEDFILEINFO& GetFixedFileInfo() const;
	VS_FIXEDFILEINFO& GetFixedFileInfo();

	// SetFileVersion - Updates file version in VS_FIXEDFILEINFO and in stringtables when bUpdateStringTables == TRUE
	void SetFileVersion(WORD dwFileVersionMSHi, WORD dwFileVersionMSLo, WORD dwFileVersionLSHi, WORD dwFileVersionLSLo, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = L", ");
	void SetFileVersion(DWORD dwFileVersionMS, DWORD dwFileVersionLS, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = L", ");

	// SetProductVersion - Updates product version in VS_FIXEDFILEINFO and ALL stringtables when bUpdateStringTables == TRUE
	void SetProductVersion(WORD dwProductVersionMSHi, WORD dwProductVersionMSLo, WORD dwProductVersionLSHi, WORD dwProductVersionLSLo, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = L", ");
	void SetProductVersion(DWORD dwProductVersionMS, DWORD dwProductVersionLS, BOOL bUpdateStringTables = TRUE, LPCTSTR lpszDelim = L", ");

	static const char* GetLibVersion();

protected:
	// Loads all structures from specified module, resource, language to version buffer
	BOOL LoadVersionInfoResource(const wstring& strModulePath, CVersionInfoBuffer &viBuf, LPCTSTR lpszResourceId = NULL /*Auto*/, WORD wLangId = 0xFFFF);
	
	// Updates module RT_VERSION resource with specified ID with data in lpData
	BOOL UpdateModuleResource(const wchar_t *strFilePath, LPCTSTR lpszResourceId, WORD wLangId, LPVOID lpData, DWORD dwDataLength);

	// Writes structures to version info buffer in order specified in m_bRegularInfoOrder (Get/SetInfoBlockOrder())
	void Write(CVersionInfoBuffer & viBuf);
	
	// Writes computed VarFileInfo structure to buffer based on the contents of String table
	void WriteVarInfo(CVersionInfoBuffer & viBuf);
	
	// Helper functions for automatic loading of first RT_VERSION resource 
	static BOOL CALLBACK EnumResourceNamesFuncFindFirst(HANDLE hModule, LPCTSTR lpType, LPTSTR lpName, LONG_PTR lParam);
	static BOOL CALLBACK EnumResourceLangFuncFindFirst(HANDLE hModule, LPCTSTR lpszType, LPCTSTR lpszName, WORD wIDLanguage, LONG_PTR lParam);
	
	// Main version info data members
	CStringFileInfo m_stringFileInfo;
	VS_FIXEDFILEINFO m_vsFixedFileInfo;

	// Information about loaded version info (Module, resource id, lang id, and order in which VarFileInfo and StringFileInfo appeared in the module)
	wstring			m_strModulePath;			//path to the module
	wstring			m_strStringResourceId;		//resource id of the RT_VERSION
	LPTSTR			m_lpszResourceId;			//resource id if string
	WORD			m_wLangId;					//lang id of the resource
	BOOL			m_bRegularInfoOrder;		//choose if StringFileInfo are placed before VarFileInfo
	BOOL			m_bHasOverlay;				//tell if the module has extra data after the PE structure
	unsigned long m_OvrOffset;				//stores the offset of the extra data
	unsigned long m_OvrSize;					//stores the size of the extra data
};

#endif //VERSIONINFO_H
