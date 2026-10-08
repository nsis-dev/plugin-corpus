#include "exdll.h"
#include "rpcsal.h"

#include <windows.h>
#include <gameux.h>
#include <shobjidl.h>
#include <KnownFolders.h>

// Replacement for std::exception
class exception
{
public:
	exception(const char* sException) { assign(sException); }
	exception(const exception& ex) { assign(ex.sException); }
	exception& operator=(const exception& ex) { delete sException; sException = NULL; assign(ex.sException); }
	~exception() { delete sException; }

	const char* what() const { return sException; }

private:
	void assign(const char* sException)
	{
		this->sException = new char[strlen(sException)+1];
		strcpy(this->sException, sException);
	}

	char* sException;
};

// Replacement for CComPtr
template <class T>
class CComPtr
{
public:
	CComPtr() : ppv(NULL) {}
	~CComPtr() { if(ppv) ppv->Release(); }

	HRESULT CoCreateInstance(const IID &rclsid)	{ return ::CoCreateInstance(rclsid, 0, CLSCTX_ALL, __uuidof(T), (void**)&ppv); }
	T* operator->() const { return ppv; }
	T** operator&() { return &ppv; }

private:
	T* ppv;
};

class GDFValues
{
public:
	TiXmlString sGamePath, sGameName, sGameDesc, sDeveloperName, sGameGUID;

	void load(const TiXmlString& sGamePath);

private:
	// Simple wrapper to release the library we're loaded
	class LibraryPtr
	{
	public:
		LibraryPtr(const char* sPath)
		{
			hMod = LoadLibrary(sPath);
			if(!hMod)
				throw exception((TiXmlString("Could not load '") + sPath + "'").c_str());
		}
		~LibraryPtr() { if(hMod) FreeLibrary(hMod); }

		const HMODULE get() const { return hMod; }

	private:
		HMODULE hMod;
	};


	// Load the GDF inside the executable located at sPath and load it into &xml.
	void loadXML(TiXmlDocument& xml);
};

// Log a message to the installer window
// Adapted from the LogEx plugin source code.
void log(HWND hwnd, const char *buf, bool fallbackToDialog = false);

// Delete a file of folder recursively
bool deleteFile(const char* sFile);

// Find the game tasks folder
TiXmlString getGameTasksFolder();

// Register the game with Games Explorer, potentially also telling it to update its GDF and
// clearing the task list, if the game is already registered.
// Returns false if it couldn't register, which probably means the target machine isn't Vista.
bool registerWithGamesExplorer(const GDFValues& values, const TiXmlString& sInstDir);

// Unregister the game with Games Explorer, also deleting the task list
bool unregisterWithGamesExplorer(const GDFValues& values);

// Register with Windows Media Center.
// If you've called this properly and the user has elevated rights (this will throw otherwise), it shouldn't fail,
// even if Media Center isn't on the target machine.
// In that case, the game will automatically be added to Media Center if the user ever upgrades.
void registerWithMediaCenter(const GDFValues& values, const TiXmlString& sImgPath, const TiXmlString& sCapabilities, const TiXmlString& sRunPath);

// Unregister with media center
void unregisterWithMediaCenter(const GDFValues& values);

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

