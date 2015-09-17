#include <stdio.h>
#include <stdlib.h>
#include "Stroke.h"

#ifdef SCAN_CONVERT
#include "polyfill.h"
#endif

#ifndef SCAN_CONVERT
GLUquadricObj * Stroke::qobj = NULL;
#endif

#define M_PI 3.1415926

Stroke::Stroke()
{
  limit = temp = NULL;
  computed = GL_FALSE;
  curveType = CUBIC_BSPLINE;

  useTexture = true;
  tapperBrush = false;

  numLevels = 3;

  z = 0;

#ifndef SCAN_CONVERT
  if (qobj == NULL)
    {
      qobj = gluNewQuadric();
    }
#endif

  ufreq = .005;
  vfreq = .1;
  ustart = .1;
  vstart =  (double(rand()) / RAND_MAX);
}

	
Stroke::~Stroke()
{
  if (limit != NULL)
    delete limit;

  if (temp != NULL)
    delete temp;
}

void Stroke::add(float x,float y)
{
  if (control.empty() || (control.back().x != x && control.back().y != y))
    {
      control.push_back(Point(x,y));
      computed = GL_FALSE;
    }
}

void Stroke::clear()
{
  computed = GL_FALSE;
  control.erase(control.begin(),control.end());
  limit->erase(control.begin(),control.end());
  ustart = .1;
  vstart =  (double(rand()) / RAND_MAX);
}

void Stroke::drawLines(vector<Point> *curve)
{
  glDisable(GL_TEXTURE_2D);
  glBegin(GL_LINE_STRIP);
  for(int i=0;i<curve->size();i++)
    {
      Point & p = (*curve)[i];
      glVertex2i(p.x,p.y);
    }
  glEnd();
}

void Stroke::forceRecompute()
{
  computed = false;

}

void Stroke::discPoint(float x,float y,float brushRadius)
{
#ifdef SCAN_CONVERT
  // really ought to implement scan-conversion of the circle here
  
  glColor3ub(0,0,255);
  drawCap(Point(x,y),1,0,0,0);
  drawCap(Point(x,y),-1,0,0,0);
#else
  glPushMatrix();
  glTranslatef(x,y,z);
  gluDisk(qobj,0,brushRadius,NUM_SLICES,1);
  glPopMatrix();
#endif
}

void Stroke::drawCap(const Point & p0, float dx, float dy,
		     float texU,float texV)
{
#ifdef SCAN_CONVERT
  glDisable(GL_TEXTURE_2D);

  float theta = atan2(-dy,-dx);

  Point p1(p0.x - radius * dx, p0.y - radius*dy);

  for(int i=1;i<NUM_SLICES-1;i++)
    {
      float dx1 = cos(theta + i * M_PI / NUM_SLICES);
      float dy1 = sin(theta + i * M_PI / NUM_SLICES);

      Point p2(p0.x + radius * dx1, p0.y + radius*dy1);

      drawTriangle(p0,p1,p2);

      p1 = p2;
    }

  drawTriangle(p0,p1,Point(p0.x+radius*dx,p0.y+radius*dy));
#else
  float theta = atan2(-dy,-dx);

  glPushMatrix();
  glTranslatef(p0.x,p0.y,z);

  glBegin(GL_TRIANGLE_FAN);
  //        glVertex3f(p0->x, p0->y,z);
  glTexCoord2f(texU,texV);
  glVertex3i(0,0,0);


  glTexCoord2f(texU+ufreq*radius * dx,texV+vfreq*dy);
  glVertex3f(-radius*dx,-radius*dy,0);

  for(int i=1;i<=NUM_SLICES-1;i++)
    {
      float dx1 = cos(theta + i * M_PI / NUM_SLICES);
      float dy1 = sin(theta + i * M_PI / NUM_SLICES);

      // these 
      glTexCoord2f(texU-ufreq*radius * dx1,texV-vfreq*dy1);

      glVertex3i(radius * dx1, radius * dy1, 0);
      //glVertex3f(p0->x + radius * dx1, p0->y + radius * dy1, z);
    }

  glTexCoord2f(texU-ufreq*radius * dx,texV-vfreq*dy);
  glVertex3f(radius*dx,radius*dy,0);

  glEnd();

  glPopMatrix();
#endif
}

