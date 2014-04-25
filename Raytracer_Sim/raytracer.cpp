//FILE: bezier.cpp
//NAME:  J.Johnson
//DATE:  Feb. 2014
//  
//---------------------------------------------------------95 point attempt-------------------------------------------------
//
//
//	An interactive ray tracer simulation 
//	
//	controls:
//		
//		left mouse click: set ray start and direction
//		R: reset simulation
//		d: automatically trace rays
//		s: manually trace ray after a hit on the chamber wall
//
//
//
 #ifdef __APPLE__       // For use with OS X
 #include <GLUT/glut.h>
 #else		       // Other (Linux)
 #include <GL/glut.h>         
 #endif

#include "Angel.h"
#include <math.h>

#include <iostream>
//using namespace std;






typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

const int MAX_GEO_POINTS = 10;

struct Ray
{
	point4 start;
	point4 current;
	vec3 direction;
	int init;
};

Ray ray;

const int RAY_INITIALIZED = 2;
const int RAY_BEGIN_INITIALIZATION = 1;
const int RAY_UNINITIALIZED = 0;


struct Geo
{
	point4 points[MAX_GEO_POINTS];
	int num_points;
	vec3 normals[MAX_GEO_POINTS]; 
};
Geo convex_chamber;

const int MAX_POINTS = 5000; //arbitrarily large amount of points
const int CHAMBER_POINTS = 4;//arbitrary number of control points 
int num_hit_points = 0;
int draw_rate;
bool draw = false; 
bool keepDrawing = false;

color4 colors[MAX_POINTS];
point4 points[MAX_POINTS];
//frame and window default values
float f_l = -0.5; 
float f_b = -0.5; 
float f_r =  0.5;
float f_t =  0.5;
int window_width = 512;
int window_height = 512;
float VPx, VPy, VPw, VPh;

//variables for mouse-return coordinates

float frame_x;
float frame_y;
int control_index = 0;
int grab = -1;
int move_index = -1;

//______________________________________________________________
//functions//
void init();
void display();
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void specialKeyboard (int key, int x, int y);
void reshape(int w, int h);
void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight);
void display();

void drawCurve(int num_controls);
//pre: 	an array of control points, curve points, and colors exist. num_controls is the number of control points.
//post:	curve point and control point data is saved into the shader buffer to be used later. 

void drawRay();	
void clearRay();
vec3 normal(point4 a, point4 b);


//______________________________________________________________
////main program 
//

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize( window_width, window_height );
	glutCreateWindow( "Bezier (105 point attempt)" );
	init();
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH); 
	glutKeyboardFunc( keyboard );
	glutMouseFunc(mouse);
	glutDisplayFunc( display );
	glutReshapeFunc(reshape);	
	
	glutMainLoop();
	return 0;
}


//______________________________________________________________


void init() 
{
	// Create a vertex array object
	GLuint vao[1];
	#ifdef __APPLE__       // For use with OS X
		glGenVertexArraysAPPLE(1, vao );
		glBindVertexArrayAPPLE(vao[0] );
	#else		       // Other (Linux)
		glGenVertexArrays(1, vao );
		glBindVertexArray(vao[0] );       
	#endif

	// create points, colors and normals for convex chamber 

	convex_chamber.points[0] = point4(-0.75, -0.75, 0.0, 1.0);
	convex_chamber.points[1] = point4(0.75, -0.75, 0.0, 1.0);
	convex_chamber.points[2] = point4(0.75, 0.75, 0.0, 1.0);
	convex_chamber.points[3] = point4(-0.75, 0.75, 0.0, 1.0);
	convex_chamber.num_points = 4; 
	for (int i = 0; i < convex_chamber.num_points; ++i)
	{
		points[i+2] = convex_chamber.points[i]; // take ray points into account, saved in 0 and 1
		convex_chamber.normals[i] = -normal(convex_chamber.points[i], convex_chamber.points[i+1]);
		colors[i+2] = color4(0.0, 0.0, 0.0, 1.0);
	}
	convex_chamber.normals[3] = -normal(convex_chamber.points[3], convex_chamber.points[0]);

	//init ray
	
	ray.init = RAY_UNINITIALIZED;

	// Create and initialize a buffer object
	    GLuint buffer;
	    glGenBuffers( 2, &buffer );
	    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
			  NULL, GL_STATIC_DRAW );
	    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );



	// Load shaders and use the resulting shader program
	GLuint program = InitShader( "v_raytracer.glsl", "f_raytracer.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	    glEnableVertexAttribArray( vPosition );
	    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );

	    GLuint vColor = glGetAttribLocation( program, "vColor" ); 
	    glEnableVertexAttribArray( vColor );
	    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(sizeof(points)) );
	
	// Generate the Projection Matrix and send to vertex shader:
	GLuint Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Ortho( -1,1,-1,1, -1 ,1 ) );

    	glShadeModel(GL_FLAT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// set clear color to white
	glClearColor (0.95, 0.95, 0.95, 1.0);

}

