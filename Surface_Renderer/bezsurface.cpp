//FILE: bezsurface.cpp
//NAME:  J.Johnson
//DATE:  Mar. 2014
//  
//---------------------------------------------------------105 point attempt-------------------------------------------------
//
//
//	An interactive curve designer that generates and displays a 2D curve based on user-selected control points.
//
//	
//	Control points are big red dots. 
//	When selected, they will become green to indicate that they are selected.
//	When green, they can be edited by clicking the window, and the curve will redraw itself automatically.
//	The curves are made of black points.
//	
//	controls:
//		
//		left mouse button: 	Click on a control point to select it. Then click elsewhere to move it. 
//					Clicking on another point will deselect the current point.
//		middle mouse button: 	Deselect all points
//		right mouse button:	Create new control point at cursor's location.
//		t key:			show/hide control points (toggle)
//		d key:			double number of curve points
//		D key:			halve number of curve points
//		l key:			Draw a green, stippled guideline connecting the 3rd and 4th control points of the most recent curve segment, which helps 
//					with making the next segment 1-smooth with the segment.  
//		p key:			Draw the control polygon (yellow)
//		h key:			Draw the convex hull (white) (this one doesn't work right. It connects the points but doesn't draw a convex polygon)
//
//
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
#include <fstream>
#include <string>
using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;


const int MAX_SURFACE_POINTS = 100000; //arbitrarily large amount of points
const int CONTROL_POINTS = 16;//arbitrary number of control points
int num_surface_vertices = 100;
int real_vert_number;
point4 control_points[4][4];
point4 surface_points[MAX_SURFACE_POINTS];
color4 colors[MAX_SURFACE_POINTS];
point4 points[MAX_SURFACE_POINTS];
//frame and window default values
float f_l = -0.5; 
float f_b = -0.5; 
float f_r =  0.5;
float f_t =  0.5;
int window_width = 512;
int window_height = 512;
float VPx, VPy, VPw, VPh;

//Spherical viewing coordinates
GLdouble elevation= 0.0;
GLdouble azimuth  = 0.0;

//fly viewing coordinates 

GLdouble cam_x = 0.0;
GLdouble cam_y = 0.0;
GLdouble cam_z = -2.0;

bool lines = false;

mat4  model, view;



//locations of variable sent to shader
GLuint  Model_loc;  
GLuint  View_loc;  
GLuint  Projection_loc;
GLuint  vPosition;
GLuint  vColor;
GLuint  program;



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

void drawSurface();
//pre: 
//post: pointsfor surface are saved in buffer

float B(int k, float t);

int factorial( int t );

mat4 flyView(GLdouble cam_x, GLdouble cam_y, GLdouble cam_z);
//pre: 	cam_x, cam_y, and cam_z are the camera's x, y, and z positional coordinates, respectively. 
//post: the camera is positioned according to the provided information	

mat4 polarView(GLdouble elevation, GLdouble azimuth);
//pre: 	
//	elevation:	y axis rotation
//	azimuth:	x axis rotation
//post: the camera is positioned according to the provided information	

//______________________________________________________________
////main program 
//

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( window_width, window_height );
	glutCreateWindow( "Bezier (110 point attempt)" );
	init();
	glEnable(GL_BLEND);
	glEnable(GL_POINT_SMOOTH); 
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specialKeyboard );
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

	// Create and initialize a buffer object
	    GLuint buffer;
	    glGenBuffers( 2, &buffer );
	    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
			  NULL, GL_STATIC_DRAW );
	    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );



	// Load shaders and use the resulting shader program
	program = InitShader( "v_bezsurface.glsl", "f_bezsurface.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	    vPosition = glGetAttribLocation( program, "vPosition" );
	    glEnableVertexAttribArray( vPosition );
	    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(0) );

	    vColor = glGetAttribLocation( program, "vColor" ); 
	    glEnableVertexAttribArray( vColor );
	    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
				   BUFFER_OFFSET(sizeof(points)) );

	
	// Retrieve transformation uniform variable locations
	Model_loc = glGetUniformLocation( program, "Model" );
	View_loc = glGetUniformLocation( program, "View" );

	// Generate the Projection Matrix and send to vertex shader:
	Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Frustum( -1, 1,-1, 1, 1, 10 ) );

	//glEnable( GL_DEPTH_TEST );

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

	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
	glUniformMatrix4fv( View_loc, 1, GL_TRUE, polarView(elevation, azimuth)*flyView(cam_x, cam_y, cam_z));
	drawSurface();

	//draw surface
	glPointSize(3.0);
	glDrawArrays(GL_POINTS, 0, real_vert_number);


	glutSwapBuffers();
}

