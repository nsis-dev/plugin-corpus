
#include <windows.h>
#include <string>
#include <algorithm>
#include <cctype>
#include "Utils.h"

// COM administration
#import "c:\\winnt\\system32\\com\\Comadmin.dll"   rename_namespace("ComAdminLib")

using namespace std;

//////////////////////////////////////////////////////////////////////
//Case insensitive string comparison predicate.
//Thanks to Scott Myers -- Effective STL
//////////////////////////////////////////////////////////////////////
bool ciCharLess(char c1, char c2)
{
	return 
		tolower(static_cast<unsigned char>(c1)) <
		tolower(static_cast<unsigned char>(c2));
}
//////////////////////////////////////////////////////////////////////
//Case insensitive string comparison
//Thanks to Scott Myers -- Effective STL
//
// Returns true if s1 is lexographically less than s2 (case 
// insensitive); false otherwise.
//////////////////////////////////////////////////////////////////////
bool ciStringCompare(const std::string& s1, const std::string& s2)
{
	return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(), ciCharLess);
}

//////////////////////////////////////////////////////////////////////
// Returns true if two strings are equal (case insensitive); false
// otherwise.
//////////////////////////////////////////////////////////////////////
bool ciStringsEqual(const std::string& s1, const std::string& s2)
{
	return !(ciStringCompare(s1, s2) || ciStringCompare(s2, s1));
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			// Initialize COM
			CoInitialize(NULL);
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
		
		case DLL_PROCESS_DETACH:
			// Uninitialize COM
			CoUninitialize();
			break;
    }
    return TRUE;
}

extern "C" void __declspec(dllexport) CreateApplication(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	// Necessary code
	COMPLUSADMIN_INIT();

	// return value will be stored here
	char res[64];

	// Try to create the application
	try
	{
		// Create the catalog component
		ComAdminLib::ICOMAdminCatalogPtr Catalog("COMAdmin.COMAdminCatalog");
		// Grab the applications catalog
		ComAdminLib::ICatalogCollectionPtr Applications = Catalog->GetCollection (L"Applications");
		Applications->Populate();

		// Add our application
		ComAdminLib::ICatalogObjectPtr NewApplication;
		NewApplication = Applications->Add();

		// Grab the parameters
		char AppName[255];
		popstring(AppName);
		char Timeout[255];
		popstring(Timeout);

		// Set appropriate options
		_variant_t vntName(AppName);
		_variant_t vntTimeout(atol(Timeout));
		NewApplication->put_Value(L"Name", vntName);
		NewApplication->put_Value(L"ShutdownAfter", vntTimeout);

		// Save the changes
		Applications->SaveChanges();
	}
	catch (_com_error &e)
	{
		// Set the error
		setuservariable(INST_R0, (LPSTR)e.ErrorMessage());

		// Return bad result
		wsprintf(res, "%d", -1);
		pushstring(res);

		return;
	}

	// return good
	wsprintf(res, "%d", 0);
	pushstring(res);
}

extern "C" void __declspec(dllexport) DeleteApplication(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
	// Necessary code
	COMPLUSADMIN_INIT();

	// return value will be stored here
	char res[64];

	// See if removed
	bool fRemoved = false;
	
	try
	{
		// Create the catalog component
		ComAdminLib::ICOMAdminCatalogPtr Catalog("COMAdmin.COMAdminCatalog");

		// Grab the applications catalog
		ComAdminLib::ICatalogCollectionPtr Applications = Catalog->GetCollection (L"Applications");
		Applications->Populate();

		// Grab the app name
		char AppName[255];
		popstring(AppName);
		
		// Remove our application
		int nCount = Applications->Count;
		for(int i = 0; i < nCount; i++)
		{
			ComAdminLib::ICatalogObjectPtr App = Applications->GetItem(i);
			_variant_t vntName;
			App->get_Value(L"Name", &vntName);
			if (ciStringsEqual(std::string(AppName), static_cast<std::string>(static_cast<_bstr_t>(vntName.bstrVal))))
			{
				Applications->Remove(i);
				Applications->SaveChanges();
				fRemoved = true;
				break;
			}

		}
	}
	catch (_com_error &e)
	{
		// Set the error
		setuservariable(INST_R0, (LPSTR)e.ErrorMessage());

		// Return bad result
		wsprintf(res, "%d", -1);
		pushstring(res);

		return;
	}

	if(!fRemoved)
	{
		// Set the error
		setuservariable(INST_R0, "The application does not exist.");

		// Return bad result
		wsprintf(res, "%d", -2);
		pushstring(res);

		return;
	}

	// Return ok
	wsprintf(res, "%d", 0);
	pushstring(res);
}

