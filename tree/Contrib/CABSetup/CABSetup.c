/*

Cabinet-based Setup Plug-in for NSIS

Developed by Duncan McKellar (Mr Inches) 2006

Microsoft's SetupAPI does most of the work, this plugin provides an NSIS
interface to these functions for handling disk changes and extracting
data files across multiple source media.

Based initially on the MSDN example for using SetupIterateCabinet, and grew from there.
Examples drawn from several sources on the Internet, but most of the code here is my original work.

BuildSetupFiles was written from scratch as an interface to Microsoft's MAKECAB utility.

0.0.0.1 DJM 27 January 2006
            Initial version, handles cabinet sets and prompts for new media as required.

0.0.0.2 DJM 01 February 2006
            Fixed bug that was preventing the spanned cabinet processing from processing all cabinets when
            the cabinet set spans cabinets and disks.

0.0.0.2 DJM 03 February 2006
            Changed FilePathSplit to allow Path and/or Filename to be NULL, meaning the caller doesn't care about that value.
            Initially used by the cabinet callback to isolate any embedded path within the cabinet.

            Added code to create the path to the cabinet destination directory if it doesn't exist. This means cabinets with
            embedded paths work properly now.

            Fixed bug in the Extract function that was missing the /ALL switch if it was not the first parameter specified.

            Built CreatePath function to create a complete path including intermediary directories.

0.0.0.3 DJM 06 February 2006
            Built BuildSetupFiles function to automate the construction of the MAKECAB directive file to assist in the 
            creation of the spanned cabinet sets that this plug-in expects.

0.0.0.4 DJM 08 February 2006
            Added code to centre the console created to hold MAKECAB on the screen over the top of the NSIS window.

            Split the code to create the setup files/directive file into separate functions allowing the addition
            of multiple file specifications to the setup files before the cabinets are created.

0.0.0.5 DJM 11 February 2006
            Built GetNSISParameterList and GetNSISParameter to allow easier access to the NSIS parameters from the plug-in.
            Completed code to handle the BuildSetupFiles process.

            Rewrote the Extract function to use the new parameter list functions.

            Added code to handle the failure of the MAKECAB process from within BuildSetupFiles and display this information
            to the user.

0.0.0.6 DJM 14 February 2006
            Add code to allow the caller to specify a path to place the setup cabinets in that is different to the path that
            the directive file is created in (this is the /target switch).

0.0.0.7 DJM 15 February 2006
            Added version information to directive file header.

0.0.0.8 DJM 20 February 2006
            Changed CreateDirectiveFileEntry to enclose the filename in double quotes (") to ensure that the plug-in handles
            filenames and source paths with spaces in them.

0.0.0.9 DJM 21 February 2006
            Changed output routines in CabinetCallBack to use a different format.

            FilePathSplit and FilePathJoin no longer have return values.

            Changed the CabinetCallBack function to return ERROR_OPERATION_ABORTED (995) rather than
            ERROR_INVALID_DISK_CHANGE (137), since this better reflects the user cancelling the operation as well as the next
            media not being supplied, both result stop the extract process.

1.0.0.0 DJM 28 February 2006
            Changed the order for the DiskSize1 directive to place it following the DiskSize directive in the directive file.

            Added the Compress option for BuildSetupFiles to allow the caller to specify the compression type.
            This is to help with Windows NT compatibility; the SetupIterateCabinet call fails under NT if the LZX
            compression method is used to create the cabinets.

            Corrected the logic in the Extract function to only perform parameter validation if the parameters were
            successfully parsed. Oops.

            Added informational headers to the DDF file created by BuildSetupFiles to assist in correlating
            Disk-to-Cabinet-to-File mappings.

            Initial release of CABSetup.

1.1.0.0 DJM 6 June 2006
            Corrected a bug in the earlier release that relied on the presence of the MS CRT DLL.
            Required the inclusion of LIBTINYC.LIB and the writing of custom versions of strtok,
            strrchr and atoi to completely eliminate the need for the CRT while keeping the
            binary size small.

            Fixed bug/design issue in the GetNSISParameterList function that emptied the NSIS stack.
            This causes problems for callers that have variables saved on the stack before calling
            any of our functions.

            Added code to make the Report function write to the DetailLogText field as well
            as the DetailLogList.

1.2.0.0 DJM 23 June 2006
            Added a progress bar to the extraction process using code from the RealProgress
            plug-in as a guide. The progress is determined by reading the summary report file
            that MAKECAB generates during the creation of the cabinets. This file is now
            specified as the /reportfile parameter to the Extract call.
            The inclusion of this function requires the inclusion of the MS CRTL functions
            for 64 bit arithmetic, however the majority of the CRTL is still not required
            and excluded.

            Added the /reportfile switch to the BuildSetupFiles function. This is to allow
            a separate, distinct summary report file for each dataset.

            Replacement progress bar is only created and destroyed if the report file parameter is
            specified and the data set size is read successfully.

1.2.1.0 DJM 31 August 2006
            DataSetSize function returns error code and dataset size separately.
            BuildSetupFiles and Extract now return LRESULTs instead of UINTs.

1.2.2.0 DJM 11 September 2006
            Rewrote DataSetSize to locate the data set size by searching for the data string in the
            report file rather than searching for the fourth line of the file.

1.2.3.0 DJM 3 October 2006
            Rewrote BuildSetupFiles to take two parameters to specify the data set name and path
            rather than passing a number of different file parameters for the directive file,
            report file, and the information file. Each of these files is now named after the
            data set name and path. Only the report file is required for DataSetSize and Extract
            functions.

            Fixed a bug in the BuildSetupFiles function that would create the wrong directive file
            if the dataset path did not exist. BuildSetupFiles will now attempt to create the
            dataset path if it does not exist.

            Allowed the Compress option for BuildSetupFiles to be 'None' to turn off compression
            for cabinets.

1.3.0.0 DJM 12 December 2006
            MakeProgressBar made static function - like it should have been.

            Added code to allow the user to cancel the extract process by enabling the NSIS
            window's Cancel button and hooking its window procedure. If the extraction process
            is aborted, then Extract will return ERROR_OPERATION_ABORTED.

            Rewrote the Report function to use a set of flags to control how it displays the
            reported information instead of the BOOLean flag used previously. This also corrected
            a design fault in the Report function where the DetailText was always updated with
            the (new) DetailLog entry. The new function allows the DetailLog and the DetailText
            controls to be updated separately.

            Added function to display the extraction rate and estimated time to completion for
            the Extract operation. (/SHOWRATE option for Extract function).
            (This code borrows heavily from the Inetc and InetLoad plug-in code to calculate
            and display this information - thanks Takhir.)

            To accomodate this information (which is displayed in the DetailText), extraction
            progress information is now only written to the DetailLog and not the DetailText
            if the option to show the rate is specified.

            Added option to resume an extraction process where it left off. (/RESUME option for
            Extract function). This resume function will not work if the extraction process has
            crossed a cabinet boundary before it is stopped. This is a function of how
            IterateCabinet currently operates.

            Relocated progress update code outside of CabinetCallback since this code is now used
            twice in the same function. This code is now in the ShowExtractionProgress function
            which incorporates the call to the ShowExtractRate function (if the extract rate is
            being shown).

            Added code to ignore any time that passes while waiting for a disk to be supplied to
            avoid skewing the extract rate if a disk change is needed.

            Added code to create the target path specified for the Extract function if it does
            not exist. Not sure why I didn't do this before, hopefully this won't break anything
            else.

1.3.1.0 DJM Fixed bug in FormatNumberString that was causing crashes under Vista; isolated to function
            freeing the caller's memory buffer (pszBuffer) instead of local allocated buffer (pszTempBuffer) - 
            generally considered a bad idea. This should have failed under XP, but I guess Vista is more
            strict with memory allocation/deallocation.

            TODO List
            ---------
            Tighten up error handling between internal functions.

            Handle UNC roots as well as drive/path roots.

            Allow use of wildcards in Extract (assuming that the SetupAPI functions can support this).

            Localisation for languages other than English (AUS).

            Capture output of ProcessDirectiveFile's run of MAKECAB and display in DetailLog.

*/

#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <setupapi.h>
#include <commctrl.h>
#include <limits.h>
#include "..\ExDll\ExDll.h"

// Link with SetupAPI.Lib - provides our workhorse functions, SetupIterateCabinet and SetupPromptForDisk.
#pragma comment (lib, "setupapi.lib")
#pragma comment (lib, "libctiny.lib")
#pragma comment (lib, "comctl32.lib")

HINSTANCE   g_hModule = NULL;
HWND        g_hwndParent;
HWND        g_hwndDetailLog;        // Handle to the Detail Log.
LONG        g_lpfnDetailLogDlgProc; // DetailLog's window procedure.
BOOL        g_bCancelled;           // Indicates the user wants to cancel the current CABSetup process.

// Handles for maintaining the progress bars, both NSIS's and ours.
HWND        g_hwndNSISProgressBar;
HWND        g_hwndCABProgressBar;

// Variables for maintaining the current progress of the extraction.
DWORD       g_dwDataSetSize;
DWORD       g_dwDataExtracted;

// Time extract operation commenced - used to provide estimate of completion time for Extract.
DWORD       g_dwStartTicks;

// Strings for displaying time units.
TCHAR       szHour[]    = "hour";
TCHAR       szMinute[]  = "minute";
TCHAR       szSecond[]  = "second";
TCHAR       szPlural[]  = "s";

// Data block passed to the cabinet callback function by SetupIterateCabinet.
typedef struct tagCABData
{
    PTSTR       pszSourcePath;
    PTSTR       pszCurrentCabinet;
    PCTSTR      pszTargetPath;
    PCTSTR      pszFileName;
    UINT        Operation;
    DWORD       dwCurrentFileSize;
} CABDATA;
typedef CABDATA *PCABDATA;

// Pointer-to-function prototype for callback functions called by ScanDir.
typedef BOOL (*DIRENTRYPROC) (PCTSTR pszEntryName, PVOID pContextData);

// Data block passed to the callback function called by ScanDir.
typedef struct tagDirProcData
{
    HANDLE  hFile;
} DIRPROCDATA;
typedef DIRPROCDATA *PDIRPROCDATA;

// Data block for building the setup cabinet files.
typedef struct tagSetupFileData
{
    HANDLE  hDirectiveFile;
    PTSTR   pszDiskSize;
    PTSTR   pszDisk1Size;
    PTSTR   pszDiskNameTemplate;
    PTSTR   pszCABNameTemplate;
    PTSTR   pszDiskDirNameTemplate;
    PTSTR   pszCompressType;
    PTSTR   pszDataSet;
    PTSTR   pszDataSetPath;
} SETUPFILEDATA;
typedef SETUPFILEDATA *PSETUPFILEDATA;

// Function prototypes.

// Callback function to be called by SetupIterateCabinet to process each cabinet in a set.
LRESULT WINAPI CabinetCallback(PVOID pContextData, UINT Notification, UINT_PTR Param1, UINT_PTR Param2);

// Functions to show the extraction rate and progress.
static void ShowExtractRate(void);
static void ShowExtractionProgress(UINT nOperation, DWORD dwFileSize);

// Function to process a CABinet file set and perform the requested operation for each file found in it.
static UINT IterateCabinet(PCTSTR pszCabinetFile, PCTSTR pszCabinetPath, PCTSTR pszTargetPath, PCTSTR szExtractFile, UINT Operation);

// Functions to parse and retrieve parameters passed from NSIS.
static BOOL GetNSISParameterList(PTSTR *ppszParamList);
static BOOL GetNSISParameter(PCTSTR pszParamList, PCTSTR pszParamName, PTSTR pszParamValue, BOOL bRetrieveValue);

// Function to construct the progress bar for showing the progress of the extract process.
static HWND MakeProgressBar(void);

// Replacement window procedure to allow us to capture the Cancel button from the NSIS DetailLog.
static LRESULT DetailLogProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

// Functions to construct and process the directive file to create the setup cabinets.
static BOOL OpenSetupFile(PSETUPFILEDATA psdSetupInfo);
static BOOL AddFilesToSetup(PSETUPFILEDATA psdSetupInfo, PCTSTR pszPath, PCTSTR pszFileSpec, BOOL bIncludeSubDirs);
static BOOL CloseSetupFile(PSETUPFILEDATA psdSetupInfo);
static BOOL ProcessDirectiveFile(PTSTR pszDirectiveFile, PCTSTR pszTargetPath);

// Supporting functions for creating the directive file.
static BOOL ScanDir(PCTSTR pszPath, PCTSTR pszFileSpec, DIRENTRYPROC pCallback, PDIRPROCDATA pProcData, BOOL bIsScanRoot, BOOL bScanSubDirs);
static BOOL CreateDirectiveFileEntry(PCTSTR pszEntry, PVOID pData);
static BOOL WriteText(HANDLE hFile, PCTSTR pszFormat, ...);

