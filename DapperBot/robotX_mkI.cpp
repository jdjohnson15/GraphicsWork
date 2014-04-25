//FILE: barnyard.cpp
//NAME: Jesse Johnson
//DATE: 11/13/2013
//CS 3014
//
//
// Displays a robot! The camera can controlled to view the scene.
//
// controls:
//
// f: toggles between "lines" and "filled" renderings. 
// 
// left and right arrows: change azimuth of camera
// up and down arrows:    change elevation of camera
// l and r:		  changes the twist of camera
// (+) and (-):		  zoom in to and out of the origin
//
// <space bar>:		  reset to default view
// 
// (esc): quit
// 
//
//



#include "Angel.h"
#include <assert.h>
#include <iostream>
using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;



////////////
class MatrixStack{
	int index;
	int size;
	mat4* matrices;

	public:
		MatrixStack( int numMat = 32 ): index(0), size(numMat){
			matrices = new mat4[numMat];
		}
		
		~MatrixStack(){
			delete[]matrices;
		}

		void push(const mat4& m) {
			assert(index + 1 < size);
			matrices[index++] = m;
		}

		mat4& pop(){
			assert(index - 1 >= 0);
			index--;
			return matrices[index];
		}
};

//////////

// Model matrix:
MatrixStack  mvstack;
mat4         model;

const int NumVertices = 36; //36 = (6 faces)*(2triangles/face)*(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

//vertices of a unit cube
point4 vertices[8] = {
	point4( -0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[6] = {

    color4( 1.0, 1.0, 1.0, 1.0 ),  // initial null value
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 0.7, 0.7, 0.7, 1.0 ),  // grey
    color4( 0.9, 0.9, 0.9, 1.0 ),  // light grey
    color4( 0.5, 0.5, 0.5, 1.0 ),  // dark grey
    color4( 0.0, 0.5, 1.0, 0.75 )   // clear blue
};
enum { none = 0, black = 1, grey = 2, light_grey = 3, dark_grey = 4, clear_blue = 5 };

// default render-mode toggle boolean:
bool lines = false;

// init vertex storage index
int Index = 0;

//frame and window default values
float f_l = -1.0; 
float f_b = -1.0; 
float f_r =  1.0;
float f_t =  1.0;
int window_width = 1024;
int window_height = 1024;

//Spherical viewing coordinates
GLdouble zoomin = 2.5;
GLdouble twist    = 0.0;
GLdouble elevation= 0.0;
GLdouble azimuth  = 0.0;

//animation defaults
float armRotate = 0.0;

//locations of variable sent to shader
GLuint  Model_loc;  
GLuint  View_loc;  
GLuint  Projection_loc;
GLuint  Color_loc;
GLuint  vPosition;
GLuint  vColor;
GLuint  program;


//______________________________________________________________
//functions//
void init();
void display();
void keyboard(unsigned char key, int x, int y);
void specialKeyboard (int key, int x, int y);
void reshape(int w, int h);
void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight);

mat4 polarView(GLdouble zoomin, GLdouble twist, GLdouble elevation, GLdouble azimuth);
//pre: 	zoomin is how far away the camera is from the subject, 
//	the rest are angles or rotation around the axes:	
//	twist:		z axis 
//	elevation:	x axis
//	azimuth:	y axis
//post: the camera is positioned according to the provided information	

void quad( int color, int a, int b, int c, int d );
//pre:	color is the color id (as identified by the vertex_colors array)
//	a is 1st vertex of face
//	b is 2nd vertex of face
//	c is 3rd vertex of face
//	d is 4th vertex of face
//	(note: a and c share the diagonal of the quad)
//
//post: points for two triangles generated for a quadrilateral face of a solid and saved to arrays (colors and points)

void color_cube (int color_index);
//pre: color_index indicates a color, as identified by the array "vertex_colors" 
//post: vertices of cube object are recolored according to indicated color.

void drawRobot();

