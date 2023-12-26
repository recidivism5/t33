#if _WIN32

#include <t33/error_box.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void error_box(char *msg){
    MessageBoxA(0,msg,"Error",MB_ICONERROR);
}
#endif