// Supporting functions for all functions.
static void Report(UINT nFlags, PCTSTR pszFormat, ...);
static void FormatNumberString(UINT nNumber, PTSTR pszBuffer);
static void FormatUnitString(DWORD dwValue, LPTSTR pszValueString);
static void FilePathSplit(PCTSTR pszFullName, PTSTR pszPath, PTSTR pszFileName);
static void FilePathJoin(PTSTR pszFullName, PCTSTR pszPath, PCTSTR pszFileName);
static BOOL IsDirectory(PCTSTR pszPath);
static BOOL CreatePath(PCTSTR pszPath);
static void StripQuotes(PTSTR pszString);
static LRESULT ReadFileLine(HANDLE hFile, LPTSTR pszText, DWORD nMaxLen);
static LRESULT GetDataSetSize(PTSTR pszInfoFileName, LPDWORD lpdwSetSize);

// Private functions to replacing CRT functions.
static int my_atoi(PCTSTR pszSource);
static LONG my_atol(PCTSTR pszSource);
static PTSTR my_strrchr(PCTSTR pszString, TCHAR cChar);
static PTSTR my_strtok(PTSTR pszString, PCTSTR pcszDelimiters);

// Version Information.
#define VERSION_STRING          "1.3.1.0"

// Unique name for CABSetup mutex object.
#define CABSETUP_MUTEX_NAME     "CABSetupMutex"

// Flags to control the Report function behaviour.
#define RPT_NEW_ITEM            (0x00001) // create a new item/line.
#define RPT_TEXT                (0x00002) // display text in detail text line.
#define RPT_LOG                 (0x00004) // display text in detail log list.

// Macros for common combinations of the Report flags.
#define RPT_NEW_TEXT            (RPT_NEW_ITEM | RPT_TEXT)
#define RPT_APPEND_TEXT         (RPT_TEXT)

#define RPT_NEW_LOG             (RPT_NEW_ITEM | RPT_LOG)
#define RPT_APPEND_LOG          (RPT_LOG)

#define RPT_NEW_BOTH            (RPT_NEW_ITEM | RPT_TEXT | RPT_LOG)
#define RPT_APPEND_BOTH         (RPT_TEXT | RPT_LOG)

// Flags to represent the parameters passed from NSIS for the Extract function.
#define CAB_OP_EXTRACTALL       (0x0001)
#define CAB_OP_EXTRACTFILE      (0x0002)
#define CAB_OP_SHOWPROGRESS     (0x0004)
#define CAB_OP_SHOWRATE         (0x0008)
#define CAB_OP_RESUME           (0x0010)
#define CAB_OP_ALL_CABINETS     (0x1000)

// Shorthand macros for allocating and freeing memory from the default process heap.
#define MALLOC(Type, Size)         (Type)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (Size))
#define REALLOC(Type, Ptr, Size)   (Type)  HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Ptr, (Size))
#define FREE(Ptr)                          HeapFree(GetProcessHeap(), 0, (Ptr))

/*
    Marker for the top of the parameters passed on the stack.

    This is the marker that GetNSISParameter list will stop on to prevent it
    emptying the entire stack.
*/
#define END_PARAM_MARKER        "|"

// Dialog control IDs for the DetailLog text control.
#define IDC_DETAIL_TEXT         (1006)
#define IDC_DETAIL_LIST         (1016)
#define IDC_PROGRESS            (1004)
#define IDC_CABPROGRESS         (1005)

// Shorthand for the normal unit sizes.
#define KILOBYTE                (1024UL)
#define MEGABYTE                (1024UL * KILOBYTE)
#define GIGABYTE                ((DWORD) (1024UL * MEGABYTE))

/*
    Macro to determine whether a file system object exists.

    This macro is subject to the same constraints as the IsDirectory function -
    see the description for this function for details.
*/
#define Exists(path)    (INVALID_FILE_ATTRIBUTES != GetFileAttributes(path))

// DLL Entry/Exit function.
//
BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)

{

    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
            g_hModule = hInstance;
            break;

        case DLL_PROCESS_DETACH:
            g_hModule = NULL;
            break;
    }
    return(TRUE);

}

// Callback function to be called by SetupIterateCabinet.
//
LRESULT WINAPI CabinetCallback(PVOID pContextData, UINT Notification, UINT_PTR Param1, UINT_PTR Param2)

{

    TCHAR                   szNewPath[MAX_PATH];
    TCHAR                   szCabinetTargetPath[MAX_PATH];
    TCHAR                   szFileSize[32];
    PFILE_IN_CABINET_INFO   pFileInfo;
    PCABINET_INFO           pCabInfo;
    PFILEPATHS              pFilePaths;
    PCABDATA                pCabData;
    LRESULT                 lRetVal;
    DWORD                   dwReportFlags;
    DWORD                   dwStartWaitTicks;

    lRetVal = 0;
    pCabData = (PCABDATA) pContextData;

    // If the user requested to Cancel the extract process, signal the abort to SetupIterateCabinet.
    if (g_bCancelled)
    {
        return(FILEOP_ABORT);
    }

    // If we are showing the extract rate, then limit the extract information to the DetailLog.
    if (pCabData->Operation & CAB_OP_SHOWRATE)
    {
        dwReportFlags = (RPT_LOG);
    }
    else
    {
        // Otherwise, extract information goes to both the DetailLog and the DetailText field.
        dwReportFlags = (RPT_TEXT | RPT_LOG);
    }

    switch(Notification)
    {
        case SPFILENOTIFY_FILEINCABINET:
            pFileInfo = (PFILE_IN_CABINET_INFO) Param1;
            // If we are extracting all files,
            if (pCabData->Operation & CAB_OP_EXTRACTALL)
            {
                // Show we want to extract the file.
                lRetVal = FILEOP_DOIT;
            }
            // If we are extracting a specific file,
            if (pCabData->Operation & CAB_OP_EXTRACTFILE)
            {
                // ...and the notify is for the file we to extract.
                if (0 == lstrcmp(pFileInfo->NameInCabinet,pCabData->pszFileName))
                {
                    // Show we want to extract the file.
                    lRetVal = FILEOP_DOIT;
                }
                else
                {
                    // This isn't the file you're looking for...
                    lRetVal = FILEOP_SKIP;
                }
            }
            if (FILEOP_DOIT == lRetVal)
            {
                // Isolate any path element in the file in the cabinet.
                FilePathSplit(pFileInfo->NameInCabinet, szCabinetTargetPath, NULL);
                // Add the path element from the cabinet to the target path.
                FilePathJoin(szNewPath, pCabData->pszTargetPath, szCabinetTargetPath);
                // This function also serves to test if a directory exists.
                if (! IsDirectory(szNewPath))
                {
                    // Create the path to extract the cabinet to.
                    if (! CreatePath(szNewPath))
                    {
                        Report(dwReportFlags | RPT_NEW_ITEM, "Failed to create cabinet target path %s", szNewPath);
                        lRetVal = FILEOP_SKIP;
                    }
                }
                // Only extract the file if the path to extract to exists.
                if (FILEOP_DOIT == lRetVal)
                {
                    FilePathJoin(pFileInfo->FullTargetName, pCabData->pszTargetPath, pFileInfo->NameInCabinet);
                    FormatNumberString(pFileInfo->FileSize, szFileSize);
                    // If the RESUME option has been selected and the target file exists, skip the extraction.
                    // The dataset progress is still updated since the file has already been 'extracted'.
                    if ((pCabData->Operation & CAB_OP_RESUME) && (Exists(pFileInfo->FullTargetName)))
                    {
                        Report(dwReportFlags | RPT_NEW_ITEM, "Skipping %s (%s bytes), target exists.", pFileInfo->NameInCabinet, szFileSize);
                        ShowExtractionProgress(pCabData->Operation, pFileInfo->FileSize);
                        lRetVal = FILEOP_SKIP;
                    }
                    else
                    {
                        Report(dwReportFlags | RPT_NEW_ITEM, "Extracting %s (%s bytes)...", pFileInfo->NameInCabinet, szFileSize);
                    }
                    // Save the uncompressed size of the file for updating the extraction progress.
                    pCabData->dwCurrentFileSize = pFileInfo->FileSize;
                }
            }
            break;

        case SPFILENOTIFY_FILEEXTRACTED:
            pFilePaths = (PFILEPATHS) Param1;
            // Show the outcome of the extract in the Details log.
            if (NO_ERROR == pFilePaths->Win32Error)
            {
                // Update the progress bar and extraction rate if we are tracking them.
                ShowExtractionProgress(pCabData->Operation, pCabData->dwCurrentFileSize);
                Report(dwReportFlags, "done.");
            }
            else
            {
                Report(dwReportFlags, "failed.");
            }

            // This callback should never return an error code.
            lRetVal = NO_ERROR;
            break;

        case SPFILENOTIFY_NEEDNEWCABINET:
            pCabInfo = (PCABINET_INFO) Param1;
            /*
                Record a watermark for the tick count before the user interaction.
                This allows us to ignore the ticks that pass while we are handling
                user input.

                This ensures that the extract rate and estimated time to completion
                are not affected by time spent waiting for the user to respond to
                the prompt for a new disk.
            */
            dwStartWaitTicks = GetTickCount();
            switch(SetupPromptForDisk(NULL, "Setup - Files Needed",
                                     (PCSTR) pCabInfo->DiskName,
                                     (PCSTR) pCabData->pszSourcePath,
                                     (PCSTR) pCabInfo->CabinetFile,
                                     NULL,
                                     (IDF_CHECKFIRST | IDF_NOBEEP | IDF_NOCOMPRESSED),
                                     szNewPath,
                                     MAX_PATH,
                                     NULL))
            {
                case DPROMPT_SUCCESS:
                    /*
                        NOTE: Param2 contains a pointer to the (new) source path for the cabinet.
                        The MSDN is not very clear on this, it simply notes that "this parameter
                        is a pointer to a null-terminated string".
                        What is not clearly noted, is that the callback function must copy the new
                        string into the buffer pointed to by Param2.
                    */
                    lstrcpy((LPSTR) Param2, szNewPath);
                    // Update the source path in the context data.
                    lstrcpy(pCabData->pszSourcePath, szNewPath);
                    // Update the current cabinet name in the context data.
                    lstrcpy(pCabData->pszCurrentCabinet, pCabInfo->CabinetFile);
                    // Show that we got a new valid source path for the cabinet and that
                    // processing should continue.
                    lRetVal = NO_ERROR;
                    break;

                case DPROMPT_CANCEL:
                default:
                    // Show that the user cancelled the request for the new disk.
                    lRetVal = ERROR_OPERATION_ABORTED;
                    break;
            }
            // Update the start tick count with those that passed during the user interaction. 
            g_dwStartTicks += (GetTickCount() - dwStartWaitTicks);
            dwStartWaitTicks = 0;
            break;

    }
    return(lRetVal);

}

/*
    Function to update the progress bar and extraction rate.

    Not strictly necessary, but since this functionality is required twice in the cabinet callback
    procedure, this means there is only one copy of the code and it can be called elsewhere in the
    plug-in.

    The function assumes that the CABSetup progress bar has been created and is ready to receive
    messages, that the dataset size has been set. If the dataset size has not been set, then the
    function will return without any action or display.
*/
static void ShowExtractionProgress(UINT nOperation, DWORD dwFileSize)

{

    DWORD   dwPercentDone;

    // If the dataset size has not been set, return with no output.
    if (0 == g_dwDataSetSize)
        return;

    // Only update the progress bar if we are showing the extraction process.
    if (nOperation & CAB_OP_SHOWPROGRESS)
    {
        g_dwDataExtracted += dwFileSize;
        dwPercentDone = (DWORD) ((UInt32x32To64(g_dwDataExtracted, 100UL) / g_dwDataSetSize));
        // Update the progress bar for this file.
        SendMessage(g_hwndCABProgressBar, PBM_SETPOS, (WPARAM) dwPercentDone, 0L);
        // If we are showing the extraction rate, do so now.
        if (nOperation & CAB_OP_SHOWRATE)
        {
            ShowExtractRate();
        }
    }

}

/*
    Function to show the rate at which data is being extracted and an estimated time to complete
    the extraction of the entire dataset.
    
    This rate is a guesstimate based on the same algorithm used by the Inetc and InetLoad
    plug-ins.

    The function assumes that the DataSetSize has been calculated/retrieved and that the system
    tick count at the start of the extraction process is in g_dwStartTicks.

    If these values are not populated, the function returns without showing any information; it
    is the caller's responsibility to ensure these preconditions are met.

*/
static void ShowExtractRate(void)

