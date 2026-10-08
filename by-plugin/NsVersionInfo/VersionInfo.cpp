// VersionInfo.cpp: implementation of the CVersionInfo class.
//
//////////////////////////////////////////////////////////////////////

/*
 * This code is a portage from MFC to CPP/GCC of the project
 * "Modification of Version Information Resources in Compiled Binaries" By Denis Zabavchik
 *
 * see http://www.codeproject.com/KB/library/VerInfoLib.aspx?display=PrintAll
 */


#define UNICODE
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype> //std::toupper
#include <assert.h>
#include "stdafx.h"
#include <malloc.h>

#include "StringTable.h"
#include "StringFileInfo.h"
#include "VersionInfoHelperStructures.h"
#include "VersionInfoBuffer.h"
#include "VersionInfo.h"
#include "WinPETools.h"

template <class T>
bool from_string(T& t,const std::string& s,std::ios_base& (*f)(std::ios_base&))
{
	std::istringstream iss(s);
	return !(iss >> f >> t).fail();
}

template <class T>
bool from_string(T& t,const std::wstring& s,std::ios_base& (*f)(std::ios_base&))
{
	string temp(s.length(), ' ');
	copy(s.begin(), s.end(), temp.begin());
	std::istringstream iss(temp);
	return !(iss >> f >> t).fail();
}

#define OVR_BAK L"ovr.bak" //name of the temporary file to store the overlay
#ifdef _DEBUG
#define LIB_VERSION "1.0.4 (debug) - " __DATE__ " " __TIME__
#else
#define LIB_VERSION "1.0.4 - " __DATE__ " " __TIME__
#endif

//////////////////////////////////////////////////////////////////////
// CVersionInfo main class wrapping Version info for modules
/*
CVersionInfo::CVersionInfo():m_lpszResourceId(NULL), m_wLangId(0xFFFF), m_bRegularInfoOrder(TRUE)
{
	ZeroMemory(&m_vsFixedFileInfo, sizeof(VS_VERSION_INFO));
	//TODO debug
	//FromFile(L"VerInfoLibTest.exe");
}
*/

/**
 * Constructor of CVersionInfo
 */
CVersionInfo::CVersionInfo(const wchar_t *strModulePath, LPCTSTR lpszResourceId, WORD wLangId):
m_strModulePath(strModulePath), m_lpszResourceId(NULL), m_wLangId(wLangId), m_bRegularInfoOrder(TRUE)
{
	// LPCTSTR lpszResourceId may contain integer value pointer to string, in case it's a string make a local copy of it
	if (lpszResourceId && IS_INTRESOURCE(lpszResourceId)){
        m_strStringResourceId = wstring(lpszResourceId);
    }

	ZeroMemory(&m_vsFixedFileInfo, sizeof VS_VERSION_INFO);
	//cout << "DEBUG: strModulePath = " << strModulePath << endl; //FIXME: cout affiche la valeur du pointeur au lieu de la chaine
#ifdef _DEBUG		
	wprintf(L"DEBUG: CVersionInfo::CVersionInfo() strModulePath = %s\n", strModulePath);
#endif		
	if(strModulePath)
		FromFile(strModulePath, lpszResourceId, wLangId);
}

CVersionInfo::~CVersionInfo()
{
    #ifdef _DEBUG
    OutputDebugString(L"~CVersionInfo()");
    #endif
}

/**
 * Quick save (saves to the same module, resource, and language that it was loaded from)
 */
BOOL CVersionInfo::Save()
{
	return ToFile();
}

/**
 * Save version information to module resource
 * (specify strModulePath, lpszResourceId & wLangId to copy resource to different module, resource, language)
 */
