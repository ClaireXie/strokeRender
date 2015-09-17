//#include <glut.h>
#include <math.h>
#include <vector>
#include "Point.h"

#include "GL/glut.h"

using namespace std;


#define NUM_SLICES 20

typedef enum { CUBIC_BSPLINE, FOUR_POINT } CurveType;



class Stroke
{
private:
  vector<Point> control;

  vector<Point> * limit;
  vector<Point> * temp;

  bool computed;

  int numLevels;

  static GLUquadricObj * qobj;
public:

  float z;

  float radius;

  bool useTexture;
  bool tapperBrush;
  float ufreq;
  float vfreq;
  float ustart;
  float vstart;

  CurveType curveType;

  Stroke();
  ~Stroke();
  void add(float x, float y);
  void clear();
  static void drawLines(vector<Point> * curve);
  void forceRecompute();
  void discPoint(float x,float y,float brushRadius);
  void drawCap(const Point & p0, float dx, float dy,float texU,float texV);
  void drawThickCurve(vector<Point> * curve, float radius, bool cap = true);
  void drawControl();
  void drawLineCurve();
  void render();
  void subdivideCubicBSpline(vector<Point> * inputCurve, 
			     vector<Point> * outputCurve);
  void subdivideFourPoint(vector<Point> * inputCurve, 
			  vector<Point> * outputCurve);
  void subdivide(vector<Point> * inputCurve, 
		 vector<Point> * outputCurve);
  void computeLimitCurve();

  float tappering(float radius, int index, int size);
};

