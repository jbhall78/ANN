// single layer perceptron as a linear classifier


// ANN-win32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Perceptron-Win32.h"

#include <stdlib.h>

#include "perceptron.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int wWidth = 640;
int wHeight = 480;

HWND ghWnd;

int formula = 1; // 2x
//int formula = 2; // 45 degree angle
//int formula = 3; // left right
//int formula = 4; // up down

int view = 1; // results
//int view = 2; // training data

Perceptron<2> *perceptron;
typedef struct {
	float point[2];
	int answer;
//	float sigmoid;
} answer_t;

answer_t *training = NULL;
answer_t *answers = NULL;
int n_answers = 1000;
int n_training = 5000;
int answers_to_draw = 0;

COLORREF black;
COLORREF blue;
COLORREF green;
COLORREF red;
COLORREF white;
HPEN bluePen;
HPEN greenPen;
HPEN redPen;
HBRUSH greenBrush;
HBRUSH redBrush;
HBRUSH blackBrush;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    EditWeights(HWND, UINT, WPARAM, LPARAM);

void TrainPerceptron(void)
{
	if (perceptron != NULL)
		delete perceptron;

	perceptron = new Perceptron<2>(0.1);

	int total = n_training;
	if (training != NULL)
		free(training);

	training = (answer_t *)malloc(total * sizeof(answer_t));
	for (int i = 0; i < total; i++) {
		float point[2];
		// generate a random point and center it on our grid
		//point[0] = (float)(rand() % 640) / (float)640.0;
		//point[1] = (float)(rand() % 480) / (float)480.0;
		point[0] = (float)(rand() % wWidth) - (float)(wWidth / 2);
		point[1] = (float)(rand() % wHeight) - (float)(wHeight / 2);

		int answer = 1;

		// determine answer based on the selected formula

		// train using a variety of linear shapes
		switch (formula) {
		case 1:
			if (point[1] < 2 * point[0]) // steep angle
				answer = -1;
			break;
		case 2:
			if (point[1] < point[0]) // 45 degree angle
				answer = -1;
			break;
		case 3:
			if (point[0] < 0.0) // left/right 
				answer = -1;
			break;
		case 4:
			if (point[1] < 0.0) // up/down
				answer = -1;
			break;
		case 5: // you can't train a perceptron with a checkerboard pattern
			if ((point[0] >= 0 && point[1] >= 0) || (point[0] < 0 && point[1] < 0))
				answer = -1;
			break;
		}

		// train the network with the random point and answer
		int guess = perceptron->learn(point, answer);
		training[i].point[0] = point[0] + (float)(wWidth / 2);
		training[i].point[1] = point[1] + (float)(wHeight / 2);
		training[i].answer = answer;
	}
	/*
	ann->debug = 1;
	float point[2];
	while (1) {
	point[0] = (float)(rand() % 640) - (float)320.0;
	point[1] = (float)(rand() % 480) - (float)240.0;

	printf("guess: %d (%f, %f)\n", ann->output(point), point[0] + 320.0, point[1] + 240.0);
	Sleep(5000);
	}*/
}