BOOL CVersionInfo::ToFile(const wchar_t *strModulePath, LPCTSTR lpszResourceId, WORD wLangId)
{
	wstring strUseModulePath(strModulePath);

	if (strUseModulePath.empty()){
		strUseModulePath = m_strModulePath;
	}

	if (NULL == lpszResourceId){
		//Try resource ID that we loaded from;
		lpszResourceId = m_lpszResourceId;

		if (NULL == lpszResourceId){
			//Use default
			lpszResourceId = MAKEINTRESOURCE(1);
		}
	}

	if (0xFFFF == wLangId){
		//Try using language that we loaded from
		wLangId = m_wLangId;

		if (0xFFFF == wLangId){
			//Use neutral
			wLangId = 0;
		}
	}

	CVersionInfoBuffer viSaveBuf;
	Write(viSaveBuf);

	return UpdateModuleResource(strUseModulePath.c_str(), lpszResourceId, wLangId, viSaveBuf.GetData(), viSaveBuf.GetPosition());
}

/**
 * Updates module RT_VERSION resource with specified ID with data in lpData
 */
BOOL CVersionInfo::UpdateModuleResource(const wchar_t* strFilePath, LPCTSTR lpszResourceId, WORD wLangId, LPVOID lpData, DWORD dwDataLength)
{
	unsigned long overlayOffset;

	//Make a copy of the module oberlay, if any
	if (m_bHasOverlay)
		WinPETools::backupOverlay(strFilePath, OVR_BAK, m_OvrOffset,  m_OvrSize);

	HANDLE hUpdate = ::BeginUpdateResource(strFilePath, FALSE);
	if (hUpdate == NULL)
		return FALSE;

	BOOL bUpdateResult = FALSE;
#ifdef _DEBUG	
	cout << "Ecriture de la ressource avec lpszResourceId = " << dec << lpszResourceId << " et wLangId = " << wLangId << " " << dwDataLength << " octets" << endl;
#endif	
	bUpdateResult = UpdateResource(hUpdate, RT_VERSION, lpszResourceId, wLangId, lpData, dwDataLength);
	EndUpdateResource(hUpdate, FALSE);

	//restore the overlay
	if (m_bHasOverlay){
		if (WinPETools::restoreOverlay(strFilePath, OVR_BAK, &overlayOffset))
			//and clean the temp file
			DeleteFile(OVR_BAK);
	}

	return bUpdateResult;
}

/**
 * Read version information from module
 */
