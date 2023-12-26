#include <t33/window.h>
#include <t33/base.h>

int main(int argc, char **argv){
	create_window("fuck",640,480);
	start_main_loop();
}

bool update_proc(double delta_time){
	glClearColor(1.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	printf("dt: %f\n",delta_time);
}

/*
User defined function called whenever the user presses or releases a key.
*/
void key_proc(int key, bool down){

}

/*
User defined function called whenever the user presses or releases a mouse button.
*/
void mouse_button_proc(int button, bool down){

}

/*
User defined function called whenever the user scrolls the mouse wheel.
*/
void mouse_scroll_proc(int scroll_amount){

}

/*
User defined function called whenever the mouse moves.
If the cursor is disabled, x and y are relative values,
otherwise, they are absolute coordinates relative to the top left
of the window.
*/
void mouse_position_proc(int x, int y){

}