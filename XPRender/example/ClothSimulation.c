#include <GL/glut.h>
#include "../lib/xprender/Vec3.h"
#include <stdio.h>

float aspectWidth = 0;
float aspectHeight = 0;

void display(void)
{
	xprVec3 eyeAt = xprVec3_(0, 1, 10);
	xprVec3 lookAt = xprVec3_(0, 0, 0);
	xprVec3 eyeUp = *xprVec3_c010();

	GLenum glerr;

	int r, c;

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, aspectWidth / aspectHeight, 0.1f, 50.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeAt.x, eyeAt.y, eyeAt.z, lookAt.x, lookAt.y, lookAt.z, eyeUp.x, eyeUp.y, eyeUp.z);
	glPushMatrix();

	glEnable(GL_DEPTH_TEST);

	glBegin(GL_POINTS);

	glColor3f(1, 1, 1);
	for(r=-2; r<=2; ++r)
		for(c=-2; c<=2; ++c)
		{
			glVertex3f((float)r, (float)c, 0);
		}

	glEnd();

	glPopMatrix();

	glutSwapBuffers();

	glerr = glGetError();

	if(glerr != GL_NO_ERROR)
	{
		printf("GL has error %d!\n", glerr);
	}
}

void reshape (int w, int h)
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

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

	glutInitWindowSize(500, 500); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow ("ClothSimulation");
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

   return 0;
}
