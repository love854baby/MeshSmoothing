// Credit to www.eecs.oregonstate.edu/capstone/opengl/sample.cpp
// for help with smooth mouse scaling.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h> // (or others, depending on the system in use)
#include <iostream>
#include <unordered_set>
using namespace std;

const float ANGFACT = { 1. };
const float MINSCALE = { 0.00f };
const float SCLFACT = { 0.010f };
const float TFACT = { 0.30f };
bool fullscreen = false;
bool mouseDown = false;
int mouseMode = 0;
int	transformation = 0;
static int menuExit;
int drawType, mousex, mousey = 0;
int win = 0;

void draw();
void menu(int);

// transformation variables
float xrot = 0.0f;
float yrot = 0.0f;

float scale = 1.0f;
float tx = 0.0f;
float ty = 0.0f;

float xdiff = 0.0f;
float ydiff = 0.0f;

unordered_set<int> *neighbor;
const float lambda = .5;
const float mu = -.4;

enum transformation
{
	ROTATE,
	SCALE,
	TRANSLATE
};

struct FLTVECT {
	float x;
	float y;
	float z;

	FLTVECT() 
	{
		x = y = z = 0;
	}

	FLTVECT(const float a, const float b, const float c)
	{
		x = a;
		y = b;
		z = c;
	}

	FLTVECT operator+(const FLTVECT v) 
	{
		return { v.x + x, v.y + y, v.z + z };
	}

	FLTVECT operator-(const FLTVECT v)
	{
		return{ x - v.x, y - v.y, z - v.z };
	}

	FLTVECT operator/(const float s)
	{
		return{ x / s, y / s, z / s };
	}

	FLTVECT operator*(const float s)
	{
		return{ x * s, y * s, z * s };
	}
};

struct INT3VECT {
	int a;
	int b;
	int c;
};

struct SurFacemesh {
	int nv;
	int nf;
	FLTVECT *vertex;
	INT3VECT *face;
};


void createMenu(void) {
	//////////
	// MENU //
	//////////

	// Create an entry
	menuExit = glutCreateMenu(menu);
	glutAddMenuEntry("Point", 1);
	glutAddMenuEntry("Fill", 2);
	glutAddMenuEntry("Line", 3);
	glutAddMenuEntry("Both", 4);
	glutAddMenuEntry("Exit Program", 0);

	// Let the menu respond on the right mouse button
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void menu(int value) {
	if (value == 0) {
		glutDestroyWindow(win);
		exit(0);
	}
	else {
		drawType = value;
	}

	// you would want to redraw now
	glutPostRedisplay();
}

SurFacemesh* readPolygon()
{
	int num, n, m;
	int a, b, c, d, e;
	float x, y, z;
	SurFacemesh *surfmesh;
	char line[256];
	FILE *fin;

	if ((fin = fopen("2CMP_noise.off", "r")) == NULL) {
		printf("read error...\n");
		exit(0);
	};

	/* OFF format */
	while (fgets(line, 256, fin) != NULL) {
		if (line[0] == 'O' && line[1] == 'F' && line[2] == 'F')
			break;
	}
	fscanf(fin, "%d %d %d\n", &m, &n, &num);

	surfmesh = (SurFacemesh*)malloc(sizeof(SurFacemesh));
	surfmesh->nv = m;
	surfmesh->nf = n;
	surfmesh->vertex = (FLTVECT *)malloc(sizeof(FLTVECT)*surfmesh->nv);
	surfmesh->face = (INT3VECT *)malloc(sizeof(INT3VECT)*surfmesh->nf);

	neighbor = new unordered_set<int>[m];

	for (n = 0; n < surfmesh->nv; n++) {
		fscanf(fin, "%f %f %f\n", &x, &y, &z);
		surfmesh->vertex[n].x = x;
		surfmesh->vertex[n].y = y;
		surfmesh->vertex[n].z = z;
	}

	for (n = 0; n < surfmesh->nf; n++) {
		fscanf(fin, "%d %d %d %d\n", &a, &b, &c, &d);
		surfmesh->face[n].a = b;
		surfmesh->face[n].b = c;
		surfmesh->face[n].c = d;

		if (a != 3)
			printf("Errors: reading surfmesh .... \n");
		
		neighbor[b].insert(c);
		neighbor[b].insert(d);
		neighbor[c].insert(b);
		neighbor[c].insert(d);
		neighbor[d].insert(b);
		neighbor[d].insert(c);
	}
	fclose(fin);

	return surfmesh;
}

// Surface mesh obtained from .off file
SurFacemesh* surfmesh = readPolygon();

void draw()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// change mode depending on menu selection
	switch (drawType)
	{
	case 1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case 2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 3:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}

	glColor3f(0.5f, 0.0f, 1.0f);
	for (int i = 0; i < surfmesh->nf; ++i) {
		glBegin(GL_POLYGON);
		glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x, surfmesh->vertex[surfmesh->face[i].a].y, surfmesh->vertex[surfmesh->face[i].a].z);
		glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x, surfmesh->vertex[surfmesh->face[i].b].y, surfmesh->vertex[surfmesh->face[i].b].z);
		glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x, surfmesh->vertex[surfmesh->face[i].c].y, surfmesh->vertex[surfmesh->face[i].c].z);
		glEnd();
	}

	// Both FILL and LINE 
	if (drawType == 4) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0, 1.0);
		for (int i = 0; i < surfmesh->nf; ++i) {
			glBegin(GL_POLYGON);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].a].x, surfmesh->vertex[surfmesh->face[i].a].y, surfmesh->vertex[surfmesh->face[i].a].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].b].x, surfmesh->vertex[surfmesh->face[i].b].y, surfmesh->vertex[surfmesh->face[i].b].z);
			glVertex3f(surfmesh->vertex[surfmesh->face[i].c].x, surfmesh->vertex[surfmesh->face[i].c].y, surfmesh->vertex[surfmesh->face[i].c].z);
			glEnd();
		}
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}