// Register the games with Games Explorer and Media Centre
// The syntax for calling the function from NSIS is:
// registerGame [/mcrun:<cmdpath>] [/mcicon:<imgpath>] [/mccapabilities:<caps(comma-separated)>] <gamepath>
// This function returns, on the stack:
// "0" if there was an error, or
// "" if it was successful, but the target machine isn't Vista (and this doesn't have Games Explorer), or
// Your game's tasks folder path, not including the PlayTasks or SupportTasks folder.
extern "C" void __declspec(dllexport) registerGame(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
try
{
	EXDLL_INIT();

	// A map of the available options passed to this function.
	TiXmlString sMCRun, sMCIcon, sMCCapabilities;

	TiXmlString sGamePath;

	// Parse all the options.  Each option takes the form of "/optname:optvalue".
	// The last parameter is the game path.
	for(;;)
	{
		TiXmlString str;
		if(popstring(str))
			throw exception("No game path was passed to registerGame");
		
		if(str.size() && str[0] == '/')
		{
			size_t iColonPos = str.find(':');
			if(iColonPos != TiXmlString::npos)
			{
				TiXmlString cmd(str.c_str()+1, iColonPos-1);
				if(cmd == "mcrun")
				{
					if(iColonPos != str.size()-1) sMCRun = TiXmlString(str.c_str()+iColonPos+1);
					continue;
				}
				else if(cmd == "mcicon")
				{
					if(iColonPos != str.size()-1) sMCIcon = TiXmlString(str.c_str()+iColonPos+1);
					continue;
				}
				else if(cmd == "mccapabilities")
				{
					if(iColonPos != str.size()-1) sMCCapabilities = TiXmlString(str.c_str()+iColonPos+1);
					continue;
				}
			}
		}

		// Okay, it wasn't an option.  It must have been the game path.  We're done now.
		sGamePath = str;
		break;
	}

	GDFValues values;
	values.load(sGamePath);

	// Register with Media Center first, because that always succeeds (it leaves Media Center 'upgrade proof' if it's not installed)
	registerWithMediaCenter(values, sMCIcon, sMCCapabilities, sMCRun);
	if(!registerWithGamesExplorer(values, TiXmlString(getuservariable(INST_INSTDIR))))
	{
		log(hwndParent, "Couldn't register with Games Explorer.  Target machine probably isn't Vista.  That's okay!");
		// No Vista, well, let's leave "" on the stack.
		pushstring("");
	}
	else
	{
		// Everything's going well, let's leave the path to play tasks
		TiXmlString sReturn = getGameTasksFolder();
		sReturn += values.sGameGUID;
		pushstring(sReturn.c_str());
	}
}
catch(exception& ex)
{
	log(hwndParent, ex.what(), true);
	// Error, push "0"
	pushstring("0");
}

// Unregisters the game from Games Explorer and Media Center
// Syntax is unregisterGame <gamepath>
extern "C" void __declspec(dllexport) unregisterGame(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
try
{
	EXDLL_INIT();
	TiXmlString sGamePath;
	if(popstring(sGamePath))
		throw exception("No game path passed to unregisterGame");

	GDFValues values;
	values.load(sGamePath);

	unregisterWithMediaCenter(values);
	if(!unregisterWithGamesExplorer(values))
		log(hwndParent, "Couldn't unregister with Games Explorer.  Target machine probably isn't Vista.  That's okay!");
}
catch(exception& ex)
{
	log(hwndParent, ex.what(), true);
}

// Log a message to the installer window
// Adapted from the LogEx plugin source code.
void log(HWND hwnd, const char *buf, bool fallbackToDialog)
{
	if(hwnd)
	{
		HWND handle = FindWindowEx(hwnd, NULL, "#32770", "");	// Find the Dialog Window
		if(handle)
		{
			handle = GetDlgItem(handle, 1016);							// Get a handle to the listbox control

			int count = (int)SendMessage(handle, LVM_GETITEMCOUNT, 0, 0);	// Get the number of lines already in the listbox

			LVITEM lvi = {0};
			lvi.mask = LVIF_TEXT;
			lvi.iItem = count;
			lvi.pszText = LPSTR(buf);

			SendMessage (handle, LVM_INSERTITEM, 0, (LPARAM) &lvi);	// Add LogString to the listbox
			SendMessage (handle, LVM_ENSUREVISIBLE, (WPARAM) count, (LPARAM) FALSE); // Scroll down

			return;
		}
	}

	// The little return above ensures that we don't get here if we managed to write to the list box.
	// Call it speghetti code, but it saved me a variable and a test.
	if(fallbackToDialog)
		MessageBox(hwnd, buf, "Log message", MB_OK|MB_ICONEXCLAMATION);
}

// Delete a file of folder recursively
bool deleteFile(const char* sFile)
{
	// The function I'm about to use requires a *doubly null terminated* string, for some weird reason.
	int iStrLen = int(strlen(sFile)+2);
	char* sDZFile = new char[iStrLen];
	sDZFile[iStrLen-1] = '\0';
  
	SHFILEOPSTRUCT fileop;
	fileop.hwnd   = NULL;											// no status display
	fileop.wFunc  = FO_DELETE;										// delete operation
	fileop.pFrom  = sDZFile;										// source file name as double null terminated string
	fileop.pTo    = NULL;											// no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_ALLOWUNDO;	// do not prompt the user, and don't save the files in the recycle bin

	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle     = NULL;
	fileop.hNameMappings         = NULL;

	bool rc = SHFileOperation(&fileop) == 0;
	delete sDZFile;
	return rc;
}

// Load the GDF inside the executable located at *sPath and load it into &xml.
void GDFValues::loadXML(TiXmlDocument& xml)
{
	LibraryPtr mod(sGamePath.c_str());
	HRSRC hResInfo = FindResource(mod.get(), "__GDF_XML", "DATA");
	if(!hResInfo)
		throw exception((TiXmlString("Failed to find GDF data embedded in '") + sGamePath + "'").c_str());

	HGLOBAL hResData = LoadResource(mod.get(), hResInfo);
	if(!hResData)
		throw exception((TiXmlString("Failed to load GDF data embedded in '") + sGamePath + "'").c_str());

	LPVOID pResLock = LockResource(hResData);
	if(!pResLock)
		throw exception((TiXmlString("Failed to attain a lock for GDF data embedded in '") + sGamePath + "'").c_str());

	// At this point, pResLock has the GDF XML data, but unfortunately it has no null byte,
	// so we have to copy the whole thing just for that (unless we modify tinyxml to take a
	// maximum size, which isn't trivial).
	// We'll just copy it into an STL string.
	DWORD iSize = SizeofResource(mod.get(), hResInfo);
	if(!iSize)
		throw exception((TiXmlString("Failed to query size of GDF data embedded in '") + sGamePath + "'").c_str());

	TiXmlString sGDFXML((char*)pResLock, iSize);

	xml.Parse(sGDFXML.c_str());	// At last, we have the XML doc

	if(xml.Error())
		throw exception((TiXmlString("The following error occurred while parsing the GDF data embedded in '") + sGamePath + "'\n" + xml.ErrorDesc()).c_str());
}

// Loads the common global strings.  It caches these so that if the user uses /NOUNLOAD, it
void GDFValues::load(const TiXmlString& sGamePath)
{
	this->sGamePath = sGamePath;
	const char* sTmp;

	// Load the GUID, Name, and Description.
	TiXmlDocument doc;
	loadXML(doc);

	TiXmlHandle docHandle(&doc);
	TiXmlElement* pDef = docHandle.FirstChildElement("GameDefinitionFile").FirstChildElement("GameDefinition").ToElement();
	if(!pDef)
		throw exception("GDF contains no GameDefinitionFile/GameDefinition element");
	
	if(!(sTmp = pDef->Attribute("gameID")))
		throw exception("GDF contains no gameID attribute in GameDefinition element");
	sGameGUID = sTmp;
	
	TiXmlElement* pTmpXML = pDef->FirstChildElement("Name");
	if(!pTmpXML)
		throw exception("GDF contains no GameDefinitionFile/GameDefinition/Name element");
	
	sGameName = pTmpXML->GetText();

	pTmpXML = pDef->FirstChildElement("Description");
	if(pTmpXML)
		sGameDesc = pTmpXML->GetText();
	else
		sGameDesc = "";	// It's okay to have no desc.

	docHandle = TiXmlHandle(pDef);
	pTmpXML = docHandle.FirstChild("Developers").FirstChild("Developer").ToElement();
	if(pTmpXML)
		sDeveloperName = pTmpXML->GetText();
	else
		sGameDesc = "";	// It's also okay to have no developers.
}

// Find the game tasks folder
TiXmlString getGameTasksFolder()
{
	// Despite the fact that Microsoft is quite adamant that you should use SHGetFolderPathEx
	// with FOLDERID_GameTasks to get this folder, neither of those are documented anywhere!
	// Based on a little research and a lot of guesswork, this is what they actually meant.
	// By the way, Microsoft has patented all this stuff.  (Some of the best documentation I
	// found was from their patent application.)  It turns out, being able to enumerate
	// known folder locations is a brilliant invention!  Thank you Microsoft, I don't know
	// how we would have done it without you.
	CComPtr<IKnownFolderManager> folderMan;
	if(S_OK != folderMan.CoCreateInstance(__uuidof(KnownFolderManager)))
		throw exception("Couldn't create instance of IKnownFolderManager");

	CComPtr<IKnownFolder> folder;
	if(S_OK != folderMan->GetFolder(FOLDERID_PublicGameTasks, &folder))
		throw exception("Couldn't find game tasks folder");

	PWSTR pszPath;
	if(S_OK != folder->GetPath(0, &pszPath))
		throw exception("Couldn't get the string value for the game tasks folder");
	
	TiXmlString rc;
	try
	{
		int iSize = WideCharToMultiByte(CP_ACP, 0, pszPath, -1, NULL, 0, 0, FALSE);
		char* szPath = new char[iSize];
		WideCharToMultiByte(CP_ACP, 0, pszPath, -1, szPath, iSize, 0, FALSE);
		rc = szPath;
		delete szPath;
	}
	catch(...)
	{
		CoTaskMemFree(pszPath);
		throw;
	}
	CoTaskMemFree(pszPath);

	// Add a trailing backslash if necessary
	if(rc.size() && rc[rc.size()-1] != '/' && rc[rc.size()-1] != '\\')
		rc += '\\';

	return rc;
}

// Small wrapper class for the registry
class RegMan
{
	// We want to use RegDeleteKeyEx if it's available, but it still has to work otherwise,
	// so we need to dynamically load it.
	typedef LONG (WINAPI* FuncRegDeleteKeyEx)(HKEY, LPCTSTR, REGSAM, DWORD);

public:
	RegMan() : key(NULL), pDeleteFunc(NULL)
	{
		hMod = LoadLibrary("Advapi32.dll");
		if(hMod)
		{
			pDeleteFunc = (FuncRegDeleteKeyEx)GetProcAddress(hMod, "RegDeleteKeyExA");
			if(!pDeleteFunc)
			{
				FreeLibrary(hMod);
				hMod = NULL;
			}
		}
	}
	~RegMan()
	{
		if(key)
			RegCloseKey(key);

		if(hMod)
			FreeLibrary(hMod);
	}

	void openKey(const TiXmlString& sKey)
	{
		this->sKey = sKey;
		if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, sKey.c_str(), 0, 0, 0, KEY_ALL_ACCESS|KEY_WOW64_64KEY, 0, &key, NULL))
			throw exception((TiXmlString("Couldn't create registry key '") + sKey + "'").c_str());
	}

	void setValue(const char* sName, const TiXmlString& sData, DWORD dwType = REG_EXPAND_SZ)
	{
		if(!key)
			throw exception("Attempted to set a value before opening a key!");

		// It's not *all* rainbows and lollypops.  Because our installer may be a 32 bit app, and Win64
		// maintains a separate 32-bit registry, we need to make sure that we bypass that and put everything
		// in the correct registry.  The KEY_WOW64_64KEY flag does that.
		if(ERROR_SUCCESS != RegSetValueEx(key, sName, 0, dwType, (const BYTE*)sData.c_str(), (DWORD)sData.size()+1))
			throw exception((TiXmlString("Couldn't create registry value '") + sKey + "\\" + sName + "'").c_str());
	}

	void deleteKey(const char* sKey)
	{
		HKEY hKey;
		if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, sKey, 0, KEY_READ|KEY_WOW64_64KEY, &hKey))
			return;
		
		DWORD numKeys, iLength;
		RegQueryInfoKey(hKey, NULL, NULL, NULL, &numKeys, &iLength, NULL, NULL, NULL, NULL, NULL, NULL);
		if(numKeys)
		{
			// Drill down and delete the subkeys.
			size_t iKeyLen = strlen(sKey);
			char* sPath = new char[iKeyLen + iLength + 2]; // +1 for the null byte, +1 for the potential added backslash
			try
			{
				strcpy(sPath, sKey);
				if(iKeyLen && sPath[iKeyLen-1] != '\\')
					sPath[++iKeyLen] = '\\';

				for(; numKeys; --numKeys)
				{
					DWORD iThisKeyLen = iLength;
					RegEnumKeyEx(hKey, numKeys, sPath+iKeyLen+1, &iThisKeyLen, 0, NULL, NULL, NULL);
					sPath[iKeyLen+iThisKeyLen+1] = '\0';
					deleteKey(sPath);
				}
			}
			catch(...)
			{
				delete sPath;
				throw;
			}
			delete sPath;
		}

		RegCloseKey(hKey);

		// Finally, we delete this key.
		if(pDeleteFunc)
			pDeleteFunc(HKEY_LOCAL_MACHINE, sKey, KEY_WOW64_64KEY, NULL);
		else
			RegDeleteKey(HKEY_LOCAL_MACHINE, sKey);
	}

