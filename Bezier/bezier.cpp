//FILE: bezier.cpp
//NAME:  J.Johnson
//DATE:  Feb. 2014
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
//using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
const int MAX_CURVE_POINTS = 5000; //arbitrarily large amount of points
const int MAX_CONTROL_POINTS = 100;//arbitrary number of control points
int num_curve_vertices = 20;
int num_controls = 0;
int num_segments = 0;
point4 control_points[MAX_CONTROL_POINTS];
point4 curve_points[MAX_CURVE_POINTS];
color4 colors[MAX_CURVE_POINTS];
point4 points[MAX_CURVE_POINTS];
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

//variables for controls
bool control_view = true; //helps disable/enable control points
bool guide = false;
bool convex_hull = false;
bool control_polygon = false;
bool autocomplete = false;



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
	
int select_point(float x, float y);
//pre:  array of colors and control points exist. Mouse input exists. int x and y are real frame x,y coordinates of mouse input.
//post: Control point matching the sent coordinate is "selected." "Selected" point is colored green, and the function returns the index number of
//	the point. If no control point is found at the clicked point, it returns -1.  

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

	// Create and initialize a buffer object
	    GLuint buffer;
	    glGenBuffers( 2, &buffer );
	    glBindBuffer( GL_ARRAY_BUFFER, buffer );
	    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
			  NULL, GL_STATIC_DRAW );
	    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );



	// Load shaders and use the resulting shader program
	GLuint program = InitShader( "v_bezier.glsl", "f_bezier.glsl" );
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
	
	drawCurve(num_controls);

	//draw control points
	if (control_view){
		glPointSize(5.0);
		glDrawArrays(GL_POINTS, 0, num_controls);
	}
	//draw curve
	glPointSize(1.0);
	glDrawArrays(GL_POINTS, num_controls, num_segments*num_curve_vertices);

	//draw 1-smooth guide
	if (guide)
	{
		
		glLineStipple(1, 0x3F07);
		glEnable(GL_LINE_STIPPLE);

		//create points for guide line
		points[num_segments*num_curve_vertices + num_controls]   = -control_points[((num_segments-1)*3)+2];
		points[num_segments*num_curve_vertices + num_controls+1] = control_points[((num_segments-1)*3)+3];
		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );

		//turn the points green
		colors[num_segments*num_curve_vertices + num_controls] = colors[num_segments*num_curve_vertices + num_controls+1] 
			= color4(0.3, 1.0, 0.0, 1.0);
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
		
		//draw the line
		glDrawArrays(GL_LINES, num_segments*num_curve_vertices + num_controls, 2);
		
		
		colors[((num_segments-1)*3)+2] = colors[((num_segments-1)*3)+3] = color4(1.0, 0.0, 0.0, 1.0);
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
		glDisable(GL_LINE_STIPPLE);

		
	}
	
	//draw control polygon
	if (control_polygon)
	{
		for (int i = 0; i < num_controls; ++i)
		{		
			colors[i] = color4(0.8, 0.7, 0.0, 1.0);
		}
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
		glDrawArrays(GL_LINE_STRIP, 0, num_controls);
		for (int i = 0; i < num_controls; ++i)
		{		
			colors[i] = color4(1.0, 0.0, 0.0, 1.0);
		}
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
	}

	//draw convex hull (doesn't work. didn't have time to figure it out)
	if (convex_hull)
	{
		for (int i = 0; i < num_controls + 1; ++i)
		{		
			colors[i] = color4(1.0, 1.0, 1.0, 1.0);
		}
		int mod; 
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
		for (int g = 0; g < num_segments; ++g)
		{
			mod = g*3;
			
			glDrawArrays(GL_LINE_LOOP, 0+mod, 2);
			glDrawArrays(GL_LINE_LOOP, 1+mod, 2);
			glDrawArrays(GL_LINE_LOOP, 2+mod, 2);
			glDrawArrays(GL_LINE_LOOP, 3+mod, 2);	
		}
		
		
		for (int i = 0; i < num_controls + 1; ++i)
		{		
			colors[i] = color4(1.0, 0.0, 0.0, 1.0);
		}
		glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );
	}

	glutSwapBuffers();
}

//______________________________________________________________