{

    DWORD       dwCurrentTicks;
    DWORD       dwElapsedSeconds;
    DWORD       dwExtractRate;
    LONG        lRemainTime;
    TCHAR       szUnitString[32];
    PTSTR       pszRemainUnitText;

    /*
        If no data has been extracted yet, no dataset size set, or the start time is not set,
        then there is nothing to show.
    */
    if ((0 == g_dwDataExtracted) || (0 == g_dwDataSetSize) || (0 == g_dwStartTicks))
        return;

    //  Calculate the number of seconds that have elapsed since the extraction started.
    dwCurrentTicks = GetTickCount();
    dwElapsedSeconds = ((dwCurrentTicks - g_dwStartTicks) / 1000);
    
    /*
        If we have not been extracting data for a least one second, the amount of data
        extracted is the extract rate.
    
        Otherwise, calculate the average extract rate across the time we have spent
        extracting data.
    */
    if (0 != dwElapsedSeconds)
    {
        dwExtractRate = (g_dwDataExtracted / dwElapsedSeconds);
    }
    else
    {
        dwExtractRate = g_dwDataExtracted;
    }

    // Display the extraction rate data in the DetailText only.
    FormatUnitString(g_dwDataExtracted, szUnitString);
    Report(RPT_NEW_TEXT, "Extracted %s ", szUnitString);
    FormatUnitString(g_dwDataSetSize, szUnitString);
    Report(RPT_APPEND_TEXT, "of %s ", szUnitString);
    FormatUnitString(dwExtractRate, szUnitString);
    Report(RPT_APPEND_TEXT, "@ %s/s, ", szUnitString);

    // Estimate the remaining time to extract the dataset based on our progress so far.
    lRemainTime = (LONG) ((UInt32x32To64(dwElapsedSeconds, g_dwDataSetSize) / g_dwDataExtracted) - dwElapsedSeconds);

    if (lRemainTime < 0)
    {
        lRemainTime = 0;
    }

    // Determine the correct unit size for the estimated amount of time left to completion.
    pszRemainUnitText = szSecond;
    if (lRemainTime >= 60)
    {
        lRemainTime /= 60;
        pszRemainUnitText = szMinute;
        if (lRemainTime >= 60)
        {
            lRemainTime /= 60;
            pszRemainUnitText = szHour;
        }
    }
    
    // Show the estimated remaining time in the DetailText only.
    Report(RPT_APPEND_TEXT,"%ld %s%s remaining", lRemainTime, pszRemainUnitText, (1 == lRemainTime) ? "" : szPlural);

}

/*
	The SetupIterateCabinet function behaves just like the EXTRACT command utility when handling cabinet sets that are spanned
	across multiple cabinets.

	That is, it will extract folders that begin in the specified cabinet, but will not process any other folders contained in
	the cabinet that contained the last folder without a specific command switch (/A).
	
	This IterateCabinet function mirrors that behaviour using a flag to indicate whether to iterate all folders in all cabinets.

    IterateCabinet - processes each cabinet within a set. Recursively calls itself to process any other cabinets in the set
    if a file/folder crosses a cabinet set.
*/
static UINT IterateCabinet(PCTSTR pszCabinetFile, PCTSTR pszCabinetPath, PCTSTR pszTargetPath, PCTSTR szExtractFile, UINT Operation)

{

    TCHAR   szCurrentCabinetFile[MAX_PATH];
    TCHAR   szCurrentCabinetName[MAX_PATH];
    CABDATA CabinetData;
    UINT    lRetVal;

    lRetVal = NO_ERROR;

    // Make a copy of the specified cabinet filename so we can tell if are crossing a cabinet set.
    lstrcpy(szCurrentCabinetName, pszCabinetFile);

    // Populate the context data structure with the source path to use to locate cabinets within a set,
    // the target path, the current cabinet, the (optional) filename to extract and the type of operation to perform.
    CabinetData.pszSourcePath     = (PTSTR) pszCabinetPath;
    CabinetData.pszTargetPath     = (PTSTR) pszTargetPath;
    CabinetData.pszCurrentCabinet = (PTSTR) szCurrentCabinetName;
    CabinetData.pszFileName       = szExtractFile;
    CabinetData.Operation         = Operation;

    // Build a full path to the cabinet file.
    FilePathJoin(szCurrentCabinetFile, pszCabinetPath, szCurrentCabinetName);
    // If we are showing the extract rate, limit our progress report to the DetailLog.
    if (Operation & CAB_OP_SHOWRATE)
        Report(RPT_NEW_LOG, "Processing %s (%s)...", szCurrentCabinetFile, szCurrentCabinetName);
    else
        Report(RPT_NEW_BOTH, "Processing %s (%s)...", szCurrentCabinetFile, szCurrentCabinetName);

    if (! SetupIterateCabinet(szCurrentCabinetFile, 0, (PSP_FILE_CALLBACK) CabinetCallback, (PVOID) &CabinetData))
    {
        lRetVal = GetLastError();
    }
    // If we are to process all cabinets, we check whether we have changed from processing the initial cabinet.
    // If so, we must have processed a folder that was part of a cabinet set.
    // Continue processing by iterating across this cabinet to extract the remaining folders in that cabinet.
    // Note that this will generally only work if the filespec to extract is "*.*".
    if ((NO_ERROR == lRetVal) && (Operation & CAB_OP_ALL_CABINETS))
    {
        if (0 != lstrcmp(pszCabinetFile, szCurrentCabinetName))
        {
            lRetVal = IterateCabinet(szCurrentCabinetName, CabinetData.pszSourcePath, pszTargetPath, szExtractFile, Operation);
        }
    }
    return(lRetVal);

}

/*
    Function to collect all of the parameters passed to the plugin into a single list with each individual
    parameters separated by a '|' symbol. This list can then be parsed/searched for particular switches by
    the GetNSISParameter function.
*/
static BOOL GetNSISParameterList(PTSTR *ppszParamList)

{

    PTSTR   pszParamList;
    PTSTR   pszParam;
    BOOL    bSucceeded;
    BOOL    bEndOfParams;

    bSucceeded = FALSE;
    // Grab a chunk of memory to hold the list and a chunk to hold each parameter as it is collected.
    // If either of these calls fail, we fail.
    pszParamList = MALLOC(PTSTR, g_stringsize);
    pszParam = MALLOC(PTSTR, g_stringsize);
    if ((NULL != pszParamList) && (NULL != pszParam))
    {
        bEndOfParams = FALSE;
        // While there are more parameters to parse...
        while (! bEndOfParams)
        {
            // Attempt to pop a parameter from the NSIS stack.
            if (0 == popstring(pszParam))
            {
                // Have we reached the end of parameter list marker?
                if (0 == lstrcmp(pszParam, END_PARAM_MARKER))
                {
                    // Yes...we are done.
                    bEndOfParams = TRUE;
                }
                else
                {
                    // No...add the parameter to the list.
                    //
                    // If this is not the first parameter, separate it from the list with '|'.
                    if (0 != lstrlen(pszParamList))
                        lstrcat(pszParamList, "|");
                    // Append the parameter to the list.
                    lstrcat(pszParamList, pszParam);
                }
            }
            else
            {
                // No more parameters - we are done.
                bEndOfParams = TRUE;
            }
        }
        // Save the parameter list memory block for return to the caller.
        *(ppszParamList) = pszParamList;
        bSucceeded = TRUE;
    }
    if (NULL != pszParam)
        FREE(pszParam);
    return(bSucceeded);

}

/*
    Function to parse and extract parameters from the list created by GetNSISParameterList.

    Returns TRUE if the parameter is found and if bRetrieveValue is TRUE, returns any value after a 
    trailing '=' sign in the buffer pointed to by pszParamValue. Parameter matching is case-insensitive.
    Returns FALSE and does not change pszParamValue, if the specified parameter is not found in the list.
*/
static BOOL GetNSISParameter(PCTSTR pszParamList, PCTSTR pszParamName, PTSTR pszParamValue, BOOL bRetrieveValue)

{

    PTSTR   pszParamBuffer;
    PTSTR   pszKeywordBuffer;
    PTSTR   pszParam;
    PTSTR   pszKeyword;
    PCTSTR  pszArg;
    BOOL    bFound;

    // Initially the parameter has not been found.
    bFound = FALSE;

    // Exit with failure if the ParamList has not been initialised.
    if (NULL != pszParamList)
    {
        pszParamBuffer = MALLOC(PTSTR, g_stringsize);
        pszKeywordBuffer = MALLOC(PTSTR, g_stringsize);

        if ((NULL != pszParamBuffer) && (NULL != pszKeywordBuffer))
        {
            // Parse each parameter off the list built by GetNSISParameterList and then check for our keywords.
            // Get pointer to argument list.
            pszArg = pszParamList;
            // Continue the search while we have not found the parameter and more parameters.
            while ((! bFound) && ('\0' != *pszArg))
            {
                // Point pszParam to the start of the parameter buffer.
                pszParam = pszParamBuffer;
                // Parameters in the list are separated by '|'.
                while (('\0' != *pszArg) && ('|' != *pszArg))
                {
                    *pszParam = *pszArg;
                    pszParam++;
                    pszArg++;
                }
                // Terminate the parameter parsed from the list.
                *pszParam = '\0';
                // Parse the parameter looking for keywords.
                // Keywords can be followed by equal signs and values or not.
                // Any value specified for the parameter is only returned if the bGetValue parameter is TRUE.
                // Copy the (potential) keyword up to the first '=' sign or end of the string.
                pszParam = pszParamBuffer;
                pszKeyword = pszKeywordBuffer;
                while ((*pszParam != '\0') && (*pszParam != '='))
                {
                    *pszKeyword = *pszParam;
                    pszKeyword++;
                    pszParam++;
                }
                // Terminate the keyword string.
                *pszKeyword = '\0';
                // If we terminated on an '=', isolate the value provided after the '=' sign.
                if ('\0' != *pszParam)
                {
                    // Move the parameter pointer to the character following the keyword and the '=' sign.
                    pszParam++;
                    // Strip any quote characters added to the parameter strings by NSIS.
                    StripQuotes(pszParam);
                }
                // Check if we got a match for the keyword.
                if (0 == lstrcmpi(pszParamName, pszKeywordBuffer))
                {
                    // Show we found the value.
                    bFound = TRUE;
                    // If we are to retrieve the value for the keyword, do so now.
                    if (bRetrieveValue)
                    {
                        lstrcpy(pszParamValue, pszParam);
                    }
                }
                // Done processing parameter, move onto next parameter unless we are at end of the parameter list.
                if ((! bFound) && ('|' == *pszArg))
                    pszArg++;
            }
            if (NULL != pszKeywordBuffer)
                FREE(pszKeywordBuffer);
            if (NULL != pszParamBuffer)
                FREE(pszParamBuffer);
        }
    }
    return(bFound);

}

/*
    MakeProgressBar - replaces the NSIS progress bar with one of our own that we use to show the extraction progress.
*/
static HWND MakeProgressBar(void)

{

    HWND    hwndProgress;
    RECT    rProgressBarPos;

    // Initialise the return value.
    hwndProgress = NULL;

    // Get and save the NSIS progress bar control.
    g_hwndNSISProgressBar = GetDlgItem(g_hwndDetailLog, IDC_PROGRESS);
    // Get the size of the NSIS progress bar.
    GetWindowRect(g_hwndNSISProgressBar, &rProgressBarPos);
    // Convert window size to a size relative to the NSIS main window.
    MapWindowPoints(HWND_DESKTOP, g_hwndParent, (LPPOINT) &rProgressBarPos, 2);
    // Create a new progress bar.
    hwndProgress = CreateWindow("msctls_progress32", "", WS_CHILD | WS_VISIBLE,
                                rProgressBarPos.left,                           // left position
                                rProgressBarPos.top,                            // top position
                                (rProgressBarPos.right - rProgressBarPos.left), // width
                                (rProgressBarPos.bottom - rProgressBarPos.top), // height
                                g_hwndParent, (HMENU) IDC_CABPROGRESS, NULL, NULL);
    // If we managed to create the new progress bar,
    if (NULL != hwndProgress)
    {
        // Set the style of the new progress bar to that of the old one.
        SetWindowLongPtr(hwndProgress, GWL_STYLE, GetWindowLongPtr(g_hwndNSISProgressBar, GWL_STYLE));
        // Hide the original NSIS progress window.
        ShowWindow(g_hwndNSISProgressBar, SW_HIDE);
        // Show our progress window.
        ShowWindow(hwndProgress, SW_SHOW);
        UpdateWindow(hwndProgress);
    }
    return(hwndProgress);

}

/*
	usage from NSIS is Extract /source=CABName /target=DestDir [/all] [/file=filename] [/reportfile=filename] [/showrate] [/resume] |
		order of switches is not important
*/
extern void __declspec(dllexport) Extract(HWND parent, int string_size, char *variables, stack_t **stacktop)