//______________________________________________________________

void drawSurface()
{

	ifstream file;
	file.open("control.txt");
	GLfloat x, y, z;		
	for (int i = 0; i < sqrt(CONTROL_POINTS); ++i){
		for (int j = 0; j < sqrt(CONTROL_POINTS); ++j){

			file >> x;
			file >> y;
			file >> z;
			control_points[i][j] = point4( x, y, z, 1.0 );
		}
	}
	float inc = 1.0 / sqrt(num_surface_vertices); 	
	float u = 0.0; 
	float v = 0.0;
	int iPoint = 0;	
	int loop = 0;			

	//calculate the vertices
	for (int c = 0; u < 1 ; ++c)	
	{ 
		u = c*inc;
		v = 0.0;
		
		for (int d = 0; v < 1; ++d)
		{
			v = d*inc;
			
			surface_points[iPoint] = point4(0.0, 0.0, 0.0, 1.0);
			for (int k = 0; k < sqrt(CONTROL_POINTS); ++k)
			{
				for (int i = 0; i < sqrt(CONTROL_POINTS); ++i)
				{
					surface_points[iPoint] += B(i, u)*B(k, v)*control_points[i][k];
					
				}
			}
			surface_points[iPoint].w = 1.0;
			++iPoint;
		}
		
		
	}
	real_vert_number = iPoint;


	//put the vertices for the control and surface points into a single array, and assign each vertex a color.
	for (int i = 0; i < real_vert_number; ++i)
	{
		points[i] = surface_points[i];
		colors[i] = color4(0.2, 1.0, 0.2, 1.0); // make surface points black
		//cout << i<<": "<<points[i]<<endl;
	

	}
	
	//load the two arrays into the buffer
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

}

//__________math functions______________________________________________________
float B( int k, float t )
{
	return (factorial(3)/(factorial(3-k)*factorial(k)))*pow((1-t), (3-k))*pow(t, k);
}

int factorial( int t ){
	if (t > 1)
	{
		t *= factorial(t-1);
	}else
	{
		return 1;
	}
		
}

//________________control functions______________________________________________

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:     //ESC// quit program
			exit(0);
			break;
		case 'f':
			if (lines){
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				lines = false;
			}else{
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				lines = true;
			}
			glutPostRedisplay();
			break;

		case 'j':
			azimuth 		+=   5.0;
			glutPostRedisplay();
			break;
		case 'l':
			azimuth			+=  -5.0;
			glutPostRedisplay();
			break;

		case 'i':
			elevation 		+=   5.0;
			glutPostRedisplay();
			break;
		case 'k':
			elevation		+=  -5.0;
			glutPostRedisplay();
			break;

		case 'a':
			cam_y 		        +=   -0.1;
			glutPostRedisplay();
			break;
		case 'z':
			cam_y 		        +=    0.1;
			glutPostRedisplay();
			break;

		case 'd':  //double number of surface points  
		       num_surface_vertices *= 2;
		       if (num_surface_vertices > MAX_SURFACE_POINTS){num_surface_vertices = 1000000;}
			glutPostRedisplay();
			break;

		case 'D':  //halve number of surface points  
			num_surface_vertices /= 2;
			if ((num_surface_vertices) < 0){num_surface_vertices = 1;}
			glutPostRedisplay();
			break;

   	}
}

void specialKeyboard (int key, int x, int y){
	switch( key ) {
	case GLUT_KEY_LEFT:
		cam_x 	+=   0.1;
		glutPostRedisplay();
		break; 	
	case GLUT_KEY_RIGHT:
		cam_x 	+=  -0.1;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_UP:
		cam_z 	+=   0.1;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_DOWN:
		cam_z 	+=  -0.1;
		glutPostRedisplay();
		break; 
	}
}


//______view port and camera functions___________________________________
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

//__________________________________________________________________________________
mat4 flyView(GLdouble cam_x, GLdouble cam_y, GLdouble cam_z){

	mat4 camera = Translate(cam_x, cam_y, cam_z);
	return camera;
}
//__________________________________________________________________________________
mat4 polarView(GLdouble elevation, GLdouble azimuth){

	mat4 camera = Translate(cam_x, cam_y, cam_z)*RotateX(elevation) * RotateY(azimuth)*Translate(-cam_x, -cam_y, -cam_z);
	return camera;
}


