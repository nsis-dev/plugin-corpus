#include <windows.h>
#include <ddraw.h>
#include <tchar.h>
#include <shlwapi.h>

// This is a small program that just does a bunch of things Media Center *should* do to
// fascilitate running games. through Media Center.
// szCmdLine should be the executable you want to launch from Media Center.
// The current working directory will be the directory that this MCEWrapper.exe is located in.
// By Rick Yorgason, Longbow Digital Arts Inc.
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szCmdLine, int iCmdShow)
{
	enum { STATE_SETUP, STATE_TESTCOOPMODE, STATE_RUNGAME, STATE_DONE, STATE_ERROR } state = STATE_SETUP;

	HWND hwnd = NULL;
	LPDIRECTDRAW pDD = NULL;
	HANDLE hJob = NULL;
	HANDLE hCompletionPort = NULL;
	MSG msg;

	for(;;)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				state = STATE_DONE;	// This can pretty much only happen via the task manager.
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			switch(state)
			{
			case STATE_SETUP:
				{
					WNDCLASS wc;
					LPCTSTR classname = _T("Media Center Game Launcher");

					wc.style = CS_HREDRAW | CS_VREDRAW;
					wc.lpfnWndProc = ::DefWindowProc;
					wc.cbClsExtra = 0;
					wc.cbWndExtra = 0;
					wc.hInstance = hInstance;
					wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
					wc.hCursor = NULL;
					wc.hbrBackground = NULL;
					wc.lpszMenuName = NULL;
					wc.lpszClassName = classname;

					RegisterClass(&wc);

					if (!(hwnd = CreateWindowEx(0, classname, classname, WS_POPUP, 0, 0, 640, 480, NULL, NULL, GetModuleHandle(0), NULL)))
					{
						MessageBox(NULL, _T("Unable to create window"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					if(DD_OK != DirectDrawCreate(NULL, &pDD, NULL))
					{
						MessageBox(NULL, _T("Unable to initialize Direct Draw"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					state = STATE_TESTCOOPMODE;
				} break;

			case STATE_TESTCOOPMODE:
				// Try setting the exclusive mode.  It can take a second or so for Media Centre to relinquish exclusive
				// mode, and full-screen games usually need exclusive mode, so we don't want to start the game
				// until it's available.
				if(DDERR_EXCLUSIVEMODEALREADYSET != pDD->SetCooperativeLevel(hwnd, DDSCL_FULLSCREEN|DDSCL_EXCLUSIVE))
				{
					pDD->Release();	// Now that we have exclusive mode, give it up, so the game can use it.
					pDD = NULL;

					// Get the directory we're currently located in, so the game can use that as its CWD.
					TCHAR sDir[MAX_PATH];
					{
						TCHAR sTempDir[MAX_PATH];
						DWORD rc;
						if((rc = GetModuleFileName(NULL, sTempDir, MAX_PATH)) == 0)
						{
							MessageBox(NULL, _T("Could not find the path of this program"), _T("Error"), MB_OK|MB_ICONERROR);
							state = STATE_ERROR;
							continue;
						}
						else if(rc == MAX_PATH)
						{
							// The only solution to this is to call GetModuleFileName in a loop, and even then,
							// functions we use later (Microsoft's, not mine) rely on the path being no longer than MAX_PATH :(
							MessageBox(NULL, _T("The path to this program is too long, install your game somewhere else"), _T("Error"), MB_OK|MB_ICONERROR);
							state = STATE_ERROR;
							continue;
						}

						// We only want the directory part
						LPWSTR sFilePart;
						GetFullPathName(sTempDir, MAX_PATH, sDir, &sFilePart);
						*sFilePart = _T('\0');
					}
					

					// Create a job object; Job objects store not only the launched process, but all child
					// processes.  This allows us to launch a game that uses multiple processes and then wait
					// for all of them.
					// I haven't played with this much yet, so it could stand to use some testing.
					if(!(hJob = CreateJobObject(NULL, NULL)))
					{
						MessageBox(NULL, _T("Could not create game job"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					// Create a "completion port" to monitor when all our processes are complete
					if(!(hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)))
					{
						MessageBox(NULL, _T("Could not create completion port for game"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					// Associate that completion port with the job object
					JOBOBJECT_ASSOCIATE_COMPLETION_PORT jacp = {0};
					jacp.CompletionPort = hCompletionPort;
					if(!SetInformationJobObject(hJob, JobObjectAssociateCompletionPortInformation, &jacp, sizeof(jacp)))
					{
						MessageBox(NULL, _T("Could not associate completion port for game"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					STARTUPINFO si = {0};
					si.cb = sizeof(si);
					PROCESS_INFORMATION pi = {0};

					// Create our process now.  Note that we're creating it suspended so that it doesn't have a
					// chance to create children until we add it to our job.
					if(!CreateProcess(NULL, szCmdLine, NULL, NULL, 0, CREATE_SUSPENDED, NULL, sDir, &si, &pi))
					{
						MessageBox(NULL, _T("Could not create game process"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					// Add it to our job...
					if(!AssignProcessToJobObject(hJob, pi.hProcess))
					{
						DWORD dw = GetLastError();
						MessageBox(NULL, _T("Could not assign game process to job"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					// ...and resume our game process.
					if(ResumeThread(pi.hThread) == -1)
					{
						MessageBox(NULL, _T("Could not start game thread"), _T("Error"), MB_OK|MB_ICONERROR);
						state = STATE_ERROR;
						continue;
					}

					state = STATE_RUNGAME;
				}
				break;

			case STATE_RUNGAME:
				{
					// Wait five seconds for the game to exit.  We could wait for an infinite amount of time,
					// but we might as well stay responsive in case the user shuts down our process.
					DWORD bytes;
					ULONG_PTR key;
					LPOVERLAPPED overlapped = 0;
					if((GetQueuedCompletionStatus(hCompletionPort, &bytes, &key, &overlapped, 5000) || GetLastError() != WAIT_TIMEOUT)
						&& int(bytes) == JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)
						state = STATE_DONE;
				} break;

			case STATE_DONE:
			case STATE_ERROR:
				// Release, just in case.
				if(pDD != NULL)
					pDD->Release();

				// Kill our job
				if(hJob)
					CloseHandle(hJob);

				// When we exit, restore Media Center's focus.
				/*
				// This is one method to restore Media Center.  It has the benefit of being safer if you don't
				// know if your game was launched from Media Center.
				HWND hwndMCE = FindWindow(_T("eHome Render Window"), NULL);
				if(hwndMCE)
					ShowWindow(hwndMCE, SW_RESTORE);
				*/
				// The other method is to launch ehshell.exe.  Don't do this unless you know your game was launched from MCE.
				const TCHAR szMediaCenterPath[] = _T("%SystemRoot%\\ehome\\ehshell.exe");
				DWORD iSize = ExpandEnvironmentStrings(szMediaCenterPath, NULL, 0);
				TCHAR* szExpandedPath = new TCHAR[iSize+2];
				if(!ExpandEnvironmentStrings(szMediaCenterPath, szExpandedPath, iSize))
					state = STATE_ERROR;
				else if(!PathFileExists(szExpandedPath))
					state = STATE_ERROR;
				else if(HINSTANCE(32) <= ShellExecute(hwnd, _T("open"), szExpandedPath, NULL, NULL, SW_SHOWNORMAL))
					state = STATE_ERROR;
				delete szExpandedPath;

				// This will close down the game as well as this process, in case the user
				// tells us to close through the task manager.
				// I'm pretty sure this should happen anyways through CloseHandle() above,
				// but safe is usually better than sorry, or so they say.
				ExitProcess(state == STATE_DONE ? 0 : 1);
				break;
			}
		}
	} 
}