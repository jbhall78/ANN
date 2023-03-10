// MultilayerMultiOutputPerceptron.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MultilayerMultiOutputPerceptron.h"

#include <stdio.h>
#include <stdlib.h>

#include <CommCtrl.h>

#include "perceptron.h"
#include "mnist.h"

#define PARTIAL_RUN 1
#define TANH_INPUT 0
#define HIDDEN_NEURONS 120
#define MAX_EPOCHS 159

#define NN1_ITERATIONS 100 //5000

#define MAX_LOADSTRING 100

volatile int Completed = 0;

int wWidth = 0;
int wHeight = 0;

int NetworkResults[60000];

mlp_layer_t *layers[3];

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND ghWnd;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Progress(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ProgressRender(HWND, UINT, WPARAM, LPARAM);


volatile HWND hDialog = NULL;
volatile HWND hDialogRender = NULL;
volatile int shutdown = 0;

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

mnist_t *mnist_training;
mnist_t *mnist_testing;

#define MNIST_TRAINING 1
#define MNIST_TESTING 0


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

void
run_nn(void)
{
	perceptron_t *inputs[2], *hiddens[5], *outputs[2];
	mlp_layer_t *layers[3];
	perceptron_t *bias;
	int i, j;

	for (i = 0; i < sizeof(layers) / sizeof(layers[0]); i++)
		layers[i] = mlp_layer_new();


	for (i = 0; i < sizeof(inputs) / sizeof(inputs[0]); i++) {
		inputs[i] = perceptron_new(PERCEPTRON_RECEPTOR);
		mlp_layer_add(layers[0], inputs[i]);
	}

	bias = perceptron_new(PERCEPTRON_BIAS);

	for (i = 0; i < sizeof(hiddens) / sizeof(hiddens[0]); i++) {
		hiddens[i] = perceptron_new(PERCEPTRON_NEURON);
		mlp_layer_add(layers[1], hiddens[i]);
		for (j = 0; j < sizeof(inputs) / sizeof(inputs[0]); j++)
			perceptron_link(inputs[j], hiddens[i]);
		perceptron_link(bias, hiddens[i]);
	}
	mlp_layer_add(layers[0], bias);

	bias = perceptron_new(PERCEPTRON_BIAS);

	for (i = 0; i < sizeof(outputs) / sizeof(outputs[0]); i++) {
		outputs[i] = perceptron_new(PERCEPTRON_OUTPUT);
		mlp_layer_add(layers[2], outputs[i]);
		for (j = 0; j < sizeof(hiddens) / sizeof(hiddens[0]); j++)
			perceptron_link(hiddens[j], outputs[i]);
		perceptron_link(bias, outputs[i]);
	}
	mlp_layer_add(layers[1], bias);

#if TANH_INPUT == 0
	int truths[4][4] = {
		{ 1, 0, 1, 1 },
		{ 0, 1, 1, 1 },
		{ 1, 1, 0, 1 },
		{ 0, 0, 1, 0 },
	};
#else
	int truths[4][4] = {
		{ 1, -1, 1, 1 },
		{ -1, 1, 1, 1 },
		{ 1, 1, 0, 1 },
		{ -1, -1, 1, 0 },
	};
#endif
	int right = 0;
	int total = 0;
	real answers[2];
	char buf[BUFSIZ];

	// train network
	for (i = 0; i < NN1_ITERATIONS; i++) {
		for (j = 0; j < 4; j++) {
			inputs[0]->input[0] = (real)truths[j][0];
			inputs[1]->input[0] = (real)truths[j][1];

			mlp_layer_compute(layers[0]);
			mlp_layer_transfer(layers[0]);
			mlp_layer_compute(layers[1]);
			mlp_layer_transfer(layers[1]);
			mlp_layer_compute(layers[2]);
#if 1//TANH_INPUT == 0
			real threshold = 0.5;
#else
			real threshold = 0.0;
#endif
			int output1 = (outputs[0]->output > threshold) ? 1 : 0;
			int output2 = (outputs[1]->output > threshold) ? 1 : 0;

			answers[0] = (real)truths[j][2];
			answers[1] = (real)truths[j][3];

			mlp_layer_train(layers[2], answers);

			sprintf_s(buf, BUFSIZ, "[%d] %d %d = %d %d (got %f %f)\n", i, truths[j][0], truths[j][1], truths[j][2], truths[j][3],
				outputs[0]->output, outputs[1]->output);
			wchar_t *str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);

			if (output1 == truths[j][2] && output2 == truths[j][3])
				right++;

			total++;
		}
	}


	sprintf_s(buf, BUFSIZ, "total %d, correct %d\n", total, right);
	wchar_t *str = char_to_wchar(buf);
	OutputDebugString(str);
	free(str);

	//exit(0);
}

uint32_t ChangeEndianness(uint32_t value)
{
	uint32_t result = 0;
	result |= (value & 0x000000FF) << 24;
	result |= (value & 0x0000FF00) << 8;
	result |= (value & 0x00FF0000) >> 8;
	result |= (value & 0xFF000000) >> 24;
	return result;
}