void torso();
void head();
void eyes();
void leftLeg();
void rightLeg();
void leftArm();
void rightArm();
void leftFoot();
void rightFoot();

void armMvmt();





//______________________________________________________________
////main program 
//

int main( int argc, char **argv )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Robot! (ESC to quit)" );
	glEnable (GL_BLEND); 
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init();

	glutDisplayFunc( display );
	glutKeyboardFunc( keyboard );
	glutSpecialFunc( specialKeyboard );
	glutReshapeFunc(reshape);

	glutMainLoop();
	return 0;
}


//______________________________________________________________



void init() 
{
	color_cube ( none );
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
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(points) +  sizeof(colors),
		  NULL, GL_STATIC_DRAW );

	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
	glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

	// Load shaders and use the resulting shader program
	program = InitShader( "vshader_robot.glsl", "fshader_robot.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
	glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

	vColor = glGetAttribLocation( program, "vColor" ); 
	glEnableVertexAttribArray( vColor );
	glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points) ) );

	// set up uniform variables for model, view, and color to be sent to the shader
	Model_loc = glGetUniformLocation( program, "Model" );
	View_loc = glGetUniformLocation( program, "View" );
	Color_loc = glGetUniformLocation( program, "newColor" );
	

	// Generate the Projection Matrix and send to vertex shader:
	Projection_loc = glGetUniformLocation( program, "Projection" );
	glUniformMatrix4fv( Projection_loc, 1, GL_TRUE, Frustum( -2.0, 2.0, -2.0, 2.0, 1.0, 5.0 ) );



	glEnable( GL_DEPTH_TEST );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClearColor( 0.8, 0.8, 0.8, 1.0 ); 
	}

//----------------------------------------------------------------------------

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// Generate the View matrix and send to vertex shader:
	glUniformMatrix4fv( View_loc, 1, GL_TRUE, polarView(zoomin, twist, elevation, azimuth) );
	
	drawRobot();
	
    glutSwapBuffers();
}

//----------------------------------------------------------------------------
void drawRobot()
{
	torso();	
}
void torso()
{
	mvstack.push(model);

	//model
	mat4 instance = Scale(0.4, 0.6, 0.4);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();

	head();
	rightLeg();
	leftLeg();
	rightArm();
	armMvmt();
	//leftArm();
	leftFoot();
	rightFoot();
}


void head()
{
	mvstack.push(model);
	
	mat4 instance = Translate(0.0, 0.425, 0.0)*Scale(0.25, 0.25, 0.25);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();
	eyes();
}

void eyes()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.08, 0.46, 0.1275)*Scale(0.1, 0.1, 0.05);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = clear_blue;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();

	//other eye
	mvstack.push(model);
	
	//model
        instance = Translate(-0.08, 0.46, 0.1275)*Scale(0.1, 0.1, 0.05);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	c = clear_blue;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );

	model = mvstack.pop();
}

void leftArm()
{
	mvstack.push(model);

	//model
	mat4 instance = RotateZ(6)*Translate(0.27, 0.0, 0.0)*Scale(0.125, 0.5, 0.125);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = light_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void rightArm()
{
	mvstack.push(model);

	//model
	mat4 instance = RotateZ(-6)*Translate(-0.27, 0.0, 0.0)*Scale(0.125, 0.5, 0.125);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );
	
	//color
	int c = light_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void leftLeg()
{
	mvstack.push(model);

	//model
	mat4 instance = RotateZ(2)*Translate(0.1, -0.465, 0.0)*Scale(0.19, 0.35, 0.23);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );
	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void rightLeg()
{
	mvstack.push(model);

	//model
	mat4 instance = RotateZ(-2)*Translate(-0.1, -0.465, 0.0)*Scale(0.19, 0.35, 0.23);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = dark_grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void leftFoot()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.13, -0.7, 0.0)*Scale(0.20, 0.18, 0.25);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}