extern "C" void __declspec(dllexport) InstallComponent(HWND hwndParent, int string_size, char *variables, stack_t **stacktop)
{
	// Necessary code
	COMPLUSADMIN_INIT();

	// return value will be stored here
	char res[64];

	try
	{
		// Grab the params
		char AppName[255];
		popstring(AppName);
		char DllPath[255];
		popstring(DllPath);

		// Create the catalog component
		ComAdminLib::ICOMAdminCatalogPtr Catalog("COMAdmin.COMAdminCatalog");

		// Add our component
		if(S_OK != Catalog->InstallComponent(AppName, DllPath, L"", L""))
		{
			// Return bad result
			wsprintf(res, "%d", -1);
			pushstring(res);

			return;
		}

	}
	catch (_com_error &e)
	{
		// Set the error
		setuservariable(INST_R0, (LPSTR)e.ErrorMessage());

		// Return bad result
		wsprintf(res, "%d", -1);
		pushstring(res);

		return;
	}

	// return good
	wsprintf(res, "%d", 0);
	pushstring(res);
}

extern "C" void __declspec(dllexport) RemoveComponent(HWND hwndParent, int string_size,
                                      char *variables, stack_t **stacktop)
{
	// Necessary code
	COMPLUSADMIN_INIT();

	// return value will be stored here
	char res[64];

	try
	{
		// Create the catalog component
		ComAdminLib::ICOMAdminCatalogPtr Catalog("COMAdmin.COMAdminCatalog");

		// Grab the applications catalog
		ComAdminLib::ICatalogCollectionPtr Applications = Catalog->GetCollection (L"Applications");
		Applications->Populate();

		// Grab the app name
		char AppName[255];
		popstring(AppName);

		// Find our application
		int nCount = Applications->Count;
		ComAdminLib::ICatalogObjectPtr TargetApp = NULL;
		for(int i = 0; i < nCount; i++)
		{
			ComAdminLib::ICatalogObjectPtr App = Applications->GetItem(i);
			_variant_t vntName;
			App->get_Value(L"Name", &vntName);
			std::string current_app_name(static_cast<std::string>(static_cast<_bstr_t>(vntName.bstrVal)));

			if (ciStringsEqual(AppName, current_app_name))
			{
				TargetApp = App;
				Catalog->ShutdownApplication(vntName.bstrVal); // we have to shut it down first
				break;
			}

		}

		// If the app wasn't found...
		if(TargetApp == NULL)
		{
			// Set the error
			setuservariable(INST_R0, "Could not find the application.");

			// Return bad result
			wsprintf(res, "%d", -1);
			pushstring(res);

			return;
		}

		// Now that we have the app, find the component
		char ComponentName[255];
		popstring(ComponentName);
		ComAdminLib::ICatalogCollectionPtr Components = Applications->GetCollection("Components", TargetApp->Key);
		Components->Populate();
		int TargetIndex = -1;

		for(i = Components->Count -1; i >= 0; i--)
		{
			ComAdminLib::ICatalogObjectPtr Component = Components->GetItem(i);
			_variant_t vntName;
			Component->get_Value(L"DLL", &vntName);
			string DllName = static_cast<string>(static_cast<_bstr_t>(vntName.bstrVal));
			DllName = DllName.substr(DllName.find_last_of('\\') + 1);

			//MessageBox(NULL, DllName.c_str(), "", MB_OK);
			if (ciStringsEqual(std::string(ComponentName), DllName))
			{
				TargetIndex = i;
				// Remove the component
				Components->Remove(TargetIndex);
			}

		}
		// If the component wasn't found...
		if(TargetIndex == -1)
		{
			// Set the error
			setuservariable(INST_R0, "Could not find the component.");

			// Return bad result
			wsprintf(res, "%d", -1);
			pushstring(res);

			return;
		}

		// Save the changes
		Components->SaveChanges();

	}
	catch (_com_error &e)
	{
		// Set the error
		setuservariable(INST_R0, (LPSTR)e.ErrorMessage());

		// Return bad result
		wsprintf(res, "%d", -1);
		pushstring(res);

		return;
	}

	// Return ok
	wsprintf(res, "%d", 0);
	pushstring(res);
}