void
run_nn2(void)
{
	int i, j;
	wchar_t *str;
	char buf[BUFSIZ];

	srand(1313);

	mnist_training = mnist_new(MNIST_TRAINING);
	mnist_testing = mnist_new(MNIST_TESTING);

	perceptron_t **inputs, *hiddens[HIDDEN_NEURONS], *outputs[10];
	//mlp_layer_t *layers[3];
	perceptron_t *bias;


	inputs = (perceptron_t **)malloc(sizeof(perceptron_t *) * mnist_training->rows * mnist_training->cols);

	for (i = 0; i < sizeof(layers) / sizeof(layers[0]); i++)
		layers[i] = mlp_layer_new();

	//bias = perceptron_new(PERCEPTRON_BIAS);
	//mlp_layer_add(layers[0], bias);

	int n = mnist_training->rows * mnist_training->cols;
	for (i = 0; i < n; i++) {
		inputs[i] = perceptron_new(PERCEPTRON_RECEPTOR);
		mlp_layer_add(layers[0], inputs[i]);
	}

	bias = perceptron_new(PERCEPTRON_BIAS);
	for (i = 0; i < sizeof(hiddens) / sizeof(hiddens[0]); i++) {
		hiddens[i] = perceptron_new(PERCEPTRON_NEURON);
		mlp_layer_add(layers[1], hiddens[i]);
		//perceptron_link(bias, hiddens[i]);

		for (j = 0; j < n; j++)
			perceptron_link(inputs[j], hiddens[i]);

		perceptron_link(bias, hiddens[i]);
	}
	mlp_layer_add(layers[0], bias);

	bias = perceptron_new(PERCEPTRON_BIAS);
	for (i = 0; i < sizeof(outputs) / sizeof(outputs[0]); i++) {
		outputs[i] = perceptron_new(PERCEPTRON_OUTPUT);
		mlp_layer_add(layers[2], outputs[i]);
		
		//perceptron_link(bias, outputs[i]);
		for (j = 0; j < sizeof(hiddens) / sizeof(hiddens[0]); j++)
			perceptron_link(hiddens[j], outputs[i]);

		perceptron_link(bias, outputs[i]);
	}
	mlp_layer_add(layers[1], bias);

	sprintf_s(buf, BUFSIZ, "INFO: training on images...\n");
	str = char_to_wchar(buf);
	OutputDebugString(str);
	free(str);


	// wait for the dialog window to initialize
	
	while (hDialog == NULL)
		;

	HWND hProgress1 = GetDlgItem(hDialog, IDC_PROGRESS1);
	HWND hProgress2 = GetDlgItem(hDialog, IDC_PROGRESS2);
	HWND hStatic1 = GetDlgItem(hDialog, IDC_STATIC1);
	HWND hStatic2 = GetDlgItem(hDialog, IDC_STATIC2);

	if (!hProgress1 || !hProgress2)
		return;


	//SendMessage(hProgress, PBM_SETRANGE, 0, 6000);

//	UpdateWindow(hDialog);




	int *indexes = NULL;

	//uint8_t *image = (uint8_t *)malloc(rows * cols);
	real results[10], answers[10];
	int correct = 0;
	int total = 0;
#if PARTIAL_RUN == 1
	int max_epochs = 3;
#else
	int max_epochs = MAX_EPOCHS;
#endif
	for (int y = 0; y < max_epochs; y++) {
		total = correct = 0;

		SendMessage(hProgress2, PBM_SETPOS, (int)((real)100 / (real)(max_epochs + 1) * (real)(y + 1)), 0);
		
		sprintf_s(buf, BUFSIZ, "Epoch %d of %d\n", y, max_epochs);
		str = char_to_wchar(buf);
		SendMessage(hStatic2, WM_SETTEXT, NULL, (LPARAM)str);
		free(str);

		//if (mnist == NULL)
		//	mnist = mnist_new(1);

		if (indexes == NULL)
			indexes = (int *)malloc(mnist_training->items * sizeof(int));
		memset(indexes, 0, mnist_training->items * sizeof(int));
#if 1
		for (int i = mnist_training->items - 1; i > 0; i--) {
			int idx, blank = 0;
			for (j = 0; j < mnist_training->items; j++)
				if (indexes[j] == 0)
					blank++;
			
			idx = rand() % blank;
			int k;
			for (j = 0, k = 0; j < mnist_training->items; j++)
				if (indexes[j] == 0) {
					if (idx == k) {
						indexes[j] = i;
						break;
					}
					k++;
				}
		}
#else
		for (int i = 0; i < mnist_training->items; i++)
			indexes[i] = i;
#endif


#if PARTIAL_RUN == 1
		int n_training = mnist_training->items;
#else
		int n_training = mnist_training->items;
#endif
	for (i = 0; i < n_training; i++) {
		if (i % 1000 == 0) {
			if (shutdown)
				return;
			sprintf_s(buf, BUFSIZ, "Training %d of %d\n", i, n_training);
			str = char_to_wchar(buf);
			SendMessage(hStatic1, WM_SETTEXT, NULL, (LPARAM)str);
			free(str);

			SendMessage(hProgress1, PBM_SETPOS, (int)((real)100 / (real)n_training * (real)i), 0);
		}
			
			//if (mnist->label != 4 && mnist->label != 5)
				//	continue;

			total++;
			/*
			for (j = 0; j < rows*cols; j++) {
				if (image[j] < 255 / 4)
					inputs[j]->input[0] = 0.0;
				else
					inputs[j]->input[0] = 0.75 + (((real)0.25 / (real)255.0) * (real)image[j]);
			}
			*/
			int n = mnist_training->rows * mnist_training->cols;
			for (j = 0; j < n; j++) {
#if TANH_INPUT == 0
				inputs[j]->input[0] = 1.0 / 255.0 * (real)mnist_training->images[indexes[i]][j];
#else
				inputs[j]->input[0] = ((2.0 / 255.0) * (real)mnist_training->images[indexes[i]][j]) - 1.0;
#endif
			}



			for (int x = 0; x < 1; x++) {


				mlp_layer_compute(layers[0]);
				mlp_layer_transfer(layers[0]);
				mlp_layer_compute(layers[1]);
				mlp_layer_transfer(layers[1]);
				mlp_layer_compute(layers[2]);

				int right = 1;
#if 1 //TANH_INPUT == 0
				real threshold = 0.5;
#else
				real threshold = 0.0;
#endif
				for (j = 0; j < 10; j++) {
					results[j] = layers[2]->perceptrons[j]->output;

					if (mnist_training->labels[indexes[i]] == j) {
						answers[j] = 1.0;
						if (!(results[j] >= threshold))
							right = 0;
					}
					else if (mnist_training->labels[indexes[i]] != j) {
						answers[j] = 0.0;
						if (!(results[j] < threshold))
							right = 0;
					}
				}

				if (x == 0)
					if (right)
						correct++;


				mlp_layer_train(layers[2], answers);

#if 0
				if (x == 0) {
					sprintf_s(buf, BUFSIZ, "DEBUG: %d [%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f] expected %d (%s)\n", i,
						results[0], results[1], results[2], results[3], results[4], results[5], results[6], results[7], results[8], results[9],
						mnist_training->labels[i], (right) ? "correct" : "incorrect");
					wchar_t *str = char_to_wchar(buf);
					OutputDebugString(str);
					free(str);
				}
#endif
			}
		}
		SendMessage(hProgress1, PBM_SETPOS, 0, 0);

		
		sprintf_s(buf, BUFSIZ, "INFO: EPOCH %d trained with %d images %d initially correct guesses, total error: %.2f%%\n", y + 1, total, correct, 100.0 - (100.0 / (real)total * (real)correct));
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
		
		//mnist_destroy(mnist);
		//mnist = NULL;
		//}

		real sum = 0;

		total = correct = 0;
#if PARTIAL_RUN == 1
		int n_testing = mnist_testing->items;
#else
		int n_testing = mnist_testing->items;
#endif
		for (i = 0; i < n_testing; i++) {
			if (i % 1000 == 0) {
				if (shutdown)
					return;
				sprintf_s(buf, BUFSIZ, "Testing %d of %d\n", i, n_testing);
				str = char_to_wchar(buf);
				SendMessage(hStatic1, WM_SETTEXT, NULL, (LPARAM)str);
				free(str);

				SendMessage(hProgress1, PBM_SETPOS, (int)((real)100 / (real)n_testing * (real)i), 0);
			}


				//if (mnist->label != 4 && mnist->label != 5)
				//	continue;

			total++;
			/*
			for (j = 0; j < rows*cols; j++) {
			if (image[j] < 255 / 4)
			inputs[j]->input[0] = 0.0;
			else
			inputs[j]->input[0] = 0.75 + (((real)0.25 / (real)255.0) * (real)image[j]);
			}
			*/
			int n = mnist_testing->rows * mnist_testing->cols;
			for (j = 0; j < n; j++) {
#if TANH_INPUT == 0
				inputs[j]->input[0] = 1.0 / 255.0 * (real)mnist_testing->images[i][j];
#else
				inputs[j]->input[0] = ((2.0 / 255.0) * (real)mnist_testing->images[i][j]) + 1.0;
#endif
			}



			for (int x = 0; x < 1; x++) {


				mlp_layer_compute(layers[0]);
				mlp_layer_transfer(layers[0]);
				mlp_layer_compute(layers[1]);
				mlp_layer_transfer(layers[1]);
				mlp_layer_compute(layers[2]);

				int right = 1;
#if TANH_INPUT == 0
				real threshold = 0.5;
#else
				real threshold = 0.0;
#endif
				for (j = 0; j < 10; j++) {
					results[j] = layers[2]->perceptrons[j]->output;

					if (mnist_testing->labels[i] == j) {
						answers[j] = 1.0;
						if (!(results[j] >= threshold))
							right = 0;
					}
					else if (mnist_testing->labels[i] != j) {
						answers[j] = 0.0;
						if (!(results[j] < threshold))
							right = 0;
					}
				}

			//	real sum = 0;
				for (j = 0; j < 10; j++) {
					sum += pow(answers[j] - results[j], 2.0);
				}

				if (x == 0)
					if (right) {
						correct++;
						NetworkResults[i] = 1;
					}
					else {
						NetworkResults[i] = 0;
					}


				//mlp_layer_train(layers[2], answers);

#if 0
				if (x == 0 && y == max_epochs - 1) {
					sprintf_s(buf, BUFSIZ, "DEBUG: %d [%.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f] expected %d (%s)\n", i,
						results[0], results[1], results[2], results[3], results[4], results[5], results[6], results[7], results[8], results[9],
						mnist_testing->labels[i], (right) ? "correct" : "incorrect");
					wchar_t *str = char_to_wchar(buf);
					OutputDebugString(str);
					free(str);
				}
#endif
			}
		}
		sprintf_s(buf, BUFSIZ, "INFO: processed %d images %d correct guesses %.2f%% accuracy (%.2f%% error) (MSE %f)\n", total, correct,
			100.0 / (real)total * (real)correct,
			100.0 - (100.0 / (real)total * (real)correct),
			sum / (real)n_testing);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
	
		//mnist_destroy(mnist);
		//mnist = NULL;
	}

	for (int i = 1; i < sizeof(layers)/sizeof(layers[0]); i++) {
		sprintf_s(buf, BUFSIZ, "LAYER %d\n-----------------\n", i);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);

		for (int j = 0; j < layers[i]->n_perceptrons; j++) {
			sprintf_s(buf, BUFSIZ, " %d (%d) [ ", j, layers[i]->perceptrons[j]->n_inputs);
			str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);


			for (int k = 0; k < layers[i]->perceptrons[j]->n_inputs; k++) {
				float weight = layers[i]->perceptrons[j]->weights[k];

				if (layers[i]->perceptrons[j]->inputs[k]->type == PERCEPTRON_BIAS)
					sprintf_s(buf, BUFSIZ, " (%.2f) ", weight);
				else
					sprintf_s(buf, BUFSIZ, " %.2f ", weight);
				str = char_to_wchar(buf);
				OutputDebugString(str);
				free(str);

			}

			sprintf_s(buf, BUFSIZ, " ]\n", j);
			str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str);

		}
	}

	Completed = 1;
	SendMessage(hDialog, WM_CLOSE, 0, 0);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
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
    LoadStringW(hInstance, IDC_MULTILAYERMULTIOUTPUTPERCEPTRON, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTILAYERMULTIOUTPUTPERCEPTRON));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTILAYERMULTIOUTPUTPERCEPTRON));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MULTILAYERMULTIOUTPUTPERCEPTRON);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