void Stroke::drawThickCurve(vector<Point> * curve, float radius,bool cap)
{

  float radius1;
  cap &= !tapperBrush;

  if (useTexture)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);

  int i=0;
  float dx,dy,mag;
  Point p0;
  Point p1;
  Point p2;	

  if (curve->empty())
    return;

  p0 = (*curve)[0];

  if (curve->size() == 1)
    {
      if (cap) discPoint(p0.x,p0.y,radius);
      return;
    }

  p1 = (*curve)[1];

  dx = p1.y - p0.y;
  dy = p0.x - p1.x;

  mag = sqrt(dx*dx + dy*dy);

  dx /= mag;
  dy /= mag;

  float textureU = ustart;
  float textureV = vstart;

  glColor3ub(0,0,255);

  if (cap)
    drawCap(p0, dx, dy,textureU,textureV);

#ifdef SCAN_CONVERT
  glDisable(GL_TEXTURE_2D);

  Point v0(p0.x + radius * dx, p0.y + radius * dy);
  Point v1(p0.x - radius * dx, p0.y - radius * dy);

#else
  glBegin(GL_TRIANGLE_STRIP);


  //////////////////////// Tappering ///////////////////////////
  radius1 = tapperBrush ? 0 : radius; 
  //////////////////////// Tappering ///////////////////////////

  glTexCoord2f(textureU,textureV + vfreq);
  glVertex3f(p0.x + radius1 * dx, p0.y + radius1 * dy,z);
  
  glTexCoord2f(textureU,textureV - vfreq);
  glVertex3f(p0.x - radius1 * dx, p0.y - radius1 * dy,z);
#endif

  textureU += ufreq;

#ifdef SCAN_CONVERT
  // draw the patch in between with extra subdivision to match the caps,
  // to prevent holes from appearing in the stroke
  if (curve->size() >= 2)
    {
      float dist = sqrt((p1.x-p0.x)*(p1.x-p0.x)+(p1.y-p0.y)*(p1.y-p0.y));
      textureU += ufreq * dist;

      v0 = Point(p0.x + radius * dx, p0.y + radius * dy);
      v1 = Point(p0.x - radius * dx, p0.y - radius * dy);
      Point v2(p1.x + radius * dx, p1.y + radius * dy);
      Point v3(p1.x - radius * dx, p1.y - radius * dy);
	  
      glColor3ub(255,0,0);
      //      drawTriangle(v2,v3,v0);
      drawTriangle(v2,p1,v0);
      glColor3ub(255,255,0);
      drawTriangle(p1,v3,v0);

      glColor3ub(0,255,0);
      //      drawTriangle(v0,v1,v3);
      drawTriangle(v0,p0,v3);
      glColor3ub(0,255,255);
      drawTriangle(p0,v1,v3);

      glColor3ub(0,0,255);

      if (curve->size() == 2)
	{
	  drawCap(p1,-dx,-dy,textureU,textureV);
	  return;
	}


      v0 = v2;
      v1 = v3;
    }
#endif

#ifdef SCAN_CONVERT
  for(i=2;i<curve->size()-1;i++)
#else
  for(i=1;i<curve->size()-1;i++)
