/*

ExitStub - self-deleting executable launcher.

Written by Duncan McKellar, February-June 2006

Based on code by Arkon (http://ragestorm.net) July 2004 and the SelfDel NSIS plugin by Kichik.

Concept is clean (given what we are doing...) and relatively simple.

Basically, we launch another process suspended, build an exit stub in it's initial thread that will delete us, and then resume
the suspended thread/process.

The exit stub waits until the initial process terminates, then deletes its executable file and kills itself, leaving no evidence
of either process.

The 'host' process used is a standard Windows program, 'msiexec.exe', but any program that is known to reside on a computer and
can remain behind will do.

Also, note that this program will *NOT* work if it is built in a debug environment. We do some serious stuffing about with the stack, and
the debug code and stack checking stuff screws with our code. This makes this code hard to debug but it should be pretty static now it
works.

In essence, this program provides the capability for a program to be executed and then to leave no executable files behind.

******************************************************************************************************************************************
* WARNING:  Exercise extreme caution using this program, it will delete any program launched by it, including standard Windows programs
*           without confirmation or warning (which it can launch since it searches the path if neccessary to find the child program.)
*
*           YOU HAVE BEEN WARNED.
******************************************************************************************************************************************

1.1.0.0 DJM 23 May 2006
            Corrected a bug in the earlier release that relied on the presence of the MS CRT DLL.
            Required the inclusion of LIBTINYC.LIB and buffer.h to completely eliminate the need for the CRT.

            Wrote GetProcessFileName to determine the executable name for a process regardless of the
            platform. i.e. this code should work on all versions of Windows back to Windows 95. The new version
            also handles Windows NT without the PSAPI library by terminating the application with an error message
            rather than just hanging.

            The BuildExitStub function will not operate on Windows 98 since the VirtualAllocEx function that
            allows us to allocate memory in another process's memory space is not available on that platform.
            (This is a limitation of Arkon's code as well, despite the comments to the contrary below.)

1.1.1.0 DJM 6 June 2006
            Rewrote BuildExitStub to use the same method as SelfDel plugin to allow it run on Windows 98,
            as the VirtualAllocEx function is not supported on platforms before Windows NT.
            Change of code means that the ExitStub is now built on the host process stack rather than in
            a separately allocated memory block.
            
            This also introduces a timing issue under Windows XP. It is possible for the ExitStub process to
            terminate before the ExitStub built in the host process gains control and retrieves the handle 
            to us. A delay has been introduced in the WinMain process if there is no child process to
            compensate for this.
            
            A better option would have been to use synchronisation events to make the exit stub wait for a
            signal from this process but using a pointer to OpenEvent or CreateEvent causes the exit stub
            to fault and terminate the ExitStub prematurely.

            Removed casts from DWORDs to pointers and replaced with UIntPtr calls to remove compiler
            errors and to improve compatibility.

Original comments follow:

//
// Self Deleting Executable
// Arkon, http://ragestorm.net
// 1st July, 2004
//
// Supposed to work on all Windows versions!
// Was checked on: 98/2000/XP/2003.
// The following code does NOT use any function that Win9x won't support (not even CreateRemoteThread)!
//
// GreeTingZ: Assaf Segal, tH@nks y0
//
// Feel free to use or doing with it anything you wish...
// Please give credit where due.
//

//
// Proof of concept for self deleting executable:
// Spawning a new process in suspended mode, using CreateProcess.
// Allocating a memory block in that process, using VirtualAllocEx.
// Getting remote thread context.
// Write a thread procedure to that memory block.
// Write some data structure for late usage by the thread procedure.
// Update remote thread stack, for using our data structure.
// Setting remote thread new context.
// Resuming remote thread.
// Voila...
//
// Note that this idea, specifically, could be implemented in other ways, I chose the simples one IMHO.
//

*/

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "buffer.h"

// Maximum size of the exit stub code, this should be plenty.
#define MAX_EXITSTUB_SIZE (512)

typedef struct tagExitData
{
    TCHAR   szExecutableName[MAX_PATH];
    DWORD   nProcessID;
    HANDLE  (__stdcall *OpenProcessPtr)(DWORD, BOOL, DWORD);
    DWORD   (__stdcall *WaitForSingleObjectPtr)(HANDLE, DWORD);
    BOOL    (__stdcall *CloseHandlePtr)(HANDLE);
    BOOL    (__stdcall *DeleteFilePtr)(LPCTSTR);
    void    (__stdcall *SleepPtr)(DWORD);
    void    (__stdcall *ExitProcessPtr)(UINT);
} EXITSTUBDATA;
typedef EXITSTUBDATA *PEXITSTUBDATA;

