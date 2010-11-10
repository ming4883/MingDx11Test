#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "Cloth.h"

Cloth* cloth = nullptr;

typedef struct Aspect
{
	float width;
	float height;
} Aspect;

Aspect _aspect;

void reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	_aspect.width = (float)w;
	_aspect.height = (float)h;

	glutPostRedisplay();
}

void display(void)
{
	xprVec3 eyeAt = xprVec3_(0, 2, 6);
	xprVec3 lookAt = xprVec3_(0, 0, 0);
	xprVec3 eyeUp = *xprVec3_c010();

	GLenum glerr;

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, _aspect.width / _aspect.height, 0.1f, 20.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeAt.x, eyeAt.y, eyeAt.z, lookAt.x, lookAt.y, lookAt.z, eyeUp.x, eyeUp.y, eyeUp.z);
	glPushMatrix();

	//glEnable(GL_DEPTH_TEST);

	Cloth_draw(cloth);

	glPopMatrix();

	glutSwapBuffers();

	glerr = glGetError();

	if(glerr != GL_NO_ERROR)
	{
		printf("GL has error %d!\n", glerr);
	}
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	}
}

typedef struct Mouse
{
	int button;
	int state;
	int x;
	int y;
} Mouse;

Mouse _mouse;

xprVec3 clothOffsets[2];

void mouse(int button, int state, int x, int y)
{
	_mouse.button = button;
	_mouse.state = state;
	_mouse.x = x;
	_mouse.y = y;

	clothOffsets[0] = cloth->fixPos[0];
	clothOffsets[1] = cloth->fixPos[cloth->segmentCount-1];
}

void motion(int x, int y)
{
	int dx = x - _mouse.x;
	int dy = y - _mouse.y;

	if(_mouse.state == GLUT_DOWN && _mouse.button == GLUT_LEFT_BUTTON)
	{
		float mouseSensitivity = 0.005f;
		cloth->fixPos[0].x = clothOffsets[0].x + dx * mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].x = clothOffsets[1].x + dx * mouseSensitivity;

		cloth->fixPos[0].z = clothOffsets[0].z + dy * mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].z = clothOffsets[1].z + dy * mouseSensitivity;
	}
}

void idle(void)
{
	static int lastTime = 0;

	xprVec3 force = {0, -10, 0};
	int currTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currTime - lastTime;
	lastTime = currTime;

	cloth->timeStep = 0.01f;	// fixed time step
	cloth->dumping = 1e-3f;

	xprVec3_MultSTo(&force, cloth->timeStep);

	Cloth_addForceToAll(cloth, &force);

	Cloth_timeStep(cloth);

	glutPostRedisplay();
}

void quit(void)
{
	Cloth_free(cloth);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

	glutInitWindowSize(500, 500); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ClothSimulation");

	glewInit();

	cloth = Cloth_new(2, 2, 16);
	
	atexit(quit);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle);
	glutMainLoop();

   return 0;
}