{

    TCHAR       szFullCabinetName[MAX_PATH];
    TCHAR       szSourceCabinetName[MAX_PATH];
    TCHAR       szSourcePath[MAX_PATH];
    TCHAR       szTargetPath[MAX_PATH];
    TCHAR       szFileToExtract[MAX_PATH];
    TCHAR       szReportFileName[MAX_PATH];
    PTSTR       pszResultBuffer;
    PTSTR       pszParameterList;
    PTSTR       pszParamValue;
    UINT        iFlags;
    LRESULT     lResult;
    HANDLE      hCancelMutex;

    EXDLL_INIT();

    g_hwndParent = parent;
    // Locate the DetailLog dialog and controls.
    g_hwndDetailLog = FindWindowEx(g_hwndParent, NULL, "#32770", NULL);

    // Initialise all of the possible parameters.
    *szFullCabinetName  = '\0';
    *szTargetPath       = '\0';
    *szFileToExtract    = '\0';
    *szReportFileName   = '\0';
    iFlags              = 0;

    lResult = NO_ERROR;

    // Get the parameters for the plugin and a buffer to hold parameters as we parse them.
    // If either fails, we cannot do anything other than return an error.
    if ((GetNSISParameterList(&pszParameterList)) && (NULL != (pszParamValue = MALLOC(PTSTR, g_stringsize))))
    {
        if (GetNSISParameter(pszParameterList, "/source", pszParamValue, TRUE))
        {
            lstrcpy(szFullCabinetName, pszParamValue);
        }
        if (GetNSISParameter(pszParameterList, "/target", pszParamValue, TRUE))
        {
            lstrcpy(szTargetPath, pszParamValue);
        }
        if (GetNSISParameter(pszParameterList, "/all", NULL, FALSE))
        {
            iFlags |= CAB_OP_ALL_CABINETS;
        }
        if (GetNSISParameter(pszParameterList, "/file", pszParamValue, TRUE))
        {
            lstrcpy(szFileToExtract, pszParamValue);
            iFlags |= CAB_OP_EXTRACTFILE;
        }
        if (GetNSISParameter(pszParameterList, "/reportfile", pszParamValue, TRUE))
        {
            lstrcpy(szReportFileName, pszParamValue);
            iFlags |= CAB_OP_SHOWPROGRESS;
        }
        if (GetNSISParameter(pszParameterList, "/showrate", NULL, FALSE))
        {
            iFlags |= CAB_OP_SHOWRATE;
        }
        if (GetNSISParameter(pszParameterList, "/resume", NULL, FALSE))
        {
            iFlags |= CAB_OP_RESUME;
        }

        // Release the memory allocated to parse the parameters.
        FREE(pszParameterList);
        FREE(pszParamValue);

        // If the call does not specify a file to extract, assume that means extract all files.
        if (! (iFlags & CAB_OP_EXTRACTFILE))
            iFlags |= CAB_OP_EXTRACTALL;

        /*
            Basic validation of parameters before passing to IterateCabinet.
            Current checks are for:
            - empty source cabinet name
            - empty target path
            - specifying the /FILE parameter with no filename.
            - specifying the /SHOWRATE parameter without the /REPORTFILE parameter
        */
        if (0 == lstrlen(szFullCabinetName))
        {
            lResult = ERROR_INVALID_PARAMETER;
        }
        if (0 == lstrlen(szTargetPath))
        {
            lResult = ERROR_INVALID_PARAMETER;
        }
        if ((iFlags & CAB_OP_EXTRACTFILE) && (0 == lstrlen(szFileToExtract)))
        {
            lResult = ERROR_INVALID_PARAMETER;
        }
        // Cannot show extract rate if there is no report file (and thus dataset size).
        if ((iFlags & CAB_OP_SHOWRATE) && (0 == lstrlen(szReportFileName)))
        {
            lResult = ERROR_INVALID_PARAMETER;
        }
        // If the target path does not exist, attempt to create it.
        if (! IsDirectory(szTargetPath))
        {
            /// Attempt to create path failed, return the error to the caller.
            if (! CreatePath(szTargetPath))
            {
                lResult = GetLastError();
            }
        }

        // Only process the cabinet(s) if the parameters check out.
        if (NO_ERROR == lResult)
        {
            // If we are showing the extraction progress, create the progress bar control and initialise its position.
            if (iFlags & CAB_OP_SHOWPROGRESS)
            {
                // Initially we have not extracted any data.
                g_dwDataExtracted = 0;
                // Get the size of the dataset from the reportfile.
                lResult = GetDataSetSize(szReportFileName, &g_dwDataSetSize);
                // If the dataset size could not be retrieved then disable the progress bar and display of
                // progress and extract rate.
                if (NO_ERROR != lResult)
                {
                    iFlags &= (~CAB_OP_SHOWPROGRESS);
                    iFlags &= (~CAB_OP_SHOWRATE);
                }
                else
                {
                    // Create our own progress bar to replace the NSIS one.
                    g_hwndCABProgressBar = MakeProgressBar();
                    if (NULL != g_hwndCABProgressBar)
                    {
                        // Progress bar will display the percentage of files extracted.
                        SendMessage(g_hwndCABProgressBar, PBM_SETRANGE, 0, MAKELPARAM (0, 100));
                        // We need to display something in the progress bar otherwise it won't be displayed.
                        SendMessage(g_hwndCABProgressBar, PBM_SETPOS, 1, 0L);
                        SendMessage(g_hwndCABProgressBar, PBM_SETPOS, 0, 0L);
                    }
                    else
                    {
                        // If we failed to create the progress bar, don't try to display the progress.
                        iFlags &= (~CAB_OP_SHOWPROGRESS);
                    }
                }
            }

            /*
                Create a mutex to protect the cancelled flag as two threads will want potentially want to
                access to it, the plugin and the NSIS thread.
            */
            hCancelMutex = CreateMutex(NULL, TRUE, CABSETUP_MUTEX_NAME);
            // Initially user has not cancelled the Extract process.
            g_bCancelled = FALSE;
            // Release the mutex now we have updated the variable.
            ReleaseMutex(hCancelMutex);
            // Hook in our window procedure so we get the Cancel button presses.
            g_lpfnDetailLogDlgProc = SetWindowLong(parent, DWL_DLGPROC, PtrToLong(DetailLogProc));
            // Enable the Cancel button on the NSIS DetailLog window.
            EnableWindow(GetDlgItem(parent, IDCANCEL), TRUE);

            // Separate the cabinet name and the source path to cabinet(s).
            FilePathSplit(szFullCabinetName, szSourcePath, szSourceCabinetName);

            // Get the time we started extracting data.
            g_dwStartTicks = GetTickCount();

            // Extract the contents of the specified cabinet to the target directory.
            lResult = IterateCabinet(szSourceCabinetName, szSourcePath, szTargetPath, szFileToExtract, iFlags);

            // Unhook our window procedure and restore the NSIS default procedure.
            SetWindowLong(parent, DWL_DLGPROC, PtrToLong(g_lpfnDetailLogDlgProc));
            // Close our handle to the mutex, this should destroy it.
            CloseHandle(hCancelMutex);

            // Set up the return value if the operation was cancelled.
            if (g_bCancelled)
            {
                lResult = ERROR_OPERATION_ABORTED;
            }

            // If we created the progress bar, remove it.
            if (iFlags & CAB_OP_SHOWPROGRESS)
            {
                // Remove our progress bar if we added it.
                if (NULL != g_hwndCABProgressBar)
                {
                    DestroyWindow(g_hwndCABProgressBar);
                    ShowWindow(g_hwndNSISProgressBar, SW_SHOW);
                }
            }
        }
    }
    else
    {
        lResult = ERROR_OUTOFMEMORY;
    }
    // Save the result/error onto the NSIS stack.
    // This will either be NO_ERROR (0) or the result of the GetLastError from the last failure.
    // If we don't have enough memory to pass back the result, return -1 instead.
    pszResultBuffer = MALLOC(PTSTR, g_stringsize);
    if (NULL != pszResultBuffer)
    {
        wsprintf(pszResultBuffer, "%u", lResult);
        pushstring(pszResultBuffer);
        FREE(pszResultBuffer);
    }
    else
    {
        pushstring("-1");
    }

}

/*
    Function to hook the messages passed to the NSIS DetailLog while we are
    extracting the data.

    This allows us to trap the press of the Cancel button without having to
    create our own separate dialog.
*/
static LRESULT DetailLogProc(HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)

{

    HANDLE  hMutex;

    switch(nMsg)
    {
        // Is this a control message?
        case WM_COMMAND:
            // Is it the Cancel button being pressed?
            if (IDCANCEL == LOWORD(wParam))
            {
                // Grab a handle to the cancel flag mutex.
                hMutex = OpenMutex(MUTEX_MODIFY_STATE, FALSE, CABSETUP_MUTEX_NAME);
                // Wait to acquire the mutex.
                WaitForSingleObject(hMutex, INFINITE);
                // Set the 'want-to-cancel' flag.
                g_bCancelled = TRUE;
                // Release the mutex now our update is done.
                ReleaseMutex(hMutex);
                CloseHandle(hMutex);
                // Show we processed the message so NSIS doesn't try to process it.
                return(TRUE);
            }
            break;
    }

    // Pass the message on to the original NSIS procedure.
    CallWindowProc(LongToPtr(g_lpfnDetailLogDlgProc), hwnd, nMsg, wParam, lParam);

    return(FALSE);

}

/*
    BuildSetupFiles - constructs a Diamond Directive File (.DDF) for use with the MAKECAB.EXE utility provided with Windows.
    Windows XP and 2003 have it installed by default, other Windows versions may have to get it from either the SUPPORT.CAB file
    on the Windows CD or from the Cabinet SDK from Microsoft. As far as I know this will work with any version of MAKECAB.EXE.

    Usage from NSIS is

        BuildSetupFiles /NOUNLOAD /CREATE /DISKSIZE=xxxx /DISK1SIZE=xxxx \
                                  /DISKNAME=xxxx /CABNAME=xxxx \
                                  /DISKDIR=xxxx \
                                  /COMPRESS=xxxx \
                                  /DATASET=xxxx \
                                  /DATASETPATH=xxxx |
        BuildSetupFiles /NOUNLOAD /ADD="C:\SOURCE\*.EXE" /S |
        BuildSetupFiles /NOUNLOAD /CLOSE |
        BuildSetupFiles /NOUNLOAD /MAKEFILES /TARGET=xxxx |
        BuildSetupFiles /DESTROY |

    NOTE:   It is very important that the plugin is not unloaded between calls to each function until the /DESTROY function is
            called. This is because the plugin retains information in a function-global (local static) variable and this variable
            is re-initialised if the library is unloaded and reloaded.
            The /NOUNLOAD switch must not be present in the call to /DESTROY otherwise the NSIS will not be able to delete
            the plugin and remove the $PLUGINS directory.

*/
extern void __declspec(dllexport) BuildSetupFiles(HWND parent, int string_size, char *variables, stack_t **stacktop)

