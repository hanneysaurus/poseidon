#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include "windows.h"

#include <gl/GL.h>		//standard OpenGL include
#include <gl/GLU.h>		//OpenGL utilities
#include "gl/glut.h"

// --------------------------------------------------------
// FUNCTION DECLARATIONS
void idle(void);
void key(unsigned char k, int x, int y);
void draw(void);
void init(void);

// --------------------------------------------------------
// GLOBAL VARIABLES
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;

int main(int argc, char* argv[]) {

	std::cout << "Hello World" << std::endl;

	//initialize GLUT and create window
	glutInit(&argc, argv);
	//set up display mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("Poseidon");

	//drawing routine
	init();

	//callback functions
	glutKeyboardFunc(key);
	glutIdleFunc(idle);
	glutDisplayFunc(draw);

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	glutMainLoop();
	return 0;
}

void init(void){}

void draw(void){}

void key(unsigned char k, int x, int y) {
	switch (k) {
	case 27:
		exit(0);
		break;
	}
}

void idle(void){}