#endif
    {
      p0 = (*curve)[i-1];
      p1 = (*curve)[i];
      p2 = (*curve)[i+1];

      dx = p2.y - p0.y;
      dy = p0.x - p2.x;

      mag = sqrt(dx*dx + dy*dy);

      dx /= mag;
      dy /= mag;

      float dist = sqrt((p1.x-p0.x)*(p1.x-p0.x)+(p1.y-p0.y)*(p1.y-p0.y));
      textureU += ufreq * dist;

#ifdef SCAN_CONVERT
      Point v2(p1.x + radius * dx, p1.y + radius * dy);
      Point v3(p1.x - radius * dx, p1.y - radius * dy);

      glColor3ub(255,0,0);
      drawTriangle(v2,v3,v0);

      glColor3ub(0,255,0);
      drawTriangle(v0,v1,v3);
	  
      v0 = v2;
      v1 = v3;
#else

      //////////////////////// Tappering ///////////////////////////
      radius1 = tapperBrush ? tappering(radius, i, curve->size()) : radius; 

      glTexCoord2f(textureU,textureV + vfreq);
      glVertex3f(p1.x + radius1 * dx, p1.y + radius1 * dy,z);
	  
      glTexCoord2f(textureU,textureV - vfreq);
      glVertex3f(p1.x - radius1 * dx, p1.y - radius1 * dy,z);
      //////////////////////// Tappering ///////////////////////////

#endif

    }

  p0 = (*curve)[curve->size()-2];
  p1 = (*curve)[curve->size()-1];
    
  dx = p1.y - p0.y;
  dy = p0.x - p1.x;
    
  mag = sqrt(dx*dx + dy*dy);

  dx /= mag;
  dy /= mag;
      
  textureU += ufreq *mag;
  
#ifdef SCAN_CONVERT
  Point v2(p1.x + radius * dx, p1.y + radius * dy);
  Point v3(p1.x - radius * dx, p1.y - radius * dy);
      
  glColor3ub(255,0,0);
  //  drawTriangle(v2,v3,v0);
  drawTriangle(v2,p1,v0);
  glColor3ub(255,255,0);
  drawTriangle(p1,v3,v0);

  glColor3ub(0,255,0);
    drawTriangle(v0,v3,v1);
    //  drawTriangle(v0,p0,v3);
    //  glColor3ub(0,255,255);
    //  drawTriangle(p0,v1,v3);

#else

  //////////////////////// Tappering ///////////////////////////
  radius1 = tapperBrush ? 0 : radius; 
  //////////////////////// Tappering ///////////////////////////

  glTexCoord2f(textureU,textureV + vfreq);
  glVertex3f(p1.x + radius1 * dx, p1.y + radius1 * dy,z);
  glTexCoord2f(textureU,textureV - vfreq);
  glVertex3f(p1.x - radius1 * dx, p1.y - radius1 * dy,z);
      
  glEnd();
#endif



  glColor3ub(0,0,255);

  if (cap)
    {
      drawCap(p1, -dx, -dy,textureU,textureV);
    }

}

void Stroke::drawControl()
{
  glColor3ub(255,0,0);
  drawLines(&control);
}

void Stroke::drawLineCurve()
{
  drawLines(limit);
}

void Stroke::render()
{
  if (!computed)
    {

      //	printf("Computing curve \n");
      computeLimitCurve();
      computed = GL_TRUE;
    }

  /*
    printf("Control = ");
    control.print();
    printf("\nLimit = ");
    limit->print();
    printf("\n");
  */

  //    drawLines(limit);
  drawThickCurve(limit,radius);
}

void Stroke::subdivideCubicBSpline(vector<Point> * inputCurve, 
				   vector<Point> * outputCurve)
{
  outputCurve->erase(outputCurve->begin(),outputCurve->end());

  //    printf("ic-count=%d\n",inputCurve->count);

  if (inputCurve->size() < 1)
    return;

  Point pi0;
  Point pi1;
  Point pi2;

  pi0 = (*inputCurve)[0];

  outputCurve->push_back(Point(pi0.x,pi0.y));

  if (inputCurve->size() == 1)
    return;

  if (inputCurve->size() == 2)
    {
      pi1 = (*inputCurve)[1];

      outputCurve->push_back(Point(pi1.x,pi1.y));

      return;
    }

  pi1 = (*inputCurve)[1];

  outputCurve->push_back(Point((pi0.x + pi1.x)/2,(pi0.y + pi1.y)/2));

  for(int i=1;i<inputCurve->size()-1;i++)
    {
      pi0 = (*inputCurve)[i-1];
      pi1 = (*inputCurve)[i];
      pi2 = (*inputCurve)[i+1];

      outputCurve->push_back(Point( (pi0.x + 6*pi1.x + pi2.x)/8,
				    (pi0.y + 6*pi1.y + pi2.y)/8));

      outputCurve->push_back(Point( (pi1.x + pi2.x)/2,(pi1.y + pi2.y)/2));
    }
	
  outputCurve->push_back(Point(pi2.x,pi2.y));
}