// Definitions for pointers to dynamically linked functions.
typedef DWORD   (WINAPI *GETMODULEFILENAMEPROC)(HANDLE, HMODULE, LPSTR, DWORD);
typedef HANDLE  (WINAPI *SNAPSHOTPROC)(DWORD, DWORD);
typedef BOOL    (WINAPI *PROCESS32FIRSTPROC)(HANDLE, LPPROCESSENTRY32);
typedef BOOL    (WINAPI *PROCESS32NEXTPROC)(HANDLE, LPPROCESSENTRY32);

// Function prototypes.
DWORD GetProcessFileName(HANDLE hProcess, DWORD dwPID, LPTSTR lpszFileName, DWORD nSize);

DWORD WINAPI ExitStub(PEXITSTUBDATA pexitdata)

{

    HANDLE  hParentProcess;

    // Grab the handle for the parent process.
    hParentProcess = pexitdata->OpenProcessPtr(PROCESS_ALL_ACCESS, FALSE, pexitdata->nProcessID);
    // If that didn't work, just bail.
    if (NULL != hParentProcess)
    {
        // Otherwise, wait for the parent/victim process to terminate.
        pexitdata->WaitForSingleObjectPtr(hParentProcess, INFINITE);
        // Close our handle to it, this should make its handle count 0, so it can be deleted.
        pexitdata->CloseHandlePtr(hParentProcess);
        // Now the parent process has exited, delete it's executable file.
        // Keep trying every second until we succeed, which will happen once Windows has cleaned up.
        while (! pexitdata->DeleteFilePtr(pexitdata->szExecutableName))
            pexitdata->SleepPtr(1000);
        // Terminate the host process now we are done.
        pexitdata->ExitProcessPtr(0);
    }
    return(0);

}

UINT BuildExitStub(void)

{

    LPVOID              pExitStub;
    STARTUPINFO         StartInfo;
    PROCESS_INFORMATION ProcInfo;
    EXITSTUBDATA        exitdata;
    CONTEXT             ctx;
    DWORD               nBytesCopied;
    HANDLE              hProcess;
    HANDLE              hThread;
    UINT                nRetVal;
    DWORD               dwOldProtection;

    // Assume success.
    nRetVal = 0;

    // Build pointers to the functions that the ExitStub needs to use to do it'a thing.
    exitdata.OpenProcessPtr         = OpenProcess;
    exitdata.WaitForSingleObjectPtr = WaitForSingleObject;
    exitdata.CloseHandlePtr         = CloseHandle;
    exitdata.DeleteFilePtr          = DeleteFile;
    exitdata.SleepPtr               = Sleep;
    exitdata.ExitProcessPtr         = ExitProcess;
    exitdata.nProcessID             = GetCurrentProcessId();

    // Get the name of our executable so the ExitStub knows what to delete.
    GetModuleFileName(NULL, exitdata.szExecutableName, MAX_PATH);
    // Create a 'host' process for our exit stub, in this case msiexec.exe which should be on every Windows 98SE or later installation.
    ZeroMemory(&StartInfo, sizeof(STARTUPINFO));
    StartInfo.cb = sizeof(STARTUPINFO);
    if (CreateProcess(NULL, "msiexec.exe", NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, NULL, NULL, &StartInfo, &ProcInfo))
    {
        // Get handles for the process and the initial thread from the returned information about the process.
        hProcess = ProcInfo.hProcess;
        hThread = ProcInfo.hThread;
        // Get the context of the thread including EIP and ESP which we need to manipulate to hook in the ExitStub stub and
        // insert the ExitStubData.
        ctx.ContextFlags = CONTEXT_CONTROL;
        if (GetThreadContext(hThread, &ctx))
        {
            // Allocate space on the host process stack for our ExitStub code.
            ctx.Esp -= MAX_EXITSTUB_SIZE;
            pExitStub = UIntToPtr(ctx.Esp);
            // Copy our ExitStub code into the host process's memory (on it's stack).
            if (WriteProcessMemory(hProcess, (LPVOID) pExitStub, (LPVOID) &ExitStub, MAX_EXITSTUB_SIZE, &nBytesCopied))
            {
                // Create new EIP pointing to the ExitStub.
                ctx.Eip = ctx.Esp;
                // Assign permissions to the ExitStub code memory allowing it to execute.
                if (0 != VirtualProtectEx(hProcess, pExitStub, MAX_EXITSTUB_SIZE, PAGE_EXECUTE_READWRITE, &dwOldProtection))
                {
                    // Copy the ExitStub data block onto the thread's stack so that the ExitStub code can reach it.
                    // Make room on the stack for the EXITSTUBDATA block and then copy the data block in.
                    ctx.Esp -= sizeof(EXITSTUBDATA);
                    if (WriteProcessMemory(hProcess, UIntToPtr(ctx.Esp), (LPVOID) &exitdata, sizeof(EXITSTUBDATA), &nBytesCopied))
                    {
                        // Push the pointer to the Exit Stub data block onto the stack for the ExitStub to use.
                        // ExitStub is expecting a single parameter, a pointer to an EXITSTUBDATA block.
                        if (WriteProcessMemory(hProcess, UIntToPtr((ctx.Esp - sizeof(DWORD))), (LPVOID) &ctx.Esp, sizeof(DWORD), &nBytesCopied))
                        {
                            // Update ESP to allow for the new parameter 'pushed' and the 'return address' onto the stack.
                            ctx.Esp -= (sizeof(DWORD) * 2);
                            // Write the altered context back to the host thread.
                            if (SetThreadContext(hThread, &ctx))
                            {
                                // Resume the thread now we are done 'hijacking' it.
                                ResumeThread(hThread);
                            }
                            else
                            {
                                nRetVal = 7; // Failed to write new context.
                            }
                        }
                        else
                        {
                            nRetVal = 6; // Failed to write ExitStub stack frame.
                        }
                    }
                    else
                    {
                        nRetVal = 5; // Failed to write ExitStub data block.
                    }
                }
                else
                {
                    nRetVal = 4; // Failed to set memory protection for ExitStub.
                }
            }
            else
            {
                nRetVal = 3; // Failed to write ExitStub code.
            }
        }
        else
        {
            nRetVal = 2; // Failed to get thread context for host process.
        }
        // Cleanup our handles to the host process; if we don't it won't be able to be deleted.
        if (ProcInfo.hProcess)
            CloseHandle(ProcInfo.hProcess);
        if (ProcInfo.hThread)
            CloseHandle(ProcInfo.hThread);
    }
    else
    {
        nRetVal = 1; // Failed to create host process.
    }
    return(nRetVal);

}