DWORD WINAPI NetworkFunc(LPVOID lpParam)
{
	run_nn2();
	return 0;
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

   ghWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
      CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

   if (!ghWnd)
   {
	   return FALSE;
   }

 
   run_nn();

   /*
   RECT RclParent;
   // Get the client rectangle of the parent window.
   GetClientRect(hWnd, &RclParent);
   int iHThumb = GetSystemMetrics(SM_CXHTHUMB);
   int iVThumb = GetSystemMetrics(SM_CYVTHUMB);

   // Create a horizontal scroll bar to put in the window.
   HWND hWndHorzScroll = CreateWindow(
	   "SCROLLBAR",
	   (LPSTR)NULL,
	   WS_CHILD | WS_VISIBLE | SBS_HORZ | SBS_BOTTOMALIGN,
	   RclParent.left, RclParent.top, RclParent.right - iHThumb, RclParent.bottom,
	   hWnd,
	   (HMENU)IDC_HSCROLL,
	   (HANDLE)hInstance,
	   NULL);

   SetScrollRange(hWndHorzScroll, SB_CTL, 0, 100, FALSE);

   // Create a vertical scroll bar to put in the window.
   HWND hWndVertScroll = CreateWindow(
	   "Scrollbar",
	   (LPSTR)NULL,
	   WS_CHILD | WS_VISIBLE | SBS_VERT | SBS_RIGHTALIGN,
	   RclParent.left, RclParent.top, RclParent.right, RclParent.bottom - iVThumb,
	   hWnd,
	   (HMENU)IDC_VERT,
	   hInstance,
	   NULL);

   SetScrollRange(hWndVertScroll, SB_CTL, 0, 10000/100, FALSE);
   */


   //SendMessage(hWnd, SBM_SETRANGEREDRAW, 1, 1);

 
//   run_nn2();
   ShowWindow(ghWnd, nCmdShow);
   UpdateWindow(ghWnd);

   return TRUE;
}

