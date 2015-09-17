#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <glut.h>
#include <assert.h>
#include "Stroke.h"
#include "noise.h"

#include "GL/glut.h"

//#include "polyfill.h"

Stroke curve;

int windowWidth = 512, windowHeight = 512;

bool opaque = true;
bool controlPolygon = true;

GLenum checkForError(char *loc);

GLubyte * texture;
int texWidth,texHeight;


void Redraw(void)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //    curve.drawTexture();

    if (opaque)
      glColor3ub(0, 64, 16);
    else
      glColor4ub(0, 64, 16, 128);
    curve.render();

    if (controlPolygon)
      {
	glColor3ub(255,0,0);
	curve.drawControl();
      }

    glutSwapBuffers();
    
    checkForError("swap");
}

void Button(int button, int state, int x, int y)
{
  if (state != GLUT_UP)	
    return;

  switch (button) 
    {
    case GLUT_LEFT_BUTTON:
      curve.add(x,windowHeight-y);
      glutPostRedisplay();
      break;

    case GLUT_RIGHT_BUTTON:
      curve.clear();
      glutPostRedisplay();
      break;
    }
}


void Reshape(int width, int height)
{
  windowWidth = width; windowHeight = height;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, (GLfloat) windowWidth, 0.0f, 
	  (GLfloat) windowHeight, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, windowWidth, windowHeight);
}
void menuSelect(int value)
{
  switch(value)
    {
    case 0:
      curve.curveType = CUBIC_BSPLINE;
      curve.forceRecompute();
      glutPostRedisplay();
      break;

    case 1:
      curve.curveType = FOUR_POINT;
      curve.forceRecompute();
      glutPostRedisplay();
      break;

    case 2:
      controlPolygon = !controlPolygon;
      glutPostRedisplay();
      break;
      
    case 3:
      opaque = !opaque;
      glutPostRedisplay();
      break;

    case 4:
      curve.radius += 5;
      glutPostRedisplay();
      break;

    case 5:
      curve.radius -= 5;
      if (curve.radius < 1)
	curve.radius = 1;
      glutPostRedisplay();
      break;

    case 8:
      exit(0);

    //////////////////////// Tappering ///////////////////////////
    case 6:
      curve.tapperBrush = !curve.tapperBrush;
      glutPostRedisplay();
      break;
    //////////////////////// Tappering ///////////////////////////

    case 7:
      curve.useTexture = !curve.useTexture;
      glutPostRedisplay();
      break;
    }
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  //  glutInitDisplayMode(GLUT_RGB|GLUT_DEPTH);
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(windowWidth, windowHeight);
  glutCreateWindow("Square");
  
  // set up world space to screen mapping
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0f, (GLfloat) windowWidth, 0.0f, 
	  (GLfloat) windowHeight, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, windowWidth, windowHeight);
  
  glClearColor(1.0,.95,.85,0);
  glutDisplayFunc(Redraw);
  glutReshapeFunc(Reshape);
  glutMouseFunc(Button);
  glEnable(GL_BLEND);	
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  curve . add(100,100);
  curve . add(200,150);
  curve . add(300,100);
  curve . add(400,300);
  curve . add(100,100);

  curve.radius = 10;
  curve.curveType = CUBIC_BSPLINE;
  //  curve.curveType = FOUR_POINT;


// make a texture map
  texWidth = texHeight = 512;

  texture = new GLubyte[4*texWidth*texHeight];

  for(int x=0;x<texWidth;x++)
    for(int y=0;y<texHeight;y++)
      {
	float n = noise2(x/100.,y/100.);
	GLubyte v = n*255;
	
	//	printf("%f(%d) ",n,v);

	texture[4*(x + y*texWidth)] = v;
	texture[4*(x + y*texWidth)+1] = v;
	texture[4*(x + y*texWidth)+2] = v;
	texture[4*(x + y*texWidth)+3] = v;
      }

// initialize texturing
	  glTexImage2D(GL_TEXTURE_2D,0,4,texHeight,texWidth,0,GL_RGBA,
		       GL_UNSIGNED_BYTE,texture);

	  //    glTexImage2D(GL_TEXTURE_2D,0,1,texHeight,texWidth,0,GL_LUMINANCE,
	  //  	       GL_UNSIGNED_BYTE,texture);

	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	  // filtering 
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);

	  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


  
  glutCreateMenu(menuSelect);
  glutAddMenuEntry("Cubic B-Spline",0);
  glutAddMenuEntry("Four Point",1);
  glutAddMenuEntry("Control Polygon",2);
  glutAddMenuEntry("Opaque/Transparent",3);
  glutAddMenuEntry("Thicker",4);
  glutAddMenuEntry("Thinner",5);
  glutAddMenuEntry("Toggle Tappering",6);
  glutAddMenuEntry("Toggle Texture",7);
  glutAddMenuEntry("Quit",8);
  glutAttachMenu(GLUT_MIDDLE_BUTTON);
  
  

  glutMainLoop();

return 0;
}

GLenum checkForError(char *loc)
{
  GLenum errCode;
  const GLubyte *errString;

  if ((errCode = glGetError()) != GL_NO_ERROR)
    {
      errString = gluErrorString(errCode);
      printf("OpenGL error: %s",errString);

      if (loc != NULL)
	printf("(%s)",loc);

      printf("\n");
    }

  return errCode;
}