//______________________________________________________________


	
void display()
{
	//clear window
  	 glClear( GL_COLOR_BUFFER_BIT );
	
	drawRay();

	//draw convex chamber
	glLineWidth(3.0);

	glDrawArrays(GL_LINE_LOOP, 2, convex_chamber.num_points);
	
	//draw current ray position

	if (ray.init > RAY_UNINITIALIZED)
	{

		glPointSize(5.0);
		glDrawArrays(GL_POINTS, 0, 1);
	}

	if (ray.init == RAY_INITIALIZED)
	{
		glLineStipple(1, 0x3F07);
		glEnable(GL_LINE_STIPPLE);
		glLineWidth(3.0);
		glDrawArrays(GL_LINE_STRIP, 0, 2);
		glDisable(GL_LINE_STIPPLE);
	}

	//draw ray trace
	
		glDrawArrays(GL_LINE_STRIP, 2 + convex_chamber.num_points, num_hit_points+1);
		//std::cout<<"point1: "<<points[2 + convex_chamber.num_points]<<"  point2: "<<points[3 + convex_chamber.num_points]<<std::endl;


	//pillars
	//glPointSize(1.0);
	//glDrawArrays(GL_POINTS, num_controls, num_segments*num_curve_vertices);


	glutSwapBuffers();
}

//______________________________________________________________

void drawRay()
{
	point4 P;
	point4 S;
	vec3   C;
	vec3   N;
	vec3 saveN;
	point4 B;
	point4 A;
	float hit_time;

	A = ray.start;
	C = ray.direction;
	
	if (ray.init >= RAY_INITIALIZED)
	{
		//add new hit point
		if (draw)
		{
			float smallest = 10000.0; //arbitrarily large number
	
			//find smallest hit time
			for (int i = 0; i < convex_chamber.num_points; ++i)
			{
				N = convex_chamber.normals[i];
				
				B = convex_chamber.points[i];
									
				vec3 meh = vec3(B.x - A.x, B.y - A.y, 0.0);
				
				hit_time = (dot(N, meh) / (dot(N, C)));
			
				if (hit_time > 0)
				{		
					if (hit_time < smallest)
					{
						smallest = hit_time;
						saveN = N;
					}
				}
				
				
			}
			hit_time = smallest - 0.005;
			
			P = point4(A.x + C.x*hit_time, A.y + C.y*hit_time, 0.0, 1.0);
		
			points[ 2 + convex_chamber.num_points + num_hit_points] = P; 
			colors[ 2 + convex_chamber.num_points + num_hit_points] = color4( 1.0, 0.0, 0.0, 1.0);
			//load the two arrays into the buffer

			ray.start = P; 
			ray.direction = C - 2*(dot(C, normalize(saveN)))*normalize(saveN);

			glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
			glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
			
		}
	}

	
	
	if (!keepDrawing)
	{
		draw = false; 
	}
	else{
		
		++num_hit_points; 
		draw = true;
		glutPostRedisplay();
	}

}