void
DrawPoint(HDC hdc, int x, int y)
{
	MoveToEx(hdc, x, y, NULL);
	LineTo(hdc, x + 1, y);
}

/*
void
Paint(HWND hWnd, HDC hdc)
{
	HDC hdcMem = CreateCompatibleDC(hdc);
	RECT rc;
	HBITMAP hbmMem;

	GetClientRect(hWnd, &rc);

	//hbmMem = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
	hbmMem = CreateCompatibleBitmap(hdc, 28 * 100, 28 * 1000);
	SelectObject(hdcMem, hbmMem);

	//  draw background
	SelectObject(hdcMem, blackBrush);
	Rectangle(hdcMem, 0, wHeight, wWidth, 0);

	SelectObject(hdcMem, WHITE_BRUSH);

	int row = 0, col = 0;
	for (int i = 0; i < mnist->items; i++) {
		for (int x = 0; x < mnist->cols; x++) {
			for (int y = 0; y < mnist->rows; y++) {
				uint8_t pixel = mnist->images[i][y * 28 + x];
				if (pixel > 128) {
					if (NetworkResults[i] == 0)
						SelectObject(hdcMem, redPen);
					else
						SelectObject(hdcMem, greenPen);
				}
				else
					SelectObject(hdcMem, bluePen);
				DrawPoint(hdcMem, (col * mnist->cols) + x, wHeight - (row * mnist->rows) + y);
			}
		}
		col++;
		if (col == 100) {
			col = 0;
			row++;
		}
	}

	BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdcMem, 0, 0, SRCCOPY);
	DeleteObject(hbmMem);
	DeleteObject(hdcMem);
//	return hbmMem;
}
*/

void
Paint(HDC hdcMem)
{
	while (hDialogRender == NULL)
		;

	HWND hProgress = GetDlgItem(hDialogRender, IDC_PROGRESS);

	SelectObject(hdcMem, greenBrush);
	Rectangle(hdcMem, 0, 28 * 100, 28 * 100, 0);


	//SelectObject(hdcMem, WHITE_BRUSH);
	COLORREF color;
	int row = 0, col = 0;
	for (int i = 0; i < mnist_testing->items; i++) {
		if (i % 100 == 0) {
			if (shutdown)
				return;
			SendMessage(hProgress, PBM_SETPOS, (int)((real)100 / (real)mnist_testing->items * (real)i), 0);
		}


		for (int x = 0; x < mnist_testing->cols; x++) {
			for (int y = 0; y < mnist_testing->rows; y++) {
				uint8_t pixel = mnist_testing->images[i][y * 28 + x];
				/*	if (pixel > 128) {
				if (NetworkResults[i] == 0)
				//SelectObject(hdcMem, redPen);
				color = red;
				else
				//SelectObject(hdcMem, greenPen);
				color = green;
				}
				else
				//SelectObject(hdcMem, bluePen);
				color = blue;*/
				if (NetworkResults[i] == 0)
					color = RGB(pixel, 0, 255 - pixel);
				else
					color = RGB(0, pixel, 255 - pixel);
				//}
				//DrawPoint(hdcMem, (col * mnist->cols) + x, wHeight - (row * mnist->rows) + y);
				SetPixel(hdcMem, (col * mnist_testing->cols) + x, (row * mnist_testing->rows) + y, color);
			}
		}
		col++;
		if (col == 100) {
			col = 0;
			row++;
		}
	}


	for (int i = 0; i < layers[1]->n_perceptrons; i++) {
		perceptron_t *p = layers[1]->perceptrons[i];

		if (p->type == PERCEPTRON_BIAS)
			continue;

		for (int x = 0; x < mnist_testing->cols; x++) {
			for (int y = 0; y < mnist_testing->rows; y++) {
				uint8_t pixel = (uint8_t)(sigmoid(p->weights[(x * mnist_testing->cols) + y]) * 255);
				color = RGB(255 - pixel, 255 - pixel, 255 - pixel);
				SetPixel(hdcMem, (i * mnist_testing->rows) + x, y, color);
			}
		}
	}

	/*
	for (int i = 0; i < layers[2]->n_perceptrons; i++) {
		perceptron_t *p = layers[2]->perceptrons[i];

		if (p->type == PERCEPTRON_BIAS)
			continue;

		for (int x = 0; x < mnist->cols; x++) {
			for (int y = 0; y < mnist->rows; y++) {
				uint8_t pixel = (uint8_t)(sigmoid(p->weights[(x * mnist->cols) + y]) * 255);
				color = RGB(pixel, pixel, pixel);
				SetPixel(hdcMem, (i * mnist->rows) + x, mnist->cols + y, color);
			}
		}
	}
	*/

	Completed = 1;
	SendMessage(hDialogRender, WM_CLOSE, 0, 0);
}

