// MultilayerPerceptron-Win32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MultilayerPerceptron-Win32.h"

#include <stdio.h>

#include "perceptron.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
WCHAR szDebug[8192];

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


HWND hEdit;

wchar_t *char_to_wchar(char *buf)
{
	int wcsize = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	wchar_t *str = (wchar_t *)malloc(wcsize * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, str, wcsize);
	return str;
}

void char_to_wchar_alloc(char *buf, wchar_t *str)
{
	int wcsize = MultiByteToWideChar(CP_UTF8, 0, buf, -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, buf, -1, str, wcsize);
}

/*
void
DebugPrint(char *str)
{
	char_to_wchar_alloc(str, szDebug);
	SendMessage(hEdit, WM_SETTEXT, lstrlen(szDebug), (LPARAM)szDebug);
}
*/

void DebugPrint(char *str)
{
	char_to_wchar_alloc(str, szDebug);

	// get the current selection
	DWORD StartPos, EndPos;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&StartPos, (WPARAM)&EndPos);

	// move the caret to the end of the text
	int outLength = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, outLength, outLength);

	// insert the text at the new caret position
	SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)szDebug);

	// restore the previous selection
//	SendMessage(hEdit, EM_SETSEL, StartPos, EndPos);

	outLength = GetWindowTextLength(hEdit) + lstrlen(szDebug);
	SendMessage(hEdit, EM_SETSEL, outLength, outLength);

	SendMessage(hEdit, WM_VSCROLL, SB_BOTTOM, NULL);

}


void
run_nn(void)
{
	perceptron_t *input1, *input2;
	perceptron_t *hidden1, *hidden2, *hidden3, *hidden4;
	perceptron_t *output1;
	perceptron_t *bias;
	mlp_layer_t *layer1, *layer2, *layer3;

	input1 = perceptron_new(PERCEPTRON_RECEPTOR);
	input2 = perceptron_new(PERCEPTRON_RECEPTOR);

	bias = perceptron_new(PERCEPTRON_BIAS);

	hidden1 = perceptron_new(PERCEPTRON_NEURON);
	hidden2 = perceptron_new(PERCEPTRON_NEURON);
	//hidden3 = perceptron_new(PERCEPTRON_NEURON);
	//hidden4 = perceptron_new(PERCEPTRON_NEURON);

	output1 = perceptron_new(PERCEPTRON_OUTPUT);

	//output2 = perceptron_new();

	layer1 = mlp_layer_new();
	layer2 = mlp_layer_new();
	layer3 = mlp_layer_new();

	//perceptron_link(bias, input1);
	//perceptron_link(bias, input2);

	perceptron_link(input1, hidden1);
	perceptron_link(input1, hidden2);
	//perceptron_link(input1, hidden3);
	//perceptron_link(input1, hidden4);

	perceptron_link(input2, hidden1);
	perceptron_link(input2, hidden2);
	//perceptron_link(input2, hidden3);
	//perceptron_link(input2, hidden4);

	perceptron_link(bias, hidden1);
	perceptron_link(bias, hidden2);
	//perceptron_link(bias, hidden3);
	//perceptron_link(bias, hidden4);


	mlp_layer_add(layer1, input1);
	mlp_layer_add(layer1, input2);
	mlp_layer_add(layer1, bias); // it is important to add bias neuron to layer 1

	perceptron_link(hidden1, output1);
	//perceptron_link(hidden1, output2);

	perceptron_link(hidden2, output1);
	//perceptron_link(hidden2, output2);

	//perceptron_link(hidden3, output1);
	//perceptron_link(hidden3, output2);

	//perceptron_link(hidden4, output1);

	perceptron_link(bias, output1);

	mlp_layer_add(layer2, hidden1);
	mlp_layer_add(layer2, hidden2);
	//mlp_layer_add(layer2, hidden3);
	//mlp_layer_add(layer2, hidden4);

	mlp_layer_add(layer3, output1);
	int right = 0;
	int i;
	for (i = 0; i < 100000; i++) {
		real high = 1.0;
		real low = 0.0;
		// execute neural network
		real xs = (rand() % 2 == 0) ? high : low;
		real ys = (rand() % 2 == 0) ? high : low;

		//real xs = (rand() % 2 == 0) ? 1 : -1;
		//real ys = (rand() % 2 == 0) ? 1 : -1;

		//real xx = (((real)rand() / (real)RAND_MAX) + (real)(rand() % 2)) * (real)xs;
		//real yy = (((real)rand() / (real)RAND_MAX) + (real)(rand() % 2)) * (real)ys;
		real xx = (real)xs;
		real yy = (real)ys;


		real x = xx;
		real y = yy;

		input1->input[0] = x;
		input2->input[0] = y;

		mlp_layer_compute(layer1);
		mlp_layer_transfer(layer1);
		mlp_layer_compute(layer2);
		mlp_layer_transfer(layer2);
		mlp_layer_compute(layer3);

		real output = output1->output;

		real answer = low;
		if ((x > low || y > low) && !(x > low && y > low)) // xor works
		//if ((x > 0 || y > 0) && !(x > 0 && y > 0))
		//if (x > low || y > low) // or (works)
			answer = high;

		real error, totalerror;
		error = answer - output;
		if (fabs(error) < 0.5)
			right++;
		totalerror = 100.0 - (100.0 / (real)i * (real)right);
		char buf[BUFSIZ];
		sprintf_s(buf, BUFSIZ, "%d [%f, %f] = net output %f (got %d) (answer %d %s) (error %f) (total error %f%%)\n",
			i, x, y, output, (output > 0.5) ? 1 : 0, (answer == high) ? 1 : 0, (fabs(error) < 0.5) ? "right" : "wrong",
			fabs(error), totalerror);
		//DebugPrint(buf);
		wchar_t *str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);

		perceptron_train(output1, answer);
	}

	char buf[BUFSIZ];
	sprintf_s(buf, BUFSIZ, "\r\n\r\nNetwork Weights\r\n---------------\r\n hidden1 [ %f  %f  (%f) ]\r\n hidden2 [ %f  %f  (%f) ]\r\n output [ %f  %f  (%f) ]\r\n\r\nTotal: %d (%d correct)\r\n",
		hidden1->weights[0], hidden1->weights[1], hidden1->weights[2],
		hidden2->weights[0], hidden2->weights[1], hidden2->weights[2],
//		hidden3->weights[0], hidden3->weights[1], hidden3->weights[2],
		output1->weights[0], output1->weights[1], output1->weights[2], //, output1->weights[3]
		i, right);
	DebugPrint(buf);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.


    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MULTILAYERPERCEPTRONWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTILAYERPERCEPTRONWIN32));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTILAYERPERCEPTRONWIN32));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MULTILAYERPERCEPTRONWIN32);
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
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   char *str = "EDIT";
   wchar_t *wc = char_to_wchar(str);

   hEdit = CreateWindowEx(WS_EX_CLIENTEDGE,
	   wc,
	   NULL,
	   WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
	   0,
	   0,
	   640,
	   480,
	   hWnd,
	   (HMENU)IDC_MAIN_MENU,
	   GetModuleHandle(NULL),
	   NULL);
   free(wc);

  // str = "Hello World";
  // wc = char_to_wchar(str);

  // SendMessage(hEdit, WM_SETTEXT, lstrlen(wc), (LPARAM)wc);

   run_nn();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
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
