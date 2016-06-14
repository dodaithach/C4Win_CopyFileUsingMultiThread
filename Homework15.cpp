// Homework15.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Homework15.h"
#include <commdlg.h>
#include <fstream>
#include <ShlObj.h>
#include <windowsx.h>
#include <wchar.h>
#include <vector>
using namespace std;

#define MAX_LOADSTRING	100
#define MAX_FILENAME	50
#define MAX_DIRECTORY	300
#define MAX_THREAD		5
#define MAX_EDITTEXT	3
#define MAX_BUFFER		10240
#define PREFIX			L"\\Copy_"

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// My variables
HWND		g_hDlg;
HWND		g_hWnd;
ifstream	g_is;
ofstream	g_os;
TCHAR		g_fileName[MAX_FILENAME];
TCHAR		g_srcFilePath[MAX_DIRECTORY];
TCHAR		g_destDirectory[MAX_DIRECTORY];
long		g_fileSize = -1;
int			g_threads = 0;
vector<int> g_threadData;
vector<int> g_threadIOPointer;
HANDLE		g_hThreads[MAX_THREAD];
HANDLE		g_mutex;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// My functions
INT_PTR CALLBACK	MainDlg(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI MainThread(LPVOID lpParam);
DWORD WINAPI ChildThread(LPVOID lpParam);
void Reset();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HOMEWORK15, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, MainDlg);

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOMEWORK15));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOMEWORK15));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HOMEWORK15);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, MainDlg);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for main dialog.
INT_PTR CALLBACK MainDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		g_hDlg = hDlg;
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(g_hWnd);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDC_BTN_COPY) {
			wchar_t editText[MAX_EDITTEXT];
			Edit_GetText(GetDlgItem(hDlg, IDC_EC_THREAD), editText, MAX_EDITTEXT);
			g_threads = wcstol(editText, NULL, 10);

			if (wcscmp(editText, L"") == 0 || g_threads > MAX_THREAD
				|| wcscmp(g_srcFilePath, L"") == 0 || wcscmp(g_destDirectory, L"") == 0) {
				MessageBox(hDlg, L"Error", NULL, MB_OK);
				break;
			}

			g_is.seekg(0, ios_base::end);
			g_fileSize = g_is.tellg();
			g_is.seekg(0);

			int div = g_fileSize / g_threads;
			int mod = g_fileSize % g_threads;
			for (int i = 0; i < g_threads; i++) {
				if (i == g_threads - 1)
					g_threadData.push_back(div + mod);
				else
					g_threadData.push_back(div);
			}

			for (int i = 0; i < g_threads; i++) {
				g_threadIOPointer.push_back(div * i);
			}

			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) MainThread, NULL, 0, 0);
		}

		if (LOWORD(wParam) == IDC_BTN_SRC) {
			OPENFILENAME ofn;
			TCHAR szFile[260];

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szFile;
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = NULL;
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = g_fileName;
			ofn.nMaxFileTitle = MAX_FILENAME;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofn)==TRUE) {
				g_is.open(ofn.lpstrFile, ifstream::binary);
				if (g_is.is_open()) {
					Static_SetText(GetDlgItem(hDlg, IDC_SRC), ofn.lpstrFile);
					wcscpy_s(g_srcFilePath, ofn.lpstrFile);
				}
				else
					MessageBox(hDlg, L"Path error", NULL, MB_OK);
			}		
		}

		if (LOWORD(wParam) == IDC_BTN_DEST) {
			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(bi));
			
			bi.lpszTitle = L"Browse folder";
			bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
			bi.lpfn = NULL;
			bi.lParam = NULL;

			LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

			if (pidl != NULL) {
				TCHAR path[MAX_DIRECTORY];
				SHGetPathFromIDList(pidl, path);
				wcscpy_s(g_destDirectory, path);
				wcscat_s(path, PREFIX);
				wcscat_s(path, g_fileName);

				g_os.open(path, ofstream::binary);
				if (g_os.good())
					Static_SetText(GetDlgItem(hDlg, IDC_DEST), path);
				else
					MessageBox(hDlg, L"Path error", NULL, MB_OK);
			}
		}

		break;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
	for (int i = 0; i < g_threads; i++) {
		int* a = new int;
		*a = i;
		g_hThreads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ChildThread, (LPVOID) a, 0, 0);
	}

	g_mutex = CreateMutex(NULL, FALSE, NULL);
	WaitForMultipleObjects(g_threads, g_hThreads, TRUE, INFINITE);

	// merge file
	for (int i = 0; i < g_threads; i++) {
		char prefix[5];
		sprintf_s(prefix, "%d", i);

		ifstream in;
		in.open(prefix, ios_base::binary);
		if (in.good()) {
			g_os.seekp(0, ios_base::end);
			g_os << in.rdbuf();
			in.close();
			remove(prefix);
		}
	}

	g_os.close();
	CloseHandle(g_mutex);

	MessageBox(g_hDlg, L"Complete", NULL, MB_OK);

	Reset();

	return 0;
}