DWORD WINAPI RenderFunc(LPVOID lpParam)
{
	HDC hdcMem = (HDC)lpParam;
	Paint(hdcMem);
	return 0;
}


bool BitmapToClipboard(HBITMAP hBM, HWND hWnd)
{
	if (!OpenClipboard(hWnd))
		return false;
	
	EmptyClipboard();

	BITMAP bm;
	GetObject(hBM, sizeof(bm), &bm);

	BITMAPINFOHEADER bi;
	ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = bm.bmWidth;
	bi.biHeight = bm.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = bm.bmBitsPixel;
	bi.biCompression = BI_RGB;
	if (bi.biBitCount <= 1)	// make sure bits per pixel is valid
		bi.biBitCount = 1;
	else if (bi.biBitCount <= 4)
		bi.biBitCount = 4;
	else if (bi.biBitCount <= 8)
		bi.biBitCount = 8;
	else // if greater than 8-bit, force to 24-bit
		bi.biBitCount = 24;

	// Get size of color table.
	SIZE_T dwColTableLen = (bi.biBitCount <= 8) ? (1 << bi.biBitCount) * sizeof(RGBQUAD) : 0;

	// Create a device context with palette
	HDC hDC = GetDC(NULL);
	HPALETTE hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);
	HPALETTE hOldPal = SelectPalette(hDC, hPal, FALSE);
	RealizePalette(hDC);

	// Use GetDIBits to calculate the image size.
	GetDIBits(hDC, hBM, 0, (UINT)bi.biHeight, NULL,	(LPBITMAPINFO)&bi, DIB_RGB_COLORS);

	// If the driver did not fill in the biSizeImage field, then compute it.
	// Each scan line of the image is aligned on a DWORD (32bit) boundary.
	if (0 == bi.biSizeImage)
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;

	// Allocate memory
	HGLOBAL hDIB = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + dwColTableLen + bi.biSizeImage);
	if (hDIB)
	{
		union tagHdr_u
		{
			LPVOID             p;
			LPBYTE             pByte;
			LPBITMAPINFOHEADER pHdr;
			LPBITMAPINFO       pInfo;
		} Hdr;

		Hdr.p = GlobalLock(hDIB);
		// Copy the header
		CopyMemory(Hdr.p, &bi, sizeof(BITMAPINFOHEADER));
		// Convert/copy the image bits and create the color table
		int nConv = GetDIBits(hDC, hBM, 0, (UINT)bi.biHeight,
			Hdr.pByte + sizeof(BITMAPINFOHEADER) + dwColTableLen,
			Hdr.pInfo, DIB_RGB_COLORS);
		GlobalUnlock(hDIB);
		if (!nConv)
		{
			GlobalFree(hDIB);
			hDIB = NULL;
		}
	}
	if (hDIB)
		SetClipboardData(CF_DIB, hDIB);
	CloseClipboard();
	SelectPalette(hDC, hOldPal, FALSE);
	ReleaseDC(NULL, hDC);
	
	return (hDIB != NULL) ? true : false;
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
	HDC hdc;
	PAINTSTRUCT ps;
	SCROLLINFO si;

	// These variables are required by BitBlt. 
	static HDC hdcWin;           // window DC 
	static HDC hdcScreen;        // DC for entire screen 
	static HDC hdcScreenCompat;  // memory DC for screen 
	static HBITMAP hbmpCompat;   // bitmap handle to old DC 
	static BITMAP bmp;           // bitmap data structure 
	static BOOL fBlt;            // TRUE if BitBlt occurred 
	static BOOL fScroll;         // TRUE if scrolling occurred 
	static BOOL fSize;           // TRUE if fBlt & WM_SIZE 

								 // These variables are required for horizontal scrolling. 
	static int xMinScroll;       // minimum horizontal scroll value 
	static int xCurrentScroll;   // current horizontal scroll value 
	static int xMaxScroll;       // maximum horizontal scroll value 

								 // These variables are required for vertical scrolling. 
	static int yMinScroll;       // minimum vertical scroll value 
	static int yCurrentScroll;   // current vertical scroll value 
	static int yMaxScroll;       // maximum vertical scroll value 

	static int painted;
	static int painting;


    switch (message)
    {
	case WM_CREATE:
		// Create a normal DC and a memory DC for the entire 
		// screen. The normal DC provides a snapshot of the 
		// screen contents. The memory DC keeps a copy of this 
		// snapshot in the associated bitmap. 
		hdcScreen = CreateDC(L"DISPLAY", (PCTSTR)NULL,
			(PCTSTR)NULL, (CONST DEVMODE *) NULL);
		hdcScreenCompat = CreateCompatibleDC(hdcScreen);
		
		// Retrieve the metrics for the bitmap associated with the 
		// regular device context. 
		bmp.bmBitsPixel =
			(BYTE)GetDeviceCaps(hdcScreen, BITSPIXEL);
		bmp.bmPlanes = (BYTE)GetDeviceCaps(hdcScreen, PLANES);
		bmp.bmWidth = 28 * 100;//GetDeviceCaps(hdcScreen, HORZRES);
		bmp.bmHeight = 28 * 100;//GetDeviceCaps(hdcScreen, VERTRES);

		// The width must be byte-aligned. 
		bmp.bmWidthBytes = ((bmp.bmWidth + 15) &~15) / 8;

		// Create a bitmap for the compatible DC. 
		hbmpCompat = CreateBitmap(bmp.bmWidth, bmp.bmHeight,
			bmp.bmPlanes, bmp.bmBitsPixel, (CONST VOID *) NULL);

		// Select the bitmap for the compatible DC. 
		SelectObject(hdcScreenCompat, hbmpCompat);

		// Initialize the flags. 
		fBlt = FALSE;
		fScroll = FALSE;
		fSize = TRUE;
		
		// Initialize the horizontal scrolling variables. 
		xMinScroll = 0;
		xCurrentScroll = 0;
		xMaxScroll = 0;

		// Initialize the vertical scrolling variables. 
		yMinScroll = 0;
		yCurrentScroll = 0;
		yMaxScroll = 0;

		painted = 0;
		painting = 0;
		break;
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
			PRECT prect;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
   
			if (!painting) {


				if (!painted) {
					painting = 1;

					HDC hdcMem = hdcScreenCompat;


					DWORD dwThreadId;

					HANDLE hThread = CreateThread(
						NULL,                   // default security attributes
						0,                      // use default stack size  
						NetworkFunc,       // thread function name
						NULL,          // argument to thread function 
						0,                      // use default creation flags 
						&dwThreadId);   // returns the thread identifier 

					DialogBox(hInst, MAKEINTRESOURCE(IDD_PROGRESSBOX), hWnd, Progress);

					if (shutdown) {
						EndPaint(hWnd, &ps);
						break;
					}

					WaitForSingleObject(hThread, INFINITE);

					//mnist = mnist_new(0);


					//DWORD dwThreadId;

					Completed = 0;
					hDialog = NULL;

					hThread = CreateThread(
						NULL,                   // default security attributes
						0,                      // use default stack size  
						RenderFunc,       // thread function name
						hdcMem,          // argument to thread function 
						0,                      // use default creation flags 
						&dwThreadId);   // returns the thread identifier 

					DialogBox(hInst, MAKEINTRESOURCE(IDD_PROGRESSRENDERBOX), hWnd, ProgressRender);

					if (shutdown) {
						EndPaint(hWnd, &ps);
						break;
					}
					
					WaitForSingleObject(hThread, INFINITE);

					//Paint(hdcMem);

					painted = 1;


					painting = 0;
				}

				// If the window has been resized and the user has 
				// captured the screen, use the following call to 
				// BitBlt to paint the window's client area. 
				if (fSize || !fScroll)
				{
					BitBlt(ps.hdc,
						0, 0,
						bmp.bmWidth, bmp.bmHeight,
						hdcScreenCompat,
						xCurrentScroll, yCurrentScroll,
						SRCCOPY);

					fSize = FALSE;
				}




				// If scrolling has occurred, use the following call to 
				// BitBlt to paint the invalid rectangle. 
				// 
				// The coordinates of this rectangle are specified in the 
				// RECT structure to which prect points. 
				// 
				// Note that it is necessary to increment the seventh 
				// argument (prect->left) by xCurrentScroll and the 
				// eighth argument (prect->top) by yCurrentScroll in 
				// order to map the correct pixels from the source bitmap. 
				if (fScroll)
				{
					/*
					BitBlt(ps.hdc,
						0, 0,
						bmp.bmWidth, bmp.bmHeight,
						hdcScreenCompat,
						xCurrentScroll, yCurrentScroll,
						SRCCOPY);
						*/


					prect = &ps.rcPaint;

					BitBlt(ps.hdc,
						prect->left, prect->top,
						(prect->right - prect->left),
						(prect->bottom - prect->top),
						hdcScreenCompat,
						prect->left + xCurrentScroll,
						prect->top + yCurrentScroll,
						SRCCOPY);

					fScroll = FALSE;
				}
			}
			EndPaint(hWnd, &ps);
        }
        break;
	case WM_MOUSEWHEEL:
	{
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

		char buf[BUFSIZ];
		wchar_t *str;
		sprintf_s(buf, BUFSIZ, "DEBUG: Mouse wheel moved [%d]\n", zDelta);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);

		zDelta /= WHEEL_DELTA;

		int xDelta = 0;
		int yDelta;     // yDelta = new_pos - current_pos 
		int yNewPos;    // new position 
						// New position must be between 0 and the screen height. 

		yNewPos = yCurrentScroll + (-zDelta * 28);

		yNewPos = max(0, yNewPos);
		yNewPos = min(yMaxScroll, yNewPos);

		// If the current position does not change, do not scroll.
		if (yNewPos == yCurrentScroll)
			break;

		// Set the scroll flag to TRUE. 
		fScroll = TRUE;

		// Determine the amount scrolled (in pixels). 
		yDelta = yNewPos - yCurrentScroll;

		// Reset the current scroll position. 
		yCurrentScroll = yNewPos;

		// Scroll the window. (The system repaints most of the 
		// client area when ScrollWindowEx is called; however, it is 
		// necessary to call UpdateWindow in order to repaint the 
		// rectangle of pixels that were invalidated.) 
		ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
			(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
			SW_INVALIDATE);
		UpdateWindow(hWnd);

		// Reset the scroll bar. 
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

	}
		break;
	case WM_KEYDOWN:
	{
		DWORD32 key = wParam;
		char buf[BUFSIZ];
		wchar_t *str;
		sprintf_s(buf, BUFSIZ, "DEBUG: Key pressed %d [%d]\n", lParam, wParam);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);
