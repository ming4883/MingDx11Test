#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "Cloth.h"

float aspectWidth = 0;
float aspectHeight = 0;
Cloth* cloth = nullptr;

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
	gluPerspective(45.0f, aspectWidth / aspectHeight, 0.1f, 20.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeAt.x, eyeAt.y, eyeAt.z, lookAt.x, lookAt.y, lookAt.z, eyeUp.x, eyeUp.y, eyeUp.z);
	glPushMatrix();

	glEnable(GL_DEPTH_TEST);

	Cloth_draw(cloth);

	glPopMatrix();

	glutSwapBuffers();

	glerr = glGetError();

	if(glerr != GL_NO_ERROR)
	{
		printf("GL has error %d!\n", glerr);
	}
}

void reshape(int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	aspectWidth = (float)w;
	aspectHeight = (float)h;

	glutPostRedisplay();
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

void idle(void)
{
	static int lastTime = 0;

	xprVec3 force = {0, -1, 0};
	int currTime = glutGet(GLUT_ELAPSED_TIME);
	int deltaTime = currTime - lastTime;
	lastTime = currTime;

	cloth->timeStep = 0.01f;	// fixed time step

	cloth->fixPos[0].z = cosf(currTime * 1e-3f);

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

	cloth = Cloth_new(2, 2, 16);
	cloth->dumping = 1e-5f;

	atexit(quit);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutMainLoop();

   return 0;
}