void drawCurve(int num_controls)
{
	if (num_curve_vertices*num_segments > MAX_CURVE_POINTS){num_curve_vertices = MAX_CURVE_POINTS/num_segments;}
	num_segments = num_controls / 4;	//ensures one segment per 4 control points for the FIRST line segment, and an additional segment for 
	if (num_segments >= 1)			//each subsequent 3 control points.
	{
		num_segments = (num_controls-1)/3;
	}

	float inc = 1.0 / num_curve_vertices; 	// gap between each point on a curve segment
	float t;				// step on curve segment
	int mod;				// tracks jumps in array index between segments

	//for each curve segment, calculate the vertices
	for (int g = 0; g < num_segments; ++g)	
	{ 
		mod = g*3;
		for (int i = (g*num_curve_vertices); i < (g+1)*num_curve_vertices; ++i)
		{
			t = inc*(i-(g*num_curve_vertices));
			curve_points[i] =	   (pow(1.0 - t, 3.0)*control_points[0+mod]) + 
					    	(3.0*pow(1.0 - t, 2.0)*t*control_points[1+mod]) + 
						(3.0*(1.0 - t)*pow(t, 2.0)*control_points[2+mod]) + 
						(pow(t, 3.0)*control_points[3+mod]);
		}
		
	}
	//put the vertices for the control and curve points into a single array, and assign each vertex a color: red for controls, black for curve.
	for (int i = 0; i < (num_controls + (num_curve_vertices*num_segments)); ++i)
	{
		if (i < num_controls)
		{
			points[i] = control_points[i];
		}
		
		if ((i > num_controls) || (i == num_controls))
		{
			points[i] = curve_points[i-num_controls];
			colors[i] = color4(0.0, 0.0, 0.0, 1.0); // make curve points black
		}
	}
	
	//load the two arrays into the buffer
	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

}



//______________________________________________________________

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:     //ESC// quit program
			exit(0);
			break;
		case 't': // toggle control points   
			if (control_view){control_view = false;}
			else{control_view = true;}
			glutPostRedisplay();
			break;
		case 'l': // toggle guide line (for 1-smoothness)   
			if (guide){guide = false;}
			else{guide = true;}
			glutPostRedisplay();
			break; 
		case 'p': // toggle control polygon    
			if (control_polygon){control_polygon = false;}
			else{control_polygon = true;}
			glutPostRedisplay();
			break; 
		case 'h': // toggle convex hull  
			if (convex_hull){convex_hull = false;}
			else{convex_hull = true;}
			glutPostRedisplay();
			break;   		
		case 'd':  //double number of curve points  
		       num_curve_vertices *= 2;
		       if ((num_curve_vertices*num_segments) > MAX_CURVE_POINTS){num_curve_vertices = MAX_CURVE_POINTS/num_segments - 500;}
			glutPostRedisplay();
			break;

		case 'D':  //halve number of curve points  
			num_curve_vertices /= 2;
			glutPostRedisplay();
			break;

		case 'f': // auto complete line
			if (num_controls == (4 + (num_segments*3)) - 1)
			{
				control_points[control_index] = control_points[0];
				colors[control_index] = color4(1.0, 0.0, 0.0, 1.0); // make new control point red
				++num_controls;
				++control_index;
				glutPostRedisplay();
			}
			break;
   	}
}
//______________________________________________________________

void mouse(int button, int state, int x, int y)
{
	frame_x = (float) x / (VPw/2) - 1.0;		//formula derived from example on pg. 100 of 
	frame_y = (float) (VPh - y) / (VPh/2) - 1.0;	// Interactive Computer Graphics textbook

//----------------------------------------------------------------------------------------------
	//control point selection
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		move_index = select_point(frame_x, frame_y); 	//Returns index number of a control point. 
								//Returns -1 if no point is present at cursor's coordinates


		if (move_index > -1)
		{
			if (grab > -1)
			{
				colors[grab] = color4(1.0, 0.0, 0.0, 1.0); // make previously selected control point red (deselected)
			}
			grab = move_index;
			colors[grab] = color4(0.0, 1.0, 0.0, 1.0); // make selected control point green 

		}
		if (grab > -1)
		{
			control_points[grab] = point4(frame_x, frame_y, 0.0, 1.0);
			colors[grab] = color4(0.0, 1.0, 0.0, 1.0); // make selected control point green	
		}
		glutPostRedisplay();
	}
//----------------------------------------------------------------------------------------------
	//deselect points (disable editing of point)
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		if (grab > -1)
		{
			colors[grab] = color4(1.0, 0.0, 0.0, 1.0); // make selected control point red
			glutPostRedisplay();
			grab = -1;	
		}
	}
//----------------------------------------------------------------------------------------------
	//Create new control points
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		control_points[control_index] = point4(frame_x, frame_y, 0.0, 1.0);
		colors[control_index] = color4(1.0, 0.0, 0.0, 1.0); // make new control point red
		++control_index;
		++num_controls;
		glutPostRedisplay();
	}
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

int select_point(float x, float y)
{
	float err = 0.01; //amount of fudge factor. 
	for (int i = 0; i < num_controls; ++i)
	{
		colors[i] = color4(1.0, 0.0, 0.0, 1.0); // make unselected control point red
		if ((fabs(x - control_points[i].x) < err ) && (fabs(y - control_points[i].y) < err))   
		{
			colors[i] = color4(0.0, 1.0, 0.0, 1.0); // make selected control point green
			glPointSize(5.0);
			glDrawArrays(GL_POINTS, 0, num_controls);
			return i;
		}
		
	}
	return -1;	
}
