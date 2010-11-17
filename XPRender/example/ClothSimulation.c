#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Mat44.h"
#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"

Cloth* cloth = nullptr;
Sphere ball;
Mesh* ballMesh = nullptr;
Mesh* floorMesh = nullptr;
xprVec3 _floorN = {0, 1, 0};
xprVec3 _floorP = {0, 0, 0};
xprVec3 _force = {0, -10, 0};

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

void drawBackground()
{
	static const xprVec3 v[] = {
		{-1,  1, 0},
		{-1, -1, 0},
		{ 1,  1, 0},
		{ 1, -1, 0},
	};

	static const xprVec3 c[] = {
		{0.57f, 0.85f, 1.0f},
		{0.29f, 0.62f, 0.81f},
		{0.57f, 0.85f, 1.0f},
		{0.57f, 0.85f, 1.0f},
	};

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	glBegin(GL_TRIANGLES);

	glColor3fv(c[0].v); glVertex3fv(v[0].v);
	glColor3fv(c[1].v);	glVertex3fv(v[1].v);
	glColor3fv(c[2].v); glVertex3fv(v[2].v);

	glColor3fv(c[3].v); glVertex3fv(v[3].v);
	glColor3fv(c[2].v); glVertex3fv(v[2].v);
	glColor3fv(c[1].v); glVertex3fv(v[1].v);

	glEnd();
}

void drawScene()
{
	xprVec3 eyeAt = xprVec3_(-1, 1.5f, 5);
	xprVec3 lookAt = xprVec3_(0, 0, 0);
	xprVec3 eyeUp = *xprVec3_c010();
	xprMat44 viewMtx;

	xprMat44_cameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	xprMat44_transpose(&viewMtx, &viewMtx);

	// projection transform
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, _aspect.width / _aspect.height, 0.1f, 30.0f);

	// viewing transform
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(viewMtx.v);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	{
		float p[] = {-10, 10, 6, 1};
		float a[] = {0.2f, 0.2f, 0.2f, 1};
		float d[] = {1, 1, 1, 1};
		float s[] = {2, 2, 2, 1};
		glLightfv(GL_LIGHT0, GL_POSITION, p);
		glLightfv(GL_LIGHT0, GL_AMBIENT, a);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, d);
		glLightfv(GL_LIGHT0, GL_SPECULAR, s);
		glEnable(GL_LIGHT0);
	}

	// draw floor
	{
		float d[] = {1.0f, 0.88f, 0.33f, 1};
		float s[] = {0, 0, 0, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

		glPushMatrix();
		glRotatef(-90, 1, 0, 0);
		Mesh_draw(floorMesh);
		glPopMatrix();
	}

	// draw cloth
	{
		float d[] = {1.0f, 0.22f, 0.0f, 1};
		float s[] = {0, 0, 0, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

		glDisable(GL_CULL_FACE);
		glPushMatrix();
		Mesh_draw(cloth->mesh);
		glPopMatrix();
		glEnable(GL_CULL_FACE);
	}

	// draw ball
	{
		float d[] = {0.9f, 0.64f, 0.35f, 1};
		float s[] = {1, 1, 1, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

		glPushMatrix();
		glTranslatef(ball.center.x, ball.center.y, ball.center.z);
		glScalef(ball.radius, ball.radius, ball.radius);
		Mesh_draw(ballMesh);
		glPopMatrix();
	}
}

void display(void)
{
	glClearDepth(1);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	drawBackground();
	drawScene();

	glutSwapBuffers();

	{	// check for any OpenGL errors
		GLenum glerr = glGetError();

		if(glerr != GL_NO_ERROR)
			printf("GL has error %d!\r", glerr);
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
	xprVec3 clothOffsets[2];
} Mouse;

Mouse _mouse;

void mouse(int button, int state, int x, int y)
{
	_mouse.button = button;
	_mouse.state = state;
	_mouse.x = x;
	_mouse.y = y;

	_mouse.clothOffsets[0] = cloth->fixPos[0];
	_mouse.clothOffsets[1] = cloth->fixPos[cloth->segmentCount-1];
}

void motion(int x, int y)
{
	int dx = x - _mouse.x;
	int dy = y - _mouse.y;

	if(_mouse.state == GLUT_DOWN && _mouse.button == GLUT_LEFT_BUTTON)
	{
		float mouseSensitivity = 0.0025f;
		cloth->fixPos[0].x = _mouse.clothOffsets[0].x + dx * mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].x = _mouse.clothOffsets[1].x + dx * mouseSensitivity;

		cloth->fixPos[0].y = _mouse.clothOffsets[0].y + dy * -mouseSensitivity;
		cloth->fixPos[cloth->segmentCount-1].y = _mouse.clothOffsets[1].y + dy * -mouseSensitivity;
	}
}

void idle(void)
{
	int iter;
	xprVec3 f;

	ball.center.z = cosf(glutGet(GLUT_ELAPSED_TIME) * 0.0005f) * 5.f;

	cloth->timeStep = 0.01f;	// fixed time step
	cloth->dumping = 5e-3f;

	xprVec3_multS(&f, &_force, cloth->timeStep);

	// Euler iteration
	for(iter = 0; iter < 5; ++iter)
	{
		Cloth_collideWithSphere(cloth, &ball);
		Cloth_collideWithPlane(cloth, _floorN.v, _floorP.v);
		Cloth_satisfyConstraints(cloth);
	}
	
	Cloth_addForceToAll(cloth, &f);
	Cloth_verletIntegration(cloth);

	Cloth_updateMesh(cloth);

	glutPostRedisplay();
}

void quit(void)
{
	Cloth_free(cloth);
	Mesh_free(ballMesh);
	Mesh_free(floorMesh);
}

int main(int argc, char** argv)
{
	GLenum err;

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

	glutInitWindowSize(500, 500); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ClothSimulation");

	if(GLEW_OK != (err = glewInit()))
		printf("failed to initialize GLEW %s\n", glewGetErrorString(err));

	cloth = Cloth_new(2, 2, xprVec3_(-1, 1.5, 0).v, 32);

	ball.center = xprVec3_(0, 0.5, 0);
	ball.radius = 0.25f;
	ballMesh = Mesh_createUnitSphere(32);

	floorMesh = Mesh_createQuad(5, 5, xprVec3_(-2.5f, -2.5f, 0).v, 1);
	
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