void
GetAnswers(void)
{
	if (answers != NULL)
		free(answers);

	answers = (answer_t *)malloc(n_answers * sizeof(answer_t));
	for (int i = 0; i < n_answers; i++) {
		answers[i].point[0] = (float)(rand() % wWidth) - (float)(wWidth / 2);
		answers[i].point[1] = (float)(rand() % wHeight) - (float)(wHeight / 2);

		answers[i].answer = perceptron->output(answers[i].point);
		//answers[i].sigmoid = perceptron->sigmoid(answers[i].point);

		answers[i].point[0] += (float)(wWidth / 2);
		answers[i].point[1] += (float)(wHeight / 2);
	}

}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize the perceptron
	TrainPerceptron();
	GetAnswers();

	// initialize our colors, pens and brushes
	black = RGB(0, 0, 0);
	blue = RGB(0, 0, 255);
	green = RGB(0, 255, 0);
	red = RGB(255, 0, 0);
	white = RGB(255, 255, 255);

	bluePen = CreatePen(PS_SOLID, 1, blue);
	greenPen = CreatePen(PS_SOLID, 1, green);
	redPen = CreatePen(PS_SOLID, 1, red);

	blackBrush = CreateSolidBrush(black);
	greenBrush = CreateSolidBrush(green);
	redBrush = CreateSolidBrush(red);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ANNWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ANNWIN32));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
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
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ANNWIN32));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ANNWIN32);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 656, 539, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // set a timer on this window
   SetTimer(hWnd, 1, 100, NULL);

   ghWnd = hWnd;

   return TRUE;
}
/*
HRESULT __fastcall AnsiToUnicode(LPCSTR pszA, LPWSTR* ppszW)
{

	ULONG cCharacters;
	DWORD dwError;

	// If input is null then just return the same.
	if (NULL == pszA)
	{
		*ppszW = NULL;
		return NOERROR;
	}

	// Determine number of wide characters to be allocated for the
	// Unicode string.
	cCharacters = strlen(pszA) + 1;

	// Use of the OLE allocator is required if the resultant Unicode
	// string will be passed to another COM component and if that
	// component will free it. Otherwise you can use your own allocator.
	*ppszW = (LPWSTR)malloc(cCharacters * 2);
	if (NULL == *ppszW)
		return E_OUTOFMEMORY;

	// Covert to Unicode.
	if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,
		*ppszW, cCharacters))
	{
		dwError = GetLastError();
		free(*ppszW);
		*ppszW = NULL;
		return HRESULT_FROM_WIN32(dwError);
	}

	return NOERROR;
}
*/

wchar_t *char_to_wchar(char *buf)
{
	int wcsize = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	wchar_t *str = (wchar_t *)malloc(wcsize * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, str, wcsize);
	return str;
}
/* untested
char *wchar_to_char(wchar_t *buf)
{
	int len = lstrlen(buf);
	char *str = (char *)malloc(len + 1);
	memset(str, 0, len + 1);
	wcstombs(str, buf, len);
	return str;
}
*/

void
DrawPoint(HDC hdc, int x, int y)
{
	MoveToEx(hdc, x, y, NULL);
	LineTo(hdc, x + 1, y);
}