private:
	HKEY key;
	TiXmlString sKey;
	HMODULE hMod;
	FuncRegDeleteKeyEx pDeleteFunc;
};

// Converts a TiXmlString into a BSTR.
class BStrWrapper
{
public:
	BStrWrapper(const TiXmlString& sStr)
	{
		bstr = NULL;
		int iSize = ::MultiByteToWideChar(CP_ACP, 0, sStr.c_str(), -1, NULL, 0);
		WCHAR* sTmp = new WCHAR[iSize];
		::MultiByteToWideChar(CP_ACP, 0, sStr.c_str(), -1, sTmp, iSize);
		bstr = SysAllocString(sTmp);
		delete sTmp;
		if(!bstr)
			throw exception("Could not allocate memory for BSTR");
	}

	~BStrWrapper() { if(bstr) SysFreeString(bstr); }

	BSTR bstr;

private:
	BStrWrapper(const BStrWrapper&);
	BStrWrapper& operator=(const BStrWrapper&);
};

// Register the game with Games Explorer, potentially also telling it to update its GDF and
// clearing the task list, if the game is already registered.
// Returns false if this didn't work, which probably means that machine isn't Vista.
bool registerWithGamesExplorer(const GDFValues& values, const TiXmlString& sInstPath)
{
	// Register the game with Games Explorer.
	// This might fail.  If it does, it probably just means we're not on a Vista machine!
	// So, we don't throw an error, we just silently fail and continue on to the Media Centre bit.
	CComPtr<IGameExplorer> gameExplorer;
	if(S_OK == gameExplorer.CoCreateInstance(__uuidof(GameExplorer)))
	{
		// We're going to use the GDF's GUID to register ourselves into the Games Explorer.
		// Microsoft doesn't do this for the games that come with Windows, but there's nothing in
		// the docs that say not to, and it works fine.  Plus, it means we don't need to track
		// extraneous GUIDs.  (Although some would say that *one* GUID is extraneous.)
		GUID guid;
		HRESULT hr;
		{
			TiXmlString sBareGUID(values.sGameGUID.c_str() + 1, 36);	// Cut off the curly braces
			UuidFromString((unsigned char*)sBareGUID.c_str(), &guid);
		
			BStrWrapper bstrGamePath(values.sGamePath);
			BStrWrapper bstrInstPath(sInstPath);
			hr = gameExplorer->AddGame(bstrGamePath.bstr, bstrInstPath.bstr, GIS_ALL_USERS, &guid);
		}

		// This seems to return E_INVALIDARG if a game is already registered with our GUID, or if there
		// really is an invalid argument (like a badly formed GUID).  I haven't found any way
		// to tell the difference between the two errors.
		// MSDN says it returns S_FALSE if the game is already registered, but that doesn't seem to happen.
		// Experimentation shows that what they *really* meant was E_FAIL.
		if(SUCCEEDED(hr) || hr == E_FAIL || hr == S_FALSE || hr == E_INVALIDARG)
		{
			// The game is already installed on the system.
			// Let's tell Windows to update the GDF, and we'll also delete our tasks folder.
			if(FAILED(gameExplorer->UpdateGame(guid)))
				throw exception("Could not add or update game in Games Explorer");

			TiXmlString sGameTasks(getGameTasksFolder());
			sGameTasks += values.sGameGUID;
			deleteFile(sGameTasks.c_str());
		}
		// Any other failure is something we know less about.
		// (See Microsoft's documentation.  Hahahaha.  Oh, I crack myself up.)
		else if(FAILED(hr))
			throw exception("Could not add game to Games Explorer");

		return true;
	}
	else
		return false;
}

