//FILE: tile.cpp
//NAME:  J.Johnson
//DATE:  Oct. 2013
//  Project1: "tile"
//


 #ifdef __APPLE__       // For use with OS X
 #include <GLUT/glut.h>
 #else		       // Other (Linux)
 #include <GL/glut.h>         
 #endif
 /* glut.h includes gl.h and glu.h*/


#include "Angel.h"  //Provides InitShader

#include <iostream>
using namespace std;

//GLOBAL VARIABLES:
int window_width=800, window_height=800;

double x=1; //the number of times the image is repeated on an axis


double xa, ya, xb, yb;
void init();							//called once
void display();							//called every time the window is redrawn
void keyboard(unsigned char key, int x, int y);			//uses ESC to quit program
void reshape( int w, int h );					//called whenever window size/position is changed

void drawDesign(double numCopy); 
	//pre: the number of times one wishes the image to be copied over each axis
	//post: the display callback is reset the specified number of times for incremented viewpoints, 
		//which will result in repeated instances of the vertex array.

void setVP(double x_1, double y_1, double x_2, double y_2);
	//pre: take in float/double values for the x and y coord of the bottom left and the width and height of the viewport 
	//post: set the viewport according to the new values


int main(int argc, char** argv) {


  //save command line input to use with drawTheThing() function
  x = atof(argv[1]);
	
  //Initialize Window, display mode (singly buffered window, RGB mode).
  
  glutInit(&argc,argv); 
  glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

  //Window Size
  glutInitWindowSize(window_width,window_height);

  //Window Position (upper left corner of the screen).
  glutInitWindowPosition(0,0); 

  //glutCreateWindow(argv[0]);  // Window title is name of program (argv[0]) 
  glutCreateWindow("tile  (ESC to quit)");

  //Pass our display function to GLUT void display(void);
  glutDisplayFunc(display);

  //register callback for keys
  glutKeyboardFunc(keyboard);  

  //register callback for reshaping the window
  glutReshapeFunc( reshape );

  //Our initialization function
  init();

  //Entger the GLUT event processing loop.
  glutMainLoop();

}
/////////////////////////////////////////////////////////////////////////////////

void display() {
	drawDesign(x); 
}


void init() {
  // Specify the vertices for face
  vec2 eye1Points[23] = {
    vec2(-0.5, 0.0), vec2(-0.5, 0.1), vec2(-0.4, 0.1), vec2(-0.4, 0.0),//left eye
    vec2(0.4, 0.0), vec2(0.4, 0.1), vec2(0.5, 0.1), vec2(0.5, 0.0), //right eye
    vec2(-0.5, -0.3), vec2(-0.5, -0.4), vec2(0.5, -0.4), vec2(0.5, -0.3), //mouth
    vec2(-0.7, 0.5), vec2(-0.6, 0.6), vec2(-0.2, 0.3), vec2(-0.3, 0.2), //left brow
    vec2(0.7, 0.5), vec2(0.6, 0.6), vec2(0.2, 0.3), vec2(0.3, 0.2), //left brow
    vec2(-0.1,-0.5), vec2(0.1, -0.5), vec2(0.0, -0.7)
  };

  
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
  glBufferData( GL_ARRAY_BUFFER, sizeof(eye1Points), eye1Points, GL_STATIC_DRAW );
  


  // Load shaders and use the resulting shader program
  GLuint program = InitShader( "vshader_tile.glsl", "fshader_tile.glsl" );
  glUseProgram( program );
  
  // Initialize the vertex position attribute from the vertex shader
  GLuint loc = glGetAttribLocation( program, "vPosition" );
  glEnableVertexAttribArray( loc );
  glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
  
  
  // set clear color to black
  glClearColor (0.0, 0.0, 0.0, 0.0);

}

void reshape( int w, int h )
{
	window_width = w;
	window_height = h;
	glutPostRedisplay();

}
void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:     //ESC
         exit(0);
         break;
   }
}

void setVP(double x_1, double y_1, double x_2, double y_2)
{
	glViewport(window_width*x_1, window_height*y_1, window_width*(x_2 - x_1), window_height*(y_2 - y_1));
}
	
void drawDesign(double numCopy)
{
	//clear window
  	glClear(GL_COLOR_BUFFER_BIT); 
	
	double inc = 1.0/ numCopy;		//amount window coord increments during loop cycles

  	

	//this will change the viewpoint in increments and then set the display callback according to the new viewpoint, 
	//looping by rows and columns
	for (int k = 0; k < numCopy; ++k){
		for(int j = 0; j < numCopy; ++j){
			setVP(j*inc, k*inc, (j+1)*inc, (k+1)*inc);
		 	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
			glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
			glDrawArrays(GL_TRIANGLES, 20, 3);
		}
	}
	
	//flush GL buffers
	glFlush(); 
}
