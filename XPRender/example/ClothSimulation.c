#include <GL/glew.h>
#include <GL/glut.h>
#include <glsw.h>
#include <stdio.h>
#include <math.h>

#include "../lib/xprender/Vec3.h"
#include "../lib/xprender/Shader.h"
#include "Cloth.h"
#include "Sphere.h"
#include "Mesh.h"

Cloth* cloth = nullptr;
xprShader* objectVp = nullptr;
xprShader* objectFp = nullptr;
int objProgName = 0;
Sphere ball;
Mesh* ballMesh = nullptr;

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

	glEnable(GL_DEPTH_TEST);

	glUseProgram(objProgName);
	Mesh_draw(cloth->mesh);

	glPushMatrix();
	glTranslatef(ball.center.x, ball.center.y, ball.center.z);
	glScalef(ball.radius, ball.radius, ball.radius);
	Mesh_draw(ballMesh);
	glPopMatrix();

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

	ball.center.z = cosf(currTime * 0.0005f) * 5.f;

	cloth->timeStep = 0.01f;	// fixed time step
	cloth->dumping = 5e-3f;

	xprVec3_multS(&force, &force, cloth->timeStep);

	Cloth_addForceToAll(cloth, &force);

	Cloth_timeStep(cloth);

	Cloth_collideWithSphere(cloth, &ball);

	glutPostRedisplay();
}

void quit(void)
{
	glDeleteProgram(objProgName);
	xprShader_free(objectVp);
	xprShader_free(objectFp);
	Cloth_free(cloth);
	Mesh_free(ballMesh);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

	glutInitWindowSize(500, 500); 
	glutInitWindowPosition(100, 100);
	glutCreateWindow("ClothSimulation");

	glewInit();

	{
		const char* code;
		
		glswInit();
		glswSetPath("../example/", ".glsl");

		code = glswGetShader("ClothSimulation.Vertex");
		if(nullptr != code)
			objectVp = xprShader_new(&code, 1, xprShaderType_Vertex);
		else
			printf(glswGetError());

		code = glswGetShader("ClothSimulation.Fragment");
		if(nullptr != code)
			objectFp = xprShader_new(&code, 1, xprShaderType_Fragment);
		else
			printf(glswGetError());

		glswShutdown();

		objProgName = glCreateProgram();
		glAttachShader(objProgName, objectVp->name);
		glAttachShader(objProgName, objectFp->name);
		glLinkProgram(objProgName);

		{
			int linkStatus;
			glGetProgramiv(objProgName, GL_LINK_STATUS, &linkStatus);
			if(GL_FALSE == linkStatus)
			{
				GLint len;
				glGetProgramiv(objProgName, GL_INFO_LOG_LENGTH, &len);
				if(len > 0)
				{
					char* buf = (char*)malloc(len);
					glGetProgramInfoLog(objProgName, len, nullptr, buf);
					printf(buf);
					free(buf);
				}
			}
		}
	}

	{
		xprVec3 offset = {-1, 1, 0};
		cloth = Cloth_new(2, 2, &offset, 32);
	}

	ball.center = xprVec3_(0, 0, 0);
	ball.radius = 0.5f;
	ballMesh = Mesh_createUnitSphere(16);
	
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