BOOL CVersionInfo::FromFile(const wchar_t *strModulePath, LPCTSTR lpszResourceId, WORD wLangId)
{
	CVersionInfoBuffer viLoadBuf;

	m_wLangId = wLangId;
	m_lpszResourceId = (LPTSTR)lpszResourceId;
#ifdef _DEBUG
	wprintf(L"DEBUG: strModulePath = %s\n", strModulePath);
#endif	
	//LoadVersionInfoResource will update member variables m_wLangId, m_lpszResourceId, which is awkward, need to change this flow
	if (!LoadVersionInfoResource(strModulePath, viLoadBuf, lpszResourceId, wLangId)){
#ifdef _DEBUG		
		wprintf(L"DEBUG: FromFile: LoadVersionInfoResource failed \n");
#endif		
		return FALSE;
	}

	//check for special attributes for modules with overlays
	//an overlay is an extra data part after the sections that are declared in the PE header
	//the problem is that overlays must be handled separately as the UpdateResource API calls truncate them
	m_bHasOverlay = WinPETools::hasOverlay(strModulePath, &m_OvrOffset, &m_OvrSize);

	m_strModulePath = strModulePath;

	DWORD dwSize = viLoadBuf.GetPosition();
	VERSION_INFO_HEADER* pVI = (VERSION_INFO_HEADER*) viLoadBuf.GetData();

	assert(!wcscmp(pVI->szKey, L"VS_VERSION_INFO"));

	VS_FIXEDFILEINFO* pFixedInfo = (VS_FIXEDFILEINFO*)DWORDALIGN(&pVI->szKey[wcslen(pVI->szKey)+1]);

	memcpy(&m_vsFixedFileInfo, pFixedInfo, sizeof(VS_FIXEDFILEINFO));

	// Iterate children StringFileInfo or VarFileInfo
	BaseFileInfo *pChild = (BaseFileInfo*) DWORDALIGN((DWORD)pFixedInfo + pVI->wValueLength);

	BOOL bHasVar = FALSE;
	BOOL bHasStrings = FALSE;
	BOOL bBlockOrderKnown = FALSE;
	vector<wstring> lstTranslations;

	while ((DWORD)pChild < ((DWORD)(pVI) + pVI->wLength)){
		if (!wcscmp(pChild->szKey, L"StringFileInfo")){
			//It is a StringFileInfo

			// removed that check because windres.exe produces StringFileInfo resources
			// with wType field == 0
			//assert(1 == pChild->wType);

			StringFileInfo* pStringFI = (StringFileInfo*)pChild;
			assert(!pStringFI->wValueLength);

			//MSDN says: Specifies an array of zero or one StringFileInfo structures.  So there should be only one StringFileInfo at most
			assert(m_stringFileInfo.IsEmpty());

			m_stringFileInfo.FromStringFileInfo(pStringFI);
			bHasStrings = TRUE;
		}
		else{
			VarFileInfo* pVarInfo = (VarFileInfo*)pChild;
			//~ Pour les resources compilé par windres.exe de mingw32 qui n'utilise pas la valeur standard Microsoft.
			//~ assert(1 == pVarInfo->wType);
			assert(!wcscmp(pVarInfo->szKey, L"VarFileInfo"));
			assert(!pVarInfo->wValueLength);
			//Iterate Var elements
			//There really must be only one
			Var* pVar = (Var*) DWORDALIGN(&pVarInfo->szKey[wcslen(pVarInfo->szKey)+1]);
			while ((DWORD)pVar < ((DWORD) pVarInfo + pVarInfo->wLength)){
				assert(!bHasVar && "Multiple Vars in VarFileInfo");
				assert(!wcscmp(pVar->szKey, L"Translation"));
				assert(pVar->wValueLength);

				DWORD *pValue = (DWORD*) DWORDALIGN(&pVar->szKey[wcslen(pVar->szKey)+1]);
				DWORD *pdwTranslation = pValue;
				while ((LPBYTE)pdwTranslation < (LPBYTE)pValue + pVar->wValueLength){
					wstring strStringTableKey;
					strStringTableKey = wformat(L"%04x%04x", LOWORD(*pdwTranslation), HIWORD(*pdwTranslation));

					lstTranslations.push_back(strStringTableKey);
					pdwTranslation++;
				}
				bHasVar = TRUE;
				pVar = (Var*) DWORDALIGN((DWORD)pVar + pVar->wLength);
			}

			assert(bHasVar && "No Var in VarFileInfo");
		}

		if (!bBlockOrderKnown){
			bBlockOrderKnown = TRUE;
			m_bRegularInfoOrder = bHasStrings;
		}
		pChild = (BaseFileInfo*) DWORDALIGN((DWORD)pChild + pChild->wLength);
	}


#ifdef _DEBUG
	assert((DWORD)lstTranslations.size() == m_stringFileInfo.GetStringTableCount());
	cout << "From file debug." << endl;

	wstring strKey = m_stringFileInfo.GetFirstStringTable().GetKey();
	cout << "GetFirstStringTable().GetKey() = " << strKey.c_str() << endl;
	for(vector<wstring>::iterator transiter = lstTranslations.begin(); transiter != lstTranslations.end(); transiter++){
		wstring strTranslation = *transiter;
		wstring strTranslationUpper (strTranslation);
		transform(strTranslationUpper.begin(),
					strTranslationUpper.end(),
					strTranslationUpper.begin(),
					(int(*)(int)) toupper);

		assert(m_stringFileInfo.HasStringTable(strTranslation) || m_stringFileInfo.HasStringTable(strTranslationUpper));
	}
	//Verify Write
	CVersionInfoBuffer viSaveBuf;
	Write(viSaveBuf);
	//assert(viSaveBuf.GetPosition() == viLoadBuf.GetPosition());
	//assert(!memcmp(viSaveBuf.GetData(), viLoadBuf.GetData(), viSaveBuf.GetPosition()));

	//CFile fOriginal(_T("f1.res"), CFile::modeCreate | CFile::modeWrite);
	ofstream fOriginal;
	fOriginal.open("f1.res", ios::out);

	//fOriginal.Write(viLoadBuf.GetData(), viLoadBuf.GetPosition());
	//fOriginal << viLoadBuf.GetData();
	fOriginal.write((char*)viLoadBuf.GetData(),viLoadBuf.GetPosition());
	fOriginal.close();

	//CFile fSaved(_T("f2.res"), CFile::modeCreate | CFile::modeWrite);
	ofstream fSaved;
	fSaved.open("f2.res", ios::out);
	//fSaved.Write(viSaveBuf.GetData(), viSaveBuf.GetPosition());
	//fSaved << viSaveBuf.GetData();
	fSaved.write((char*)viSaveBuf.GetData(),viSaveBuf.GetPosition());
	fSaved.close();

#endif
	return TRUE;
}