// GetProcessFileName - gets the filename for the executable for a process that is not the current process.
//
// Uses PSAPI or ToolHelp, depending on what platform we are running on and dynamically links
// in the required libraries at runtime so runtime dependencies are reduced.
//
DWORD GetProcessFileName(HANDLE hProcess, DWORD dwPID, LPTSTR lpszFileName, DWORD nSize)

{

    HANDLE                  hSnapShot;
    HANDLE                  hLib;
    BOOL                    bMoreProcesses;
    BOOL                    bFound;
    DWORD                   dwRetVal;
    PROCESSENTRY32          peProcess;
    OSVERSIONINFO           oviVersionInfo;
    GETMODULEFILENAMEPROC   lpfnPSAPIProc;
    SNAPSHOTPROC            lpfnSnapShotProc;
    PROCESS32FIRSTPROC      lpfnP32FirstProc;
    PROCESS32NEXTPROC       lpfnP32NextProc;

    // Initialise return value, 0 = failure.
    dwRetVal = 0;

    // Get the Operating System information.
    oviVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&oviVersionInfo);

    // Use PSAPI if we are running Windows NT or later.
    if (VER_PLATFORM_WIN32_NT == oviVersionInfo.dwPlatformId)
    {
        // Explicitly load the PSAPI library.
        hLib = LoadLibrary("PSAPI.DLL");
        if (NULL != hLib)
        {
            // Create a reference to the GetModuleFileNameA function.
            lpfnPSAPIProc = (GETMODULEFILENAMEPROC) GetProcAddress(hLib, "GetModuleFileNameExA");
            if (NULL != lpfnPSAPIProc)
            {
                // Use the PSAPI function to get the process executable name.
                dwRetVal = (lpfnPSAPIProc)(hProcess, NULL, lpszFileName, nSize);
                // Release the function reference.
                FreeProcInstance(lpfnPSAPIProc);
            }
            // Release the PSAPI library reference.
            FreeLibrary(hLib);
        }
    }
    else
    {
        // Windows 98/95 or NT without the PSAPI library- use the ToolHelper functions to locate the process.
        if (VER_PLATFORM_WIN32_WINDOWS == oviVersionInfo.dwPlatformId)
        {
            // Explicitly load the ToolHelper library, KERNEL32.
            hLib = LoadLibrary("KERNEL32.DLL");
            if (NULL != hLib)
            {
                // Create references to the ToolHelper functions we will use.
                lpfnSnapShotProc =  (SNAPSHOTPROC)          GetProcAddress(hLib, "CreateToolhelp32Snapshot");
                lpfnP32FirstProc =  (PROCESS32FIRSTPROC)    GetProcAddress(hLib, "Process32First");
                lpfnP32NextProc =   (PROCESS32NEXTPROC)     GetProcAddress(hLib, "Process32Next");
                if ((NULL != lpfnSnapShotProc) &&
                    (NULL != lpfnP32FirstProc) &&
                    (NULL != lpfnP32NextProc))
                {
                    // Initially, we have not found the process.
                    bFound = FALSE;
                    // Create a snapshot of all processes in the system.
                    hSnapShot = (lpfnSnapShotProc)(TH32CS_SNAPPROCESS, 0);
                    // Only proceed if we got the snapshot.
                    if (INVALID_HANDLE_VALUE != hSnapShot)
                    {
                        // Walk through the list of processes, looking for a matching PID.
                        peProcess.dwSize = sizeof(PROCESSENTRY32);
                        bMoreProcesses = (lpfnP32FirstProc)(hSnapShot, &peProcess);
                        // Keep looking as long as we have not found the process and there are more processes.
                        while ((! bFound) && (bMoreProcesses))
                        {
                            if (dwPID == peProcess.th32ProcessID)
                            {
                                // Found our process, make a copy of the executable path.
                                lstrcpyn(lpszFileName, peProcess.szExeFile, nSize);
                                // Setup the return value in the same form as GetModuleFileNameEx's.
                                dwRetVal = lstrlen(lpszFileName);
                                // Show that we found the process we were looking for.
                                bFound = TRUE;
                            }
                            else
                            {
                                // Didn't find the process, move on to the next one.
                                bMoreProcesses = (lpfnP32NextProc)(hSnapShot, &peProcess);
                            }
                        }
                        // Cleanup the snapshot resources.
                        CloseHandle(hSnapShot);
                    }
                }
                // Release all of the dynamically linked function references.
                if (NULL != lpfnSnapShotProc)
                    FreeProcInstance(lpfnSnapShotProc);
                if (NULL != lpfnP32FirstProc)
                    FreeProcInstance(lpfnP32FirstProc);
                if (NULL != lpfnP32NextProc)
                    FreeProcInstance(lpfnP32NextProc);
                // Release the helper library reference.
                FreeLibrary(hLib);
            }
        }
    }
    return(dwRetVal);

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCommandLine, int nCmdshow)

