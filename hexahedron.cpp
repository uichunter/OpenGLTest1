
#ifdef _MSC_VER  
#pragma comment( lib, "cg.lib" )  
#pragma comment( lib, "cgGL.lib" )  
#endif  

#include <math.h>  
#include <stdio.h>  
#include <stdlib.h>  

#ifdef __APPLE__  
#include <GLUT/glut.h>  
#else  
#define GLUT_DISABLE_ATEXIT_HACK  
#include <GL/glut.h>  
#endif  

#include <Cg/cg.h>  
#include <Cg/cgGL.h>  
																				 /*** Static Data ***/
																				 /* New Cg global variables */
static CGcontext Context = NULL;
static CGprogram VertexProgram = NULL;
static CGparameter KdParam = NULL;
static CGparameter ModelViewProjParam = NULL;
static CGparameter VertexColorParam = NULL;

#ifdef __APPLE__  
static CGprofile VertexProfile = CG_PROFILE_ARBVP1;
#else  
static CGprofile VertexProfile = CG_PROFILE_VP20;
#endif  
/* End new Cg global variables */

GLfloat CubeNormals[6][3] =//法向量  
{
	{ -1.0, 0.0, 0.0 },{ 0.0, 1.0, 0.0 },{ 1.0, 0.0, 0.0 },
	{ 0.0, -1.0, 0.0 },{ 0.0, 0.0, 1.0 },{ 0.0, 0.0, -1.0 }
};
GLint CubeFaces[6][4] =//六个面的顶点的索引  
{
	{ 0, 1, 2, 3 },{ 3, 2, 6, 7 },{ 7, 6, 5, 4 },
	{ 4, 5, 1, 0 },{ 5, 6, 2, 1 },{ 7, 4, 0, 3 }
};
GLfloat CubeVertices[8][3];//顶点坐标  

static void DrawCube(void)//绘制立方体函数  
{
	int i;
	cgGLBindProgram(VertexProgram);//bind a program to the current state  
								   /* Set various uniform parameters including the ModelViewProjection
								   * matrix for transforming the incoming position into HPOS.*/
	if (KdParam != NULL)
		cgGLSetParameter4f(KdParam, 1.0, 1.0, 0.0, 1.0);
	/* Set the concatenate modelview and projection matrices */
	if (ModelViewProjParam != NULL)
		cgGLSetStateMatrixParameter(ModelViewProjParam,
			CG_GL_MODELVIEW_PROJECTION_MATRIX,
			CG_GL_MATRIX_IDENTITY);
	cgGLEnableProfile(VertexProfile);
	/*
	* Create cube with per-vertex varying attributes
	*/
	for (i = 0; i < 6; i++)
	{
		glBegin(GL_QUADS);
		{
			glNormal3fv(&CubeNormals[i][0]);//设置当前法向量  
			cgGLSetParameter3f(VertexColorParam, 1.0, 0.0, 0.0);
			glVertex3fv(&CubeVertices[CubeFaces[i][0]][0]);
			cgGLSetParameter3f(VertexColorParam, 0.0, 1.0, 0.0);
			glVertex3fv(&CubeVertices[CubeFaces[i][1]][0]);
			cgGLSetParameter3f(VertexColorParam, 0.0, 0.0, 1.0);
			glVertex3fv(&CubeVertices[CubeFaces[i][2]][0]);
			cgGLSetParameter3f(VertexColorParam, 1.0, 1.0, 1.0);
			glVertex3fv(&CubeVertices[CubeFaces[i][3]][0]);
		}
		glEnd();
	}
	cgGLDisableProfile(VertexProfile);
}
static void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawCube();
	glutSwapBuffers();
}
static void InitializeCube(GLfloat v[8][3])//顶点坐标初始化  
{
	/* Setup cube vertex data. */
	v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
	v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
	v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
	v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
	v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
	v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;
}
static void InitializeGlut(int *argc, char *argv[])
{
	glutInit(argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(Display);
	InitializeCube(CubeVertices);
	/* Use depth buffering for hidden surface elimination. */
	glEnable(GL_DEPTH_TEST);
	/* Setup the view of the cube. */
	glMatrixMode(GL_PROJECTION);
	gluPerspective( /* field of view in degree */ 40.0,
		/* aspect ratio */ 1.0,
		/* Z near */ 1.0, /* Z far */ 10.0);
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 5.0, /* eye is at (0,0,5) */
		0.0, 0.0, 0.0, /* center is at (0,0,0) */
		0.0, 1.0, 0.); /* up is in positive Y direction */
					   /* Adjust cube position to be asthetic angle. */
	glTranslatef(0.0, 0.0, -1.0);

#if 1  
	glRotatef(60, 1.0, 0.0, 0.0);
	glRotatef(-20, 0.0, 0.0, 1.0);
#endif  
}
int main(int argc, char *argv[])
{
	InitializeGlut(&argc, argv);
	Context = cgCreateContext();/* Create one context which all shaders will use */
	VertexProgram = cgCreateProgramFromFile(Context,
		CG_SOURCE,
		"vertexShader.cg",
		VertexProfile,
		NULL,
		NULL);/* Adds shader to the context */
	if (VertexProgram != NULL)
	{
		cgGLLoadProgram(VertexProgram);
		KdParam = cgGetNamedParameter(VertexProgram, "Kd");
		ModelViewProjParam = cgGetNamedParameter(VertexProgram, "ModelViewProj");
		VertexColorParam = cgGetNamedParameter(VertexProgram, "IN.VertexColor");
	}
	glutMainLoop();
	cgDestroyProgram(VertexProgram);
	cgDestroyContext(Context);
	return 0;
}