#define xVK_PGUP 33
#define xVK_PGDN 34
		if (key == VK_UP || key == VK_DOWN || key == xVK_PGUP || key == xVK_PGDN) {
/*			sprintf_s(buf, BUFSIZ, "DEBUG: Up key pressed %d %d (%d)\n", lParam, wParam, VK_UP);
			str = char_to_wchar(buf);
			OutputDebugString(str);
			free(str); */
			int xDelta = 0;
			int yDelta;     // yDelta = new_pos - current_pos 
			int yNewPos;    // new position 
							// New position must be between 0 and the screen height. 

			if (key == VK_UP)
				yNewPos = yCurrentScroll - 28;
			else if (key == VK_DOWN)
				yNewPos = yCurrentScroll + 28;
			else if (key == xVK_PGUP)
				yNewPos = yCurrentScroll - 28 * 10;
			else if (key == xVK_PGDN)
				yNewPos = yCurrentScroll + 28 * 10;

			yNewPos = max(0, yNewPos);
			yNewPos = min(yMaxScroll, yNewPos);

			// If the current position does not change, do not scroll.
			if (yNewPos == yCurrentScroll)
				break;

			// Set the scroll flag to TRUE. 
			fScroll = TRUE;

			// Determine the amount scrolled (in pixels). 
			yDelta = yNewPos - yCurrentScroll;

			// Reset the current scroll position. 
			yCurrentScroll = yNewPos;

			// Scroll the window. (The system repaints most of the 
			// client area when ScrollWindowEx is called; however, it is 
			// necessary to call UpdateWindow in order to repaint the 
			// rectangle of pixels that were invalidated.) 
			ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
				(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
				SW_INVALIDATE);
			UpdateWindow(hWnd);

			// Reset the scroll bar. 
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			si.nPos = yCurrentScroll;
			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		}
		else if (key == VK_LEFT || key == VK_RIGHT) {
			int xDelta;     // xDelta = new_pos - current_pos  
			int xNewPos;    // new position 
			int yDelta = 0;

			if (key == VK_LEFT)
				xNewPos = xCurrentScroll - 28;
			else if (key == VK_RIGHT)
				xNewPos = xCurrentScroll + 28;

			// New position must be between 0 and the screen width. 
			xNewPos = max(0, xNewPos);
			xNewPos = min(xMaxScroll, xNewPos);

			// If the current position does not change, do not scroll.
			if (xNewPos == xCurrentScroll)
				break;

			// Set the scroll flag to TRUE. 
			fScroll = TRUE;

			// Determine the amount scrolled (in pixels). 
			xDelta = xNewPos - xCurrentScroll;

			// Reset the current scroll position. 
			xCurrentScroll = xNewPos;

			// Scroll the window. (The system repaints most of the 
			// client area when ScrollWindowEx is called; however, it is 
			// necessary to call UpdateWindow in order to repaint the 
			// rectangle of pixels that were invalidated.) 
			ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
				(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
				SW_INVALIDATE);
			UpdateWindow(hWnd);

			// Reset the scroll bar. 
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS;
			si.nPos = xCurrentScroll;
			SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		}
	}
		break;
	case WM_SIZE:
		wWidth = LOWORD(lParam);
		wHeight = HIWORD(lParam);

		int xNewSize;
		int yNewSize;

		xNewSize = LOWORD(lParam);
		yNewSize = HIWORD(lParam);

		fSize = TRUE;

		// The horizontal scrolling range is defined by 
		// (bitmap_width) - (client_width). The current horizontal 
		// scroll value remains within the horizontal scrolling range. 
		xMaxScroll = max(bmp.bmWidth - xNewSize, 0);
		xCurrentScroll = min(xCurrentScroll, xMaxScroll);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = xMinScroll;
		si.nMax = bmp.bmWidth;
		si.nPage = xNewSize;
		si.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		// The vertical scrolling range is defined by 
		// (bitmap_height) - (client_height). The current vertical 
		// scroll value remains within the vertical scrolling range. 
		yMaxScroll = max(bmp.bmHeight - yNewSize, 0);
		yCurrentScroll = min(yCurrentScroll, yMaxScroll);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = yMinScroll;
		si.nMax = bmp.bmHeight;
		si.nPage = yNewSize;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

	case WM_HSCROLL:
	{
		int xDelta;     // xDelta = new_pos - current_pos  
		int xNewPos;    // new position 
		int yDelta = 0;

		switch (LOWORD(wParam))
		{
			// User clicked the scroll bar shaft left of the scroll box. 
		case SB_PAGEUP:
			xNewPos = xCurrentScroll - 28*10;
			break;

			// User clicked the scroll bar shaft right of the scroll box. 
		case SB_PAGEDOWN:
			xNewPos = xCurrentScroll + 28*10;
			break;

			// User clicked the left arrow. 
		case SB_LINEUP:
			xNewPos = xCurrentScroll - 28;
			break;

			// User clicked the right arrow. 
		case SB_LINEDOWN:
			xNewPos = xCurrentScroll + 28;
			break;

			// User dragged the scroll box. 
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			xNewPos = HIWORD(wParam);
			break;
		default:
			xNewPos = xCurrentScroll;
		}

		// New position must be between 0 and the screen width. 
		xNewPos = max(0, xNewPos);
		xNewPos = min(xMaxScroll, xNewPos);

		// If the current position does not change, do not scroll.
		if (xNewPos == xCurrentScroll)
			break;

		// Set the scroll flag to TRUE. 
		fScroll = TRUE;

		// Determine the amount scrolled (in pixels). 
		xDelta = xNewPos - xCurrentScroll;

		// Reset the current scroll position. 
		xCurrentScroll = xNewPos;

		// Scroll the window. (The system repaints most of the 
		// client area when ScrollWindowEx is called; however, it is 
		// necessary to call UpdateWindow in order to repaint the 
		// rectangle of pixels that were invalidated.) 
		ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
			(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
			SW_INVALIDATE);
		UpdateWindow(hWnd);

		// Reset the scroll bar. 
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = xCurrentScroll;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);

		break;
	}

	case WM_VSCROLL:
	{
		int xDelta = 0;
		int yDelta;     // yDelta = new_pos - current_pos 
		int yNewPos;    // new position 

		switch (LOWORD(wParam))
		{
			// User clicked the scroll bar shaft above the scroll box. 
		case SB_PAGEUP:
			yNewPos = yCurrentScroll - 28*10;
			break;

			// User clicked the scroll bar shaft below the scroll box. 
		case SB_PAGEDOWN:
			yNewPos = yCurrentScroll + 28*10;
			break;

			// User clicked the top arrow. 
		case SB_LINEUP:
			yNewPos = yCurrentScroll - 28;
			break;

			// User clicked the bottom arrow. 
		case SB_LINEDOWN:
			yNewPos = yCurrentScroll + 28;
			break;

			// User dragged the scroll box. 
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			yNewPos = HIWORD(wParam);
			break;

		default:
			yNewPos = yCurrentScroll;
		}
		// New position must be between 0 and the screen height. 
		yNewPos = max(0, yNewPos);
		yNewPos = min(yMaxScroll, yNewPos);

		// If the current position does not change, do not scroll.
		if (yNewPos == yCurrentScroll)
			break;

		// Set the scroll flag to TRUE. 
		fScroll = TRUE;

		// Determine the amount scrolled (in pixels). 
		yDelta = yNewPos - yCurrentScroll;

		// Reset the current scroll position. 
		yCurrentScroll = yNewPos;

		// Scroll the window. (The system repaints most of the 
		// client area when ScrollWindowEx is called; however, it is 
		// necessary to call UpdateWindow in order to repaint the 
		// rectangle of pixels that were invalidated.) 
		ScrollWindowEx(hWnd, -xDelta, -yDelta, (CONST RECT *) NULL,
			(CONST RECT *) NULL, (HRGN)NULL, (PRECT)NULL,
			SW_INVALIDATE);
		UpdateWindow(hWnd);

		// Reset the scroll bar. 
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = yCurrentScroll;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);

		break;
	}

	
	case WM_RBUTTONDOWN:
	{
		BitmapToClipboard(hbmpCompat, hWnd);
		/*
		// Get the compatible DC of the client area. 
		hdcWin = GetDC(hWnd);

		// Fill the client area to remove any existing contents. 
		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect(hdcWin, &rect, (HBRUSH)(COLOR_WINDOW + 1));

		// Copy the contents of the current screen 
		// into the compatible DC. 
		BitBlt(hdcScreenCompat, 0, 0, bmp.bmWidth,
			bmp.bmHeight, hdcScreen, 0, 0, SRCCOPY);

		// Copy the compatible DC to the client area.
		BitBlt(hdcWin, 0, 0, bmp.bmWidth, bmp.bmHeight,
			hdcScreenCompat, 0, 0, SRCCOPY);

		ReleaseDC(hWnd, hdcWin);
		fBlt = TRUE;
		break;
		*/
	}
	
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
/*
void CALLBACK
ProgressCallback(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	HWND hProgress = GetDlgItem(hWnd, IDC_PROGRESS1);

	if (!hProgress)
		return;

//	if (idEvent = 1001) {
	//SendMessage(hProgress, PBM_SETRANGE, 0, 100);
		//SendMessage(hProgress, PBM_SETPOS, 50, 0);

		UpdateWindow(hWnd);
//	}

}
*/