{

    STARTUPINFO         StartInfo;
    PROCESS_INFORMATION ProcInfo;
    TCHAR               szChildExecutableName[MAX_PATH];
    int                 nRetVal;

    nRetVal = 0;

    // Try to get our own executable path to confirm whether the GetProcessFileName function will work
    // without locking up the application while waiting for our child process to initialise.
    //
    // In theory, this function should only fail if we are run under Windows NT without the PSAPI
    // library loaded or under Windows for Workgroups.
    //
    if (0 == GetProcessFileName(GetCurrentProcess(), GetCurrentProcessId(), szChildExecutableName, MAX_PATH))
    {
        MessageBox(NULL, "Requires PSAPI library.\r\nExecution cannot continue.", "ExitStub", MB_OK);
        return(10);
    }

    // Create the exit stub to delete us when we complete.
    if (0 == (nRetVal = BuildExitStub()))
    {
        // ExitStub is in place, launch the program specified on the command line.
        ZeroMemory(&StartInfo, sizeof(STARTUPINFO));
        StartInfo.cb = sizeof(STARTUPINFO);
        // Run the specified process provided on the command line.
        if (CreateProcess(NULL, lpszCommandLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
        {
            // Get the name of the executable the process launched.
            // The call will fail if the process has not completed initialisation when the call is made.
            // Therefore, if the call fails, wait a little while for process to initialise, and try again.
            while (0 == GetProcessFileName(ProcInfo.hProcess, ProcInfo.dwProcessId, szChildExecutableName, MAX_PATH))
            {
                Sleep(500);
            }
            // Wait until the child process exits before we 'complete'.
            WaitForSingleObject(ProcInfo.hProcess, INFINITE);
            // Cleanup after the child process.
            CloseHandle(ProcInfo.hProcess);
            CloseHandle(ProcInfo.hThread);
            // Continue trying to delete the child executable until it works, this should only fail until Windows cleans
            // up after the child process.
            while (! DeleteFile(szChildExecutableName))
                Sleep(1000);
        }
        else
        {
            // If there is no child process, delay for a brief period to allow the ExitStub to pick up its handle
            // to our process. Otherwise, the parent process could terminate before the ExitStub retrieves the handle.
            Sleep(500);
        }
    }

    // Return the result of setting up the ExitStub.
    return(nRetVal);

}