void
Paint(HWND hWnd, HDC hdc)
{
	HDC hdcMem = CreateCompatibleDC(hdc);
	RECT rc;
	HBITMAP hbmMem;

	GetClientRect(hWnd, &rc);

	hbmMem = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
	SelectObject(hdcMem, hbmMem);

	//  draw background
	SelectObject(hdcMem, blackBrush);
	Rectangle(hdcMem, 0, wHeight, wWidth, 0);

	int size = 3;
	/*
	#if 0
	answer_t *a = training;
	int n = n_training;
	#else
	answer_t *a = answers;
	//int n = n_answers;
	int n = answers_to_draw;
	#endif
	*/

#if 1
	answer_t *a = NULL;
	int n;

	// select which dataset to draw
	if (view == 1) {
		a = answers;
		//n = n_answers;
		n = answers_to_draw;
	}
	else if (view == 2) {
		a = training;
		n = n_training;
	}

	// draw points
	for (int i = 0; i < n; i++) {
		if (a[i].answer == -1) {
			SelectObject(hdcMem, bluePen);
			SelectObject(hdcMem, redBrush);
		}
		else {
			SelectObject(hdcMem, bluePen);
			SelectObject(hdcMem, greenBrush);
		}
		Ellipse(hdcMem,
			a[i].point[0] - size, //left
			wHeight - a[i].point[1] + size, //top
			a[i].point[0] + size, //right
			wHeight - a[i].point[1] - size); //bottom			
	}
#endif

	//MoveToEx(hdc, 0, 0, NULL);
	//LineTo(hdc, 640, 480);
#if 1
	// draw weights
	char buf[BUFSIZ];
	sprintf_s(buf, BUFSIZ, "Weights: [ w1: %f, w2: %f, Bias: %f ]  Samples: %d", perceptron->weights[0], perceptron->weights[1], perceptron->weights[2], n_training);
	wchar_t *str = char_to_wchar(buf);
	
	//const TCHAR *str = TEXT(buf);
	SIZE tsize;
	GetTextExtentPoint32(hdcMem, str, lstrlen(str), &tsize);
	SetBkColor(hdcMem, black);
	SetTextColor(hdcMem, white);
	TextOut(hdcMem, 0, wHeight - tsize.cy, str, lstrlen(str));
	free(str);

	// draw origin
	sprintf_s(buf, BUFSIZ, "0,0");
	str = char_to_wchar(buf);
	GetTextExtentPoint32(hdcMem, str, lstrlen(str), &tsize);
	SetBkMode(hdcMem, TRANSPARENT);
	TextOut(hdcMem, wWidth / 2 - tsize.cx, wHeight - (wHeight / 2), str, lstrlen(str));
	free(str);

	// draw X
	sprintf_s(buf, BUFSIZ, "X");
	str = char_to_wchar(buf);
	SetBkMode(hdcMem, TRANSPARENT);
	TextOut(hdcMem, 0, wHeight - (wHeight / 2), str, lstrlen(str));
	free(str);

	// draw Y
	sprintf_s(buf, BUFSIZ, "Y");
	str = char_to_wchar(buf);
	GetTextExtentPoint32(hdcMem, str, lstrlen(str), &tsize);
	SetBkMode(hdcMem, TRANSPARENT);
	TextOut(hdcMem, wWidth / 2 - tsize.cx, 0, str, lstrlen(str));
	free(str);

#endif
	// draw axis
#if 1
	SelectObject(hdcMem, bluePen);
	MoveToEx(hdcMem, wWidth / 2, 0, NULL);
	LineTo(hdcMem, wWidth / 2, wHeight);

	MoveToEx(hdcMem, 0, wHeight / 2, NULL);
	LineTo(hdcMem, wWidth, wHeight / 2);

	int width = 6;
	for (int i = 0; i < wWidth; i += 10) {
		MoveToEx(hdcMem, i, wHeight / 2 - width / 2, NULL);
		LineTo(hdcMem, i, wHeight / 2 + width / 2);
	}
	for (int i = 0; i < wHeight; i += 10) {
		MoveToEx(hdcMem, wWidth / 2 - width / 2, i, NULL);
		LineTo(hdcMem, wWidth / 2 + width / 2, i);
	}
#endif 

#if 1
	SelectObject(hdcMem, redPen);
	switch (formula) {
	case 1: // 2x
		for (int x = -(wWidth / 2); x < (wWidth / 2); x++) {
			/*char buf[BUFSIZ];
			sprintf_s(buf, BUFSIZ, "%d\n", 320 + x);
			wchar_t *str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);*/
			DrawPoint(hdcMem, (wWidth / 2) + x, wHeight - (wHeight / 2) - (2 * x));
		}
		break;
	case 2: // x
		for (int x = -(wWidth / 2); x < (wWidth / 2); x++) {
			DrawPoint(hdcMem, (wWidth / 2) + x, wHeight - (wHeight / 2) - x);
		}
		break;
	case 3: // x > 0
		for (int x = -(wWidth / 2); x < (wWidth / 2); x++) {
			DrawPoint(hdcMem, (wWidth / 2) + 0, wHeight - (wHeight / 2) - x);
		}
		break;
	case 4: // y > 0
		for (int x = -(wWidth / 2); x < (wWidth / 2); x++) {
			DrawPoint(hdcMem, (wWidth / 2) + x, wHeight - (wHeight / 2) - 0);
		}
		break;
	}
#endif

#if 1
	SelectObject(hdcMem, greenPen);

		for (int x = -(wWidth / 2); x < (wWidth / 2); x++) {
			/*char buf[BUFSIZ];
			sprintf_s(buf, BUFSIZ, "%d\n", 320 + x);
			wchar_t *str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);*/
			/* solve for y: (bias - w1 * x) / w2 */
			int y = (int)((perceptron->weights[2] - perceptron->weights[0] * (float)x) / perceptron->weights[1]);
			//DrawPoint(hdcMem, (wWidth / 2) + x, wHeight - (wHeight / 2) - (int)((perceptron->weights[2] - perceptron->weights[0] * (float)x) / perceptron->weights[1]));
			DrawPoint(hdcMem, (wWidth / 2) + x, wHeight - (wHeight / 2) - y);
		}

#endif



	BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
	DeleteObject(hbmMem);
	DeleteObject(hdcMem);

}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			HMENU hmenu = GetMenu(hWnd);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
			case ID_EDIT_WEIGHTS:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_EDITWEIGHTS), hWnd, EditWeights);
				break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_VIEW_2X_PLUS_1:
				CheckMenuItem(hmenu, ID_VIEW_2X_PLUS_1, MF_CHECKED);
				CheckMenuItem(hmenu, ID_VIEW_45_DEGREE_ANGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_LEFT_RIGHT, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_UP_DOWN, MF_UNCHECKED);
				formula = 1;
				TrainPerceptron();
				GetAnswers();
				answers_to_draw = 0;
				break;
			case ID_VIEW_45_DEGREE_ANGLE:
				CheckMenuItem(hmenu, ID_VIEW_2X_PLUS_1, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_45_DEGREE_ANGLE, MF_CHECKED);
				CheckMenuItem(hmenu, ID_VIEW_LEFT_RIGHT, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_UP_DOWN, MF_UNCHECKED);
				formula = 2;
				TrainPerceptron();
				GetAnswers();
				answers_to_draw = 0;
				break;
			case ID_VIEW_LEFT_RIGHT:
				CheckMenuItem(hmenu, ID_VIEW_2X_PLUS_1, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_45_DEGREE_ANGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_LEFT_RIGHT, MF_CHECKED);
				CheckMenuItem(hmenu, ID_VIEW_UP_DOWN, MF_UNCHECKED);
				formula = 3;
				TrainPerceptron();
				GetAnswers();
				answers_to_draw = 0;
				break;
			case ID_VIEW_UP_DOWN:
				CheckMenuItem(hmenu, ID_VIEW_2X_PLUS_1, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_45_DEGREE_ANGLE, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_LEFT_RIGHT, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_UP_DOWN, MF_CHECKED);
				formula = 4;
				TrainPerceptron();
				GetAnswers();
				answers_to_draw = 0;
			{
				/*
				wchar_t *msg, *title;
				AnsiToUnicode("Callback function works", &msg);
				AnsiToUnicode("Window title", &title);
				MessageBox(hWnd, msg, title, MB_OK);
			*/
			}
				break;
			case ID_VIEW_RESULTS:
				CheckMenuItem(hmenu, ID_VIEW_RESULTS, MF_CHECKED);
				CheckMenuItem(hmenu, ID_VIEW_TRAINING_DATA, MF_UNCHECKED);
				view = 1;
				answers_to_draw = 0;
				break;
			case ID_VIEW_TRAINING_DATA:
				CheckMenuItem(hmenu, ID_VIEW_RESULTS, MF_UNCHECKED);
				CheckMenuItem(hmenu, ID_VIEW_TRAINING_DATA, MF_CHECKED);
				view = 2;
				answers_to_draw = 0;
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_ERASEBKGND: // handle this event for flicker free display
		return 1;
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			Paint(hWnd, hdc);
			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_SIZE:
		wWidth = LOWORD(lParam);
		wHeight = HIWORD(lParam);
		if (wWidth != 0 && wHeight != 0) {
			TrainPerceptron();
			GetAnswers();
			answers_to_draw = 0;
		}
		break;
	case WM_TIMER:
		if (wParam == 1) {
			answers_to_draw++;
			if (answers_to_draw > n_answers)
				answers_to_draw = 0;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK EditWeights(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		char buf[BUFSIZ];
		sprintf_s(buf, BUFSIZ, "%f", perceptron->weights[0]);
		wchar_t *str = char_to_wchar(buf);
		SetDlgItemText(hDlg, IDC_EDITWEIGHTS1, str);
		free(str);

		sprintf_s(buf, BUFSIZ, "%f", perceptron->weights[1]);
		str = char_to_wchar(buf);
		SetDlgItemText(hDlg, IDC_EDITWEIGHTS2, str);
		free(str);

		sprintf_s(buf, BUFSIZ, "%f", perceptron->weights[2]);
		str = char_to_wchar(buf);
		SetDlgItemText(hDlg, IDC_EDITWEIGHTS3, str);
		free(str);

		
		//auto hEdit1 = GetDlgItem(hDlg, IDC_EDITWEIGHTS1);
		//auto hEdit2 = GetDlgItem(hDlg, IDC_EDITWEIGHTS2);
		//SendMessage(hEdit1, WM_GETTEXT, (WPARAM)BUFSIZ, (LPARAM)buf);
	}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			if (LOWORD(wParam) == IDOK) {
				int recalculate = 0;
				wchar_t buf[BUFSIZ];
				GetDlgItemText(hDlg, IDC_EDITWEIGHTS1, buf, BUFSIZ);
				float w1 = wcstof(buf, NULL);
				if (perceptron->weights[0] != w1) { // normally i wouldnt do this but it should be the same value since we just did a string conversion
					perceptron->weights[0] = w1;
					recalculate = 1;
				}
				GetDlgItemText(hDlg, IDC_EDITWEIGHTS2, buf, BUFSIZ);
				float w2 = wcstof(buf, NULL);
				if (perceptron->weights[1] != w2) {
					perceptron->weights[1] = w2;
					recalculate = 1;
				}
				GetDlgItemText(hDlg, IDC_EDITWEIGHTS3, buf, BUFSIZ);
				float bias = wcstof(buf, NULL);
				if (perceptron->weights[2] != bias) {
					perceptron->weights[2] = bias;
					recalculate = 1;
				}
				if (recalculate) {
					GetAnswers();
					answers_to_draw = 0;
					HMENU hmenu = GetMenu(ghWnd);
					CheckMenuItem(hmenu, ID_VIEW_2X_PLUS_1, MF_UNCHECKED);
					CheckMenuItem(hmenu, ID_VIEW_45_DEGREE_ANGLE, MF_UNCHECKED);
					CheckMenuItem(hmenu, ID_VIEW_LEFT_RIGHT, MF_UNCHECKED);
					CheckMenuItem(hmenu, ID_VIEW_UP_DOWN, MF_UNCHECKED);
				}
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_BUTTON_DEFAULTS) {
			char buf[BUFSIZ];
			sprintf_s(buf, BUFSIZ, "%f", perceptron->weights[0]);
			wchar_t *str = char_to_wchar(buf);
			SetDlgItemText(hDlg, IDC_EDITWEIGHTS1, str);
			free(str);

			sprintf_s(buf, BUFSIZ, "%f", perceptron->weights[1]);
			str = char_to_wchar(buf);
			SetDlgItemText(hDlg, IDC_EDITWEIGHTS2, str);
			free(str);

			sprintf_s(buf, BUFSIZ, "%f", perceptron->weights[2]);
			str = char_to_wchar(buf);
			SetDlgItemText(hDlg, IDC_EDITWEIGHTS3, str);
			free(str);
		}
		break;
	}
	return (INT_PTR)FALSE;
}