vec3 normal(point4 a, point4 b)
{
	vec3 V = vec3(a.x - b.x, a.y - b.y, a.z - b.z);
	vec3 N = vec3(-V.y, V.x, 0.0);

	return normalize(N);	
}

//______________________________________________________________

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:     //ESC// quit program
			exit(0);
			break;
	  		
		case 'd':    
		       	if (keepDrawing){keepDrawing = false;}
			else{ keepDrawing = true;}
			glutPostRedisplay();
			
			break;

		case 'r': // restart
			clearRay();
			glutPostRedisplay();
			
			break;
		case 's': // add another ray trace
			
			if (keepDrawing == false)
			{
				++num_hit_points;
				draw = true; 
			}else{
				keepDrawing = false;
			}
			glutPostRedisplay();			
			break;
   	}
}
//______________________________________________________________

void mouse(int button, int state, int x, int y)
{
	frame_x = (float) x / (VPw/2) - 1.0;		//formula derived from example on pg. 100 of 
	frame_y = (float) (VPh - y) / (VPh/2) - 1.0;	// Interactive Computer Graphics textbook

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if ( ray.init == RAY_UNINITIALIZED)
		{
			ray.start = point4( frame_x, frame_y, 0.0, 1.0);
			++ray.init;

			points[0] = ray.start;
			colors[0] = color4(0.0, 0.0, 0.0, 1.0);
			points[2 + convex_chamber.num_points] = ray.start;
			colors[2 + convex_chamber.num_points] = color4(1.0, 0.0, 0.0, 1.0);
			
			glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
			glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
			glutPostRedisplay();
		}else{
			if ( ray.init == RAY_BEGIN_INITIALIZATION)
			{
				float _x, _y, _z;
				_x = frame_x - ray.start.x;
				_y = frame_y - ray.start.y;
				_z = 0.0;
				ray.direction = (vec3(_x, _y, _z));
					//normalize(vec3(_x, _y, _z))/2.0; 
				++ray.init;
				points[1] = point4(ray.start.x+ray.direction.x, ray.start.y+ray.direction.y, 0.0, 1.0);
				colors[1] =color4(1.0, 0.0, 0.0, 1.0);


				glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
				glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
				glutPostRedisplay();
			}
		}

	}
	glutPostRedisplay();

}

//______________________________________________________________
void reshape( int w, int h )
{
	window_width = w;
	window_height = h;
	glutPostRedisplay();
	SetAutoVP(f_l, f_b, f_r, f_t, window_width, window_height);

}
//______________________________________________________________

void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight)
{
	float Frame_a_r  = (r-l)/(t-b);
	float Window_a_r = wWidth/wHeight;	
	if (Frame_a_r > Window_a_r){
		VPx = 0;
		VPw = wWidth; 
		VPh = VPw / Frame_a_r;
		VPy = (wHeight - VPh)/2;
	}
	if (Frame_a_r <= Window_a_r){
		VPy = 0;
		VPh = wHeight; 
		VPw = VPh * Frame_a_r;
		VPx = (wWidth - VPw)/2;
	}
	glViewport(VPx, VPy, VPw, VPh);

}
//______________________________________________________________

void clearRay()
{
	ray.start = point4(0.0, 0.0, 0.0, 0.0); 
	ray.direction = vec3(0.0, 0.0, 0.0);
	ray.init = 0; 
	for (int i = convex_chamber.num_points + 2; i < num_hit_points - convex_chamber.num_points - 2; ++i)
	{
		points[i] = point4(0.0, 0.0, 0.0, 0.0);
		colors[i] = color4(0.0, 0.0, 0.0, 0.0);
	}
	num_hit_points = 0;
	keepDrawing = false;
	draw = false;
}