{

    static  SETUPFILEDATA   SetupFileInfo;
            TCHAR           szFilePath[MAX_PATH];
            TCHAR           szFileSpec[MAX_PATH];
            TCHAR           szDirectiveFile[MAX_PATH];
            PTSTR           pszParameterList;
            PTSTR           pszParamValue;
            PTSTR           pszResultBuffer;
            BOOL            bIncludeSubDirs;
            LRESULT         lResult;

    EXDLL_INIT();
    g_hwndParent = parent;
    // Locate the DetailLog dialog and controls.
    g_hwndDetailLog = FindWindowEx(g_hwndParent, NULL, "#32770", NULL);

    lResult = NO_ERROR;
    // Get the parameters for the plugin and a buffer to hold parameters as we parse them.
    // If either fails, we cannot do anything other than return an error.
    if ((GetNSISParameterList(&pszParameterList)) && (NULL != (pszParamValue = MALLOC(PTSTR, g_stringsize))))
    {
        // /CREATE switch - gather the settings from the parameters and create stub directive file.
        if (GetNSISParameter(pszParameterList, "/create", NULL, FALSE))
        {
            // Allocate memory for the SetupFile structure and fail the call if we cannot get all memory.
            SetupFileInfo.pszDiskSize            = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszDisk1Size           = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszCABNameTemplate     = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszDiskNameTemplate    = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszDiskDirNameTemplate = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszCompressType        = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszDataSet             = MALLOC(PTSTR, MAX_PATH);
            SetupFileInfo.pszDataSetPath         = MALLOC(PTSTR, MAX_PATH);

            if ((NULL == SetupFileInfo.pszDiskSize) ||
                (NULL == SetupFileInfo.pszDisk1Size) ||
                (NULL == SetupFileInfo.pszDiskNameTemplate) ||
                (NULL == SetupFileInfo.pszCABNameTemplate) ||
                (NULL == SetupFileInfo.pszDiskDirNameTemplate) ||
                (NULL == SetupFileInfo.pszCompressType) ||
                (NULL == SetupFileInfo.pszDataSet) ||
                (NULL == SetupFileInfo.pszDataSetPath))
            {
                lResult = ERROR_OUTOFMEMORY;
            }
            else
            {
                // Populate the SetupFile structure with default values.
                lstrcpy(SetupFileInfo.pszDiskSize, "CDROM");
                // First disk field is blank, meaning don't use a specific size for Disk1.
                lstrcpy(SetupFileInfo.pszDiskNameTemplate, "SETUP#*");
                lstrcpy(SetupFileInfo.pszCABNameTemplate, "DATA*.CAB");
                lstrcpy(SetupFileInfo.pszDiskDirNameTemplate, "Disk*");
                lstrcpy(SetupFileInfo.pszCompressType, "LZX");
                lstrcpy(SetupFileInfo.pszDataSet, "SETUP");
                lstrcpy(SetupFileInfo.pszDataSetPath, ".\\");

                // Get each of the parameters from the plugin call and replace the default values with them
                // if they are present.
                if (GetNSISParameter(pszParameterList, "/disksize", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszDiskSize, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/disk1size", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszDisk1Size, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/diskname", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszDiskNameTemplate, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/cabname", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszCABNameTemplate, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/diskdir", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszDiskDirNameTemplate, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/compress", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszCompressType, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/dataset", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszDataSet, pszParamValue);
                }
                if (GetNSISParameter(pszParameterList, "/datasetpath", pszParamValue, TRUE))
                {
                    lstrcpy(SetupFileInfo.pszDataSetPath, pszParamValue);
                }
                // If the dataset path exists or we successfully created it, create the directive file.
                if ((IsDirectory(SetupFileInfo.pszDataSetPath)) || (CreatePath(SetupFileInfo.pszDataSetPath)))
                {
                    // Create the directive file and write the initial headers to it.
                    // If the operation fails, return the error code to the caller.
                    if (! OpenSetupFile(&SetupFileInfo))
                    {
                        lResult = GetLastError();
                    }
                }
                else
                {
                    // Create of dataset directory failed - return the error code.
                    lResult = GetLastError();
                }
            }
        }
        // /ADD switch - add the files specified by the parameter and include subdirectories if requested.
        if (GetNSISParameter(pszParameterList, "/add", pszParamValue, TRUE))
        {
            FilePathSplit(pszParamValue, szFilePath, szFileSpec);
            if (GetNSISParameter(pszParameterList, "/s", pszParamValue, FALSE))
                bIncludeSubDirs = TRUE;
            else
                bIncludeSubDirs = FALSE;
            // If the operation fails, return the error code to the caller.
            if (! AddFilesToSetup(&SetupFileInfo, szFilePath, szFileSpec, bIncludeSubDirs))
            {
                lResult = GetLastError();
            }
        }
        // /CLOSE - close the directive file.
        if (GetNSISParameter(pszParameterList, "/close", NULL, FALSE))
        {
            // If the operation fails, return the error code to the caller.
            if (! CloseSetupFile(&SetupFileInfo))
            {
                lResult = GetLastError();
            }
        }
        // /MAKEFILES - run MAKECAB to process the directive file and create the setup cabinets.
        if (GetNSISParameter(pszParameterList, "/makefiles", NULL, FALSE))
        {
            *pszParamValue = '\0';
            // If the caller specified a target path, get a copy of it in ParamValue.
            // If no path was specified, pszParamValue will remain as an empty string,
            // indicating 'use the default target path'.
            GetNSISParameter(pszParameterList, "/target", pszParamValue, TRUE);
            
            // Build the directive filename based on the dataset path and name.
            FilePathJoin(szDirectiveFile, SetupFileInfo.pszDataSetPath, SetupFileInfo.pszDataSet);
            lstrcat(szDirectiveFile, ".DDF");

            // If the operation fails, return the error code to the caller.
            if (! ProcessDirectiveFile(szDirectiveFile, pszParamValue))
            {
                lResult = GetLastError();
            }
        }
        // /DESTROY - clean up after setup process is complete.
        if (GetNSISParameter(pszParameterList, "/destroy", NULL, FALSE))
        {
            FREE(SetupFileInfo.pszDiskSize);
            FREE(SetupFileInfo.pszDisk1Size);
            FREE(SetupFileInfo.pszCABNameTemplate);
            FREE(SetupFileInfo.pszDiskNameTemplate);
            FREE(SetupFileInfo.pszDiskDirNameTemplate);
            FREE(SetupFileInfo.pszCompressType);
            FREE(SetupFileInfo.pszDataSet);
            FREE(SetupFileInfo.pszDataSetPath);
        }
        // Release the memory allocated to parse the parameters.
        FREE(pszParameterList);
        FREE(pszParamValue);
    }
    else
    {
        lResult = ERROR_OUTOFMEMORY;
    }
    // Save the result/error onto the NSIS stack.
    // This will either be NO_ERROR (0) or the result of the GetLastError from the last failure.
    // If we don't have enough memory to pass back the result, return -1 instead.
    pszResultBuffer = MALLOC(PTSTR, g_stringsize);
    if (NULL != pszResultBuffer)
    {
        wsprintf(pszResultBuffer, "%u", lResult);
        pushstring(pszResultBuffer);
        FREE(pszResultBuffer);
    }
    else
    {
        pushstring("-1");
    }
    return;

}

/*
    Opens the Diamond Directive File (DDF) and writes the initial headers to it, including parameters passed to the plugin from NSIS.
*/
static BOOL OpenSetupFile(PSETUPFILEDATA psdSetupInfo)

{

    BOOL    bSucceeded;
    HANDLE  hDirectiveFile;
    TCHAR   szFileName[MAX_PATH];

    bSucceeded = TRUE;

    // Build the directive filename based on the dataset name.
    FilePathJoin(szFileName, psdSetupInfo->pszDataSetPath, psdSetupInfo->pszDataSet);
    lstrcat(szFileName, ".DDF");

    // Create and write the standard headers to the specified directive file.
    Report(RPT_NEW_BOTH, "Creating directive file %s...", szFileName);
    // Create and write the standard headers to the specified directive file.
    hDirectiveFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hDirectiveFile)
    {
        // Save the file handle for other calls to use.
        psdSetupInfo->hDirectiveFile = hDirectiveFile;
        // Write the headers to the directive file, inserting the settings specified by the caller.
        // No syntax checking is performed, so MAKECAB will fail if these values are invalid.
        bSucceeded = ((WriteText(hDirectiveFile, "; Cabinet (Diamond) Directive File for data set \"%s\"", psdSetupInfo->pszDataSet)) &&
                      (WriteText(hDirectiveFile, " - Generated by CABSetup %s\r\n", VERSION_STRING)) &&
                      (WriteText(hDirectiveFile, ";\r\n")) &&
                      (WriteText(hDirectiveFile, ".OPTION EXPLICIT\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set DiskLabelTemplate=%s\r\n", psdSetupInfo->pszDiskNameTemplate)) &&
                      (WriteText(hDirectiveFile, ".Set CabinetNameTemplate=%s\r\n", psdSetupInfo->pszCABNameTemplate)) &&
                      (WriteText(hDirectiveFile, ".Set DiskDirectoryTemplate=%s\r\n", psdSetupInfo->pszDiskDirNameTemplate)) &&
                      (WriteText(hDirectiveFile, ".Set MaxDiskSize=%s\r\n", psdSetupInfo->pszDiskSize)));
        // If a specific first disk size is requested, write it to the directive file.
        if (0 != lstrlen(psdSetupInfo->pszDisk1Size))
        {
            bSucceeded = (bSucceeded && (WriteText(hDirectiveFile, ".Set MaxDiskSize1=%s\r\n", psdSetupInfo->pszDisk1Size)));
        }

        // Build the report and information filenames based on the dataset name.
        FilePathJoin(szFileName, psdSetupInfo->pszDataSetPath, psdSetupInfo->pszDataSet);
        bSucceeded = (bSucceeded & ((WriteText(hDirectiveFile, ".Set RptFileName=%s.RPT\r\n", szFileName)) &&
                                    (WriteText(hDirectiveFile, ".Set InfFileName=%s.INF\r\n\r\n", szFileName))));

        bSucceeded = ((WriteText(hDirectiveFile, ".Set InfHeader=\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfFooter=\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfDiskHeader=\"[disk list]\"\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfDiskLineFormat=\"Disk *disk#*, *label*\"\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfCabinetHeader=\"[cabinet list]\"\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfCabinetLineFormat=\"Disk *disk#*, Cabinet *cab#*, *cabfile*\"\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfFileHeader=\"[file list]\"\r\n")) &&
                      (WriteText(hDirectiveFile, ".Set InfFileLineFormat=\"*file#*: Cabinet *cab#*, *file*, *size*\"\r\n\r\n")));

        bSucceeded = (bSucceeded && (WriteText(hDirectiveFile, ".Set Cabinet=On\r\n")));
        // If no compression is selected, write the directive to disable compression.
        if (0 == lstrcmpi("none", psdSetupInfo->pszCompressType))
        {
            bSucceeded = (bSucceeded && (WriteText(hDirectiveFile, ".Set Compress=Off\r\n")));
        }
        else
        {
            bSucceeded = (bSucceeded && (WriteText(hDirectiveFile, ".Set CompressionType=%s\r\n", psdSetupInfo->pszCompressType)));
            bSucceeded = (bSucceeded && (WriteText(hDirectiveFile, ".Set Compress=On\r\n")));
        }

    }
    else
    {
        bSucceeded = FALSE;
    }
    // Only return success if all the headers were written.
    if (! bSucceeded)
        Report(RPT_APPEND_BOTH, "failed.");
    else
        Report(RPT_APPEND_BOTH, "done.");
    return(bSucceeded);

}

/*
    Adds a series of files to the directive file.
*/
static BOOL AddFilesToSetup(PSETUPFILEDATA psdSetupInfo, PCTSTR pszPath, PCTSTR pszFileSpec, BOOL bIncludeSubDirs)

{

    DIRPROCDATA dpdData;
    BOOL        bSucceeded;

    bSucceeded = TRUE;

    if (INVALID_HANDLE_VALUE != psdSetupInfo->hDirectiveFile)
    {
        Report(RPT_NEW_BOTH, "Adding %s%s to directive file...", pszPath, pszFileSpec);
        dpdData.hFile = psdSetupInfo->hDirectiveFile;
        // Recurse the source directory looking for files to include in the directive file.
        bSucceeded = ScanDir(pszPath, pszFileSpec, CreateDirectiveFileEntry, &dpdData, TRUE, bIncludeSubDirs);
        Report(RPT_APPEND_BOTH, "done.");
    }
    else
    {
        bSucceeded = FALSE;
    }

    return(bSucceeded);

}

/*
    Closes the directive file thereby ensuring that all entries are written to disk.
*/
static BOOL CloseSetupFile(PSETUPFILEDATA psdSetupInfo)

{

    return(CloseHandle(psdSetupInfo->hDirectiveFile));

}

/*
    Processes the Diamond Directive File (DDF) file to create the setup cabinets.

    Returns the result of the process, syntax checking of the entries will probably have to be done externally with manual runs of MAKECAB.

    This function reports the path to the directive file before the run and on failure to assist with this process.
*/
static BOOL ProcessDirectiveFile(PTSTR pszDirectiveFile, PCTSTR pszTargetPath)

