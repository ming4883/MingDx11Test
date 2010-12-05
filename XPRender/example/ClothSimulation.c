#include <GL/glew.h>
#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Mat44.h"
#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"
#include "Menu.h"

Menu* _menu = nullptr;
Cloth* _cloth = nullptr;
#define BallCount 2
Sphere _ball[BallCount];
Mesh* _ballMesh = nullptr;
Mesh* _floorMesh = nullptr;
XprVec3 _floorN = {0, 1, 0};
XprVec3 _floorP = {0, 0, 0};
float _gravity = 50;
float _airResistance = 5;
float _impact = 3;
xprBool _showDebug = xprFalse;

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

	_menu->windowWidth = w;
	_menu->windowHeight = h;

	glutPostRedisplay();
}

void drawBackground()
{
	static const XprVec3 v[] = {
		{-1,  1, 0},
		{-1, -1, 0},
		{ 1,  1, 0},
		{ 1, -1, 0},
	};

	static const XprVec3 c[] = {
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
	XprVec3 eyeAt = XprVec3_(-2.5f, 1.5f, 5);
	XprVec3 lookAt = XprVec3_(0, 0, 0);
	XprVec3 eyeUp = *XprVec3_c010();
	XprMat44 viewMtx;

	XprMat44_cameraLookAt(&viewMtx, &eyeAt, &lookAt, &eyeUp);
	XprMat44_transpose(&viewMtx, &viewMtx);

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
		Mesh_draw(_floorMesh);
		glPopMatrix();
	}

	// draw _cloth
	{
		float d[] = {1.0f, 0.22f, 0.0f, 1};
		float s[] = {0, 0, 0, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

		glDisable(GL_CULL_FACE);
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1);
		glPushMatrix();
		Mesh_draw(_cloth->mesh);
		glPopMatrix();
		glEnable(GL_CULL_FACE);
		glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0);
	}

	// draw _ball
	{
		int i;
		float d[] = {0.9f, 0.64f, 0.35f, 1};
		float s[] = {1, 1, 1, 1};
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, d);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, s);
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);

		for(i=0; i<BallCount; ++i)
		{
			glPushMatrix();
			glTranslatef(_ball[i].center.x, _ball[i].center.y, _ball[i].center.z);
			glScalef(_ball[i].radius, _ball[i].radius, _ball[i].radius);
			Mesh_draw(_ballMesh);
			glPopMatrix();
		}
	}

	if(xprTrue == _showDebug)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glPushMatrix();

		glColor3f(1, 1, 1);
		Mesh_drawPoints(_cloth->mesh);

		glPopMatrix();
	}
}

void drawItem(float x, float y, int selected, const char* str)
{
	const char *c;

	if(1 == selected)
		glColor4f(0.94f, 0.21f, 0, 1);
	else
		glColor4f(0.3f, 0.52f, 0.64f, 1);

	glRasterPos2f(x, y);

	for(c=str; *c != '\0'; ++c)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
}

void drawMenu()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	Menu_draw(_menu, -1, 1, 5, drawItem);
}

void display(void)
{
	glClearDepth(1);
	glClear(GL_DEPTH_BUFFER_BIT);
	
	drawBackground();
	drawScene();
	drawMenu();

	glutSwapBuffers();

	{	// check for any OpenGL errors
		GLenum glerr = glGetError();

		if(glerr != GL_NO_ERROR)
			printf("GL has error %d!\r", glerr);
	}
}

void keyboard(int key, int x, int y)
{
	switch(key)
	{
	case 27:
		exit(0);
		break;
	case GLUT_KEY_UP:
		Menu_selectPrevItem(_menu);
		break;
	case GLUT_KEY_DOWN:
		Menu_selectNextItem(_menu);
		break;
	case GLUT_KEY_LEFT:
		MenuItem_decreaseValue(_menu->currentItem);
		break;
	case GLUT_KEY_RIGHT:
		MenuItem_increaseValue(_menu->currentItem);
		break;
	case GLUT_KEY_F1:
		_showDebug = !_showDebug;
		break;
	}
}

typedef struct Mouse
{
	int button;
	int state;
	int x;
	int y;
	XprVec3 clothOffsets[2];
} Mouse;

