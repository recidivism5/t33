/*
Adapted from Vitali Baumtrok's work here:
https://github.com/vbsw/opengl-win32-example/blob/3.0/Main.cpp
*/

#if _WIN32

#include <t33/window.h>
#include <t33/base.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* copied from wglext.h */
typedef BOOL(WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
typedef HGLRC(WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int *attribList);
typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);
#define WGL_SAMPLE_BUFFERS_ARB            0x2041
#define WGL_SAMPLES_ARB                   0x2042
#define WGL_DRAW_TO_WINDOW_ARB            0x2001
#define WGL_SUPPORT_OPENGL_ARB            0x2010
#define WGL_DOUBLE_BUFFER_ARB             0x2011
#define WGL_PIXEL_TYPE_ARB                0x2013
#define WGL_TYPE_RGBA_ARB                 0x202B
#define WGL_ACCELERATION_ARB              0x2003
#define WGL_FULL_ACCELERATION_ARB         0x2027
#define WGL_COLOR_BITS_ARB                0x2014
#define WGL_ALPHA_BITS_ARB                0x201B
#define WGL_DEPTH_BITS_ARB                0x2022
#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_SWAP_EXCHANGE_ARB             0x2028
#define WGL_SWAP_METHOD_ARB               0x2007
#define WGL_SWAP_COPY_ARB                 0x2029

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

static char *get_formatted_error(void){
	char buf[256];
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
		buf, (sizeof(buf) / sizeof(char)), NULL);
	return buf;
}

#ifndef _CONSOLE
int APIENTRY WinMain(
	_In_     HINSTANCE hInstance, //apparently WinMain needs these stupid annotations to not cause warnings in Visual Studio
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPWSTR    lpCmdLine,
	_In_     int       nCmdShow){
	return main(__argc,__argv);
}
#endif

static PROC getProc(char *procName){
	PROC proc = wglGetProcAddress((LPCSTR)procName);
	if (!proc){
		fatal_error("Failed to get %s",procName);
	}
	return proc;
}

static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	HDC dc = GetDC(hWnd);
	switch (message){
		case WM_SIZE:
			glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_KEYDOWN:
			if (wParam == 27){
				PostMessageA(hWnd, WM_CLOSE, 0, 0);
			} else if (wParam == 122){
				//setFullscreen(window.fullscreen = !window.fullscreen);
			}
			break;
		case WM_PAINT:{
			static bool started = false;
			static LARGE_INTEGER t0, t1, freq;
			if (!started){
				QueryPerformanceCounter(&t0);
				QueryPerformanceFrequency(&freq);
				t1 = t0;
				started = true;
			} else {
				QueryPerformanceCounter(&t1);
			}
			update_proc((double)(t1.QuadPart-t0.QuadPart)/(double)freq.QuadPart);
			t0 = t1;
			SwapBuffers(dc);
			return 0;
		}
		case WM_CLOSE:
			/*wglMakeCurrent(window.deviceContext, NULL);
			wglDeleteContext(window.renderContext);
			ReleaseDC(hWnd, window.deviceContext);
			DestroyWindow(hWnd);*/
			PostQuitMessage(0);
			break;
	}
	return DefWindowProcA(hWnd, message, wParam, lParam);
}

void create_window(char *title, int width, int height){
	char *className = "t33";
	HINSTANCE instance = GetModuleHandleA(NULL);
	WNDCLASSEXA wcex = {
		.cbSize = sizeof(WNDCLASSEXA),
		.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		.lpfnWndProc = (WNDPROC)wndProc,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = instance,
		.hIcon = 0,
		.hCursor = LoadCursorA(NULL, IDC_ARROW),
		.hbrBackground = NULL,
		.lpszMenuName = NULL,
		.lpszClassName = className,
		.hIconSm = NULL,
	};
	if (!RegisterClassExA(&wcex)){
		fatal_error("Failed to register window class: %s",get_formatted_error());
	}
	HWND fakeWindow = CreateWindowExA(0,className,"fakeWindow",WS_OVERLAPPEDWINDOW,0,0,1,1,NULL,NULL,instance,NULL);
	if (!fakeWindow){
		fatal_error("Failed to make fake window: %s",get_formatted_error());
	}
	HDC fakeDC = GetDC(fakeWindow);
	if (!fakeDC){
		fatal_error("Failed to get device context for fake window.");
	}
	int pixelFormat;
	PIXELFORMATDESCRIPTOR pixelFormatDesc;
	memset(&pixelFormatDesc, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pixelFormatDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelFormatDesc.nVersion = 1;
	pixelFormatDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pixelFormatDesc.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDesc.cColorBits = 32;
	pixelFormatDesc.cAlphaBits = 8;
	pixelFormatDesc.cDepthBits = 24;
	pixelFormat = ChoosePixelFormat(fakeDC,&pixelFormatDesc);
	if (!pixelFormat){
		fatal_error("Failed to choose pixel format for fake window.");
	}
	if (!SetPixelFormat(fakeDC,pixelFormat,&pixelFormatDesc)){
		fatal_error("Failed to set pixel format for fake window.");
	}
	HGLRC fakeRC = wglCreateContext(fakeDC);
	if (!fakeRC){
		fatal_error("Failed to create render context for fake window.");
	}
	RECT rect = {0, 0, width, height};
	if (!AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,false)){
		fatal_error("AdjustWindowRect failed.");
	}
	int newWidth = rect.right - rect.left;
	int newHeight = rect.bottom - rect.top;
	HWND window = CreateWindowExA(0,className,title,WS_OVERLAPPEDWINDOW,GetSystemMetrics(SM_CXSCREEN)/2-newWidth/2,GetSystemMetrics(SM_CYSCREEN)/2-newHeight/2,newWidth,newHeight,NULL,NULL,instance,NULL);
	if (!window){
		fatal_error("Failed to create window.");
	}
	HDC dc = GetDC(window);
	if (!dc){
		fatal_error("Failed to get device context for real window.");
	}
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
	wglMakeCurrent(fakeDC,fakeRC);
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)getProc("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)getProc("wglCreateContextAttribsARB");
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)getProc("wglSwapIntervalEXT");

	BOOL status;
	UINT numFormats;
	const int pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		/* WGL_SWAP_EXCHANGE_ARB causes problems with window menu in fullscreen */
		WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		0
	};
	int  contextAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};
	status = wglChoosePixelFormatARB(dc, pixelAttribs, NULL, 1, &pixelFormat, &numFormats);
	if (!status || !numFormats){
		fatal_error("wglChoosePixelFormatARB(...) failed.");
	}
	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	DescribePixelFormat(dc, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	if (!SetPixelFormat(dc, pixelFormat, &pfd)){
		fatal_error("SetPixelFormat(...) failed.");
	}
	HGLRC rc = wglCreateContextAttribsARB(dc, 0, contextAttributes);
	if (!rc){
		fatal_error("wglCreateContextAttribsARB() failed.");
	}
	wglMakeCurrent(fakeDC,NULL);
	wglDeleteContext(fakeRC);
	ReleaseDC(fakeWindow,fakeDC);
	DestroyWindow(fakeWindow);
	wglMakeCurrent(dc,rc);
	wglSwapIntervalEXT(1);//enable vsync
	gladLoadGL();
	ShowWindow(window,SW_SHOWDEFAULT);
}

void start_main_loop(void){
	MSG msg;
	while (GetMessageA(&msg,0,0,0)){
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

#endif