/**
 * Writes computed VarFileInfo structure to buffer based on the contents of String table
 */
void CVersionInfo::WriteVarInfo(CVersionInfoBuffer & viBuf)
{
	//Check string tables
	if (m_stringFileInfo.IsEmpty())
		return;

	//Prepare to write VarFileInfo
	DWORD posVarInfo = viBuf.PadToDWORD();

	//Skip size of VarFileInfo for now;
	viBuf.Pad(sizeof(WORD));

	//Write wValueLength
	viBuf.WriteWord(0);

	//Write type
	viBuf.WriteWord(1);
	viBuf.WriteString(L"VarFileInfo");

	//Save offset of Var structure (Translation)
	DWORD posTranslation = viBuf.PadToDWORD();
	viBuf.Pad(sizeof(WORD));

	//Write size of translation, that is number of string tables * size of DWORD
	DWORD dwTableCount = m_stringFileInfo.GetStringTableCount();
	viBuf.WriteWord(LOWORD(dwTableCount * sizeof(DWORD)));

	//Write type
	viBuf.WriteWord(0);

	//Write key (Translation)
	viBuf.WriteString(L"Translation");

	//Pad for value
	viBuf.PadToDWORD();

	//Collect all id's in one DWORD array
	DWORD *pTranslationBuf = (DWORD*)_alloca(dwTableCount * sizeof(DWORD));
	DWORD *pTranslation = pTranslationBuf;

	//TODO on doit parcourir le vecteur
	vector<CStringTable*>::const_iterator posTable = m_stringFileInfo.GetFirstStringTablePosition();
	while (!m_stringFileInfo.IsLastStringTablePosition(posTable)){
		CStringTable * pStringTable = m_stringFileInfo.GetNextStringTable(posTable);
		TCHAR* pchEnding = NULL;
		DWORD dwKey; //= _tcstol(pStringTable->GetKey(),&pchEnding, 16);
		from_string<DWORD>(dwKey, pStringTable->GetKey(), std::hex);
		*pTranslation = (LOWORD(dwKey) << 16) | (HIWORD(dwKey));
		pTranslation++;
		posTable++;
	}
	viBuf.Write(pTranslationBuf, dwTableCount * sizeof(DWORD));

	//Write structure sizes
	viBuf.WriteStructSize(posTranslation);
	viBuf.WriteStructSize(posVarInfo);
}

/**
 * Writes structures to version info buffer in order specified in m_bRegularInfoOrder (Get/SetInfoBlockOrder())
 */