// Message handler for progress box.
INT_PTR CALLBACK Progress(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		hDialog = hDlg;

		HWND hProgress1 = GetDlgItem(hDlg, IDC_PROGRESS1);

		if (!hProgress1)
			return (INT_PTR)TRUE;

		SendMessage(hProgress1, PBM_SETRANGE, 0, MAKELPARAM(1, 100));
		SendMessage(hProgress1, PBM_SETSTEP, 0, 1);

		HWND hProgress2 = GetDlgItem(hDlg, IDC_PROGRESS2);

		if (!hProgress2)
			return (INT_PTR)TRUE;

		SendMessage(hProgress2, PBM_SETRANGE, 0, MAKELPARAM(1, 100));
		SendMessage(hProgress2, PBM_SETSTEP, 0, 1);

	//	SetTimer(hDlg, 1001, 1000, &ProgressCallback);
	}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON_CANCEL || LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			if (!Completed) {
				//ExitProcess(0);
				SendMessage(ghWnd, WM_CLOSE, 0, 0);
				shutdown = 1;
				//EndDialog(hDlg, 0);
				return (INT_PTR)TRUE;
			}
			else {
				EndDialog(hDlg, 0);
				return (INT_PTR)TRUE;
			}
		}
		break;
	case WM_CLOSE:
		if (!Completed) {
			//		ExitProcess(0);
			SendMessage(ghWnd, WM_CLOSE, 0, 0);
			shutdown = 1;
		}
			break;
	}

	return (INT_PTR)FALSE;
}


