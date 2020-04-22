#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include "windows.h"

#include "ShaderProgram.h"
#include "Shader.h"

#include <gl/GL.h>		//standard OpenGL include
#include <gl/GLU.h>		//OpenGL utilities
#include "gl/glut.h"
#include "glm/glm.hpp"

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

	//initialize GLUT and create window
	glutInit(&argc, argv);
	//set up display mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("Poseidon");

	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);

	//drawing routine
	//ShaderProgram shaderProgram = ShaderProgram();
	//Shader vertexShader = Shader("", GL_VERTEX_SHADER);

	//callback functions
	glutKeyboardFunc(key);
	glutIdleFunc(idle);
	glutDisplayFunc(draw);

	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	glutMainLoop();
	return 0;
}

void init(void) {
}

void drawSquare(glm::vec3 position, float size){
	
	//draw plane
	glBegin(GL_TRIANGLES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(position.x, position.y, position.z);
	glVertex3f(position.x + size, position.y, position.z);
	glVertex3f(position.x, position.y + size, position.z);
	glEnd();
	glBegin(GL_TRIANGLES);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(position.x + size, position.y, position.z);
	glVertex3f(position.x + size, position.y + size, position.z);
	glVertex3f(position.x, position.y + size, position.z);
	glEnd();
}

void draw(void){

	//clear the current window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//make changes to the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	//initialise the modelview matrix to the identity matrix
	glLoadIdentity();


	drawSquare(glm::vec3(0.0, 0.0, 0.0), 0.5);


	glFlush();
	//swap the back buffer with the front buffer
	glutSwapBuffers();

}

void key(unsigned char k, int x, int y) {
	switch (k) {
	case 27:
		exit(0);
		break;
	}
}

void idle(void){}