DWORD WINAPI ChildThread(LPVOID lpParam) {
	int id = *(int*) lpParam;

	//DWORD wait = WaitForSingleObject(g_mutex, INFINITE);
	//if (wait != WAIT_OBJECT_0)
	//	return 0;

	int count = 0;

	int div = g_threadData[id] / MAX_BUFFER;
	int mod = g_threadData[id] % MAX_BUFFER;
	if (div == 0 && mod == 0)
		return 0;

	ifstream in;
	in.open(g_srcFilePath, ios_base::binary);
	if (!in.good())
		return 0;

	TCHAR prefix[5];
	wsprintf(prefix, L"%d", id);
	ofstream out;
	out.open(prefix, ios_base::binary);
	if (!out.good())
		return 0;

	in.seekg(g_threadIOPointer[id], ios_base::beg);
	out.seekp(0);
	wchar_t* updateView = new wchar_t[255];
	char buffer[MAX_BUFFER];
	for (int i = 0; i < div; i++) {
		in.read(buffer, MAX_BUFFER);
		out.write(buffer, MAX_BUFFER);
		// update view
		count += MAX_BUFFER;
		wsprintf(updateView, L"%d %%", count * 100 / g_threadData[id]);
		switch (id)
		{
		case 0:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD1), updateView);
			break;
		case 1:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD2), updateView);
			break;
		case 2:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD3), updateView);
			break;
		case 3:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD4), updateView);
			break;
		case 4:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD5), updateView);
			break;
		default:
			break;
		}
	}

	if (mod != 0) {
		in.read(buffer, mod);
		out.write(buffer, mod);

		// update view
		count += mod;
		wsprintf(updateView, L"%d %%", count * 100 / g_threadData[id]);
		switch (id)
		{
		case 0:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD1), updateView);
			break;
		case 1:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD2), updateView);
			break;
		case 2:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD3), updateView);
			break;
		case 3:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD4), updateView);
			break;
		case 4:
			Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD5), updateView);
			break;
		default:
			break;
		}
	}

	in.close();
	out.close();

	delete updateView;

	ReleaseMutex(g_mutex);
	return 0;
}

void Reset()  {
	g_fileName[0] = L'\0';
	g_srcFilePath[0] = L'\0';
	g_destDirectory[0] = L'\0';
	g_fileSize = 0;
	for (int i = 0; i < g_threads; i++)
		CloseHandle(g_hThreads[i]);
	g_threads = 0;

	Static_SetText(GetDlgItem(g_hDlg, IDC_SRC), L"");
	Static_SetText(GetDlgItem(g_hDlg, IDC_DEST), L"");
	Edit_SetText(GetDlgItem(g_hDlg, IDC_EC_THREAD), L"");

	Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD1), L"Not used");
	Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD2), L"Not used");
	Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD3), L"Not used");
	Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD4), L"Not used");
	Static_SetText(GetDlgItem(g_hDlg, IDC_THREAD5), L"Not used");
}