// Message handler for progress box.
INT_PTR CALLBACK ProgressRender(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		hDialogRender = hDlg;

		char buf[BUFSIZ];
		wchar_t *str;
		sprintf_s(buf, BUFSIZ, "INFO: hDialogRender: 0x%08x\n", (int)hDialogRender);
		str = char_to_wchar(buf);
		OutputDebugString(str);
		free(str);


		HWND hProgress = GetDlgItem(hDlg, IDC_PROGRESS);

		if (!hProgress)
			return (INT_PTR)TRUE;

		SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(1, 100));
		SendMessage(hProgress, PBM_SETSTEP, 0, 1);

		//	SetTimer(hDlg, 1001, 1000, &ProgressCallback);
	}
	return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BUTTON_CANCEL || LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			if (!Completed) {
				//SendMessage(ghWnd, WM_CLOSE, 0, 0);
				//shutdown = 1;
				exit(0);
				//EndDialog(hDlg, 0);
				return (INT_PTR)TRUE;
			}
			else {
				EndDialog(hDlg, 0);
				return (INT_PTR)TRUE;
			}
		}
		break;
	case WM_CLOSE:
		if (!Completed) {
			exit(0);
			//SendMessage(ghWnd, WM_CLOSE, 0, 0);
			//shutdown = 1;
		}
		break;
	}

	return (INT_PTR)FALSE;
}