{

    static  TCHAR   szConsoleTitle[] = "Making setup cabinet files, please wait...";

            TCHAR               szProcessCommandLine[MAX_PATH];
            TCHAR               szTargetDirectory[MAX_PATH];
            PROCESS_INFORMATION ProcInfo;
            STARTUPINFO         StartInfo;
            HWND                hwndConsoleWindow;
            RECT                rRect;
            DWORD               iWidth;
            DWORD               iHeight;
            POINT               ptCentre;
            DWORD               ChildExitCode;
            BOOL                bSucceeded;
            BOOL                bSearchForChild;

    bSucceeded = TRUE;

    // Run MAKECAB to process the directive file.
    //
    // Calculate the centre of the desktop window.
    GetWindowRect(GetDesktopWindow(), &rRect);
    iWidth      = (rRect.right - rRect.left);
    iHeight     = (rRect.bottom - rRect.top);
    ptCentre.x  = (iWidth / 2);
    ptCentre.y  = (iHeight / 2);

    /*  
        Populate the start information to create a new console window for MAKECAB which will be hidden until we
        have centred it on the screen.
        Fix the console dimensions at 80 columns and 25 rows.
    */
    StartInfo.cb            = sizeof(STARTUPINFO);
    StartInfo.lpReserved    = 0;
    StartInfo.lpDesktop     = NULL;
    StartInfo.lpTitle       = szConsoleTitle;
    StartInfo.dwX           = CW_USEDEFAULT;
    StartInfo.dwY           = CW_USEDEFAULT;
    StartInfo.dwXCountChars = 80;
    StartInfo.dwYCountChars = 25;
    StartInfo.dwFlags       = (STARTF_USECOUNTCHARS | STARTF_USEPOSITION | STARTF_USESHOWWINDOW);
    StartInfo.cbReserved2   = 0;
    StartInfo.lpReserved2   = NULL;
    StartInfo.wShowWindow   = SW_HIDE;

    // If a specific target path is specified, use it as the target path, otherwise use the path part of the directive file.
    if ('\0' != *pszTargetPath)
    {
        lstrcpy(szTargetDirectory, pszTargetPath);
        CreatePath(szTargetDirectory);
    }
    else
    {
        // Isolate the path to directive file and use it as the target directory.
        FilePathSplit(pszDirectiveFile, szTargetDirectory, NULL);
    }
    Report(RPT_NEW_BOTH, "BuildSetupFiles, using directive file %s", pszDirectiveFile);
    Report(RPT_NEW_BOTH, "Cabinets will be placed under %s", szTargetDirectory);
    Report(RPT_NEW_BOTH, "Making setup cabinet files, please wait...");

    // Construct the command line for to run MAKECAB and process our directive file.
    wsprintf(szProcessCommandLine,"MAKECAB.EXE /f %s /L %s", pszDirectiveFile, szTargetDirectory);

    // Create a new console process to contain the MAKECAB process including direction to the directive file and target directory.
    if (CreateProcess(NULL, szProcessCommandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, szTargetDirectory, &StartInfo, &ProcInfo))
    {
        /*
            Grab the handle to the new console window.

            We need to wait a while for the process to initialise before we can obtain the handle, so keep trying until we get it.
            However, while we are waiting for the process, the process may have finished, if there were not many files to process.
            Therefore we search for the child's window, but also check if the process has exited in which case we do not need to control
            it's window, so we just fall through.
        */
        bSearchForChild = TRUE;
        while (bSearchForChild)
        {
            // Try to find the console window for our child process.
            hwndConsoleWindow = FindWindow(NULL, szConsoleTitle);
            // If the window was not found, check that the child process is still running.
            if (NULL == hwndConsoleWindow)
            {
                // Is the child process still running?
                if ((GetExitCodeProcess(ProcInfo.hProcess, &ChildExitCode)) &&
                    (STILL_ACTIVE == ChildExitCode))
                {
                    // Yes...wait a bit longer.
                    Sleep(500);
                }
                else
                {
                    // Error occurred, the child has probably exited or failed to run.
                    bSearchForChild = FALSE;
                }
            }
            else
            {
                // Found the window, stop the search.
                bSearchForChild = FALSE;
            }
        }
        // We only need to manipulate the window if the window was found.
        if (NULL != hwndConsoleWindow)
        {
            // Get the current size of the console window for the child process.
            if (GetWindowRect(hwndConsoleWindow, &rRect))
            {
                // Calculate the co-ords to move the window in order to centre it on the screen.
                iWidth      = (rRect.right - rRect.left);
                iHeight     = (rRect.bottom - rRect.top);
                ptCentre.x -= (iWidth / 2);
                ptCentre.y -= (iHeight / 2);
                // Centre the new console on the screen and ask it to repaint itself.
                MoveWindow(hwndConsoleWindow, ptCentre.x, ptCentre.y, iWidth, iHeight, TRUE);
                // Now it is centred, show it on screen.
                ShowWindow(hwndConsoleWindow, SW_SHOW);
                // Return the console window to the foreground since the 'foreground' will have been retained
                // by us since we didn't show the window immediately.
                SetForegroundWindow(hwndConsoleWindow);
            }
        }

        // Wait the child process to complete.
        WaitForSingleObject(ProcInfo.hProcess, INFINITE);
        // Retrieve the child processes exit code so we can check whether if completed or not.
        GetExitCodeProcess(ProcInfo.hProcess, &ChildExitCode);

        // Cleanup after the child process.
        CloseHandle(ProcInfo.hProcess);
        CloseHandle(ProcInfo.hThread);
        // Update the Details log with the outcome of the MAKECAB run.
        if (0 == ChildExitCode)
        {
            Report(RPT_APPEND_BOTH, "done.");
        }
        else
        {
            // Show the caller that MAKECAB failed and provide it's exit code.
            Report(RPT_APPEND_BOTH, "failed.");
            Report(RPT_NEW_BOTH, "MAKECAB returned with error code %u.", ChildExitCode);
            Report(RPT_NEW_BOTH, "Check %s for errors and try '%s'.", pszDirectiveFile, szProcessCommandLine);
            bSucceeded = FALSE;
        }
    }
    else
    {
        Report(RPT_NEW_BOTH, "Failed to create setup cabinet files (%u)", GetLastError());
        bSucceeded = FALSE;
    }
    return(bSucceeded);

}

/*
    DataSetSize - returns the size of the dataset specified in the ReportFile
    referred to by /reportfile parameter.
    
    Returns:
    Top of NSIS stack   error code of the operation, 0 = success.
                        data set size is on NSIS stack if error code is 0.

	usage from NSIS is DataSetSize /reportfile=filename |
*/
extern void __declspec(dllexport) DataSetSize(HWND parent, int string_size, char *variables, stack_t **stacktop)

{

    TCHAR   szReportFileName[MAX_PATH];
    PTSTR   pszResultBuffer;
    PTSTR   pszParameterList;
    PTSTR   pszParamValue;
    DWORD   dwSetSize;
    LRESULT lResult;

    EXDLL_INIT();

    g_hwndParent = parent;
    // Locate the DetailLog dialog and controls.
    g_hwndDetailLog = FindWindowEx(g_hwndParent, NULL, "#32770", NULL);

    lResult = NO_ERROR;

    // Get the parameters for the plugin and a buffer to hold parameters as we parse them.
    // If either fails, we cannot do anything other than return an error.
    if ((GetNSISParameterList(&pszParameterList)) && (NULL != (pszParamValue = MALLOC(PTSTR, g_stringsize))))
    {
        if (GetNSISParameter(pszParameterList, "/reportfile", pszParamValue, TRUE))
        {
            lstrcpy(szReportFileName, pszParamValue);
        }
        // Get the size of the dataset from the reportfile.
        lResult = GetDataSetSize(szReportFileName, &dwSetSize);
        // Release the memory allocated to parse the parameters.
        FREE(pszParameterList);
        FREE(pszParamValue);
    }
    else
    {
        lResult = ERROR_OUTOFMEMORY;
    }

    /*
        Save the result/error onto the NSIS stack.
        This will either be NO_ERROR (0) or the result of the GetLastError from the last failure.
        If we don't have enough memory to pass back the result, return -1 instead.
    */
    pszResultBuffer = MALLOC(PTSTR, g_stringsize);
    if (NULL != pszResultBuffer)
    {
        // If the function succeeded, push the data set size onto the stack.
        if (NO_ERROR == lResult)
        {
            wsprintf(pszResultBuffer, "%lu", dwSetSize);
            pushstring(pszResultBuffer);
        }
        // Push the error code onto the stack.
        wsprintf(pszResultBuffer, "%ld", lResult);
        pushstring(pszResultBuffer);
        FREE(pszResultBuffer);
    }
    else
    {
        pushstring("-1");
    }

}

/*
    Function to scan through the specified directory searching for files that match the filespec.

    For each matching file, pCallback is called passing the pProcData pointer and the filename.

    If bScanSubDirs is TRUE, all subdirectories of the specified path are scanned for the same filespec.

    The method used uses minimal memory for each path traversal (both on the stack and on the heap) by
    adding paths to and removing them from, a single path string (pszBranch).
*/
static BOOL ScanDir(PCTSTR pszPath, PCTSTR pszFileSpec, DIRENTRYPROC pCallback, PDIRPROCDATA pProcData, BOOL bIsScanRoot, BOOL bScanSubDirs)