bool init()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0f);

	return true;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// Change eye position for translation
	gluLookAt(
		tx, ty, 100.0f,
		tx, ty, 0.0f,
		0.0f, 1.0f, 0.0f);

	// Rotate with rotation vars
	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

	// Scale with scale variables
	glScalef(scale, scale, scale);


	glPushMatrix();
	draw();
	glPopMatrix();

	glFlush();
	glutSwapBuffers();
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f, 1.0f * w / h, 1.0f, -100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle()
{
	if (!mouseDown)
	{
		xrot += 0.3f;
		yrot += 0.4f;
	}

	glutPostRedisplay();
}

// shrk = true, shrink operation
// shrk = false, inflate operation
void shrink(bool shrk) 
{
	int n = surfmesh->nv;
	FLTVECT *newlist = new FLTVECT[n];
	FLTVECT vec;

	FLTVECT *oldlist = surfmesh->vertex;
	for (int i = 0; i < n; i++)
	{
		for (auto elem = neighbor[i].begin(); elem != neighbor[i].end(); ++elem)
		{
			vec = vec + oldlist[*elem];
		}

		vec = vec / neighbor[i].size() - oldlist[i];

		if(shrk)
			newlist[i] = vec * lambda + oldlist[i];
		else
			newlist[i] = vec * mu + oldlist[i];
	}

	free(oldlist);
	oldlist = NULL;
	surfmesh->vertex = newlist;
	newlist = NULL;
}

void smooth(int times) 
{
	for (int k = 0; k < times; k++)
	{
		shrink(true);
		shrink(false);
	}

	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'r':
		transformation = ROTATE;
		break;
	case 'R':
		transformation = ROTATE;
		break;
	case 's':
		transformation = SCALE;
		break;
	case 'S':
		transformation = SCALE;
		break;
	case 't':
		transformation = TRANSLATE;
		break;
	case 'T':
		transformation = TRANSLATE;
		break;
	case 'x':
		smooth(5);
		break;
	case 'X':
		smooth(5);
		break;
	case 27:
		exit(1);
		break;
	}
}

void specialKeyboard(int key, int x, int y)
{
	if (key == GLUT_KEY_F1)
	{
		fullscreen = !fullscreen;

		if (fullscreen)
			glutFullScreen();
		else
		{
			glutReshapeWindow(500, 500);
			glutPositionWindow(50, 50);
		}
	}
}

void mouse(int button, int state, int x, int y)
{

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = true;
		mousex = x;
		mousey = y;
	}

	else
		mouseDown = false;
}

void mouseMotion(int x, int y)
{

	if (mouseDown)
	{
		int dx = x - mousex;
		int dy = y - mousey;

		switch (transformation)
		{
		case ROTATE:
			xrot += (ANGFACT*dy);
			yrot += (ANGFACT*dx);
			break;
		case SCALE:
			scale += SCLFACT * (float)(dx - dy);
			if (scale < MINSCALE)
				scale = MINSCALE;
			break;
		case TRANSLATE:
			tx -= dx * TFACT;
			ty += dy * TFACT;
			break;
		}

		mousex = x;
		mousey = y;
		glutPostRedisplay();
	}


}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	win = glutCreateWindow("3D Polygon");
	createMenu();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);
	//glutIdleFunc(idle);

	if (!init())
		return 1;

	glutMainLoop();

	return 0;
}