// Unregister the game with Games Explorer, also deleting the task list
bool unregisterWithGamesExplorer(const GDFValues& values)
{
	CComPtr<IGameExplorer> gameExplorer;
	if(S_OK == gameExplorer.CoCreateInstance(__uuidof(GameExplorer)))
	{
		GUID guid;
		TiXmlString sBareGUID(values.sGameGUID.c_str()+1, 36);	// Cut off the curly braces
		UuidFromString((unsigned char*)sBareGUID.c_str(), &guid);
		gameExplorer->RemoveGame(guid);

		// Delete the game tasks
		TiXmlString sGameTasks = getGameTasksFolder();
		sGameTasks += values.sGameGUID;
		deleteFile(sGameTasks.c_str());

		return true;
	}
	else
		return false;
}

// Register with Windows Media Center.
void registerWithMediaCenter(const GDFValues& values, const TiXmlString& sImgPath, const TiXmlString& sCapabilities, const TiXmlString& sRunPath)
{
	// For registering soemthing with Windows Media Center, you have a few options:
	// * Use an over-complicated, managed library that requires you to first create a temporary XML file.
	// * Use an over-complicated, hosted HTML library that requires you to first create a temporary XML file.
	// * Just put the damn things in the registry yourself, with the added benefit that users who install Media Center
	//   later will automatically have their apps included!
	//
	// I've opted for the latter option.
	TiXmlString sRoot("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Media Center\\Extensibility\\");
	RegMan regMan;

	// I should note that I'm using the GDF's GUID here, even though Microsoft's docs specifically tell you
	// not to.  However, their own /Age of Empires III/ uses their GDF GUID, so I figure it's okay for me to
	// do the same.

	// List ourselves in the applications menu
	regMan.openKey(sRoot + "Applications\\" + values.sGameGUID);
	regMan.setValue("Title", values.sGameName);
	if(values.sDeveloperName.size())
		regMan.setValue("CompanyName", values.sDeveloperName);

	// List ourselves in the Games service
	regMan.openKey(sRoot + "Categories\\Services\\Games\\" + values.sGameGUID);
	regMan.setValue("AppId", values.sGameGUID, REG_SZ);
	if(!sCapabilities.empty())
		regMan.setValue("CapabilitiesRequired", sCapabilities, REG_SZ);

	// Also in the More Programs service, for backwards compatability with Windows XP
	regMan.openKey(sRoot + "Categories\\More Programs\\" + values.sGameGUID);
	regMan.setValue("AppId", values.sGameGUID, REG_SZ);
	if(!sCapabilities.empty())
		regMan.setValue("CapabilitiesRequired", sCapabilities, REG_SZ);

	// Here, we're actually supposed to create a different GUID for each entry point, but, like
	// AoE3, I'm just going to reuse our GDF's GUID.
	// I imagine it's only a problem if you want multiple entry points for your game.
	regMan.openKey(sRoot + "Entry Points\\" + values.sGameGUID);
	regMan.setValue("AppId", values.sGameGUID, REG_SZ);
	regMan.setValue("Description", values.sGameDesc);
	regMan.setValue("Title", values.sGameName);
	regMan.setValue("Run", sRunPath.empty() ? values.sGamePath : sRunPath);
	if(!sImgPath.empty())
		regMan.setValue("ImageUrl", sImgPath);
}

// Unregister with media center
void unregisterWithMediaCenter(const GDFValues& values)
{
	RegMan regMan;
	TiXmlString sRoot("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Media Center\\Extensibility\\");
	regMan.deleteKey((sRoot + "Applications\\" + values.sGameGUID).c_str());
	regMan.deleteKey((sRoot + "Categories\\Services\\Games\\" + values.sGameGUID).c_str());
	regMan.deleteKey((sRoot + "Categories\\More Programs\\" + values.sGameGUID).c_str());
	regMan.deleteKey((sRoot + "Entry Points\\" + values.sGameGUID).c_str());
}