{

    static  PTSTR               pszRootPath;
    static  PTSTR               pszBranch;
    static  PTSTR               pszRootFileSpec;
            PTSTR               pszEndPath;
            PTSTR               pszSearchPath;
            PTSTR               pszSubBranch;
            PTSTR               pszRootPathRover;
            PWIN32_FIND_DATA    pffdFileInfo;
            HANDLE              hFindFile;
            BOOL                bSucceeded;
            BOOL                bMoreFiles;

    bSucceeded = TRUE;

    // If this is the initial call to the function, the root path, branch to scan and file specification to use.
    if (bIsScanRoot)
    {
        pszRootPath     = MALLOC(PTSTR, MAX_PATH);
        pszBranch       = MALLOC(PTSTR, MAX_PATH);
        pszRootFileSpec = MALLOC(PTSTR, MAX_PATH);
        if ((NULL == pszRootPath) || (NULL == pszBranch) || (NULL == pszRootFileSpec))
        {
            bSucceeded = FALSE;
            return(bSucceeded);
        }
        FilePathSplit(pszPath, pszRootPath, pszRootFileSpec);
        // Branch starts at the root of the path to scan.
        lstrcpy(pszBranch, pszRootPath);
        lstrcpy(pszRootFileSpec, pszFileSpec);
    }

    // Ensure that the branch ends with a '\'.
    if (pszBranch[lstrlen(pszBranch)-1] != '\\')
        lstrcat(pszBranch, "\\");
    // Locate the end of the path specification in the current branch.
    pszEndPath = my_strrchr(pszBranch, '\\')+1;

    pszSearchPath = MALLOC(PTSTR, MAX_PATH);
    pffdFileInfo = MALLOC(PWIN32_FIND_DATA, sizeof(WIN32_FIND_DATA));

    if ((NULL != pszSearchPath) && (NULL != pffdFileInfo))
    {
        // Scan for sub-directories first, but only if we are asked to.
        if (bScanSubDirs)
        {
            FilePathJoin(pszSearchPath, pszBranch, "*.*");
            hFindFile = FindFirstFile(pszSearchPath, pffdFileInfo);
            bMoreFiles = (INVALID_HANDLE_VALUE != hFindFile);
            while (bMoreFiles)
            {
                if (pffdFileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    // If we found a directory that is not one of the system ones ('.' and '..')
                    if ('.' != pffdFileInfo->cFileName[0])
                    {
                        // Add the subdirectory to the branch we are processing.
                        lstrcat(pszBranch, pffdFileInfo->cFileName);
                        lstrcat(pszBranch, "\\");
                        // Recurse down to the new subdirectory.
                        ScanDir(pszBranch, pszRootFileSpec, pCallback, pProcData, FALSE, bScanSubDirs);
                        // Recursion is done, strip off the subdirectory.
                        *pszEndPath = '\0';
                    }
                }
                bMoreFiles = FindNextFile(hFindFile, pffdFileInfo);
            }
            FindClose(hFindFile);
        }

        // pszSubBranch is the relative path of this directory from the RootPath.
        if (0 != lstrcmp(pszBranch, pszRootPath))
        {
            pszSubBranch = pszBranch;
            pszRootPathRover = pszRootPath;
            while (*pszSubBranch == *pszRootPathRover)
            {
                pszSubBranch++;
                pszRootPathRover++;
            }
        }
        else
            pszSubBranch = "";

        // Scan for files to process.
        FilePathJoin(pszSearchPath, pszBranch, pszRootFileSpec);
        // Write the start of a new file section in the directive file.
        WriteText(pProcData->hFile, "\r\n.Set SourceDir=\"%s\"\r\n", pszBranch);
        WriteText(pProcData->hFile, ".Set DestinationDir=\"%s\"\r\n", pszSubBranch);

        hFindFile = FindFirstFile(pszSearchPath, pffdFileInfo);
        bMoreFiles = (INVALID_HANDLE_VALUE != hFindFile);
        while (bMoreFiles)
        {
            if (! (pffdFileInfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // Only perform the callback if the user specified one.
                // Not sure why you would call ScanDir and not have it do anything, but the capability is there!
                if (NULL != pCallback)
                {
                    (*pCallback)(pffdFileInfo->cFileName, pProcData);
                }
            }
            bMoreFiles = FindNextFile(hFindFile, pffdFileInfo);
        }
        FindClose(hFindFile);

        FREE(pffdFileInfo);
        FREE(pszSearchPath);
    }
    else
        bSucceeded = FALSE;

    // Clean up the scan wide variables if we exiting the call that specified the RootPath and FileSpec.
    if (bIsScanRoot)
    {
        FREE(pszRootFileSpec);
        FREE(pszBranch);
        FREE(pszRootPath);
    }
    // Return with success status.
    return(bSucceeded);

}

/*
    Callback function for ScanDir to write the directive file entry for the specified file.
    Use by AddFilesToSetup function.
*/
static BOOL CreateDirectiveFileEntry(PCTSTR pszEntry, PVOID pData)

{

    return(WriteText(((PDIRPROCDATA) pData)->hFile, "\"%s\"\r\n", pszEntry));

}

/*
    Write a fprintf-like function, using dynamic memory for the target string, and Windows file functions.
*/
static BOOL WriteText(HANDLE hFile, PCTSTR pszFormat, ...)

{

    PTSTR   pszText;
    DWORD   nBytesWritten;
    DWORD   nTextLen;
    BOOL    bSucceeded;

    bSucceeded = FALSE;

    pszText = MALLOC(PTSTR, g_stringsize);
    if (NULL != pszText)
    {
        va_list vaArgs;
        va_start(vaArgs, pszFormat);
        wvsprintf(pszText, pszFormat, vaArgs);
        va_end(vaArgs);

        // Only need to write the text if the resulting string is not empty.
        if (0 != lstrlen(pszText))
        {
            nTextLen = lstrlen(pszText);
            bSucceeded = ((WriteFile(hFile, pszText, nTextLen, &nBytesWritten, NULL)) && (nBytesWritten == nTextLen));
        }
        FREE(pszText);
    }

    return(bSucceeded);

}

/*
    printf-like behaviour in the progress log window.
    Based on Tim Kosse's LogMessage function, but allows formatting strings to be used, and allows the update/appending
    of messages to existing line items, and can write separately to the DetailLog and DetailText fields.
*/
static void Report(UINT nFlags, PCTSTR pszFormat, ...)

{

    HWND    hwndDetailList;
    LVITEM  item = {0};
    UINT    nItemCount;
    PTSTR   pszNewItemText;
    PTSTR   pszItemText;

    if (NULL == g_hwndDetailLog)
        return;

    // Get the handle for the DetailLog ListView.
    hwndDetailList  = GetDlgItem(g_hwndDetailLog, IDC_DETAIL_LIST);

    // Return immediately if neither display flag is set, since there is nothing to display.
    if (! (nFlags & (RPT_LOG | RPT_TEXT)))
        return;

    pszNewItemText = MALLOC(PTSTR, g_stringsize);
    pszItemText = MALLOC(PTSTR, g_stringsize);
    if ((NULL != pszNewItemText) && (NULL != pszItemText))
    {
        va_list vaArgs;
        va_start(vaArgs, pszFormat);
        wvsprintf(pszNewItemText, pszFormat, vaArgs);
        va_end(vaArgs);

        if (0 != lstrlen(pszNewItemText))
        {
            // Display the text in the DetailLog control.
            if (nFlags & RPT_LOG)
            {
                nItemCount = (UINT) SendMessage(hwndDetailList, LVM_GETITEMCOUNT, 0, 0);

                // Always add a new item if the Detail Log is empty.
                if ((nFlags & RPT_NEW_ITEM) || (nItemCount < 1))
                {
                    // Add a new item (just like a normal DetailPrint command).
                    item.mask = LVIF_TEXT;
                    item.pszText = pszNewItemText;
                    item.cchTextMax = 0;
                    item.iItem = nItemCount;
                    ListView_InsertItem(hwndDetailList, &item);
                }
                else
                {
                    item.iItem = nItemCount - 1;
                    item.mask = LVIF_TEXT;
                    item.cchTextMax = (g_stringsize - 1);
                    item.pszText = pszItemText;
                    ListView_GetItem(hwndDetailList, &item);
                    lstrcpy(pszItemText, item.pszText);
                    /*
                        Watch for overflow here - two NSIS strings are being concatenated into a buffer
                        the size of one string.
                    */
                    lstrcat(pszItemText, pszNewItemText);
                    ListView_SetItem(hwndDetailList, &item);
                }
                ListView_EnsureVisible(hwndDetailList, item.iItem, 0);
            }
            
            // Display the text in the DetailText control.
            if (nFlags & RPT_TEXT)
            {
                // 'New' item - means replace the existing detail text.
                if (nFlags & RPT_NEW_ITEM)
                {
                    SetDlgItemText(g_hwndDetailLog, IDC_DETAIL_TEXT, pszNewItemText);
                }
                else
                {
                    // Append the new item to the existing Detail text.
                    GetDlgItemText(g_hwndDetailLog, IDC_DETAIL_TEXT, pszItemText, g_stringsize - 1);
                    /*
                        Watch for overflow here - two NSIS strings are being concatenated into a buffer
                        the size of one string.
                    */
                    lstrcat(pszItemText, pszNewItemText);
                    SetDlgItemText(g_hwndDetailLog, IDC_DETAIL_TEXT, pszItemText);
                }
            }
        }
    }
    // Free the memory allocated for the item text.
    if (NULL != pszItemText)
        FREE(pszItemText);
    if (NULL != pszNewItemText)
        FREE(pszNewItemText);

}

/*
    Formats a number according to the Windows locale, except that is does not use any decimal places.
    This allows us to show files sizes, with digit groupings for easier reading.
*/
static void FormatNumberString(UINT nNumber, PTSTR pszBuffer)

{

    TCHAR       szDecSep[32];
    TCHAR       szThouSep[32];
    TCHAR       LocaleInfBuffer[32];
    NUMBERFMT   nfFormatInfo;
    PTSTR       pszTempBuffer;

    pszTempBuffer = MALLOC(PTSTR, g_stringsize);
    if (NULL != pszTempBuffer)
    {
        wsprintf(pszTempBuffer, "%lu", nNumber);
        GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SDECIMAL, szDecSep, sizeof(szDecSep)-1);
        nfFormatInfo.lpDecimalSep = szDecSep;
        GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_STHOUSAND, szThouSep, sizeof(szThouSep)-1);
        nfFormatInfo.lpThousandSep = szThouSep;
        GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SGROUPING, LocaleInfBuffer, sizeof(LocaleInfBuffer));
        nfFormatInfo.Grouping = my_atoi(LocaleInfBuffer);
        GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ILZERO, LocaleInfBuffer, sizeof(LocaleInfBuffer));
        nfFormatInfo.LeadingZero = my_atoi(LocaleInfBuffer);
        GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_INEGNUMBER, LocaleInfBuffer, sizeof(LocaleInfBuffer));
        nfFormatInfo.NegativeOrder = my_atoi(LocaleInfBuffer);
        // Do not use decimal places when formatting numbers.
        nfFormatInfo.NumDigits = 0;
        GetNumberFormat(LOCALE_SYSTEM_DEFAULT, 0, pszTempBuffer, &nfFormatInfo, pszBuffer, g_stringsize);
        FREE(pszTempBuffer);
    }

}

/*
    Function to construct a string with a value formatted according to the Windows locale
    and with a unit size specifier (GB, MB, KB) appropriate to the value.
*/
static void FormatUnitString(DWORD dwValue, LPTSTR pszValueString)

{

    DWORD   dwUnit;
    TCHAR   cUnitChar;
    TCHAR   szValue[32];

    // Set the unit size and specifier character based on the size of dwValue.
    if (dwValue >= GIGABYTE)
    {
        dwUnit = GIGABYTE;
        cUnitChar = 'G';
    }
    else
    {
        if (dwValue >= MEGABYTE)
        {
            dwUnit = MEGABYTE;
            cUnitChar = 'M';
        }
        else
        {
            dwUnit = KILOBYTE;
            cUnitChar = 'K';
        }
    }

    // Setup a string o.t.f xxxx.yyXB where xxxx is the number of units and yy is the percentage of a unit remainder.
    FormatNumberString((dwValue / dwUnit), szValue);
    wsprintf(pszValueString, "%s.%02u%cB", szValue, (DWORD) ((UInt32x32To64(dwValue, 100UL) / dwUnit) % 100), cUnitChar);

}

/*
    Function to split a path and filename specification into a Path element and a FileSpec/Filename element.
*/
static void FilePathSplit(PCTSTR pszFullName, PTSTR pszPath, PTSTR pszFileName)

{

    PTSTR       pszEndPath;
    UINT_PTR    nPathLen;

    // Check that we have at least been given a filename to parse.
    if (NULL == pszFullName)
        return;

    // Locate the end of the path (if any) in the filename.
    pszEndPath = my_strrchr(pszFullName, '\\');
    // If no path was found...
    if (NULL == pszEndPath)
    {
        if (NULL != pszPath)
        {
            // Source path is blank.
            *(pszPath) = '\0';
        }
        if (NULL != pszFileName)
        {
            // Split off the filename part - filename is the FullName.
            lstrcpy(pszFileName, pszFullName);
        }
    }
    else
    {
        if (NULL != pszFileName)
        {
            // Copy the filename name from within the specified path-filename parameter.
            lstrcpy(pszFileName, pszEndPath+1);
        }
        if (NULL != pszPath)
        {
            // Split off the path - copy only up to the end of path character.
            nPathLen = ((UINT_PTR) pszEndPath - (UINT_PTR) pszFullName)+2;
            lstrcpyn(pszPath, pszFullName, (int) nPathLen);
        }
    }

}

/*
    Function to join a Path element and FileName/FileSpec element into a complete absolute filename.
*/
static void FilePathJoin(PTSTR pszFullName, PCTSTR pszPath, PCTSTR pszFileName)

{

    PTSTR   pszEndPath;

    // Validate the pointers to each component.
    if ((NULL == pszFullName) || (NULL == pszPath) || (NULL == pszFileName))
        return;

    // Initialise the full filename.
    *pszFullName = '\0';

    // Build filename starting with the path if it is present and not empty.
    if ((NULL != pszPath) && (0 != lstrlen(pszPath)))
    {
        lstrcat(pszFullName, pszPath);
        // Locate the end of the path (if any) in the new filename.
        pszEndPath = my_strrchr(pszFullName, '\\');
        // If no end path character was found or we have been passed a path that ends with directory
        // but does not have a trailing backslash, then add a trailing backslash to complete the path.
        if ((NULL == pszEndPath) ||
            (IsDirectory(pszPath) && (pszPath[lstrlen(pszPath)-1] != '\\')))
        {
            lstrcat(pszFullName,"\\");
        }
    }

    // Append the specified filename to constructed path.
    lstrcat(pszFullName, pszFileName);

}

/*
    Function to determine whether the directory specified by pszPath exists.

    Note: This function will return FALSE for a directory on a drive with removable media and no media inserted.
*/
static BOOL IsDirectory(PCTSTR pszPath)

{

    DWORD   dwFileAttributes;
    BOOL    bRetVal;

    bRetVal = FALSE;
    dwFileAttributes = GetFileAttributes(pszPath);
    if (INVALID_FILE_ATTRIBUTES != dwFileAttributes)
    {
        if (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            bRetVal = TRUE;
        }
    }
    return(bRetVal);

}

/*
    Function to create a path and all intervening paths if they do not already exist.

    Output messages from this function are only recorded to the DetailLog, to ensure
    that they don't interfere with the extraction rate display (if it is being
    displayed).
*/
static BOOL CreatePath(PCTSTR pszPath)

{

    PTSTR   pszPathToCreate;
    PTSTR   pszBranch;
    PTSTR   pszEndPath;
    PTSTR   pszEndRoot;
    BOOL    bSucceeded;

    // Assume will succeed in creating the path. This will also happen if the path already exists.
    bSucceeded = TRUE;

    pszPathToCreate = MALLOC(PTSTR, MAX_PATH);
    pszBranch = MALLOC(PTSTR, MAX_PATH);

    if ((NULL != pszPathToCreate) && (NULL != pszBranch))
    {
        // Make a local copy of the path, otherwise strtok will fail since it modifies parameter 1.
        lstrcpy(pszPathToCreate, pszPath);
        /*
            Find the root of the directory tree to create.
            This code works for drives and relative paths, will need different code to isolate paths with UNC paths.
        */
        pszEndRoot = my_strtok(pszPathToCreate, "\\");
        // Branch starts at the root of the path to create.
        lstrcpy(pszBranch, pszEndRoot);
        // pszEndPath points to next directory element in the path to create.
        pszEndPath = my_strtok(NULL, "\\");
        // While there are more directory elements to parse.
        while ((bSucceeded) && (NULL != pszEndPath))
        {
            // Append this element to the branch we are creating.
            lstrcat(pszBranch, "\\");
            lstrcat(pszBranch, pszEndPath);
            // If the directory referred to by the branch does not exist, attempt to create it.
            if (! IsDirectory(pszBranch))
            {
                Report(RPT_NEW_LOG, "Creating %s...", pszBranch);
                bSucceeded = CreateDirectory(pszBranch, NULL);
                if (bSucceeded)
                    Report(RPT_APPEND_LOG, "done.");
                else
                    Report(RPT_APPEND_LOG, "failed.");
            }
            // Move on to the next element in the path to create.
            pszEndPath = my_strtok(NULL, "\\");
        }
        FREE(pszBranch);
        FREE(pszPathToCreate);
    }
    else
    {
        // Out of memory - signal failure to create the path.
        bSucceeded = FALSE;
    }

    return(bSucceeded);

}

/*
    Function to strip all quotes out of a string.
*/
static void StripQuotes(PTSTR pszString)