void CVersionInfo::Write(CVersionInfoBuffer & viBuf)
{
	//Pad to DWORD and save position for wLength
	DWORD pos = viBuf.PadToDWORD();

	//Skip size for now;
	viBuf.Pad(sizeof(WORD));

	//Write wValueLength
	viBuf.WriteWord(sizeof(VS_FIXEDFILEINFO));

	//Write wType
	viBuf.WriteWord(0);

	//Write key
	viBuf.WriteString(L"VS_VERSION_INFO");

	//Pad Fixed info
	viBuf.PadToDWORD();

	//Write Fixed file info
	viBuf.Write(&m_vsFixedFileInfo, sizeof(VS_FIXEDFILEINFO));

	if (m_bRegularInfoOrder){
		//Write string file info, it will pad as needed
		m_stringFileInfo.Write(viBuf);

		WriteVarInfo(viBuf);
	}
	else{
		WriteVarInfo(viBuf);

		//Write string file info, it will pad as needed
		m_stringFileInfo.Write(viBuf);
	}


	//Set the size of the Version Info
	viBuf.WriteStructSize(pos);
}

/**
 * Resets (removes all string tables and cleans fixed version info
 */
void CVersionInfo::Reset()
{
	m_stringFileInfo.Reset();
	m_strModulePath.clear();
	m_lpszResourceId = NULL;
	m_wLangId = 0xFFFF;
	ZeroMemory(&m_vsFixedFileInfo, sizeof(VS_FIXEDFILEINFO));
}

BOOL CVersionInfo::IsValid() const
{
	return (m_vsFixedFileInfo.dwSignature == 0xFEEF04BD);
}

/**
 * Get the order of blocks (Regular (TRUE) = StringFileInfo first, VarFileInfo 2nd)
 */
BOOL CVersionInfo::GetInfoBlockOrder() const
{
	return m_bRegularInfoOrder;
}

/**
 * Set the order of blocks (Regular (TRUE) = StringFileInfo first, VarFileInfo 2nd)
 */
void CVersionInfo::SetInfoBlockOrder(BOOL bRegularStringsFirst)
{
	m_bRegularInfoOrder = bRegularStringsFirst;
}

/**
 * Helper functions for automatic loading of first RT_VERSION resource - returns ResourceNames
 */
BOOL CVersionInfo::EnumResourceNamesFuncFindFirst(
    HANDLE hModule,   // module handle
    LPCTSTR lpType,   // address of resource type
    LPTSTR lpName,    // address of resource name
    LONG_PTR lParam)      // extra parameter, could be
{
	CVersionInfo * pVI= (CVersionInfo *)lParam;

    pVI->m_lpszResourceId = lpName;

	if (!IS_INTRESOURCE(lpName)){
		pVI->m_strStringResourceId = lpName;

		//And repoint lpszResourceId to the string
		pVI->m_lpszResourceId = /*(LPTSTR)(LPCTSTR)*/(TCHAR*)pVI->m_strStringResourceId.c_str();
    }

	//Stop enumeration
    return FALSE;
}

/**
 * Helper functions for automatic loading of first RT_VERSION resource - returns ResourceLang
 */
BOOL CVersionInfo::EnumResourceLangFuncFindFirst(
  HANDLE hModule,    // module handle
  LPCTSTR lpszType,  // resource type
  LPCTSTR lpszName,  // resource name
  WORD wIDLanguage,  // language identifier
  LONG_PTR lParam)    // application-defined parameter
{
	CVersionInfo * pVI= (CVersionInfo *)lParam;

    pVI->m_wLangId = wIDLanguage;

	//Stop enumeration
	return FALSE;
}