Mouse _mouse;

void mouse(int button, int state, int x, int y)
{
	_mouse.button = button;
	_mouse.state = state;
	_mouse.x = x;
	_mouse.y = y;

	_mouse.clothOffsets[0] = _cloth->fixPos[0];
	_mouse.clothOffsets[1] = _cloth->fixPos[_cloth->segmentCount-1];
}

void motion(int x, int y)
{
	int dx = x - _mouse.x;
	int dy = y - _mouse.y;

	if(_mouse.state == GLUT_DOWN && _mouse.button == GLUT_LEFT_BUTTON)
	{
		float mouseSensitivity = 0.0025f;
		_cloth->fixPos[0].x = _mouse.clothOffsets[0].x + dx * mouseSensitivity;
		_cloth->fixPos[_cloth->segmentCount-1].x = _mouse.clothOffsets[1].x + dx * mouseSensitivity;

		_cloth->fixPos[0].y = _mouse.clothOffsets[0].y + dy * -mouseSensitivity;
		_cloth->fixPos[_cloth->segmentCount-1].y = _mouse.clothOffsets[1].y + dy * -mouseSensitivity;
	}
}

void idle(void)
{
	int iter;
	XprVec3 f;

	static float t = 0;
	t += 0.0005f * _impact;
	
	_ball[0].center.z = cosf(t) * 5.f;
	_ball[1].center.z = sinf(t) * 5.f;

	_cloth->timeStep = 0.01f;	// fixed time step
	_cloth->damping = _airResistance * 1e-3f;

	// perform relaxation
	for(iter = 0; iter < 5; ++iter)
	{
		int i;
		for(i=0; i<BallCount; ++i)
			Cloth_collideWithSphere(_cloth, &_ball[i]);

		Cloth_collideWithPlane(_cloth, &_floorN, &_floorP);
		Cloth_satisfyConstraints(_cloth);
	}
	
	f = XprVec3_(0, -_gravity * _cloth->timeStep, 0);
	Cloth_addForceToAll(_cloth, &f);

	Cloth_verletIntegration(_cloth);

	Cloth_updateMesh(_cloth);

	glutPostRedisplay();
}

void quit(void)
{
	Menu_free(_menu);
	Cloth_free(_cloth);
	Mesh_free(_ballMesh);
	Mesh_free(_floorMesh);
}

int main(int argc, char** argv)
{
	GLenum err;
	XprVec3 offset;

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

	glutInitWindowSize(500, 500); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ClothSimulation");

	if(GLEW_OK != (err = glewInit()))
		printf("failed to initialize GLEW %s\n", glewGetErrorString(err));

	offset = XprVec3_(-1, 1.5f, 0);
	_cloth = Cloth_new(2, 2, &offset, 32);

	_ball[0].center = XprVec3_(-0.5f, 0.5f, 0);
	_ball[0].radius = 0.25f;
	_ball[1].center = XprVec3_(0.5f, 0.5f, 0);
	_ball[1].radius = 0.25f;
	_ballMesh = Mesh_createUnitSphere(32);

	offset = XprVec3_(-2.5f, -2.5f, 0);
	_floorMesh = Mesh_createQuad(5, 5, &offset, 1);

	// set up on screen menu
	{
		MenuItem* item;

		_menu = Menu_new();
	
		item = MenuItem_new(&_gravity);
		Menu_addItem(_menu, item);
		MenuItem_setText(item, "Gravity");
		MenuItem_setBounds(item, 0.0f, 100.0f, 0.5f);

		item = MenuItem_new(&_airResistance);
		Menu_addItem(_menu, item);
		MenuItem_setText(item, "Air Resistance");
		MenuItem_setBounds(item, 0, 20, 1);		
		
		item = MenuItem_new(&_impact);
		Menu_addItem(_menu, item);
		MenuItem_setText(item, "Impact");
		MenuItem_setBounds(item, 0, 10, 1);

		Menu_selectNextItem(_menu);
	}
	
	atexit(quit);
	glutDisplayFunc(display); 
	glutReshapeFunc(reshape);
	//glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle);
	glutMainLoop();

   return 0;
}
