/*
Window code adapted from Vitali Baumtrok's work here:
https://github.com/vbsw/opengl-win32-example/blob/3.0/Main.cpp

Audio code adapted from floooh's work here:
https://github.com/floooh/sokol/blob/master/sokol_audio.h
*/
#include <t33/t33.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define COBJMACROS
#include <mmdeviceapi.h>
#include <audioclient.h>

void error_box(char *msg){
    MessageBoxA(0,msg,"Error",MB_ICONERROR);
}

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


static void fill_audio_buffer(void){
    
}
static HANDLE audioThread;
static DWORD WINAPI audio_thread_proc(_In_ LPVOID lpParameter){
    _saudio_wasapi_submit_buffer(_saudio.backend.thread.src_buffer_frames);
    IAudioClient_Start(_saudio.backend.audio_client);
    while (!_saudio.backend.thread.stop) {
        WaitForSingleObject(_saudio.backend.thread.buffer_end_event, INFINITE);
        UINT32 padding = 0;
        if (FAILED(IAudioClient_GetCurrentPadding(_saudio.backend.audio_client, &padding))) {
            continue;
        }
        SOKOL_ASSERT(_saudio.backend.thread.dst_buffer_frames >= padding);
        int num_frames = (int)_saudio.backend.thread.dst_buffer_frames - (int)padding;
        if (num_frames > 0) {
            _saudio_wasapi_submit_buffer(num_frames);
        }
    }
    return 0;
}

void create_window(char *title, int width, int height){
    //init window:
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
		fatal_error("Failed to register window class.");
	}
	HWND fakeWindow = CreateWindowExA(0,className,"fakeWindow",WS_OVERLAPPEDWINDOW,0,0,1,1,NULL,NULL,instance,NULL);
	if (!fakeWindow){
		fatal_error("Failed to make fake window.");
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

    //init audio:
    const GUID _CLSID_MMDeviceEnumerator = {0xbcde0395, 0xe52f, 0x467c, {0x8e,0x3d, 0xc4,0x57,0x92,0x91,0x69,0x2e}};
	const GUID _IID_IMMDeviceEnumerator = {0xa95664d2, 0x9614, 0x4f35, {0xa7,0x46, 0xde,0x8d,0xb6,0x36,0x17,0xe6}};
    const GUID _IID_IAudioClient = {0x1cb9ad4c, 0xdbfa, 0x4c32, {0xb1,0x78, 0xc2,0xf5,0x68,0xa7,0x03,0xb2}};
    const GUID _KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
    const GUID _IID_IAudioRenderClient = {0xf294acfc, 0x3146, 0x4483, {0xa7, 0xbf, 0xad, 0xdc, 0xa7, 0xc2, 0x60, 0xe2}};
    IMMDeviceEnumerator *enu = NULL;
    IMMDevice *dev = NULL;
    IAudioClient *client = NULL;
    IAudioRenderClient* renderClient = NULL;
    CoInitializeEx(0, COINIT_MULTITHREADED);
    HANDLE endEvent = CreateEventA(0, FALSE, FALSE, 0);
    if (!endEvent){
        fatal_error("Failed to create audio end event.");
    }
	if (FAILED(CoCreateInstance(&_CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &_IID_IMMDeviceEnumerator, (void**)&enu))){
        fatal_error("Failed to create WASAPI device enumerator.");
    }
    if (FAILED(enu->lpVtbl->GetDefaultAudioEndpoint(enu, eRender, eConsole, &dev))){
        fatal_error("Failed to get WASAPI default device.");
    }
    if (FAILED(dev->lpVtbl->Activate(dev, &_IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&client))){
        fatal_error("Failed to activate WASAPI default device.");
    }
    WAVEFORMATEXTENSIBLE fmtex = {0};
    fmtex.Format.nChannels = 2;
    fmtex.Format.nSamplesPerSec = 44100;
    fmtex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    fmtex.Format.wBitsPerSample = 32;
    fmtex.Format.nBlockAlign = (fmtex.Format.nChannels * fmtex.Format.wBitsPerSample) / 8;
    fmtex.Format.nAvgBytesPerSec = fmtex.Format.nSamplesPerSec * fmtex.Format.nBlockAlign;
    fmtex.Format.cbSize = 22;   /* WORD + DWORD + GUID */
    fmtex.Samples.wValidBitsPerSample = 32;
    fmtex.dwChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;
    fmtex.SubFormat = _KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    REFERENCE_TIME dur = (REFERENCE_TIME)(((double)2048) / (((double)fmtex.Format.nSamplesPerSec) * (1.0/10000000.0)));
    if (FAILED(client->lpVtbl->Initialize(
        client,
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK|AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM|AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY,
        dur, 0, (WAVEFORMATEX*)&fmtex, 0))){
        fatal_error("Failed to initialize WASAPI audio client.");
    }
    UINT32 bufferFrames;
    if (FAILED(client->lpVtbl->GetBufferSize(client, &bufferFrames))){
        fatal_error("Failed to get WASAPI buffer size.");
    }
    if (FAILED(client->lpVtbl->GetService(client, &_IID_IAudioRenderClient, (void**)&renderClient))){
        fatal_error("Failed to get WASAPI render client.");
    }
    if (FAILED(client->lpVtbl->SetEventHandle(client,endEvent))){
        fatal_error("Failed to set WASAPI event handle.");
    }
    audioThread = CreateThread(0,0,audio_thread_proc,0,0,0);
    if (!audioThread){
        fatal_error("Failed to create audio thread.");
    }

    //show window:
    ShowWindow(window,SW_SHOWDEFAULT);
}

void start_main_loop(void){
	MSG msg;
	while (GetMessageA(&msg,0,0,0)){
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}