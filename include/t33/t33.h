#pragma once

#include <t33/base.h>
#include <t33/linalg.h>
#include <t33/miniglad.h>
#include <t33/qoa.h>
#include <t33/qoi.h>

/*
Put all shaders into a hash table. User calls use_shader("colorNormal"); for instance.
Then set_uniform("mvp",mvp);
*/

/*
Creates a centered, resizable, maximizable, minimizable,
OpenGL 3.3 Core + VSync window of dimensions width*height.
Width and height must be positive.
*/
void create_window(char *title, int width, int height);

/*
Sets fullscreen mode on/off.
*/
//void set_fullscreen(bool fullscreen);

/*
Returns the width and height of the window.
*/
//void get_window_dimensions(int *width, int *height);

/*
Starts the main loop (processing input and calling update_proc).
Does not return until update_proc returns true.
*/
void start_main_loop(void);

/************************** USER DEFINED FUNCTIONS: you MUST define these *****************
I could have done callbacks for these but that would mean
    1. More code
    2. More state
    3. More aliasing
    4. Opening the door to different function names
So instead we just have the standard names update_proc, key_proc, etc,
which will appear in all projects using t33.
*/

/*
User defined main function. This is effectively the program entry
point. If you compile with subsystem:windows and don't change the
entry point, this gets called from WinMain in window_win32.c.
*/
int main(int argc, char **argv);

/*
User defined function that receives the time since last frame.
When this function returns, t33 calls SwapBuffers to blit the window
contents to the screen.
*/
void update_proc(double delta_time);

/*
User defined function called whenever the user presses or releases a key.
*/
void key_proc(int key, bool down);

/*
User defined function called whenever the user presses or releases a mouse button.
*/
void mouse_button_proc(int button, bool down);

/*
User defined function called whenever the user scrolls the mouse wheel.
*/
void mouse_scroll_proc(int scroll_amount);

/*
User defined function called whenever the mouse moves.
If the cursor is disabled, x and y are delta values,
otherwise, they are absolute coordinates relative to the top left
of the window.
*/
void mouse_position_proc(int x, int y);