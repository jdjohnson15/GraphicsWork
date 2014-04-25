// FILE: gasket1.cpp
// NAME: J.Johnson
// DATE: Week 3, Fall 2013
//  Two-Dimensional Sierpinski Gasket   
//  Adapted From Angel's book, Chapter 2    
//  Generated using randomly selected vertices and bisection

#include "Angel.h"
#include <iostream>
using namespace std;

//GLOBAL VARIABLES:
//no longer a constant variable: changed by user input
int NumPoints = 0;

//FUNCTIONS:
void init();
void display();
void keyboard( unsigned char key, int x, int y );

//////////////////////////////////////////////////////////////////////

int main( int argc, char **argv )
{
   
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA );
    glutInitWindowSize( 512, 512 );

    glutCreateWindow( "Sierpinski Gasket" );

    //use atoi to convert string into an int value (http://www.cplusplus.com/reference/cstdlib/atoi/)
    NumPoints = atoi (argv[1]);
    init();
    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );

    glutMainLoop();
    return 0;
}

////////////////////////////////////////////////////////////////////////
void init()
{
    vec2 points[NumPoints];

    // Specifiy the vertices for a triangle
    vec2 vertices[3] = {
        vec2( -1.0, -1.0 ), vec2( 0.0, 1.0 ), vec2( 1.0, -1.0 )
    };

    // Select an arbitrary initial point inside of the triangle
    points[0] = vec2( 0.25, 0.50 );

    // compute and store N-1 new points
    for ( int i = 1; i < NumPoints; ++i ) {
        int j = rand() % 3;   // pick a vertex at random

        // Compute the point halfway between the selected vertex
        //   and the previous point
        points[i] = ( points[i - 1] + vertices[j] ) / 2.0;
    }

    // Create a vertex array object
    GLuint vao[1];


    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader_gasket1.glsl", "fshader_gasket1.glsl" );
    glUseProgram( program );

    // Initialize the vertex position attribute from the vertex shaders
    GLuint loc = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( loc );
    glVertexAttribPointer( loc, 2, GL_FLOAT, GL_FALSE, 0,
                           BUFFER_OFFSET(0) );

    glClearColor( 1.0, 1.0, 1.0, 1.0 ); // white background
}
/////////////////////////////////////////////////////////////////////////////////////
//void display(int numPoints)
void display()
{
    glClear( GL_COLOR_BUFFER_BIT );     // clear the window
    glDrawArrays( GL_POINTS, 0, NumPoints );    // draw the points
    glFlush();
}

/////////////////////////////////////////////////////////////////////////////////////

void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
    }
}