void Stroke::subdivideFourPoint(vector<Point> * inputCurve, 
				vector<Point> * outputCurve)
{
  outputCurve->erase(outputCurve->begin(),outputCurve->end());

  if (inputCurve->size() < 1)
    return;

  Point pi0;
  Point pi1;
  Point pi2;
  Point pi3;

  if (inputCurve->size() == 1)
    {
      pi0 = (*inputCurve)[0];
      outputCurve->push_back(Point(pi0.x,pi0.y));

      return;
    }

  if (inputCurve->size() == 2)
    {
      pi0 = (*inputCurve)[0];
      pi1 = (*inputCurve)[1];
	
      outputCurve->push_back(Point(pi0.x,pi0.y));
      outputCurve->push_back(Point((pi0.x+pi1.x)/2,(pi0.y+pi1.y)/2));
      outputCurve->push_back(Point(pi1.x,pi1.y));
	
      return;
    }

  pi0 = (*inputCurve)[0];
  pi1 = (*inputCurve)[1];

  Point piminus1(2*pi0.x - pi1.x,2*pi0.y - pi1.y);

  pi0 = (*inputCurve)[inputCurve->size()-1];
  pi1 = (*inputCurve)[inputCurve->size()-2];

  Point piplus1(2*pi0.x - pi1.x,2*pi0.y - pi1.y);
    
  for(int i=0;i<inputCurve->size()-1;i++)
    {
      pi0 = (i==0 ? piminus1 : (*inputCurve)[i-1]);
      pi1 = (*inputCurve)[i];
      pi2 = (*inputCurve)[i+1];
      pi3 = (i==inputCurve->size()-2? piplus1:(*inputCurve)[i+2]);

      outputCurve->push_back(Point( pi1.x, pi1.y));

      outputCurve->push_back(Point( (-pi0.x + 9*pi1.x + 9*pi2.x - pi3.x)/16,
				    (-pi0.y + 9*pi1.y + 9*pi2.y - pi3.y)/16));
    }

  pi0 = inputCurve->back();

  outputCurve->push_back(Point(pi0.x,pi0.y));
}

void Stroke::subdivide(vector<Point> * inputCurve, vector<Point> * outputCurve)
{
  switch(curveType)
    {
    case CUBIC_BSPLINE:
      subdivideCubicBSpline(inputCurve,outputCurve);
      break;

    case FOUR_POINT:
      subdivideFourPoint(inputCurve,outputCurve);
      break;

    default:
      printf( "Illegal subdivision scheme selected\n");
      exit(-1);
    }
}

void Stroke::computeLimitCurve()
{
  //    printf("Computing limit curve.  Input length = %d\n",control.count);

  if (limit == NULL)
    limit = new vector<Point>();

  if (temp == NULL)
    temp = new vector<Point>();

  subdivide(&control,limit);

  //    limit->print();
  //    printf(" count = %d\n",limit->size());

  for(int i=0;i<numLevels/2;i++)
    {
      subdivide(limit,temp);
      subdivide(temp,limit);
    }
}

/*
* @parms
* radius: stroke radius
* index: current index of the control pt
* size: number of control pts
* @return
* radius after tappering 
*/
float Stroke::tappering(float radius, int index, int size)
{
  float factor = sqrt(radius);

  // cut-off factor such that 
  // 1 <= factor <= 2
  factor = (min)(factor, 2.0f);
  factor = max(factor, 1.0f);

// mapping
  float k = min(size*factor/5, 40.0f);

  // if colose or the line is too short
  if (index<k)
    return index*(radius/k);
  else if (index>size-k-1)
    return radius-(index-size+k+1)*(radius/k);
  else
    return radius;
}


