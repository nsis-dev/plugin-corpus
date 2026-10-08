This dll is a plugin for NSIS installation system and is designed to provide simple COM+ administration functions. The dll only allows creating and deleting applications and adding and removing components. It does not allow modification of application's properties as I didn't need this functionality for my installer. If anyone extends this dll with more functionality please email me at vakhmechet@intellectconsulting.com. I'd love to hear of your extensions and/or improvements.

*** FUNCTIONALITY ***
The plugin provides the following functions:

1. ComPlusAdmin::CreateApplication "Application Name" Timeout
This functions creates a COM+ application and accepts two paremeters. The first is the name of the application and the second is an integer value that specifies inactivity timeout after which COM+ shuts down the application.

2. ComPlusAdmin::DeleteApplication "Application Name"
This function deletes a COM+ application. It's only parameter is application name.

3. ComPlusAdmin::InstallComponent "Application Name" "C:\BinD\EEHCore.dll"
This function installs a component within the application. It accepts two parameters. The first parameter is the name of the application and the second parameter is a path to the component's dll file.

4. ComPlusAdmin::RemoveComponent "Application Name" "EEHCore.dll"
This function removes a component from the application. It accepts two parameters. The first parameter is the name of the application while the second parameter is the name of the component's DLL file (*not* the path).

*** ERROR HANDLING ***
Every function pushes a value on the stack upon its completion. If the function is executed successfully this value is 0. If the function failed this value is a negative integer (usually -1 or -2). If the function fails the $R0 register is assigned the detailed error message (either from COM+ or an error description I added).

*** SYSTEM REQUIREMENTS ***
This plugin works with COM+ services on Microsoft Windows 2000 and above. It does not work on Windows NT.