BOOL CVersionInfo::LoadVersionInfoResource(const wstring& strModulePath, CVersionInfoBuffer &viBuf, LPCTSTR lpszResourceId, WORD wLangId)
{
	HRSRC hResInfo;

	HMODULE hModule = LoadLibraryEx(strModulePath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
	if (NULL == hModule){
#ifdef _DEBUG		
		wprintf(L"DEBUG: CVersionInfo::LoadVersionInfoResource() LoadLibraryEx failed\n");	
#endif		
		return FALSE;
	}

	if ((NULL == lpszResourceId) && (wLangId == 0xFFFF)){
#ifdef _DEBUG		
		wprintf(L"DEBUG: CVersionInfo::LoadVersionInfoResource() pas de resourceId ni langId\n");
#endif		
		//Load first RT_VERSION resource that will be found

		m_lpszResourceId = NULL;

		EnumResourceNames(hModule, RT_VERSION, (ENUMRESNAMEPROC)EnumResourceNamesFuncFindFirst, (LONG_PTR)this);

		if (NULL == m_lpszResourceId)
		{
#ifdef _DEBUG			
			wprintf(L"DEBUG: CVersionInfo::LoadVersionInfoResource() pas trouve de resourceid avec EnumResourceNames\n");
#endif			
			FreeLibrary(hModule);
			return FALSE;
		}

		// Now the m_lpszResourceId must be the name of the resource
		m_wLangId = 0xFFFF;
		EnumResourceLanguages(hModule, RT_VERSION, m_lpszResourceId, (ENUMRESLANGPROC)EnumResourceLangFuncFindFirst, (LONG_PTR)this);

		// Found resource, copy the ID's to local vars
		lpszResourceId = m_lpszResourceId;
		wLangId = m_wLangId;
	}
#ifdef _DEBUG		
	cout << "Lecture de la ressource avec lpszResourceId = " << lpszResourceId << " et wLangId = " << wLangId << endl;
#endif	
	hResInfo = FindResourceEx(hModule, RT_VERSION, lpszResourceId, wLangId); 
	// Write the resource language to the resource information file. 

	DWORD dwSize = SizeofResource(hModule, hResInfo);
	if (dwSize){
		HGLOBAL hgRes = LoadResource(hModule, hResInfo);
		if (hgRes){
			LPVOID lpMemory = LockResource(hgRes);
			if (lpMemory){
#ifdef _DEBUG				
				wprintf(L"DEBUG: CVersionInfo::LoadVersionInfoResource() trouve une resource avec FindResourceEx\n");
#endif				
				viBuf.Write(lpMemory,dwSize);

				UnlockResource(hgRes);
				FreeLibrary(hModule);
				return TRUE;
			}
		}
	}
#ifdef _DEBUG	
	wprintf(L"DEBUG: CVersionInfo::LoadVersionInfoResource() pas trouve de resource avec FindResourceEx\n");
#endif	
	FreeLibrary(hModule);
	return FALSE;
}

/**
 * Get reference to CStringFileInfo
 */
const CStringFileInfo& CVersionInfo::GetStringFileInfo() const
{
	return m_stringFileInfo;
}

/**
 * Get reference to CStringFileInfo
 */
CStringFileInfo& CVersionInfo::GetStringFileInfo()
{
	return m_stringFileInfo;
}

/**
 * Overloaded bracket operators allow quick access to first string table in StringFileInfo r/w
 */
const wstring CVersionInfo::operator[] (const wstring &strName) const
{
	return m_stringFileInfo.GetFirstStringTable().operator[] (strName);
}

/**
 * Overloaded bracket operators allow quick access to first string table in StringFileInfo r/w
 */
wstring & CVersionInfo::operator[] (const wstring &strName)
{
	return m_stringFileInfo.GetFirstStringTable().operator[] (strName);
}

/**
 * Get reference to VS_FIXEDFILEINFO
 */
const VS_FIXEDFILEINFO& CVersionInfo::GetFixedFileInfo() const
{
	return m_vsFixedFileInfo;
}

/**
 * Get reference to VS_FIXEDFILEINFO
 */
VS_FIXEDFILEINFO& CVersionInfo::GetFixedFileInfo()
{
	return m_vsFixedFileInfo;
}

/**
 * SetFileVersion - Updates file version in VS_FIXEDFILEINFO and in stringtables when bUpdateStringTables == TRUE
 */
void CVersionInfo::SetFileVersion(WORD dwFileVersionMSHi, WORD dwFileVersionMSLo, WORD dwFileVersionLSHi, WORD dwFileVersionLSLo, BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
	SetFileVersion((dwFileVersionMSHi << 16) | dwFileVersionMSLo, (dwFileVersionLSHi << 16) | dwFileVersionLSLo, bUpdateStringTables, lpszDelim);
}

/**
 * SetFileVersion - Updates file version in VS_FIXEDFILEINFO and in stringtables when bUpdateStringTables == TRUE
 */
void CVersionInfo::SetFileVersion(DWORD dwFileVersionMS, DWORD dwFileVersionLS, BOOL bUpdateStringTables /*=TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
	m_vsFixedFileInfo.dwFileVersionMS = dwFileVersionMS;
	m_vsFixedFileInfo.dwFileVersionLS = dwFileVersionLS;

	if (bUpdateStringTables){
		vector<CStringTable*>::const_iterator posTable = m_stringFileInfo.GetFirstStringTablePosition();
		wstring strVersion;
		strVersion = wformat(L"%d%s%d%s%d%s%d", HIWORD(dwFileVersionMS), lpszDelim, LOWORD(dwFileVersionMS), lpszDelim, HIWORD(dwFileVersionLS), lpszDelim, LOWORD(dwFileVersionLS));
		while (!m_stringFileInfo.IsLastStringTablePosition(posTable)){
			CStringTable * pStringTable = m_stringFileInfo.GetNextStringTable(posTable);
			(*pStringTable)[L"FileVersion"] = strVersion;
			posTable++;
		}
	}
}

/**
 * SetProductVersion - Updates product version in VS_FIXEDFILEINFO and ALL stringtables when bUpdateStringTables == TRUE
 */
void CVersionInfo::SetProductVersion(WORD dwProductVersionMSHi, WORD dwProductVersionMSLo, WORD dwProductVersionLSHi, WORD dwProductVersionLSLo, BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
	SetProductVersion((dwProductVersionMSHi << 16) | dwProductVersionMSLo, (dwProductVersionLSHi << 16) | dwProductVersionLSLo, bUpdateStringTables, lpszDelim);
}

/**
 * SetProductVersion - Updates product version in VS_FIXEDFILEINFO and ALL stringtables when bUpdateStringTables == TRUE
 */
void CVersionInfo::SetProductVersion(DWORD dwProductVersionMS, DWORD dwProductVersionLS, BOOL bUpdateStringTables /* =TRUE */, LPCTSTR lpszDelim /*= _T(", ") */)
{
	m_vsFixedFileInfo.dwProductVersionMS = dwProductVersionMS;
	m_vsFixedFileInfo.dwProductVersionLS = dwProductVersionLS;

	if (bUpdateStringTables){
		vector<CStringTable*>::const_iterator posTable = m_stringFileInfo.GetFirstStringTablePosition();
		wstring strVersion;
		strVersion = wformat(L"%d%s%d%s%d%s%d", HIWORD(dwProductVersionMS), lpszDelim, LOWORD(dwProductVersionMS), lpszDelim, HIWORD(dwProductVersionLS), lpszDelim, LOWORD(dwProductVersionLS));
		while (!m_stringFileInfo.IsLastStringTablePosition(posTable)){
			CStringTable * pStringTable = m_stringFileInfo.GetNextStringTable(posTable);
			(*pStringTable)[L"ProductVersion"] = strVersion;
			posTable++;
		}
	}
}

/**
 * Returns the version of the verInfoLib
 */
const char* CVersionInfo::GetLibVersion()
{
	return LIB_VERSION;
}