{

    PTSTR   pszRover;
    PTSTR   pszNewStr;

    // Initialise both pointers to the source string
    pszNewStr = pszRover = pszString;
    // Strip all quote(s) from the string - brutal but effective!.
    // This is fine for filenames and directories since the quote character is not allowed in them.
    while ('\0' != *pszRover)
    {
        if ('\"' != *pszRover)
        {
            *(pszNewStr) = *(pszRover);
            pszNewStr++;
        }
        pszRover++;
    }
    *(pszNewStr) = '\0';

}

/*
    Custom built atoi function to avoid use of CRT functions.
*/
static INT my_atoi(PCTSTR pszSource)

{

    INT     nValue;
    TCHAR   cChar;

    nValue = 0;

    // Check for hexadecimal format specifiers.
    if (('0' == *pszSource) &&
        (('x' == *(pszSource+1)) || ('X' == *(pszSource+1))))
    {
        // Move source pointer past the hexadecimal specifiers.
        pszSource++;
        pszSource++;
        for (;;)
        {
            // Grab the next character from the string.
            cChar = *pszSource++;
            // If it is numerical, convert to correct numerical values.
            if ((cChar >= '0') && (cChar <= '9'))
                cChar -= '0';
            else
            {
                // Convert to uppercase to simplify parsing.                
                _toupper(cChar);
                // Convert A-F to correct hex values.
                if ((cChar >= 'A') && (cChar <= 'F'))
                {
                    // Subtract of -10 = +10
                    cChar -= ('A' - 10);
                }
                else
                {
                // invalid character default is to abort.
                    break;
                }
            }
            nValue <<= 4;
            nValue += cChar;
        }
    }
    else
    {
        // Check for octal format specifiers.
        if (('0' == *pszSource) &&
            ((*(pszSource+1) >= '0') && (*(pszSource+1) <= '7')))
        {
            // Move source pointer past the octal specifier.
            pszSource++;
            for (;;)
            {
                // Grab the next character from the string.
                cChar = *pszSource++;
                // If it is numerical and in range for an octal digit, convert to correct numerical values.
                if ((cChar >= '0') && (cChar <= '7'))
                    cChar -= '0';
                else
                {
                    // Invalid octal number character, abort.
                    break;
                }
                nValue <<= 3;
                nValue += cChar;
            }
        }
        else
        {
            // Decimal number format.
            for (;;)
            {
                // Grab the next character from the string.
                cChar = *pszSource++;
                // If it is numerical, convert to correct numerical values.
                if ((cChar >= '0') && (cChar <= '9'))
                    cChar -= '0';
                else
                {
                    // invalid character default is to abort.
                    break;
                }
                nValue *= 10;
                nValue += cChar;
            }
        }
    }

    return(nValue);

}

/*
    Custom built atol function to avoid use of CRT functions.
*/
static LONG my_atol(PCTSTR pszSource)

{

    LONG    nValue;
    TCHAR   cChar;

    nValue = 0L;

    // Check for hexadecimal format specifiers.
    if (('0' == *pszSource) &&
        (('x' == *(pszSource+1)) || ('X' == *(pszSource+1))))
    {
        // Move source pointer past the hexadecimal specifiers.
        pszSource++;
        pszSource++;
        for (;;)
        {
            // Grab the next character from the string.
            cChar = *pszSource++;
            // If it is numerical, convert to correct numerical values.
            if ((cChar >= '0') && (cChar <= '9'))
                cChar -= '0';
            else
            {
                // Convert to uppercase to simplify parsing.                
                _toupper(cChar);
                // Convert A-F to correct hex values.
                if ((cChar >= 'A') && (cChar <= 'F'))
                {
                    // Subtract of -10 = +10
                    cChar -= ('A' - 10);
                }
                else
                {
                // invalid character default is to abort.
                    break;
                }
            }
            nValue <<= 4;
            nValue += cChar;
        }
    }
    else
    {
        // Check for octal format specifiers.
        if (('0' == *pszSource) &&
            ((*(pszSource+1) >= '0') && (*(pszSource+1) <= '7')))
        {
            // Move source pointer past the octal specifier.
            pszSource++;
            for (;;)
            {
                // Grab the next character from the string.
                cChar = *pszSource++;
                // If it is numerical and in range for an octal digit, convert to correct numerical values.
                if ((cChar >= '0') && (cChar <= '7'))
                    cChar -= '0';
                else
                {
                    // Invalid octal number character, abort.
                    break;
                }
                nValue <<= 3;
                nValue += cChar;
            }
        }
        else
        {
            // Decimal number format.
            for (;;)
            {
                // Grab the next character from the string.
                cChar = *pszSource++;
                // If it is numerical, convert to correct numerical values.
                if ((cChar >= '0') && (cChar <= '9'))
                    cChar -= '0';
                else
                {
                    // invalid character default is to abort.
                    break;
                }
                nValue *= 10;
                nValue += cChar;
            }
        }
    }

    return(nValue);

}

static PTSTR my_strrchr(PCTSTR pszString, TCHAR cChar)

{

    PCTSTR  pszRover;

    // Scan for the end of the string.
    pszRover = pszString;
    while ('\0' != *pszRover)
        pszRover++;
    pszRover--;
    // Scan the string from the end to the start looking for a match.
    while ((pszRover != pszString) && (cChar != *pszRover))
        pszRover--;
    // If we ended up back at the start of the string, we didn't find the
    // character we were looking for.
    if (pszRover == pszString)
        pszRover = NULL;
    return((PTSTR) pszRover);

}

static PTSTR my_strtok(PTSTR pszString, PCTSTR pcszDelimiters)

{

    static  PTSTR   pszLastToken;
            PTSTR   pszTokenStart;
            PTSTR   pszTokenEnd;
            PCTSTR  pcszCurrDelim;
            TCHAR   cChar;
            TCHAR   cDelim;

    // If the call is to continue parsing tokens from the previous string
    if (NULL == pszString)
    {
        // Get a pointer to the last token parsed.        
        pszTokenEnd = pszLastToken;
        // If the last parsed token terminated the string, return NULL.
        if (NULL == pszTokenEnd)
        {
            return(NULL);
        }
        else
        {
            // Move the last token pointer onto the start of the next token.
            pszLastToken++;
        }
    }
    else
    {
        // Set the last token pointer to the start of the source string.
        pszLastToken = pszString;
    }

    // Create a pointer to the start of the next token.
    pszTokenStart = pszLastToken;
    // Skip any leading delimiter characters.
    while (1)
    {
        cChar = *pszTokenStart;
        for (pcszCurrDelim = pcszDelimiters; cDelim = *pcszCurrDelim; pcszCurrDelim++)
        {
            if (cChar == cDelim)
            {
                break;
            }
        }
        if ((cChar == cDelim) && (cChar != 0))
        {
            pszTokenStart++;
            continue;
        }
        else
            break;
    }
    /*
        If we reached the end of the string before reaching a non-delimeter
        character, then return 'token not found'.
    */
    if ('\0' == cChar)
    {
        pszLastToken = NULL;
        return(NULL);
    }

    // Initialise the start-of-token and end-of-token pointers.
    pszTokenEnd = pszTokenStart;
    // Scan through the string looking for any character that matches our delimiters.
    while (1)
    {
        // Grab the next character from the string.
        cChar = *pszTokenEnd;
        // Create a pointer to the beginning of the list delimiters.
        pcszCurrDelim = pcszDelimiters;
        do
        {
            // Get the next delimiter to search for (including the terminating nul).
            cDelim = *pcszCurrDelim;
            // If we got a match...
            if (cChar == cDelim)
            {
                // If we reached the end of both strings, return 'not found'.
                if (0 == cChar)
                {
                    pszLastToken = NULL;
                }
                else
                {
                    // Terminate the current token in the source string.
                    *(pszTokenEnd) = '\0';
                    // Save the new end of last token pointer.
                    pszLastToken = pszTokenEnd;
                }
                // Return the pointer to the new token to caller.
                return(pszTokenStart);
            }
            else
                // Move onto the next delimiter.
                pcszCurrDelim++;
        }
        // Continue searching until we reach the end of the delimiter list.
        while (0 != cDelim);
        // Move on to the next token string character.
        pszTokenEnd++;
    }

}

/*
    ReadFileLine - fgets replacement, reads a line of text from a file.
*/
static LRESULT ReadFileLine(HANDLE hFile, LPTSTR pszText, DWORD nMaxLen)

{

    LRESULT lResult;
    TCHAR   cChar;
    DWORD   nBytesRead;

    lResult = NO_ERROR;

    // Read at most nMaxLen characters.
    while (nMaxLen)
    {
        // Read a character from the file until the end of file is reached.
        if (! ReadFile(hFile, &cChar, 1, &nBytesRead, NULL))
        {
            lResult = GetLastError();
            break;
        }
        // If we reached then end of the file, return an appropriate error value.
        if (0 == nBytesRead)
        {
            lResult = ERROR_HANDLE_EOF;
            break;
        }
        // Append the character to the buffer.
        *pszText++ = cChar;
        // If we reach an end of line character, we are done.
        if ('\n' == cChar)
            break;
        // Otherwise, read the next character until we reach nMaxLen characters.
        nMaxLen--;
    }
    // Terminate the string buffer we are returning.
    *pszText = '\0';
    // Return with any error code relevant to the read operation.
    return(lResult);

}

/*
    GetDataSetSize - retrieves the size of the entire data set from the report file that is generated by MAKECAB.
*/
static LRESULT GetDataSetSize(PTSTR pszInfoFileName, LPDWORD lpdwSetSize)

{

            HANDLE  hReportFile;
            PTSTR   pszBuffer;
            PTSTR   pszLinePtr;
            PTSTR   pszNewLinePtr;
            DWORD   nSetSize;
            BOOL    bFoundStatusLine;
            BOOL    bIsBigDataSet;
            LRESULT lResult;
    static  PTSTR   pszSizeLineTag = "Bytes before:";

    // Initialise the return code.
    lResult = NO_ERROR;
    nSetSize = 0;
    // Initially, we have not found a data set > 2GB.
    bIsBigDataSet = FALSE;

    // Open the report file for reading, exclusive access is not required.
    hReportFile = CreateFile(pszInfoFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE != hReportFile)
    {
        pszBuffer = MALLOC(PTSTR, g_stringsize);
        if (NULL != pszBuffer)
        {
            bFoundStatusLine = FALSE;
            while (NO_ERROR == (lResult = ReadFileLine(hReportFile, pszBuffer, g_stringsize)))
            {
                /*
                    Search for the status line tag text.
                    pszNewLinePtr points to the line just read from the report file,
                    pszLinePtr points to the tag string we are searching for.
                */
                for (pszNewLinePtr = pszBuffer, pszLinePtr = pszSizeLineTag;
                    (('\0' != *pszNewLinePtr) && (*pszNewLinePtr == *pszLinePtr));
                    pszNewLinePtr++, pszLinePtr++)
                    ;
                // If we reached the end of the tag string, set the found flag.
                if ('\0' == *pszLinePtr)
                {
                    bFoundStatusLine = TRUE;
                    break;
                }
            }
            // Only continue if we found the status line.
            if (bFoundStatusLine)
            {
                /*
                    Parse the report file line.

                    The line is o.t.f
                        Bytes before: [whitespace][sign]n,nnn,nnn,nnn

                    We parse the line by extracting only the numerical value.
                    This allows us to ignore the leading text and also to remove the commas.
                */
                pszLinePtr = pszNewLinePtr;
                pszNewLinePtr = pszBuffer;
                while (('\r' != *pszLinePtr) && ('\n' != *pszLinePtr))
                {
                    /*
                        If we found a minus sign, that means that the data set is > 2GB.
                        Signal this so we can adjust the data set size read from the report file.
                    */
                    if ('-' == *pszLinePtr)
                    {
                        bIsBigDataSet = TRUE;
                    }
                    else
                    {
                        if ((*pszLinePtr >= '0') && (*pszLinePtr <= '9'))
                            *(pszNewLinePtr++) = *pszLinePtr;
                    }
                    pszLinePtr++;
                }
                *pszNewLinePtr = '\0';
                // Convert the string to a numeric value.
                nSetSize = my_atol(pszBuffer);
                /*
                    If the data set size in the report file is negative, this means that the real data set size is > 2GB.

                    Convert the negative size to the correct size by subtracting the size in the report file from 4GB and adding 1.
                */
                if (bIsBigDataSet)
                {
                    nSetSize = ((ULONG_MAX - nSetSize) + 1);
                }
                // Return the dataset size to the caller.
                *(lpdwSetSize) = nSetSize;

            }
            else
            {
                // Return a value that indicates we could not find the dataset size.
                lResult = -1;
            }
            FREE(pszBuffer);
        }
        else
        {
            lResult = ERROR_OUTOFMEMORY;
        }
        CloseHandle(hReportFile);
    }
    else
    {
        lResult = GetLastError();
    }
    return(lResult);

}