void rightFoot()
{
	mvstack.push(model);

	//model
	mat4 instance = Translate(-0.13, -0.7, 0.0)*Scale(0.20, 0.18, 0.25);
	model *= instance ;	
	glUniformMatrix4fv( Model_loc, 1, GL_TRUE, model );

	//color
	int c = grey;
	glUniformMatrix4fv( Color_loc, 1, GL_TRUE, Scale(vertex_colors[c].x, vertex_colors[c].y, vertex_colors[c].z) );

	glDrawArrays( GL_TRIANGLES, 0, NumVertices );
	model = mvstack.pop();
}


//----------------------------------------------------------------------------


void armMvmt()

{
	mvstack.push(model);

	//model
	mat4 instance = Translate(0.27, 0.25, 0.0)*RotateX(armRotate)*Translate(-0.27, -0.25, 0.0);

	model *= instance ;	

	leftArm();
	
	model = mvstack.pop();
}











//----------------------------------------------------------------------------
void keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;

	case 'w':
		if (lines){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			lines = false;
		}else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			lines = true;
		}
		glutPostRedisplay();
		break;

	
	case 'l':
		twist 		+=  1.0;
		glutPostRedisplay();
		break;
	case 'r':
		twist 		+= -1.0;
		glutPostRedisplay();
		break;
	case '+':
		zoomin	+= -0.1;
		glutPostRedisplay();
		break;
	case '-':
		zoomin	+=  0.1;
		glutPostRedisplay();
		break;
	case 040: //space key
		zoomin	= 1.7;
		twist		= 0.0;
		elevation	= 0.0;
		azimuth		= 0.0;
		glutPostRedisplay();
		break;



	case 'a':
		armRotate     +=  10.0;
		glutPostRedisplay();
		break;

	case 'z':
		armRotate     += -10.0;
		glutPostRedisplay();
		break;
			
    }
}
//----------------------------------------------------------------------------
void specialKeyboard (int key, int x, int y){
	switch( key ) {
	case GLUT_KEY_LEFT:
		azimuth 	+=  1.0;
		glutPostRedisplay();
		break; 	
	case GLUT_KEY_RIGHT:
		azimuth 	+= -1.0;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_UP:
		elevation 	+= -1.0;
		glutPostRedisplay();
		break; 
	case GLUT_KEY_DOWN:
		elevation 	+=  1.0;
		glutPostRedisplay();
		break; 
	}
}

//----------------------------------------------------------------------------
void reshape( int w, int h )
{	
	window_width = w;
	window_height = h;
	glutPostRedisplay();
	SetAutoVP(f_l, f_b, f_r, f_t, window_width, window_height);

}

//----------------------------------------------------------------------------
void SetAutoVP(float l, float b, float r, float t, int wWidth, int wHeight)
{
	float Frame_a_r  = (r-l)/(t-b);
	float Window_a_r = wWidth/wHeight;	
	float VPx, VPy, VPw, VPh;
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

mat4 polarView(GLdouble zoomin, GLdouble twist, GLdouble elevation, GLdouble azimuth){

	mat4 camera = Translate(0.0, 0.0, -zoomin) * RotateZ(twist) * RotateX(elevation) * RotateY(-azimuth);
	return camera;
}

//----------------------------------------------------------------------------
void quad( int color, int a, int b, int c, int d )
{
	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[a]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[b]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[c]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[a]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[d]; 
	Index++;

	colors[Index] = vertex_colors[color]; 
	points[Index] = vertices[c]; 
	Index++;
}

//----------------------------------------------------------------------------
void color_cube (int color_index)
{
    quad( color_index, 1, 0, 3, 2 );		//Front 
    quad( color_index, 2, 3, 7, 6 );		//Right	
    quad( color_index, 3, 0, 4, 7 );		//Bottom 
    quad( color_index, 6, 5, 1, 2 );		//Top  
    quad( color_index, 4, 5, 6, 7 );		//Back	
    quad( color_index, 5, 4, 0, 1 );		//Left 

}












